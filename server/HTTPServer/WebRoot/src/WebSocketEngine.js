(function (){
    // initialize
    WebSocketEngine = function(sIP,iPort,funInit,callbackMessage, callbackError) {
        var hSocket = io.connect('http://' + sIP + ':' + iPort);
        hSocket.on('connect', function () {
            funInit();
            hSocket.on('message', function(sBuffer) {
                var parsed = JSON.parse(sBuffer);
                TSLog(parsed.MM);
                callbackMessage(parsed);
            });
            hSocket.on('disconnect', function() {
                alert("断开连接");
                callbackError();
            });
        });

        TSLog("GoInThere!");
        return hSocket;
    };

    TSSendBuffer = function(sPacket) {
        if ("send" in G_hSocket) {
            G_hSocket.send(sPacket);
        } else {
			TSLog("G_hSocket non!")
		}
    }
}())