#include "TSLog.h"
#include "TSScriptJS.h"
#include "ScriptingCore.h"
#include "TSEvent.h"
#include "jsapi.h"
#include <stdarg.h>
#include "TSEngine.h"
#include "TSLog.h"
#include "TSTCP.h"

TSScriptJS::TSScriptJS()
{
}

TSScriptJS::~TSScriptJS()
{

}

JSBool TSScriptJS::TS_JSOut( JSContext *cx, uint32_t argc, jsval *vp )
{
    if (argc > 0) {
        JSString *string = NULL;
        JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &string);
        if (string) {
            JSStringWrapper wrapper(string);
            TSLog(wrapper.get().c_str());
        }
    }
    return JS_TRUE;
}

void TSScriptJS::Init()
{
    ScriptingCore* sc = ScriptingCore::getInstance();
    auto GC = sc->getGlobalContext();
    auto GO = sc->getGlobalObject();
    JS_DefineFunction(GC, GO, "TSLog", TSScriptJS::TS_JSOut, 0, JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineFunction(GC, GO, "TSSendBuffer", TSScriptJS::TS_SendBuffer, 0, JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineFunction(GC, GO, "TSLocalMsg", TSScriptJS::TS_LocalMsg, 0, JSPROP_READONLY | JSPROP_PERMANENT);
}

bool TSScriptJS::RunFunction( std::string funName, std::string& arg )
{
    JSCallFuncName(funName.c_str(), 1, arg.c_str());
    return true;
}


jsval TSScriptJS::JSCallFuncName(const char *name, int count, ...)
{
    ScriptingCore* sc = ScriptingCore::getInstance();
    int argc = count;
    jsval res;
    jsval *argv = new jsval[count];
    va_list insvalist;
    va_start(insvalist,count);
    for (int i = 0; i < count; ++i)
    {    
	    const char* _str =va_arg(insvalist,const char *);
		std::string sttr=std::string("")+_str;
        argv[i] = std_string_to_jsval(sc->getGlobalContext(),sttr);
    }
    va_end(insvalist) ;
    JS_CallFunctionName(sc->getGlobalContext(), sc->getGlobalObject(), name, argc, argv, &res);
    delete [] argv;
    return res ;
}

JSBool TSScriptJS::TS_LocalMsg( JSContext *cx, uint32_t argc, jsval *vp )
{
    if (argc > 0) {
        JSString *string = NULL;
        JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &string);
        if (string) {
            JSStringWrapper wrapper(string);
            std::string header = TSEngine::GetSingleTon()->GetHeader((char*)wrapper.get().c_str(),wrapper.get().length());
            TSEvent::GetSingleTon()->SendMsg(header, wrapper.get());
        }
    }
    return JS_TRUE;
}

JSBool TSScriptJS::TS_SendBuffer( JSContext *cx, uint32_t argc, jsval *vp )
{
    if (argc > 0) {
        JSString *string = NULL;
        JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &string);
        if (string) {
            JSStringWrapper wrapper(string);
            TSTCP::GetSingleTon()->SendMessageToServer(wrapper.get());
        }
    }
    return JS_TRUE;
}

void TSScriptJS::GetWebConfig(std::string& Adapter_Ip, int& Adapter_Port) 
{
    ScriptingCore* sc = ScriptingCore::getInstance();
    sc->runScript("Config/Config.js");
    JSObject* pObj;    	
	jsval obj;
    JS_GetProperty(sc->getGlobalContext(),sc->getGlobalObject(),"IP", &obj); 
    JS_ValueToObject(sc->getGlobalContext(),obj,&pObj);
    jsval AdaptServerIP;  
    JS_GetProperty(sc->getGlobalContext(),pObj,"AdaptServerIP", &AdaptServerIP);  
    JSString* pS = JS_ValueToString(sc->getGlobalContext(), AdaptServerIP);  
    JSStringWrapper pW(pS);  
	Adapter_Ip = pW.get().c_str();
    TSLog("AdapteServer Ip = %s", pW.get().c_str()); 
    jsval ip_port;  
    JS_GetProperty(sc->getGlobalContext(),pObj,"AdaptServerPort", &ip_port);  
    JSString* pS_port = JS_ValueToString(sc->getGlobalContext(), ip_port);  
    JSStringWrapper pWs(pS_port); 
    Adapter_Port = atoi(pWs.get().c_str());	
    TSLog("AdapteServer Port = %s",pWs.get().c_str());
}
