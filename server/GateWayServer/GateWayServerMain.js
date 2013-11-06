var tcp = require("../LighterWebEngine/TCP");
var ws = require("../LighterWebEngine/WebSocket");
var cfg = require("../Common/Config");
var log = require("../Common/TSLog");
TSLog = log.create(log.INFO, "GateWayServerMain.log");

/////////////////////////////////////////////////////////
// 向AdapterServer请求UUID和端口号
var hASSocket = tcp.CreateClient(cfg.AdaptServerPort, "",
    function(){
        var sPacket = {};
        sPacket["MM"] = "GW_GetUuidPort"; //请求UUID Port
        tcp.SendBuffer(hASSocket, JSON.stringify(sPacket));
    },
    function(sBuffer){
        var oPacket = JSON.parse(sBuffer);
        switch(oPacket.MM){
            case "GW_GetUuidPort":
                var iPORT_WS = parseInt(oPacket.PORT_WS);
                var iPORT_TCP = parseInt(oPacket.PORT_TCP);
                var iUUID = parseInt(oPacket.UUID);

                G_GateWay.UUID = iUUID;
                G_GateWay.IP = cfg.GateWayServerIP;
                TSLog.info("GateWay UUID:" + G_GateWay.UUID + " IP:" + G_GateWay.IP);
                TSLog = log.create(log.INFO, "GateWayServerMain"+G_GateWay.UUID+".log");

                RunServer_WS(iPORT_WS, iUUID);
                RunServer_TCP(iPORT_TCP, iUUID);
                RunServer(cfg.GateWayServerPort + iUUID, iUUID);
                break;
            case "HS_ConnectHall": //大厅挂了重新连接大厅
                //重新连接大厅
                console.log("重新连接大厅. 成功!");
                G_HallSocket = tcp.CreateClient(cfg.HallServerPort, cfg.HallServerIP,
                    HallConnectSuccess,
                    HallMessageRoute
                );
                break;
        };
    }
);


/////////////////////////////////////////////////////////
// ws服务器启动完成.监听消息
var G_GateWay = {}; // 适配服分配的该进程ID
var G_HallSocket = null; //大厅的Client连接
var G_GateWayTCP = null; //服务器内部通信使用的TCP

var G_ClientNumber = 0;
var G_ClientUUID = 0;
var G_PoolClientSocket = {};

var G_PoolUUIDInGameServer = {};

function RunServer_TCP(iPORT, iUUID) {
    tcp.CreateServer(iPORT,
        function(){
            TSLog.trace("CLient GateWay TCP Init Success! Port:" + iPORT);
            var sPacket = {};
            sPacket.MM = "GW_RegGateWay"; //客户端自动连接网关操作
            sPacket.IP = cfg.GateWayServerIP;
            sPacket.Port = iPORT;
            sPacket.Type = 1;
            sPacket.UUID = iUUID;
            tcp.SendBuffer(hASSocket,JSON.stringify(sPacket));
            ClientServer_Init();
        },
        ClientServer_Msg,
        ClientServer_Close,
        function (hSocket) {
            G_ClientNumber ++;
            G_ClientUUID++;
            hSocket.UUID = G_ClientUUID * cfg.GateWayServerPlayerIDRule + G_GateWay.UUID;
            hSocket.CType = 1;
            G_PoolClientSocket[hSocket.UUID] = hSocket;
            TSLog.debug("TCP newSocket 网关客户数:" + G_ClientNumber + " UUID:" + hSocket.UUID);
            var packet = {};
            packet.MM = "RegistUUID";
            packet.UUID = hSocket.UUID;
            tcp.SendBuffer(hSocket, JSON.stringify(packet))
        }
    )
}

function RunServer_WS(iPORT, iUUID) {
    // 创建客户端服务器
    ws.CreateServer(iPORT,
        function(){
            TSLog.trace("Client GateWay WS Init Success! Port:" + iPORT);
            var sPacket = {};
            sPacket.MM = "GW_RegGateWay"; //客户端自动连接网关操作
            sPacket.IP = cfg.GateWayServerIP;
            sPacket.Port = iPORT;
            sPacket.Type = 2;
            sPacket.UUID = iUUID;
            tcp.SendBuffer(hASSocket,JSON.stringify(sPacket));
            ClientServer_Init();
        },
        ClientServer_Msg,
        ClientServer_Close,
        function (hSocket) {
            G_ClientNumber ++;
            G_ClientUUID++;
            hSocket.UUID = G_ClientUUID * cfg.GateWayServerPlayerIDRule + G_GateWay.UUID;
            hSocket.CType = 2;
            G_PoolClientSocket[hSocket.UUID] = hSocket;
            TSLog.debug("WebSocket newSocket 网关客户数:" + G_ClientNumber + " UUID:" + hSocket.UUID);
            var packet = {};
            packet.MM = "RegistUUID";
            packet.UUID = hSocket.UUID;
            ws.SendBuffer(hSocket, JSON.stringify(packet))
        }
    );
};

function ClientServer_Init() {
    //连接大厅
    G_HallSocket = tcp.CreateClient(cfg.HallServerPort, cfg.HallServerIP,
        HallConnectSuccess,
        HallMessageRoute
    );
}

