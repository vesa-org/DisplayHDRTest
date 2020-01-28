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
#include <iostream>
#include <math.h>
#include "basicmath.h"

using namespace std;


// White points for 2deg FoV
const float2 D5000White = float2(0.34567, 0.35850);		// D50
const float2 D6000White = float2(0.32168, 0.33767);		// D60
const float2 D6500White = float2(0.31271, 0.32902);		// D65	-assumed in Win10

// RGB gamut primaries for common spaces
const float2 primaryR_NTSC = float2(0.67, 0.33);		// NTSC 1953  uses D67 white
const float2 primaryG_NTSC = float2(0.21, 0.71);
const float2 primaryB_NTSC = float2(0.14, 0.08);

const float2 primaryR_SMPTEC = float2(0.630, 0.340);	// SMPTE C
const float2 primaryG_SMPTEC = float2(0.310, 0.595);
const float2 primaryB_SMPTEC = float2(0.155, 0.070);

const float2 primaryR_709 = float2(0.640, 0.330);		// 709, sRGB
const float2 primaryG_709 = float2(0.300, 0.600);
const float2 primaryB_709 = float2(0.150, 0.060);

const float2 primaryR_Adobe = float2(0.640, 0.330);		// Adobe RGB
const float2 primaryG_Adobe = float2(0.210, 0.710);
const float2 primaryB_Adobe = float2(0.150, 0.060);

const float2 primaryR_DCIP3 = float2(0.680, 0.32);		// DCI-P3
const float2 primaryG_DCIP3 = float2(0.265, 0.69);
const float2 primaryB_DCIP3 = float2(0.150, 0.06);

const float2 primaryR_2020 = float2(0.708, 0.292);		// Rec.2020 or Rec.2100
const float2 primaryG_2020 = float2(0.170, 0.797);
const float2 primaryB_2020 = float2(0.131, 0.046);

const float2 primaryR_ACES = float2(0.7347, 0.2653);	// ACES AP0  uses D60 white
const float2 primaryG_ACES = float2(0.0000, 1.0000);
const float2 primaryB_ACES = float2(0.0001,-0.0770);

// From Lindbloom -transposed
// These are all D65 spaces, except ACES
const float3x3 XYZ_to_SMPTECRGB = float3x3(
	 3.5053960, -1.0690722,  0.0563200,
	-1.7394894,  1.9778245, -0.1970226,
	-0.5439640,  0.0351722,  1.0502026
);
const float3x3 XYZ_to_709RGB = float3x3(
	 3.2404542, -.9692660, 0.0556434,
	-1.5371585, 1.8760108, -.2040259,
	-0.4985314, 0.0415560, 1.0572252
);
const float3x3 XYZ_to_AdobeRGB = float3x3(
	 2.0413690, -0.9692660,  0.0134474,
	-0.5649464,  1.8760108, -0.1183897,
	-0.3446944,  0.0415560,  1.0154096
);
const float3x3 XYZ_to_BT2020RGB = float3x3(
	 1.716650, -0.3556710, -0.2533660,
	-0.666684,  1.6164800,  0.0157681,
	 0.017640, -0.0427711,  0.9421030
);
const float3x3 BT2020RGB_to_XYZ = float3x3(
	 0.636958,  0.144617,  0.168881,
	 0.262700,  0.677998,  0.059302,
	 0.000000,  0.028073,  1.060985
);
const float3x3 XYZtoLMS = float3x3(
	 0.400200,  0.707500, -0.080700,
	-0.228000,  1.150000,  0.061200,
	 0.000000,  0.000000,  0.918400
);
const float3x3 LMStoXYZ = float3x3(
	 1.850243, -1.138302,  0.238435,
	 0.366831,  0.643885, -0.0106734,
	 0.000000,  0.000000,  1.088850
);

const float3x3 XYZ_to_ACESRGB = float3x3(	// NOTE this technically asumes D6000 white point
	 1.0498110175, 0.0000000000, -0.0000974845,
	-0.4959030231, 1.3733130458,  0.0982400361,
	 0.0000000000, 0.0000000000,  0.9912520182
);


// pre-multiplied matrices for direct conversion
static const float3x3 mat709to2020 = float3x3 (
	0.6274040, 0.3292820, 0.0433136,
	0.0690970, 0.9195400, 0.0113612,
	0.0163916, 0.0880132, 0.8955950
);

static const float3x3 mat2020to709 = float3x3 (
	 1.660491, -0.587640, -0.0728517,
	-0.124550,  1.132900, -0.0083480,
	-0.018151, -0.100579,  1.1187300
);

