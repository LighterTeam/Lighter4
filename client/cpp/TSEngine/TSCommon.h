#ifndef __TSCOMMON_H__
#define __TSCOMMON_H__

#include "TSObject.h"
#include "TSSocket.h"
#include "TSTCP.h"
#include "TSEvent.h"
#include "TSEngine.h"

//"192.168.1.24"   JL
//"192.168.1.224"  TS
//"192.168.1.34"   YL
//"127.0.0.1"      Local

const int AdaptServerPort = 9900; // Ψһ
const int AdaptServerPort_WS = 9901;
const char* AdaptServerIP = "192.168.1.224";

// ��Ϣ��ؼ��� (���ĳ����ڽ���Ϣ��, ǰ�˳���,��̨����,����ʹ�õ���Ϣ��):
// BL_Begine (WP)
// BL_End (WP)
// BL (WP)


#endif