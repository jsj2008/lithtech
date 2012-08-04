#include "precompile.h"

#include "pixelformat.h"
#include "d3d_surface.h"
#include "d3d_texture.h"
#include "renderstruct.h"
#include "common_stuff.h"
#include "d3d_draw.h"
#include "3d_ops.h"
#include "dirtyrect.h"
#include "d3d_texture.h"


#define DO_MASK_LOOP(srcType, destType, srcIt, destIt, srcTColor, destAlphaMask)	\
	srcIt = (srcType*)pSrcLine;														\
	destIt = (destType*)pDestLine;													\
	while(xCounter) {																\
		xCounter--;																	\
		if(*srcIt != srcTColor)														\
			*destIt |= destAlphaMask;												\
		srcIt++; destIt++; }

static uint32 g_ValidTileSizes[] = {2, 4, 8, 16, 32, 64};
#define NUM_VALIDTILESIZES (sizeof(g_ValidTileSizes) / sizeof(g_ValidTileSizes[0]))

#define MAX_OPTIMIZED_SURFACE_SIZE	64
#define IMAGE_SIZE_PER_TILE			(MAX_OPTIMIZED_SURFACE_SIZE - 1)

extern bool d3d_GetScreenFormat(PFormat *pFormat);
extern FormatMgr g_FormatMgr; // d3d_texture.cpp.

static uint32 g_OldFogEnable;

static LTSurfaceBlend g_Optimized2DBlend(LTSURFACEBLEND_ALPHA);
static HLTCOLOR g_Optimized2DColor(0xFFFFFFFF);
static DWORD g_minFilter[4] = { D3DTEXF_LINEAR,D3DTEXF_LINEAR,D3DTEXF_LINEAR,D3DTEXF_LINEAR};
static DWORD g_magFilter[4] = { D3DTEXF_LINEAR,D3DTEXF_LINEAR,D3DTEXF_LINEAR,D3DTEXF_LINEAR};

uint32 d3d_GetValidTileSize(uint32 size)
{
	for (uint32 i=0; i < NUM_VALIDTILESIZES; ++i) {
		if (size < g_ValidTileSizes[i]) {
			return g_ValidTileSizes[i]; } }
	return g_ValidTileSizes[NUM_VALIDTILESIZES-1];
}

void d3d_DestroyTiles(RSurface* pSurface)
{
	if (pSurface->m_pTiles) {
		uint32 count = pSurface->m_pTiles->m_nTilesX * pSurface->m_pTiles->m_nTilesY;
		for (uint32 i=0; i < count; ++i) {
			SurfaceTile* pTile = &pSurface->m_pTiles->m_Tiles[i];
			if (pTile->m_pTexture) pTile->m_pTexture->Release(); }

		dfree(pSurface->m_pTiles);
		pSurface->m_pTiles = NULL; }
}

// Set solid alpha on the dest data in the FMConvertRequest.
void d3d_SetSolidAlpha(FMConvertRequest* pRequest, uint32 destAlpha32, GenericColor &tColor)
{
	uint8*  pDestLine   = pRequest->m_pDest;
	uint32  destAlpha16 = (uint16)destAlpha32;

	// Supports 16 and 32-bit..
	uint32 yCounter = pRequest->m_Height;
	while (yCounter) {
		--yCounter;

		uint32 xCounter = pRequest->m_Width;
		if (pRequest->m_pDestFormat->GetType() == BPP_16) {
			uint16* pDest16 = (uint16*)pDestLine;
			while (xCounter) {
				xCounter--;
				*pDest16 |= destAlpha16;
				++pDest16; } }
		else {
			uint32* pDest32 = (uint32*)pDestLine;
			while (xCounter) {
				--xCounter;
				*pDest32 |= destAlpha32;
				++pDest32; } }

		pDestLine += pRequest->m_DestPitch; }
}

