/**
 * Created with JetBrains WebStorm.
 * User: Enel
 * Date: 13-4-8
 * Time: 下午1:03
 * To change this template use File | Settings | File Templates.
 */
var tcp = require("../LighterWebEngine/TCP");
var cfg = require("../Common/Config");
var def = require("./StructDefine");
var log = require("../Common/TSLog")

// 初始化Log系统
TSLog = log.create(log.INFO, "HallServer.log")

// 玩家池
var Pool_User = {UUID:{}, NAME:{}};

// 房间ID分配器
var G_RoomIDAdapt = 0;

// 通过UUID查找ROOMID的查找器
var Pool_UUID_ROOM = {};

// 房间池
var Pool_Room = {}; //Key: RoomID Value: CRoom
function CRoom(){
    this.ClientArr = {}; //Key: PUUID, Value: Name
    this.RoomID = 0;
    this.LeaderID = 0;
    this.LeaderName = "";
};

//网关池. Key:网关UUID Value:网关Socket
var Pool_GateWaySocket = {};

//游戏服ID分配器
var G_GameServerIDAdapt = 0;
var Pool_GameServerSocket = {}; //包含游戏服上面的所有信息. Key:UUID Value:CGameServer
function CGameServer(){
    this.RoomNumber = 0;
    this.UUID = 0;
    this.Socket = null;
};

