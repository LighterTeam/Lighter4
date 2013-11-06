#ifndef  __TSConnect_H__
#define  __TSConnect_H__

#include "cocos2d.h"
#include <string>
#include "TSEngine/TSObject.h"
#include "json/value.h"

using namespace std;
using namespace cocos2d;

class TSConnect : TSObject
{
public:
    TSConnect();
    ~TSConnect();

    static TSConnect* GetSingleTon(){
        static TSConnect tsh;
        return &tsh;
    }

    bool ReConnect();
    void initSocket();

    // ������������������;
    void TSEventConnectGateWay( Json::Value jValue);

public:
    //�������IP�Ͷ˿ں�
    std::string m_Adapter_Ip;
    int m_Adapter_Port;

    //������Ҫ������
    std::string m_sWG_IP;   //IP
    int m_iWG_Port;         //�˿ں�

    //UUID
    int m_iUUID;    //���ط�����ͻ��˵�ΨһID
protected:

private:
};

#endif
