#include "FcD3d9RenderDevice.h"

#include <string.h>
#include <algorithm>


class FcD3d9RenderDeviceImpl
{
public:
    enum eTextureCaps
    {
        kTexCaps_VolumeMap_NonPow2 = (1 << 2), // Device supports nonpow2 3D textures.
        kTexCaps_ProjBumpEnv = (1 << 3), // Device supports projected bump environmet maps.

    };

    struct D3DDISPLAYMODE_EX : public D3DDISPLAYMODE
    {
        D3DDISPLAYMODE_EX(const D3DDISPLAYMODE& rhs)
        {
            Width = rhs.Width;
            Height = rhs.Height;
            RefreshRate = rhs.RefreshRate;
            Format = rhs.Format;
            RefreshRates.reserve(8);
            RefreshRates.push_back(RefreshRate);
        }

        void sortRefreshRate()
        {
            std::stable_sort(RefreshRates.begin(), RefreshRates.end());
            auto it = std::find_if(RefreshRates.begin(), RefreshRates.end(),
                                       [](const UINT& rr)
                                        {
                                            return rr == 60;
                                        });
            if (it != RefreshRates.end())
            {
                RefreshRate = *it;
            }
            else
            {
                auto it = std::find_if(RefreshRates.begin(), RefreshRates.end(),
                                       [](const UINT& rr)
                                        {
                                            return rr > 60;
                                        });
                if (it != RefreshRates.end())
                {
                    RefreshRate = *it;
                }
                else
                {
                    RefreshRate = RefreshRates.back();
                }
            }
        }
        std::vector<UINT> RefreshRates;
    };

     FcD3d9RenderDeviceImpl();
    ~FcD3d9RenderDeviceImpl();

    FcInt32 Initialize(HWND hWnd);
    void Terminate();

    FcUInt32 GetDisplayModeCount() { return displayModes_.size(); }
    bool GetDisplayMode(FcUInt32 iMode,
                        FcUInt32& width,
                        FcUInt32& height,
                        FcUInt32& defRefreshRate,
                        FcUInt32& nRefreshRates);
    bool GetDisplayModeRefreshRate(FcUInt32 iMode, FcUInt32 iRefreshRate, FcUInt32& refreshRate);

    LPDIRECT3D9 pD3D_               {nullptr};
    LPDIRECT3DDEVICE9 pD3DDevice_   {nullptr};
    HWND hTargetWnd_                {0};
    HWND hDesktopWnd_               {0};

    D3DCAPS9 d3DCaps_               {};
    int textureCaps_                {};
    char vtxShaderVersion_[4];
    char pixShaderVersion_[4];

    UINT iAdapter_                  {D3DADAPTER_DEFAULT};
    D3DDISPLAYMODE currentMode_;
    std::vector<D3DDISPLAYMODE_EX> displayModes_;
};

namespace fcinternal
{

struct ErrorCleaner
{
    ErrorCleaner() = default;

    virtual ~ErrorCleaner()
    {

    }

    void reset()
    {
        fOn_ = false;
    }

protected:
    bool fOn_ {true};
};

struct D3DErrorCleaner : public ErrorCleaner
{
    D3DErrorCleaner() : ErrorCleaner() {}

    ~D3DErrorCleaner()
    {
        if (fOn_)
        {
            FcReleaseCom(pD3D_);
            pD3D_ = nullptr;
        }
    }

    LPDIRECT3D9 getD3D() { return pD3D_; }
    void setD3D(LPDIRECT3D9 pD3D) { pD3D_ = pD3D; }

private:
    LPDIRECT3D9 pD3D_ {nullptr};

};

struct D3DDeviceErrorCleaner : public ErrorCleaner
{
    D3DDeviceErrorCleaner() : ErrorCleaner() {}

    ~D3DDeviceErrorCleaner()
    {
        if (fOn_)
        {
            FcReleaseCom(pD3DDevice_);
            pD3DDevice_ = nullptr;
        }
    }

    LPDIRECT3DDEVICE9 getD3DDevice() { return pD3DDevice_; }
    void setD3D(LPDIRECT3DDEVICE9 pD3DDevice) { pD3DDevice_ = pD3DDevice; }

private:
    LPDIRECT3DDEVICE9 pD3DDevice_   {nullptr};

};
} // end of fcinternal

//=============================================================================
// FcD3d9RenderDeviceImpl implementation

//-----------------------------------------------------------------------------
FcD3d9RenderDeviceImpl::FcD3d9RenderDeviceImpl() = default;

//-----------------------------------------------------------------------------
FcD3d9RenderDeviceImpl::~FcD3d9RenderDeviceImpl()
{
    Terminate();
}
//-----------------------------------------------------------------------------
void FcD3d9RenderDeviceImpl::Terminate()
{
    FcReleaseCom(pD3DDevice_);
    FcReleaseCom(pD3D_);
}
//-----------------------------------------------------------------------------