function ClientServer_Msg(hSocket, sBuffer) {
    TSLog.info("Buffer:" + sBuffer);

    var oPacket = JSON.parse(sBuffer);
    oPacket.UUID = hSocket.UUID;

    // 通过hSocket.UUID 找对应的Game服Socket.然后把数据转发过去.
    if (hSocket.UUID in G_PoolUUIDInGameServer) {
        tcp.SendBuffer(G_PoolUUIDInGameServer[hSocket.UUID],JSON.stringify(oPacket));
        return;
    }

    // 如果没有路由到GameServer. 则进入Hall路由
    if(G_HallSocket == null){
        console.log("没有连接到大厅服务!");
        return;
    }
    tcp.SendBuffer(G_HallSocket,JSON.stringify(oPacket));
}

function ClientServer_Close(hSocket) {
    G_ClientNumber --;
    console.log("close 网关客户数:" + G_ClientNumber + " UUID:" + hSocket.UUID);

    delete G_PoolClientSocket[hSocket.UUID];

    // 通知大厅玩家断开链接
    var sPacket = {
        MM:"ClientOffLine",
        UUID:hSocket.UUID
    };

    tcp.SendBuffer(G_HallSocket,JSON.stringify(sPacket));

    // 如果该玩家在GameServer.那么通知GS服.该玩家断线.
    if (hSocket.UUID in G_PoolUUIDInGameServer) {
        sPacket = {
            MM:"LeaveGame",
            UUID:hSocket.UUID
        };
        tcp.SendBuffer(G_PoolUUIDInGameServer[hSocket.UUID],JSON.stringify(sPacket));
    }
}

function RunServer(iPORT, iUUID) {
    G_GateWayTCP = tcp.CreateServer(iPORT,
        function () {
            TSLog.trace("GateWay TCP Init Success! Port:" + iPORT + " UUID:" + iUUID);

            G_GateWayTCP.UUID = iUUID;
            G_GateWayTCP.PORT = G_GateWayTCP.address().port;
            G_GateWayTCP.IP = G_GateWayTCP.address().address;

        },

        function (hSocket, sBuffer) {
            var oPacket = JSON.parse(sBuffer);

            // 系统消息处理
            switch(oPacket.MM) {
                case "RouteToGameServer":
                    var room = oPacket.Room;
                    for (var iUUID in room.ClientArr){
                        G_PoolUUIDInGameServer[iUUID] = hSocket;
                        hSocket.ClientArr.push(iUUID);

                        if (iUUID in G_PoolClientSocket) {
                            var sPacket = {MM:"EnterGame"};
                            SendBufferToClient(iUUID, sPacket);
                        }
                    }
                    return;
                case "LeaveGame":
                    var iUUID = oPacket.UUID;
                    delete G_PoolUUIDInGameServer[iUUID];
                    return;
            }

            // 玩家消息路由
            var iUUID = oPacket.UUID;
            if (!(iUUID in G_PoolClientSocket)){
                TSLog.error("网关获取了 错误的玩家UUID:" + iUUID + " G_GateWay.UUID:" + G_GateWay.UUID);
                return;
            }
            SendBufferToClient(iUUID, oPacket);
        },

        function (hSocket) {
            for (var i = 0 ; i < hSocket.ClientArr.length; i++) {
                if (hSocket.ClientArr[i] in G_PoolUUIDInGameServer){
                    delete G_PoolUUIDInGameServer[hSocket.ClientArr[i]];
                }
            }
        },

        function (hSocket) {
            hSocket.ClientArr = [];
        }
    );
}

///////////////////////////////////////////////
//大厅消息处理

//大厅连接成功
function HallConnectSuccess() {
    var iUUID = G_GateWayTCP.UUID;
    var sIP = G_GateWayTCP.IP;
    var iPort = G_GateWayTCP.PORT;
    var sPacket = {
        MM:"RegGateWay",
        UUID:iUUID,
        IP: sIP,
        PORT: iPort
    };
    tcp.SendBuffer(G_HallSocket, JSON.stringify(sPacket));
};

//大厅消息路由, 大厅消息发送给客户端.
function HallMessageRoute(sBuffer) {
    var oPacket = JSON.parse(sBuffer);

    var iUUID = oPacket.UUID;

    if (!(iUUID in G_PoolClientSocket)){
        TSLog.error("网关获取了 错误的玩家UUID:" + iUUID + " G_GateWay.UUID:" + G_GateWay.UUID);
        return;
    }
    SendBufferToClient(iUUID, oPacket);
};

//发消息给客户端
function SendBufferToClient(iUUID, oPacket) {
    var hSocket = G_PoolClientSocket[iUUID];
	if(hSocket.CType == 1) {
		tcp.SendBuffer(hSocket, JSON.stringify(oPacket));
		TSLog.debug("TCP SendBufferToClient:" + oPacket);
	} else if(hSocket.CType == 2) {
	    ws.SendBuffer(hSocket, JSON.stringify(oPacket));
		TSLog.debug("WS SendBufferToClient:" + oPacket);
	}
}