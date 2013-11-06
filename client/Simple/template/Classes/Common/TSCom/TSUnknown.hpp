#pragma once

#include <Windows.h>
#include "TSError.hpp"
#include "TSGUID.hpp"

//TSUnKnown Interface
#define interface struct
interface TSUnKnown
{

};

/************************************************************************/
// ------------- CREATE OBJECT -----------------
/************************************************************************/
typedef HRESULT (CALLBACK GET_TSCOM_OBJECT_FUNC)(TSGUID , void**);
typedef HRESULT (CALLBACK DESTROY_TSCOM_OBJECT_FUNC)(void*);

inline HRESULT CoTsCreateInstance(const TCHAR* pszLibName, 
    TSGUID interfaceGUID, void** ppv)
{
    if ( !pszLibName || !ppv )
    {
        return E_FAIL;
    }

    HMODULE hModule = LoadLibrary(pszLibName);
    if ( !hModule )
    {
        return E_FAIL;
    }

    GET_TSCOM_OBJECT_FUNC* pfn = (GET_TSCOM_OBJECT_FUNC*)GetProcAddress(hModule, "DllGetObject");

    if ( !pfn )
    {
        return E_FAIL;
    }

    HRESULT hr = pfn(interfaceGUID, ppv);
    if ( FAILED(hr) && nullptr != *ppv )
    {
        FreeLibrary(hModule);
        return E_FAIL;
    }
    return S_OK;
}

inline HRESULT CoTsDestroyInstance(const TCHAR* pszLibName)
{
    HMODULE hModule = LoadLibrary(pszLibName);
    if ( !hModule )
    {
        return E_FAIL;
    }

    DESTROY_TSCOM_OBJECT_FUNC* pfn = (DESTROY_TSCOM_OBJECT_FUNC*)GetProcAddress(hModule, "DllDestroyModel");
    if ( !pfn )
    {
        return E_FAIL;
    }

    HRESULT hr = pfn((void*)hModule);
    if ( SUCCEEDED(hr) )
    {
        FreeLibrary(hModule);
        return E_FAIL;
    }
}