FcInt32 FcD3d9RenderDeviceImpl::Initialize(HWND hWnd)
{
    FC_ASSERT(hWnd);
    hTargetWnd_ = hWnd;

    fcinternal::D3DErrorCleaner result;
    result.setD3D(::Direct3DCreate9(D3D_SDK_VERSION));
    if (!result.getD3D())
    {
        return kFCD3D9RENDERDEVICE_ERRC_FAILED_D3D;
    }

    D3DADAPTER_IDENTIFIER9 d3DAdapterIdentifier {};
    HRESULT hr = result.getD3D()->GetAdapterIdentifier(iAdapter_, 0, &d3DAdapterIdentifier);

    FC_ASSERT (D3DERR_INVALIDCALL != hr); // Verify arguments
#ifdef FCD3D9RENDERDEVICE__PRINT_DEVICE_CAPS
    {
        printf("Driver: %s\n", d3DAdapterIdentifier.Driver);
        printf("Description: %s\n", d3DAdapterIdentifier.Description);
        printf("Version: %d.%d.%d.%d\n",
            HIWORD(d3DAdapterIdentifier.DriverVersion.QuadPart),
            (*((WORD*)&(d3DAdapterIdentifier.DriverVersion.QuadPart)+2)),
            HIWORD(d3DAdapterIdentifier.DriverVersion.u.LowPart),
            LOWORD(d3DAdapterIdentifier.DriverVersion.LowPart));
    }
#endif
    hr = result.getD3D()->GetDeviceCaps(iAdapter_, D3DDEVTYPE_HAL, &d3DCaps_);
    if (D3D_OK != hr)
    {
        return kFCD3D9RENDERDEVICE_ERRC_FAILED_GETCAPS;
    }
    if ((d3DCaps_.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0)
    {
        return kFCD3D9RENDERDEVICE_ERRC_NOTnL;
    }

    // Shaders support
    DWORD vsVersion = d3DCaps_.VertexShaderVersion;
    auto defineVtxShaderVersion = [vsVersion](char* pOut)
    {
        if (vsVersion >= D3DVS_VERSION(1, 1))
        {
            strcpy(pOut, "1.1");
        }
        if (vsVersion >= D3DVS_VERSION(2, 0))
        {
            strcpy(pOut, "2.0");
        }
        if (vsVersion >= D3DVS_VERSION(3, 0))
        {
            strcpy(pOut, "3.0");
        }
    };

    defineVtxShaderVersion(vtxShaderVersion_);

     DWORD psVersion = d3DCaps_.PixelShaderVersion;
    auto definePixhaderVersion = [psVersion](char* pOut)
    {
        if (psVersion >= D3DPS_VERSION(1, 1))
        {
            strcpy(pOut, "1.1");
        }
        if (psVersion >= D3DPS_VERSION(1, 2))
        {
            strcpy(pOut, "1.2");
        }
        if (psVersion >= D3DPS_VERSION(1, 3))
        {
            strcpy(pOut, "1.3");
        }
        if (psVersion >= D3DPS_VERSION(1, 4))
        {
            strcpy(pOut, "1.4");
        }
        if (psVersion >= D3DPS_VERSION(2, 0))
        {
            strcpy(pOut, "2.0");
        }
        if (psVersion >= D3DPS_VERSION(3, 0))
        {
            strcpy(pOut, "3.0");
        }
    };

    definePixhaderVersion(pixShaderVersion_);
#ifdef FCD3D9RENDERDEVICE__PRINT_DEVICE_CAPS
    printf("%s vertex shader supported: \n", vtxShaderVersion_);
    printf("%s pixel shader supported: \n", pixShaderVersion_);
#endif

    // texture caps
    if ( (d3DCaps_.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP_POW2) == 0 )
    {
        textureCaps_ |= kTexCaps_VolumeMap_NonPow2;
#ifdef FCD3D9RENDERDEVICE__PRINT_DEVICE_CAPS
        printf("3D Textures NONPOW2 supported!\n");
#endif
    }

    if ( (d3DCaps_.TextureCaps & D3DPTEXTURECAPS_NOPROJECTEDBUMPENV) == 0 )
    {
        textureCaps_ |= kTexCaps_ProjBumpEnv;
    }

    D3DDISPLAYMODE d3dDisplayModeCurrent{};

    if ( SUCCEEDED(result.getD3D()->GetAdapterDisplayMode(iAdapter_, &d3dDisplayModeCurrent) ))
    {
#ifdef FCD3D9RENDERDEVICE__PRINT_DEVICE_CAPS
        printf("Desktop Mode: %dx%d @ %d\n",
            d3dDisplayModeCurrent.Width,
            d3dDisplayModeCurrent.Height,
            d3dDisplayModeCurrent.RefreshRate);
#endif

        currentMode_.Width          = d3dDisplayModeCurrent.Width;
        currentMode_.Height         = d3dDisplayModeCurrent.Height;
        currentMode_.RefreshRate    = d3dDisplayModeCurrent.RefreshRate;
        currentMode_.Format         = d3dDisplayModeCurrent.Format;
        hDesktopWnd_ = ::GetDesktopWindow();
    }

    int nAdapterModeCount = result.getD3D()->GetAdapterModeCount(iAdapter_, D3DFMT_X8R8G8B8);

    for ( int m_i = 0; m_i < nAdapterModeCount; ++m_i )
    {
        D3DDISPLAYMODE d3dDisplayMode;

        if ( SUCCEEDED(result.getD3D()->EnumAdapterModes(iAdapter_, D3DFMT_X8R8G8B8, m_i, &d3dDisplayMode)) &&
            d3dDisplayMode.Format == D3DFMT_X8R8G8B8 )
        {
#ifdef FCD3D9RENDERDEVICE__PRINT_DEVICE_CAPS
        printf("Display Mode: %dx%d @ %d\n",
            d3dDisplayMode.Width,
            d3dDisplayMode.Height,
            d3dDisplayMode.RefreshRate);
#endif
            auto it = std::find_if(displayModes_.begin(), displayModes_.end(),
                                       [&d3dDisplayMode](const D3DDISPLAYMODE_EX& dm)
                                        {
                                            return  d3dDisplayMode.Width == dm.Width &&
                                                    d3dDisplayMode.Height == dm.Height;
                                        });
            if (it != displayModes_.end())
            {
                it->RefreshRates.push_back(d3dDisplayMode.RefreshRate);
            }
            else
            {
                displayModes_.push_back(d3dDisplayMode);
            }
        }

    }

    // define default refresh rate and sort
    for (FcUInt32 m_i = 0; m_i < displayModes_.size(); ++m_i )
    {
        displayModes_[m_i].sortRefreshRate();
    }

    pD3D_ = result.getD3D();
    result.reset();
    return kFCD3D9RENDERDEVICE_ERRC_OK;
}


//-----------------------------------------------------------------------------
bool FcD3d9RenderDeviceImpl::GetDisplayMode(FcUInt32 iMode, FcUInt32& width, FcUInt32& height, FcUInt32& defRefreshRate, FcUInt32& nRefreshRates)
{
    if (iMode >= displayModes_.size())
        return false;

    D3DDISPLAYMODE_EX& displayMode = displayModes_[iMode];
    width = displayMode.Width;
    height = displayMode.Height;
    defRefreshRate = displayMode.RefreshRate;
    nRefreshRates = displayMode.RefreshRates.size();
    return true;
}

//-----------------------------------------------------------------------------
bool FcD3d9RenderDeviceImpl::GetDisplayModeRefreshRate(FcUInt32 iMode, FcUInt32 iRefreshRate, FcUInt32& refreshRate)
{
     if (iMode >= displayModes_.size())
        return false;

    D3DDISPLAYMODE_EX& displayMode = displayModes_[iMode];
    if (iRefreshRate >= displayMode.RefreshRates.size())
        return false;

    refreshRate = displayMode.RefreshRates[iRefreshRate];
    return true;
}

//=============================================================================
// FcD3d9RenderDevice
//-----------------------------------------------------------------------------
FcD3d9RenderDevice::FcD3d9RenderDevice()
{
    pImpl_ = new FcD3d9RenderDeviceImpl();
}

//-----------------------------------------------------------------------------
FcD3d9RenderDevice::~FcD3d9RenderDevice()
{
    FC_ASSERT(pImpl_);
    delete pImpl_;
}
//-----------------------------------------------------------------------------
void FcD3d9RenderDevice::Terminate()
{
    FC_ASSERT(pImpl_);
    return pImpl_->Terminate();
}
//-----------------------------------------------------------------------------

FcInt32 FcD3d9RenderDevice::Initialize(HWND hWnd)
{
    FC_ASSERT(pImpl_);
    return pImpl_->Initialize(hWnd);
}

//-----------------------------------------------------------------------------
FcUInt32 FcD3d9RenderDevice::GetDisplayModeCount()
{
    FC_ASSERT(pImpl_);
    return pImpl_->GetDisplayModeCount();
}
//-----------------------------------------------------------------------------
bool FcD3d9RenderDevice::GetDisplayMode(FcUInt32 iMode, FcUInt32& width, FcUInt32& height, FcUInt32& defRefreshRate, FcUInt32& nRefreshRates)
{
    FC_ASSERT(pImpl_);
    return pImpl_->GetDisplayMode(iMode, width, height, defRefreshRate, nRefreshRates);
}

//-----------------------------------------------------------------------------
bool FcD3d9RenderDevice::GetDisplayModeRefreshRate(FcUInt32 iMode, FcUInt32 iRefreshRate, FcUInt32& refreshRate)
{
    FC_ASSERT(pImpl_);
    return pImpl_->GetDisplayModeRefreshRate(iMode, iRefreshRate, refreshRate);
}

