var cfg = require("../Common/Config");
var util = require("../Common/TSUtil");
var uuid = require("../LighterWebEngine/UUID");
var tcp = require("../LighterWebEngine/TCP");
var ws = require('../LighterWebEngine/WebSocket');
var log = require('../Common/TSLog');
TSLog = log.create(log.INFO, "AdaptServer.log");

// 网关池.用来分析.玩家与哪个网关相连
var Pool_GateWay = [];
function CGateWay(Port, IP, Socket, Type, UID) {
    this.UID = UID;
    this.Port = Port;
    this.IP = IP;
    this.Socket = Socket;
    this.Type = Type; //1:TCP 2:WS
}

// 启动适配服
tcp.CreateServer(cfg.AdaptServerPort,
    function() {
        TSLog.trace("Timeshift AdaptServer Success! PORT:" + cfg.AdaptServerPort);
    },

    function(hSocket, sBuffer) {
        var oPacket = JSON.parse(sBuffer);
        switch (oPacket.MM) {
            case "GW_GetUuidPort":  //网关启动获取UUID和动态Port
                GateWay_GetUUID(hSocket);
                break;
            case "GW_RegGateWay":   //通过分配的Port启动网关之后的回馈.注册网关
                GateWay_RegGateWay(hSocket, oPacket);
                break;
            case "HS_ConnectHall":  //大厅重启的时候.通过该消息重连
                Hall_ConnectHall();
                break;
        }
    },

    function(hSocket) {
        var gw;
        for (var i = Pool_GateWay.length-1 ; i >= 0 ; i--) {
            if (Pool_GateWay[i].Socket === hSocket) {
                gw = Pool_GateWay[i];
                Pool_GateWay.splice(i,1);
                console.log("服务器关闭 UUID = " + gw.UID + " 如果是网关断开的端口 = " + gw.Port + " 目前AS上网关数量 = " + Pool_GateWay.length);
            }
        }
    },

    function(hSocket) {

    }
);

function GateWay_GetUUID(hSocket){
    var iUUID = uuid.G_UUID();
    var iPORT = uuid.G_PORT();

    var sPacket = {};
    sPacket["MM"] = "GW_GetUuidPort";
    sPacket["UUID"] = iUUID;
    sPacket["PORT_WS"] = iPORT + cfg.GateWayServerPort_WS;
    sPacket["PORT_TCP"] = iPORT + cfg.GateWayServerPort_TCP;

    tcp.SendBuffer(hSocket, JSON.stringify(sPacket));
}

function GateWay_RegGateWay(hSocket, oPacket) {
    var GW = new CGateWay(oPacket.Port, oPacket.IP, hSocket, oPacket.Type, oPacket.UUID);
    Pool_GateWay.push(GW);
    TSLog.trace("GateWay Regist Success! Port:" + oPacket.Port + " IP:" + oPacket.IP +
        " Type:" + oPacket.Type + " UID:" + oPacket.UUID);
}

function Hall_ConnectHall() {
    for (var i = 0 ; i < Pool_GateWay.length ; i++) {
        var GW = Pool_GateWay[i];
        var sPacket = {
            MM:"HS_ConnectHall"
        };
        tcp.SendBuffer(GW.Socket, JSON.stringify(sPacket));
    }
}

//////////////////////////////////////////////
//获取TCP网关List
function GetTCPGWList() {
    var TCPList = [];
    for (var i = 0 ; i < Pool_GateWay.length ; i++) {
        if (Pool_GateWay[i].Type === 1) {
            TCPList.push(Pool_GateWay[i]);
        }
    }
    return TCPList;
}

//获取WS网关List
function GetWSGWList() {
    var WSList = [];
    for (var i = 0 ; i < Pool_GateWay.length ; i++) {
        if (Pool_GateWay[i].Type === 2) {
            WSList.push(Pool_GateWay[i]);
        }
    }
    return WSList;
}

//服务器流程: 决定登陆的客户端连接哪个网关
var G_ClientNumberTCP = 0;
var G_ClientNumberWS = 0;

var G_ClientNumberTCP_Count = 0;
var G_ClientNumberWS_Count = 0;

tcp.CreateServer(cfg.AdaptServerPort_TCP,
    function () {
        TSLog.trace("Timeshift ClientAdaptTCPServer Success! PORT:" + cfg.AdaptServerPort_TCP);
    },

    function (hSocket, sBuffer) {
        TSLog.info("Buffer:" + sBuffer);
        var oPacket = JSON.parse(sBuffer);
        switch(oPacket.MM) {
            case "ConnectGateWay":
                var list = GetTCPGWList();
                if(list.length <= 0){
                    TSLog.error("当前没有网关开启!");
                    return;
                }

                TSLog.debug("G_ClientNumberTCP:" + G_ClientNumberTCP + " len:" + list.length);
                var index = (G_ClientNumberTCP - 1) % list.length;
                var GW = list[index];

                var sPacket = {};
                sPacket.MM = "ConnectGateWay";
                sPacket.IP = GW.IP;
                sPacket.Port = GW.Port;
                tcp.SendBuffer(hSocket, JSON.stringify(sPacket));
                break;
        }
    },

    function (hSocket) {
        G_ClientNumberTCP_Count --;
        TSLog.trace("TCP客户端下线 登陆人数:" + G_ClientNumberTCP_Count + " 总人数:" +
            (G_ClientNumberTCP_Count + G_ClientNumberWS_Count));
    },

    function (hSocket) {
        G_ClientNumberTCP ++;
        G_ClientNumberTCP_Count++;
        TSLog.trace("TCP客户端上线 登陆人数:" + G_ClientNumberTCP_Count + " 总人数:" +
            (G_ClientNumberTCP_Count + G_ClientNumberWS_Count));
    }
)

ws.CreateServer(cfg.AdaptServerPort_WS,
    function () {
        TSLog.trace("Timeshift ClientAdaptWebSocketServer Success! PORT:" + cfg.AdaptServerPort_WS);
    },

    function (hSocket, sBuffer) {
        var oPacket = JSON.parse(sBuffer);
        switch(oPacket.MM) {
            case "ConnectGateWay":
                var list = GetWSGWList();
                if(list.length <= 0){
                    console.log("当前没有网关开启!");
                    return;
                }

                TSLog.debug("G_ClientNumberWS:" + G_ClientNumberWS + " len:" + list.length);
                var index = (G_ClientNumberWS - 1) % list.length;
                var GW = list[index];

                var sPacket = {};
                sPacket.MM = "ConnectGateWay";
                sPacket.IP = GW.IP;
                sPacket.Port = GW.Port;
                ws.SendBuffer(hSocket, JSON.stringify(sPacket));
                break;
        }
    },

    function (hSocket) {
        G_ClientNumberWS_Count --;
        TSLog.trace("WS客户端下线 登陆人数:" + G_ClientNumberWS_Count + " 总人数:" +
            (G_ClientNumberWS_Count + G_ClientNumberTCP_Count));
    },

    function (hSocket) {
        G_ClientNumberWS ++;
        G_ClientNumberWS_Count++;
        TSLog.trace("WS客户端上线 登陆人数:" + G_ClientNumberWS_Count + " 总人数:" +
            (G_ClientNumberWS_Count + G_ClientNumberTCP_Count));
    }
);

