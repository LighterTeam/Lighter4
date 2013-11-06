#ifndef __TSLog_H__
#define __TSLog_H__

#define TSLog(FMT,...) cocos2d::CCLog2(__FILE__,__LINE__,FMT,__VA_ARGS__);

#endif