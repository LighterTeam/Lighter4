#pragma once

#ifndef WIN32
typedef int HRESULT;


//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

//
// and the inverse
//

#define FAILED(hr) (((HRESULT)(hr)) < 0)

//
// MessageId: S_OK
//
// MessageText:
//
// Catastrophic ok
//
#define S_OK 0

//
// MessageId: E_FAIL
//
// MessageText:
//
// Unspecified error
//
#define E_FAIL 1

#endif
