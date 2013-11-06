#include "TSEvent.h"
#include "TSLog.h"
#include "..\CCCommon.h"

void TSEvent::RegistEvent( string sEventKey, void* pInst, TpInstEventFun pFun )
{
    m_MapEvent[sEventKey][pInst] = pFun;
}

void TSEvent::UnRegistEvent( string sEventKey, void* pInst )
{
    m_MapEvent[sEventKey].erase(pInst);
}

void TSEvent::UnRegistEvent( string sEventKey )
{
    m_MapEvent.erase(sEventKey);
}

void TSEvent::SendMsg( string sEventKey, string sBuffer )
{
    if (m_MapEvent.count(sEventKey)) {
        map<void*, TpInstEventFun>& pInstMap = m_MapEvent[sEventKey];

        map<void*, TpInstEventFun>::iterator iter = pInstMap.begin();
        for (; iter != pInstMap.end(); iter++) {
            TSObject* pInst = (TSObject*)iter->first;
            TpInstEventFun pFun = iter->second;
            (pInst->*pFun)(sBuffer);
        }
    }
    else {
        TSLog("String NotFound is message regist:%s", sEventKey.c_str());
    }
}

void TSEvent::PushMessge( string sEventKey, string sBuffer )
{
    m_MsgList.push_back(std::make_pair(sEventKey, sBuffer));
}

void TSEvent::JSON_RegistEvent( string sEventKey, void* pInst, TpInstEventJsonFun pFun )
{
    m_MapJsonEvent[sEventKey][pInst] = pFun;
}

void TSEvent::JSON_UnRegistEvent( string sEventKey, void* pInst )
{
    m_MapJsonEvent[sEventKey].erase(pInst);
}

void TSEvent::JSON_UnRegistEvent( string sEventKey )
{
    m_MapJsonEvent.erase(sEventKey);
}

void TSEvent::JSON_SendMsg( string sEventKey, Json::Value jValue )
{
    if (m_MapJsonEvent.count(sEventKey)) {
        map<void*, TpInstEventJsonFun>& pInstMap = m_MapJsonEvent[sEventKey];

        map<void*, TpInstEventJsonFun>::iterator iter = pInstMap.begin();
        for (; iter != pInstMap.end(); iter++) {
            TSObject* pInst = (TSObject*)iter->first;
            TpInstEventJsonFun pFun = iter->second;
            (pInst->*pFun)(jValue);
        }
    }
    else {
        TSLog("Json NotFound is message regist:%s", sEventKey.c_str());
    }
}

void TSEvent::JSON_PushMessge( string sEventKey, Json::Value jValue )
{
    m_MsgJsonList.push_back(std::make_pair(sEventKey,jValue));
}

