#include "TSTCP.h"
#include "TSLog.h"
#include "TSSocket.h"
#include "TSEvent.h"
#include "exbuffer.h"
#include <vector>
#include "..\CCCommon.h"
#include "TSEngine.h"
#include "json/json.h"
#include <sstream>

#ifdef WIN32
#include "pthread\pthread.h"
#else
#include <sys/socket.h>
#include <pthread.h>
#endif
#include "TSScriptJS.h"

char LB_BUFFER[1024*30] = {0};

TSTCP* TSTCP::g_pTCPPtr = NULL;

TSTCP::TSTCP()
    : m_hSocket(0)
{

}

char G_PacketBuffer[1024*16] = {0};
char G_TCPBuffer[1024*8] = {0};

static void recvHandle(unsigned char *rbuf, size_t len)
{
    TSTCP::GetSingleTon()->Lock();
    {
        memset(G_PacketBuffer,0,sizeof(G_PacketBuffer));
        memcpy(G_PacketBuffer, (char*)(rbuf), len);

        Json::Reader reader;
        Json::Value root;
        if (reader.parse(G_PacketBuffer, root))  
        {
            std::string sHeader = root["MM"].asString();
            TSEvent::GetSingleTon()->JSON_PushMessge(sHeader, root);
        }
        else
        {
            std::string& sHeader = TSEngine::GetHeader(G_PacketBuffer, len);
            TSEvent::GetSingleTon()->PushMessge(sHeader, G_PacketBuffer);
        }
    }
    TSTCP::GetSingleTon()->UnLock();
}


static void* GF_thread_function(void *arg) 
{
    TSTCP* pTCP = ((TSTCP*)arg);
    SOCKET tcpsocket = pTCP->m_hSocket;
    std::string IP = pTCP->m_sIP;
    int Port = pTCP->m_iPort;

    memset(G_TCPBuffer,0,sizeof(G_TCPBuffer));
    exbuffer_t* exB;
    exB = exbuffer_new();
    exB->recvHandle = recvHandle;

    for (;;) {
        memset(G_TCPBuffer, 0, sizeof(G_TCPBuffer));
        int bufLen = recv(tcpsocket, G_TCPBuffer, 1024*8, 0);
        if (bufLen == -1)
        {
            pTCP->m_hSocket = 0;
            break;
        }
        
        if (bufLen > 0) {
            exbuffer_put(exB,(unsigned char*)G_TCPBuffer,0,bufLen);
        }
    }

    std::stringstream disstr;
    disstr << "Disconnect IP:" << IP.c_str() << " Port:" << Port;
    TSEvent::GetSingleTon()->PushMessge("Disconnect", disstr.str());
    
    exbuffer_free(&exB);
    return NULL;
}

SOCKET TSTCP::CreateClient( std::string sIP, int iPort)
{
    TSLog("Connect Server IP:%s Port:%d", sIP.c_str(), iPort);
    TSSocket* tsS = TSSocket::GetSingleTon();
    m_hSocket = tsS->CreateClient(sIP, iPort);
    if (m_hSocket == 0)
    {
        return 0;
    }

    m_sIP = sIP;
    m_iPort = iPort;

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
        //TSScriptJS::GetSingleTon()->RunFunction("MessageProc", pE->m_MsgList.begin()->second);
        pE->m_MsgList.pop_front();
    }
    if (pE->m_MsgJsonList.size() > 0)
    {
        pE->JSON_SendMsg(pE->m_MsgJsonList.begin()->first, pE->m_MsgJsonList.begin()->second);
        TSScriptJS::GetSingleTon()->RunFunction("MessageProc", pE->m_MsgJsonList.begin()->second.toStyledString());
        pE->m_MsgJsonList.pop_front();
    }
    UnLock();
}

int TSTCP::SendMessageToServer( char* cBuffer, int iLen ) 
{
    TSLog("SendBuffer: BUF:%s Len:%d", cBuffer, iLen);
    char* sendBuf = new char[iLen + 4];
    unsigned short* BufLen = (unsigned short*)sendBuf;
    *BufLen = _ntohs(iLen + 2, EXBUFFER_BIG_ENDIAN);
    BufLen += 1;
    *BufLen = _ntohs(iLen, EXBUFFER_BIG_ENDIAN);
    memcpy(sendBuf + 4, cBuffer, iLen);
    int err = send(m_hSocket, sendBuf, iLen + 4, 0);
    if (err < 0)
    {
        TSEvent::GetSingleTon()->PushMessge("Disconnect","Disconnect");
    }

    delete [] sendBuf;
    return err;
}

int TSTCP::SendMessageToServer( std::string sBuffer )
{
    TSLog("SendBuffer: BUF:%s", sBuffer.c_str());
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
        TSEvent::GetSingleTon()->PushMessge("Disconnect","Disconnect");
    }

    delete [] sendBuf;
    return err;
}