// Iterates thru the ConvertRequest data and sets the alpha color you specify
// on any pixels that have the color of tColor.
void d3d_DoAlphaFromColorKey(FMConvertRequest* pRequest, uint32 destAlpha32, GenericColor &tColor)
{
	uint32 destAlpha16;
	uint32 yCounter, xCounter;
	uint32 *pSrc32, *pDest32;
	uint16 *pSrc16, *pDest16;
	uint8 *pSrcLine, *pDestLine;
	
	// for PCX bug FIX
	uint32* srcIt;
	uint16* destIt;

	pSrcLine = pRequest->m_pSrc;
	pDestLine = pRequest->m_pDest;
	destAlpha16 = (uint16)destAlpha32;

	// Supports 16 and 32-bit..
	yCounter = pRequest->m_Height;
	while (yCounter) {
		--yCounter;

		xCounter = pRequest->m_Width;
		if (pRequest->m_pSrcFormat->GetType() == BPP_16) {
			if (pRequest->m_pDestFormat->GetType() == BPP_16) {
				DO_MASK_LOOP(uint16, uint16, pSrc16, pDest16, tColor.wVal, destAlpha16) }
			else if (pRequest->m_pDestFormat->GetType() == BPP_32) {
				DO_MASK_LOOP(uint16, uint32, pSrc16, pDest32, tColor.wVal, destAlpha32) } }
		else {
			if (pRequest->m_pDestFormat->GetType() == BPP_16) {
				// This is a fix for the PCX trancparency bug
				//DO_MASK_LOOP(uint32, uint16, pSrc32, pDest16, tColor.dwVal, destAlpha16)
				srcIt  = (uint32*)pSrcLine;
				destIt = (uint16*)pDestLine;
				while (xCounter) {
					--xCounter;
					if (*srcIt !=  tColor.dwVal) *destIt |= destAlpha16;
					else *destIt &= 0x7FFF; // set alpha bit to 0
					++srcIt; ++destIt; } }
			else if (pRequest->m_pDestFormat->GetType() == BPP_32) {
				DO_MASK_LOOP(uint32, uint32, pSrc32, pDest32, tColor.dwVal, destAlpha32) } }

		pSrcLine  += pRequest->m_SrcPitch;
		pDestLine += pRequest->m_DestPitch; }
}

bool d3d_FillSurfaceTiles(RSurface *pSurface, TextureFormat *pDestFormat, PValue transparentColor)
{
	//setup the formats we will be using, we convert from the image format to the screen
	//format
	PFormat srcFormat, destFormat;
	if(!d3d_GetScreenFormat(&srcFormat))
		return false;

	pDestFormat->SetupPFormat(&destFormat);

	GenericColor tColor;
	g_FormatMgr.PValueToFormatColor(&srcFormat, transparentColor, tColor);

	FMConvertRequest cRequest;
	cRequest.m_pSrcFormat  = &srcFormat;
	cRequest.m_pDestFormat = &destFormat;
	pSurface->m_bTilesTransparent = !(transparentColor == 0xFFFFFFFF);

	// Convert the data over..
	D3DLOCKED_RECT SrcLockRect;
	HRESULT hResult = pSurface->m_pSurface->LockRect(&SrcLockRect, NULL, D3DLOCK_READONLY);
	if (hResult == D3D_OK) 
	{
		uint8* pSrcData = (uint8*)SrcLockRect.pBits;

		for (uint32 x=0; x < pSurface->m_pTiles->m_nTilesX; x++) 
		{
			for (uint32 y=0; y < pSurface->m_pTiles->m_nTilesY; y++) 
			{
				SurfaceTile* pTile = &pSurface->m_pTiles->m_Tiles[y*pSurface->m_pTiles->m_nTilesX+x];
				
				D3DLOCKED_RECT DestLockRect;
				hResult = pTile->m_pTexture->LockRect(0,&DestLockRect, NULL, NULL);
				if (hResult == D3D_OK) 
				{
					// Setup the ConvertRequest.
					cRequest.m_pSrc		 = &pSrcData[pTile->m_SrcImageRect.top * SrcLockRect.Pitch];
					cRequest.m_pSrc	    += pTile->m_SrcImageRect.left * srcFormat.GetBytesPerPixel();
					cRequest.m_SrcPitch  = SrcLockRect.Pitch;
					cRequest.m_pDest	 = (uint8*)DestLockRect.pBits;
					cRequest.m_DestPitch = DestLockRect.Pitch;

					cRequest.m_Width	 = pTile->m_nXPixels;
					cRequest.m_Height	 = pTile->m_nYPixels;

					LTRESULT dResult = g_FormatMgr.ConvertPixels(&cRequest);
					if (dResult == LT_OK) 
					{
						// Set the alpha if we need to.  This stuff is really special case because 
						// it's a pretty unusual thing to be doing.
						if (transparentColor == 0xFFFFFFFF) 
						{
							d3d_SetSolidAlpha(&cRequest,destFormat.m_Masks[CP_ALPHA],tColor); 
						}
						else 
						{
							d3d_DoAlphaFromColorKey(&cRequest,destFormat.m_Masks[CP_ALPHA],tColor); 
						} 
					}

					pTile->m_pTexture->UnlockRect(0); 
				}
				else 
				{
					pSurface->m_pSurface->UnlockRect();
					return false; 
				} 
			} 
		}

		pSurface->m_pSurface->UnlockRect();
		return true; 
	}
	else 
	{
		return false; 
	}
}

