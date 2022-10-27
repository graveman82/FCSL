#ifndef FCD3D9RENDERDEVICE_MAIN_H_INCL
#define FCD3D9RENDERDEVICE_MAIN_H_INCL

#include <windows.h>
#include "FcD3d9RenderDeviceIntTypes.h"

/*  To use this exported function of dll, include this header
 *  in your project.
 */

#ifdef BUILD_DLL
    #define FCD3D9RENDERDEVICE_API __declspec(dllexport)
#else
    #define FCD3D9RENDERDEVICE_API __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C"
{
#endif


FcInt32     FCD3D9RENDERDEVICE_API __stdcall fcRD3d9Initialize(HWND hWnd);
void        FCD3D9RENDERDEVICE_API __stdcall fcRD3d9Terminate();

FcUInt32    FCD3D9RENDERDEVICE_API __stdcall fcRD3d9GetDisplayModeCount();

bool        FCD3D9RENDERDEVICE_API __stdcall fcRD3d9GetDisplayMode(FcUInt32 iMode,
                    FcUInt32& width,
                    FcUInt32& height,
                    FcUInt32& defRefreshRate,
                    FcUInt32& nRefreshRates);

bool        FCD3D9RENDERDEVICE_API __stdcall fcRD3d9GetDisplayModeRefreshRate(FcUInt32 iMode,
                                                                              FcUInt32 iRefreshRate,
                                                                              FcUInt32& refreshRate);
#ifdef __cplusplus
}
#endif


//=============================================================================
// exported functions typedefs
typedef FcInt32     (*__stdcall fcRD3d9Initialize_t)(HWND hWnd);
typedef void        (*__stdcall fcRD3d9Terminate_t)();

typedef FcUInt32    (*__stdcall fcRD3d9GetDisplayModeCount_t)();

typedef bool        (*__stdcall fcRD3d9GetDisplayMode_t)(FcUInt32 iMode,
                    FcUInt32& width,
                    FcUInt32& height,
                    FcUInt32& defRefreshRate,
                    FcUInt32& nRefreshRates);

typedef bool        (*__stdcall fcRD3d9GetDisplayModeRefreshRate_t)(FcUInt32 iMode,
                                                            FcUInt32 iRefreshRate,
                                                            FcUInt32& refreshRate);
#endif // FCD3D9RENDERDEVICE_MAIN_H_INCL
