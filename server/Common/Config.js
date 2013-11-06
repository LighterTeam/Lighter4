module.exports = {
    AdaptServerPort: 9900, // 唯一 TCP其他服务器
    AdaptServerPort_WS: 9901, // 唯一 WebSocket浏览器使用
    AdaptServerPort_TCP: 9902, // 唯一 TCP客户端使用
    AdaptServerIP: "127.0.0.1",

    GateWayServerPort: 10000, // 取余用
    GateWayServerPort_WS: 20000, // WebSocket浏览器连接的网关地址 20000起
    GateWayServerPort_TCP: 30000, // TCP连接的网关地址 30000起

    GateWayServerPlayerIDRule: 1000, // UUID规则.UUID中包含网关信息
    GateWayServerIP: "127.0.0.1",

    HallServerPort: 9001, // 唯一 大厅服务器(用来组玩家房子)
    HallServerIP: "127.0.0.1",

    HTTPServerPort: 9010, // 唯一 游戏的位置
    HTTPServerIP: "127.0.0.1",

    GameServerMaxRoom: 20, // 一个游戏服务器 最多多少个房子

    end: null
};