void d3d_UnoptimizeSurface(HLTBUFFER hBuffer)
{
	if(!hBuffer)
		return;

	d3d_DestroyTiles((RSurface*)hBuffer);
}

static uint32 d3d_CalcNumTilesInDir(uint32 nVal)
{
	//handle 0 degenerate case
	if(nVal == 0)
		return 0;

	//we need to figure out how many tiles we need based upon the given value
	//Note: The first tile can fit the maximum number of pixels, but the others have
	//to reserve one for bilinear filtering
	if(nVal <= MAX_OPTIMIZED_SURFACE_SIZE)
		return 1;

	//alright, it needs multiple tiles, the first one is now implicit
	uint32 nTileInc = 1;
	nVal -= MAX_OPTIMIZED_SURFACE_SIZE;

	//see if we have an exact fit
	if(nVal % (MAX_OPTIMIZED_SURFACE_SIZE - 1) != 0)
		nTileInc++;

	return (nVal / (MAX_OPTIMIZED_SURFACE_SIZE - 1)) + nTileInc;
}

bool d3d_OptimizeSurface(HLTBUFFER hBuffer, PValue transparentColor )
{
	if (!hBuffer || !g_CV_OptimizeSurfaces) 
		return false;

	RSurface* pSurface = (RSurface*)hBuffer;

	// Do we have a texture format we can use?

	TextureFormat* pFormat = g_TextureManager.GetTextureFormat(CTextureManager::FORMAT_INTERFACE);
	if (!pFormat) 
		return false;

	// Create the tiles if there aren't any yet.
	if (!pSurface->m_pTiles) 
	{
		//when allocating the textures, we need to keep in mind bilinear filtering which requires us
		//to create a redundant pixel along the top and left edge so that the filtering is smooth
		//across tile boundaries
		
		// How many tiles do we need?
		uint32 nTilesX = d3d_CalcNumTilesInDir(pSurface->m_Desc.Width);
		uint32 nTilesY = d3d_CalcNumTilesInDir(pSurface->m_Desc.Height);
		
		if (nTilesX == 0 && nTilesY == 0) 
			return false;

		//allocate the memory and setup the tile holder
		LT_MEM_TRACK_ALLOC(pSurface->m_pTiles = (SurfaceTiles*)dalloc_z(sizeof(SurfaceTiles) + sizeof(SurfaceTile)*((nTilesX*nTilesY)-1)),LT_MEM_TYPE_RENDERER);
		if (!pSurface->m_pTiles) 
			return false;

		pSurface->m_pTiles->m_nTilesX = nTilesX;
		pSurface->m_pTiles->m_nTilesY = nTilesY;

		//now go through each tile and setup the dimensions
		uint32 nCurrX = 0;
		for (uint32 x=0; x < nTilesX; ++x) 
		{
			//figure out the X size of the tile
			uint32 tileXSize = pSurface->m_Desc.Width - nCurrX;
			tileXSize = d3d_GetValidTileSize(tileXSize);

			//figure out the coverage of this tile
			uint32 nTileXCoverage = MAX_OPTIMIZED_SURFACE_SIZE;
			if(x > 0)
				nTileXCoverage--;

			uint32 nCurrY = 0;
			for (uint32 y=0; y < nTilesY; ++y) 
			{
				//figure out the Y size
				uint32 tileYSize = pSurface->m_Desc.Height - nCurrY;
				tileYSize = d3d_GetValidTileSize(tileYSize);
				
				SurfaceTile* pTile = &pSurface->m_pTiles->m_Tiles[y * nTilesX + x];

				//figure out the coverage of this tile
				uint32 nTileYCoverage = MAX_OPTIMIZED_SURFACE_SIZE;
				if(y > 0)
					nTileYCoverage--;

				//figure out and store the actual pixel extents
				pTile->m_nXPixels = LTMIN(MAX_OPTIMIZED_SURFACE_SIZE, pSurface->m_Desc.Width - nCurrX);
				pTile->m_nYPixels = LTMIN(MAX_OPTIMIZED_SURFACE_SIZE, pSurface->m_Desc.Height - nCurrY);
				pTile->m_SrcImageRect.left = nCurrX;
				pTile->m_SrcImageRect.top = nCurrY;
				pTile->m_SrcImageRect.right = nCurrX + pTile->m_nXPixels;
				pTile->m_SrcImageRect.bottom = nCurrY + pTile->m_nYPixels;

				if(x > 0 && x < nTilesX - 1)
					pTile->m_SrcImageRect.right--;
				if(y > 0 && y < nTilesY - 1)
					pTile->m_SrcImageRect.bottom--;

				//handle square textures
				if (g_Device.GetDeviceCaps()->TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) 
				{
					pTile->m_nTileWidth  = LTMAX(tileXSize, tileYSize);
					pTile->m_nTileHeight = LTMAX(tileXSize, tileYSize); 
				}
				else 
				{
					pTile->m_nTileWidth  = tileXSize;
					pTile->m_nTileHeight = tileYSize; 
				}
				
				// Create the texture surface.
				HRESULT hResult = PD3DDEVICE->CreateTexture(pTile->m_nTileWidth, pTile->m_nTileHeight, 1, NULL, pFormat->m_PF, D3DPOOL_MANAGED, &pTile->m_pTexture);
				if (hResult != D3D_OK) 
				{
					d3d_DestroyTiles(pSurface);
					return false; 
				}

				nCurrY += nTileYCoverage;
			} 


			nCurrX += nTileXCoverage;
		} 
	}

	assert(pSurface->m_pTiles);
		
	// Convert the data.	
	if (!d3d_FillSurfaceTiles(pSurface, pFormat, transparentColor)) 
	{
		d3d_DestroyTiles(pSurface);
		return false; 
	}

	return true;
}


