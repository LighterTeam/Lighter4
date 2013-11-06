#pragma once
#include "../TSCom/TSUnknown.hpp"

#define CRASH_DUMP_LIBRARY _T("CrashDump.dll")

enum DumpType
{
    enDumpLog,          // Log(如果此项为FALSE，则一下几项除了MiniDump都无效)
    enDumpBasicInfo,    // 基本信息
    enDumpModuleList,   // 模块列表
    enDumpWindowList,   // 窗口列表
    enDumpProcessList,  // 进程列表
    enDumpRegister,     // 寄存器
    enDumpCallStack,    // 调用栈
    enDumpMiniDump,     // 输出dmp文件
    enDumpCount
};

#define ICrashDumpGUID "48EDF4F7-CCDE-49cc-8AF8-54A9209AD8B7"

interface ICrashDump : public TSUnKnown
{
    virtual HRESULT STDMETHODCALLTYPE Initialize(const TCHAR* lpszDumpFileName_no_suffix) = 0;

    // For Test
    virtual HRESULT STDMETHODCALLTYPE DumpMiniDump(PEXCEPTION_POINTERS) = 0;

    // Call this method to reinstall hook in system.
    virtual HRESULT STDMETHODCALLTYPE EnsureHookIsWorking() = 0;

    virtual HRESULT STDMETHODCALLTYPE SetDumpType(DumpType type, bool enable) = 0;
};
