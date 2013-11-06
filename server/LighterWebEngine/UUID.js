var Pool_GateWayUUIDSocket = [];
var Adapt_UUID = 0;
var Adapt_PORT = 0;

function G_UUID(){
    return ++Adapt_UUID;
};

function G_PORT(){
    return Adapt_PORT++;
};

function G_GetSocket(UUID) {
    return Pool_GateWayUUIDSocket[UUID];
};

function G_GetUUID(Socket) {
    return Socket.UUID;
};

function G_SetSU(Socket, UUID) {
    Pool_GateWayUUIDSocket[UUID] = Socket;
};

function G_RemoveS(Socket) {
    delete Pool_GateWayUUIDSocket[Socket.UUID];
};

function G_RemoveU(UUID) {
    delete Pool_GateWayUUIDSocket[UUID];
};


function G_GetList() {
    return Pool_GateWayUUIDSocket;
}

module.exports = {
    G_UUID: G_UUID,
    G_PORT: G_PORT,
    G_GetSocket: G_GetSocket,
    G_GetUUID: G_GetUUID,
    G_SetSU: G_SetSU,
    G_RemoveS: G_RemoveS,
    G_RemoveU:G_RemoveU,
    G_GetList:G_GetList
};