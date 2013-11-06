#include "InterfaceJNI.h"
#include "platform/android/jni/JniHelper.h"
#include <jni.h>
#include <android/log.h>
#include "String.h"
#include <map>
using namespace cocos2d;
using namespace std;

map<char*,int>& InterfaceJNI::sendAudioData(){

	JavaVM* jvm = JniHelper::getJavaVM();
	int status;
	JNIEnv *env;
	jbyteArray jbArr;
	jmethodID mid;
	jobject thiz;
	jbyte* bytes;
	
	bool isAttached = false;
	
	char* data;
	int size=0;
	CCLog("sendAudioData");
	// Get Status
	status = jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
	if(status < 0)
	{
		//LOGE("callback_handler: failed to get JNI environment, " // "assuming native thread");
		status = jvm->AttachCurrentThread(&env, NULL);
		CCLog("Status 2: %d", status);
		if(status < 0)
		{
			// LOGE("callback_handler: failed to attach " // "current thread");
			//return NULL;
		}
		isAttached = true;
		CCLog("Status isAttached: %d", isAttached);
	}
	CCLog("Status: %d", status);
	jclass mClass = env->FindClass("cn/com/CatchMike2");
	CCLog("jClass Located?");
    mid = env->GetStaticMethodID(mClass, "getAudioString", "()[B");
	CCLog("mID: %d", mid);
	if (mid!=0){
	   thiz=env->CallStaticObjectMethod(mClass,mid);
	   jbArr=(jbyteArray)thiz;
	   bytes=env->GetByteArrayElements(jbArr,0);
	   data=(char*)bytes;
	   size=(int)env->GetArrayLength(jbArr);
	   CCLog("Finish=%d",size);
	   _map[data]=size;
	}
	env->ReleaseByteArrayElements(jbArr,bytes,0);
	//env->DeleteLocalRef(thiz);
	CCLog("Finish");
	if(isAttached)
		jvm->DetachCurrentThread();

	return _map;
}

void InterfaceJNI::delMap(char* data){
	_map.erase(data);
}