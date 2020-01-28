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

// Custom effects using pixel shaders should use HLSL helper functions defined in
// d2d1effecthelpers.hlsli to make use of effect shader linking.
#define D2D_INPUT_COUNT 0           // The pixel shader is a source and does not take inputs.
#define D2D_REQUIRES_SCENE_POSITION // The pixel shader requires the SCENE_POSITION input.

// Note that the custom build step must provide the correct path to find d2d1effecthelpers.hlsli when calling fxc.exe.
#include "d2d1effecthelpers.hlsli"

cbuffer constants : register(b0)
{
    float dpi : packoffset(c0.x);
    float2 center : packoffset(c0.y);
    float initialWavelength : packoffset(c0.w);
    float wavelengthHalvingDistance : packoffset (c1.x);
    float whiteLevelMultiplier : packoffset (c1.y);
};

D2D_PS_ENTRY(main)
{
    float PI = 3.141592653589;
    float2 pos = D2DGetScenePosition().xy;

    float dist = sqrt(pow(pos.x - center.x, 2) + pow(pos.y - center.y, 2)); // Euclidean distance

    // As distance increases, also increase frequency of sine wave: doubles every halving distance.
    float multiplier = pow(2, dist / wavelengthHalvingDistance);
    float val = sin(1 / initialWavelength * dist * 2 * PI * multiplier);

    val = (val + 1.0f) / 2.0f;   // Normalize from 0 to 1.
    val = pow(val, 2.2f);        // Apply sRGB gamma to produce perceptually uniform gradient.
    val *= whiteLevelMultiplier; // Scale to desired white level (CCCS).

    float4 color = { val, val, val, 1.0f };

    return color;
}