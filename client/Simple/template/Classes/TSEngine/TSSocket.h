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
    TSSocket(){
    }

    static TSSocket* GetSingleTon() {
        static TSSocket tsS;
        return &tsS;
    } 

    SOCKET CreateClient(std::string sIp, unsigned short usPort);
};

#endif
