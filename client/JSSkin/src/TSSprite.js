
var TSSprite = cc.Sprite.extend({
    pos:new TSPoint(0,0),
    iColor:0
});

TSSprite.CreateSprite = function(pos, color) {

    var fileName = "res/chess" + color + ".png";

    var argnum = arguments.length;
    var sprite = cc.Sprite.create(fileName)

    sprite.pos = pos;
    sprite.iColor = color;

    return sprite;
}


