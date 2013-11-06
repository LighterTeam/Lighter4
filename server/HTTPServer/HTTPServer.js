//------------------------------------------------
//TS_HTTP服务
//------------------------------------------------

//请求模块
var libHttp = require('http'); //HTTP协议模块
var libUrl=require('url'); //URL解析模块
var libFs = require("fs"); //文件系统模块
var libPath = require("path"); //路径解析模块
var cfg = require("../Common/Config");
var socketio = require('socket.io');

//Web服务器主函数,解析请求,返回Web内容
var funWebSvr = function (req, res){
    var reqUrl = req.url; //获取请求的url

    //使用url解析模块获取url中的路径名
    var pathName = libUrl.parse(reqUrl).pathname;
    if (libPath.extname(pathName)=="") {
        //如果路径没有扩展名
        pathName+="/"; //指定访问目录
    }

    if (pathName.charAt(pathName.length-1)=="/"){
        //如果访问目录
        pathName+="index.html"; //指定为默认网页
    }

    //使用路径解析模块,组装实际文件路径
    var filePath = libPath.join("./WebRoot",pathName);

    //判断文件是否存在
    libPath.exists(filePath,function(exists){
        if(exists){//文件存在
            //创建只读流用于返回
            var stream = libFs.createReadStream(filePath, {flags : "r", encoding : null});
            stream.pipe(res);
        }
        else { //文件不存在
            console.log(filePath);
        }
    });
}

//创建一个http服务器
var webSvr=libHttp.createServer(funWebSvr);

//指定服务器错误事件响应
webSvr.on("error", function(error) {
    console.log(error); //在控制台中输出错误信息
});

//开始侦听8124端口
webSvr.listen(cfg.HTTPServerPort, function(){
    //向控制台输出服务启动的信息
    console.log('running at http://' + cfg.HTTPServerIP + ":" + cfg.HTTPServerPort + '/');
});

socketio.listen(webSvr);



