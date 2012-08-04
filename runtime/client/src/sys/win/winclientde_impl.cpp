#include "bdefs.h"

#include "clientmgr.h"
#include "iltclient.h"
#include "console.h"
#include "winclientde_impl.h"
#include "interface_helpers.h"
#include "text_mgr.h"
#include "load_pcx.h"
#include "bindmgr.h"
#include "client_ticks.h"
#include "ltgraphicscaps.h"
#include <d3d9.h>

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//a IClientFormatMgr interface
#include "client_formatmgr.h"
static IClientFormatMgr *format_mgr;
define_holder(IClientFormatMgr, format_mgr);

//the ILTClient game interface
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);





extern int32 g_CV_ForceClear;
extern int32 g_nConsoleLines;

// The screen surface.. treated specially.
CisSurface g_ScreenSurface;

// The render struct we're using..
RenderStruct *g_pCisRenderStruct = LTNULL;

// Used for transparent drawing.
GenericColor g_TransparentColor, g_SolidColor;


// The surface allocator..
static StructBank g_SurfaceBank;


// All the surfaces..
static CGLinkedList<CisSurface*> g_Surfaces;


// Used for masked drawing.
static CisSurface *g_pMask;

// Helper for masked drawing.
static uint8 g_MaskLookup[257];

PFormat g_ScreenFormat;
uint32 g_nScreenPixelBytes;

// Used when restoring surfaces.
static PFormat g_PrevScreenFormat;
static uint32 g_nPrevScreenPixelBytes;



static LTRESULT cis_WarpSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc, 
	LTWarpPt *pCoords, int nCoords);
static LTRESULT cis_WarpSurfaceToSurfaceTransparent(HSURFACE hDest, HSURFACE hSrc, 
	LTWarpPt *pCoords, int nCoords, HLTCOLOR hColor);
static LTRESULT cis_WarpSurfaceToSurfaceSolidColor(HSURFACE hDest, HSURFACE hSrc, 
	LTWarpPt *pCoords, int nCoords, HLTCOLOR hTransColor, HLTCOLOR hFillColor);

static LTRESULT cis_End3D(uint flags);


typedef LTRESULT (*DrawPixelsFn)(bool bSameSurface, uint8 *pSrcData, uint8 *pDestData, 
	long srcPitch, long destPitch, LTRect *pSrcRect, LTRect *pDestRect);


// ----------------------------------------------------------------- //
// Internal helper functions.
// ----------------------------------------------------------------- //

inline void cis_SetTransparentColor(uint32 inColor)
{
	format_mgr->Mgr()->PValueToFormatColor(&g_ScreenFormat, inColor, g_TransparentColor);
}

inline void cis_SetSolidColor(uint32 inColor)
{
	format_mgr->Mgr()->PValueToFormatColor(&g_ScreenFormat, inColor, g_SolidColor);
}


static LTRESULT cis_OpaqueDraw(bool bSameSurface,
	uint8 *pSrcLine, uint8 *pDestLine, 
	long srcPitch, long destPitch, LTRect *pSrcRect, LTRect *pDestRect)
{
	uint32 y, rectWidth, rectHeight, bytesPerLine;

	rectWidth = (uint32)(pSrcRect->right - pSrcRect->left);
	rectHeight = (uint32)(pSrcRect->bottom - pSrcRect->top);
	bytesPerLine = rectWidth * g_nScreenPixelBytes;

	pSrcLine += pSrcRect->top*srcPitch + pSrcRect->left*g_nScreenPixelBytes;
	pDestLine += pDestRect->top*destPitch + pDestRect->left*g_nScreenPixelBytes;
	
	for(y=0; y < rectHeight; y++)
	{
		if(bSameSurface)
		{
			memmove(pDestLine, pSrcLine, bytesPerLine);
		}
		else
		{
			memcpy(pDestLine, pSrcLine, bytesPerLine);
		}

		pSrcLine += srcPitch;
		pDestLine += destPitch;
	}

	return LT_OK;
}


template<class P>
inline void cis_SolidColorDrawLine(uint8 *pSrcLine, uint8 *pDestLine, uint32 width, P *pixelType)
{
	P src, dest;

	src = pSrcLine;
	dest = pDestLine;
	while(width)
	{
		width--;
		
		if(src != g_TransparentColor)
			dest = g_SolidColor;

		++src;
		++dest;
	}
}

static LTRESULT cis_SolidColorDraw(bool bSameSurface,
	uint8 *pSrcLine, uint8 *pDestLine, 
	long srcPitch, long destPitch, LTRect *pSrcRect, LTRect *pDestRect)
{
	uint32 y, rectWidth, rectHeight;

	rectWidth = (uint32)(pSrcRect->right - pSrcRect->left);
	rectHeight = (uint32)(pSrcRect->bottom - pSrcRect->top);

	pSrcLine += pSrcRect->top*srcPitch + pSrcRect->left*g_nScreenPixelBytes;
	pDestLine += pDestRect->top*destPitch + pDestRect->left*g_nScreenPixelBytes;

	for(y=0; y < rectHeight; y++)
	{
		if(g_ScreenFormat.GetType() == BPP_16)
		{
			cis_SolidColorDrawLine(pSrcLine, pDestLine, rectWidth, (Pixel16*)LTNULL);
		}
		else if(g_ScreenFormat.GetType() == BPP_32)
		{
			cis_SolidColorDrawLine(pSrcLine, pDestLine, rectWidth, (Pixel32*)LTNULL);
		}
		
		pSrcLine += srcPitch;
		pDestLine += destPitch;
	}

	return LT_OK;
}


template<class P>
inline void cis_TransparentDrawLine(uint8 *pSrcLine, uint8 *pDestLine, uint32 width, P *pixelType)
{
	P src, dest;

	src = pSrcLine;
	dest = pDestLine;
	while(width)
	{
		width--;

		if(src != g_TransparentColor)
			dest = src;

		++src;
		++dest;
	}
}

static LTRESULT cis_TransparentDraw(bool bSameSurface,
	uint8 *pSrcData, uint8 *pDestData, 
	long srcPitch, long destPitch, LTRect *pSrcRect, LTRect *pDestRect)
{
	uint32 y, rectWidth, rectHeight;
	uint8 *pSrcLine, *pDestLine;
	
	rectWidth = (uint32)(pSrcRect->right - pSrcRect->left);
	rectHeight = (uint32)(pSrcRect->bottom - pSrcRect->top);

	pSrcLine = pSrcData + pSrcRect->top*srcPitch + pSrcRect->left*g_nScreenPixelBytes;
	pDestLine = pDestData + pDestRect->top*destPitch + pDestRect->left*g_nScreenPixelBytes;

	for(y=0; y < rectHeight; y++)
	{
		if(g_ScreenFormat.GetType() == BPP_16)
		{
			cis_TransparentDrawLine(pSrcLine, pDestLine, rectWidth, (Pixel16*)LTNULL);
		}
		else if(g_ScreenFormat.GetType() == BPP_32)
		{
			cis_TransparentDrawLine(pSrcLine, pDestLine, rectWidth, (Pixel32*)LTNULL);
		}
		
		pSrcLine += srcPitch;
		pDestLine += destPitch;
	}

	return LT_OK;
}


inline void cis_MaskedDrawLine16(uint8 *pSrcLine, uint8 *pDestLine, uint32 width,
	uint8 *pMaskLine, uint32 maskMaskX, uint32 maskX)
{
	Pixel16 src, dest, mask;

	src = pSrcLine;
	dest = pDestLine;
	mask = pMaskLine;

	while(width)
	{
		width--;

		if(*(mask.m_pPixel) == g_TransparentColor.wVal)
		{
			//Don't draw this pixel
		}
		else if (*(src.m_pPixel) == g_TransparentColor.wVal)
		{
			//Don't draw this pixel
		}
		else
		{
			dest = (*src.m_pPixel);
		}

		++mask;
		++src;
		++dest;
	}
}

inline void cis_MaskedDrawLine32(uint8 *pSrcLine, uint8 *pDestLine, uint32 width,
								 uint8 *pMaskLine, uint32 maskMaskX, uint32 maskX)
{
	Pixel32 src, dest, mask;

	src = pSrcLine;
	dest = pDestLine;
	mask = pMaskLine;

	while(width)
	{
		width--;

		if(*(mask.m_pPixel) == g_TransparentColor.dwVal)
		{
			//Don't draw this pixel
		}
		else if (*(src.m_pPixel) == g_TransparentColor.dwVal)
		{
			//Don't draw this pixel
		}
		else
		{
			dest = (*src.m_pPixel);
		}

		++mask;
		++src;
		++dest;
	}
}

template<class P>
inline void cis_MaskedDrawLine(uint8 *pSrcLine, uint8 *pDestLine, uint32 width,
							   uint8 *pMaskLine, uint32 maskMaskX, uint32 maskX, P *pixelType)
{
	P src, dest, mask;

	src = pSrcLine;
	dest = pDestLine;
	mask = pMaskLine;

	while(width)
	{
		width--;

		if(src != g_TransparentColor)
		{
			dest = mask[maskX & maskMaskX];
		}

		++maskX;
		++src;
		++dest;
	}
}