inline LTBOOL GetRectIntersection(LTRect *pDest, LTRect *pRect1, LTRect *pRect2)
{
	pDest->left = LTMAX(pRect1->left, pRect2->left);
	pDest->top = LTMAX(pRect1->top, pRect2->top);
	pDest->right = LTMIN(pRect1->right, pRect2->right);
	pDest->bottom = LTMIN(pRect1->bottom, pRect2->bottom);

	// Test for rejection..
	if (pDest->left >= pDest->right) return LTFALSE;
	if (pDest->top >= pDest->bottom) return LTFALSE;
	return LTTRUE;
}


bool d3d_StartOptimized2D()
{
	if (!PD3DDEVICE || !g_Device.IsIn3D()) return false;
	if (g_bInOptimized2D) return true;

	// Set states...
	PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, (unsigned long *)&g_OldFogEnable);
	PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, FALSE);

	PD3DDEVICE->SetRenderState(D3DRS_ZENABLE, FALSE);
	PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

	// Possibly reset bilinear for 2d ? 
	for(uint32 nCurrStage = 0; nCurrStage < 4; nCurrStage++)
	{
		// save the sampler states
		PD3DDEVICE->GetSamplerState(nCurrStage, D3DSAMP_MINFILTER, &g_minFilter[nCurrStage]);
		PD3DDEVICE->GetSamplerState(nCurrStage, D3DSAMP_MAGFILTER, &g_magFilter[nCurrStage]);

		// check for device caps anisotropic filter for min filter
		if ((g_Device.GetDeviceCaps()->VertexTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) && (g_CV_2DAnisotropic.m_Val))
		{
			D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC));
		}
		else
		{
			D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MINFILTER, g_CV_2DBilinear.m_Val ? D3DTEXF_LINEAR : D3DTEXF_POINT));
		}

		// check for device caps anistropic filter for mag filter
		if ((g_Device.GetDeviceCaps()->VertexTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC) && (g_CV_2DAnisotropic.m_Val))
		{
			D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC));
		}
		else
		{
			D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MAGFILTER, g_CV_2DBilinear.m_Val ? D3DTEXF_LINEAR : D3DTEXF_POINT));
		}

	}


	g_bInOptimized2D = true;
	return true;
}

