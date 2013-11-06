#pragma once

#include <map>
#include <string>
#include <tchar.h>
#include <windows.h>
#include "TSUnknown.hpp"

#ifndef WIN32
#define CALLBACK __stdcall
#endif

using namespace std;

template<class T>
class TSComModule
{
public:
    TSComModule() : count(0)
    {
        
    }

    HRESULT CreateObject(TSGUID interfaceGUID, void** ppv)
    {
        AddRef();

        if (InterfacePointMap.count(interfaceGUID))
        {
            *ppv = InterfacePointMap[interfaceGUID];
        }
        else
        {
            *ppv = new T();
            InterfacePointMap[interfaceGUID] = *ppv;
        }

        return S_OK;
    }

    unsigned long getRefCount()
    {
        return count;
    }

    HRESULT DllDestroy(HMODULE hModule)
    {
        for (unsigned long i = 0 ; i < count ; i++)
        {
            if (!FreeLibrary(hModule))
            {
                return E_FAIL;
            }
        }

        map<string,void*>::iterator iter = InterfacePointMap.begin();
        for (;iter != InterfacePointMap.end();iter++)
        {
            delete iter->second;
            iter->second = 0;
        }

        return S_OK;
    }

private:
    void AddRef()
    {
        count++;
    }

private:
    unsigned long count; //调用次数
    map<string,void*> InterfacePointMap;
};
