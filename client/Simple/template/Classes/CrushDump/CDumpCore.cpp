#include "stdafx.h"
#include <tlhelp32.h>
#include "CDumpCore.h"
#include "GetWinVer.h"
#include "MiniVersion.h"
#include <time.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "dbghelp.lib")

const int NumCodeBytes = 16;    // Number of code bytes to record - IP所指向的

FILE *CDumpCore::m_fpLog = 0;

TCHAR g_szForDumpMsg[1024*8] = {0};

struct DumpContext
{
    HANDLE exceptionThread;
    PEXCEPTION_POINTERS pExceptionInfo ;
    DWORD exceptionThreadID;
};

DumpContext g_dumpContext;

#ifndef _WIN64
void DisableSetUnhandledExceptionFilter()
{
    void *addr = (void*)GetProcAddress(LoadLibrary(_T("kernel32.dll")),
        "SetUnhandledExceptionFilter");
    if (addr)
    {
        unsigned char code[16];
        int size = 0;
        code[size++] = 0x33;
        code[size++] = 0xC0;
        code[size++] = 0xC2;
        code[size++] = 0x04;
        code[size++] = 0x00;

        DWORD dwOldFlag, dwTempFlag;
        VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &dwOldFlag);
        WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
        VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
    }
}
#endif

DWORD WINAPI CDumpCore::HandlingMyExceptProc(LPVOID lpParam)
{
    cout << "11" << endl;
    __try
    {
        DumpContext* pContext = ( DumpContext* )(lpParam);

#ifdef _WIN64
        SuspendAllButThisThread(true);
#endif

        OutputDebugString(_T("Dump: Begin"));
        // MINI DUMP
        if(CDumpCore::Instance()->m_dumpSwitch[enDumpMiniDump])
            CDumpCore::Instance()->DumpMiniDump(pContext->pExceptionInfo);

        time_t ttime;
        ttime = time(nullptr);
        struct tm _tm;
        (void)localtime_s(&_tm, &ttime);

#ifdef _WIN64
        _stprintf_s(g_szForDumpMsg, 256,  _T("%s(%d-%d-%d`%d-%d-%d).errorlog.txt"), CDumpCore::Instance()->m_szOriginalFileName, 
            _tm.tm_mon + 1,
            _tm.tm_mday,
            _tm.tm_year + 1900,
            _tm.tm_hour,
            _tm.tm_min,
            _tm.tm_sec);
#else
        _stprintf_s(g_szForDumpMsg, 256,  _T("%s(%d-%d-%d`%d-%d-%d).errorlog.txt"), CDumpCore::Instance()->m_szOriginalFileName, 
            _tm.tm_mon + 1,
            _tm.tm_mday,
            _tm.tm_year + 1900,
            _tm.tm_hour,
            _tm.tm_min,
            _tm.tm_sec);
#endif

        cout << "12" << endl;

        OutputDebugString(_T("Dump: CreateFile"));
        CDumpCore::Instance()->m_handleLogFile = CreateFile(
            g_szForDumpMsg,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
            NULL);

        if (CDumpCore::Instance()->m_handleLogFile == INVALID_HANDLE_VALUE)
        {
            return EXCEPTION_EXECUTE_HANDLER;
        }

        cout << "13" << endl;
        CDumpCore::Instance()->Dump(_T("-------------------Dump begin----------------------"));
        if ( !pContext->pExceptionInfo )
        {
            CDumpCore::Instance()->Dump(_T("------------ERROR: NULL Exception record-----------\n"));
            CloseHandle(CDumpCore::Instance()->m_handleLogFile);
            CDumpCore::Instance()->m_handleLogFile = INVALID_HANDLE_VALUE;
            return 1L;
        }

        if ( pContext->pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_STACK_OVERFLOW )
        {
            CDumpCore::Instance()->Dump(_T("------------ERROR: stack overflow-----------\n"));
        }

        //先弹出对话框给用户，让其选择汇报问题出现情况。
        CDumpCore::Instance()->createReportProcess();

       // text file
        {
            OutputDebugString(_T("Dump: DumpCallStack"));
            if(CDumpCore::Instance()->m_dumpSwitch[enDumpCallStack])
            {
                CONTEXT context = *(pContext->pExceptionInfo->ContextRecord);
                CDumpCore::Instance()->DumpCallStack(&context, pContext->exceptionThread);
            }

            if(CDumpCore::Instance()->m_dumpSwitch[enDumpLog])
                CDumpCore::Instance()->DumpAuxi(pContext->pExceptionInfo);
        }
    }
    __finally
    {
        //FlushFileBuffers(CDumpCore::Instance()->m_handleLogFile);
        CloseHandle(CDumpCore::Instance()->m_handleLogFile);
        CDumpCore::Instance()->m_handleLogFile = INVALID_HANDLE_VALUE;

        //调用
        //CDumpCore::Instance()->OnDumpStart();

        OutputDebugString(_T("Dump: Exception occur!!"));
        //return CDumpCore::Instance()->OnDumpFinish(pContext->pExceptionInfo);
#ifdef _WIN64
        SuspendAllButThisThread(false);
#endif
    }

    return 0;
}

CDumpCore::CDumpCore()
    :m_thandle(0)
{
    //默认的ErrLog文件名是
    m_szOriginalFileName[0] = 0;

    //默认dump所有
    for(int i = 0;i < enDumpCount;i++)
        m_dumpSwitch[i] = TRUE;

    //默认dump类型
    m_lCrashDumpType = IsWindows98() ? MiniDumpNormal : MiniDumpWithIndirectlyReferencedMemory;

    m_handleLogFile = INVALID_HANDLE_VALUE;
}

CDumpCore::~CDumpCore()
{
    if(m_previousFilter)
        SetUnhandledExceptionFilter( m_previousFilter );
}

void CDumpCore::OnDumpStart()
{
    (void)fopen_s(&m_fpLog,"Log\\cd.txt","wb+");
    //if(m_pSink)m_pSink->OnDumpStart(Instance());
}

LONG CDumpCore::OnDumpFinish(PEXCEPTION_POINTERS pExceptionInfo)
{
    if(m_fpLog)fclose(m_fpLog);
    //if(m_pSink)
    //{
    //    return m_pSink->OnDumpFinish(Instance(),pExceptionInfo);
    //}
    pExceptionInfo;
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI CDumpCore::UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
    cout << "1" << endl;
    CDumpCore::Instance()->Dump(_T("------------------- CDumpCore::UnhandledExceptionFilter  ----------------------"));
    if (CDumpCore::Instance()->m_previousFilter != nullptr
        && CDumpCore::Instance()->m_previousFilter != &CDumpCore::UnhandledExceptionFilter)
    {
        LONG lRt = CDumpCore::Instance()->m_previousFilter(pExceptionInfo);
        if(EXCEPTION_CONTINUE_SEARCH != lRt)
            return lRt;
    }

    cout << "2" << endl;
    ::OutputDebugString(_T("CDumpCore::UnhandledExceptionFilter"));
    static BOOL bFirstTime = TRUE;
    if (!bFirstTime)
    {
        CDumpCore::Instance()->Dump(_T("ERROR : The Exception handler run in twice!"));
        return EXCEPTION_EXECUTE_HANDLER;
    }

    cout << "3" << endl;
    bFirstTime = FALSE;

    g_dumpContext.exceptionThread = GetCurrentThread();
    g_dumpContext.exceptionThreadID = GetCurrentThreadId();
    g_dumpContext.pExceptionInfo = pExceptionInfo;

    ResumeThread(CDumpCore::Instance()->m_thandle);

    // wait until it has written the report
    WaitForSingleObject(CDumpCore::Instance()->m_thandle, 1000*50);

    return EXCEPTION_EXECUTE_HANDLER;
}

