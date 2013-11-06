"use strict";

var socketio = require('socket.io');
var http = require('http');

function CreateServer(port, funInit, funReceive, funClose, funConnect) {
    var httpServer = http.createServer();
    httpServer.on('listening', function() {
        funInit();
    });

    httpServer.listen(port);
    var sio = socketio.listen(httpServer);

    sio.on('connection', function (hSocket) {
        funConnect(hSocket);

        hSocket.on('message', function (msg) {
            try {
                funReceive(hSocket, msg);
            } catch(e){
                console.log("WebSocket: " + e.stack);
            }
        });

        hSocket.on('disconnect', function () {
            funClose(hSocket);
        });
    });
};

function SendBuffer(hSocket, sBuffer) {
    hSocket.emit('message', sBuffer);
}

module.exports = {
    SendBuffer: SendBuffer,
    CreateServer: CreateServer
};