void d3d_EndOptimized2D()
{
	if (!PD3DDEVICE || !g_Device.IsIn3D()) return;
	if (!g_bInOptimized2D) return;

	PD3DDEVICE->SetRenderState(D3DRS_ZENABLE, TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE, g_OldFogEnable);

	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));

	// Set sampler states back to saved states
	for(uint32 nCurrStage = 0; nCurrStage < 4; nCurrStage++)
	{
		// Restore the saved states 
		D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MINFILTER, g_minFilter[nCurrStage]));
		D3D_CALL(PD3DDEVICE->SetSamplerState(nCurrStage, D3DSAMP_MAGFILTER, g_magFilter[nCurrStage]));
	}


	g_bInOptimized2D = false;
}

bool d3d_IsInOptimized2D()
{
	return g_bInOptimized2D;
}

// Set up the src/dest blend modes for a given blend
bool d3d_SetOptimized2DBlend(LTSurfaceBlend blend)
{
	// Save the blend mode;
	g_Optimized2DBlend = blend;

	switch(blend)
	{
		case LTSURFACEBLEND_ALPHA : // Alpha
		{
			PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		}
		case LTSURFACEBLEND_SOLID : // Solid
		{
			PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			break;
		}
		case LTSURFACEBLEND_ADD : // Add
		{
			PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		}
		case LTSURFACEBLEND_MULTIPLY : // Multiply
		{
			PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			break;
		}
		case LTSURFACEBLEND_MULTIPLY2 : // Saturate
		{
			PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
			break;
		}
		case LTSURFACEBLEND_MASK : // Mask
		{
			PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
			break;
		}
		case LTSURFACEBLEND_MASKADD : // Mask + Add
		{
			PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
			break;
		}
		default :
		{
			// This shouldn't ever happen unless we add a new blend mode
			return false;
		}
	}

	return true;
}

bool d3d_SetOptimized2DColor(HLTCOLOR color)
{
	g_Optimized2DColor = 0xFF000000 | (color & 0xFFFFFF);

	return true;
}

bool d3d_GetOptimized2DBlend(LTSurfaceBlend &blend)
{
	blend = g_Optimized2DBlend;
	return true;
}

bool d3d_GetOptimized2DColor(HLTCOLOR &color)
{
	color = g_Optimized2DColor;
	return true;
}


