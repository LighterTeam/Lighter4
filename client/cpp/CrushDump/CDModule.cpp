// ComSystem.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "TSCom.hpp"
#include "TSUnknown.hpp"

#include <ITSComSystem.h>
#include "CDModule.h"

CComModule g_TSModele;

HRESULT CALLBACK DllGetObject(TSGUID interfaceGUID, void** ppv)
{
    return g_TSModele.CreateObject(interfaceGUID,ppv);
}

HRESULT CALLBACK DllCanUnloadNow(void)
{
    return S_OK;
}

HRESULT CALLBACK DllDestroyModel(void* HModule)
{
    return g_TSModele.DllDestroy((HMODULE)HModule);
}
