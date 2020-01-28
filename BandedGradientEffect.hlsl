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
    float2 outputSize : packoffset(c0.y);
};

D2D_PS_ENTRY(main)
{
	float2 posScene = D2DGetScenePosition().xy;
    float2 pos = float2(posScene.x / outputSize.x, posScene.y / outputSize.y);

	float c = pos.x;				// gradient from zero to 12.5% on right hand side
	c = pow(c,0.45454);				// preshape with gamma of 2.2 for now.
	c = c*0.25;						// use only bottom quarter of range

	if (pos.y < 0.2f )							// top 20%
	{
		c = trunc( c*64.0f)/64.0f;				// quantize to 6-bit
	}

#ifdef SHOW_6PLUS2
	if (pos.y > 0.2f && pos.y < 0.4f)			// second 20%
	{
		c = trunc(c*256.0f) / 256.0f;			// quantize to 8-bit to avoid higher info

		float2 ij = floor(fmod(posScene, 2.0));	// 2x2 ordered dither
		float index = ij.x + 2.0*ij.y;			// unfold the index
		float4 m = step(abs( index - float4(0, 1, 2, 3)), 0.5) * float4(0.75, 0.25, 0.00, 0.50);
		float d = m.x + m.y + m.z + m.w;

/*		float2 n = floor(abs(posScene.xy ));	// uses verticals not checkerboard for 50% case
		float2 s = floor( frac( n / 2.0 ) * 2.0 );
		float f = (  2.0 * s.x + s.y  ) / 4.0;
		float d = (f - 0.375) * 1.0;
*/
		c = c + d/64.0;
		c = trunc( c*64.0f )/64.0f;				// quantize to 6-bit
	}
#endif

	if (pos.y > 0.4f && pos.y < 0.6f)			// middle 20%
	{
		c = trunc( c*256.0f)/256.0f;			// quantize to 8-bit
	}

	if (pos.y > 0.8f)							// bottom 20%
	{
		c = trunc( c*1024.0f)/1024.0f;			// quantize to 10-bit
	}

	c = pow(c,2.2f);

    float4 color = { c, c, c, 1.0f };

    return color;
}