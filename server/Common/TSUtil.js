var fs = require("fs");

function TSRandom(Max){
    return parseInt(Math.random()*Max);
}

function TSReverse(srcString){
    var temp = [];
    for(var i=srcString.length-1;i>-1;i--){
        temp.push(srcString.charAt(i));
    }
    return temp.join("").toString();
}

function TSOpenFile(filename) { //"./Config/CMRule.csv"
    var CMRule = [];
    var fData = fs.readFileSync(filename);
    var sFileList = fData.toString().split('\n');
    for (var i = 4 ; i < sFileList.length; i++) {
        var slineList = sFileList[i].split(',');
        var data = [slineList[0],slineList[1],parseInt(slineList[2])];
        CMRule.push(data);
    }
    return CMRule;
}

module.exports = {
    TSReverse: TSReverse,
    TSRandom: TSRandom,
    TSOpenFile: TSOpenFile
};

