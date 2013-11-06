#include "TSScriptJS.h"
#include "ScriptingCore.h"
#include "TSEvent.h"
#include "jsapi.h"
#include <stdarg.h>
#include "TSEngine.h"
#include "../TSHallNum.h"

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
            TSEvent::GetSingleTon()->SendMsg("TS_JSOut", wrapper.get());
        }
    }
    return JS_TRUE;
}

void TSScriptJS::Init()
{
    ScriptingCore* sc = ScriptingCore::getInstance();
    JS_DefineFunction(sc->getGlobalContext(), sc->getGlobalObject(), "TS_JSOut", TSScriptJS::TS_JSOut, 0, JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineFunction(sc->getGlobalContext(), sc->getGlobalObject(), "TS_SendBuffer", TSScriptJS::TS_SendBuffer, 0, JSPROP_READONLY | JSPROP_PERMANENT);
}

bool TSScriptJS::RunFunction( std::string funName, std::string arg )
{
    std::string rStr = funName + "(\"" + arg + "\");";
    jsval v1;
    JSBool r = ScriptingCore::getInstance()->evalString(rStr.c_str(), &v1);
    return r;
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

JSBool TSScriptJS::TS_SendBuffer( JSContext *cx, uint32_t argc, jsval *vp )
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

void TSScriptJS::GetWebConfig() 
{
    ScriptingCore* sc = ScriptingCore::getInstance();
    sc->runScript("CMAI/Config.js");
    JSObject* pObj;    	
	jsval obj;
    JS_GetProperty(sc->getGlobalContext(),sc->getGlobalObject(),"IP", &obj); 
    JS_ValueToObject(sc->getGlobalContext(),obj,&pObj);
    jsval AdaptServerIP;  
    JS_GetProperty(sc->getGlobalContext(),pObj,"AdaptServerIP", &AdaptServerIP);  
    JSString* pS = JS_ValueToString(sc->getGlobalContext(), AdaptServerIP);  
    JSStringWrapper pW(pS);  
	TSHallNum::getSingleTon()->m_Adapter_Ip=pW.get().c_str();
    CCLog("================ip=%s",pW.get().c_str()); 
    jsval ip_port;  
    JS_GetProperty(sc->getGlobalContext(),pObj,"AdaptServerPort_WS", &ip_port);  
    JSString* pS_port = JS_ValueToString(sc->getGlobalContext(), ip_port);  
    JSStringWrapper pWs(pS_port); 
    TSHallNum::getSingleTon()->m_Adapter_Port=pWs.get().c_str();	
    CCLog("================ip_port=%s",pWs.get().c_str());
    
}
