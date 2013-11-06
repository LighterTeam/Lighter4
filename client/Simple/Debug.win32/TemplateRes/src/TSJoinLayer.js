
var TSJoinLayer = cc.Layer.extend({

    subMenu: null,

    init:function () {
        var bRet = false;
        if (this._super()) {

            G_SceneState = G_SceneType.TSJoinLayer;

            var sp = cc.Sprite.create(s_loading);
            sp.setAnchorPoint(cc.p(0,0));
            this.addChild(sp, 0, 1);

            cc.MenuItemFont.setFontName("Arial");
            cc.MenuItemFont.setFontSize(26);
            var label = cc.LabelTTF.create("Back MainMenu", "Arial", 20);
            var back = cc.MenuItemLabel.create(label, this.onBackCallback);
            back.setScale(0.8);

            var menu = cc.Menu.create(back);
            this.addChild(menu);

            this.subMenu = cc.Menu.create();
            this.addChild(this.subMenu);

            var cp_back = back.getPosition();
            cp_back.y = 200.0;
            back.setPosition(cp_back);

            bRet = true;
        }

        return bRet;
    },

    onBackCallback:function (pSender) {
        var scene = cc.Scene.create();
        scene.addChild(TSMainMenu.create());
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(1.2, scene));

        var sPacket = {
            MM:"SysOrder",
            Order:"LeaveRoom"
        };
        SendBuffer(G_hSocket, sPacket);
    },

    onMessageProc:function (oPacket) {
        switch(oPacket.MM) {
            case "JoinRoom":
            case "LeaveRoom":

                var room = oPacket.Data;

                this.subMenu.removeAllChildren(true);

                var index = 1;
                for (var i in room.ClientArr) {
                    var sName = room.ClientArr[i];

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
                scene.addChild(TSRoomLayer.create());
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

var G_TSJoinLayer = null;

TSJoinLayer.create = function () {
    var sg = new TSJoinLayer();
    if (sg && sg.init()) {
        G_TSJoinLayer = sg;
        return sg;
    }
    return null;
};

TSJoinLayer.MessageProc = function(oPacket) {
    G_TSJoinLayer.onMessageProc(oPacket);
};