// If the surface's contents are dirty, reoptimize the surface.
static void cis_OptimizeDirty(CisSurface *pSurface)
{
	if(!pSurface)
		return;
	
	if((pSurface->m_Flags & SURFFLAG_OPTIMIZED) && (pSurface->m_Flags & SURFFLAG_OPTIMIZEDIRTY))
	{
		g_pCisRenderStruct->OptimizeSurface(pSurface->m_hBuffer, pSurface->m_OptimizedTransparentColor);
		pSurface->m_Flags &= ~SURFFLAG_OPTIMIZEDIRTY;
	}
}


static LTRESULT cis_MaskedDraw(bool bSameSurface,
	uint8 *pSrcLine, uint8 *pDestLine,
	long srcPitch, long destPitch, LTRect *pSrcRect, LTRect *pDestRect)
{
	FN_NAME(cis_MaskedDraw);

	uint8 *pMaskData;
	uint32 y, maskMaskX, maskMaskY;
	long maskPitch;
	uint32 rectWidth, rectHeight;
	
	CHECK_PARAMS2(g_pMask && g_pMask->m_Width <= 256 && g_pMask->m_Height <= 256);

	maskMaskX = g_MaskLookup[g_pMask->m_Width];
	maskMaskY = g_MaskLookup[g_pMask->m_Height];

	CHECK_PARAMS2(maskMaskX && maskMaskY);

	pMaskData = (uint8*)cis_LockSurface(g_pMask, maskPitch);
	if(!pMaskData)
	{
		ERR(1, LT_ERROR);
	}

	rectWidth = (uint32)(pSrcRect->right - pSrcRect->left);
	rectHeight = (uint32)(pSrcRect->bottom - pSrcRect->top);
	pSrcLine += pSrcRect->top*srcPitch + pSrcRect->left*g_nScreenPixelBytes;
	pDestLine += pDestRect->top*destPitch + pDestRect->left*g_nScreenPixelBytes;

	for(y=0; y < rectHeight; y++)
	{
		if(g_ScreenFormat.GetType() == BPP_16)
		{
			
			cis_MaskedDrawLine16(pSrcLine, pDestLine, rectWidth,
				&pMaskData[((pSrcRect->top+y)&maskMaskY)*maskPitch], maskMaskX,
				pSrcRect->left);
			
		}
		else if(g_ScreenFormat.GetType() == BPP_32)
		{
			cis_MaskedDrawLine32(pSrcLine, pDestLine, rectWidth,
				&pMaskData[((pSrcRect->top+y)&maskMaskY)*maskPitch], maskMaskX,
				pSrcRect->left);
		}
		
		pSrcLine += srcPitch;
		pDestLine += destPitch;
	}

	cis_UnlockSurface(g_pMask);
	return LT_OK;
}



static LTRESULT cis_DoDrawSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc, 
	LTRect *pSrcRect, int destX, int destY, DrawPixelsFn fn)
{
	FN_NAME(cis_DoDrawSurfaceToSurface);

	CisSurface *pSrc = (CisSurface*)hSrc;
	CisSurface *pDest = (CisSurface*)hDest;
	uint8 *pSrcData, *pDestData;
	LTRect srcRect, destRect;
	LTBOOL bIsInside;
	long srcPitch, destPitch;
	BlitRequest blitRequest;
	bool bOk;

	CHECK_PARAMS2(pSrc && pDest);

	if(pSrcRect)
	{
		bIsInside = cis_ClipRectsNonScaled(
			pSrc->m_Width, pSrc->m_Height, pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom, 
			pDest->m_Width, pDest->m_Height, destX, destY, &srcRect, &destRect);
	}
	else
	{
		bIsInside = cis_ClipRectsNonScaled(
			pSrc->m_Width, pSrc->m_Height, 0, 0, pSrc->m_Width, pSrc->m_Height,
			pDest->m_Width, pDest->m_Height, destX, destY, &srcRect, &destRect);
	}

	if(!bIsInside)
		return LT_OK;
	
	// Try to translate the command to let the RenderStruct do it.
	if(fn == cis_OpaqueDraw || 
		fn == cis_TransparentDraw)
	{
		if(pDest == &g_ScreenSurface)
		{
			if(r_GetRenderStruct()->BlitToScreen)
			{
				cis_OptimizeDirty(pSrc);

				blitRequest.m_hBuffer = pSrc->m_hBuffer;
				blitRequest.m_TransparentColor = g_TransparentColor;
				blitRequest.m_pSrcRect = &srcRect;
				blitRequest.m_pDestRect = &destRect;
				blitRequest.m_BlitOptions = (fn == cis_TransparentDraw) ? BLIT_TRANSPARENT : 0;
				blitRequest.m_Alpha = pSrc->m_Alpha;
				
				r_GetRenderStruct()->BlitToScreen(&blitRequest);

				cis_SetDirty(pDest);
				return LT_OK;
			}
		} 
		else if (pSrc==&g_ScreenSurface)
		{
			// do a screen read...
			if ( r_GetRenderStruct()->BlitFromScreen )
			{
				blitRequest.m_hBuffer = pDest->m_hBuffer;
				blitRequest.m_TransparentColor = g_TransparentColor;
				blitRequest.m_pSrcRect = &srcRect;
				blitRequest.m_pDestRect = &destRect;
				blitRequest.m_BlitOptions = (fn == cis_TransparentDraw) ? BLIT_TRANSPARENT : 0;
				
				r_GetRenderStruct()->BlitFromScreen(&blitRequest);
				return LT_OK;
			}
		}
	}

	bOk = false;
	if(pSrcData = (uint8*)cis_LockSurface(pSrc, srcPitch))
	{
		if(pSrc == pDest)
		{
			if(fn(true, pSrcData, pSrcData, srcPitch, srcPitch, &srcRect, &destRect) == LT_OK)
				bOk = true;
		}
		else
		{
			if(pDestData = (uint8*)cis_LockSurface(pDest, destPitch, true))
			{
				if(fn(false, pSrcData, pDestData, srcPitch, destPitch, &srcRect, &destRect) == LT_OK)
					bOk = true;
				
				cis_UnlockSurface(pDest);
			}
		}
		
		cis_UnlockSurface(pSrc);
	}

	if(bOk)
	{
		return LT_OK;
	}
	else
	{
		ERR(1, LT_ERROR);
	}
}


static void cis_InternalBitmapToSurface(CisSurface *pDest, LoadedBitmap *pSrc,
	const LTRect *pSrcRect, int destX, int destY)
{
	LTBOOL bIsInside;
	LTRect srcRect, destRect;
	FMConvertRequest request;
	LTRESULT dResult;


	if(pSrcRect)
	{
		bIsInside = cis_ClipRectsNonScaled(
			pSrc->m_Width, pSrc->m_Height, pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom, 
			pDest->m_Width, pDest->m_Height, destX, destY, &srcRect, &destRect);
	}
	else
	{
		bIsInside = cis_ClipRectsNonScaled(
			pSrc->m_Width, pSrc->m_Height, 0, 0, pSrc->m_Width, pSrc->m_Height,
			pDest->m_Width, pDest->m_Height, destX, destY, &srcRect, &destRect);
	}

	// Read the stuff in.
	request.m_pDest = (uint8*)cis_LockSurface(pDest, request.m_DestPitch, true);
	if(!request.m_pDest)
		return;

	request.m_pDestFormat = &g_ScreenFormat;
	request.m_pDest += destRect.top*request.m_DestPitch + destRect.left*g_nScreenPixelBytes;

	request.m_pSrcFormat = &pSrc->m_Format;
	request.m_pSrc = pSrc->m_Data.GetArray();
	request.m_pSrc += srcRect.top*pSrc->m_Pitch + srcRect.left;
	request.m_pSrcPalette = pSrc->m_Palette;
	request.m_SrcPitch = pSrc->m_Pitch;
	
	request.m_Width = (uint32)(srcRect.right - srcRect.left);
	request.m_Height = (uint32)(srcRect.bottom - srcRect.top);

	dResult = format_mgr->Mgr()->ConvertPixels(&request);
	cis_UnlockSurface(pDest);
}


static LTRESULT cis_InternalWarpSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc, 
	LTWarpPt *pCoords, int nCoords, DrawWarpFn fn)
{
	CisSurface *pSrc, *pDest;
	int i;
	LTBOOL bIsVisible;
	WarpCoords leftCoords[1024], rightCoords[1024];
	uint32 minY, maxY;

	
	pSrc = (CisSurface*)hSrc;
	pDest = (CisSurface*)hDest;
	if(!pSrc || !pDest)
		RETURN_ERROR(1, InternalWarpSurfaceToSurface, LT_INVALIDPARAMS);
	
	// Make sure they specified the coordinates correctly.
	if(nCoords > MAX_WARP_POINTS)
		nCoords = MAX_WARP_POINTS;

	if(nCoords < 3)
		RETURN_ERROR_PARAM(1, InternalWarpSurfaceToSurface, LT_INVALIDPARAMS, "nCoords < 3");

	// Clamp the source coordinates.
	for(i=0; i < nCoords; i++)
	{
		pCoords[i].source_x = LTCLAMP(pCoords[i].source_x, 0.0f, (float)(pSrc->m_Width-1));
		pCoords[i].source_y = LTCLAMP(pCoords[i].source_y, 0.0f, (float)(pSrc->m_Height-1));
	}

	// Clip the dest coordinates..
	bIsVisible = cis_Clip2dPoly(pCoords, nCoords, 0.0f, 0.0f, 
		(float)(pDest->m_Width-1), (float)(pDest->m_Height-1));
	if(!bIsVisible)
		return LT_OK;

	// Get the warp coordinates into the lookup tables.
	cis_GetWarpCoordinates(leftCoords, rightCoords, pCoords, nCoords, minY, maxY);

	// Draw it.
	return fn(pDest, pSrc, leftCoords, rightCoords, minY, maxY);
}


