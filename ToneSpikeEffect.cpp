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
#include "ToneSpikeEffect.h"

#define XML(X) TEXT(#X)

ToneSpikeEffect::ToneSpikeEffect() :
    m_refCount(1)
{
    m_constants.center = { 0.0f, 0.0f };
    m_constants.initialWavelength = 10.0f;
    m_constants.wavelengthHalvingDistance = 100.0f;
    m_constants.whiteLevelMultiplier = 1.0f;
}

HRESULT __stdcall ToneSpikeEffect::CreateToneSpikeImpl(_Outptr_ IUnknown** ppEffectImpl)
{
    // Since the object's refcount is initialized to 1, we don't need to AddRef here.
    *ppEffectImpl = static_cast<ID2D1EffectImpl*>(new (std::nothrow) ToneSpikeEffect());

    if (*ppEffectImpl == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    else
    {
        return S_OK;
    }
}

HRESULT ToneSpikeEffect::Register(_In_ ID2D1Factory1* pFactory)
{
    // The inspectable metadata of an effect is defined in XML. This can be passed in from an external source
    // as well, however for simplicity we just inline the XML.
    PCWSTR pszXml =
        XML(
            <?xml version='1.0'?>
            <Effect>
                <!-- System Properties -->
                <Property name='DisplayName' type='string' value='SineSweep'/>
                <Property name='Author' type='string' value='Microsoft Corporation'/>
                <Property name='Category' type='string' value='Source'/>
                <Property name='Description' type='string' value='Generates a heightmap of a sinewave based on distance from origin'/>
                <Inputs />
                <!-- Custom Properties go here -->
                <Property name = 'WavelengthHalvingDistance' type = 'float'>
                    <Property name = 'DisplayName' type = 'string' value = 'Wavelength Halving Distance'/>
                    <Property name = 'Min' type = 'float' value = '0.1'/>
                    <Property name = 'Max' type = 'float' value = '1000.0'/>
                    <Property name = 'Default' type = 'float' value = '100.0'/>
                </Property>
                <Property name = 'InitialWavelength' type = 'float'>
                    <Property name = 'DisplayName' type = 'string' value = 'Initial Wavelength'/>
                    <Property name = 'Min' type = 'float' value = '0.1'/>
                    <Property name = 'Max' type = 'float' value = '1000.0'/>
                    <Property name = 'Default' type = 'float' value = '10.0'/>
                </Property>
                <Property name = 'Center' type = 'vector2'>
                    <Property name = 'DisplayName' type = 'string' value = 'Center'/>
                    <Property name = 'Default' type = 'vector2' value = '(0.0, 0.0)'/>
                </Property>
                <Property name = 'WhiteLevelMultiplier' type = 'float'>
                    <Property name = 'DisplayName' type = 'string' value = 'White Level Multiplier'/>
                    <Property name = 'Min' type = 'float' value = '1.0'/>
                    <Property name = 'Max' type = 'float' value = '6.0'/>
                    <Property name = 'Default' type = 'float' value = '1.0'/>
                </Property>
            </Effect>
            );

    const D2D1_PROPERTY_BINDING bindings[] =
    {
        D2D1_VALUE_TYPE_BINDING(L"WavelengthHalvingDistance", &SetWavelengthHalvingDistance, &GetWavelengthHalvingDistance),
        D2D1_VALUE_TYPE_BINDING(L"InitialWavelength", &SetInitialWavelength, &GetInitialWavelength),
        D2D1_VALUE_TYPE_BINDING(L"Center", &SetCenter, &GetCenter),
        D2D1_VALUE_TYPE_BINDING(L"WhiteLevelMultiplier", &SetWhiteLevelMultiplier, &GetWhiteLevelMultiplier),
    };

    // This registers the effect with the factory, which will make the effect
    // instantiatable.
    return pFactory->RegisterEffectFromString(
        CLSID_CustomToneSpikeEffect,
        pszXml,
        bindings,
        ARRAYSIZE(bindings),
        CreateToneSpikeImpl
        );
}

IFACEMETHODIMP ToneSpikeEffect::Initialize(
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
        DX::ReadDataFromFile(DX::GetAbsolutePath(std::wstring(L"ToneSpikeEffect.cso")).c_str(), &data, &size);
    }
    catch (std::exception)
    {
        // Most likely is caused by a missing or invalid file.
        hr = D2DERR_FILE_NOT_FOUND;
    }

    if (SUCCEEDED(hr))
    {
        hr = pEffectContext->LoadPixelShader(GUID_ToneSpikePixelShader, data, size);
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

HRESULT ToneSpikeEffect::UpdateConstants()
{
    // Update the DPI if it has changed. This allows the effect to scale across different DPIs automatically.
    m_effectContext->GetDpi(&m_dpi, &m_dpi);
    m_constants.dpi = m_dpi;

    return m_drawInfo->SetPixelShaderConstantBuffer(reinterpret_cast<BYTE*>(&m_constants), sizeof(m_constants));
}

IFACEMETHODIMP ToneSpikeEffect::PrepareForRender(D2D1_CHANGE_TYPE changeType)
{
    return UpdateConstants();
}

// SetGraph is only called when the number of inputs changes. This never happens as we publish this effect
// as a single input effect.
IFACEMETHODIMP ToneSpikeEffect::SetGraph(_In_ ID2D1TransformGraph* pGraph)
{
    return E_NOTIMPL;
}

// Called to assign a new render info class, which is used to inform D2D on
// how to set the state of the GPU.
IFACEMETHODIMP ToneSpikeEffect::SetDrawInfo(_In_ ID2D1DrawInfo* pDrawInfo)
{
    m_drawInfo = pDrawInfo;

    return m_drawInfo->SetPixelShader(GUID_ToneSpikePixelShader);
}

// Calculates the mapping between the output and input rects.
IFACEMETHODIMP ToneSpikeEffect::MapOutputRectToInputRects(
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

IFACEMETHODIMP ToneSpikeEffect::MapInputRectsToOutputRect(
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

IFACEMETHODIMP ToneSpikeEffect::MapInvalidRect(
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

IFACEMETHODIMP_(UINT32) ToneSpikeEffect::GetInputCount() const
{
    return 0;
}

// D2D ensures that that effects are only referenced from one thread at a time.
// To improve performance, we simply increment/decrement our reference count
// rather than use atomic InterlockedIncrement()/InterlockedDecrement() functions.
IFACEMETHODIMP_(ULONG) ToneSpikeEffect::AddRef()
{
    m_refCount++;
    return m_refCount;
}

IFACEMETHODIMP_(ULONG) ToneSpikeEffect::Release()
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
// of the SineSweep interface, this method simply enables the developer
// to cast a SineSweep instance to an ID2D1EffectImpl or IUnknown instance.
IFACEMETHODIMP ToneSpikeEffect::QueryInterface(
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

HRESULT ToneSpikeEffect::SetCenter(D2D1_POINT_2F center)
{
    m_constants.center = center;

    return S_OK;
}

D2D1_POINT_2F ToneSpikeEffect::GetCenter() const
{
    return m_constants.center;
}

HRESULT ToneSpikeEffect::SetWavelengthHalvingDistance(float dist)
{
    m_constants.wavelengthHalvingDistance = dist;

    return S_OK;
}

float ToneSpikeEffect::GetWavelengthHalvingDistance() const
{
    return m_constants.wavelengthHalvingDistance;
}

HRESULT ToneSpikeEffect::SetInitialWavelength(float wavelength)
{
    m_constants.initialWavelength = wavelength;

    return S_OK;
}

float ToneSpikeEffect::GetInitialWavelength() const
{
    return m_constants.initialWavelength;
}

HRESULT ToneSpikeEffect::SetWhiteLevelMultiplier(float multiplier)
{
    m_constants.whiteLevelMultiplier = multiplier;

    return S_OK;
}

float ToneSpikeEffect::GetWhiteLevelMultiplier() const
{
    return m_constants.whiteLevelMultiplier;
}
