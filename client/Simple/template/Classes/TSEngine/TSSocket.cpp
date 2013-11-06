#include "TSSocket.h"
#include "..\CCCommon.h"

#ifdef WIN32

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include "netdb.h"
#include "netinet/in.h"
#include <arpa/inet.h>
  
#endif
using namespace std;

SOCKET TSSocket::CreateClient(string sIp, unsigned short usPort) {

    #ifdef WIN32
    WORD wVersionRequested;
    WSAData wasdata;
    int err;
    wVersionRequested = MAKEWORD(1,1);

    err = WSAStartup( wVersionRequested, &wasdata );
    if(err!=0){
        return NULL;
    }

    if ( LOBYTE( wasdata.wVersion ) != 1 ||
        HIBYTE( wasdata.wVersion ) != 1 ) {
            WSACleanup( );
            return NULL; 
    }

    //创建套接字
    SOCKET hSocket = socket(AF_INET,SOCK_STREAM,0);
    SOCKADDR_IN addrSrv;

    //设置套接字的相关参数;
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(usPort);
    addrSrv.sin_addr.S_un.S_addr=inet_addr(sIp.c_str());

    //连接TCP
    int connectState = connect(hSocket,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	#else
	 //定义网络地址结构体
     struct sockaddr_in addrSrv;
     //数据初始化--清零
     memset(&addrSrv, 0, sizeof(addrSrv));
     //配置socket相关参数
     addrSrv.sin_family = AF_INET;
     addrSrv.sin_port = htons(usPort);
     addrSrv.sin_addr.s_addr = inet_addr(sIp.c_str());
     //创建一个socket
     hSocket = socket(AF_INET, SOCK_STREAM, 0);
    //socket创建失败
    if (hSocket < 0)
    {
      //CCLog("%s", "socket failed!!!");
    }
    else
    {
      //CCLog("%s", "socket success!!!");
    }
    //创建一个socket链接
    connectState = connect(hSocket, (struct sockaddr *) &addrSrv, sizeof(addrSrv));
	#endif
    if (connectState < 0)
    {   
        #ifdef WIN32
		closesocket(hSocket);
		#else
		close(hSocket);
		#endif
        return 0;
    }

    return hSocket;
}