static LTRESULT cis_InternalTransformSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc,
	LTFloatPt *pRotOrigin, int destX, int destY, float angle, float scaleX, float scaleY,
	bool bTransparent, HLTCOLOR tColor)
{
	CisSurface *pSrc, *pDest;
	LTFloatPt rotOrigin;
	float halfSrcWidth, halfSrcHeight, ca, sa;
	LTWarpPt warpPoints[4];
	LTFloatPt points[4], translate, shiftBack;
	BlitRequest blitRequest;
	int i;
	
	pSrc = (CisSurface*)hSrc;
	pDest = (CisSurface*)hDest;
	if(!pSrc || !pDest)
		RETURN_ERROR(1, InternalTransformSurfaceToSurface, LT_INVALIDPARAMS);

	halfSrcWidth = (float)pSrc->m_Width * 0.5f;
	halfSrcHeight = (float)pSrc->m_Height * 0.5f;
	if(pRotOrigin)
	{
		rotOrigin = *pRotOrigin;
	}
	else
	{
		rotOrigin.x = (float)destX + halfSrcWidth;
		rotOrigin.y = (float)destY + halfSrcHeight;
	}

	
	// Setup the points at the rotation origin.
	translate.x = (float)destX - rotOrigin.x;
	translate.y = (float)destY - rotOrigin.y;
	points[0].x = translate.x;
	points[0].y = translate.y;
	points[1].x = translate.x + (float)pSrc->m_Width;
	points[1].y = translate.y;
	points[2].x = translate.x + (float)pSrc->m_Width;
	points[2].y = translate.y + (float)pSrc->m_Height;
	points[3].x = translate.x;
	points[3].y = translate.y + (float)pSrc->m_Height;

	// Scale, rotate, and shift them back to the destX, destY offsets.
	ca = (float)cos(angle);
	sa = (float)sin(angle);

	shiftBack.x = -translate.x + (float)destX;
	shiftBack.y = -translate.y + (float)destY;
	
	for(i=0; i < 4; i++)
	{
		points[i].x *= scaleX;
		points[i].y *= scaleY;

		warpPoints[i].dest_x = (ca * points[i].x) - (sa * points[i].y);
		warpPoints[i].dest_y = (sa * points[i].x) + (ca * points[i].y);
		warpPoints[i].dest_x += shiftBack.x;
		warpPoints[i].dest_y += shiftBack.y;
	}

	warpPoints[0].source_x = warpPoints[0].source_y = 0.0f;
	warpPoints[1].source_x = (float)(pSrc->m_Width-1);
	warpPoints[1].source_y = 0.0f;
	warpPoints[2].source_x = (float)(pSrc->m_Width-1);
	warpPoints[2].source_y = (float)(pSrc->m_Height-1);
	warpPoints[3].source_x = 0.0f;
	warpPoints[3].source_y = (float)(pSrc->m_Height - 1);

	// Try to translate the command to let the RenderStruct do it
	if(pDest == &g_ScreenSurface)
	{
		if(r_GetRenderStruct()->WarpToScreen)
		{
			// Clamp the source coordinates.
			warpPoints[0].source_x = LTCLAMP(warpPoints[0].source_x, 0.0f, (float)(pSrc->m_Width - 1));
			warpPoints[0].source_y = LTCLAMP(warpPoints[0].source_y, 0.0f, (float)(pSrc->m_Height - 1));
			warpPoints[1].source_x = LTCLAMP(warpPoints[1].source_x, 0.0f, (float)(pSrc->m_Width - 1));
			warpPoints[1].source_y = LTCLAMP(warpPoints[1].source_y, 0.0f, (float)(pSrc->m_Height - 1));
			warpPoints[2].source_x = LTCLAMP(warpPoints[2].source_x, 0.0f, (float)(pSrc->m_Width - 1));
			warpPoints[2].source_y = LTCLAMP(warpPoints[2].source_y, 0.0f, (float)(pSrc->m_Height - 1));
			warpPoints[3].source_x = LTCLAMP(warpPoints[3].source_x, 0.0f, (float)(pSrc->m_Width - 1));
			warpPoints[3].source_y = LTCLAMP(warpPoints[3].source_y, 0.0f, (float)(pSrc->m_Height - 1));

			// Clip the dest coordinates..
//			bIsVisible = cis_Clip2dPoly(warpPoints, 4, 0.0f, 0.0f, (float)(pDest->m_Width - 1), (float)(pDest->m_Height - 1));
//			if(!bIsVisible)
//				return LT_OK;

			// Setup source and destination rectangles for reference in the warp render function
			LTRect rSrcRect;
			LTRect rDestRect;
			rSrcRect.Init(0, 0, pSrc->m_Width - 1, pSrc->m_Height - 1);
			rDestRect.Init(0, 0, pDest->m_Width - 1, pDest->m_Height - 1);

			cis_OptimizeDirty(pSrc);

			blitRequest.m_hBuffer = pSrc->m_hBuffer;
			blitRequest.m_TransparentColor = g_TransparentColor;
			blitRequest.m_pSrcRect = &rSrcRect;
			blitRequest.m_pDestRect = &rDestRect;
			blitRequest.m_BlitOptions = bTransparent ? BLIT_TRANSPARENT : 0;
			blitRequest.m_Alpha = pSrc->m_Alpha;

			blitRequest.m_pWarpPts = warpPoints;
			blitRequest.m_nWarpPts = 4;

			// Do the blit unless there's something wrong...
			if(r_GetRenderStruct()->WarpToScreen(&blitRequest))
			{
				cis_SetDirty(pDest);
				return LT_OK;
			}
		}
	}

	// Run this case if any of the above fails
	if(bTransparent)
		return cis_WarpSurfaceToSurfaceTransparent(hDest, hSrc, warpPoints, 4, tColor);
	else
		return cis_WarpSurfaceToSurface(hDest, hSrc, warpPoints, 4);
}


// tType 0 = transparent
// tType 1 = solid
// tType 2 = solid color fill (for nontransparent pixels)

static LTRESULT cis_InternalScaleSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc, LTRect *pDestRect, LTRect *pSrcRect, int tType, HLTCOLOR tColor, HLTCOLOR fillColor)
{
	LTRect destRect, srcRect;
	CisSurface *pSrc, *pDest;
	LTWarpPt warpPoints[4];
	LTBOOL bIsInside;
	BlitRequest blitRequest;


	pSrc = (CisSurface*)hSrc;
	pDest = (CisSurface*)hDest;
	if(!pSrc || !pDest)
		RETURN_ERROR(4, InternalScaleSurfaceToSurface, LT_INVALIDPARAMS);

	if(pDestRect)
	{
		destRect = *pDestRect;
	}
	else
	{
		destRect.left = destRect.top = 0;
		destRect.right = pDest->m_Width;
		destRect.bottom = pDest->m_Height;
	}

	if(pSrcRect)
	{
		srcRect = *pSrcRect;
	}
	else
	{
		srcRect.left = srcRect.top = 0;
		srcRect.right = pSrc->m_Width;
		srcRect.bottom = pSrc->m_Height;
	}

	// Try to get the RenderStruct to do it.
	if(tType == 0 || tType == 1)
	{
		if(pDest == &g_ScreenSurface)
		{
			bIsInside = cis_ClipRectsScaled(
				pSrc->m_Width, pSrc->m_Height, srcRect.left, srcRect.top, srcRect.right, srcRect.bottom, 
				pDest->m_Width, pDest->m_Height, destRect.left, destRect.top, destRect.right, destRect.bottom,
				&srcRect, &destRect);
			
			if(bIsInside)
			{
				if(r_GetRenderStruct()->BlitToScreen)
				{
					cis_OptimizeDirty(pSrc);

					blitRequest.m_hBuffer = pSrc->m_hBuffer;
					blitRequest.m_TransparentColor = g_TransparentColor;
					blitRequest.m_pSrcRect = &srcRect;
					blitRequest.m_pDestRect = &destRect;
					blitRequest.m_BlitOptions = (tType == 0) ? BLIT_TRANSPARENT : 0;
					blitRequest.m_Alpha = pSrc->m_Alpha;
					blitRequest.m_bUseOld = true;
					
					r_GetRenderStruct()->BlitToScreen(&blitRequest);

					cis_SetDirty(pDest);
					return LT_OK;
				}
			}
		}
	}


	warpPoints[0].dest_x = (float)destRect.left;
	warpPoints[0].dest_y = (float)destRect.top;
	warpPoints[0].source_x = (float)srcRect.left;
	warpPoints[0].source_y = (float)srcRect.top;

	warpPoints[1].dest_x = (float)destRect.right;
	warpPoints[1].dest_y = (float)destRect.top;
	warpPoints[1].source_x = (float)srcRect.right;
	warpPoints[1].source_y = (float)srcRect.top;

	warpPoints[2].dest_x = (float)destRect.right;
	warpPoints[2].dest_y = (float)destRect.bottom;
	warpPoints[2].source_x = (float)srcRect.right;
	warpPoints[2].source_y = (float)srcRect.bottom;

	warpPoints[3].dest_x = (float)destRect.left;
	warpPoints[3].dest_y = (float)destRect.bottom;
	warpPoints[3].source_x = (float)srcRect.left;
	warpPoints[3].source_y = (float)srcRect.bottom;

	if(tType == 0)
		return cis_WarpSurfaceToSurfaceTransparent(hDest, hSrc, warpPoints, 4, tColor);
	else if(tType == 1)
		return cis_WarpSurfaceToSurface(hDest, hSrc, warpPoints, 4);
	else// if(tType == 2)
		return cis_WarpSurfaceToSurfaceSolidColor(hDest, hSrc, warpPoints, 4, tColor, fillColor);
}


