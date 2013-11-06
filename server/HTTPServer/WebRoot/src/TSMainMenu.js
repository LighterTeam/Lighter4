cc.dumpConfig();


var TSMainMenu = cc.Layer.extend({

    init:function() {
        var bRet = false;
        if (this._super()) {

            G_SceneState = G_SceneType.TSMainMenuLayer;

            if (G_UserInfo != null) {
                this.InitFrame(G_UserInfo.NAME);
            }

            bRet = true;
        }
        return bRet;
    },

    InitFrame:function (sName) {
        var sp = cc.Sprite.create(s_loading);
        sp.setAnchorPoint(cc.p(0, 0));
        this.addChild(sp, 0, 1);

        var logo = cc.Sprite.create(s_logo);
        logo.setAnchorPoint(cc.p(0, 0));
        logo.setPosition(cc.p(0, 250));
        this.addChild(logo, 10, 1);

        var lbScore = cc.LabelBMFont.create("Name:" + sName + " Port:" + G_SocketInfo.PORT, s_arial14_fnt);
        lbScore.setAnchorPoint( cc.p(1,0) );
        lbScore.setAlignment( cc.TEXT_ALIGNMENT_RIGHT );
        lbScore.setPosition(winSize.width - 15 , winSize.height - 30);
        this.addChild(lbScore, 1000);

        var newGameNormal = cc.Sprite.create(s_menu, cc.rect(0, 0, 126, 33));
        var newGameSelected = cc.Sprite.create(s_menu, cc.rect(0, 33, 126, 33));
        var newGameDisabled = cc.Sprite.create(s_menu, cc.rect(0, 33 * 2, 126, 33));

        var gameSettingsNormal = cc.Sprite.create(s_menu, cc.rect(126, 0, 126, 33));
        var gameSettingsSelected = cc.Sprite.create(s_menu, cc.rect(126, 33, 126, 33));
        var gameSettingsDisabled = cc.Sprite.create(s_menu, cc.rect(126, 33 * 2, 126, 33));

        var aboutNormal = cc.Sprite.create(s_menu, cc.rect(252, 0, 126, 33));
        var aboutSelected = cc.Sprite.create(s_menu, cc.rect(252, 33, 126, 33));
        var aboutDisabled = cc.Sprite.create(s_menu, cc.rect(252, 33 * 2, 126, 33));


        cc.MenuItemFont.setFontName("Arial");
        cc.MenuItemFont.setFontSize(26);
        var label = cc.LabelTTF.create("CreateRoom", "Arial", 20);
        var createRoom = cc.MenuItemLabel.create(label, this.onCreateRoom);
        createRoom.setScale(0.8);

        label = cc.LabelTTF.create("JoinRoom", "Arial", 20);
        var JoinRoom = cc.MenuItemLabel.create(label, this.onJoinRoom);
        createRoom.setScale(0.8);

        var newGame = cc.MenuItemSprite.create(newGameNormal, newGameSelected, newGameDisabled,
            this.onNewGame, this);
        var gameSettings = cc.MenuItemSprite.create(gameSettingsNormal, gameSettingsSelected, gameSettingsDisabled,
            this.onSettings, this);
        var about = cc.MenuItemSprite.create(aboutNormal, aboutSelected, aboutDisabled,
            this.onAbout, this);

        var menu = cc.Menu.create(newGame, gameSettings, about, createRoom, JoinRoom);
        menu.alignItemsVerticallyWithPadding(10);
        this.addChild(menu, 1, 2);
        menu.setPosition(winSize.width / 2, winSize.height / 2 - 80);
        this.schedule(this.update, 0.1);
    },


    onCreateRoom:function(){
        var scene = cc.Scene.create();
        scene.addChild(TSHallLayer.create());
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(1.2, scene));
    },

    onJoinRoom:function(){
        var scene = cc.Scene.create();
        scene.addChild(TSRoomLayer.create());
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(1.2, scene));
    },

    onNewGame:function (pSender) {
        cc.log("onNewGame Clicked!");

        var scene = cc.Scene.create();
        scene.addChild(TSGameLayer.create());
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(1.2, scene));
    },
    onSettings:function (pSender) {
        cc.log("onSettings Clicked!");

        var scene = cc.Scene.create();
        scene.addChild(TSSettingsLayer.create());
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(1.2, scene));
    },
    onAbout:function (pSender) {
        cc.log("onAbout Clicked!");

        var scene = cc.Scene.create();
        scene.addChild(TSAboutLayer.create());
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(1.2, scene));
    },
    update:function () {

    }
});

TSMainMenu.create = function () {
    var sg = new TSMainMenu();
    if (sg && sg.init()) {
        return sg;
    }
    return null;
};

TSMainMenu.scene = function () {
    var scene = cc.Scene.create();
    var layer = TSMainMenu.create();
    scene.addChild(layer);
    return scene;
};

TSMainMenu.MessageProc = function(oPacket) {
    // create a scene. it's an autorelease object

}