void CDumpCore::DumpMiniDump(PEXCEPTION_POINTERS pExceptionInfo)
{
    time_t ttime;
    ttime = time(nullptr);
    struct tm _tm;
    (void)localtime_s(&_tm, &ttime);

#ifdef _WIN64
    _stprintf_s(g_szForDumpMsg, 256,  _T("%s(%d-%d-%d`%d-%d-%d).dmp"), m_szOriginalFileName, 
        _tm.tm_mon + 1,
        _tm.tm_mday,
        _tm.tm_year + 1900,
        _tm.tm_hour,
        _tm.tm_min,
        _tm.tm_sec);
#else
    _stprintf_s(g_szForDumpMsg, 256,  _T("%s(%d-%d-%d`%d-%d-%d).dmp"), m_szOriginalFileName, 
        _tm.tm_mon + 1,
        _tm.tm_mday,
        _tm.tm_year + 1900,
        _tm.tm_hour,
        _tm.tm_min,
        _tm.tm_sec);
#endif

    HANDLE hMiniDumpFile = CreateFile(
        g_szForDumpMsg,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
        NULL);

    if (hMiniDumpFile != INVALID_HANDLE_VALUE)
    {
        DumpMiniDump(pExceptionInfo,hMiniDumpFile);
        CloseHandle(hMiniDumpFile);
    }
}

BOOL CDumpCore::IsWindows98()
{
    DWORD dwVersion = GetVersion();
    // Get the Windows version.
    DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
    //DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
    DWORD dwBuild;
    // Get the build number.
    
    BOOL bBeforeWin2000 = FALSE;

    if (dwVersion < 0x80000000)              // Windows NT/2000, Whistler
    {
        dwBuild = (DWORD)(HIWORD(dwVersion));
    }
    else if (dwWindowsMajorVersion < 4)      // Win32s
    {
        dwBuild = (DWORD)(HIWORD(dwVersion) & ~0x8000);
        bBeforeWin2000 = TRUE;
    }
    else                                     // Windows 95/98/Me
    {
        bBeforeWin2000 = TRUE;
    }

    return bBeforeWin2000;
}

BOOL CALLBACK MiniDumpCallback(PVOID pParam,const PMINIDUMP_CALLBACK_INPUT pInput,PMINIDUMP_CALLBACK_OUTPUT pOutput) 
{
    //这个地方可以自己控制Dump的详细信息，但试验发现并不好用，再看看
    pParam;
    pInput;
    pOutput;
    return TRUE;

    //BOOL bRet = FALSE; 

    //// Check parameters 
    //
    //if( pInput == 0 ) 
    //    return FALSE; 
    //
    //if( pOutput == 0 ) 
    //    return FALSE; 
    //
    ////char szMsg[256];
    ////sprintf(szMsg,"Type = %d",pInput->CallbackType);
    ////MessageBox(0,szMsg,szMsg,0);
    //
    //// Process callbacks 
    //
    //switch( pInput->CallbackType ) 
    //{
    //case IncludeModuleCallback: 
    //    {
    //        // Include the module into the dump 
    //        //_tprintf( _T("IncludeModuleCallback (module: %08I64x) \n"), 
    //        //    pInput->IncludeModule.BaseOfImage); 
    //        bRet = FALSE; 
    //    }
    //    break; 
    //case IncludeThreadCallback: 
    //    {
    //        // Include the thread into the dump 
    //        //_tprintf( _T("IncludeThreadCallback (thread: %x) \n"), 
    //        //    pInput->IncludeThread.ThreadId); 
    //        bRet = FALSE; 
    //    }
    //    break; 
    //case ModuleCallback: 
    //    {
    //        // Include all available information 
    //        //wprintf( L"ModuleCallback (module: %s) \n", pInput->Module.FullPath ); 
    //        bRet = FALSE; 
    //    }
    //    break; 
    //case ThreadCallback: 
    //    {
    //        // Include all available information 
    //        //_tprintf( _T("ThreadCallback (thread: %x) \n"), pInput->Thread.ThreadId ); 
    //        bRet = FALSE;  
    //    }
    //    break; 
    //case ThreadExCallback: 
    //    {
    //        // Include all available information 
    //        //_tprintf( _T("ThreadExCallback (thread: %x) \n"), pInput->ThreadEx.ThreadId ); 
    //        bRet = FALSE;  
    //    }
    //    break; 
    //case MemoryCallback: 
    //    {
    //        // We do not include any information here -> return FALSE 
    //        //_tprintf( _T("MemoryCallback\n") ); 
    //        bRet = FALSE; 
    //    }
    //    break;
    //}
    //
    //return bRet; 
    //
}

void CDumpCore::DumpMiniDump(PEXCEPTION_POINTERS pExceptionInfo,HANDLE hFile)
{
    if (pExceptionInfo == NULL) 
    {
        // Generate exception to get proper context in dump
        __try 
        {
            OutputDebugString(_T("raising exception\r\n"));
            CloseHandle(hFile);
            RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
        } 
        __except(DumpMiniDump(GetExceptionInformation()),
                 EXCEPTION_CONTINUE_EXECUTION) 
        {
        }
    }
    else
    {
        MINIDUMP_EXCEPTION_INFORMATION eInfo;
        eInfo.ThreadId = g_dumpContext.exceptionThreadID;
        eInfo.ExceptionPointers = pExceptionInfo;
        eInfo.ClientPointers = FALSE;

        MINIDUMP_CALLBACK_INFORMATION mci; 

        mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback; 
        mci.CallbackParam       = 0;     // this example does not use the context

        // note:  MiniDumpWithIndirectlyReferencedMemory does not work on Win98
        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            (MINIDUMP_TYPE)m_lCrashDumpType,
            pExceptionInfo ? &eInfo : NULL,
            NULL,
            &mci);
    }
}

void CDumpCore::DumpAuxi(PEXCEPTION_POINTERS pExceptionInfo)
{
    if(m_dumpSwitch[enDumpBasicInfo])
        DumpBasicInfo(pExceptionInfo);
    if(m_dumpSwitch[enDumpModuleList])
        DumpModuleList(pExceptionInfo);
    if(m_dumpSwitch[enDumpWindowList])
        DumpWindowList(pExceptionInfo);
    if(m_dumpSwitch[enDumpProcessList])
        DumpProcessList(pExceptionInfo);
    if(m_dumpSwitch[enDumpRegister])
        DumpRegister(pExceptionInfo);
}

void CDumpCore::Dump(LPCTSTR szMsg,...)
{
    va_list argptr; 
    va_start( argptr, szMsg );
    int nResult = _vstprintf_s(g_szForDumpMsg, sizeof(g_szForDumpMsg)/sizeof(TCHAR),szMsg, argptr );
    va_end( argptr );

    if ( m_handleLogFile != INVALID_HANDLE_VALUE )
    {
        DWORD dw;
        WriteFile(m_handleLogFile, g_szForDumpMsg, nResult * sizeof(TCHAR), &dw,0);
        //FlushFileBuffers(m_handleLogFile);
    }

    if(m_fpLog)
    {
        _ftprintf(m_fpLog,g_szForDumpMsg);
        fflush(m_fpLog);
    }

}

