//********************************************************* 
// 
// Copyright (c) Microsoft. All rights reserved. 
// This code is licensed under the MIT License (MIT). 
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY 
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR 
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT. 
// 
//*********************************************************

#pragma once

namespace DX
{
    // Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
    interface IDeviceNotify
    {
        virtual void OnDeviceLost() = 0;
        virtual void OnDeviceRestored() = 0;
    };

    // Controls all the DirectX device resources.
    class DeviceResources
    {
    public:
        DeviceResources(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT,
                        DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
                        UINT backBufferCount = 2,
                        D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0);

        void CreateDeviceIndependentResources();
        void CreateDeviceResources();
        void CreateWindowSizeDependentResources();
        void SetWindow(HWND window, int width, int height);
        bool WindowSizeChanged(int width, int height);
        void SetDpi(float dpi);
        void HandleDeviceLost();
        void RegisterDeviceNotify(IDeviceNotify* deviceNotify) { m_deviceNotify = deviceNotify; }
        void Present();
        void ChangeBackBufferFormat(DXGI_FORMAT fmt);
		void SetMetadataNeutral();

        // The size of the render target, in pixels.
        RECT            GetOutputSize() const { return m_outputSize; }
        // The size of the render target, in dips.
        D2D1_RECT_F     GetLogicalSize() const { return m_logicalSize; }
        float           GetDpi() const { return m_dpi; }

        // Direct3D Accessors.
        ID3D11Device3*          GetD3DDevice() const                    { return m_d3dDevice.Get(); }
        ID3D11DeviceContext3*   GetD3DDeviceContext() const             { return m_d3dContext.Get(); }
        IDXGISwapChain4*        GetSwapChain() const                    { return m_swapChain.Get(); }
        D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const           { return m_d3dFeatureLevel; }
        ID3D11Texture2D*        GetRenderTarget() const                 { return m_renderTarget.Get(); }
        ID3D11Texture2D*        GetDepthStencil() const                 { return m_depthStencil.Get(); }
        ID3D11RenderTargetView* GetRenderTargetView() const             { return m_d3dRenderTargetView.Get(); }
        ID3D11DepthStencilView* GetDepthStencilView() const             { return m_d3dDepthStencilView.Get(); }
        DXGI_FORMAT             GetBackBufferFormat() const             { return m_backBufferFormat; }
        DXGI_FORMAT             GetDepthBufferFormat() const            { return m_depthBufferFormat; }
        D3D11_VIEWPORT          GetScreenViewport() const               { return m_screenViewport; }
        UINT                    GetBackBufferCount() const              { return m_backBufferCount; }

        // D2D Accessors.
        ID2D1Factory3*          GetD2DFactory() const                   { return m_d2dFactory.Get(); }
        ID2D1Device2*           GetD2DDevice() const                    { return m_d2dDevice.Get(); }
        ID2D1DeviceContext2*    GetD2DDeviceContext() const             { return m_d2dContext.Get(); }
        ID2D1Bitmap1*           GetD2DTargetBitmap() const              { return m_d2dTargetBitmap.Get(); }
        IDWriteFactory3*        GetDWriteFactory() const                { return m_dwriteFactory.Get(); }
        IWICImagingFactory2*    GetWicImagingFactory() const            { return m_wicFactory.Get(); }

        // Performance events
        void PIXBeginEvent(_In_z_ const wchar_t* name)
        {
            if (m_d3dAnnotation)
            {
                m_d3dAnnotation->BeginEvent(name);
            }
        }

        void PIXEndEvent()
        {
            if (m_d3dAnnotation)
            {
                m_d3dAnnotation->EndEvent();
            }
        }

        void PIXSetMarker(_In_z_ const wchar_t* name)
        {
            if (m_d3dAnnotation)
            {
                m_d3dAnnotation->SetMarker(name);
            }
        }

    private:
        void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
        void UpdateLogicalSize(RECT outputSize, float dpi);

        // Direct3D objects.
        Microsoft::WRL::ComPtr<ID3D11Device3>           m_d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext3>    m_d3dContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain4>         m_swapChain;
        Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_d3dAnnotation;

        // Direct3D rendering objects. Required for 3D.
        Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_renderTarget;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_depthStencil;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_d3dRenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_d3dDepthStencilView;
        D3D11_VIEWPORT                                  m_screenViewport;

        // Direct2D drawing components.
        Microsoft::WRL::ComPtr<ID2D1Factory3>           m_d2dFactory;
        Microsoft::WRL::ComPtr<ID2D1Device2>            m_d2dDevice;
        Microsoft::WRL::ComPtr<ID2D1DeviceContext2>     m_d2dContext;
        Microsoft::WRL::ComPtr<ID2D1Bitmap1>            m_d2dTargetBitmap;

        // DirectWrite drawing components.
        Microsoft::WRL::ComPtr<IDWriteFactory3>         m_dwriteFactory;
        Microsoft::WRL::ComPtr<IWICImagingFactory2>     m_wicFactory;

        // Direct3D properties.
        DXGI_FORMAT                                     m_backBufferFormat;
        DXGI_FORMAT                                     m_depthBufferFormat;
        UINT                                            m_backBufferCount;

        // Cached device properties.
        HWND                                            m_window;
        D3D_FEATURE_LEVEL                               m_d3dFeatureLevel;
        RECT                                            m_outputSize;
        D2D1_RECT_F                                     m_logicalSize;
        float                                           m_dpi;

        // The IDeviceNotify can be held directly as it owns the DeviceResources.
        IDeviceNotify*                                  m_deviceNotify;
    };
}