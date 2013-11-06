"use strict";

var net = require('net');
var ExBuffer = require('ExBuffer');
var ByteBuffer = require('ByteBuffer');

function CreateServer(iPort, funInit, funReceive, funClose, funConnect) {
    var server = net.createServer(function(hSocket) {
        // 粘包
        var exBuffer = new ExBuffer();
        exBuffer.on('data', function(hBuffer){
            var bytebuf = new ByteBuffer(hBuffer);
            var resArr = bytebuf.string().unpack();
            funReceive(hSocket, resArr[0]);
        });

        // 监听收包
        hSocket.on('data', function(data) {
            try {
                exBuffer.put(data);
            }
            catch(e){
                console.log("Server TCP: " + e.stack);
            }
        });

        // 断开
        hSocket.on('close', function() {
            funClose(hSocket);
        });

        //数据错误事件
        hSocket.on('error',function(exception){
            //console.log('socket error:' + exception);
            hSocket.end();
        });

    });
    server.listen(iPort);

    server.on('listening', function() {
        funInit();
    });

    server.on('connection', function(hSocket) {
        funConnect(hSocket);
    });

    return server;
};

function SendBuffer(hSocket, sBuffer) {
    var byBuffer = new ByteBuffer();
    var buf = byBuffer.string(sBuffer).pack(true);
    hSocket.write(buf);
};


function CreateClient(iPort, sHost, funInit, funReceive) {

    if(sHost = "") {
        sHost = "127.0.0.1";
    }

    var exBuffer = new ExBuffer();
    var hSocket = net.connect(iPort, sHost, function() {
        if (funInit != null)
            funInit();
    });

    hSocket.on('data', function(data) {
        try {
            exBuffer.put(data);
        }
        catch(e){
            console.log("Client TCP: " + e.stack);
        }
    });

    hSocket.on('error',function(error){
        console.log('error:'+ error);
        //hSocket.destory();
    });

    hSocket.on('close',function(){
        console.log('Connection closed');
    });

    //当客户端收到完整的数据包时
    exBuffer.on('data', function(buffer) {
        var bytebuf = new ByteBuffer(buffer);
        var resArr = bytebuf.string().unpack();
        if (funReceive != null)
            funReceive(resArr[0]);
    });

    return hSocket;
}

function Close(server) {
    server.close();
}


module.exports = {
    CreateServer: CreateServer,
    CreateClient: CreateClient,
    SendBuffer: SendBuffer,
    Close: Close
};





