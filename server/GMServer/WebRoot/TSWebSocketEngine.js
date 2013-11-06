(function (){
    // initialize
    WebSocketEngine = function(sIP,iPort,funInit,callbackMessage, callbackError) {
        var hSocket = io.connect('http://' + sIP + ':' + iPort);
        hSocket.on('connect', function () {
            funInit();
            hSocket.on('message', function(sBuffer) {
                var parsed = JSON.parse(sBuffer);
                callbackMessage(parsed);
            });
            hSocket.on('disconnect', function() {
                alert("断开连接");
                callbackError();
            });
        });
        return hSocket;
    };

    SendBuffer = function(hSocket, sPacket) {
        hSocket.send(sPacket);
    }
}())