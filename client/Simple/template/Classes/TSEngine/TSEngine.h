#ifndef __TSENGINE_H__
#define __TSENGINE_H__

#include "TSObject.h"
#include <string>
#include <vector>

class TSEngine : public TSObject {
public:
    static TSEngine* GetSingleTon() {
        static TSEngine tsE;
        return &tsE;
    }
	
    static int toInt(std::string str) ;

    static int TST_StringFilt(std::string& lpszString, char szSeps, std::vector<std::string>& tArray);

    static std::string toString(int id);

    //UTF8转化为GBK格式
    static void ConvertUtf8ToGBK(std::string &strUtf8);

    //GBK转化为UTF8格式
    static void ConvertGBKToUtf8(std::string &strGBK);

    static std::string GetHeader(char* rbuf, int len){
        std::string sheader;
        for (int i = 0 ; i < len ; i++)
        {
            if (rbuf[i] == ',')
            {
                return sheader;
            }
            sheader += rbuf[i];
        }
        return sheader;
    }

    int g_Num;
};

#endif