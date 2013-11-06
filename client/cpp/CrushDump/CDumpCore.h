#ifndef _CRASH_DUMP_H_
#define _CRASH_DUMP_H_

#include <dbghelp.h>
#include <string>
#include <list>
#include "crash_interface.h"

#ifdef _WIN64
#define IP Rip
#define DI Rdi
#define AX Rax
#define BX Rbx
#define CX Rcx
#define DX Rdx
#define BP Rbp
#define SI Rsi
#define SP Rsp
#define  OFFSET_TYPE DWORD64
#else
#define IP Eip
#define DI Edi
#define AX Eax
#define BX Ebx
#define CX Ecx
#define DX Edx
#define BP Ebp
#define SI Esi
#define SP Esp
#define  OFFSET_TYPE DWORD32
#endif

using namespace std;

enum BasicType
{
    btNoType = 0,
    btVoid = 1,
    btChar = 2,
    btWChar = 3,
    btInt = 6,
    btUInt = 7,
    btFloat = 8,
    btBCD = 9,
    btBool = 10,
    btLong = 13,
    btULong = 14,
    btCurrency = 25,
    btDate = 26,
    btVariant = 27,
    btComplex = 28,
    btBit = 29,
    btBSTR = 30,
    btHresult = 31
};

#ifdef UNICODE
typedef wstring OString;
#else
typedef string OString;
#endif

class CDumpCore
{
public:
    bool InstallExceptionFilter();
    //void SetLogFileName(LPCTSTR szLogFileName);
    //void SetDumpFileName(LPCTSTR szDumpFileName);
    void SetOriginalFileName(LPCTSTR szOriginalFileName);

    void StartDumpDaemon();

    void SetDumpType(long lDumpType);
    void SetDumpSwitch(int iEnum,BOOL bOn);

    DWORD GetExceptionCode(PEXCEPTION_POINTERS pExceptionInfo);
    LPCTSTR GetExceptionDesc(PEXCEPTION_POINTERS pExceptionInfo);
    OFFSET_TYPE GetCrashFileOffset(PEXCEPTION_POINTERS pExceptionInfo);
    LPCTSTR GetCrashModuleName(PEXCEPTION_POINTERS pExceptionInfo);
    void DumpLn(LPCTSTR szMsg);
    void GetCrashModuleProductVer(PEXCEPTION_POINTERS pExceptionInfo,WORD pwVer[4]);
    OString GetCallStack(CONTEXT& Context,HANDLE hThread);

    static DWORD WINAPI HandlingMyExceptProc(LPVOID lpParam);

protected:
    CDumpCore();

public:
    static CDumpCore *Instance()
    {
        static CDumpCore cd;
        return &cd;
    }
    ~CDumpCore();

    void DumpAuxi(PEXCEPTION_POINTERS pExceptionInfo);

    void DumpDirectForTest(PEXCEPTION_POINTERS pExceptionInfo);

protected:
    void DumpModuleList(PEXCEPTION_POINTERS pExceptionInfo);
    void DumpWindowList(PEXCEPTION_POINTERS pExceptionInfo);
    void DumpProcessList(PEXCEPTION_POINTERS pExceptionInfo);
    void DumpRegister(PEXCEPTION_POINTERS pExceptionInfo);
    void DumpCallStack(PCONTEXT,HANDLE);
    void DumpBasicInfo(PEXCEPTION_POINTERS pExceptionInfo);
    void DumpMiniDump(PEXCEPTION_POINTERS pExceptionInfo);

private:
    void DumpMiniDump(PEXCEPTION_POINTERS pExceptionInfo,HANDLE hFile);
    LPCTSTR GetExceptionDescription(DWORD ExceptionCode);
    LPTSTR GetFilePart(LPCTSTR);
    LPTSTR lstrrchr(LPCTSTR,int ch);
    void DumpModuleInfo(HINSTANCE ModuleHandle);
    void FormatTime(LPTSTR output, FILETIME TimeToPrint);
    void WriteStackDetails(PCONTEXT pContext,HANDLE);
    static BOOL CALLBACK EnumerateSymbolsCallback(PSYMBOL_INFO  pSymInfo,ULONG SymbolSize,PVOID UserContext);
    BOOL GetLogicalAddress(PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset );
    bool FormatSymbolValue(PSYMBOL_INFO pSym,STACKFRAME64 * sf, TCHAR * pszBuffer,unsigned int szWords);
    TCHAR * CDumpCore::DumpTypeIndex(TCHAR * pszCurrBuffer, unsigned int & szWords,DWORD64 modBase,DWORD dwTypeIndex,unsigned nestingLevel,DWORD64 offset,bool & bHandled);
    BasicType GetBasicType(DWORD typeIndex, DWORD64 modBase );

    TCHAR * CDumpCore::FormatOutputValue(TCHAR * pszCurrBuffer, unsigned int& szWords, 
        BasicType basicType,DWORD64 length,PVOID pAddress);

    void OnDumpStart();
    LONG OnDumpFinish(PEXCEPTION_POINTERS pExceptionInfo);

    static BOOL SuspendAllButThisThread( bool bsuspend = true );

    static FILE *m_fpLog;
    HANDLE m_handleLogFile;
private:
    BOOL    m_dumpSwitch[enDumpCount];
    long    m_lCrashDumpType;
private:
    static LONG WINAPI UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);
    BOOL IsWindows98(); 
    void createReportProcess();
private:
    //TCHAR m_szLogFileName[MAX_PATH];
    //TCHAR m_szDumpFileName[MAX_PATH];
    TCHAR m_szOriginalFileName[MAX_PATH];

    HANDLE m_thandle;

private:
    void Dump(LPCTSTR szMsg,...);
public:
    LPTOP_LEVEL_EXCEPTION_FILTER m_previousFilter;
};

#endif
