/****************************************************************************
Copyright (c) 2010 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "platform/CCCommon.h"
#include "CCStdC.h"

#include <fstream>
#include <time.h> 
#include <stdio.h> 
#include <string>

NS_CC_BEGIN

#define MAX_LEN         (cocos2d::kMaxLogLen + 1)

void CCLog2(const char* filename, int lineNumber, const char * pszFormat, ...)
{
    static std::ofstream* outfile = 0;
    time_t t = time(0); 
    char tmp[64]; 
    strftime( tmp, sizeof(tmp), "%Y-%m-%d-%H-%M-%S",localtime(&t)); 
    if (outfile == 0)
    {
        std::string& nn = std::string("") + tmp + "-TSLog.log";
        outfile = new std::ofstream(nn.c_str());
    }

    char szBuf[MAX_LEN];

    va_list ap;
    va_start(ap, pszFormat);
    vsnprintf_s(szBuf, MAX_LEN, MAX_LEN, pszFormat, ap);
    va_end(ap);

    WCHAR wszBuf[MAX_LEN] = {0};
    MultiByteToWideChar(CP_UTF8, 0, szBuf, -1, wszBuf, sizeof(wszBuf));
    OutputDebugStringW(wszBuf);
    OutputDebugStringA("\n");

    WideCharToMultiByte(CP_ACP, 0, wszBuf, sizeof(wszBuf), szBuf, sizeof(szBuf), NULL, FALSE);

    WORD colorOld = 0;
    HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(handle, &csbi);
    colorOld = csbi.wAttributes;
    SetConsoleTextAttribute(handle, 2);
    printf("[%s]", tmp);
    SetConsoleTextAttribute(handle, 6);
    printf("%s\n", szBuf);
    SetConsoleTextAttribute(handle, 3);
    printf("[%s:%d]\n", filename, lineNumber);
    SetConsoleTextAttribute(handle, colorOld);

    *outfile<<"["<<tmp<<"] "<<szBuf<<" ["<<filename<<":"<<lineNumber<<"]"<<"\n";
}
void CCLog(const char * pszFormat, ...)
{
    char szBuf[MAX_LEN];

    va_list ap;
    va_start(ap, pszFormat);
    vsnprintf_s(szBuf, MAX_LEN, MAX_LEN, pszFormat, ap);
    va_end(ap);

    WCHAR wszBuf[MAX_LEN] = {0};
    MultiByteToWideChar(CP_UTF8, 0, szBuf, -1, wszBuf, sizeof(wszBuf));
    OutputDebugStringW(wszBuf);
    OutputDebugStringA("\n");

    WideCharToMultiByte(CP_ACP, 0, wszBuf, sizeof(wszBuf), szBuf, sizeof(szBuf), NULL, FALSE);
    printf("%s\n", szBuf);
}

void CCMessageBox(const char * pszMsg, const char * pszTitle)
{
    MessageBoxA(NULL, pszMsg, pszTitle, MB_OK);
}

void CCLuaLog(const char *pszMsg)
{
    int bufflen = MultiByteToWideChar(CP_UTF8, 0, pszMsg, -1, NULL, 0);
    WCHAR* widebuff = new WCHAR[bufflen + 1];
    memset(widebuff, 0, sizeof(WCHAR) * (bufflen + 1));
    MultiByteToWideChar(CP_UTF8, 0, pszMsg, -1, widebuff, bufflen);

    OutputDebugStringW(widebuff);
    OutputDebugStringA("\n");

	bufflen = WideCharToMultiByte(CP_ACP, 0, widebuff, -1, NULL, 0, NULL, NULL);
	char* buff = new char[bufflen + 1];
	memset(buff, 0, sizeof(char) * (bufflen + 1));
	WideCharToMultiByte(CP_ACP, 0, widebuff, -1, buff, bufflen, NULL, NULL);
	puts(buff);

	delete[] widebuff;
	delete[] buff;
}

NS_CC_END
