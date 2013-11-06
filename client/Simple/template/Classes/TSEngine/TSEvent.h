#ifndef __TSEVENT_H__
#define __TSEVENT_H__

#include <string>
#include <map>
#include <list>
#include "TSObject.h"
#include "json/json.h"

using namespace std;

// 全局回调类型定义
typedef void (TSObject::*TpInstEventFun)(string sBuffer); 
typedef void (TSObject::*TpInstEventJsonFun)(Json::Value jvalue); 


// TS事件系统
class TSEvent : public TSObject {
public:
    TSEvent (){
    }

    static TSEvent* GetSingleTon() {
        static TSEvent tsE;
        return &tsE;
    };

    //---------------------String-------------------------------------------------------
    // 注册函数
    void RegistEvent(string sEventKey, void* pInst, TpInstEventFun pFun);

    // 反注册
    void UnRegistEvent(string sEventKey, void* pInst); 
    void UnRegistEvent(string sEventKey);

    // 发送消息
    void SendMsg(string sEventKey, string sBuffer );

    // Push网络包消息链
    void PushMessge(string sEventKey, string sBuffer);

    //---------------------JSON----------------------------------------------------------
    // 注册函数
    void JSON_RegistEvent(string sEventKey, void* pInst, TpInstEventJsonFun pFun);

    // 反注册
    void JSON_UnRegistEvent(string sEventKey, void* pInst); 
    void JSON_UnRegistEvent(string sEventKey);

    // 发送消息
    void JSON_SendMsg(string sEventKey, Json::Value jValue);

    // Push网络包消息链
    void JSON_PushMessge(string sEventKey, Json::Value jValue);

public:
    map<string, map<void*, TpInstEventFun> > m_MapEvent;
    std::list<std::pair<string, string> > m_MsgList;

    map<string, map<void*, TpInstEventJsonFun> > m_MapJsonEvent;
    std::list<std::pair<string, Json::Value> > m_MsgJsonList;
};

#endif
