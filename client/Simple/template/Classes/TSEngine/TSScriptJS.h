#ifndef __TSSCRIPTJS_H__
#define __TSSCRIPTJS_H__

#include <string>
#include "TSObject.h"
#include "jsapi.h"

class TSScriptJS : public TSObject {
public:
    TSScriptJS();
    ~TSScriptJS();

    static TSScriptJS* GetSingleTon() {
        static TSScriptJS js;
        return &js;
    }

    //Debug
    void enableTracing(JSContext *cx);
    void disableTracing(JSContext *cx);

    void Init();

public:
    static JSBool TS_JSOut(JSContext *cx, uint32_t argc, jsval *vp);
    static JSBool TS_SendBuffer(JSContext *cx, uint32_t argc, jsval *vp);
    static JSBool TS_LocalMsg( JSContext *cx, uint32_t argc, jsval *vp );

public:
    bool RunFunction(std::string funName, std::string& arg);
    jsval JSCallFuncName(const char *name, int count, ...);
    void GetWebConfig(std::string& Adapter_Ip, int& Adapter_Port);
};


#endif