static const float3x3 matRGBtoYCbCr = float3x3(
	0.299, -0.168736,  0.500,
	0.587, -0.331264, -0.418688,
	0.114,  0.500, -0.081312
);

static const float3x3 matYCbCrtoRGB = float3x3(
	1.0, 1.0, 1.0,
	0.0, -0.344136, 1.772,
	1.402, -0.714136, 0.0
);

// TODO: stealing namespace is bad
const float gamutVolumeLuv709   = 1487896;
const float gamutVolumeLuvAdobe = 1978981;
const float gamutVolumeLuvDCIP3 = 1959840;
const float gamutVolumeLuv2020  = 2665875;
const float gamutVooumeAcesCG   = 2090408;
const float gamutVolumeLuvHuman = 3000000;		// estimate
const float gamutVolumeLuvACES  = 4200943;

const float gamutVolumeLab709   = 820285;
const float gamutVolumeLabAdobe = 1195981;
const float gamutVolumeLabDCIP3 = 1230953;
const float gamutVolumeLab2020  = 1854871;
const float gamutVolumeAcesCG   = 2775907;
const float gamutVolimeLabHuman = 2381085;		// per Bruce Lindbloom
const float gamutVolumeLabACES  = 4306401;

// Gamma ramps and encoding transfer functions
//
// Orthogonal to color space though usually tightly coupled.  For instance, sRGB is both a
// color space (defined by three basis vectors and a white point) and a gamma ramp.  Gamma
// ramps are designed to reduce perceptual error when quantizing floats to integers with a
// limited number of bits.  More variation is needed in darker colors because our eyes are
// more sensitive in the dark.  The way the curve helps is that it spreads out dark values
// across more code words allowing for more variation.  Likewise, bright values are merged
// together into fewer code words allowing for less variation.
//
// The sRGB curve is not a true gamma ramp but rather a piecewise function comprising a linear
// section and a power function.  When sRGB-encoded colors are passed to an LCD monitor, they
// look correct on screen because the monitor expects the colors to be encoded with sRGB, and it
// removes the sRGB curve to linearize the values.  When textures are encoded with sRGB--as many
// are--the sRGB curve needs to be removed before involving the colors in linear mathematics such
// as physically based lighting.

float ApplySRGBCurve(float x)
{
	// Approximately pow(x, 1.0 / 2.2)
	return x < 0.0031308f ? 12.92f * x : 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
}
float3 ApplySRGBCurve(float3 c)				// vector version
{
	c.r = ApplySRGBCurve(c.r);
	c.g = ApplySRGBCurve(c.g);
	c.b = ApplySRGBCurve(c.b);

	return c;
}

float RemoveSRGBCurve(float x)
{
	// Approximately pow(x, 2.2)
	return x < 0.04045f ? x / 12.92f : pow((x + 0.055f) / 1.055f, 2.4f);
}
float3 RemoveSRGBCurve(float3 c)			// vector version
{
	c.r = RemoveSRGBCurve(c.r);
	c.g = RemoveSRGBCurve(c.g);
	c.b = RemoveSRGBCurve(c.b);

	return c;
}

// These functions avoid pow() to efficiently approximate sRGB with an error < 0.4%.
float ApplySRGBCurve_Fast(float x)
{
	return x < 0.0031308f ? 12.92f * x : 1.13005f * sqrt(x - 0.00228f) - 0.13448f * x + 0.005719f;
}

float RemoveSRGBCurve_Fast(float x)
{
	return x < 0.04045f ? x / 12.92f : -7.43605f * x - 31.24297f * sqrt(-0.53792f * x + 1.279924f) + 35.34864f;
}

// The OETF recommended for content shown on HDTVs.  This "gamma ramp" may increase contrast as
// appropriate for viewing in a dark environment.  Always use this curve with Limited RGB as it is
// used in conjunction with HDTVs.
float ApplyRec709Curve(float x)
{
	return x < 0.0181f ? 4.5f * x : 1.0993f * pow(x, 0.45f) - 0.0993f;
}

float RemoveRec709Curve(float x)
{
	return x < 0.08145f ? x / 4.5f : pow((x + 0.0993f) / 1.0993f, 1.0f / 0.45f);
}

#if 0
float RemoveSRGB(float c)
{
	//	c = saturate(c);				// should guarantee unorm range
	if (c <= 0.04045)
		c = c / 12.92;
	else
	{
		c = (c + 0.055) / 1.055;
		c = pow(c, 2.4f);
	}
	return c;
}
#endif

