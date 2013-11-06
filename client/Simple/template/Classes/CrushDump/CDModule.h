#pragma once

#include <TSCom.hpp>
#include "XCrash.h"
#include <Windows.h>

HRESULT CALLBACK DllGetObject(TSGUID interfaceGUID, void** ppv);
HRESULT CALLBACK DllCanUnloadNow(void);
HRESULT CALLBACK DllDestroyModel(void* hModule);

class CComModule: public TSComModule<XCrashDump>
{
public:
};

extern CComModule g_TSModele;


