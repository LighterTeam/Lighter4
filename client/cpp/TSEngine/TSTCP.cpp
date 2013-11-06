#include "TSTCP.h"
#include "TSSocket.h"
#include "TSEvent.h"
#include "exbuffer.h"
#include <vector>
#include "../CCCommon.h"
#include "TSEngine.h"
#include "TSHallNum.h"

#ifdef WIN32
#include "pthread\pthread.h"
#else
#include <sys/socket.h>
#include <pthread.h>
#endif
char LB_BUFFER[1024*30] = {0};

TSTCP* TSTCP::g_pTCPPtr = NULL;

TSTCP::TSTCP()
    : m_hSocket(0)
{
   // TSEvent::GetSingleTon()->RegistEvent("ReConnect",(void*)this,(TpInstEventFun)&TSTCP::TSEventReConnect);
}

static void recvHandle(unsigned char *rbuf, size_t len)
{
    char buffer[1024*2] = {0};
    memcpy(buffer, (char*)(rbuf), len);    

    TSTCP::GetSingleTon()->Lock();
    {
        static int s_iLBLen = 0;
        string sHeader = TSEngine::GetHeader((char*)buffer, len);
        if ( sHeader == string("LB") )
        {
			//len = 103;
            char* begin = (char*)(rbuf + 3);
            memcpy(LB_BUFFER + s_iLBLen, begin, len-3);
            s_iLBLen += (len-3);
			
			CCLOG("LB len= %d", len);
			CCLOG("LB = %d", s_iLBLen);
        }
        else if ( sHeader == string("LB_Begin") )
        {
			CCLOG("LB_Begin+");
            s_iLBLen = 0;
        }
        else if ( sHeader == string("LB_End") )
        {
            if (s_iLBLen > 0)
            {
                //buffer取完了;
				CCLOG("LB_End+ %d",s_iLBLen);
                TSEvent::GetSingleTon()->PushRoot(LB_BUFFER, s_iLBLen);
                memset(LB_BUFFER,0,sizeof(LB_BUFFER));
            }
        }
        else
        {
            cocos2d::CCLog(string(buffer).c_str());
            TSEvent::GetSingleTon()->PushMessge(sHeader, buffer);
        }
    }
    TSTCP::GetSingleTon()->UnLock();
}

static void* GF_thread_function(void *arg) 
{
    TSTCP* pTcp = (TSTCP*)arg;
    char cBuffer[1024] = {0};

    exbuffer_t* exB;
    exB = exbuffer_new();
    exB->recvHandle = recvHandle;

    for (;;) {
        memset(cBuffer, 0, sizeof(cBuffer));
        int bufLen = recv(pTcp->m_hSocket, cBuffer, 1024, 0);
        if (bufLen == -1)
        {
            return NULL;
        }
        
        if (bufLen > 0) {
            exbuffer_put(exB,(unsigned char*)cBuffer,0,bufLen);
        }
    }

    exbuffer_dump(exB,exB->bufferlen);
    exbuffer_free(&exB);
}

SOCKET TSTCP::CreateClient( std::string sIP, int iPort)
{
    TSSocket* tsS = TSSocket::getSingleTon();
    m_hSocket = tsS->CreateClient(sIP, iPort);

    pthread_attr_t tAttr;
    int errCode = pthread_attr_init(&tAttr);
    errCode = pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED);
    if (errCode != 0) {
        pthread_attr_destroy(&tAttr);
    }
    pthread_mutex_init(&m_mMutex,NULL);
    pthread_create(&m_idThread, &tAttr, GF_thread_function, this);
    return m_hSocket;
}

void TSTCP::threadFunction( std::string& sBuffer )
{

}

void TSTCP::Lock()
{
    if (!m_hSocket)
    {
        return;
    }
    pthread_mutex_lock(&m_mMutex);
}

void TSTCP::UnLock()
{
    if (!m_hSocket)
    {
        return;
    }
    pthread_mutex_unlock(&m_mMutex);
}

void TSTCP::ProcessMsg()
{
    if (!m_hSocket)
    {
        return;
    }

    Lock();
    TSEvent* pE = TSEvent::GetSingleTon();
    if (pE->m_MsgList.size() > 0)
    {
        pE->SendMsg(pE->m_MsgList.begin()->first, pE->m_MsgList.begin()->second);
        pE->m_MsgList.pop_front();
    }

    if (pE->m_MsgListRoot.size() > 0)
    {
		CCLOG("SendMM");
        char* pSrc = pE->m_MsgListRoot.begin()->first;
        pE->SendRoot(pSrc, pE->m_MsgListRoot.begin()->second);
        delete pSrc;
        pE->m_MsgListRoot.pop_front();
    }
    UnLock();
}

int TSTCP::SendMessageToServer( char* cBuffer, int iLen ) 
{
    char* sendBuf = new char[iLen + 4];
    unsigned short* BufLen = (unsigned short*)sendBuf;
    *BufLen = _ntohs(iLen + 2, EXBUFFER_BIG_ENDIAN);
    BufLen += 1;
    *BufLen = _ntohs(iLen, EXBUFFER_BIG_ENDIAN);
    memcpy(sendBuf + 4, cBuffer, iLen);
    int err = send(m_hSocket, sendBuf, iLen + 4, 0);

    delete [] sendBuf;
    return err;
}

int TSTCP::SendMessageToServer( std::string sBuffer )
{
    int len = sBuffer.length();
    char* sendBuf = new char[len + 4];
    unsigned short* BufLen = (unsigned short*)sendBuf;
    *BufLen = _ntohs(len+2, EXBUFFER_BIG_ENDIAN);
    BufLen += 1;
    *BufLen = _ntohs(len, EXBUFFER_BIG_ENDIAN);
    memcpy(sendBuf+4, sBuffer.c_str(), len);
    int err = send(m_hSocket, sendBuf, len+4, 0);

    if (err < 0)
    {
        //弹出窗口.
        TSHallNum::getSingleTon()->OpenReConnectWnd();
    }


    delete [] sendBuf;
    return err;
}

//void TSTCP::TSEventReConnect(std::string& sBuffer){
//
//    std::vector<std::string> oPacket;
//    TSEngine::TST_StringFilt(sBuffer,',',oPacket);
//    std::vector<std::string>::iterator iter=oPacket.begin()+1;
//    for(;iter!=oPacket.end();iter++){
//        
//        std::string& str=*iter;
//        CCLOG("TSEventReConnect",str.c_str());
//    }
//    rec=true;
//}