#if 1
// SMPTE ST 2084 profile (PQ:Preceptual Quantizer):
float Apply2084(float L)
{
	float m1 = 2610.0 / 4096.0 / 4;
	float m2 = 2523.0 / 4096.0 * 128;
	float c1 = 3424.0 / 4096.0;
	float c2 = 2413.0 / 4096.0 * 32;
	float c3 = 2392.0 / 4096.0 * 32;
	float Lp = powf(L, m1);
	return powf((c1 + c2 * Lp) / (1 + c3 * Lp), m2);
}

float Remove2084(float N)
{
	float m1 = 2610.0 / 4096.0 / 4;
	float m2 = 2523.0 / 4096.0 * 128;
	float c1 = 3424.0 / 4096.0;
	float c2 = 2413.0 / 4096.0 * 32;
	float c3 = 2392.0 / 4096.0 * 32;
	float Np = powf(N, 1 / m2);
	float num = Np - c1;
	if (num < 0) num = 0.0f;
	return powf( num / (c2 - c3 * Np), 1 / m1);
}

#else
float Apply2084(float value)
{
	// value = saturate(value);       // guarantee unorm range
	const float c1 = 0.8359375;
	const float c2 = 18.8515625;
	const float c3 = 18.6875;
	float num = c1 + c2 * powf(value, 0.1593017578125);
	float den = 1 + c3 * powf(value, 0.1593017578125);
	return powf(num / den, 78.84375);
};

float Remove2084(float value)
{
	//	value = saturate(value);       // guarantee unorm range
	const float c1 = 0.8359375;
	const float c2 = 18.8515625;
	const float c3 = 18.6875;
	float num = powf(value, 0.012683313515655966) - c1;
	if (num < 0) num = 0.0f;
	float div = c2 - c3 * powf(value, 0.012683313515655966);
	return powf(num / div, 6.277394636015326);
};
#endif

float3 Apply2084(float3 c)
{
	c = saturate(c);
	return float3( Apply2084(c.x),
				   Apply2084(c.y),
				   Apply2084(c.z) );
}

float3 Remove2084(float3 c)
{
	return float3( Remove2084(c.x),
				   Remove2084(c.y),
				   Remove2084(c.z) );
}

float3 Rec709ToRec2020(float3 color)		// assuming D65 white
{
	static const float3x3 conversion =
	{
		0.627402, 0.329292, 0.043306,
		0.069095, 0.919544, 0.011360,
		0.016394, 0.088028, 0.895578
	};
	return mul(conversion, color);
}

float3 Rec2020ToRec709(float3 color)		// assuming D65 white
{
	static const float3x3 conversion =
	{
		 1.660496, -0.587656, -0.072840,
		-0.124547,  1.132895, -0.008348,
		-0.018154, -0.100597,  1.118751
	};
	return mul(conversion, color);
}

float3 RecDCIP3toRec2020(float3 color)		// assuming D65 white
{
	static const float3x3 conversion =
	{
		 0.75383472,  0.19860256,  0.04756273,
		 0.04574290,  0.94178025,  0.01247684,
		-0.00121051,  0.01760467,  0.98360584
	};
	return mul(conversion, color);
}

float3 Rec2020toDCIP3(float3 color)			// assuming D65 white
{
	static const float3x3 conversion =
	{
		 1.34357525, -0.28218550, -0.06138975,
		-0.06529575,  1.07578445, -0.01048870,
		 0.00282219, -0.01960177,  1.01677958
	};
	return mul(conversion, color);
}

float3 AdobeRGBtoRec2020(float3 color)		// assuming D65 white
{
	static const float3x3 conversion =
	{
		0.87733865,  0.07749581,  0.04516554,
		0.09662091,  0.89153109,  0.01184800,
		0.02292443,  0.04304395,  0.93403162
	};
	return mul(conversion, color);
}

float3 Rec2020toAdobeRGB(float3 color)		// assuming D65 white
{
	static const float3x3 conversion =
	{
		 1.15197208, -0.09750475, -0.05446733,
		-0.12454710,  1.13289511, -0.00834801,
		-0.02253383, -0.04981527,  1.07234910
	};
	return mul(conversion, color);
}

float3 Rec709toDCIP3(float3 RGB709)
{
	static const float3x3 ConvMat =
	{
		0.822458, 0.177542, 0.000000,
		0.033193, 0.966807, 0.000000,
		0.017085, 0.072410, 0.910505
	};
	return mul(ConvMat, RGB709);
}

float3 DCIP3toRec709(float3 RGB709)
{
	static const float3x3 ConvMat =
	{
		1.224947, -0.224947, 0.000000,
		-0.042056, 1.042056, 0.000000,
		-0.019641, -0.078651, 1.098291
	};
	return mul(ConvMat, RGB709);
}