static void cis_DeleteSurfaceBuffer(CisSurface *pSurface)
{
	if(pSurface->m_hBuffer)
	{
		ASSERT(g_pCisRenderStruct);
		
		g_pCisRenderStruct->DeleteSurface(pSurface->m_hBuffer);
		pSurface->m_hBuffer = LTNULL;
	}
}


static void cis_DeleteSurfaceBackupBuffer(CisSurface *pSurface)
{
	if(pSurface->m_pBackupBuffer)
	{
		dfree(pSurface->m_pBackupBuffer);
		pSurface->m_pBackupBuffer = LTNULL;
	}
}


static bool cis_BackupSurface(CisSurface *pSurface)
{
	// Don't back it up if it's already been deleted
	if (!pSurface->m_hBuffer)
		return true;

	uint32 y;
	uint8 *pSrcPos, *pDestPos, *pSrcBuffer;

	ASSERT(!pSurface->m_pBackupBuffer);
	ASSERT(g_pCisRenderStruct);

	// Setup the backup buffer.
	LT_MEM_TRACK_ALLOC(pSurface->m_pBackupBuffer = (uint8*)dalloc(g_nScreenPixelBytes * pSurface->m_Width * pSurface->m_Height),LT_MEM_TYPE_MISC);

	// Copy the pixels over.
	uint32 iPitch = 0;
	pSrcBuffer = (uint8*)g_pCisRenderStruct->LockSurface(pSurface->m_hBuffer,iPitch);
	if(!pSrcBuffer)
		return false;

	for(y=0; y < pSurface->m_Height; y++)
	{
		pSrcPos = &pSrcBuffer[y*iPitch];
		pDestPos = &pSurface->m_pBackupBuffer[y * pSurface->m_Width * g_nScreenPixelBytes];
		memcpy(pDestPos, pSrcPos, pSurface->m_Width * g_nScreenPixelBytes);
	}

	g_pCisRenderStruct->UnlockSurface(pSurface->m_hBuffer);

	// Get rid of the buffer.
	cis_DeleteSurfaceBuffer(pSurface);
	
	return true;
}


static bool cis_RestoreSurface(CisSurface *pSurface)
{
	uint32 surfWidth, surfHeight;
	FMConvertRequest request;
	LTRESULT dResult;


	ASSERT(!pSurface->m_hBuffer);
	ASSERT(pSurface->m_pBackupBuffer);
	ASSERT(g_pCisRenderStruct);

	// Recreate the surface.
	pSurface->m_hBuffer = g_pCisRenderStruct->CreateSurface(pSurface->m_Width, pSurface->m_Height);
	if(!pSurface->m_hBuffer)
		return false;

	g_pCisRenderStruct->GetSurfaceInfo(pSurface->m_hBuffer, &surfWidth, &surfHeight); //&pSurface->m_Pitch);

	// Restore the pixels.
	request.m_pDestFormat = &g_ScreenFormat; uint32 iPitch = 0;
	request.m_pDest = (uint8*)g_pCisRenderStruct->LockSurface(pSurface->m_hBuffer,iPitch);
	if(!request.m_pDest)
		return false;

	request.m_DestPitch = iPitch;
	
	request.m_pSrcFormat = &g_PrevScreenFormat;
	request.m_pSrc = pSurface->m_pBackupBuffer;
	request.m_SrcPitch = pSurface->m_Width * g_nPrevScreenPixelBytes;

	request.m_Width = pSurface->m_Width;
	request.m_Height = pSurface->m_Height;

	dResult = format_mgr->Mgr()->ConvertPixels(&request);
	g_pCisRenderStruct->UnlockSurface(pSurface->m_hBuffer);	

	if(dResult != LT_OK)
	{
		return false;
	}

	// Get rid of the backup buffer.
	cis_DeleteSurfaceBackupBuffer(pSurface);

	// Reoptimize it...
	if(pSurface->m_Flags & SURFFLAG_OPTIMIZED)
	{
		g_pCisRenderStruct->OptimizeSurface(pSurface->m_hBuffer, pSurface->m_OptimizedTransparentColor);
	}

	return true;
}


static bool cis_BackupSurfaces()
{
	GPOS pos;
	CisSurface *pSurface;

	for(pos=g_Surfaces; pos; )
	{
		pSurface = g_Surfaces.GetNext(pos);

		if(!cis_BackupSurface(pSurface))
			return false;
	}

	return true;
}


static bool cis_RestoreSurfaces()
{
	GPOS pos;
	CisSurface *pSurface;

	for(pos=g_Surfaces; pos; )
	{
		pSurface = g_Surfaces.GetNext(pos);

		if(!cis_RestoreSurface(pSurface))
			return false;
	}

	return true;
}


static void cis_DeleteSurfaces()
{
	GPOS pos;
	CisSurface *pSurface;

	for(pos=g_Surfaces; pos; )
	{
		pSurface = g_Surfaces.GetNext(pos);

		cis_DeleteSurfaceBackupBuffer(pSurface);
		cis_DeleteSurfaceBuffer(pSurface);
		
		sb_Free(&g_SurfaceBank, pSurface);
	}

	g_Surfaces.RemoveAll();
}

CisSurface* cis_InternalCreateSurface(uint32 width, uint32 height)
{
	CisSurface *pSurface;
	HLTBUFFER hBuffer;
	uint32 surfWidth, surfHeight;
//	long surfPitch;

	if(width == 0 || height == 0 || width > 5000 || height > 5000)
		return LTNULL;

	if(!g_pCisRenderStruct)
		return LTNULL;

	hBuffer = g_pCisRenderStruct->CreateSurface(width, height);
	if(!hBuffer)
		return LTNULL;

	g_pCisRenderStruct->GetSurfaceInfo(hBuffer, &surfWidth, &surfHeight); //, &surfPitch);

	pSurface = (CisSurface*)sb_Allocate(&g_SurfaceBank);
	pSurface->m_hBuffer = hBuffer;
	pSurface->m_pBackupBuffer = LTNULL;
	pSurface->m_Width = surfWidth;
	pSurface->m_Height = surfHeight;
//	pSurface->m_Pitch = surfPitch;
	pSurface->m_Flags = 0;
	pSurface->m_pUserData = LTNULL;
	pSurface->m_Alpha = 1.0f;

	g_Surfaces.AddHead(pSurface);	
	return pSurface;
}

// ----------------------------------------------------------------- //
// Interface implementation functions.
// ----------------------------------------------------------------- //
static HLTCOLOR cis_CreateColor(float r, float g, float b, bool bTransparent)
{
	if(bTransparent)
	{
		return SETRGB_FT(r,g,b);
	}
	else
	{
		return SETRGB_F(r,g,b);
	}
}

static void cis_DeleteColor(HLTCOLOR hColor)
{
}

static HLTCOLOR cis_SetupColor1(float r, float g, float b, bool bTransparent)
{
	return cis_CreateColor(r, g, b, bTransparent);
}

static HLTCOLOR cis_SetupColor2(float r, float g, float b, bool bTransparent)
{
	return cis_CreateColor(r, g, b, bTransparent);
}


static bool IsVertSpanSolidColor(uint8 *pBuf, GenericColor color, uint32 height, long pitch)
{
	if(g_ScreenFormat.GetType() == BPP_16)
	{
		while(height)
		{
			if(*((uint16*)pBuf) != color.wVal)
				return false;
			
			pBuf += pitch;
			--height;
		}
	}
	else if(g_ScreenFormat.GetType() == BPP_32)
	{
		while(height)
		{
			if(*((uint32*)pBuf) != color.dwVal)
				return false;
			
			pBuf += pitch;
			--height;
		}
	}

	return true;
}

static bool IsHorzSpanSolidColor(uint8 *pBuf, GenericColor color, uint32 width)
{
	if(g_ScreenFormat.GetType() == BPP_16)
	{
		while(width)
		{
			if(*((uint16*)pBuf) != color.wVal)
				return false;
			
			--width;
			pBuf += sizeof(uint16);
		}
	}
	else if(g_ScreenFormat.GetType() == BPP_32)
	{
		while(width)
		{
			if(*((uint32*)pBuf) != color.dwVal)
				return false;
			
			--width;
			pBuf += sizeof(uint32);
		}
	}

	return true;
}


