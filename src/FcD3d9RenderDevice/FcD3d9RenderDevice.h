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
Purpose: direct3d9 based render device.

----------------------
 for developers notes
----------------------

*/
#ifndef FCD3D9RENDERDEVICE_DEVICE_H_INCL
#define FCD3D9RENDERDEVICE_DEVICE_H_INCL

#include "FcD3d9RenderDeviceCfg.h"

#ifdef FCD3D9RENDERDEVICE__USE_STL_VECTOR_FOR_DYNAMICARRAY
#include <vector>
#endif

template <typename TCom>
void FcReleaseCom(TCom* pCom)
{
    if (pCom)
        pCom->Release();
}

enum eFcD3d9RenderDeviceErrorCodes
{
    kFCD3D9RENDERDEVICE_ERRC_OK,
    kFCD3D9RENDERDEVICE_ERRC_FAILED_D3D,
    kFCD3D9RENDERDEVICE_ERRC_FAILED_GETCAPS,
    kFCD3D9RENDERDEVICE_ERRC_NOTnL,
};

//-----------------------------------------------------------------------------

class FcD3d9RenderDevice
{
public:
    FcD3d9RenderDevice();
    ~FcD3d9RenderDevice();

    FcInt32 Initialize(HWND hWnd);
    void Terminate();

    FcUInt32 GetDisplayModeCount();
    bool GetDisplayMode(FcUInt32 iMode,
                        FcUInt32& width,
                        FcUInt32& height,
                        FcUInt32& defRefreshRate,
                        FcUInt32& nRefreshRates);
    bool GetDisplayModeRefreshRate(FcUInt32 iMode, FcUInt32 iRefreshRate, FcUInt32& refreshRate);
private:
    class FcD3d9RenderDeviceImpl* pImpl_{};
};
#endif // FCD3D9RENDERDEVICE_DEVICE_H_INCL

