#ifndef __TSTCP_H__
#define __TSTCP_H__

#include <string>
#include "TSObject.h"

#ifdef WIN32
#include <winsock2.h>
#include "pthread\pthread.h"
#else
#include <pthread.h>
#define SOCKET int
#endif

class TSTCP : public TSObject {
public:
    TSTCP();
    
    static TSTCP* GetSingleTon() {
        if (g_pTCPPtr == NULL) {
            g_pTCPPtr = new TSTCP();
        }
        return g_pTCPPtr;
    }

    SOCKET CreateClient(std::string sIP, int iPort);

    int GetConnectState() {
        if (m_hSocket == 0) {
            return 0;
        }
        return 1;
    }

    SOCKET getSocket() {
        return m_hSocket;
    }

    void CloseSocket()
    {   
#ifdef WIN32
        ::closesocket(m_hSocket);
#else
        ::close(m_hSocket);
#endif
        m_hSocket = 0;
    }

    void Lock();
    void UnLock();
    void ProcessMsg();

    int SendMessageToServer(std::string sBuffer);
    int SendMessageToServer( char* cBuffer, int iLen );

    void threadFunction(std::string& sBuffer);

public:
    static TSTCP* g_pTCPPtr;

public:
    SOCKET m_hSocket;
    pthread_t m_idThread;
    pthread_mutex_t m_mMutex;
    std::string m_sIP;
    int m_iPort;
};

#endif 