static LTRESULT cis_GetBorderSize(HSURFACE hSurface, HLTCOLOR hColor, LTRect *pRect)
{
	CisSurface *pSurface;
	uint8 *pBuf;
	GenericColor theColor;

	pSurface = (CisSurface*)hSurface;
	if (!pSurface || !pRect || !pSurface->m_hBuffer) {
		RETURN_ERROR(1, GetBorderSize, LT_INVALIDPARAMS); }
	else if (!g_pCisRenderStruct) {
		RETURN_ERROR(1, GetBorderSize, LT_NOTINITIALIZED); }

	uint32 iPitch = 0;
	pBuf = (uint8*)g_pCisRenderStruct->LockSurface(pSurface->m_hBuffer,iPitch);
	if (!pBuf) { RETURN_ERROR(1, GetBorderSize, LT_ERROR); }

	pRect->left = pRect->top = pRect->right = pRect->bottom = 0;
	format_mgr->Mgr()->PValueToFormatColor(&g_ScreenFormat, hColor, theColor);

	// Test each side.
	while (pRect->left < (int)pSurface->m_Width && IsVertSpanSolidColor(&pBuf[pRect->left], theColor, pSurface->m_Height, iPitch)) {
		++pRect->left; }

	while (pRect->right < (int)pSurface->m_Width && IsVertSpanSolidColor(&pBuf[pSurface->m_Width-pRect->right-1], theColor, pSurface->m_Height, iPitch)) {
		++pRect->right; }

	while(pRect->top < (int)pSurface->m_Height && IsHorzSpanSolidColor(&pBuf[pRect->top*iPitch], theColor, pSurface->m_Width)) {
		++pRect->top; }

	while(pRect->bottom < (int)pSurface->m_Height && IsHorzSpanSolidColor(&pBuf[(pSurface->m_Height-pRect->bottom-1)*iPitch], theColor, pSurface->m_Width)) {
		++pRect->bottom; }
		
	g_pCisRenderStruct->UnlockSurface(pSurface->m_hBuffer);	
	return LT_OK;
}


static LTRESULT cis_OptimizeSurface(HSURFACE hSurface, HLTCOLOR hTransparentColor)
{
	CisSurface *pSurface;

	if(!hSurface)
		RETURN_ERROR(1, OptimizeSurface, LT_INVALIDPARAMS);

	pSurface = (CisSurface*)hSurface;
	pSurface->m_OptimizedTransparentColor = (uint32)hTransparentColor;

	// The render driver wants a uint32 color so either get rid of the 
	// COLOR_TRANSPARENCY_MASK or set it to OPTIMIZE_NO_TRANSPARENCY.	
	if(pSurface->m_OptimizedTransparentColor & COLOR_TRANSPARENCY_MASK)
		pSurface->m_OptimizedTransparentColor &= ~COLOR_TRANSPARENCY_MASK;
	else
		pSurface->m_OptimizedTransparentColor = OPTIMIZE_NO_TRANSPARENCY;

	pSurface->m_Flags |= SURFFLAG_OPTIMIZED;

	if(g_pCisRenderStruct && g_pCisRenderStruct->m_bInitted)
	{
		if(g_pCisRenderStruct->OptimizeSurface(pSurface->m_hBuffer, pSurface->m_OptimizedTransparentColor))
		{
			pSurface->m_Flags &= ~SURFFLAG_OPTIMIZEDIRTY;
			return LT_OK;
		}
		else
		{
			return LT_ERROR;
		}
	}
	else
	{
		// Just return out.. we'll optimize it when we recreate the surface.
		return LT_OK;
	}
}


static LTRESULT cis_UnoptimizeSurface(HSURFACE hSurface)
{
	CisSurface *pSurface;

	if(!hSurface)
		RETURN_ERROR(1, OptimizeSurface, LT_INVALIDPARAMS);

	pSurface = (CisSurface*)hSurface;
	if(!(pSurface->m_Flags & SURFFLAG_OPTIMIZED))
		return LT_OK;

	pSurface->m_Flags &= ~SURFFLAG_OPTIMIZED;
	if(g_pCisRenderStruct->m_bInitted)
	{
		g_pCisRenderStruct->UnoptimizeSurface(pSurface->m_hBuffer);
		return LT_OK;
	}
	else
	{
		return LT_OK;
	}
}


static HSURFACE cis_GetScreenSurface()
{
	if(!g_pCisRenderStruct)
		return LTNULL;

	return (HSURFACE)&g_ScreenSurface;
}


static bool cis_LoadPcx(const char *pBitmapName, LoadedBitmap *pBitmap)
{
	ILTStream *pStream;
	FileRef ref;
	bool bRet;

	ref.m_FileType = FILE_CLIENTFILE;
	ref.m_pFilename = pBitmapName;

	pStream = client_file_mgr->OpenFile(&ref);
	if(!pStream)
		return LTNULL;

	bRet = pcx_Create2(pStream, pBitmap) != 0;
	pStream->Release();
	return bRet;
}


HSURFACE cis_CreateSurfaceFromPcx(LoadedBitmap *pLoadedBitmap)
{
	CisSurface *pSurface;

	// Create a surface for it.
	pSurface = cis_InternalCreateSurface(pLoadedBitmap->m_Width, pLoadedBitmap->m_Height);
	if(!pSurface)
	{
		return LTNULL;
	}

	cis_InternalBitmapToSurface(pSurface, pLoadedBitmap, LTNULL, 0, 0);
	return (HSURFACE)pSurface;
}


static LTRESULT cis_CreateHeightmapFromBitmap(const char* pBitmap, uint32* pWidth, uint32* pHeight, uint8** ppData)
{
	//check the params
	if(!pBitmap || !pWidth || !pHeight || !ppData)
		return LT_INVALIDPARAMS;

	//load up the bitmap
	LoadedBitmap Bitmap;	
	if(!cis_LoadPcx((char*)pBitmap, &Bitmap))
		return LT_ERROR;

	//now we need to convert the image data over to the pData

	//create the buffer
	LT_MEM_TRACK_ALLOC(*ppData		= new uint8 [Bitmap.m_Width * Bitmap.m_Height],LT_MEM_TYPE_HEIGHTMAP);
	*pWidth		= Bitmap.m_Width;
	*pHeight	= Bitmap.m_Height;

	//check the allocation
	if(!*ppData)
		return LT_ERROR;

	//create a greyscale palette
	uint8 nPalette[256];

	for(uint32 nCurrColor = 0; nCurrColor < 256; nCurrColor++)
	{
		nPalette[nCurrColor] =	(uint8)(((uint32)Bitmap.m_Palette[nCurrColor].rgb.r + 
										 (uint32)Bitmap.m_Palette[nCurrColor].rgb.g + 
										 (uint32)Bitmap.m_Palette[nCurrColor].rgb.b) / 3);
	}

	//the current pixel being modified
	uint8* pCurr = *ppData;

	//now copy the data over and convert
	for(uint32 nCurrY = 0; nCurrY < Bitmap.m_Height; nCurrY++)
	{
		for(uint32 nCurrX = 0; nCurrX < Bitmap.m_Width; nCurrX++)
		{
			//convert the pixel to a uint8
			*pCurr = nPalette[Bitmap.Pixel(nCurrX, nCurrY)];
			++pCurr;
		}
	}

	//success
	return LT_OK;
}

static LTRESULT cis_FreeHeightmap(uint8* pData)
{
	//check the parameters for validity
	if(pData == NULL)
		return LT_INVALIDPARAMS;

	delete [] pData;

	return LT_OK;
}


static HSURFACE cis_CreateSurfaceFromBitmap(char *pBitmapName)
{
	LoadedBitmap bitmap;
	HSURFACE hRet;

	
	if(!cis_LoadPcx(pBitmapName, &bitmap))
		return LTNULL;

	hRet = cis_CreateSurfaceFromPcx(&bitmap);
	return hRet;
}


static HSURFACE cis_CreateSurface(uint32 width, uint32 height)
{
	return (HSURFACE)cis_InternalCreateSurface(width, height);
}


LTRESULT cis_DeleteSurface(HSURFACE hSurface)
{
	CisSurface *pSurface = (CisSurface*)hSurface;

	if(!pSurface)
	{
		RETURN_ERROR_PARAM(1, ILTClient::DeleteSurface, LT_ERROR, "(null surface)");
	}

	if(!g_pCisRenderStruct)
	{
		RETURN_ERROR_PARAM(1, ILTClient::DeleteSurface, LT_ERROR, "(renderer not initialized)");
	}

	if(cis_IsScreenSurface(pSurface))
	{
	}
	else
	{
		g_Surfaces.RemoveAt(pSurface);
		cis_DeleteSurfaceBackupBuffer(pSurface);
		cis_DeleteSurfaceBuffer(pSurface);
		sb_Free(&g_SurfaceBank, pSurface);
	}

	return LT_OK;
}


static void* cis_GetSurfaceUserData(HSURFACE hSurf)
{
	if(hSurf)
	{
		return ((CisSurface*)hSurf)->m_pUserData;
	}
	else
	{
		return LTNULL;
	}
}


static void cis_SetSurfaceUserData(HSURFACE hSurf, void *pUserData)
{
	if(hSurf)
	{
		((CisSurface*)hSurf)->m_pUserData = pUserData;
	}
}


LTRESULT cis_GetPixel(HSURFACE hSurface, uint32 x, uint32 y, HLTCOLOR *color)
{
	CisSurface *pSurface;
	uint8 *pBuffer;

	pSurface = (CisSurface*)hSurface;
	if(!pSurface || x >= pSurface->m_Width || y >= pSurface->m_Height)
		RETURN_ERROR(1, ILTClient::GetPixel, LT_INVALIDPARAMS);

	uint32 iPitch = 0;
	pBuffer = (uint8*)g_pCisRenderStruct->LockSurface(pSurface->m_hBuffer,iPitch);
	if(!pBuffer)
		RETURN_ERROR(1, ILTClient::GetPixel, LT_ERROR);

	pBuffer += y*iPitch + x*g_nScreenPixelBytes;
	format_mgr->Mgr()->PValueFromFormatColor(&g_ScreenFormat, *((GenericColor*)pBuffer), *color);

	g_pCisRenderStruct->UnlockSurface(pSurface->m_hBuffer);
	return LT_OK;
}


