#include "cocos2d.h"
#include <map>
#include <string>
using namespace std;

class InterfaceJNI
{
public:
    
	map<char*,int>& sendAudioData();
	void delMap(char* data);
	
protected:
private:
    map<char*,int> _map;
};