// called to convert from CCCS to HDMI-friendly format e.g. on present/scan-out
float3 Linear709ToHDR10(float3 c)
{
	// Rotate from 709 to 2020 primaries
	c = mul(mat709to2020, c);
//	c = Rec709ToRec2020(c);

	// Divide by 125
	c = c * 0.008f;

	// Apply 2084 profile (unorm in and out)
	c = saturate(c);
	c.x = Apply2084(c.x);
	c.y = Apply2084(c.y);
	c.z = Apply2084(c.z);

	return c;
};

// called to convert HDR10 content into CCCS for composition
float3 HDR10ToLinear709(float3 c)
{
	// Remove 2084 profile resulting in photon linear
	c = saturate(c);
	c.x = Remove2084(c.x);
	c.y = Remove2084(c.y);
	c.z = Remove2084(c.z);

	// Multiply by 125
	c = c * 125.0f;

	// Rotate to 709 primaries
	c = mul(mat2020to709, c);

	return c;
};

float3 RGBToYCoCg( float3 RGB )
{
	float Y  = dot(RGB, float3( 1, 2,  1)) * 0.25f;
	float Co = dot(RGB, float3( 2, 0, -2)) * 0.25f + (0.5f * 256.0f / 255.0f);
	float Cg = dot(RGB, float3(-1, 2, -1)) * 0.25f + (0.5f * 256.0f / 255.0f);
	return float3(Y, Co, Cg);
}

float3 YCoCgToRGB(float3 YCoCg)
{
	float Y  = YCoCg.x;
	float Co = YCoCg.y - (0.5f * 256.0f / 255.0f);
	float Cg = YCoCg.z - (0.5f * 256.0f / 255.0f);
	float R = Y + Co - Cg;
	float G = Y + Cg;
	float B = Y - Co - Cg;
	return float3(R, G, B);
}

float3 RGBtoYCbCr( float3 rgb)
{
	float Y  = 0.299f * rgb.x + .587f * rgb.y + .114f * rgb.z; // Luminance
	float Cb = -.169f * rgb.x - .331f * rgb.y + .500f * rgb.z; // Chrominance Blue
	float Cr = 0.500f * rgb.x - .419f * rgb.y - .081f * rgb.z; // Chrominance Red
	return float3(Y, Cb + 128.f/255.f, Cr + 128.f/255.f);
}

float3 YCbCrtoRGB(float3 ycc)
{
	float3 c = ycc - float3(0., 128.f / 255.f, 128.f / 255.f);

	float R = c.x + 1.400f * c.z;
	float G = c.x - 0.343f * c.y - 0.711f * c.z;
	float B = c.x + 1.765f * c.y;
	return float3(R, G, B);
}

float2 uvtoxy(float2 uv)
{
	float2 xy;

	float d = (6.f*uv.u - 16.f*uv.v + 12.f);
	xy.x = 9.f*uv.u / d;
	xy.y = 4.f*uv.v / d;

	return xy;
}

float2 xytouv(float2 xy)
{
	float2 uv;
	
	float d = (-2.f*xy.x + 12.f*xy.y + 3.f);
	uv.u = 4.f*xy.x / d;
	uv.v = 9.f*xy.y / d;

	return uv;
}

// Convert from 1931 CIE Yxy Chromaticities to XYZ color space (Y = 1.0)
#if 1
float3 xytoXYZ(float2 xy, float Y)
{
	float3 XYZ = float3(0, Y, 0);	// default to black

									// Convert 1931 Yxy to XYZ where Y = 1.0
	if ( xy.x <= 1.0f && xy.x >= 0.0f && xy.y <= 1.0f && xy.y > 0.0f )
	{
		XYZ = float3(xy.x / xy.y, 1.0f, (1.0f - xy.x - xy.y) / xy.y);
		XYZ *= Y;
	}
	return XYZ;
}
#else
float3 xytoXYZ(float2 xy, float w = 1.f)
{
	return float3(xy.x, xy.y, 1.0f - xy.x - xy.y);
}
#endif

// Convert from 1931 CIE Yxy Chromaticities to linear 709 color space (Y = 1.0)
// to use with CCCS
float3 xytosRGB(float2 xy)
{
	float3 XYZ = xytoXYZ( xy, 1.0f );
	float3 rgb709 = XYZ * XYZ_to_709RGB; 		// matrix times vector
												//	float Y = rgb709.r*0.2126 + rgb709.g*0.7152 + rgb709.b*0.0722;
	float Y = std::max(rgb709.x, std::max(rgb709.y, rgb709.z));

	return rgb709 / Y;
}

