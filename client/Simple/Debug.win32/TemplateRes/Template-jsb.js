// boot code needed for cocos2d + JS bindings.
// Not needed by cocos2d-html5

require("jsb.js");

var MW = MW || {};

var appFiles = [
    //Engine
    'engine/TSAStar.js',
    'engine/TSPoint.js',

    //Game
    'src/resource.js',
    'src/TSJoinLayer.js',
    'src/TSHallLayer.js',
    'src/TSRoomLayer.js',
    'src/TSSprite.js',
    'src/TSAboutLayer.js',
    'src/TSGameLayer.js',
    'src/TSSettingLayer.js',
    'src/TSMainMenu.js'
];

cc.dumpConfig();

for( var i=0; i < appFiles.length; i++) {
    require( appFiles[i] );
}

var director = cc.Director.getInstance();
director.setDisplayStats(true);

// set FPS. the default value is 1.0/60 if you don't call this
director.setAnimationInterval(1.0 / 60);

G_SocketInfo = {};

winSize = {};
winSize.width = 640;
winSize.height = 480;

G_UserInfo = {};
G_UserInfo.NAME = "Enel";
G_UserInfo.UUID = "123";

G_SceneType = {
    TSMainMenuLayer:0,
    TSAboutLayer:1,
    TSGameLayer:2,
    TSHallLayer:3,
    TSSettingLayer:4,
    TSRoomLayer:5,
    TSJoinLayer:6
};

G_SceneState = -1;//G_SceneType.TSMainMenuLayer;

function MessageProc(sBuffer){
    TSLog("JS->MessageProc:" + sBuffer);

    var oPacket = JSON.parse(sBuffer);
    switch(oPacket.MM) {
        case "RegistUUID":
            var mainScene = TSMainMenu.scene();
            director.runWithScene(mainScene); //进入主场景
            break;
        case "ConnectGateWay":
            G_SocketInfo.PORT = oPacket.Port.toString();

            var sPacket = {};
            sPacket.MM = "SysOrder";
            sPacket.Order = "login:Enel12";
            TSSendBuffer(JSON.stringify(sPacket));
            break;
    }

    switch (G_SceneState) {
        case G_SceneType.TSGameLayer:
            TSGameLayer.MessageProc(oPacket);
            break;
        case G_SceneType.TSMainMenuLayer:
            TSMainMenu.MessageProc(oPacket);
            break;
        case G_SceneType.TSHallLayer:
            TSHallLayer.MessageProc(oPacket);
            break;
        case G_SceneType.TSRoomLayer:
            TSRoomLayer.MessageProc(oPacket);
            break;
        case G_SceneType.TSJoinLayer:
            TSJoinLayer.MessageProc(oPacket);
            break;
    }
}


