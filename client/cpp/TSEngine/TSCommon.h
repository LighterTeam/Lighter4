#ifndef __TSCOMMON_H__
#define __TSCOMMON_H__

#include "TSObject.h"
#include "TSSocket.h"
#include "TSTCP.h"
#include "TSEvent.h"
#include "TSEngine.h"

//"192.168.1.24"   JL
//"192.168.1.224"  TS
//"192.168.1.34"   YL
//"127.0.0.1"      Local

const int AdaptServerPort = 9900; // 唯一
const int AdaptServerPort_WS = 9901;
const char* AdaptServerIP = "192.168.1.224";

// 消息码关键字 (核心程序内建消息码, 前端程序,后台程序,不可使用的消息码):
// BL_Begine (WP)
// BL_End (WP)
// BL (WP)


#endif