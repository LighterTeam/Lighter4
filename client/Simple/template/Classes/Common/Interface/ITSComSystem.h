#pragma once

#include "TSUnknown.hpp"

#define ComSystem _T("ComSystem.dll")

#define ITSComSystemGUID "B8498F82-9041-4918-A71D-BCD07A0C3277"

interface ITSComSystem: public TSUnKnown
{
    virtual HRESULT CALLBACK Initialize() = 0;
    virtual HRESULT CALLBACK UnInitialize() = 0;
};

