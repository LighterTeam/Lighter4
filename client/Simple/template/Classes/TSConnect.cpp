#include "TSConnect.h"
#include "TSEngine/TSTCP.h"
#include "TSEngine/TSEvent.h"
#include "TSEngine/TSSocket.h"
#include "TSEngine/TSScriptJS.h"
#include "json/json.h"
#include "TSEngine/TSLog.h"

TSConnect::TSConnect() : m_iUUID(0) {

}

TSConnect::~TSConnect() {

}

// 窗口绑定
bool TSConnect::ReConnect()
{
    TSTCP::GetSingleTon()->CreateClient(m_sWG_IP, m_iWG_Port);
    int err = TSTCP::GetSingleTon()->GetConnectState();
    if (err >= 0)
    {
        Json::Value jValue;
        jValue["MM"] = "ReConnect";
        jValue["UUID"] = m_iUUID; 
        TSTCP::GetSingleTon()->SendMessageToServer(jValue.toStyledString());   
    }
    return true;
}

void TSConnect::initSocket()
{
    TSConnect* pC = TSConnect::GetSingleTon();
    TSScriptJS::GetSingleTon()->GetWebConfig(pC->m_Adapter_Ip, pC->m_Adapter_Port);
    if (TSTCP::GetSingleTon()->CreateClient(pC->m_Adapter_Ip, pC->m_Adapter_Port) != 0)
    {
        TSTCP::GetSingleTon()->SendMessageToServer("{\"MM\":\"ConnectGateWay\"}"); 
    } else {
        TSEvent::GetSingleTon()->SendMsg("Disconnect","Disconnect");
    }
}

void TSConnect::TSEventConnectGateWay( Json::Value jValue )
{
    //{
    //    "IP" : "127.0.0.1",
    //    "MM" : "ConnectGateWay",
    //    "Port" : 30000
    //}
    TSEvent* pE = TSEvent::GetSingleTon();
    TSTCP* pT = TSTCP::GetSingleTon();
    TSConnect* pC = TSConnect::GetSingleTon();
    TSLog("TSEventConnectGateWay: %s", jValue.toStyledString().c_str());
    TSTCP::GetSingleTon()->CloseSocket(); // 断开网关

    std::string& sIP = jValue["IP"].asString();
    int iPort = jValue["Port"].asInt();

    // 连接网关服
    if (pT->CreateClient(sIP, iPort) == 0) {
        TSLog("GateWayConnect Failed!");
    } else {
        TSLog("GateWayConnect Success!");
        TSConnect::GetSingleTon()->m_sWG_IP = sIP;
        TSConnect::GetSingleTon()->m_iWG_Port = iPort;
    }
}
