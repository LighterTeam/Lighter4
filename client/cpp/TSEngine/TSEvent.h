#ifndef __TSEVENT_H__
#define __TSEVENT_H__

#include <string>
#include <map>
#include <list>
#include "TSEngine/TSObject.h"
using namespace std;

// 全局回调类型定义;
typedef void (TSObject::*TpInstEventFun)(string sBuffer); 
typedef void (TSObject::*TpInstEventFunRoot)(char* cBuffer, int iLen); 

// TS事件系统;
class TSEvent : public TSObject {
public:
    TSEvent () : m_RootEvent(0), m_pFunRootEvent(0){
    }

    static TSEvent* GetSingleTon() {
        static TSEvent tsE;
        return &tsE;
    };

    // 注册函数;
    void RegistEvent(string sEventKey, void* pInst, TpInstEventFun pFun);
    void RegistEventRoot(void* pInst, TpInstEventFunRoot pFun);

    // 反注册;
    void UnRegistEvent(string sEventKey, void* pInst); 
    void UnRegistEvent(string sEventKey);
    void UnRegistEventRoot();

    // 发送消息;
    void SendMsg(string sEventKey, string sBuffer );
    void SendRoot(char* pBuffer, int iLen);

    // Push网络包消息链;
    void PushMessge(string sEventKey, string sBuffer);
    void PushRoot(char* pBuffer, int iLen);

public:
    map<string, map<void*, TpInstEventFun> > m_MapEvent;

    std::list<std::pair<string, string> > m_MsgList;
    std::list<std::pair<char*, int> > m_MsgListRoot;

    void* m_RootEvent;
    TpInstEventFunRoot m_pFunRootEvent;
};

#endif