LTRESULT cis_SetPixel(HSURFACE hSurface, uint32 x, uint32 y, HLTCOLOR color)
{
	CisSurface *pSurface;
	uint8 *pBuffer;

	pSurface = (CisSurface*)hSurface;
	if(!pSurface || x >= pSurface->m_Width || y >= pSurface->m_Height)
		RETURN_ERROR(1, ILTClient::SetPixel, LT_INVALIDPARAMS);

	uint32 iPitch = 0;
	pBuffer = (uint8*)g_pCisRenderStruct->LockSurface(pSurface->m_hBuffer,iPitch);
	if(!pBuffer)
		RETURN_ERROR(1, ILTClient::SetPixel, LT_ERROR);

	pBuffer += y*iPitch + x*g_nScreenPixelBytes;
	format_mgr->Mgr()->PValueToFormatColor(&g_ScreenFormat, color, *((GenericColor*)pBuffer));

	g_pCisRenderStruct->UnlockSurface(pSurface->m_hBuffer);
	cis_SetDirty(pSurface);
	return LT_OK;
}


static void cis_GetSurfaceDims(HSURFACE hSurf, uint32 *pWidth, uint32 *pHeight)
{
	CisSurface *pSurface = (CisSurface*)hSurf;

	if(pSurface)
	{
		if(pWidth)
			*pWidth = pSurface->m_Width;
		
		if(pHeight)
			*pHeight = pSurface->m_Height;
	}
	else
	{
		*pWidth = *pHeight = 0;
	}
}

static bool cis_DrawBitmapToSurface(HSURFACE hDest, const char *pSourceBitmapName, 
	const LTRect *pSrcRect, int destX, int destY)
{
	LoadedBitmap bitmap;
	CisSurface *pSurface;


	pSurface = (CisSurface*)hDest;
	if(!pSurface)
		return false;

	if(!cis_LoadPcx(pSourceBitmapName, &bitmap))
		return false;

	cis_InternalBitmapToSurface(pSurface, &bitmap, pSrcRect, destX, destY);
	return true;
}

static LTRESULT cis_DrawSurfaceSolidColor(HSURFACE hDest, HSURFACE hSrc,
	LTRect *pSrcRect, int destX, int destY, HLTCOLOR hTransColor, HLTCOLOR hFillColor)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, DrawSurfaceSolidColor, LT_NOTINITIALIZED);

	cis_SetTransparentColor(hTransColor);
	cis_SetSolidColor(hFillColor);
	return cis_DoDrawSurfaceToSurface(hDest, hSrc, pSrcRect, destX, destY, cis_SolidColorDraw);
}

static LTRESULT cis_DrawSurfaceMasked(HSURFACE hDest, HSURFACE hSrc, HSURFACE hMask,
	LTRect *pSrcRect, int destX, int destY, HLTCOLOR hColor)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, DrawSurfaceMasked, LT_NOTINITIALIZED);

	g_pMask = (CisSurface*)hMask;
	cis_SetTransparentColor(hColor);
	return cis_DoDrawSurfaceToSurface(hDest, hSrc, pSrcRect, destX, destY, cis_MaskedDraw);
}

LTRESULT cis_DrawSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc, 
	LTRect *pSrcRect, int destX, int destY)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, DrawSurfaceToSurface, LT_NOTINITIALIZED);

	return cis_DoDrawSurfaceToSurface(hDest, hSrc, pSrcRect, destX, destY, cis_OpaqueDraw);
}


static LTRESULT cis_DrawSurfaceToSurfaceTransparent(HSURFACE hDest, HSURFACE hSrc, 
	LTRect *pSrcRect, int destX, int destY, HLTCOLOR hColor)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, DrawSurfaceToSurfaceTransparent, LT_NOTINITIALIZED);

	cis_SetTransparentColor(hColor);
	return cis_DoDrawSurfaceToSurface(hDest, hSrc, pSrcRect, destX, destY, cis_TransparentDraw);
}


static LTRESULT cis_ScaleSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc,
	LTRect *pDestRect, LTRect *pSrcRect)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(4, ScaleSurfaceToSurface, LT_NOTINITIALIZED);

	return cis_InternalScaleSurfaceToSurface(hDest, hSrc, pDestRect, pSrcRect, 1, LTNULL, LTNULL);
}


static LTRESULT cis_ScaleSurfaceToSurfaceTransparent(HSURFACE hDest, HSURFACE hSrc,
	LTRect *pDestRect, LTRect *pSrcRect, HLTCOLOR hColor)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, ScaleSurfaceToSurfaceTransparent, LT_NOTINITIALIZED);

	cis_SetTransparentColor(hColor);
	return cis_InternalScaleSurfaceToSurface(hDest, hSrc, pDestRect, pSrcRect, 0, hColor, LTNULL);
}


static LTRESULT cis_ScaleSurfaceToSurfaceSolidColor(HSURFACE hDest, HSURFACE hSrc,
	LTRect *pDestRect, LTRect *pSrcRect, HLTCOLOR hTransColor, HLTCOLOR hFillColor)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, ScaleSurfaceToSurfaceSolidColor, LT_NOTINITIALIZED);

	return cis_InternalScaleSurfaceToSurface(hDest, hSrc, pDestRect, pSrcRect, 2, hTransColor, hFillColor);
}


static LTRESULT cis_WarpSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc, 
	LTWarpPt *pCoords, int nCoords)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, WarpSurfaceToSurface, LT_NOTINITIALIZED);

	return cis_InternalWarpSurfaceToSurface(hDest, hSrc, pCoords, nCoords, cis_DrawWarp);
}


static LTRESULT cis_WarpSurfaceToSurfaceTransparent(HSURFACE hDest, HSURFACE hSrc, 
	LTWarpPt *pCoords, int nCoords, HLTCOLOR hColor)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, WarpSurfaceToSurfaceTransparent, LT_NOTINITIALIZED);

	cis_SetTransparentColor(hColor);
	return cis_InternalWarpSurfaceToSurface(hDest, hSrc, pCoords, nCoords, cis_DrawWarpTransparent);
}


static LTRESULT cis_WarpSurfaceToSurfaceSolidColor(HSURFACE hDest, HSURFACE hSrc, 
	LTWarpPt *pCoords, int nCoords, HLTCOLOR hTransColor, HLTCOLOR hFillColor)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, WarpSurfaceToSurfaceSolidColor, LT_NOTINITIALIZED);

	cis_SetTransparentColor(hTransColor);
	cis_SetSolidColor(hFillColor);
	return cis_InternalWarpSurfaceToSurface(hDest, hSrc, pCoords, nCoords, cis_DrawWarpSolidColor);
}


static LTRESULT cis_TransformSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc,
	LTFloatPt *pRotOrigin, int destX, int destY, float angle, float scaleX, float scaleY)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, TransformSurfaceToSurface, LT_NOTINITIALIZED);

	return cis_InternalTransformSurfaceToSurface(hDest, hSrc, pRotOrigin, destX, destY,
		angle, scaleX, scaleY, false, 0);
}


static LTRESULT cis_TransformSurfaceToSurfaceTransparent(HSURFACE hDest, HSURFACE hSrc,
	LTFloatPt *pRotOrigin, int destX, int destY, float angle, float scaleX, float scaleY,
	HLTCOLOR hColor)
{
	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, TransformSurfaceToSurfaceTransparent, LT_NOTINITIALIZED);

	cis_SetTransparentColor(hColor);
	return cis_InternalTransformSurfaceToSurface(hDest, hSrc, pRotOrigin, destX, destY,
		angle, scaleX, scaleY, true, hColor);
}


static LTRESULT cis_FillRect(HSURFACE hDest, LTRect *pRect, HLTCOLOR hColor)
{
	LTRect theRect, tempRect;
	LTBOOL bIsVisible;
	CisSurface *pDest;
	FMRectRequest request;
	LTRESULT dResult;


	if(!g_pCisRenderStruct)
		RETURN_ERROR(1, FillRect, LT_NOTINITIALIZED);

	pDest = (CisSurface*)hDest;
	if(!pDest)
		RETURN_ERROR(1, FillRect, LT_INVALIDPARAMS);

	if(pRect)
	{
		tempRect.left = tempRect.top = 0;
		tempRect.right = pDest->m_Width;
		tempRect.bottom = pDest->m_Height;
		bIsVisible = cis_RectIntersection(&theRect, &tempRect, pRect);
		if(!bIsVisible)
			return LT_OK;
	}
	else
	{
		theRect.left = theRect.top = 0;
		theRect.right = pDest->m_Width;
		theRect.bottom = pDest->m_Height;
	}

	request.m_pDest = (uint8*)cis_LockSurface(pDest, request.m_DestPitch, true);
	if(request.m_pDest)
	{
		request.m_pDestFormat = &g_ScreenFormat;
		request.m_Rect = theRect;
		request.m_Color = hColor;

		dResult = format_mgr->Mgr()->FillRect(&request);
		cis_UnlockSurface(pDest);
		return dResult;
	}
	else
	{
		RETURN_ERROR(1, FillRect, LT_ERROR);
	}
}


