#include "TSEvent.h"
#include "../CCCommon.h"

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
        cocos2d::CCLog("Message Not Find:%s", sEventKey.c_str());
    }
}

void TSEvent::PushMessge( string sEventKey, string sBuffer )
{
    m_MsgList.push_back(std::make_pair(sEventKey, sBuffer));
}

void TSEvent::RegistEventRoot( void* pInst, TpInstEventFunRoot pFun )
{
    m_RootEvent = pInst;
    m_pFunRootEvent = pFun;
}

void TSEvent::UnRegistEventRoot()
{
    m_RootEvent = 0;
    m_pFunRootEvent = 0;
}

void TSEvent::SendRoot(char* pBuffer, int iLen)
{
    if (m_RootEvent == 0 || m_pFunRootEvent == 0)
    {
        return;
    }
    ((TSObject*)m_RootEvent->*m_pFunRootEvent)(pBuffer, iLen);
}

void TSEvent::PushRoot( char* pBuffer, int iLen )
{
    char* cBuffer = new char[iLen];
    memcpy(cBuffer,pBuffer,iLen);
    m_MsgListRoot.push_back(std::make_pair(cBuffer, iLen));
}