float nitstoCCCS(float c)
{
	return c / 80.0f;
}


// Convert a color in CIE-xyY space to CIE-XYZ space.
float3 xyYtoXYZ(const float3& xyY)
{
    float3 ret;
    ret.x = xyY.z * xyY.x / xyY.y;
    ret.y = xyY.z;
    ret.z = xyY.z * (1.0f - xyY.x - xyY.y) / xyY.y;
    return ret;
}

// Helper for Lab conversions.
float f(float t)
{
    const float delta = 6.0f / 29.0f;
    if (t > pow(delta, 3.0f)) return pow(t, 1.0f / 3.0f);
    return t / (3.0f * delta * delta) + 4.0f / 29.0f;
}

// Helper for Lab conversions.
float f_inv(float t)
{
    const float delta = 6.0f / 29.0f;
    if (t > delta) return pow(t, 3.0f);
    return 3.0f * pow(delta, 2.0f) * (t - 4.0f / 29.0f);
}

// Convert a color in CIE-XYZ space to CIE-Lab space.
float3 XYZ_to_Lab(const float3& color_XYZ, const float3& white_XYZ)
{
    float3 ret;
    ret.x = 116.0f * f(color_XYZ.y / white_XYZ.y) - 16.0f;
    ret.y = 500.0f * (f(color_XYZ.x / white_XYZ.x) - f(color_XYZ.y / white_XYZ.y));
    ret.z = 200.0f * (f(color_XYZ.y / white_XYZ.y) - f(color_XYZ.z / white_XYZ.z));
    return ret;
}

// Convert a color in CIE-Lab space to CIE-XYZ space.
float3 Lab_to_XYZ(const float3& color_Lab, const float3& white_XYZ)
{
    float3 ret;
    ret.x = white_XYZ.x * f_inv((color_Lab.x + 16.0f) / 116.0f + color_Lab.y / 500.0f);
    ret.y = white_XYZ.y * f_inv((color_Lab.x + 16.0f) / 116.0f);
    ret.z = white_XYZ.z * f_inv((color_Lab.x + 16.0f) / 116.0f - color_Lab.z / 200.0f);
    return ret;
}

// Convert a color in CIE-XYZ space to CIE-Luv space.
float3 XYZ_to_Luv(const float3& color_XYZ, const float3& white_XYZ)
{
    float3 ret;
    const float delta = 6.0f / 29.0f;
    float Y_ratio = color_XYZ.y / white_XYZ.y;
    if (Y_ratio <= pow(delta, 3)) ret.x = pow(2.0f / delta, 3) * Y_ratio;
    else ret.x = 116.0f * pow(Y_ratio, 1.0f / 3.0f) - 16.0f;
    float u_prime = 4.0f * color_XYZ.x / (color_XYZ.x + 15.0f * color_XYZ.y + 3.0f * color_XYZ.z);
    float v_prime = 9.0f * color_XYZ.y / (color_XYZ.x + 15.0f * color_XYZ.y + 3.0f * color_XYZ.z);
    float u_prime_n = 4.0f * white_XYZ.x / (white_XYZ.x + 15.0f * white_XYZ.y + 3.0f * white_XYZ.z);
    float v_prime_n = 9.0f * white_XYZ.y / (white_XYZ.x + 15.0f * white_XYZ.y + 3.0f * white_XYZ.z);
    ret.y = 13.0f * ret.x * (u_prime - u_prime_n);
    ret.z = 13.0f * ret.x * (v_prime - v_prime_n);
    return ret;
}

