var TSHallLayer = cc.Layer.extend({

    menu:null,

    subMenu:null,

    init:function () {
        var bRet = false;
        if (this._super()) {

            G_SceneState = G_SceneType.TSHallLayer;

            var sp = cc.Sprite.create(s_loading);
            sp.setAnchorPoint(cc.p(0,0));
            this.addChild(sp, 0, 1);

            cc.MenuItemFont.setFontName("Arial");
            cc.MenuItemFont.setFontSize(26);
            var label = cc.LabelTTF.create("Back MainMenu", "Arial", 20);
            var back = cc.MenuItemLabel.create(label, this.onBackCallback);
            back.setScale(0.8);

            var cp_back = back.getPosition();
            cp_back.y = -175.0;
            back.setPosition(cp_back);

            label = cc.LabelTTF.create("EnterGame", "Arial", 20);
            var enter = cc.MenuItemLabel.create(label, this.onEnterGameServer);
            enter.setScale(0.8);

            cp_back = enter.getPosition();
            cp_back.y = -200.0;
            enter.setPosition(cp_back);

            this.menu = cc.Menu.create(back, enter);
            this.addChild(this.menu);

            this.subMenu = cc.Menu.create();
            this.addChild(this.subMenu);

            bRet = true;

            var sPacket = {
                MM:"SysOrder",
                Order:"CreateRoom"
            };
            TSSendBuffer(JSON.stringify(sPacket));
        }

        return bRet;
    },

    onEnterGameServer:function(pSender) {
        var sPacket = {
            MM:"SysOrder",
            Order:"EnterGame"
        };
        TSSendBuffer(JSON.stringify(sPacket));
    },

    onBackCallback:function (pSender) {
        var sPacket = {
            MM:"SysOrder",
            Order:"RemoveRoom"
        };
        TSSendBuffer(JSON.stringify(sPacket));
    },

    onMessageProc:function(oPacket){

        switch (oPacket.MM) {
            case "CreateRoom":
                var sName = oPacket.NAME;

                var label = cc.LabelTTF.create("MainPlayer:"+sName, "Arial", 20);
                var back = cc.MenuItemLabel.create(label);
                back.setScale(0.8);

                var cp_back = back.getPosition();
                cp_back.y = 200;
                back.setPosition(cp_back);

                this.menu.addChild(back);
                break;

            case "JoinRoom":
            case "LeaveRoom":
                var room = oPacket.Data;

                this.subMenu.removeAllChildren(true);

                var index = 1;
                for (var i in room.ClientArr) {
                    var sName = room.ClientArr[i];
                    var iUUID = i;

                    var label = cc.LabelTTF.create("Player:"+sName, "Arial", 20);
                    var back = cc.MenuItemLabel.create(label);
                    back.setScale(0.8);

                    var cp_back = back.getPosition();
                    cp_back.y = 175 - 25 * index++;
                    back.setPosition(cp_back);

                    this.subMenu.addChild(back);
                }
                break;

            case "RemoveRoom":
                var scene = cc.Scene.create();
                scene.addChild(TSMainMenu.create());
                cc.Director.getInstance().replaceScene(cc.TransitionFade.create(1.2, scene));
                break;

            case "EnterGame":
                var scene = cc.Scene.create();
                scene.addChild(TSGameLayer.create());
                cc.Director.getInstance().replaceScene(cc.TransitionFade.create(1.2, scene));
                break;
        }
    }
});

var G_TSHallLayer = null;

TSHallLayer.create = function () {
    var sg = new TSHallLayer();
    if (sg && sg.init()) {
        G_TSHallLayer = sg;
        return sg;
    }
    return null;
};

TSHallLayer.MessageProc = function (oPacket) {
    if (G_TSHallLayer == null)
        return;
    G_TSHallLayer.onMessageProc(oPacket);
};

