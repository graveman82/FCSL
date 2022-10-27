/*
-----------------
 Persistent info
-----------------
(C) 2012-2022 FCSL project. Marat Sungatullin

.........
License:
.........

is in the "LICENSE" file.

......
 Web:
......

 + <forum> (for questions and help)

------
 Desc
------
Purpose: configuration header for render library.

----------------------
 for developers notes
----------------------

*/
#ifndef FCD3D9RENDERDEVICE_CFG_H_INCL
#define FCD3D9RENDERDEVICE_CFG_H_INCL

// system headers
#include <d3d9.h>
#include <d3dx9.h>


#include "FcD3d9RenderDeviceIntTypes.h"


#ifdef FC_DEBUG
#include <stdio.h>
#   define FC_ASSERT(expr) \
{\
    if (!(expr)) \
    { \
        char msg[1024]; sprintf(msg, "%s in\n%s[%d]", #expr, __FILE__,__LINE__);\
        MessageBoxA(0, msg, "assert failed", MB_OK|MB_TOPMOST|MB_ICONERROR);\
        abort();\
    }\
}
#else
#   define FC_ASSERT(expr) ((void)sizeof(char))
#endif

#endif // FCD3D9RENDERDEVICE_CFG_H_INCL
