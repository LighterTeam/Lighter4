var tcp = require("../LighterWebEngine/TCP");
var cfg = require("../Common/Config");
var hs = require("./HallSystem");
var log = require("../Common/TSLog");
TSLog = log.create(log.INFO, "HallServer.log");

var HallSystem = new hs.HallSystem();

// 重启流程保护: 通知AS, AS通知GWS, 重新连接HS.
var G_ASSocket = tcp.CreateClient(cfg.AdaptServerPort, cfg.AdaptServerIP,
    function (){
        TSLog.trace("ConnectAdaptServer Success! Port:" + cfg.AdaptServerPort + " IP:" + cfg.AdaptServerIP);

        // 给AS发请求.然后AS发通知给网关.重新连接HS
        var sPacket = {
            MM:"HS_ConnectHall"
        };
        tcp.SendBuffer(G_ASSocket,JSON.stringify(sPacket));
    },

    function (sBuffer){

    }
);

// 启动大厅服务器
tcp.CreateServer(cfg.HallServerPort,
    function() {
        TSLog.trace("Timeshift HallTCPServer Success! PORT:" + cfg.HallServerPort);
    },

    function(hSocket, sBuffer) {
        var oPacket = JSON.parse(sBuffer);
        switch (oPacket.MM) {
            case "SysOrder": //用户命令行
                console.log(oPacket.MM + ":" +oPacket.Order + ":" + oPacket.UUID);
                HallSystem.ProcessOrder(oPacket.Order, parseInt(oPacket.UUID), hSocket);
                break;
            case "ClientOffLine": //客户端下线
                HallSystem.ClientOffLine(parseInt(oPacket.UUID));
                break;
            case "RegGateWay": //注册网关
                HallSystem.RegGateWay(oPacket, hSocket);
                break;
            case "RegGameServer": //注册游戏服
                HallSystem.RegGameServer(hSocket);
                break;
            case "GS_RemoveRoom": //游戏结束去掉房间计数该值用于负载平衡
                HallSystem.GS_RemoveRoom(oPacket);
                break;
        }
    },

    function(hSocket) {
        HallSystem.Disconnect(hSocket); //断开连接
    },

    function(hSocket) {

    }
);