const float fSub = 0.51f;
void d3d_BlitToScreen3D(BlitRequest *pRequest)
{
	RSurface* pRSurface	= (RSurface*)pRequest->m_hBuffer;
	SurfaceTiles* pTiles = pRSurface->m_pTiles;

	assert(pTiles);
	if (!pTiles) return;

	StateSet ssAlphaBlendEnable(D3DRS_ALPHABLENDENABLE, pRSurface->m_bTilesTransparent || (pRequest->m_Alpha != 1.0f));

	// Make sure the states we want are set!
	VERIFY_RENDERSTATE(D3DRS_FOGENABLE, FALSE);
	VERIFY_RENDERSTATE(D3DRS_ZENABLE, FALSE);
	VERIFY_RENDERSTATE(D3DRS_ZWRITEENABLE, FALSE);
	VERIFY_RENDERSTATE(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	VERIFY_RENDERSTATE(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	LTRect* pSrcRect		= pRequest->m_pSrcRect;
	float	srcRectWidth	= (float)(pSrcRect->right - pSrcRect->left);
	float	srcRectHeight	= (float)(pSrcRect->bottom - pSrcRect->top);
	
	LTRect* pDestRect		= pRequest->m_pDestRect;
	float	destRectWidth	= (float)(pDestRect->right - pDestRect->left);
	float	destRectHeight	= (float)(pDestRect->bottom - pDestRect->top);

	// Init default stuff in the verts.
	TLVertex verts[4];		
	verts[0].color = verts[1].color = verts[2].color = verts[3].color = g_Optimized2DColor;
	verts[0].rgb.a = verts[1].rgb.a = verts[2].rgb.a = verts[3].rgb.a = (uint8)(pRequest->m_Alpha * 255.0f);
	verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = 1.0f;

	// Remember the previous texture in there.
	LPDIRECT3DBASETEXTURE9 pOldTexture = NULL;
	D3D_CALL(PD3DDEVICE->GetTexture(0, &pOldTexture));
	
	SurfaceTile* pTile;
	float destLeft, destRight, destTop, destBottom;
	float tDestLeft, tDestTop, tDestRight, tDestBottom;
	HRESULT hResult;
	LTRect rcIntersection;
	int nXOffset = pSrcRect->left;
	int nYOffset = pSrcRect->top;

	// Draw each tile.
	for(uint32 nXTile = 0; nXTile < pTiles->m_nTilesX; nXTile++) 
	{
		for(uint32 nYTile = 0; nYTile < pTiles->m_nTilesY; nYTile++) 
		{
			pTile = &pTiles->m_Tiles[nYTile * pTiles->m_nTilesX + nXTile];
			if (!GetRectIntersection(&rcIntersection, pSrcRect, &pTile->m_SrcImageRect))
				continue;

			destLeft	= (float)pDestRect->left + (float)(rcIntersection.left - nXOffset);
			destRight	= (float)pDestRect->left + (float)(rcIntersection.right - nXOffset);
			destTop		= (float)pDestRect->top + (float)(rcIntersection.top - nYOffset);
			destBottom  = (float)pDestRect->top + (float)(rcIntersection.bottom - nYOffset);

			tDestLeft	= (float)(rcIntersection.left - pTile->m_SrcImageRect.left) / (float)pTile->m_nTileWidth;
			tDestRight	= (float)(rcIntersection.right - pTile->m_SrcImageRect.left) / (float)pTile->m_nTileWidth;
			tDestTop	= (float)(rcIntersection.top - pTile->m_SrcImageRect.top) / (float)pTile->m_nTileHeight;
			tDestBottom = (float)(rcIntersection.bottom - pTile->m_SrcImageRect.top) / (float)pTile->m_nTileHeight;

			// This adjusts for the DX fill convention so the optimized surfaces cover
			// the same pixels that nonoptimized surfaces would.
			destLeft = LTMAX(0.0f, destLeft - fSub);
			destTop = LTMAX(0.0f, destTop - fSub);
			destRight = LTMAX(0.0f, destRight - fSub);
			destBottom = LTMAX(0.0f, destBottom - fSub);

			// Draw the poly!
			verts[0].m_Vec.Init(destLeft, destTop, 1.0f);
			verts[1].m_Vec.Init(destRight, destTop, 1.0f);
			verts[2].m_Vec.Init(destRight, destBottom, 1.0f);
			verts[3].m_Vec.Init(destLeft, destBottom, 1.0f);

			verts[0].SetTCoords(tDestLeft, tDestTop);
			verts[1].SetTCoords(tDestRight, tDestTop);
			verts[2].SetTCoords(tDestRight, tDestBottom);
			verts[3].SetTCoords(tDestLeft, tDestBottom);

			d3d_SetTextureDirect(pTile->m_pTexture, 0);

			D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
			D3D_CALL(PD3DDEVICE->SetFVF(TLVERTEX_FORMAT));
			hResult = D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(TLVertex)));
		}
	}

	d3d_SetTextureDirect(pOldTexture, 0);
	InvalidateRect(pDestRect);
}