// Convert a color in CIE-Luv space to CIE-XYZ space.
float3 Luv_to_XYZ(const float3& color_Luv, const float3& white_XYZ)
{
    float3 ret;
    const float delta = 6.0f / 29.0f;
    float u_prime_n = 4.0f * white_XYZ.x / (white_XYZ.x + 15.0f * white_XYZ.y + 3.0f * white_XYZ.z);
    float v_prime_n = 9.0f * white_XYZ.y / (white_XYZ.x + 15.0f * white_XYZ.y + 3.0f * white_XYZ.z);
    float u_prime = color_Luv.y / (13.0f * color_Luv.x) + u_prime_n;
    float v_prime = color_Luv.z / (13.0f * color_Luv.x) + v_prime_n;
    if (color_Luv.x <= 8.0f) ret.y = white_XYZ.y * color_Luv.x * pow(delta / 2.0f, 3.0f);
    else ret.y = white_XYZ.y * pow((color_Luv.x + 16.0f) / 116.0f, 3.0f);
    ret.x = ret.y * 9.0f * u_prime / (4.0f * v_prime);
    ret.z = ret.y * (12.0f - 3.0f * u_prime - 20.0f * v_prime) / (4.0f * v_prime);
    return ret;
}
// Construct a CIE-XYZ -> linear-RGB transformation matrix.
float3x3 Make_XYZ_to_RGB_Matrix(const float2& R, const float2& G, const float2& B, const float2& W_xy, const float &Y)
{
	float3x3 mat;
	mat._11 = R.x / R.y;
	mat._12 = G.x / G.y;
	mat._13 = B.x / B.y;
	mat._21 = mat._22 = mat._23 = 1.0f;
	mat._31 = (1.0f - R.x - R.y) / R.y;
	mat._32 = (1.0f - G.x - G.y) / G.y;
	mat._33 = (1.0f - B.x - B.y) / B.y;
	float3x3 matInv = inv(mat);
	float3 W_XYZ = xytoXYZ(W_xy, Y );
	float3 S = mul(matInv, W_XYZ);
	float3x3 mat3 = matInv;
	mat3._11 /= S.x;
	mat3._12 /= S.x;
	mat3._13 /= S.x;
	mat3._21 /= S.y;
	mat3._22 /= S.y;
	mat3._23 /= S.y;
	mat3._32 /= S.z;
	mat3._31 /= S.z;
	mat3._33 /= S.z;
	return mat3;
}

// Construct a linear-RGB -> CIE-XYZ transformation matrix.
float3x3 Make_RGB_to_XYZ_Matrix(const float2& R, const float2& G, const float2& B, const float2& W_xy, const float&Y )
{
    float3x3 mat = Make_XYZ_to_RGB_Matrix(R, G, B, W_xy, Y);
    return inv(mat);
}

void PrintUsage()
{
    cout << "Usage: " << "gamut.exe" << " r_x r_y g_x g_y b_x b_y [w_x] [w_y]" << endl;
    cout << "  r_[x,y]: chromaticity of red primary" << endl;
    cout << "  g_[x,y]: chromaticity of green primary" << endl;
    cout << "  b_[x,y]: chromaticity of blue primary" << endl;
    cout << "  w_[x,y]: chromaticity of white point (default: D65 <0.3127, 0.3290>)" << endl;
}


float gamutVolumeLab( const float2& red_xy, const float2 green_xy, const float2& blue_xy, const float2& white_xy )
{
	const float Y = 100.0;
	float3 white_XYZ = xytoXYZ(white_xy, Y );
	float3x3 RGBXYZ = Make_RGB_to_XYZ_Matrix(red_xy, green_xy, blue_xy, white_xy, Y);
	float3x3 XYZRGB = Make_XYZ_to_RGB_Matrix(red_xy, green_xy, blue_xy, white_xy, Y);

	unsigned long long hits = 0;
	for (int L = 0; L <= 100; L++)
	{
		for (int a = -200; a <= 200; a++)
		{
			for (int b = -200; b <= 200; b++)
			{
				float Ld = (float)L;
				float ad = (float)a;
				float bd = (float)b;
				float3 testColor_Lab{ Ld, ad, bd };
				float3 testColor_XYZ = Lab_to_XYZ(testColor_Lab, white_XYZ);
				float3 testColor_RGB = mul(XYZRGB, testColor_XYZ);
				if (testColor_RGB.x < 0.0f || testColor_RGB.x > 1.0f) continue;
				if (testColor_RGB.y < 0.0f || testColor_RGB.y > 1.0f) continue;
				if (testColor_RGB.z < 0.0f || testColor_RGB.z > 1.0f) continue;
				hits++;
			}
		}
	}
	return (float) hits;		// return volume
}

float gamutVolumeLuv(const float2& red_xy, const float2 green_xy, const float2& blue_xy, const float2& white_xy)
{
	const float Y = 100.0;
	float3 white_XYZ = xytoXYZ(white_xy, Y) ;
	float3x3 RGBXYZ = Make_RGB_to_XYZ_Matrix(red_xy, green_xy, blue_xy, white_xy, Y );
	float3x3 XYZRGB = Make_XYZ_to_RGB_Matrix(red_xy, green_xy, blue_xy, white_xy, Y );

	unsigned long long hits = 0;
	for (int L = 0; L <= 100; L++)
	{
		for (int u = -200; u <= 200; u++)
		{
			for (int v = -200; v <= 200; v++)
			{
				float Ld = (float)L;
				float ud = (float)u;
				float vd = (float)v;
				float3 testColor_Luv{ Ld, ud, vd };
				float3 testColor_XYZ = Luv_to_XYZ(testColor_Luv, white_XYZ);
				float3 testColor_RGB = mul(XYZRGB, testColor_XYZ);
				if (testColor_RGB.x < 0.0f || testColor_RGB.x > 1.0f) continue;
				if (testColor_RGB.y < 0.0f || testColor_RGB.y > 1.0f) continue;
				if (testColor_RGB.z < 0.0f || testColor_RGB.z > 1.0f) continue;
				hits++;
			}
		}
	}
	return (float) hits;		// return volume
}