//////////////////////////////////////////////////////
// 大厅服务处理
function HallSystem(){

    this.Disconnect = function(hSocket) {
        if(hSocket.UUID in Pool_GameServerSocket) {
            delete Pool_GameServerSocket[hSocket.UUID];
        }
    }

    this.RegGateWay = function(oPacket, hSocket) {
        Pool_GateWaySocket[oPacket.UUID] = {};
        Pool_GateWaySocket[oPacket.UUID].Socket = hSocket;
        Pool_GateWaySocket[oPacket.UUID].IP = oPacket.IP;
        Pool_GateWaySocket[oPacket.UUID].PORT = oPacket.PORT;
        hSocket.UUID = oPacket.UUID;
    };

    this.RegGameServer = function(hSocket) {
        var iUUID = ++G_GameServerIDAdapt;

        var gs = new CGameServer();
        gs.RoomNumber = 0;
        gs.UUID = iUUID;
        gs.Socket = hSocket;

        Pool_GameServerSocket[iUUID] = gs;
        hSocket.UUID = iUUID;

        var sPacket = {
            MM:"RegGameServer",
            UUID:iUUID
        };
        tcp.SendBuffer(hSocket, JSON.stringify(sPacket));
    };

    this.GS_RemoveRoom = function(oPacket) {
        if (oPacket.GSID in Pool_GameServerSocket) {
            Pool_GameServerSocket[oPacket.GSID].RoomNumber--;
        } else {
            TSLog.error("GS_RemoveRoom Pool_GameServerSocket not find gsid:" + oPacket.GSID);
        }
    };

    this.SendBuffer = function(iUUID, sPacket) {
        tcp.SendBuffer(this.PlayerUUIDGetGateWaySocket(iUUID), JSON.stringify(sPacket));
    };

    this.PlayerUUIDGetGateWayUUID = function (iUUID){
        var iGWUUID = iUUID % cfg.GateWayServerPlayerIDRule;
        return iGWUUID;
    };

    this.PlayerUUIDGetGateWayIP = function(iUUID) {
        var iGWUUID = this.PlayerUUIDGetGateWayUUID(iUUID);
        if (iGWUUID in Pool_GateWaySocket) {
            return Pool_GateWaySocket[iGWUUID].IP;
        }
        return null;
    };

    this.PlayerUUIDGetGateWayPort = function(iUUID) {
        var iGWUUID = this.PlayerUUIDGetGateWayUUID(iUUID);
        if (iGWUUID in Pool_GateWaySocket) {
            return Pool_GateWaySocket[iGWUUID].PORT;
        }
        return null;
    };

    this.PlayerUUIDGetGateWaySocket = function(iUUID) {
        var iGWUUID = this.PlayerUUIDGetGateWayUUID(iUUID);
        if (iGWUUID in Pool_GateWaySocket) {
            return Pool_GateWaySocket[iGWUUID].Socket;
        }
        return null;
    };

    this.ClientOffLine = function(iUUID) {
        if(iUUID == 0) {
            console.log("ClientOffLine:UUID=" + iUUID);
            return;
        }

        this.Msg_RemoveRoom(iUUID);

        if(iUUID in Pool_User.UUID){
            var name = Pool_User.UUID[iUUID].Name;
            delete Pool_User.NAME[name];
            delete Pool_User.UUID[iUUID];

            console.log("玩家下线 名字:" + name +
                " UUID:" + iUUID +
                " 大厅人数:" + Object.keys(Pool_User.UUID).length);
        }
    };

    // 玩家操作处理.
    this.ProcessOrder = function (sBuffer, iUUID, hSocket) {
        var sOrder = sBuffer.split(":");

        switch(sOrder[0]) {
            case "Login":
            case "login":
                var name = sOrder[1];
                var password = 0;
                if(sOrder.length > 2)
                    password = sOrder[2];

                //记录登陆玩家名字和UUID
                if (name in Pool_User.NAME) {
                    console.log("该名称已存在.不能重复登录!");
                    return;
                }

                if (iUUID in Pool_User.UUID) {
                    console.log("该UUID已存在.不能重复登录!");
                    return;
                }

                this.Msg_Login(iUUID, name, password, hSocket);
                return;
        }


        if (!(iUUID in Pool_User.UUID)){
            return;
        }

        switch(sOrder[0]) {
            case "CreateRoom":
                this.Msg_CreateRoom(iUUID, hSocket);
                break;
            case "GetRoomList":
                this.Msg_GetRoomList(iUUID, hSocket);
                break;
            case "RemoveRoom":
                this.Msg_RemoveRoom(iUUID);
                break;
            case "JoinRoom":
                var iRoomID = parseInt(sOrder[1]);
                this.Msg_JoinRoom(iUUID, iRoomID);
                break;
            case "LeaveRoom":
                this.Msg_LeaveRoom(iUUID);
                break;
            case "EnterGame":
                this.Msg_EnterGame(iUUID);
                break;
        }
    };

    this.Msg_EnterGame = function (iUUID) {
        if (!(iUUID in Pool_UUID_ROOM)) {
            TSLog.error("Pool_UUID_ROOM 不存在! "+ iUUID);
            return;
        }
        var roomID = Pool_UUID_ROOM[iUUID];
        var room = Pool_Room[roomID];

        if (room == null) {
            TSLog.error("roomID 不存在! "+ roomID);
            return
        }

        // 找到压力比较小的GameServer;
        var gs = null;
        var iMax = cfg.GameServerMaxRoom;
        for (var i in Pool_GameServerSocket){
            var iter = Pool_GameServerSocket[i];
            if (iter.RoomNumber < iMax) {
                iMax = iter.RoomNumber;
                gs = iter;
            }
        }

        if(gs == null) {
            console.error("Error! 没有可以承载Room的GameServer了!");
            return;
        }

        gs.RoomNumber ++;

        // 发包给游戏服.并主动链接GateWay.需要多少连接多少.
        var sPacket = {};
        sPacket.MM = "EnterGame";
        sPacket.WS = {};
        for (var iU in room.ClientArr) {
            var iGWUUID = this.PlayerUUIDGetGateWayUUID(iU);
            sPacket.WS[iGWUUID] = {};
            sPacket.WS[iGWUUID].IP = this.PlayerUUIDGetGateWayIP(iU);
            sPacket.WS[iGWUUID].Port = this.PlayerUUIDGetGateWayPort(iU);
            sPacket.WS[iGWUUID].UUID = iGWUUID;
        }
        sPacket.Room = room;
        tcp.SendBuffer(gs.Socket, JSON.stringify(sPacket));

        // 进入游戏后销毁房间
        delete Pool_Room[roomID];
    };

    this.Msg_LeaveRoom = function (iUUID) {
        if(!(iUUID in Pool_UUID_ROOM)) {
            return;
        }
        var roomID = Pool_UUID_ROOM[iUUID];

        if(!(roomID in Pool_Room))
        {
            return;
        }

        var room = Pool_Room[roomID];

        delete room.ClientArr[iUUID];

        for (var i in room.ClientArr) {
            var sPacket = {};
            sPacket.UUID = i;
            sPacket.MM = "LeaveRoom";
            sPacket.Data = room;
            this.SendBuffer(sPacket.UUID, sPacket);
        }
    };

    this.Msg_JoinRoom = function(iUUID, iRoomID) {

        if (iRoomID in Pool_Room) {
            var room = Pool_Room[iRoomID];
            room.ClientArr[iUUID] = Pool_User.UUID[iUUID].Name;

            Pool_UUID_ROOM[iUUID] = room.RoomID;

            for (var i in room.ClientArr) {
                var sPacket = {};
                sPacket.UUID = i;
                sPacket.MM = "JoinRoom";
                sPacket.Data = room;
                this.SendBuffer(sPacket.UUID, sPacket);
            }
        } else {
            TSLog.error("加入错误的房间:" + iRoomID);
        }
    };

    this.Msg_RemoveRoom = function(iUUID){
        for (var i in Pool_Room) {
            var room = Pool_Room[i];
            if (room.LeaderID == iUUID){

                for (var j in room.ClientArr) {
                    var sPacket = {};
                    sPacket.UUID = j;
                    sPacket.MM = "RemoveRoom";
                    this.SendBuffer(sPacket.UUID, sPacket);
                }

                delete Pool_Room[i];
                break;
            }
        }
    };

    this.Msg_Login = function(iUUID, sName, sPassword, hSocket){
        Pool_User.UUID[iUUID] = new def.UserStruct(iUUID, sName, sPassword);
        Pool_User.NAME[sName] = Pool_User.UUID[iUUID];

        TSLog.info("大厅的玩家数量:" + Object.keys(Pool_User.UUID).length);

        //通知玩家登陆成功.
        var sPacket = {
            MM : "LoginSuccess",
            UUID : iUUID,
            NAME : sName
        };
        tcp.SendBuffer(hSocket, JSON.stringify(sPacket));
    };

    this.Msg_CreateRoom = function(iUUID, hSocket){
        var room = new CRoom();
        room.RoomID = ++G_RoomIDAdapt;
        room.ClientArr[iUUID] = Pool_User.UUID[iUUID].Name;
        room.LeaderID = iUUID;
        room.LeaderName = Pool_User.UUID[iUUID].Name;
        Pool_Room[room.RoomID] = room;

        Pool_UUID_ROOM[iUUID] = room.RoomID;

        var sName = Pool_User.UUID[iUUID].Name;
        var sPacket = {
            MM: "CreateRoom",
            UUID: iUUID,
            NAME: sName,
            OK: true
        };
        tcp.SendBuffer(hSocket, JSON.stringify(sPacket));
    };

    this.Msg_GetRoomList = function(iUUID, hSocket){
        var sPacket = {};
        sPacket.UUID = iUUID;
        sPacket.MM = "GetRoomList";
        sPacket.Data = Pool_Room;
        tcp.SendBuffer(hSocket, JSON.stringify(sPacket));
    };

};

module.exports = {
    HallSystem:HallSystem
};