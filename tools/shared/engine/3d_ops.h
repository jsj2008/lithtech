
// This module defines lots of the main 3d operations like transformation,
// clipping, projection, rejection tests, etc..

#ifndef __3D_OPS_H__
#define __3D_OPS_H__

#include "common_stuff.h"

inline int RoundFloatToInt(float f)
{
	int nResult;

	__asm
	{
		fld f
		fistp nResult
	}
	return nResult;
}

// The formats for the vertices.
#define BASE_TLVERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR)
#define TLVERTEX_FORMAT (BASE_TLVERTEX_FORMAT | D3DFVF_TEX1)
#define LMVERTEX_FORMAT (BASE_TLVERTEX_FORMAT | D3DFVF_TEX2)

// STRUCTURES
struct TLRGB    { uint8 b; uint8 g; uint8 r; uint8 a; };
struct RGBColor {
	union {
		TLRGB rgb;
		uint32 color; }; };

class TCoordSet
{
public:
	float	tu, tv;
};

// Our internal structure for a D3DTLVERTEX which makes it easier to
// access the color values.
class TLVertex
{
	public:

			static void ClipExtra(TLVertex *pPrev, TLVertex *pCur, TLVertex *pOut, float t)
			{
				pOut->tu = pPrev->tu + t * (pCur->tu - pPrev->tu);
				pOut->tv = pPrev->tv + t * (pCur->tv - pPrev->tv);
				pOut->rgb.r = (uint8)RoundFloatToInt(pPrev->rgb.r + t * (pCur->rgb.r - pPrev->rgb.r));  
				pOut->rgb.g = (uint8)RoundFloatToInt(pPrev->rgb.g + t * (pCur->rgb.g - pPrev->rgb.g));  
				pOut->rgb.b = (uint8)RoundFloatToInt(pPrev->rgb.b + t * (pCur->rgb.b - pPrev->rgb.b)); 
				pOut->rgb.a = (uint8)RoundFloatToInt(pPrev->rgb.a + t * (pCur->rgb.a - pPrev->rgb.a)); 
				pOut->specular.rgb.a = (uint8)RoundFloatToInt(pPrev->specular.rgb.a + t * (pCur->specular.rgb.a - pPrev->specular.rgb.a)); 
			}

		LTVector m_Vec;
		float rhw;

		union
		{
			TLRGB rgb;
			uint32 color;
		};

		RGBColor specular;

		#include "vertextcoord.h"

		union
		{
			struct
			{
				float tu, tv;
			};
			TCoordSet m_TCoords[1];
		};


		#ifdef LGLIDE_COMPILE
			float tutv_oow;
		#endif
};


class LMVertex
{
	public:

		static void ClipExtra(LMVertex *pPrev, LMVertex *pCur, LMVertex *pOut, float t)
		{
			pOut->tu = pPrev->tu + t * (pCur->tu - pPrev->tu);
			pOut->tv = pPrev->tv + t * (pCur->tv - pPrev->tv);
			pOut->lm_tu = pPrev->lm_tu + t * (pCur->lm_tu - pPrev->lm_tu);
			pOut->lm_tv = pPrev->lm_tv + t * (pCur->lm_tv - pPrev->lm_tv);
			pOut->rgb.r = (uint8)RoundFloatToInt(pPrev->rgb.r + t * (pCur->rgb.r - pPrev->rgb.r));  
			pOut->rgb.g = (uint8)RoundFloatToInt(pPrev->rgb.g + t * (pCur->rgb.g - pPrev->rgb.g));  
			pOut->rgb.b = (uint8)RoundFloatToInt(pPrev->rgb.b + t * (pCur->rgb.b - pPrev->rgb.b)); 
			pOut->rgb.a = (uint8)RoundFloatToInt(pPrev->rgb.a + t * (pCur->rgb.a - pPrev->rgb.a)); 
			pOut->specular.rgb.a = (uint8)RoundFloatToInt(pPrev->specular.rgb.a + t * (pCur->specular.rgb.a - pPrev->specular.rgb.a)); 
		}

		LTVector m_Vec;
		float rhw;

		union
		{
			TLRGB rgb;
			uint32 color;
		};

		RGBColor specular;

		float tu, tv;
		float lm_tu, lm_tv;

		#include "vertextcoord.h"
};

#endif