static LTRESULT cis_GetSurfaceAlpha(HSURFACE hSurface, float &alpha)
{
	FN_NAME(cis_GetSurfaceAlpha);
	CisSurface *pSurface;

	CHECK_PARAMS2(hSurface);
	
	pSurface = (CisSurface*)hSurface;
	alpha = pSurface->m_Alpha;
	return LT_OK;
}


static LTRESULT cis_SetSurfaceAlpha(HSURFACE hSurface, float alpha)
{
	FN_NAME(cis_SetSurfaceAlpha);
	CisSurface *pSurface;

	CHECK_PARAMS2(hSurface);
	
	pSurface = (CisSurface*)hSurface;
	pSurface->m_Alpha = LTCLAMP(alpha, 0.0f, 1.0f);
	return LT_OK;
}


static LTRESULT cis_GetEngineHook(char *pName, void **pData)
{
	if(stricmp(pName, "hwnd") == 0)
	{
		*pData = g_ClientGlob.m_hMainWnd;
		return LT_OK;
	}
	else if(stricmp(pName, "cres_hinstance")==0)
	{
		return bm_GetInstanceHandle(g_pClientMgr->m_hClientResourceModule, pData);
	}
	else if(stricmp(pName, "cresl_hinstance")==0)
	{
		return bm_GetInstanceHandle(g_pClientMgr->m_hLocalizedClientResourceModule, pData);
	}
	else if(stricmp(pName, "cshell_hinstance")==0)
	{
		return bm_GetInstanceHandle(g_pClientMgr->m_hShellModule, pData);
	}
	else if(stricmp(pName, "d3ddevice")==0)
	{
		//IDirect3DDevice9*
		*pData = (void*)r_GetRenderStruct()->GetD3DDevice();
		return LT_OK;
	}
	
	return LT_ERROR;
}

static LTRESULT cis_QueryGraphicDevice(LTGraphicsCaps* pCaps)
{
	if(!r_GetRenderStruct())
	{
		return LT_ERROR;
	}

	if(!r_GetRenderStruct()->GetD3DDevice())
	{
		return LT_ERROR;
	}

	D3DCAPS9 caps;
	HRESULT hres = r_GetRenderStruct()->GetD3DDevice()->GetDeviceCaps(&caps);

	if(hres != D3D_OK)
	{
		return LT_ERROR;
	}

	pCaps->PixelShaderVersion = (uint32)caps.PixelShaderVersion;
	pCaps->VertexShaderVersion = (uint32)caps.VertexShaderVersion;
	

	return LT_OK;
}


static LTRESULT cis_FlipScreen(uint32 flags)
{
	if(!r_GetRenderStruct()->m_bInitted)
		RETURN_ERROR(1, FlipScreen, LT_NOTINITIALIZED);

	// Clear out in 3d
	while (r_GetRenderStruct()->IsIn3D())
	{
		cis_End3D(END3D_CANDRAWCONSOLE);
	}

	g_pClientMgr->ShowDrawSurface(flags);	
	return LT_OK;
}


static LTRESULT cis_ClearScreen(LTRect *pRect, uint32 flags, LTRGB* pClearColor)
{
	LTRect clearRect;

	if(!r_GetRenderStruct()->m_bInitted)
		RETURN_ERROR(1, ClearScreen, LT_NOTINITIALIZED);

	if(pRect)
	{
		memcpy(&clearRect, pRect, sizeof(LTRect));

		clearRect.left = LTCLAMP(clearRect.left, 0, (int)r_GetRenderStruct()->m_Width);
		clearRect.right = LTCLAMP(clearRect.right, 0, (int)r_GetRenderStruct()->m_Width);
		clearRect.top = LTCLAMP(clearRect.top, 0, (int)r_GetRenderStruct()->m_Height);
		clearRect.bottom = LTCLAMP(clearRect.bottom, 0, (int)r_GetRenderStruct()->m_Height);
	}
	else
	{
		clearRect.left = clearRect.top = 0;
		clearRect.right = r_GetRenderStruct()->m_Width;
		clearRect.bottom = r_GetRenderStruct()->m_Height;
	}

	if(g_CV_ForceClear)
		flags |= CLEARSCREEN_SCREEN;

	LTRGBColor ClearColor;
	if (pClearColor) { ClearColor.rgb.a = pClearColor->a; ClearColor.rgb.r = pClearColor->r; ClearColor.rgb.g = pClearColor->g; ClearColor.rgb.b = pClearColor->b; }
	else { ClearColor.rgb.a = 0; ClearColor.rgb.r = 0; ClearColor.rgb.g = 0; ClearColor.rgb.b = 0; }

	r_GetRenderStruct()->Clear(&clearRect, flags, ClearColor);
	return LT_OK;
}


static LTRESULT cis_Start3D()
{
	if(!r_GetRenderStruct()->m_bInitted)
		RETURN_ERROR(1, Start3D, LT_NOTINITIALIZED);

	++(r_GetRenderStruct()->m_nIn3D);

	if(r_GetRenderStruct()->m_nIn3D != 1)
	{
		// Act like it's all cool..
		return LT_OK;
	}

	r_GetRenderStruct()->Start3D();
	return LT_OK;
}


static LTRESULT cis_RenderCamera(HLOCALOBJ hObj, float fFrameTime)
{
	CameraInstance *pCamera;

	if(!r_GetRenderStruct()->m_bInitted)
		RETURN_ERROR(15, RenderCamera, LT_NOTINITIALIZED);

	if(!r_GetRenderStruct()->IsIn3D())
		RETURN_ERROR(15, RenderCamera, LT_NOTIN3D);

	pCamera = (CameraInstance*)hObj;
	if(!pCamera || pCamera->m_ObjectType != OT_CAMERA)
		RETURN_ERROR(15, RenderCamera, LT_INVALIDPARAMS);

	g_pClientMgr->Render(pCamera, DRAWMODE_NORMAL, LTNULL, 0, fFrameTime);
	return LT_OK;
}

static LTRESULT cis_MakeCubicEnvMap(HLOCALOBJ hCamera, uint32 nSize, const char* pszFilePrefix)
{

	if(!r_GetRenderStruct()->m_bInitted)
		RETURN_ERROR(15, MakeCubicEnvMap, LT_NOTINITIALIZED);

	if(r_GetRenderStruct()->IsIn3D())
		RETURN_ERROR(15, MakeCubicEnvMap, LT_ALREADYIN3D);

	CameraInstance *pCamera = (CameraInstance*)hCamera;

	if(!pCamera || pCamera->m_ObjectType != OT_CAMERA)
		RETURN_ERROR(15, MakeCubicEnvMap, LT_INVALIDPARAMS);

	g_pClientMgr->MakeCubicEnvMap(pCamera, nSize, pszFilePrefix);
	return LT_OK;

}

static LTRESULT cis_RenderObjects(HLOCALOBJ hCamera, HLOCALOBJ *pObjects, int nObjects, float fFrameTime)
{
	CameraInstance *pCamera;

	if(!r_GetRenderStruct()->m_bInitted)
		RETURN_ERROR(1, RenderCamera, LT_NOTINITIALIZED);

	if(!r_GetRenderStruct()->IsIn3D())
		RETURN_ERROR(1, RenderCamera, LT_NOTIN3D);

	pCamera = (CameraInstance*)hCamera;
	if(!pCamera || pCamera->m_ObjectType != OT_CAMERA)
		RETURN_ERROR(1, RenderCamera, LT_INVALIDPARAMS);

	if(!pObjects || nObjects <= 0)
		return LT_OK;

	if(g_pClientMgr->Render(pCamera, DRAWMODE_OBJECTLIST, (LTObject**)pObjects, nObjects, fFrameTime))
		return LT_OK;
	else
		RETURN_ERROR(1, RenderObjects, LT_ERROR);
}


static LTRESULT cis_StartOptimized2D()
{
	RenderStruct *pStruct;

	pStruct = r_GetRenderStruct();
	if(!pStruct->m_bInitted)
		RETURN_ERROR(1, StartOptimized2D, LT_NOTINITIALIZED);

	if(!pStruct->IsIn3D())
		RETURN_ERROR(1, StartOptimized2D, LT_NOTIN3D);
	
	++(pStruct->m_nInOptimized2D);

	if (pStruct->m_nInOptimized2D == 1)
		pStruct->StartOptimized2D();

	return LT_OK;
}


static LTRESULT cis_EndOptimized2D()
{
	RenderStruct *pStruct;

	pStruct = r_GetRenderStruct();
	if(!pStruct->m_bInitted)
		RETURN_ERROR(1, EndOptimized2D, LT_NOTINITIALIZED);

	if(!pStruct->IsIn3D())
		RETURN_ERROR(1, EndOptimized2D, LT_NOTIN3D);
	
	if(!pStruct->IsInOptimized2D())
		RETURN_ERROR(1, EndOptimized2D, LT_NOTIN3D);
	
	--(pStruct->m_nInOptimized2D);

	if (pStruct->m_nInOptimized2D == 0)
		pStruct->EndOptimized2D();

	return LT_OK;
}


static LTRESULT cis_SetOptimized2DBlend(LTSurfaceBlend blend)
{
	RenderStruct *pStruct;

	pStruct = r_GetRenderStruct();
	if(!pStruct->m_bInitted)
		RETURN_ERROR(1, SetOptimized2DBlend, LT_NOTINITIALIZED);

	if(!pStruct->IsIn3D())
		RETURN_ERROR(1, SetOptimized2DBlend, LT_NOTIN3D);
	
	// This shouldn't ever fail unless an invalid blend mode is specified
	bool bResult = pStruct->SetOptimized2DBlend(blend);
	ASSERT(bResult);
	return (bResult) ? LT_OK : LT_ERROR;
}

