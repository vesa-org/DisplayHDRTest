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

#include "pch.h"
#include <initguid.h>
#include "BandedGradientEffect.h"

#define XML(X) TEXT(#X)

BandedGradientEffect::BandedGradientEffect() :
    m_refCount(1),
    m_constants{}
{
}

HRESULT __stdcall BandedGradientEffect::CreateBandedGradientImpl(_Outptr_ IUnknown** ppEffectImpl)
{
    // Since the object's refcount is initialized to 1, we don't need to AddRef here.
    *ppEffectImpl = static_cast<ID2D1EffectImpl*>(new (std::nothrow) BandedGradientEffect());

    if (*ppEffectImpl == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    else
    {
        return S_OK;
    }
}

HRESULT BandedGradientEffect::Register(_In_ ID2D1Factory1* pFactory)
{
    // The inspectable metadata of an effect is defined in XML. This can be passed in from an external source
    // as well, however for simplicity we just inline the XML.
    PCWSTR pszXml =
        XML(
            <?xml version='1.0'?>
            <Effect>
                <!-- System Properties -->
                <Property name='DisplayName' type='string' value='BandedGradient'/>
                <Property name='Author' type='string' value='Microsoft Corporation'/>
                <Property name='Category' type='string' value='Source'/>
                <Property name='Description' type='string' value='Renders gray gradients at varying bit depths'/>
                <Inputs />
                <!-- Custom Properties go here -->
                <Property name = 'OutputSize' type = 'vector2'>
                    <Property name = 'DisplayName' type = 'string' value = 'Output Size (Pixels)'/>
                    <Property name = 'Default' type = 'vector2' value = '(0.0, 0.0)'/>
                </Property>
            </Effect>
            );

    const D2D1_PROPERTY_BINDING bindings[] =
    {
        D2D1_VALUE_TYPE_BINDING(L"OutputSize", &SetOutputSize, &GetOutputSize),
    };

    // This registers the effect with the factory, which will make the effect
    // instantiatable.
    return pFactory->RegisterEffectFromString(
        CLSID_CustomBandedGradientEffect,
        pszXml,
        bindings,
        ARRAYSIZE(bindings),
        CreateBandedGradientImpl
        );
}

IFACEMETHODIMP BandedGradientEffect::Initialize(
    _In_ ID2D1EffectContext* pEffectContext,
    _In_ ID2D1TransformGraph* pTransformGraph
    )
{
    // To maintain consistency across different DPIs, this effect needs to cover more pixels at
    // higher than normal DPIs. The context is saved here so the effect can later retrieve the DPI.
    m_effectContext = pEffectContext;

    byte* data = nullptr;
    UINT size = 0;

    HRESULT hr = S_OK;

    try
    {
        DX::ReadDataFromFile(L"BandedGradientEffect.cso", &data, &size);
    }
    catch (std::exception)
    {
        // Most likely is caused by a missing or invalid file.
        hr = D2DERR_FILE_NOT_FOUND;
    }

    if (SUCCEEDED(hr))
    {
        hr = pEffectContext->LoadPixelShader(GUID_BandedGradientPixelShader, data, size);
    }

    // This loads the shader into the Direct2D image effects system and associates it with the GUID passed in.
    // If this method is called more than once (say by other instances of the effect) with the same GUID,
    // the system will simply do nothing, ensuring that only one instance of a shader is stored regardless of how
    // many time it is used.
    if (SUCCEEDED(hr))
    {
        // The graph consists of a single transform. In fact, this class is the transform,
        // reducing the complexity of implementing an effect when all we need to
        // do is use a single pixel shader.
        hr = pTransformGraph->SetSingleTransformNode(this);
    }

    return hr;
}

HRESULT BandedGradientEffect::UpdateConstants()
{
    // Update the DPI if it has changed. This allows the effect to scale across different DPIs automatically.
    m_effectContext->GetDpi(&m_dpi, &m_dpi);
    m_constants.dpi = m_dpi;

    return m_drawInfo->SetPixelShaderConstantBuffer(reinterpret_cast<BYTE*>(&m_constants), sizeof(m_constants));
}

IFACEMETHODIMP BandedGradientEffect::PrepareForRender(D2D1_CHANGE_TYPE changeType)
{
    return UpdateConstants();
}

// SetGraph is only called when the number of inputs changes. This never happens as we publish this effect
// as a single input effect.
IFACEMETHODIMP BandedGradientEffect::SetGraph(_In_ ID2D1TransformGraph* pGraph)
{
    return E_NOTIMPL;
}

// Called to assign a new render info class, which is used to inform D2D on
// how to set the state of the GPU.
IFACEMETHODIMP BandedGradientEffect::SetDrawInfo(_In_ ID2D1DrawInfo* pDrawInfo)
{
    m_drawInfo = pDrawInfo;

    return m_drawInfo->SetPixelShader(GUID_BandedGradientPixelShader);
}

// Calculates the mapping between the output and input rects.
IFACEMETHODIMP BandedGradientEffect::MapOutputRectToInputRects(
    _In_ const D2D1_RECT_L* pOutputRect,
    _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects,
    UINT32 inputRectCount
    ) const
{
    // This effect has no inputs.
    if (inputRectCount != 0)
    {
        return E_INVALIDARG;
    }

    // There should never be any inputs to map to.

    return S_OK;
}

IFACEMETHODIMP BandedGradientEffect::MapInputRectsToOutputRect(
    _In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputRects,
    _In_reads_(inputRectCount) CONST D2D1_RECT_L* pInputOpaqueSubRects,
    UINT32 inputRectCount,
    _Out_ D2D1_RECT_L* pOutputRect,
    _Out_ D2D1_RECT_L* pOutputOpaqueSubRect
    )
{
    // This effect has no inputs.
    if (inputRectCount != 0)
    {
        return E_INVALIDARG;
    }

    // The output is infinite and always opaque.
    *pOutputRect = { LONG_MIN, LONG_MIN, LONG_MAX, LONG_MAX };
    *pOutputOpaqueSubRect = { LONG_MIN, LONG_MIN, LONG_MAX, LONG_MAX };

    return S_OK;
}

IFACEMETHODIMP BandedGradientEffect::MapInvalidRect(
    UINT32 inputIndex,
    D2D1_RECT_L invalidInputRect,
    _Out_ D2D1_RECT_L* pInvalidOutputRect
    ) const
{
    HRESULT hr = S_OK;

    // Indicate that the entire output may be invalid.
    *pInvalidOutputRect = m_inputRect;

    return hr;
}

IFACEMETHODIMP_(UINT32) BandedGradientEffect::GetInputCount() const
{
    return 0;
}

// D2D ensures that that effects are only referenced from one thread at a time.
// To improve performance, we simply increment/decrement our reference count
// rather than use atomic InterlockedIncrement()/InterlockedDecrement() functions.
IFACEMETHODIMP_(ULONG) BandedGradientEffect::AddRef()
{
    m_refCount++;
    return m_refCount;
}

IFACEMETHODIMP_(ULONG) BandedGradientEffect::Release()
{
    m_refCount--;

    if (m_refCount == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return m_refCount;
    }
}

// This enables the stack of parent interfaces to be queried. In the instance
// of the BandedGradient interface, this method simply enables the developer
// to cast a BandedGradient instance to an ID2D1EffectImpl or IUnknown instance.
IFACEMETHODIMP BandedGradientEffect::QueryInterface(
    _In_ REFIID riid,
    _Outptr_ void** ppOutput
    )
{
    *ppOutput = nullptr;
    HRESULT hr = S_OK;

    if (riid == __uuidof(ID2D1EffectImpl))
    {
        *ppOutput = reinterpret_cast<ID2D1EffectImpl*>(this);
    }
    else if (riid == __uuidof(ID2D1DrawTransform))
    {
        *ppOutput = static_cast<ID2D1DrawTransform*>(this);
    }
    else if (riid == __uuidof(ID2D1Transform))
    {
        *ppOutput = static_cast<ID2D1Transform*>(this);
    }
    else if (riid == __uuidof(ID2D1TransformNode))
    {
        *ppOutput = static_cast<ID2D1TransformNode*>(this);
    }
    else if (riid == __uuidof(IUnknown))
    {
        *ppOutput = this;
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    if (*ppOutput != nullptr)
    {
        AddRef();
    }

    return hr;
}

// Size in pixels.
HRESULT BandedGradientEffect::SetOutputSize(D2D1_POINT_2F size)
{
    m_constants.outputSize = size;
    return S_OK;
}

D2D1_POINT_2F BandedGradientEffect::GetOutputSize() const
{
    return m_constants.outputSize;
}
