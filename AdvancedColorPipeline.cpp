//
// AdvancedColorPipeline.cpp
//

#include "pch.h"

/*

#include "basicmath.h"
#include "colorspaces.h"

#define HALF_MAX 65504.0


// TODO: Use real OS enum
//#define ADOBE_RGB
#if 0
typedef enum DXGI_COLOR_SPACE_TYPE {
	DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 = 0,
	DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709 = 1,
	DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P709 = 2,
	DXGI_COLOR_SPACE_RGB_STUDIO_G22_NONE_P2020 = 3,
	DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_PADOBE = 4,		// hack TODO fix
	DXGI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601 = 5,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601 = 6,
	DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P601 = 7,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709 = 8,
	DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P709 = 9,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020 = 10,
	DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P2020 = 11,
	DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 = 12,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_LEFT_P2020 = 13,
	DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020 = 14,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_TOPLEFT_P2020 = 15,
	DXGI_COLOR_SPACE_YCBCR_STUDIO_G2084_TOPLEFT_P2020 = 16,
	DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020 = 17,
	DXGI_COLOR_SPACE_CUSTOM = 0xFFFFFFFF
} DXGI_COLOR_SPACE_TYPE;
#endif

typedef float3 Image;		// for bring-up
							// can be 256x1 scanline,  or a real surface someday

typedef struct st2086_struct
{
	float peakLuminance;				// in cd/m2
	float frameAverageLuminance;		// CALL or FALL
	float minLuminance;					// black level
	float2 r, g, b;						// color primaries in 1931 xy coords
} st2086;

//globals:
const static float defaultSDRBoost = 1.5;
const static float defaultBrightnessSetting = 1.6666666;

// model state parameters
st2086 ContentMetadata;					// passed using global instead of an actual argument
static float GlobalBrightnessSetting;
static float SDRBoost;



st2086 GetDisplayCharacteristics()
{
	st2086 dc;
	dc.minLuminance = 0.0f;
	dc.minLuminance = 1200.0f;
	dc.frameAverageLuminance = 600;

	return dc;
}

void DWM_SetContentMetadata(st2086 in)
{
	ContentMetadata = in;
}

st2086 DWM_GetContentMetadata()
{
	return ContentMetadata;
}


float UI_SetSDRBoostSlider(uint sliderPercentage)
{
	return SDRBoost = sliderPercentage * 50.0f / 100.0f;
}

// accessor
float UI_GetSDRBoostSetting()
{
	return SDRBoost;
}

float UI_GetGLobalBrightnessSetting()
{
	return GlobalBrightnessSetting;
}

// Percentage UI slider
float UI_GlobalBrightnessSlider(uint sliderPercentage)
{
	//	100 % is maxFALL of this power supply in Nits
	// ideally should be a log scale

	return defaultBrightnessSetting*sliderPercentage / 100.0f;
}

// Nits brightness slider
float setNitsBrightness(float nits)
{
	float factor = nits / 80.0f;
	return factor;
}

float Monitor_GetOSDBrightnessSlider(void)
{
	return 1.0;		// monitors should default to OSD does no change in HDR mode
}


Image ClassicApp_Render()
{
	Image image;
	image = float3(1.0, 1.0, 1.0);

	return image;
}

float Image_Average(Image inImage)
{
	return inImage.y;
}

float Image_Peak(Image inImage)
{
	return inImage.z;
}

Image Image_Mult(Image inImage, float factor)
{
	Image image;

	image.x = inImage.x*factor;
	image.y = inImage.y*factor;
	image.z = inImage.z*factor;

	return image;
}

Image Image_Apply2084(Image inImage)
{
	Image image;

	image.x = apply2084(inImage.x);
	image.y = apply2084(inImage.y);
	image.z = apply2084(inImage.z);

	return image;
}

Image Image_Remove2084(Image inImage)
{
	Image image;

	image.x = remove2084(inImage.x);
	image.y = remove2084(inImage.y);
	image.z = remove2084(inImage.z);

	return image;
}

Image Image_Rec2100toRec709(Image inImage)
{
	Image image;

	image = Rec709ToRec2020(inImage);

	return image;
}

Image Image_Rec709toRec2100(Image inImage)
{
	Image image;

	image = Rec709ToRec2020(inImage);

	return image;
}

Image Image_2020toDCIP3(Image inImage)
{
	Image image;

	image = Rec2020toDCIP3(inImage);

	return image;

}

Image Image_ToneMap( Image inImage, float inputPeakLuminance, float outputPeakLuminance )
{
	return inImage;	// TODO implement this.
}

Image HDRMasterAndEncode(Image inImage)
{
	Image image;

	// do exposure adjustment
	// comput average scene luminance
	float avg = Image_Average(inImage);

	// rescale image intensity to limited range
	image = Image_Mult( inImage, 0.18f / avg );

	// handle any peaks above the range of the encoding format
	float maxContentLuminance = Image_Peak(image);
	float maxEncodeLuminance = 10000.0f;
	image = Image_ToneMap( image, maxContentLuminance, maxEncodeLuminance);

	return image;
}

Image SDRMasterAndEncode(Image inImage)
{
	Image image;

	// do exposure adjustment
	// comput average scene luminance
	float avg = Image_Average(inImage);

	// rescale image intensity to limited range
	image = Image_Mult( inImage, 0.18f / avg );

	// handle any peaks above the range of the encoding format
	float maxContentLuminance = Image_Peak(image);
	float maxEncodeLuminance = 80.0f;
	image = Image_ToneMap(image, maxContentLuminance, maxEncodeLuminance);

	return image;
}

Image HDRApp_Render()
{
	Image image;

	image = float3( HALF_MAX, HALF_MAX, HALF_MAX );	// simple test pattern
//	image = LoadOpenEXRFile();						// get fp16/ACES content

	// determine capabilities of display
	st2086 displayCharacteristics = GetDisplayCharacteristics();


	// define what our content range is
	st2086 contentMetadata;
	contentMetadata.peakLuminance = Image_Peak( image );		// this content uses entire range of float16 format


	// TOSO: compensate for brightness setting


	// decide if we need to done map:

	// compare peak luminance of content and max capability of display
	float brightnessDiff = fabs(contentMetadata.peakLuminance - displayCharacteristics.peakLuminance)/
												displayCharacteristics.peakLuminance;
	//	if ( PeakLuminance is different enough to matter)
	if (brightnessDiff > 0.1f)
	{
		// Decide whether to tone map ourselves or the hardware
		if ( false )					// ( if we trust display to tone map!)
		{
			// send metadata about content, thus notifiying implementation to handle it
			DWM_SetContentMetadata(contentMetadata);
		}
		else
		{
			// We use app code to tone map image to match display's peak luminance
			image = Image_ToneMap(image, contentMetadata.peakLuminance, displayCharacteristics.peakLuminance);
			// leave metadata at OS default from boot
			//		contentMetadata = displayCharacteristics;
			//		Panel_SetMetadata( contentMetadata );
		}
	}


#ifdef GAMUT_REMAP
	if (gamut is different enough)
		&& (we trust display to gamut map)
	{
		st2086 contentMeta;
		contentMeta = GetDisplayCharacteristics();	// default all fields to current settings
		contentMeta.peakLumance = HALF_MAX;			// this content uses entire range of float format
		Panel_SetMetadata(contentMeta);				// tell display panel to handle that
	}
	else
	{
		// assert default system metadata
		// Remap to display primaries
		image = GamutMap(image, PanelGamut);
	}
#endif

	return image;						// Image rendered by HDR app

}


Image HDR10App_Render()
{
	Image image;

	image = float3(10000.0, 10000.0, 10000.0);			// simple test pattern
	return image;

#ifdef MORESTUFF
	// determine capabilities of display
	GetDisplayCharacteristics();

	// compensate for brightness setting

	if (gamut is different enough)
		&& (we trust display to gamut map)
	{
		// send content gamut metadata to output
		// notifying display to handle it
	}
	else
	{
		// assert default system metadata
		// Remap to display primaries
		image = GamutMap(image, PanelGamut);
	}


	if (PeakLuminance is different enough)
		&& (we trust display to tone map)
	{
		// send metadata about content
		// notifiying implementation to handle it
	}
	else
	{
		// Tone map image to peak luminance
		image = ToneMap(image, PanelPeakLuminance);
	}
#endif

}

//compose or flipl
Image DWM_Present( Image inImage, DXGI_FORMAT format, bool fullscreen, DXGI_COLOR_SPACE_TYPE space, float SDRboost )
{
	Image image;				// input image any color space
	Image linearImage;			// image converted to linear profile
	Image linear709Image;		// image converted to linear and 709 gamut

	switch ( space )		// depending on the color space
	{
	case DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709:					// CCCS
		if (format == DXGI_FORMAT_R16G16B16A16_FLOAT)
		{
			image = inImage;
		}
		else
		{
			printf("Color Space %s is not supported with Format %s\n", space, format);
			exit( 1 );
		}
		break;

	case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:				// HDR10
		if (format == DXGI_FORMAT_R10G10B10A2_UNORM)
		{
			// convert from HDR10 to CCCS for composition
			// remove PQ/2082 profile curve
			Image linearImage = Image_Remove2084(inImage);

			// rotate to 709 primaries
			Image linear709Image = Image_Rec2100toRec709(linearImage);

			// output
			image = linear709Image;
		}
		else
		{
			printf("Color Space %s is not supported with Format %s\n", space, format);
			exit(1);
		}
		break;

#ifdef ADOBE_RGB
	case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_PADOBE:					// Adobe RGB
		if (format == DXGI_FORMAT_R8G8B8A8_UNORM
		 || format == DXGI_FORMAT_R10G10B10A2_UNORM)

		{
			// convert from AdobeRGB to 709 color primaries
			Image linearAdobeImage = Image_RemoveAdobeProfile(inImage);

			// convert to linear profile
			Image linear709Image = Image_convertAdobeto709(linearAdobeImage);

			image = linear709Image*UI_GetSDRBoostSetting();
		}
		else
		{
			printf("Color Space %s is not supported with Format %s\n", space, format);
			exit(1);
		}
#endif
		
	default:														// SDR
		if (format == DXGI_FORMAT_R8G8B8A8_UNORM			
		 || format == DXGI_FORMAT_R10G10B10A2_UNORM)
		{
			image = inImage*UI_GetSDRBoostSetting();					// apply adjustment to classic content
		}
		else
		{
			printf("Color Space %s is not supported with Format %s\n", space, format);
			exit(1);
		}
	}

	return image;

}

Image GPU_Display( Image inImage, bool HDR, float brightnessFactor)
{
	Image image;

	image = Image_Mult( inImage, UI_GetGLobalBrightnessSetting() );

	if ( true )			// if (link is in HDR mode)
	{
		// Convert from 709 to 2020 primaries
		image = Image_Rec709toRec2100(image);

		// convert from linear to PQ profile
		image = Image_Apply2084( image );

		// at this point, the image should be in 2084 range
//		ASSERT(Image_Peak( image )<= 10000.0);
	}
	else
		exit(1);				// this sample shows only HDR mode, not SDR mode

	return image;		// image provided in wire format (HDR10)
}

Image Scaler_Rec2020toPanelPrimaries(Image inImage)
{
	Image image;

	// For now, assume panel has DCIP3 primaries
	image = Image_2020toDCIP3(inImage);

	return image;
}

Image Scaler_ApplyPanelProfile(Image inImage)
{
	Image image;

	// for now assume panel has a gamma 4.0
	image = powf(inImage.x, 1.0 / 4.0);

	return image;
}

Image Scaler_Scale(Image inImage)
{
	Image image;

	// Scaler knows its own characteristics:
	st2086 displayCharacteristics = GetDisplayCharacteristics();

	// It also has metadata from input stream:
	st2086 contentMetadata = DWM_GetContentMetadata();

	// Apply OSD brightness factor
	image = inImage * Monitor_GetOSDBrightnessSlider();

	// convert from PQ to linear
	image = Image_Remove2084(image);

	// if metadata says so, then tone map
	image = Image_ToneMap(image, contentMetadata.peakLuminance, displayCharacteristics.peakLuminance);

	// convert from 2020 primaries to hardware primaries
	image = Scaler_Rec2020toPanelPrimaries(image);

#if 0
	//if gamut is different, then gamut remap
	if (contentMetadata != displayCharacteristics, )
		then gamut map
#endif

		// Apply profile curve of this hardware panel
		image = Scaler_ApplyPanelProfile(image);

	return image;			// Image is ready for TCON and driver IC
}


Image Panel_Show(Image inImage)
{
	Image image;

	image = inImage;

	return image;
}

#if 0

Whatever()
{
	// Copy bits to RT
	// if no scaling, shader-free path works fine.
	// Sampler uses degamma mode to convert from sRGB to linear profile
	// No change to primaries needed
	// UNORM 1.0 = fp16 1.0
	if (fullscreen)
	{
		flipSRGB(image)
	}
	else
	{
		image = ComposeSRGB(image);
		flipCCCS(Image);
	}
}

AnotherFun()					// CCCS( image )
{
	// Copy bits to RT
	// if no scaling, blt works fine.
	// Sampler uses default/linear mode so that linear data is preserved
	// No change to primaries needed
	ComposeCCCS(image);
}

// inside the display driver.
Image GPU_DisplayCCCS( Image inImage )
{
	apply brightness scale
	Convert to 2020/2100 primaries
	convert to 2084/PQ profile
	chroma-subsample if HDMI bw requires it at this resolution
	Encode in 10bit using dithering
	output to wire
}

Image HardwareToneMapper( Image inImage )
{
	check input metadata
	check characteristics of this panel.
	if (these are similar)
	{
		return identity;
	}
	else
	{
		ratio = inputPeakLuminance/PanelPeakLuminance;
		image = ToneMapByRatio( image, ratio );

	}
}
#endif

// Routine that takes the image at any point and displays it on our current PC.
void Image_DebugShow( Image inImage )
{
	printf( "RGB: %6.4f %6.4f %6.4f\n", inImage.r, inImage.g, inImage.b );
}


// Entire display pipeline in compilable code format
void ACPipeline()
{
	Image image;
	DXGI_FORMAT format;
	DXGI_COLOR_SPACE_TYPE colorSpace;
	st2086 displayCharacteristics;
	st2086 contentMetadata;

	// OS Boot Sequence
	// identify characteristics of display
	displayCharacteristics = GetDisplayCharacteristics();

	// on dislpay connection, we set  metadata to default to characteristics of panel
	DWM_SetContentMetadata(displayCharacteristics);
	// end OS boot sequence


	// Get control inputs from OS UI
	float SDRboost = UI_GetSDRBoostSetting();
	float brightnessFactor = UI_GlobalBrightnessSlider(100);


	// Select an app and run it
	switch (1)
	{
	case 0:
		image = ClassicApp_Render();				// sRGB 8-bit app
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
		break;

	case 1:											// CCCS: HDR game or image viewer
		image = HDRApp_Render();
		format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
		break;

	case 2:
		image = HDR10App_Render();					// HDR10 e.g. Video Player
		format = DXGI_FORMAT_R10G10B10A2_UNORM;
		colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
		break;
#ifdef ADOBE_RGB
	case 3:
		image = AdobeApp_Render();					// e.g. Photoshop or Premiere
		format = DXGI_FORMAT_R10G10B10A2_UNORM;
		colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_PADOBE;
		break;
#endif
	}

	// DWM composes the Window to a canonical color space and format
	bool fullscreen = false;
	if (!fullscreen)
		image = DWM_Present(image, format, fullscreen, colorSpace, SDRboost);	// DWM Compositor

																				//	image is now in CCCS encoding:
																				//		1.0 is 80 nits
																				//		gamut potentially outside 709 if panel gamut > 709
																				//		content is tone mapped
																				//		Peak brightness can be up to 5M nits

																				// Display final DWM composed image using GPU display hardware:
	bool HDR = true;   //  or SDR;
	image = GPU_Display(image, HDR, brightnessFactor);					// GPU Display Hardware


	if (true)				// HDMI or DisplayPort
	{
//		image = Wire_Send(image);			// Wire protocol
		image = Scaler_Scale(image);		// DSP in monitor
		image = Panel_Show(image);			// TCON and Driver IC
	}
	else 				// integrated panel
	{
//		image = eDP(image);					// Wire Protocol
		image = Panel_Show(image);			// TCON and driver IC
	}

	Image_DebugShow(image);

	exit(0);
}
*/