//Dump系统及异常的基本信息
void CDumpCore::DumpBasicInfo(PEXCEPTION_POINTERS pExceptionInfo)
{
    PEXCEPTION_RECORD pException = pExceptionInfo->ExceptionRecord;
    PCONTEXT          pContext   = pExceptionInfo->ContextRecord;

    TCHAR *pszCrashModuleFileName = (TCHAR*)GetCrashModuleName(pExceptionInfo);

    Dump(_T("%s (0x%08x) %s at %04x:%08x.\r\n\r\n"),
                GetExceptionDesc(pExceptionInfo),
                pException->ExceptionCode,
                pszCrashModuleFileName, pContext->SegCs, pContext->IP);

    Dump(_T("Crashed at FileOffset %08x@%s.\r\n\r\n"),GetCrashFileOffset(pExceptionInfo),pszCrashModuleFileName);

    DumpModuleInfo(GetModuleHandle(pszCrashModuleFileName));

    SYSTEMTIME st;
    GetLocalTime(&st);
    TCHAR szTimeBuffer[256];
    _stprintf_s(szTimeBuffer, sizeof(szTimeBuffer)/sizeof(TCHAR),
        _T("%04d.%02d.%02d %02d:%02d:%02d"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);

    Dump(_T("%s.\r\n"), szTimeBuffer);

    TCHAR szModuleName[MAX_PATH*2];
    ZeroMemory(szModuleName, sizeof(szModuleName));
    if (GetModuleFileName(0, szModuleName, _countof(szModuleName)-2) <= 0)
        lstrcpy(szModuleName, _T("Unknown"));

    TCHAR szUserName[200];
    ZeroMemory(szUserName, sizeof(szUserName));
    DWORD UserNameSize = _countof(szUserName)-2;
    if (!GetUserName(szUserName, &UserNameSize))
        lstrcpy(szUserName, _T("Unknown"));

    Dump(_T("%s, run by %s.\r\n"), szModuleName, szUserName);

    //操作系统相关
    TCHAR szWinVer[50], szMajorMinorBuild[50];
    int nWinVer;
    GetWinVer(szWinVer, &nWinVer, szMajorMinorBuild);
    Dump(_T("Operating system:  %s (%s).\r\n"), 
        szWinVer, szMajorMinorBuild);

    SYSTEM_INFO    SystemInfo;
    GetSystemInfo(&SystemInfo);
    Dump(_T("%d processor(s), type %d.\r\n"),
                SystemInfo.dwNumberOfProcessors, SystemInfo.dwProcessorType);

    MEMORYSTATUS SysMemInfo;
    SysMemInfo.dwLength = sizeof(SysMemInfo);
    GlobalMemoryStatus(&SysMemInfo);
#define ONEM (1024 * 1024)
    //内存信息
    Dump(_T("%d%% memory in use.\r\n"), SysMemInfo.dwMemoryLoad);
    Dump(_T("%d MBytes physical memory.\r\n"), (SysMemInfo.dwTotalPhys +
                ONEM - 1) / ONEM);
    Dump(_T("%d MBytes physical memory free.\r\n"), 
        (SysMemInfo.dwAvailPhys + ONEM - 1) / ONEM);
    Dump(_T("%d MBytes paging file.\r\n"), (SysMemInfo.dwTotalPageFile +
                ONEM - 1) / ONEM);
    Dump(_T("%d MBytes paging file free.\r\n"), 
        (SysMemInfo.dwAvailPageFile + ONEM - 1) / ONEM);
    Dump(_T("%d MBytes user address space.\r\n"), 
        (SysMemInfo.dwTotalVirtual + ONEM - 1) / ONEM);
    Dump(_T("%d MBytes user address space free.\r\n"), 
        (SysMemInfo.dwAvailVirtual + ONEM - 1) / ONEM);

    // 对于access violation类型的异常
    if (pException->ExceptionCode == STATUS_ACCESS_VIOLATION &&
                pException->NumberParameters >= 2)
    {
        TCHAR szDebugMessage[1024];
        const TCHAR* readwrite = _T("Read from");
        if (pException->ExceptionInformation[0])
            readwrite = _T("Write to");
        wsprintf(szDebugMessage, _T("%s location %08x caused an access violation.\r\n"),
                    readwrite, pException->ExceptionInformation[1]);

#ifdef    _DEBUG
        //此AccessViolation是读还是写导致的
        OutputDebugString(_T("Exception handler: "));
        OutputDebugString(szDebugMessage);
#endif

        Dump(_T("%s"), szDebugMessage);
    }
}

void CDumpCore::DumpModuleList(PEXCEPTION_POINTERS pExceptionInfo)
{
    pExceptionInfo;
    Dump(_T("\r\nModule List:\r\n"));
    HANDLE hModuleShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,GetCurrentProcessId());
    MODULEENTRY32 me2 = {sizeof(me2)};
    BOOL bOk = Module32First(hModuleShot,&me2);
    for(;bOk;bOk = Module32Next(hModuleShot,&me2))
    {
        Dump(_T("Base = %p\t%s\r\n"),me2.modBaseAddr,me2.szExePath);
        DumpModuleInfo(me2.hModule);
    }
    CloseHandle(hModuleShot);
}

void CDumpCore::DumpWindowList(PEXCEPTION_POINTERS pExceptionInfo)
{
    pExceptionInfo;
    Dump(_T("\r\n"));
    Dump(_T("Window List:\r\n"));
    TCHAR szWindowText[256];
    HWND hWndTopLevel=GetDesktopWindow();
    hWndTopLevel=GetWindow(hWndTopLevel,GW_CHILD);
    for(;hWndTopLevel;hWndTopLevel=GetWindow(hWndTopLevel,GW_HWNDNEXT))
    {
        GetWindowText(hWndTopLevel,szWindowText,256);
        if( _tcscmp(szWindowText, _T("")) != 0 
            && _tcsicmp(szWindowText,_T("Default IME")) != 0 
            && _tcsicmp(szWindowText,_T("DDE Server Window")) != 0)
        {
            TCHAR szTemp[6];
            if(_tcslen(szWindowText) >= 5)
            {
                memcpy(szTemp,szWindowText,5);
                szTemp[5] = 0;
                if(_tcsicmp(szTemp,_T("msime")) == 0)continue;
            }
            DWORD dwProcessID,dwThreadID;
            //获得创建窗口的进程和线程
            dwThreadID=GetWindowThreadProcessId(hWndTopLevel,&dwProcessID);
            Dump(_T("\tHWND = %x\tProcessID = %d\t%s\r\n"),hWndTopLevel,dwProcessID,szWindowText);
        }
    }
}

void CDumpCore::DumpProcessList(PEXCEPTION_POINTERS pExceptionInfo)
{
    pExceptionInfo;
    Dump(_T("\r\n"));
    Dump(_T("Process List:\r\n"));
    PROCESSENTRY32 me={sizeof(me)};
    HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,GetCurrentProcessId());
    BOOL fOk=Process32First(hSnapshot,&me);
    for(;fOk;fOk=Process32Next(hSnapshot,&me))
    {
        Dump(_T("\tProcessID = %d\t%s\r\n"),me.th32ProcessID,me.szExeFile);
    }
    CloseHandle(hSnapshot);
}

