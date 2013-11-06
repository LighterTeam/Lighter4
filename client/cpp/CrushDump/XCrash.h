#pragma once
#include <TSCom.hpp>
#include <crash_interface.h>

class XCrashDump : public ICrashDump
{
public:
    virtual HRESULT STDMETHODCALLTYPE Initialize(const TCHAR* lpszDumpFileName_no_suffix);
    virtual HRESULT STDMETHODCALLTYPE DumpMiniDump(PEXCEPTION_POINTERS);
    virtual HRESULT STDMETHODCALLTYPE EnsureHookIsWorking();
    virtual HRESULT STDMETHODCALLTYPE SetDumpType(DumpType type, bool enable);

    XCrashDump() { }
    ~XCrashDump() { }
};
