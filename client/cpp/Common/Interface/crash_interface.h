#pragma once
#include "../TSCom/TSUnknown.hpp"

#define CRASH_DUMP_LIBRARY _T("CrashDump.dll")

enum DumpType
{
    enDumpLog,          // Log(�������ΪFALSE����һ�¼������MiniDump����Ч)
    enDumpBasicInfo,    // ������Ϣ
    enDumpModuleList,   // ģ���б�
    enDumpWindowList,   // �����б�
    enDumpProcessList,  // �����б�
    enDumpRegister,     // �Ĵ���
    enDumpCallStack,    // ����ջ
    enDumpMiniDump,     // ���dmp�ļ�
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