#if 0
int main(int argc, char* argv[])
{
    float2 red_xy, green_xy, blue_xy, white_xy;
    if (argc == 7 || argc == 9)
    {
        red_xy.x = atof(argv[1]);
        red_xy.y = atof(argv[2]);
        green_xy.x = atof(argv[3]);
        green_xy.y = atof(argv[4]);
        blue_xy.x = atof(argv[5]);
        blue_xy.y = atof(argv[6]);
        if (argc == 9)
        {
            white_xy.x = atof(argv[7]);
            white_xy.y = atof(argv[8]);
        }
        else
        {
            // cout << "Warning: using D65 white point <0.3127, 0.3290>" << endl;
            white_xy.x = 0.3127;
            white_xy.y = 0.3290;
        }
    }
    else
    {
        PrintUsage();
        exit(-1);
    }

	unsigned long long volumeLuv = gamutVolumeLuv(red_xy, green_xy, blue_xy, white_xy);
	unsigned long long volumeLab = gamutVolumeLab(red_xy, green_xy, blue_xy, white_xy);

    cout << "Lab Volume: " << volumeLab << endl;
    cout << "Luv Volume: " << volumeLuv << endl;

    return 0;
}
#endif


struct Triangle
{
	float2 a;
	float2 b;
	float2 c;
};

struct Polygon6
{
	float2 points[6];
	int numPoints;
};

// Check whether point p is on the "left" side of the line ab extended to infinity.
bool ClipCheck(float2 a, float2 b, float2 p)
{
	float val = (b.y - a.y) * p.x - (b.x - a.x) * p.y + b.x * a.y - b.y * a.x;
	return val >= 0;
}

// Return intersection point of lines ab and cd.
float2 Intersect(float2 a, float2 b, float2 c, float2 d)
{
	float2 r{};
	r.x = ((a.x*b.y - a.y*b.x)*(c.x - d.x) - (a.x - b.x)*(c.x*d.y - c.y*d.x)) / ((a.x - b.x) * (c.y - d.y) - (a.y - b.y) * (c.x - d.x));
	r.y = ((a.x*b.y - a.y*b.x)*(c.y - d.y) - (a.y - b.y)*(c.x*d.y - c.y*d.x)) / ((a.x - b.x) * (c.y - d.y) - (a.y - b.y) * (c.x - d.x));
	return r;
}

// Intersects two clockwise triangles to generate a clockwise Polygon with up to six sides.
// Uses Sutherland-Hodgman algorithm with p as subject and q as clip.
Polygon6 Intersect(const Triangle& p, const Triangle& q)
{
	Polygon6 r = {};
	r.points[0] = p.a;
	r.points[1] = p.b;
	r.points[2] = p.c;
	r.numPoints = 3;
	for (int i = 0; i < 3; i++)
	{
		float2 clipA = i == 0 ? q.a : i == 1 ? q.b : q.c;
		float2 clipB = i == 0 ? q.b : i == 1 ? q.c : q.a;
		Polygon6 t = r;
		r.numPoints = 0;
		float2 s = t.points[t.numPoints - 1];
		for (int j = 0; j < t.numPoints; j++)
		{
			float2 e = t.points[j];
			if (ClipCheck(clipA, clipB, e))
			{
				if (!ClipCheck(clipA, clipB, s))
				{
					r.points[r.numPoints++] = Intersect(s, e, clipA, clipB);
				}
				r.points[r.numPoints++] = e;
			}
			else if (ClipCheck(clipA, clipB, s))
			{
				r.points[r.numPoints++] = Intersect(s, e, clipA, clipB);
			}
			s = e;
		}
	}

	float2 c = r.points[0];
	std::sort(&r.points[0], &r.points[r.numPoints], [c](const float2& a, const float2& b) {
		float det = (a.x - c.x) * (b.y - c.y) - (b.x - c.x) * (a.y - c.y);
		return det < 0;
	});

	return r;
}