static LTRESULT cis_GetOptimized2DBlend(LTSurfaceBlend &blend)
{
	RenderStruct *pStruct;

	pStruct = r_GetRenderStruct();
	if(!pStruct->m_bInitted)
		RETURN_ERROR(1, GetOptimized2DBlend, LT_NOTINITIALIZED);

	// This shouldn't ever fail unless an invalid blend mode is specified
	bool bResult = pStruct->GetOptimized2DBlend(blend);
	ASSERT(bResult);
	return (bResult) ? LT_OK : LT_ERROR;
}


static LTRESULT cis_SetOptimized2DColor(HLTCOLOR hColor)
{
	RenderStruct *pStruct;

	pStruct = r_GetRenderStruct();
	if(!pStruct->m_bInitted)
		RETURN_ERROR(1, SetOptimized2DColor, LT_NOTINITIALIZED);

	if(!pStruct->IsIn3D())
		RETURN_ERROR(1, SetOptimized2DColor, LT_NOTIN3D);
	
	// This shouldn't ever fail unless an invalid blend mode is specified
	bool bResult = pStruct->SetOptimized2DColor(hColor);
	ASSERT(bResult);
	return (bResult) ? LT_OK : LT_ERROR;
}

static LTRESULT cis_GetOptimized2DColor(HLTCOLOR &hColor)
{
	RenderStruct *pStruct;

	pStruct = r_GetRenderStruct();
	if(!pStruct->m_bInitted)
		RETURN_ERROR(1, GetOptimized2DColor, LT_NOTINITIALIZED);

	// This shouldn't ever fail unless an invalid blend mode is specified
	bool bResult = pStruct->GetOptimized2DColor(hColor);
	ASSERT(bResult);
	return (bResult) ? LT_OK : LT_ERROR;
}


static LTRESULT cis_End3D(uint flags)
{
	if(!r_GetRenderStruct()->m_bInitted)
		RETURN_ERROR(1, End3D, LT_NOTINITIALIZED);

	if(!r_GetRenderStruct()->IsIn3D())
	{
		// Make sure our In3D count is correct...  This can happen in some strange loss-of-focus on startup situations
		r_GetRenderStruct()->m_nIn3D = 0;
		RETURN_ERROR(1, End3D, LT_NOTIN3D);
	}

	// Make sure we're not in optimized 2D
	while (r_GetRenderStruct()->m_nInOptimized2D)
	{
		cis_EndOptimized2D();
	}

	// Only do end3d once for the entire pair set
	--(r_GetRenderStruct()->m_nIn3D);

	if(r_GetRenderStruct()->m_nIn3D != 0)
	{
		return LT_OK;
	}

#ifndef _FINAL
	if ((flags & END3D_CANDRAWCONSOLE) != 0) {
		CountAdder cTicks_Console(&g_Ticks_Render_Console);
		if (dsi_IsConsoleUp()) {
			con_Draw(); }
		else if(g_nConsoleLines > 0) {
			con_DrawSmall( g_nConsoleLines ); }	} // Show the last 4 or so lines.
#endif _FINAL

	r_GetRenderStruct()->End3D();
	return LT_OK;
}


// ----------------------------------------------------------------- //
// Interface functions.
// ----------------------------------------------------------------- //



void cis_Init()
{
	uint32 i;

	// Init the interface pointers.
	ilt_client->FlipScreen = cis_FlipScreen;
	ilt_client->ClearScreen = cis_ClearScreen;
	ilt_client->Start3D = cis_Start3D;
	ilt_client->RenderCamera = cis_RenderCamera;
	ilt_client->RenderObjects = cis_RenderObjects;
	ilt_client->MakeCubicEnvMap = cis_MakeCubicEnvMap;
	ilt_client->StartOptimized2D = cis_StartOptimized2D;
	ilt_client->EndOptimized2D = cis_EndOptimized2D;
	ilt_client->SetOptimized2DBlend = cis_SetOptimized2DBlend;
	ilt_client->SetOptimized2DColor = cis_SetOptimized2DColor;
	ilt_client->End3D = cis_End3D;

	ilt_client->CreateColor = cis_CreateColor;
	ilt_client->DeleteColor = cis_DeleteColor;
	ilt_client->SetupColor1 = cis_SetupColor1;
	ilt_client->SetupColor2 = cis_SetupColor2;
	
	ilt_client->GetBorderSize = cis_GetBorderSize;

	ilt_client->OptimizeSurface = cis_OptimizeSurface;
	ilt_client->UnoptimizeSurface = cis_UnoptimizeSurface;
	
	ilt_client->GetScreenSurface = cis_GetScreenSurface;
	
	ilt_client->CreateHeightmapFromBitmap = cis_CreateHeightmapFromBitmap;
	ilt_client->FreeHeightmap = cis_FreeHeightmap;
	ilt_client->CreateSurfaceFromBitmap = cis_CreateSurfaceFromBitmap;
	ilt_client->CreateSurface = cis_CreateSurface;
	ilt_client->DeleteSurface = cis_DeleteSurface;

	ilt_client->GetSurfaceUserData = cis_GetSurfaceUserData;
	ilt_client->SetSurfaceUserData = cis_SetSurfaceUserData;

	ilt_client->GetPixel = cis_GetPixel;
	ilt_client->SetPixel = cis_SetPixel;
	
	ilt_client->GetSurfaceDims = cis_GetSurfaceDims;
	ilt_client->DrawBitmapToSurface = cis_DrawBitmapToSurface;
	ilt_client->DrawSurfaceSolidColor = cis_DrawSurfaceSolidColor;
	ilt_client->DrawSurfaceMasked = cis_DrawSurfaceMasked;
	ilt_client->DrawSurfaceToSurface = cis_DrawSurfaceToSurface;
	ilt_client->DrawSurfaceToSurfaceTransparent = cis_DrawSurfaceToSurfaceTransparent;
	
	ilt_client->ScaleSurfaceToSurface = cis_ScaleSurfaceToSurface;
	ilt_client->ScaleSurfaceToSurfaceTransparent = cis_ScaleSurfaceToSurfaceTransparent;
	ilt_client->ScaleSurfaceToSurfaceSolidColor = cis_ScaleSurfaceToSurfaceSolidColor;

	ilt_client->WarpSurfaceToSurface = cis_WarpSurfaceToSurface;
	ilt_client->WarpSurfaceToSurfaceTransparent = cis_WarpSurfaceToSurfaceTransparent;
	ilt_client->WarpSurfaceToSurfaceSolidColor = cis_WarpSurfaceToSurfaceSolidColor;

	ilt_client->TransformSurfaceToSurface = cis_TransformSurfaceToSurface;
	ilt_client->TransformSurfaceToSurfaceTransparent = cis_TransformSurfaceToSurfaceTransparent;
	
	ilt_client->FillRect = cis_FillRect;
	ilt_client->GetEngineHook = cis_GetEngineHook;
	ilt_client->QueryGraphicDevice = cis_QueryGraphicDevice;

	ilt_client->GetSurfaceAlpha = cis_GetSurfaceAlpha;
	ilt_client->SetSurfaceAlpha = cis_SetSurfaceAlpha;

	// Init the screen surface.
	g_ScreenSurface.m_Flags = SURFFLAG_SCREEN;

	// Init the allocators..
	sb_Init(&g_SurfaceBank, sizeof(CisSurface), 10);

	// Init the mask lookup.
	memset(g_MaskLookup, 0, sizeof(g_MaskLookup));
	for(i=0; i < 8; i++)
		g_MaskLookup[1 << i] = (1 << i) - 1;

	// Init the text manager..
	tmgr_Init();
}

void cis_Term()
{
	cis_DeleteSurfaces();
	
	// Shutdown the allocators.
	sb_Term(&g_SurfaceBank);

	tmgr_Term();
	
	g_pCisRenderStruct = LTNULL;
}


bool cis_RendererIsHere(RenderStruct *pStruct)
{
	g_pCisRenderStruct = pStruct;
	
	g_ScreenSurface.m_Width = pStruct->m_Width;
	g_ScreenSurface.m_Height = pStruct->m_Height;
	pStruct->GetScreenFormat(&g_ScreenFormat);
	g_nScreenPixelBytes = g_ScreenFormat.GetBytesPerPixel();
	
	return cis_RestoreSurfaces();
}


bool cis_RendererGoingAway()
{
	bool bRet;

	g_PrevScreenFormat = g_ScreenFormat;
	g_nPrevScreenPixelBytes = g_nScreenPixelBytes;

	bRet = cis_BackupSurfaces();
	g_pCisRenderStruct = LTNULL;

	return bRet;
}


// For memorywatch.cpp.
unsigned long GetInterfaceSurfaceMemory()
{
	GPOS pos;
	CisSurface *pSurface;
	uint32 total;

	total = 0;
	for(pos=g_Surfaces; pos; )
	{
		pSurface = g_Surfaces.GetNext(pos);
	
		total += pSurface->m_Width * pSurface->m_Height * 2;
		if(pSurface->m_Flags & SURFFLAG_OPTIMIZED)
		{
			total += pSurface->m_Width * pSurface->m_Height * 2;
		}
	}

	return total;
}