void d3d_BlitToScreen3D_Old(BlitRequest *pRequest)
{
	RSurface*		pRSurface	= (RSurface*)pRequest->m_hBuffer;
	SurfaceTiles*	pTiles		= pRSurface->m_pTiles;

	assert(pTiles);
	if (!pTiles) return;

	StateSet ssAlphaBlendEnable(D3DRS_ALPHABLENDENABLE, pRSurface->m_bTilesTransparent || (pRequest->m_Alpha != 1.0f));

	// Make sure the states we want are set!
	VERIFY_RENDERSTATE(D3DRS_FOGENABLE, FALSE);
	VERIFY_RENDERSTATE(D3DRS_ZENABLE, FALSE);
	VERIFY_RENDERSTATE(D3DRS_ZWRITEENABLE, FALSE);
	VERIFY_RENDERSTATE(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	VERIFY_RENDERSTATE(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	LTRect* pSrcRect		= pRequest->m_pSrcRect;
	float	srcRectWidth	= (float)(pSrcRect->right - pSrcRect->left);
	float	srcRectHeight	= (float)(pSrcRect->bottom - pSrcRect->top);

	LTRect* pDestRect		= pRequest->m_pDestRect;
	float	destRectWidth	= (float)(pDestRect->right - pDestRect->left);
	float	destRectHeight	= (float)(pDestRect->bottom - pDestRect->top);

	// Init default stuff in the verts.
	TLVertex verts[4];		
	verts[0].color = verts[1].color = verts[2].color = verts[3].color = g_Optimized2DColor;
	verts[0].rgb.a = verts[1].rgb.a = verts[2].rgb.a = verts[3].rgb.a = (uint8)(pRequest->m_Alpha * 255.0f);
	verts[0].rhw = verts[1].rhw = verts[2].rhw = verts[3].rhw = 1.0f;

	// Remember the previous texture in there.
	LPDIRECT3DBASETEXTURE9 pOldTexture = NULL;
	D3D_CALL(PD3DDEVICE->GetTexture(0, &pOldTexture));

	// Draw each tile.
	for(uint32 nXTile = 0; nXTile < pTiles->m_nTilesX; nXTile++) 
	{
		for(uint32 nYTile = 0; nYTile < pTiles->m_nTilesY; nYTile++) 
		{
			SurfaceTile* pTile = &pTiles->m_Tiles[nYTile * pTiles->m_nTilesX + nXTile];

			float destLeft	  = (float)pDestRect->left + ((pTile->m_SrcImageRect.left - pSrcRect->left) / srcRectWidth) * destRectWidth;
			float destRight	  = (float)pDestRect->left + ((pTile->m_SrcImageRect.right - pSrcRect->left) / srcRectWidth) * destRectWidth;
			float destTop	  = (float)pDestRect->top + ((pTile->m_SrcImageRect.top - pSrcRect->top) / srcRectHeight) * destRectHeight;
			float destBottom  = (float)pDestRect->top + ((pTile->m_SrcImageRect.bottom - pSrcRect->top) / srcRectHeight) * destRectHeight;

			float tDestLeft	= 0.5f / pTile->m_nTileWidth;
			float tDestRight  = ((float)pTile->m_nXPixels - 0.5f) / pTile->m_nTileWidth;
			float tDestTop	= 0.5f / pTile->m_nTileHeight;
			float tDestBottom = ((float)pTile->m_nYPixels - 0.5f) / pTile->m_nTileHeight;

			// This adjusts for the DX fill convention so the optimized surfaces cover
			// the same pixels that nonoptimized surfaces would.
			destLeft = LTMAX(0.0f, destLeft - fSub);
			destTop = LTMAX(0.0f, destTop - fSub);
			destRight = LTMAX(0.0f, destRight - fSub);
			destBottom = LTMAX(0.0f, destBottom - fSub);

			// Draw the poly!
			verts[0].m_Vec.Init(destLeft, destTop, 1.0f);
			verts[1].m_Vec.Init(destRight, destTop, 1.0f);
			verts[2].m_Vec.Init(destRight, destBottom, 1.0f);
			verts[3].m_Vec.Init(destLeft, destBottom, 1.0f);

			verts[0].SetTCoords(tDestLeft, tDestTop);
			verts[1].SetTCoords(tDestRight, tDestTop);
			verts[2].SetTCoords(tDestRight, tDestBottom);
			verts[3].SetTCoords(tDestLeft, tDestBottom);

			d3d_SetTextureDirect(pTile->m_pTexture, 0);

			D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
			D3D_CALL(PD3DDEVICE->SetFVF(TLVERTEX_FORMAT));
			HRESULT hResult = D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(TLVertex)));
		}
	}
	d3d_SetTextureDirect(pOldTexture, 0);
	InvalidateRect(pDestRect);
}


