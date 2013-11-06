#include "main.h"
#include "AppDelegate.h"
#include "CCEGLView.h"
#include "TSApplication.h"


#ifdef WIN32
#include "Common/Interface/crash_interface.h"
#include <tchar.h>
const TCHAR* SCENE_CRASH_DUMP = _T("SceneCrashDump");
#endif

USING_NS_CC;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    //ParseJsonFromString();
#ifdef WIN32
    ICrashDump* pDumpSys = nullptr;
    HRESULT hr = CoTsCreateInstance(CRASH_DUMP_LIBRARY,ICrashDumpGUID,(void**)&pDumpSys);
    pDumpSys->Initialize(SCENE_CRASH_DUMP);
#endif

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef USE_WIN32_CONSOLE
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif

    // create the application instance
    AppDelegate app;
    CCEGLView* eglView = CCEGLView::sharedOpenGLView();
    eglView->setFrameSize(640, 480);
    int ret = TSApplication::sharedApplication()->run();

#ifdef USE_WIN32_CONSOLE
    FreeConsole();
#endif

    return ret;
}
