cc.dumpConfig();

var TSGameLayer = cc.Layer.extend({
    //Sys
    sprite: null,

    //GameAS
    m_pMeshPos: null,
    m_pOO: null,
    m_Map: null,
    m_Star: null,

    //对手
    m_pSubOO: null,
    m_pSubMeshPos: null,
    m_pPoolSubBall: null,

    //Game
    m_Choose: null,
    m_SpiritPool: null,
    m_pPathSpriteList: null,
    m_MapSpr: null,
    m_pPath: null,
    m_iIndexPath: null,
    m_iStat: null,

    //缓存ball
    m_vCacheSpr: [],

    rand: function(num) {
        return parseInt(Math.random()*num);
    },

    randomBall: function(){
        var Pos = new TSPoint(0,0);
        var spr = TSSprite.CreateSprite(Pos, this.rand(5));
        spr.setPosition(cc.p(50 + 33*this.m_vCacheSpr.length, winSize.height - 60));
        this.addChild(spr);
        this.m_vCacheSpr.push(spr);

        return true;
    },

    random3Ball: function() {
        for (var i = 0 ; i < this.m_vCacheSpr.length ; i++) {
            var EmptyMap = [];
            for (var j = 0; j < this.m_Map.m_width * this.m_Map.m_height; j++) {
                if (this.m_Map.m_map[j] == 0) {
                    var l = parseInt(j / this.m_Map.m_width);
                    var h = parseInt(j % this.m_Map.m_height);
                    EmptyMap.push([l, h]);
                }
            }

            if (EmptyMap.length <= 0) {
                return false;
            }

            var choose = this.rand(EmptyMap.length);
            var Pos = new TSPoint(EmptyMap[choose][0], EmptyMap[choose][1]);
            var spr = this.m_vCacheSpr[i];
            spr.pos = Pos;
            this.m_Map.m_map[Pos.m_x * this.m_Map.m_width + Pos.m_y] = 1;
            this.m_MapSpr[Pos.m_x][Pos.m_y] = spr;
            this.m_SpiritPool.push(spr);
            var loc = this.m_pMeshPos[Pos.m_x][Pos.m_y];
            spr.setPosition(loc);

            var action = cc.ScaleBy.create(0.5, 0.5);
            var action_back = action.reverse();
            var seq = cc.Sequence.create(action, action_back);
            spr.runAction(seq);

            this.removeBall(spr);
        }
        this.m_vCacheSpr = [];

        for (var i = 0; i < 3; i++) {
            if(!this.randomBall()) {
                return false;
            }
        }

        return true;
    },

    init:function () {
        this.sprite = null;

        //GameAS
        this.m_pMeshPos = [[0],[0],[0],[0],[0],[0],[0],[0],[0]];
        this.m_pOO = new TSPoint(0,0);
        this.m_Map = new TSMap();
        this.m_Star = new TSAStar();

        //对手起点坐标
        this.m_pSubOO = new TSPoint(0,0);
        this.m_pSubMeshPos = [[0],[0],[0],[0],[0],[0],[0],[0],[0]];
        this.m_pPoolSubBall = [];

        //Game
        this.m_Choose = null;
        this.m_SpiritPool = [];
        this.m_pPathSpriteList = [];
        this.m_MapSpr = [[null],[null],[null],[null],[null],[null],[null],[null],[null]];
        this.m_pPath = [];
        this.m_iIndexPath = 0;
        this.m_iStat = 0;
        this.m_vCacheSpr = [];

        this.schedule(this.draw, 0);

        var bRet = false;
        if (this._super()) {
            G_SceneState = G_SceneType.TSGameLayer;

            var sp = cc.Sprite.create(s_background);
            sp.setPosition(cc.p(winSize.width / 2, winSize.height / 2));
            this.addChild(sp, 0, 1000);

            var bg = cc.Sprite.create(s_board_test);
            bg.setPosition(cc.p(winSize.width / 2 - 320 / 2, winSize.height / 2));
            bg.setScale(0.5);
            this.addChild(bg, 0, 1001);

            bg = cc.Sprite.create(s_board_test);
            bg.setPosition(cc.p(winSize.width / 2 + 320 / 2, winSize.height / 2));
            bg.setScale(0.5);
            this.addChild(bg, 0, 1001);


            var pR = bg.getTextureRect();
            cc.log(JSON.stringify(pR))
            this.m_pOO = new TSPoint(winSize.width/2 - 320 / 2 - pR.width/4, winSize.height/2 - pR.height/4);

            for (var i = 0; i < 9; i++) {
                for (var j = 0; j < 9; j++) {
                    this.m_pMeshPos[i][j] = cc.p(this.m_pOO.m_x + i * 33 + 32/2, this.m_pOO.m_y + j * 33 + 32/2);
                }
            }

            // 初始化对手数据
            this.m_pSubOO = new TSPoint(winSize.width/2 + 320 / 2 - pR.width/4, winSize.height/2 - pR.height/4);
            for (var i = 0; i < 9; i++) {
                for (var j = 0; j < 9; j++) {
                    this.m_pSubMeshPos[i][j] = cc.p(this.m_pSubOO.m_x + i * 33 + 32/2, this.m_pSubOO.m_y + j * 33 + 32/2);
                }
            }

            this.random3Ball();
            this.random3Ball();

            // 一次移动球操作之后.需要发包同步球位置数据
            var sPacket = {};
            sPacket.MM = "C_AllBallSys";
            sPacket.Ball = [];
            for (var i = 0 ; i < this.m_SpiritPool.length; i++) {
                var spr = this.m_SpiritPool[i];
                var _tmp = {};
                _tmp.Pos = spr.pos;
                _tmp.Color = spr.iColor;
                sPacket.Ball.push(_tmp);
            }
            sPacket.Map = this.m_Map.m_map;
            TSSendBuffer(JSON.stringify(sPacket));


            ///Sys
            cc.MenuItemFont.setFontName("Arial");
            cc.MenuItemFont.setFontSize(26);
            var label = cc.LabelTTF.create("MainMenu", "Arial", 20);
            var back = cc.MenuItemLabel.create(label, this.onBackCallback);
            back.setScale(0.8);

            var menu = cc.Menu.create(back);
            menu.alignItemsInColumns(1);
            this.addChild(menu);

            var cp_back = back.getPosition();
            cp_back.x = 0;
            cp_back.y = -210.0;
            back.setPosition(cp_back);

            if( 'keyboard' in sys.capabilities ) {
                this.setKeyboardEnabled(true);
                TSLog("keyboard!");
            }
            if( 'mouse' in sys.capabilities ) {
                this.setMouseEnabled(true);
                TSLog("mouse!");
            }
            if( 'touches' in sys.capabilities ) {
                this.setTouchEnabled(true);
                TSLog("touches!");
            }

            bRet = true;
        }
        return bRet;
    },

    onBackCallback:function (pSender) {
        var sPacket = {MM:"LeaveGame"};
        TSSendBuffer(JSON.stringify(sPacket));

        //TSTS
        var scene = cc.Scene.create();
        scene.addChild(TSMainMenu.create());
        cc.Director.getInstance().replaceScene(cc.TransitionFade.create(1.2, scene));
    },

    onTouchesBegan:function (touches, event) {
        this.processEvent( touches[0] );
        return true;
    },

    onMouseDown:function( event ) {
        this.processEvent( event );
        return true;
    },

    GetMeshSprite: function(pos) {
        for (var i = 0; i < this.m_SpiritPool.length; i++) {
            var spr = this.m_SpiritPool[i];
            if (spr.pos.m_x == pos.m_x && spr.pos.m_y == pos.m_y) {
                return spr;
            }
        }
        return null;
    },

    processEvent:function( event ) {
        var touchLocation = event.getLocation();
        var pGY = new TSPoint(touchLocation.x - this.m_pOO.m_x, touchLocation.y - this.m_pOO.m_y);

        var xy = new TSPoint(pGY.m_x / 33, pGY.m_y / 33);
        if (xy.m_x > 8 || xy.m_y > 8) {
            return false;
        }

        cc.log("我被点中了! x = " + xy.m_x + " y = " + xy.m_y);

        if (this.m_iStat == 0) {
            this.m_Choose = this.GetMeshSprite(xy);
            if (this.m_Choose != null) {
                this.m_iStat = 1;
                var action = cc.ScaleBy.create(0.5, 0.5);
                var action_back = action.reverse();
                var seq = cc.Sequence.create(action, action_back);
                this.m_Choose.runAction(cc.RepeatForever.create(seq));
            }
        }else if(this.m_iStat == 1) {
            this.m_pPath = [];
            this.m_iIndexPath = 0;

            //new to find for best path
            var pO = new TSPoint(this.m_Choose.pos.m_x, this.m_Choose.pos.m_y);
            var pT = xy;

            this.m_Star.Init(pO, pT, this.m_Map);
            this.m_Star.run();

            var tsNode = this.m_Star.getResult();
            if (tsNode.pPos.m_x != pT.m_x || tsNode.pPos.m_y != pT.m_y) {
                cc.log("错误的寻路!");
                this.m_iStat = 0;

                this.m_Choose.stopAllActions();
                this.m_Choose.setScale(1);
                this.m_Choose = null;

                return false;
            }

            var pR = [];
            while (tsNode.pFather != null) {
                pR.unshift(tsNode.pPos);
                tsNode = tsNode.pFather;
            }

            for (var i = 0; i < pR.length; i++) {
                this.m_pPath.push(pR[i]);
            }

            this.m_iStat = 2;
            this.m_Map.m_map[this.m_Choose.pos.m_x * this.m_Map.m_width + this.m_Choose.pos.m_y] = 0;
            this.m_MapSpr[this.m_Choose.pos.m_x][this.m_Choose.pos.m_y] = null;
        }

        return true;
    },

    removeBall:function(pChoose) {
        if (pChoose == null) {
            return false;
        }
        var pO = pChoose.pos.get();
        var pRList = [];

        var pUDList = [];
        var count = 0;
        var index = 0;
        //up down
        for (; ; index++) {
            pO.m_y--;
            if (pO.m_y < 0) {
                break;
            }

            var spr = this.m_MapSpr[pO.m_x][pO.m_y];
            if (spr != null) {
                if (spr.iColor == pChoose.iColor) {
                    count ++;
                    pUDList.push(spr);
                }
                else{
                    break;
                }
            }
            else{
                break;
            }
        }

        pO = pChoose.pos.get();
        index = 0;
        //up down
        for (; ; index++) {
            pO.m_y++;
            if (pO.m_y > 8) {
                break;
            }

            var spr = this.m_MapSpr[pO.m_x][pO.m_y];
            if (spr != null) {
                if (spr.iColor == pChoose.iColor) {
                    count ++;
                    pUDList.push(spr);
                }
                else{
                    break;
                }
            }
            else{
                break;
            }
        }

        if (count >= 4) {
            pRList = pRList.concat(pUDList);
        }


        /////////////////////////////
        pO = pChoose.pos.get();
        count = 0;
        index = 0;
        var pLRList = [];
        //left right
        for (; ; index++) {
            pO.m_x--;
            if (pO.m_x < 0) {
                break;
            }

            var spr = this.m_MapSpr[pO.m_x][pO.m_y];
            if (spr != null) {
                if (spr.iColor == pChoose.iColor) {
                    count ++;
                    pLRList.push(spr);
                }
                else{
                    break;
                }
            }
            else{
                break;
            }
        }

        pO = pChoose.pos.get();
        index = 0;
        //up down
        for (; ; index++) {
            pO.m_x++;
            if (pO.m_x > 8) {
                break;
            }

            var spr = this.m_MapSpr[pO.m_x][pO.m_y];
            if (spr != null) {
                if (spr.iColor == pChoose.iColor) {
                    count ++;
                    pLRList.push(spr);
                }
                else{
                    break;
                }
            }
            else{
                break;
            }
        }

        if (count >= 4) {
            pRList = pRList.concat(pLRList);
        }


        // \//
        pO = pChoose.pos.get();
        count = 0;
        index = 0;
        var pXList = [];
        //left right
        for (; ; index++) {
            pO.m_x--;
            pO.m_y--;
            if (pO.m_x < 0 || pO.m_y < 0) {
                break;
            }

            var spr = this.m_MapSpr[pO.m_x][pO.m_y];
            if (spr != null) {
                if (spr.iColor == pChoose.iColor) {
                    count ++;
                    pXList.push(spr);
                }
                else{
                    break;
                }
            }
            else{
                break;
            }
        }

        pO = pChoose.pos.get();
        index = 0;
        //up down
        for (; ; index++) {
            pO.m_x++;
            pO.m_y++;
            if (pO.m_x > 8 || pO.m_y > 8) {
                break;
            }

            var spr = this.m_MapSpr[pO.m_x][pO.m_y];
            if (spr != null) {
                if (spr.iColor == pChoose.iColor) {
                    count ++;
                    pXList.push(spr);
                }
                else{
                    break;
                }
            }
            else{
                break;
            }
        }

        if (count >= 4) {
            pRList = pRList.concat(pXList);
        }

        ///////////////////
        pO = pChoose.pos.get();
        count = 0;
        index = 0;
        var pXXList = [];
        for (; ; index++) {
            pO.m_x--;
            pO.m_y++;
            if (pO.m_x < 0 || pO.m_y > 8) {
                break;
            }

            var spr = this.m_MapSpr[pO.m_x][pO.m_y];
            if (spr != null) {
                if (spr.iColor == pChoose.iColor) {
                    count ++;
                    pXXList.push(spr);
                }
                else{
                    break;
                }
            }
            else{
                break;
            }
        }

        pO = pChoose.pos.get();
        index = 0;
        for (; ; index++) {
            pO.m_x++;
            pO.m_y--;
            if (pO.m_x > 8 || pO.m_y < 0) {
                break;
            }

            var spr = this.m_MapSpr[pO.m_x][pO.m_y];
            if (spr != null) {
                if (spr.iColor == pChoose.iColor) {
                    count ++;
                    pXXList.push(spr);
                }
                else{
                    break;
                }
            }
            else{
                break;
            }
        }

        if (count >= 4) {
            pRList = pRList.concat(pXXList);
        }

        for (var i = 0; i < pRList.length; i++) {
            var spr = pRList[i];

            this.m_Map.m_map[spr.pos.m_x * this.m_Map.m_width + spr.pos.m_y] = 0;
            this.m_MapSpr[spr.pos.m_x][spr.pos.m_y] = null;

            this.removeChild(spr, true);

            for (var j = 0 ; j < this.m_SpiritPool.length ; j++) {
                if (spr.pos.equal(this.m_SpiritPool[j].pos)) {
                    this.m_SpiritPool.splice(j,1);
                    break;
                }
            }
        }

        if (pRList.length > 0) {
            var spr = pChoose;

            this.m_Map.m_map[spr.pos.m_x * this.m_Map.m_width + spr.pos.m_y] = 0;
            this.m_MapSpr[spr.pos.m_x][spr.pos.m_y] = null;
            this.removeChild(spr, true);

            for (var i = 0 ; i < this.m_SpiritPool.length ; i++) {
                if (spr.pos.equal(this.m_SpiritPool[i].pos)) {
                    this.m_SpiritPool.splice(i,1);
                    break;
                }
            }

            return true;
        }
        return false;
    },

    draw : function() {
        if (this.m_iStat != 2) {
            if (this.m_Choose != null) {
            }
            return;
        }

        if (this.m_pPath.length <= 0) {
            return;
        }

        if (this.m_pPath.length <= this.m_iIndexPath) {
            var iter = this.m_pPath[this.m_pPath.length-1];
            var l = iter.m_x;
            var h = iter.m_y;
            this.m_Map.m_map[l * this.m_Map.m_width + h] = 1;
            this.m_MapSpr[l][h] = this.m_Choose;
            this.m_iStat = 0;

            this.m_Choose.stopAllActions();
            this.m_Choose.setScale(1);

            if (this.removeBall(this.m_Choose)) {
                //add score
                this.m_Choose = null;
            }
            else {
                //add 3 ball
                if (!this.random3Ball()) {

                }

                // 结束游戏流程.
                if (this.m_SpiritPool.length >= 81) {
                    for (var i = 0; i < 81; i++) {
                        this.m_Map.m_map[i] = 0;
                    }
                    this.m_MapSpr = [[null],[null],[null],[null],[null],[null],[null],[null],[null]];
                    for (var i = 0; i < this.m_SpiritPool.length; i++) {
                        this.removeChild(this.m_SpiritPool[i], true);
                    }
                    this.m_SpiritPool = [];
                    this.m_Choose = null;
                    this.m_iStat = 0;

                    this.random3Ball();
                }
            }

            // 一次移动球操作之后.需要发包同步球位置数据
            var sPacket = {};
            sPacket.MM = "C_AllBallSys";
            sPacket.Ball = [];
            for (var i = 0 ; i < this.m_SpiritPool.length; i++) {
                var spr = this.m_SpiritPool[i];
                var _tmp = {};
                _tmp.Pos = spr.pos;
                _tmp.Color = spr.iColor;
                sPacket.Ball.push(_tmp);
            }
            sPacket.Map = this.m_Map.m_map;
            TSSendBuffer(JSON.stringify(sPacket));

            //测试用
//            for (var i = 0; i < this.m_pPathSpriteList.length; i++) {
//                this.removeChild(this.m_pPathSpriteList[i], true);
//            }
//            this.m_pPathSpriteList = [];
//
//            for (var i = 0; i < this.m_Map.m_width * this.m_Map.m_height; i++) {
//                var l = parseInt(i / this.m_Map.m_width);
//                var h = parseInt(i % this.m_Map.m_height);
//
//                if (this.m_Map.m_map[i] == 0) {
//                    continue;
//                }
//                var pT = cc.Sprite.create("res/chess1.png");
//                this.m_pPathSpriteList.push(pT);
//                var pP = this.m_pMeshPos[l][h];
//                pT.setPosition(pP);
//                pT.setScale(0.5);
//                this.addChild(pT, 2, 1);
//            }
            return;
        }

        var pPos = this.m_pPath[this.m_iIndexPath];

        var pEnd = this.m_pMeshPos[pPos.m_x][pPos.m_y];
        var pT = this.m_Choose.getPosition();
        var pMove = pT;

        if (Math.abs(pEnd.x - pT.x) < 16 && Math.abs(pEnd.y - pT.y) < 16 ) {
            this.m_iIndexPath ++;
            this.m_Choose.pos = pPos;
            pMove = pEnd;
        }
        else {
            if (pEnd.x < pT.x) {
                pMove.x -= 8;
            }
            else if (pEnd.x > pT.x) {
                pMove.x += 8;
            }
            if (pEnd.y < pT.y) {
                pMove.y -= 8;
            }
            else if (pEnd.y > pT.y) {
                pMove.y += 8;
            }
        }
        this.m_Choose.setPosition(pMove);
    },

    onMessageProc:function (oPacket) {
        switch(oPacket.MM){
            case "C_AllBallSys":
                for (var i = 0 ; i < this.m_pPoolSubBall.length ; i++){
                    this.removeChild(this.m_pPoolSubBall[i], true);
                }
                this.m_pPoolSubBall = [];

                for (var i = 0 ; i < oPacket.Ball.length ; i++){
                    var iter = oPacket.Ball[i];
                    var spr = TSSprite.CreateSprite(iter.Pos, iter.Color);
                    var loc = this.m_pSubMeshPos[iter.Pos.m_x][iter.Pos.m_y];
                    spr.setPosition(loc);
                    this.addChild(spr);

                    this.m_pPoolSubBall.push(spr);
                }

                break;
        }
    }
});

var G_TSGameLayer = null;

TSGameLayer.create = function () {
    var sg = new TSGameLayer();
    if (sg && sg.init()) {
        G_TSGameLayer = sg;
        return sg;
    }
    return null;
};

TSGameLayer.scene = function () {
    var scene = cc.Scene.create();
    var layer = TSGameLayer.create();
    scene.addChild(layer);
    return scene;
};

TSGameLayer.MessageProc = function(oPacket) {
    G_TSGameLayer.onMessageProc(oPacket);
}