void d3d_WarpToScreen3D(BlitRequest *pRequest)
{
	uint8 byAlpha		= (uint8)(pRequest->m_Alpha * 255.0f);
	LTWarpPt *pWarpPts	= pRequest->m_pWarpPts;

	RSurface* pRSurface  = (RSurface*)pRequest->m_hBuffer;
	SurfaceTiles *pTiles = pRSurface->m_pTiles;
	SurfaceTile *pTile;

	// If there aren't any tiles, get us out of here cause it's not an optimized surface
	assert(pTiles);
	if(!pTiles) return;

	// Get the number of tiles in this surface
	uint32 nXTiles = pRSurface->m_pTiles->m_nTilesX;
	uint32 nYTiles = pRSurface->m_pTiles->m_nTilesY;

	// Setup the widths of our surface that we're going to blit
	float srcRectWidth  = (float)(pRequest->m_pSrcRect->right - pRequest->m_pSrcRect->left);
	float srcRectHeight = (float)(pRequest->m_pSrcRect->bottom - pRequest->m_pSrcRect->top);

	// Set the rendering device to allow for translucency
	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, pRSurface->m_bTilesTransparent || (pRequest->m_Alpha != 1.0f)));

	VERIFY_RENDERSTATE(D3DRS_FOGENABLE, FALSE);		// Make sure the states we want are set!
	VERIFY_RENDERSTATE(D3DRS_ZENABLE, FALSE);
	VERIFY_RENDERSTATE(D3DRS_ZWRITEENABLE, FALSE);
	VERIFY_RENDERSTATE(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	VERIFY_RENDERSTATE(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	LPDIRECT3DBASETEXTURE9 pOldTexture = NULL;		// Remember the previous texture so we can reset it at the end
	D3D_CALL(PD3DDEVICE->GetTexture(0, &pOldTexture));

	TLVertex verts[12];								// Set all of the verticies to the same default values for color, alpha, and rhw
	verts[0].color	= verts[1].color	= verts[2].color	= verts[3].color	= g_Optimized2DColor;
	verts[0].rgb.a	= verts[1].rgb.a	= verts[2].rgb.a	= verts[3].rgb.a	= (uint8)(pRequest->m_Alpha * 255.0f);
	verts[0].rhw	= verts[1].rhw		= verts[2].rhw		= verts[3].rhw		= 1.0f;


	// Setup the directions of the x and y drawing planes
	LTVector vWarpStart(pWarpPts[0].dest_x, pWarpPts[0].dest_y, 1.0f);
	LTVector vWarpDirX = LTVector(pWarpPts[1].dest_x, pWarpPts[1].dest_y, 1.0f) - vWarpStart;
	LTVector vWarpDirY = LTVector(pWarpPts[3].dest_x, pWarpPts[3].dest_y, 1.0f) - vWarpStart;

	// Get the scale values
	float fScaleX = VEC_MAG(vWarpDirX) / srcRectWidth;
	float fScaleY = VEC_MAG(vWarpDirY) / srcRectHeight;

	// Set the vectors to the appropriate scale
	vWarpDirX.Norm(fScaleX);
	vWarpDirY.Norm(fScaleY);

	uint32 xPos, yPos = 0;

	// Calculate the values for our verticies based off of the passed in warp points
	for (uint32 i = 0; i < nYTiles; i++) {
		// Reset the X position
		xPos = 0;

		for (uint32 j = 0; j < nXTiles; j++) {
			// Get a pointer to the current tile
			pTile = &pTiles->m_Tiles[i * nXTiles + j];

			// Setup the destination points
			verts[0].m_Vec = vWarpStart + (vWarpDirX * (float)xPos) + (vWarpDirY * (float)yPos);
			verts[1].m_Vec = verts[0].m_Vec + (vWarpDirX * (float)pTile->m_nTileWidth);
			verts[2].m_Vec = verts[0].m_Vec + (vWarpDirX * (float)pTile->m_nTileWidth) + (vWarpDirY * (float)pTile->m_nTileHeight);
			verts[3].m_Vec = verts[0].m_Vec + (vWarpDirY * (float)pTile->m_nTileHeight);

			// Setup the source U,V coordinates
			verts[0].SetTCoords(0.0f, 0.0f);
			verts[1].SetTCoords(1.0f, 0.0f);
			verts[2].SetTCoords(1.0f, 1.0f);
			verts[3].SetTCoords(0.0f, 1.0f);

			// Skip this tile if it's completely outside the clipping rect

			// Blit the entire tile if it's completely inside

			// Clip the points if we're overlapping the dest rect

			// Set the texture to the correct tile, and draw it using the verticies
			d3d_SetTextureDirect(pTile->m_pTexture, 0);

			D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
			D3D_CALL(PD3DDEVICE->SetFVF(TLVERTEX_FORMAT));
			HRESULT hResult = D3D_CALL(PD3DDEVICE->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(TLVertex)));

			// Increment the X position
			xPos += pTile->m_nTileWidth; 
		}

		// Increment the Y position
		yPos += pTiles->m_Tiles[i * nXTiles].m_nTileHeight; 
	}

	// Reset our texture to the one that was loaded before we entered this function
	d3d_SetTextureDirect(pOldTexture, 0);
}