void CDumpCore::DumpRegister(PEXCEPTION_POINTERS pExceptionInfo)
{
    PCONTEXT pContext   = pExceptionInfo->ContextRecord;
    Dump(_T("\r\n\r\nContext:\r\n"));
    Dump(_T("EDI:    0x%08x  ESI: 0x%08x  EAX:   0x%08x\r\n"),
                pContext->DI, pContext->SI, pContext->AX);
    Dump(_T("EBX:    0x%08x  ECX: 0x%08x  EDX:   0x%08x\r\n"),
                pContext->BX, pContext->CX, pContext->DX);
    Dump(_T("EIP:    0x%08x  EBP: 0x%08x  SegCs: 0x%08x\r\n"),
                pContext->IP, pContext->BP, pContext->SegCs);
    Dump(_T("EFlags: 0x%08x  ESP: 0x%08x  SegSs: 0x%08x\r\n"),
                pContext->EFlags, pContext->SP, pContext->SegSs);

    //当前IP所指向的几个字节的值
    //因为无法判断内存是否可读，所有用了try-catch
    Dump(_T("\r\nBytes at CS:EIP:\r\n"));
    BYTE * code = (BYTE *)pContext->IP;
    for (int codebyte = 0; codebyte < NumCodeBytes; codebyte++)
    {
        __try
        {
            Dump(_T("%02x "), code[codebyte]);

        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            Dump(_T("?? "));
        }
    }
}

//Dump调用栈 - 类似于WinDbg的打印CallStack功能
void CDumpCore::DumpCallStack(PCONTEXT pContext,HANDLE hThread)
{    
    SymSetOptions(SYMOPT_DEFERRED_LOADS);//SYMOPT_DEBUG

    if(!SymInitialize(GetCurrentProcess(),0,TRUE))
    {
        Dump(_T("\r\n\r\n%x\r\n\r\n"),GetLastError());
        return;
    }

    WriteStackDetails(pContext,hThread);
    SymCleanup(GetCurrentProcess());
}

LPCTSTR CDumpCore::GetExceptionDescription(DWORD ExceptionCode)
{
    struct ExceptionNames
    {
        DWORD    ExceptionCode;
        TCHAR *    ExceptionName;
    };

#if 0  //winnt.h
#define STATUS_WAIT_0                    ((DWORD   )0x00000000L)    
#define STATUS_ABANDONED_WAIT_0          ((DWORD   )0x00000080L)    
#define STATUS_USER_APC                  ((DWORD   )0x000000C0L)    
#define STATUS_TIMEOUT                   ((DWORD   )0x00000102L)    
#define STATUS_PENDING                   ((DWORD   )0x00000103L)    
#define STATUS_SEGMENT_NOTIFICATION      ((DWORD   )0x40000005L)    
#define STATUS_GUARD_PAGE_VIOLATION      ((DWORD   )0x80000001L)    
#define STATUS_DATATYPE_MISALIGNMENT     ((DWORD   )0x80000002L)    
#define STATUS_BREAKPOINT                ((DWORD   )0x80000003L)    
#define STATUS_SINGLE_STEP               ((DWORD   )0x80000004L)    
#define STATUS_ACCESS_VIOLATION          ((DWORD   )0xC0000005L)    
#define STATUS_IN_PAGE_ERROR             ((DWORD   )0xC0000006L)    
#define STATUS_INVALID_HANDLE            ((DWORD   )0xC0000008L)    
#define STATUS_NO_MEMORY                 ((DWORD   )0xC0000017L)    
#define STATUS_ILLEGAL_INSTRUCTION       ((DWORD   )0xC000001DL)    
#define STATUS_NONCONTINUABLE_EXCEPTION  ((DWORD   )0xC0000025L)    
#define STATUS_INVALID_DISPOSITION       ((DWORD   )0xC0000026L)    
#define STATUS_ARRAY_BOUNDS_EXCEEDED     ((DWORD   )0xC000008CL)    
#define STATUS_FLOAT_DENORMAL_OPERAND    ((DWORD   )0xC000008DL)    
#define STATUS_FLOAT_DIVIDE_BY_ZERO      ((DWORD   )0xC000008EL)    
#define STATUS_FLOAT_INEXACT_RESULT      ((DWORD   )0xC000008FL)    
#define STATUS_FLOAT_INVALID_OPERATION   ((DWORD   )0xC0000090L)    
#define STATUS_FLOAT_OVERFLOW            ((DWORD   )0xC0000091L)    
#define STATUS_FLOAT_STACK_CHECK         ((DWORD   )0xC0000092L)    
#define STATUS_FLOAT_UNDERFLOW           ((DWORD   )0xC0000093L)    
#define STATUS_INTEGER_DIVIDE_BY_ZERO    ((DWORD   )0xC0000094L)    
#define STATUS_INTEGER_OVERFLOW          ((DWORD   )0xC0000095L)    
#define STATUS_PRIVILEGED_INSTRUCTION    ((DWORD   )0xC0000096L)    
#define STATUS_STACK_OVERFLOW            ((DWORD   )0xC00000FDL)    
#define STATUS_CONTROL_C_EXIT            ((DWORD   )0xC000013AL)    
#define STATUS_FLOAT_MULTIPLE_FAULTS     ((DWORD   )0xC00002B4L)    
#define STATUS_FLOAT_MULTIPLE_TRAPS      ((DWORD   )0xC00002B5L)    
#define STATUS_ILLEGAL_VLM_REFERENCE     ((DWORD   )0xC00002C0L)     
#endif

    static ExceptionNames ExceptionMap[] =
    {
        {0x40010005, _T("Control-C")},
        {0x40010008, _T("Control-Break")},
        {0x80000002, _T("Datatype Misalignment")},
        {0x80000003, _T("Breakpoint")},
        {0xc0000005, _T("Access Violation")},
        {0xc0000006, _T("In Page Error")},
        {0xc0000017, _T("No Memory")},
        {0xc000001d, _T("Illegal Instruction")},
        {0xc0000025, _T("Noncontinuable Exception")},
        {0xc0000026, _T("Invalid Disposition")},
        {0xc000008c, _T("Array Bounds Exceeded")},
        {0xc000008d, _T("Float Denormal Operand")},
        {0xc000008e, _T("Float Divide by Zero")},
        {0xc000008f, _T("Float Inexact Result")},
        {0xc0000090, _T("Float Invalid Operation")},
        {0xc0000091, _T("Float Overflow")},
        {0xc0000092, _T("Float Stack Check")},
        {0xc0000093, _T("Float Underflow")},
        {0xc0000094, _T("Integer Divide by Zero")},
        {0xc0000095, _T("Integer Overflow")},
        {0xc0000096, _T("Privileged Instruction")},
        {0xc00000fD, _T("Stack Overflow")},
        {0xc0000142, _T("DLL Initialization Failed")},
        {0xe06d7363, _T("Microsoft C++ Exception")},
    };

    for (int i = 0; i < sizeof(ExceptionMap) / sizeof(ExceptionMap[0]); i++)
        if (ExceptionCode == ExceptionMap[i].ExceptionCode)
            return ExceptionMap[i].ExceptionName;

    return _T("Unknown Exception");
}

LPTSTR CDumpCore::GetFilePart(LPCTSTR source)
{
    TCHAR *result = lstrrchr(source, _T('\\'));
    if (result)
        result++;
    else
        result = (LPTSTR)source;
    return result;
}

LPTSTR CDumpCore::lstrrchr(LPCTSTR string, int ch)
{
    TCHAR *start = (TCHAR *)string;

    while (*string++)                       /* 到字符串末尾 */
        ;
                                            /* 从后向前找 */
    while (--string != start && *string != (TCHAR) ch)
        ;

    if (*string == (TCHAR) ch)
        return (TCHAR *)string;

    return 0;
}

