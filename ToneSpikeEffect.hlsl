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
    float whiteLevelMultiplier : packoffset (c1.y);			// actual nits to tone map to
};

// SMPTE ST 2084 profile (PQ:Preceptual Quantizer):
float Apply2084(float L)
{
	float m1 = 2610.0 / 4096.0 / 4;
	float m2 = 2523.0 / 4096.0 * 128;
	float c1 = 3424.0 / 4096.0;
	float c2 = 2413.0 / 4096.0 * 32;
	float c3 = 2392.0 / 4096.0 * 32;
	float Lp = pow(L, m1);
	return pow((c1 + c2 * Lp) / (1 + c3 * Lp), m2);
}

float Remove2084(float N)
{
	float m1 = 2610.0 / 4096.0 / 4;
	float m2 = 2523.0 / 4096.0 * 128;
	float c1 = 3424.0 / 4096.0;
	float c2 = 2413.0 / 4096.0 * 32;
	float c3 = 2392.0 / 4096.0 * 32;
	float Np = pow(N, 1 / m2);
	float num = Np - c1;
	if (num < 0) num = 0.0f;
	return pow(num / (c2 - c3 * Np), 1 / m1);
}

float ACESFilm(float x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float shoulder(float p, float x)
{
	float y, k;

	k = 1.0 / (p - 1.0);
	y = x * (k + 1.0) / (k + x);		// like fewerda

	return y;
}

// p = contentMax/panelCapability
float profile(float p, float input)
{
	float output;

	if (input > p)
	{
		output = 1.0f;					// hard clip
		return output;
	}

	// get shift s from excel curve fit
	float s = 1 - log(p)*0.165f;

	// for inverse tone mapping use different shift
	if ( p < 1 )
		s = p*0.7f;
//		s = 1 - log(1/p)*0.2;

	// compute tone curve linear slope
	float m = 1.0f / (p - s);

	// if below the threshold
	if ( input <= s )
		output = input;		// do nothing
	else					// tone map or ITM
		output = shoulder((p - s) / (1.f - s), m * (input - s)) * (1.f - s) + s;

	return output;
}

D2D_PS_ENTRY(main)
{
    float PI = 3.141592653589f;
    float2 pos = D2DGetScenePosition().xy;				// units of pixels

	float rArea = sqrt(center.x * center.y*4./PI);

	float2 del = pos - center;
    float r = length( del );							// Euclidean distance
	float theta = atan2(del.y, -del.x) + PI;

	float x = 1.12f* (rArea - r) / rArea;				// from perimeter to center
	if ( sin(theta*48.f) > 0.0f)
		x -= 10.f / 255.0f;

	float val = saturate( x );
	val = Remove2084(val);								// apply EOTF
	
	if (pos.y > center.y)								// on bottom half of screen,
	{
		// p is ContentPeak over DisplayCapability
		float p = 10000.f/whiteLevelMultiplier;

		val = val*10000.f/whiteLevelMultiplier;			// normalize by display capability
		val = profile(p, val);							// tone map
		val = val*whiteLevelMultiplier/10000.f;			// un-normalize back into nits
	}

	float c = val * 10000.0f / 80.0f;					// for float16 CCCS range

	float4 color = { c, c*0.5f, 0.f, 1.0f };

    return color;
}