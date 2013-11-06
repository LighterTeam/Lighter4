#ifndef __TSEVENT_H__
#define __TSEVENT_H__

#include <string>
#include <map>
#include <list>
#include "TSObject.h"
#include "json/json.h"

using namespace std;

// ȫ�ֻص����Ͷ���
typedef void (TSObject::*TpInstEventFun)(string sBuffer); 
typedef void (TSObject::*TpInstEventJsonFun)(Json::Value jvalue); 


// TS�¼�ϵͳ
class TSEvent : public TSObject {
public:
    TSEvent (){
    }

    static TSEvent* GetSingleTon() {
        static TSEvent tsE;
        return &tsE;
    };

    //---------------------String-------------------------------------------------------
    // ע�ắ��
    void RegistEvent(string sEventKey, void* pInst, TpInstEventFun pFun);

    // ��ע��
    void UnRegistEvent(string sEventKey, void* pInst); 
    void UnRegistEvent(string sEventKey);

    // ������Ϣ
    void SendMsg(string sEventKey, string sBuffer );

    // Push�������Ϣ��
    void PushMessge(string sEventKey, string sBuffer);

    //---------------------JSON----------------------------------------------------------
    // ע�ắ��
    void JSON_RegistEvent(string sEventKey, void* pInst, TpInstEventJsonFun pFun);

    // ��ע��
    void JSON_UnRegistEvent(string sEventKey, void* pInst); 
    void JSON_UnRegistEvent(string sEventKey);

    // ������Ϣ
    void JSON_SendMsg(string sEventKey, Json::Value jValue);

    // Push�������Ϣ��
    void JSON_PushMessge(string sEventKey, Json::Value jValue);

public:
    map<string, map<void*, TpInstEventFun> > m_MapEvent;
    std::list<std::pair<string, string> > m_MsgList;

    map<string, map<void*, TpInstEventJsonFun> > m_MapJsonEvent;
    std::list<std::pair<string, Json::Value> > m_MsgJsonList;
};

#endif
