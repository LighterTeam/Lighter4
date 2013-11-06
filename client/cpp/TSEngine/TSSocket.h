#ifndef __TSSOCKET_H__
#define __TSSOCKET_H__

#include <string>
#include "TSObject.h"

#ifdef WIN32
#include <WinSock.h>
#pragma comment(lib, "WS2_32.lib")
#else
#define SOCKET int
#endif

class TSSocket : public TSObject {
public:
    TSSocket() : m_hSocket(0), m_ConnectState(0) {

    }

    static TSSocket* getSingleTon() {
        static TSSocket tsS;
        return &tsS;
    } 

    SOCKET CreateClient(std::string sIp, unsigned short usPort);

    SOCKET getSocket() {
        return m_hSocket;
    }

    int getConnectState() {
        return m_ConnectState;
    }

    void CloseSocket();

private:
    SOCKET m_hSocket;
    int m_ConnectState;
};

#endif
