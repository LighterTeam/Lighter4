#ifndef __TSTCP_H__
#define __TSTCP_H__

#include <string>
#include "TSEngine/TSObject.h"

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
        static TSTCP g_pTCPPtr;
        return &g_pTCPPtr;
    }

    SOCKET CreateClient(std::string sIP, int iPort);

    SOCKET getSocket() {
        return m_hSocket;
    }

    void Lock();
    void UnLock();
    void ProcessMsg();

    int SendMessageToServer(std::string sBuffer);
    int SendMessageToServer( char* cBuffer, int iLen );

    void threadFunction(std::string& sBuffer);
   // void TSEventReConnect(std::string& sBuffer);

public:
    static TSTCP* g_pTCPPtr;

public:
    SOCKET m_hSocket;
    pthread_t m_idThread;
    pthread_mutex_t m_mMutex;
  //  bool rec;
};

#endif