void CDumpCore::DumpModuleInfo(HINSTANCE ModuleHandle)
{
//    bool rc = false;
    TCHAR szModName[MAX_PATH*2];
    ZeroMemory(szModName, sizeof(szModName));

    __try
    {
        if (GetModuleFileName(ModuleHandle, szModName, sizeof(szModName)-2) > 0)
        {
            // If GetModuleFileName returns greater than zero then this must
            // be a valid code module address. Therefore we can try to walk
            // our way through its structures to find the link time stamp.
            IMAGE_DOS_HEADER *DosHeader = (IMAGE_DOS_HEADER*)ModuleHandle;
            if (IMAGE_DOS_SIGNATURE != DosHeader->e_magic)
                return;

            IMAGE_NT_HEADERS *NTHeader = (IMAGE_NT_HEADERS*)((TCHAR *)DosHeader
                        + DosHeader->e_lfanew);
            if (IMAGE_NT_SIGNATURE != NTHeader->Signature)
                return;

            // 取得szModName文件日期和大小
            HANDLE ModuleFile = CreateFile(szModName, GENERIC_READ,
                        FILE_SHARE_READ, 0, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, 0);

            TCHAR TimeBuffer[100];
            TimeBuffer[0] = _T('\0');
            
            DWORD FileSize = 0;
            if (ModuleFile != INVALID_HANDLE_VALUE)
            {
                FileSize = GetFileSize(ModuleFile, 0);
                FILETIME LastWriteTime;
                if (GetFileTime(ModuleFile, 0, 0, &LastWriteTime))
                {
                    FormatTime(TimeBuffer, LastWriteTime);
                }
                CloseHandle(ModuleFile);
            }

            Dump(_T("Image Base: 0x%08x  Image Size: 0x%08x\r\n"), 
                NTHeader->OptionalHeader.ImageBase, 
                NTHeader->OptionalHeader.SizeOfImage), 

            Dump(_T("Checksum:   0x%08x  Time Stamp: 0x%08x\r\n"), 
                NTHeader->OptionalHeader.CheckSum,
                NTHeader->FileHeader.TimeDateStamp);

            Dump(_T("File Size:  %-10d  File Time:  %s\r\n"),
                        FileSize, TimeBuffer);

            Dump(_T("Version Information:\r\n"));

            CMiniVersion ver(szModName);
            TCHAR szBuf[200];
            WORD dwBuf[4];

            ver.GetCompanyName(szBuf, sizeof(szBuf)-1);
            Dump(_T("   Company:    %s\r\n"), szBuf);

            ver.GetProductName(szBuf, sizeof(szBuf)-1);
            Dump(_T("   Product:    %s\r\n"), szBuf);

            ver.GetFileDescription(szBuf, sizeof(szBuf)-1);
            Dump(_T("   FileDesc:   %s\r\n"), szBuf);

            ver.GetFileVersion(dwBuf);
            Dump(_T("   FileVer:    %d.%d.%d.%d\r\n"), 
                dwBuf[0], dwBuf[1], dwBuf[2], dwBuf[3]);

            ver.GetProductVersion(dwBuf);
            Dump(_T("   ProdVer:    %d.%d.%d.%d\r\n"), 
                dwBuf[0], dwBuf[1], dwBuf[2], dwBuf[3]);

            ver.Release();

            Dump(_T("\r\n"));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

void CDumpCore::FormatTime(LPTSTR output, FILETIME TimeToPrint)
{
    output[0] = _T('\0');
    WORD Date, Time;
    if (FileTimeToLocalFileTime(&TimeToPrint, &TimeToPrint) &&
                FileTimeToDosDateTime(&TimeToPrint, &Date, &Time))
    {
        wsprintf(output, _T("%d/%d/%d %02d:%02d:%02d"),
                    (Date / 32) & 15, Date & 31, (Date / 512) + 1980,
                    (Time >> 11), (Time >> 5) & 0x3F, (Time & 0x1F) * 2);
    }
}

void CDumpCore::WriteStackDetails(PCONTEXT pContext,HANDLE hThread)
{
    Dump( _T("\r\n\r\nCall stack:\r\n") );

    Dump( _T("Address   Frame(EBP)     Function            SourceFile\r\n") );

    //DWORD dwMachineType = 0;
    // Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag

    STACKFRAME64 sf;
    memset( &sf, 0, sizeof(sf) );


    // init STACKFRAME for first call
    STACKFRAME64 s; // in/out stackframe
    memset(&s, 0, sizeof(s));
    DWORD imageType;
#ifdef _M_IX86
    // normally, call ImageNtHeader() and use machine info from PE header
    imageType = IMAGE_FILE_MACHINE_I386;
    sf.AddrPC.Offset = pContext->Eip;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrFrame.Offset = pContext->Ebp;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrStack.Offset = pContext->Esp;
    sf.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
    imageType = IMAGE_FILE_MACHINE_AMD64;
    sf.AddrPC.Offset = pContext->Rip;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrFrame.Offset = pContext->Rsp;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrStack.Offset = pContext->Rsp;
    sf.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
    imageType = IMAGE_FILE_MACHINE_IA64;
    sf.AddrPC.Offset = pContext->StIIP;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrFrame.Offset = pContext->IntSp;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrBStore.Offset = pContext->RsBSP;
    sf.AddrBStore.Mode = AddrModeFlat;
    sf.AddrStack.Offset = pContext->IntSp;
    sf.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif

    BOOL bLoop = TRUE;
    while ( bLoop )
    {
        // 获得下一个栈帧
        if ( ! StackWalk64(  imageType,
                            GetCurrentProcess(),
                            hThread,
                            &sf,
                            pContext,
                            0,
                            SymFunctionTableAccess64,
                            SymGetModuleBase64,
                            0 ) )
            break;

        if ( 0 == sf.AddrFrame.Offset ) // 检查Frame是否正确
            break;                      // 无效Frame

        Dump( _T("%08X  %08X  "), sf.AddrPC.Offset, sf.AddrFrame.Offset );

        // 获得此栈帧的函数名
        const int functionNameLength = 512;
        BYTE symbolBuffer[ sizeof(SYMBOL_INFO) + functionNameLength* sizeof(TCHAR) ];

        PSYMBOL_INFO pSymbol = (PSYMBOL_INFO) /*new char[sizeof(SYMBOL_INFO) + 1024]*/symbolBuffer;
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = functionNameLength;

        DWORD64 symDisplacement = 0;    // Displacement of the input address,
                                        // relative to the start of the symbol

        if ( SymFromAddr( GetCurrentProcess(), sf.AddrPC.Offset, &symDisplacement,pSymbol ) )
        {
            TCHAR szBuffer[512];
            
            if(UnDecorateSymbolName(pSymbol->Name,szBuffer,sizeof(szBuffer),UNDNAME_COMPLETE))
            {
                Dump( _T("%s+%I64X"), szBuffer, symDisplacement );
            }
            else
            {
                Dump( _T("%s+%I64X"), pSymbol->Name, symDisplacement );
            }
        }
        else    // 没找到符号，则打印地址
        {
            TCHAR szModule[MAX_PATH] = _T("");
            DWORD section = 0, offset = 0;

            GetLogicalAddress(  (PVOID)sf.AddrPC.Offset,
                                szModule, sizeof(szModule), section, offset );

            Dump( _T("%04X:%08X %s"), section, offset, szModule );
        }

        //获得源文件行号 
        IMAGEHLP_LINE64 lineInfo = { sizeof(IMAGEHLP_LINE64) };
        DWORD dwLineDisplacement;
        if ( SymGetLineFromAddr64( GetCurrentProcess(), sf.AddrPC.Offset,
                                &dwLineDisplacement, &lineInfo ) )
        {
            Dump(_T("  %s line %u"),lineInfo.FileName,lineInfo.LineNumber); 
        }

        //4个可能参数
        Dump(_T("(%d,%d,%d,%d)"),sf.Params[0],sf.Params[1],sf.Params[2],sf.Params[3]);

        Dump( _T("\r\n") );

        // 局部变量和参数
        {
            // Use SymSetContext to get just the locals/params for this frame
            IMAGEHLP_STACK_FRAME imagehlpStackFrame;
            imagehlpStackFrame.InstructionOffset = sf.AddrPC.Offset;
            SymSetContext( GetCurrentProcess(), &imagehlpStackFrame, 0 );

            // Enumerate the locals/parameters
            SymEnumSymbols( GetCurrentProcess(), 0, 0, EnumerateSymbolsCallback, &sf );

            Dump( _T("\r\n") );
        }
    }

}

BOOL CALLBACK CDumpCore::EnumerateSymbolsCallback(
    PSYMBOL_INFO  pSymInfo,
    ULONG         SymbolSize,
    PVOID         UserContext )
{

    SymbolSize;
    const int nBufferSize = 1024*4;
    TCHAR szBuffer[nBufferSize] = {0};

    __try
    {
        if ( CDumpCore::Instance()->FormatSymbolValue( pSymInfo, (STACKFRAME64*)UserContext,
                                szBuffer, sizeof(szBuffer)/sizeof(TCHAR)/*sizeof(szBuffer)/sizeof(TCHAR)*/ ) )  
        {
                                CDumpCore::Instance()->Dump( _T("\t%s\r\n"), szBuffer );
        }
    }
    __except( 1 )
    {
        CDumpCore::Instance()->Dump( _T("punting on symbol %s\r\n"), pSymInfo->Name );
    }

    return TRUE;
}

BOOL CDumpCore::GetLogicalAddress(
        PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset )
{
    MEMORY_BASIC_INFORMATION mbi;
    
    HMODULE h = GetModuleHandle(0);

    if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
        return FALSE;

    DWORD hMod = (DWORD)mbi.AllocationBase;
    if(hMod == 0)
        hMod += (DWORD)h;

    if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
        return FALSE;

    //DOS_HEADER
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

    //NT_HEADER
    PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

    DWORD rva = (DWORD)addr - hMod; // RVA - 文件代码偏移量

    //对所有节，找到一个包含addr线性地址的
    for (   unsigned i = 0;
            i < pNtHdr->FileHeader.NumberOfSections;
            i++, pSection++ )
    {
        DWORD sectionStart = pSection->VirtualAddress;
        DWORD sectionEnd = sectionStart
                    + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

        //address是否处于section内
        if ( (rva >= sectionStart) && (rva <= sectionEnd) )
        {
            //是的话，用section和offset带回
            section = i+1;
            offset = rva - sectionStart;

            return TRUE;
        }
    }

    return FALSE;   // Should never get here!
}

bool CDumpCore::FormatSymbolValue(
            PSYMBOL_INFO pSym,
            STACKFRAME64 * sf,
            TCHAR * pszBuffer,
            unsigned int szWords )
{
    TCHAR * pszCurrBuffer = pszBuffer;
    int nOffset = 0;

    //variable是局部变量还是参数
    if ( pSym->Flags & SYMFLAG_PARAMETER )
    {
        nOffset = _stprintf_s( pszCurrBuffer, szWords, _T("Parameter ") );
    }
    else if ( pSym->Flags & SYMFLAG_LOCAL )
    {
        nOffset = _stprintf_s( pszCurrBuffer, szWords , _T("Local ") );
    }

    pszCurrBuffer += nOffset;
    szWords -= nOffset;

    // pSym->Tag == 5表示是一个函数
    if ( pSym->Tag == SymTagFunction )   // SymTagFunction from CVCONST.H from the DIA SDK
        return false;


    nOffset = _stprintf_s( pszCurrBuffer, szWords, _T("\'%s\'"), pSym->Name );

    pszCurrBuffer += nOffset;
    szWords -= nOffset;

    DWORD64 pVariable = 0;    //此地址指向variable的内存

    if ( pSym->Flags & SYMFLAG_REGREL )
    {
        // if ( pSym->Register == 8 )   // EBP is the value 8 (in DBGHELP 5.1)
        {                               //  This may change!!!
            pVariable = sf->AddrFrame.Offset;
            pVariable += (DWORD_PTR)pSym->Address;
        }
        // else
        //  return false;
    }
    else if ( pSym->Flags & SYMFLAG_REGISTER )
    {
        return false;   // 这里不打印寄存器变量
    }
    else
    {
        pVariable = (DWORD_PTR)pSym->Address;   //全局变量
    }

    //此variable是否UDT，bHandled   
    bool bHandled = false;
    DWORD dwSysTag = 0;
    SymGetTypeInfo( GetCurrentProcess(), pSym->ModBase, pSym->TypeIndex, TI_GET_SYMTAG,
        &dwSysTag );
    if ( dwSysTag == SymTagUDT )
    {
        pszCurrBuffer = DumpTypeIndex(pszCurrBuffer, szWords, pSym->ModBase, pSym->TypeIndex,
            0, pVariable, bHandled );
    }



    if ( !bHandled )
    {
        //如果不是个UDT，则根据此Variable的长度，猜测其类型 - 目前只支持char,WORD,DWORD
        BasicType basicType = GetBasicType( pSym->TypeIndex, pSym->ModBase );
        
        pszCurrBuffer = FormatOutputValue(pszCurrBuffer, szWords, basicType, pSym->Size,
                                            (PVOID)pVariable ); 
    }


    return true;
}

//对于UDT，递归它的成员，直到成员的类型是C++基础类型
TCHAR * CDumpCore::DumpTypeIndex(
        TCHAR * pszCurrBuffer,
        unsigned int & szWords,
        DWORD64 modBase,
        DWORD dwTypeIndex,
        unsigned nestingLevel,
        DWORD64 offset,
        bool & bHandled )
{
    bHandled = false;
    int nOffset = 0;
    //获得symbol的名称 - 对于UDT，是其类型名
    //否则是一个成员名称
    TCHAR * pwszTypeName;
    if ( SymGetTypeInfo( GetCurrentProcess(), modBase, dwTypeIndex, TI_GET_SYMNAME,
                        &pwszTypeName ) )
    {
        nOffset =  _stprintf_s( pszCurrBuffer, szWords, _T(" %ls"), pwszTypeName );
        LocalFree( pwszTypeName );
    }

    pszCurrBuffer += nOffset;
    szWords -= nOffset;

    //if ( dwNest )
    //{
    //    return pszCurrBuffer;
    //}

    //有多少个成员
    DWORD dwChildrenCount = 0;
    SymGetTypeInfo( GetCurrentProcess(), modBase, dwTypeIndex, TI_GET_CHILDRENCOUNT,
                    &dwChildrenCount );

    if ( !dwChildrenCount )     // If no children, we're done
        return pszCurrBuffer;

    // Prepare to get an array of "TypeIds", representing each of the children.
    // SymGetTypeInfo(TI_FINDCHILDREN) expects more memory than just a
    // TI_FINDCHILDREN_PARAMS struct has.  Use derivation to accomplish this.
    struct FINDCHILDREN : TI_FINDCHILDREN_PARAMS
    {
        ULONG   MoreChildIds[1024];
        FINDCHILDREN(){Count = sizeof(MoreChildIds) / sizeof(MoreChildIds[0]);}
    } children;

    children.Count = dwChildrenCount;
    children.Start= 0;

    // Get the array of TypeIds, one for each child type
    if ( !SymGetTypeInfo( GetCurrentProcess(), modBase, dwTypeIndex, TI_FINDCHILDREN,
                            &children ) )
    {
        return pszCurrBuffer;
    }

    // Append a line feed
    nOffset = _stprintf_s( pszCurrBuffer, szWords, _T("\r\n") );
    pszCurrBuffer += nOffset;
    szWords -= nOffset;

    // Iterate through each of the children
    for ( unsigned i = 0; i < dwChildrenCount; i++ )
    {
        // Add appropriate indentation level (since this routine is recursive)
        for ( unsigned j = 0; j <= nestingLevel+1; j++ )
        {
            nOffset = _stprintf_s( pszCurrBuffer, szWords , _T("\t") );
            pszCurrBuffer += nOffset;
            szWords -= nOffset;
        }

        //if ( szWords < 256 )
        //{
        //    break;
        //}

        // Recurse for each of the child types
        bool bHandled2 = false;

        //DWORD dwSysTag = 0;
        //SymGetTypeInfo( GetCurrentProcess(), modBase, dwTypeIndex, TI_GET_SYMTAG,
        //    &dwSysTag );
        //if ( dwSysTag == SymTagUDT )
        //{
        //    pszCurrBuffer = DumpTypeIndex( pszCurrBuffer, szWords, modBase,
        //        children.ChildId[i], nestingLevel+1,
        //        offset, bHandled2 );
        //}


        // If the child wasn't a UDT, format it appropriately
        if ( !bHandled2 )
        {
            // Get the offset of the child member, relative to its parent
            DWORD dwMemberOffset;
            if ( !SymGetTypeInfo( GetCurrentProcess(), modBase, children.ChildId[i],
                            TI_GET_OFFSET, &dwMemberOffset ) )
            {
                continue;
            }

            // Get the real "TypeId" of the child.  We need this for the
            // SymGetTypeInfo( TI_GET_TYPEID ) call below.
            DWORD typeId;
            if ( ! SymGetTypeInfo( GetCurrentProcess(), modBase, children.ChildId[i],
                            TI_GET_TYPEID, &typeId ) )
            {
                continue;
            }

            // Get the size of the child member
            ULONG64 length;
            if ( !SymGetTypeInfo(GetCurrentProcess(), modBase, typeId, TI_GET_LENGTH,&length) )
            {
                continue;
            }

            TCHAR * pszChildName;
            if ( !SymGetTypeInfo(GetCurrentProcess(), modBase, typeId, TI_GET_SYMNAME,&pszChildName) )
            {
                continue;
            }
            else
            {
                if ( pszChildName )
                {
                    nOffset = _stprintf_s( pszCurrBuffer, szWords, _T(" %s"), pszChildName );
                    pszCurrBuffer += nOffset;
                    szWords -= nOffset;

                    LocalFree(pszChildName);
                }
            }

            // Calculate the address of the member
            DWORD64 dwFinalOffset = offset + dwMemberOffset;

            BasicType basicType = GetBasicType(children.ChildId[i], modBase );

            pszCurrBuffer = FormatOutputValue( pszCurrBuffer, szWords, basicType,
                                                length, (PVOID)dwFinalOffset ); 

            nOffset = _stprintf_s( pszCurrBuffer, szWords, _T("\r\n") );
            pszCurrBuffer += nOffset;
            szWords -= nOffset;

        }
    }

    bHandled = true;
    return pszCurrBuffer;
}

BasicType CDumpCore::GetBasicType( DWORD typeIndex, DWORD64 modBase )
{
    BasicType basicType;
    if ( SymGetTypeInfo( GetCurrentProcess(), modBase, typeIndex,
                        TI_GET_BASETYPE, &basicType ) )
    {
        return basicType;
    }

    // Get the real "TypeId" of the child.  We need this for the
    // SymGetTypeInfo( TI_GET_TYPEID ) call below.
    DWORD typeId;
    if (SymGetTypeInfo(GetCurrentProcess(),modBase, typeIndex, TI_GET_TYPEID, &typeId))
    {
        if ( SymGetTypeInfo( GetCurrentProcess(), modBase, typeId, TI_GET_BASETYPE,
                            &basicType ) )
        {
            return basicType;
        }
    }

    return btNoType;
}


TCHAR * CDumpCore::FormatOutputValue(   TCHAR * pszCurrBuffer, unsigned int& szWords,
                                                    BasicType basicType,
                                                    DWORD64 length,
                                                    PVOID pAddress
                                                    )
{
    // Format appropriately (assuming it's a 1, 2, or 4 bytes (!!!)
    
    int nOffset = 0;
    
    if ( length == 1 )
    {
        nOffset = _stprintf_s( pszCurrBuffer, szWords, _T(" 9= %X"), *(PBYTE)pAddress );
    }
    else if ( length == 2 )
    {
        nOffset = _stprintf_s( pszCurrBuffer, szWords, _T(" = %X"), *(PWORD)pAddress );
    }
    else if ( length == 4 )
    {
        if ( basicType == btFloat )
        {
            nOffset = _stprintf_s(pszCurrBuffer, szWords,_T(" = %f"), *(PFLOAT)pAddress);
        }
        else if ( basicType == btChar  )
        {
            if ( !IsBadStringPtrA( *(LPSTR*)pAddress, 32) )
            {
#if defined(UNICODE)
                // to wide char
                TCHAR szWbuffer[64];
                ::MultiByteToWideChar(CP_ACP,0, *(LPSTR*)pAddress, 32, szWbuffer, sizeof(szWbuffer)/sizeof(TCHAR));
                nOffset = _stprintf_s( pszCurrBuffer, szWords, _T(" = \"%.32s\""),
                    szWbuffer );
#else
                nOffset = _stprintf_s( pszCurrBuffer, szWords, _T(" = \"%.32s\""),
                                            *(PDWORD)pAddress );
#endif
            }
            else
            {
                nOffset = _stprintf_s( pszCurrBuffer, szWords, _T(" = %X"),
                                            *(PDWORD)pAddress );
            }
        }
        else if ( basicType == btWChar )
        {
            if ( !IsBadStringPtrW( *(LPTSTR*)pAddress, 32) )
            {
#if !defined(UNICODE)
                // to multi-byte
                TCHAR szWbuffer[64];
                ::WideCharToMultiByte(CP_ACP,0, *(LPTSTR*)pAddress, 32, szWbuffer, sizeof(szWbuffer)/sizeof(TCHAR));
                nOffset = _stprintf_s( pszCurrBuffer, szWords, _T(" = \"%.32s\""),
                    szWbuffer );
#else
                nOffset = _stprintf_s( pszCurrBuffer, szWords, _T(" = \"%.32s\""),
                    *(PDWORD)pAddress );
#endif
            }
            else
            {
                nOffset = _stprintf_s( pszCurrBuffer, szWords, _T(" = %X"),
                    *(PDWORD)pAddress );
            }
        }
        else
        {
            nOffset = _stprintf_s(pszCurrBuffer, szWords, _T(" = %X"), *(PDWORD)pAddress);
        }
    }
    else if ( length == 8 )
    {
        if ( basicType == btFloat )
        {
            nOffset = _stprintf_s( pszCurrBuffer, szWords,_T(" = %lf"),
                                        *(double *)pAddress );
        }
        else
            nOffset = _stprintf_s( pszCurrBuffer, szWords, _T(" = %I64X"),
                                        *(DWORD64*)pAddress );
    }

    pszCurrBuffer += nOffset;
    szWords -= nOffset;

    return pszCurrBuffer;
}


void CDumpCore::SetDumpSwitch(int iEnum,BOOL bOn)
{
    if(iEnum >= 0 && iEnum < enDumpCount)
    {
        m_dumpSwitch[iEnum] = bOn;
    }
}

//
//void CDumpCore::SetLogFileName(LPCTSTR szLogFileName)
//{
//    if(szLogFileName)
//    {
//        _tcscpy_s(m_szLogFileName, sizeof(m_szLogFileName)/sizeof(TCHAR),szLogFileName);
//    }
//}


void CDumpCore::SetOriginalFileName(LPCTSTR szOriginalFileName)
{
    if(szOriginalFileName)
    {
        _tcscpy_s(m_szOriginalFileName, sizeof(m_szOriginalFileName)/sizeof(TCHAR),szOriginalFileName);
    }
}


DWORD CDumpCore::GetExceptionCode(PEXCEPTION_POINTERS pExceptionInfo)
{
    if(pExceptionInfo)
    {
        return pExceptionInfo->ExceptionRecord->ExceptionCode;
    }

    return 0;
}


LPCTSTR    CDumpCore::GetExceptionDesc(PEXCEPTION_POINTERS pExceptionInfo)
{
    if(pExceptionInfo)
    {
        return GetExceptionDescription(pExceptionInfo->ExceptionRecord->ExceptionCode);
    }

    return _T("Unknown Exception");
}


OFFSET_TYPE CDumpCore::GetCrashFileOffset(PEXCEPTION_POINTERS pExceptionInfo)
{
    if(!pExceptionInfo)return 0;

//    PEXCEPTION_RECORD pException = pExceptionInfo->ExceptionRecord;
    PCONTEXT          pContext   = pExceptionInfo->ContextRecord;

    HINSTANCE hCrashMod = GetModuleHandle(GetCrashModuleName(pExceptionInfo));
    OFFSET_TYPE lOffset = pContext->IP - (OFFSET_TYPE)hCrashMod;

    return lOffset;
}


LPCTSTR    CDumpCore::GetCrashModuleName(PEXCEPTION_POINTERS pExceptionInfo)
{
//    PEXCEPTION_RECORD pException = pExceptionInfo->ExceptionRecord;
    PCONTEXT          pContext   = pExceptionInfo->ContextRecord;

    static TCHAR szCrashModulePathName[MAX_PATH*2];
    _tcscpy_s(szCrashModulePathName, sizeof(szCrashModulePathName)/sizeof(TCHAR), _T("Unknown"));

    TCHAR *pszCrashModuleFileName = szCrashModulePathName;

    MEMORY_BASIC_INFORMATION MemInfo;

    // From MSDN:
    // VirtualQuery can be used to get the allocation base associated with a
    // code address, which is the same as the ModuleHandle. This can be used
    // to get the filename of the module that the crash happened in.
    if (VirtualQuery((void*)pContext->IP, &MemInfo, sizeof(MemInfo)) &&
                        (GetModuleFileName((HINSTANCE)MemInfo.AllocationBase,
                                          szCrashModulePathName,
                                          sizeof(szCrashModulePathName)-2) > 0))
    {
        pszCrashModuleFileName = GetFilePart(szCrashModulePathName);
    }

    return pszCrashModuleFileName;
}


void CDumpCore::DumpLn(LPCTSTR szMsg)
{
    Instance()->Dump(szMsg);
    Instance()->Dump(_T("\r\n"));
}


void CDumpCore::GetCrashModuleProductVer(PEXCEPTION_POINTERS pExceptionInfo,WORD pwVer[4])
{
    LPCTSTR pszCrashModuleName = GetCrashModuleName(pExceptionInfo);
    CMiniVersion ver(pszCrashModuleName);
    ver.GetProductVersion(pwVer);
}

OString CDumpCore::GetCallStack(CONTEXT& Context,HANDLE hThread)
{
//    DumpCallStack(&Context,hThread);
//    return m_szRpt;
    Context;
    hThread;
    return _T("Unavailable!");
}

void CDumpCore::SetDumpType(long lDumpType)
{
    m_lCrashDumpType = lDumpType;
}

void CDumpCore::DumpDirectForTest( PEXCEPTION_POINTERS pExceptionInfo )
{
    //DumpCallStack(pExceptionInfo->ContextRecord, GetCurrentThread());
    UnhandledExceptionFilter(pExceptionInfo);
}

void CDumpCore::StartDumpDaemon()
{
    (void)InstallExceptionFilter();
    m_thandle = CreateThread(NULL, 0, HandlingMyExceptProc, &g_dumpContext, CREATE_SUSPENDED, NULL);
}

BOOL CDumpCore::SuspendAllButThisThread( bool bsuspend /*= true */ )
{
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
    THREADENTRY32 te32; 

    // Take a snapshot of all running threads  
    hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
    if( hThreadSnap == INVALID_HANDLE_VALUE ) 
        return( FALSE ); 

    // Fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32 ); 

    // Retrieve information about the first thread,
    // and exit if unsuccessful
    if( !Thread32First( hThreadSnap, &te32 ) ) 
    {
        CloseHandle( hThreadSnap );     // Must clean up the snapshot object!
        return( FALSE );
    }

    // Suspend all threads but this one while we write the report
    do 
    { 
        if( te32.th32OwnerProcessID == GetCurrentProcessId()
            && te32.th32ThreadID != GetCurrentThreadId() // don't stop this thread!
            )
        {
            HANDLE hthread2suspend = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
            if (bsuspend)// suspend all other threads
                SuspendThread(hthread2suspend);
            else
                ResumeThread(hthread2suspend);

            CloseHandle(hthread2suspend);
        }
    } while( Thread32Next(hThreadSnap, &te32)); 

    CloseHandle( hThreadSnap );
    return( TRUE );
}

bool CDumpCore::InstallExceptionFilter()
{
    m_previousFilter = ::SetUnhandledExceptionFilter(UnhandledExceptionFilter);

#ifndef _WIN64
    //禁止CRT设置，为了捕获C++异常
    DisableSetUnhandledExceptionFilter();
#endif
    return true;
}


void CDumpCore::createReportProcess()
{
    Dump(_T("CDumpCore::createReportProcess"));
    TCHAR chPath[512];
    ::GetCurrentDirectory(512,(LPTSTR)chPath);

    std::wstring strTmp = chPath;
    std::wstring filePath;
    size_t dot = strTmp.rfind(_T("\\"));
    if ( dot != std::string::npos )
    {
        filePath = strTmp.substr(0, dot);
    }
    else
    {
        filePath = chPath;
    }
    //filePath = chPath;
    std::wstring ReportFileName = filePath + _T("\\bugReport.exe");

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &pi, sizeof(pi) );
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

    SetCurrentDirectoryW(filePath.c_str());
    if (CreateProcess(ReportFileName.c_str(), _T(""), NULL, NULL, FALSE, 0, NULL, _T(".\\"), &si, &pi))
    {
        Dump(_T("create process success."));
    }
    else
    {
        Dump(_T("create process fail."));
    }
    SetCurrentDirectoryW(chPath);

}
