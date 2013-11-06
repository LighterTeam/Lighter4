#ifndef __TSApplication_H__
#define __TSApplication_H__

#include "CCApplication.h"
#include "CCStdC.h"
#include "platform/CCCommon.h"
#include "platform/CCApplicationProtocol.h"
#include <string>

NS_CC_BEGIN

class TSApplication : public CCApplication
{
public:
    /**
    @brief    Run the message loop.
    */
    virtual int run();
};

NS_CC_END
#endif