// Calculate the area of a convex clockwise polygon.
float Area(const Polygon6& p)
{
	float sum = 0;
	int numTris = p.numPoints - 2;
	for (int i = 0; i < numTris; i++)
	{
		float2 a = p.points[0];
		float2 b = p.points[i + 1];
		float2 c = p.points[i + 2];
		float area = -0.5f * (a.x * b.y + b.x * c.y + c.x * a.y - b.x * a.y - c.x * b.y - a.x * c.y);
		sum += area;
	}
	return sum;
}

#if 0
typedef float2[3] triangle2D;

float areaOfTriangle2D(triangle2D tri)
{
	return cross(tri[1] - tri[0], tri[2] - tri[0])*0.5f;
}


float2 baryTriangle2D(triangle2D tri, float3 pt)
{
	// Compute vectors        
	float2 v0 = tri[2] - tri[0];
	float2 v1 = tri[1] - tri[0];
	float2 v2 = pt - tri[0];

	// Compute dot products
	float dot00 = dot(v0, v0);
	float dot01 = dot(v0, v1);
	float dot02 = dot(v0, v2);
	float dot11 = dot(v1, v1);
	float dot12 = dot(v1, v2);

	// Compute barycentric coordinates
	float invDenom = 1.f / (dot00 * dot11 - dot01 * dot01);
	float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	return float2(u, v);
}


bool pointInTriangle2D(triangle2D tri, float3 pt)
{
	float2 bary = baryTriangle2D(tri, pt);
	return (bary.x >= 0) && (bary.y >= 0) && (bary.x + bary.y < 1.0f);
}

float sdfTriangle2D(triangle2D tri)
{



}
#endif

// Computes area of gamut triangle inputs are in xy,
// but math and output are in uv 
float ComputeGamutArea(float2 r, float2 g, float2 b)
{
	// Convert from 1931 xy to 1976 uv coordinates:
	float2 red_uv, grn_uv, blu_uv;
	red_uv = xytouv(r);
	grn_uv = xytouv(g);
	blu_uv = xytouv(b);

	float2 vr = red_uv - grn_uv;
	float2 vb = blu_uv - grn_uv;
	return cross(vb, vr)*0.5f;
}


// compute proportion of triangle2 that is covered by triangle 1
// inputs are xy chromatiticies, but math and output are in uv 
float ComputeGamutCoverage(float2 r1, float2 g1, float2 b1, float2 r2, float2 g2, float2 b2)
{
	Triangle tri1, tri2;

	// Convert from 1931 xy to 1976 uv coordinates:
	tri1.a = xytouv(b1);
	tri1.b = xytouv(g1);
	tri1.c = xytouv(r1);

	tri2.a = xytouv(b2);
	tri2.b = xytouv(g2);
	tri2.c = xytouv(r2);

	Polygon6 Overlap = Intersect( tri1, tri2 );

	float area = Area(Overlap);

	return area / ComputeGamutArea(r2, g2, b2);	// return as fraction of tri2
}

// Shape of human visual gamut in xy 1931 chromaticities
#if 0
const vec2 aa = vec2(0.174112257, 0.004963727); // 380.0
const vec2 ab = vec2(0.150985408, 0.022740193); // 455.0
const vec2 ac = vec2(0.135502671, 0.039879121); // 465.0
const vec2 ad = vec2(0.124118477, 0.057802513); // 470.0
const vec2 ae = vec2(0.109594324, 0.086842511); // 475.0
const vec2 af = vec2(0.091293516, 0.132702055); // 480.0
const vec2 ag = vec2(0.068705910, 0.200723220); // 485.0
const vec2 ah = vec2(0.045390735, 0.294975965); // 490.0
const vec2 ai = vec2(0.023459943, 0.412703479); // 495.0
const vec2 aj = vec2(0.008168028, 0.538423071); // 500.0
const vec2 ak = vec2(0.003858521, 0.654823151); // 505.0
const vec2 al = vec2(0.013870246, 0.750186428); // 510.0
const vec2 am = vec2(0.038851802, 0.812016021); // 515.0
const vec2 an = vec2(0.074302424, 0.833803082); // 520.0
const vec2 ao = vec2(0.114160721, 0.826206968); // 525.0
const vec2 ap = vec2(0.154722061, 0.805863545); // 530.0
const vec2 aq = vec2(0.192876183, 0.781629131); // 535.0
const vec2 ar = vec2(0.229619673, 0.754329090); // 540.0
const vec2 as = vec2(0.265775085, 0.724323925); // 545.0
const vec2 at = vec2(0.736842105, 0.263157895); // 780.0
#endif
