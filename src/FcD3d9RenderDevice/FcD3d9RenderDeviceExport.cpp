#include "FcD3d9RenderDevice.h"
#include "FcD3d9RenderDeviceMain.h"
#include <memory>
//=============================================================================
// export

typedef std::unique_ptr<FcD3d9RenderDevice> FcD3d9RenderDevicePtr;
static FcD3d9RenderDevicePtr spFcD3d9RenderDevice;

FcInt32 FCD3D9RENDERDEVICE_API __stdcall fcRD3d9Initialize(HWND hWnd)
{
    spFcD3d9RenderDevice.reset(new FcD3d9RenderDevice);
    return spFcD3d9RenderDevice->Initialize(hWnd);
}

void FCD3D9RENDERDEVICE_API __stdcall fcRD3d9Terminate()
{
    spFcD3d9RenderDevice->Terminate();
}

FcUInt32 FCD3D9RENDERDEVICE_API __stdcall fcRD3d9GetDisplayModeCount()
{
    return spFcD3d9RenderDevice->GetDisplayModeCount();
}

bool FCD3D9RENDERDEVICE_API __stdcall fcRD3d9GetDisplayMode(FcUInt32 iMode,
                    FcUInt32& width,
                    FcUInt32& height,
                    FcUInt32& defRefreshRate,
                    FcUInt32& nRefreshRates)
{
    return spFcD3d9RenderDevice->GetDisplayMode(iMode, width, height, defRefreshRate, nRefreshRates);
}

bool FCD3D9RENDERDEVICE_API __stdcall fcRD3d9GetDisplayModeRefreshRate(FcUInt32 iMode, FcUInt32 iRefreshRate, FcUInt32& refreshRate)
{
    return spFcD3d9RenderDevice->GetDisplayModeRefreshRate(iMode, iRefreshRate, refreshRate);
}
