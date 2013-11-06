#include "cocos2d.h"
#include <map>
#include <string>
using namespace std;

class InterfaceJNI
{
public:
        //录制音频;
	map<char*,int>& sendAudioData();
	void delMap(char* data);
        //获取手机设备id；
	static char* get_phoneIMIE();
	//返回当前用户位置信息;
	static char* get_Location();
protected:
private:
    map<char*,int> _map;
};
