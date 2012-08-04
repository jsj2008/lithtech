//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#include "bdefs.h"
#include "draw_d3d.h"
#include "texture.h"
#include "cyclemgr.h"


#define NUM_VALID_TEXTURE_SIZES (sizeof(g_TextureSizes) / sizeof(g_TextureSizes[0]))


// ------------------------------------------------------------------------ //
// Structures.
// ------------------------------------------------------------------------ //

typedef struct Direct3dTexture_t
{
	
						Direct3dTexture_t()
						{
							m_Link.Init();
							m_pSurface = NULL;
							m_pTexture = NULL;
							m_pSystemTexture = NULL;
							m_pDrawMgr = NULL;
							m_uMul = m_vMul = 1.0f;
						}
	
	DLink				m_Link;
	
	LPDIRECTDRAWSURFACE4	m_pSurface;
	IDirect3DTexture2		*m_pTexture;

	CTexture			*m_pSystemTexture;
	D3DRender			*m_pDrawMgr;
	float				m_uMul, m_vMul;
} Direct3dTexture;



// ------------------------------------------------------------------------ //
// Globals.
// ------------------------------------------------------------------------ //

DWORD g_TextureSizes[] = {4, 8, 16, 32, 64, 128, 256, 512};

extern float g_TextureMipScale;
extern float g_uMul, g_vMul;

//detail texture settings
extern float g_fDetailTextureScale;
extern float g_fDetailTextureAngleCos;
extern float g_fDetailTextureAngleSin;



// ------------------------------------------------------------------------ //
// Internals.
// ------------------------------------------------------------------------ //

static void d3d_UnloadTexture(Direct3dTexture *pTexture, int iBindingNumber)
{
	dl_Remove(&pTexture->m_Link);
	
	pTexture->m_pTexture->Release();
	pTexture->m_pSurface->Release();
	
	if(pTexture->m_pSystemTexture)
	{
		pTexture->m_pSystemTexture->m_Bindings[iBindingNumber] = NULL;
	}

	delete pTexture;
}


// Sets D3D up to use this texture slot.
static inline void d3d_SetupTextureSlot(D3DRender *pRender, Direct3dTexture *pTexture)
{
	HRESULT hResult;

	// Setup coordinate multipliers.
	g_uMul = pTexture->m_uMul * g_TextureMipScale;
	g_vMul = pTexture->m_vMul * g_TextureMipScale;

	// Move it to the end of the LRU.
	dl_Remove(&pTexture->m_Link);
	dl_Insert(pRender->m_Textures.m_pPrev, &pTexture->m_Link);

	hResult = pRender->m_pDevice->SetTexture(0, pTexture->m_pTexture);
}


LPDIRECTDRAWSURFACE4 d3d_CreateTextureSurface2(
	D3DRender *pRender, 
	DWORD width, 
	DWORD height, 
	DWORD extraCaps,
	TextureFormat *pFormat)
{
	DDSURFACEDESC2 desc;
	LPDIRECTDRAWSURFACE4 pSurface;
	HRESULT hResult;

	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	desc.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	desc.dwWidth = width;
	desc.dwHeight = height;
	memcpy(&desc.ddpfPixelFormat, &pFormat->m_PF, sizeof(desc.ddpfPixelFormat));

	hResult = pRender->m_pDirectDraw->CreateSurface(&desc, &pSurface, NULL);
	if(hResult == DD_OK)
		return pSurface;
	else
		return NULL;
}


LPDIRECTDRAWSURFACE4 d3d_CreateTextureSurface(D3DRender *pRender, DWORD width, DWORD height, DWORD extraCaps)
{
	return d3d_CreateTextureSurface2(pRender, width, height, extraCaps, pRender->m_pTextureFormat);
}


// Frees up a texture.  Returns FALSE if there aren't any textures to free up.
static BOOL d3d_FreeSomeSpace(D3DRender *pRender)
{
	if(pRender->m_Textures.m_pNext == &pRender->m_Textures)
		return FALSE;

	d3d_UnloadTexture((Direct3dTexture*)pRender->m_Textures.m_pNext->m_pData, pRender->m_BindingNumber);
	return TRUE;
}


static inline BOOL d3d_PutTextureIntoSurface(D3DRender *pRender, 
	CTexture *pSystemTexture, LPDIRECTDRAWSURFACE4 pSurface, IDirect3DTexture2 *pTexture)
{
	WORD *pCurOutPos, *pCurOutLine, *pEndOutLine, *pEndOutPos;
	WORD *pCurInPos, *pCurInLine;
	DDSURFACEDESC2 desc;
	DWORD outPixelPitch;
	WORD rPart, gPart, bPart;
	WORD rRightShift, rLeftShift;
	WORD gRightShift, gLeftShift;
	WORD bRightShift, bLeftShift;
	CDib *pDib;
	HRESULT hResult;
	RECT rect;
	D3DTEXTUREHANDLE retHandle;


	pDib = pSystemTexture->m_pDib;

	// Convert from 5-5-5 to the destination format.
	desc.dwSize = sizeof(desc);

	hResult = pSurface->Lock(NULL, &desc, DDLOCK_WAIT, NULL);
	if(hResult == DD_OK)
	{
		// Get the shift info on the stack.
		rRightShift = pRender->m_pTextureFormat->m_RRightShift;
		rLeftShift = pRender->m_pTextureFormat->m_RLeftShift;
		
		gRightShift = pRender->m_pTextureFormat->m_GRightShift;
		gLeftShift = pRender->m_pTextureFormat->m_GLeftShift;
		
		bRightShift = pRender->m_pTextureFormat->m_BRightShift;
		bLeftShift = pRender->m_pTextureFormat->m_BLeftShift;

		pCurOutLine = (WORD*)desc.lpSurface;
		outPixelPitch = desc.lPitch >> 1;
		pEndOutLine = pCurOutLine + (pDib->GetHeight() * outPixelPitch);

		{
			pCurInLine = pDib->GetBuf16();
			while(pCurOutLine < pEndOutLine)
			{
				pCurOutPos = pCurOutLine;
				pCurInPos = pCurInLine;
				pEndOutPos = pCurOutPos + desc.dwWidth;

				while(pCurOutPos < pEndOutPos)
				{
					rPart = (*pCurInPos >> 7) & 0xF8;
					gPart = (*pCurInPos >> 2) & 0xF8;
					bPart = (*pCurInPos << 3) & 0xF8;
					
					*pCurOutPos = ((rPart >> rRightShift) << rLeftShift) |
						((gPart >> gRightShift) << gLeftShift) |
						((bPart >> bRightShift) << bLeftShift);

					++pCurOutPos;
					++pCurInPos;
				}

				pCurOutLine += outPixelPitch;
				pCurInLine += pDib->GetPitch();
			}
		}
		
		pSurface->Unlock(NULL);
	}
	else
	{	
		return FALSE;
	}

	return TRUE;
}


static BOOL CheckTextureSize(D3DRender *pRender, DWORD width, DWORD height)
{
	return width <= pRender->m_Caps.dwMaxTextureWidth &&
		height <= pRender->m_Caps.dwMaxTextureHeight;
}


static Direct3dTexture* d3d_SetupNewTexture(D3DRender *pRender, CTexture *pInTexture)
{
	Direct3dTexture *pRet;
	CDib *pDib;
	DWORD extraCaps, width, height;
	LPDIRECTDRAWSURFACE4 pSurface;
	IDirect3DTexture2 *pTexture;
	D3DTEXTUREHANDLE textureHandle;
	BOOL bRet, bSpecialMipmap;
	DWORD i, testWidth, testHeight;
	int iMipmap;
	DtxHeader *pHeader;
	CTexture *pSystemTexture;


	pSystemTexture = pInTexture;
	pDib = pSystemTexture->m_pDib;
	bSpecialMipmap = FALSE;
	iMipmap = 0;

	if(!CheckTextureSize(pRender, pDib->GetWidth(), pDib->GetHeight()))
	{
		// See if there is an acceptable mipmap.
		testWidth = pSystemTexture->m_Header.m_BaseWidth;
		testHeight = pSystemTexture->m_Header.m_BaseHeight;
		pHeader = &pSystemTexture->m_Header;
		for(i=0; i < pHeader->m_nMipmaps; i++)
		{
			if(CheckTextureSize(pRender, testWidth, testHeight))
			{
				iMipmap = (int)i;
				bSpecialMipmap = TRUE;
				break;
			}

			testWidth >>= 1;
			testHeight >>= 1;
		}

		// If we didn't find one, get outta here..
		if(!bSpecialMipmap)
			return NULL;
	}

	// Load the mipmap if we need to.
	if(bSpecialMipmap)
	{
		pSystemTexture = dib_LoadMipmap(pInTexture->m_pIdent, iMipmap);
		if(!pSystemTexture)
			return NULL;

		pDib = pSystemTexture->m_pDib;

		// Make sure it's really valid.
		if(!CheckTextureSize(pRender, pDib->GetWidth(), pDib->GetHeight()))
		{
			return NULL;
		}
	}

	
	// Try to create a surface.
	if(pRender->m_Caps.dwDevCaps & D3DDEVCAPS_TEXTURESYSTEMMEMORY)
		extraCaps = DDSCAPS_SYSTEMMEMORY;
	else
		extraCaps = DDSCAPS_VIDEOMEMORY;

	while(1)
	{
		pSurface = d3d_CreateTextureSurface(pRender, pDib->GetWidth(), pDib->GetHeight(), extraCaps);
		if(pSurface)
		{
			break;
		}
		else
		{
			// Try to free some texture memory.
			if(!d3d_FreeSomeSpace(pRender))
				return NULL;
		}
	}
	
	// Get the IDirect3DTexture2.
	pSurface->QueryInterface(IID_IDirect3DTexture2, (void**)&pTexture);
	if(!pTexture)
	{
		pSurface->Release();
		return NULL;
	}

	// Translate colors and put the texture in video memory.
	bRet = d3d_PutTextureIntoSurface(pRender, pSystemTexture, pSurface, pTexture);
	if(!bRet)
	{
		pSurface->Release();
		pTexture->Release();
		return NULL;
	}

	
	// Create the texture.
	pRet = new Direct3dTexture;
	pRet->m_Link.m_pData = pRet;
	pRet->m_pTexture = pTexture;
	pRet->m_pSurface = pSurface;
	pRet->m_pSystemTexture = pInTexture;
	pRet->m_pDrawMgr = pRender;
	pRet->m_uMul = 1.0f / (float)pSystemTexture->m_Header.m_BaseWidth;
	pRet->m_vMul = 1.0f / (float)pSystemTexture->m_Header.m_BaseHeight;

	pRet->m_uMul *= pInTexture->m_UIMipmapScale;
	pRet->m_vMul *= pInTexture->m_UIMipmapScale;

	pInTexture->m_Bindings[pRender->m_BindingNumber] = pRet;

	dl_Insert(pRender->m_Textures.m_pPrev, &pRet->m_Link);

	if(bSpecialMipmap)
	{
		dib_DestroyDibTexture(pSystemTexture);
	}

	return pRet;
}


static WORD d3d_GetBitCount(DWORD mask)
{
	WORD	i, bits=0;
	
	for(i=0; i < 32; i++)
	{
		if(mask & 1)
			++bits;

		mask >>= 1;
	}

	return bits;
}


static int d3d_GetFormatCode(TextureFormat *pFormat)
{
	// We only support 16 bit textures currently..
	if(pFormat->m_PF.dwRGBBitCount != 16)
		return -1;

	if(pFormat->m_PF.dwFlags & DDPF_ALPHAPIXELS)
	{
		// We like alpha pixels a little better than normal so add 3 to its value.
		return pFormat->m_RBits + pFormat->m_GBits + pFormat->m_BBits + 3;
	}
	else if(pFormat->m_PF.dwFlags & DDPF_RGB)
	{
		// The more color bits, the better!
		return pFormat->m_RBits + pFormat->m_GBits + pFormat->m_BBits;
	}

	// Unsupported format.
	return -1;
}


// --------------------------------------------------------------------------------- //
// Gets the right and left shifts that you'll need to take and 8-bit color and turn
// it into a WORD, given its mask (the bits it occupies) in a WORD.
// --------------------------------------------------------------------------------- //

static void d3d_GetMaskShifts(DWORD mask, DWORD *pRightShift, DWORD *pLeftShift)
{
	DWORD	bitCount = d3d_GetBitCount(mask);
	DWORD	i, shift, testMask;

	*pRightShift = 8 - bitCount;
	
	// Starting with the MSB of a WORD, find out where mask starts in the WORD.
	testMask = 0x8000;
	for(i=0; i < 16; i++)
	{
		if(testMask & mask)
			break;
		
		testMask >>= 1;
	}

	shift = 16 - i;
	*pLeftShift = shift - bitCount;
}


static HRESULT WINAPI d3d_TextureFormatCallback(LPDDPIXELFORMAT pPF, LPVOID pUser)
{
	TextureFormat *pFormat;
	D3DRender *pRender = (D3DRender*)pUser;

	// Copy the desc over.
	pFormat = (TextureFormat*)dalloc(sizeof(TextureFormat));
	memcpy(&pFormat->m_PF, pPF, sizeof(pFormat->m_PF));

	pFormat->m_RBits = d3d_GetBitCount(pPF->dwRBitMask);
	pFormat->m_GBits = d3d_GetBitCount(pPF->dwGBitMask);
	pFormat->m_BBits = d3d_GetBitCount(pPF->dwBBitMask);
	pFormat->m_ABits = d3d_GetBitCount(pPF->dwRGBAlphaBitMask);
	
	d3d_GetMaskShifts(pPF->dwRBitMask, &pFormat->m_RRightShift, &pFormat->m_RLeftShift);
	d3d_GetMaskShifts(pPF->dwGBitMask, &pFormat->m_GRightShift, &pFormat->m_GLeftShift);
	d3d_GetMaskShifts(pPF->dwBBitMask, &pFormat->m_BRightShift, &pFormat->m_BLeftShift);

	pFormat->m_TextureCode = d3d_GetFormatCode(pFormat);

	pFormat->m_Link.m_pData = pFormat;
	dl_Insert(&pRender->m_TextureFormats, &pFormat->m_Link);	

	return D3DENUMRET_OK;
}


static TextureFormat* d3d_SelectIdealTextureFormat(D3DRender *pRender)
{
	DLink *pCur;
	TextureFormat *pFormat, *pBest=NULL;
	int iBest = -1;

	for(pCur=pRender->m_TextureFormats.m_pNext; pCur != &pRender->m_TextureFormats; pCur=pCur->m_pNext)
	{
		pFormat = (TextureFormat*)pCur->m_pData;

		if(pFormat->m_TextureCode > iBest)
		{
			iBest = pFormat->m_TextureCode;
			pBest = pFormat;
		}
	}

	return pBest;
}

static void d3d_AddBinding(D3DRender* pRender, CTexture* pSystemTexture)
{
	//safety check
	ASSERT(pRender);
	ASSERT(pSystemTexture);

	void **pNewBindings;

	// Allocate the block for the binding
	pNewBindings = (void**)dalloc(sizeof(void*) * (pRender->m_BindingNumber + 1));
	memset(pNewBindings, 0, sizeof(void*) * (pRender->m_BindingNumber + 1));
	memcpy(pNewBindings, pSystemTexture->m_Bindings, sizeof(void*) * pSystemTexture->m_nBindings);
	
	if(pSystemTexture->m_Bindings)
	{
		dfree(pSystemTexture->m_Bindings);
	}

	pSystemTexture->m_Bindings	= pNewBindings;
	pSystemTexture->m_nBindings = pRender->m_BindingNumber + 1;
}


static BOOL d3d_SetupPolyTextureBase(D3DRender *pRender, DFileIdent* pTextureFile)
{
	CTexture *pSystemTexture;
	Direct3dTexture *pTexture;
	void **pNewBindings;

	if(!pTextureFile)
	{
		return FALSE;
	}

	// Get the system memory texture in memory.
	pSystemTexture = dib_GetDibTexture(pTextureFile);	
	if(!pSystemTexture)
	{
		pRender->m_pDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
		return FALSE;
	}

	// Get its binding.
	if(pRender->m_BindingNumber < pSystemTexture->m_nBindings)
	{
		pTexture = (Direct3dTexture*)pSystemTexture->m_Bindings[pRender->m_BindingNumber];
		if(pTexture)
		{
			d3d_SetupTextureSlot(pRender, pTexture);
			return TRUE;
		}
	}
	else
	{
		// Add bindings for it..
		d3d_AddBinding(pRender, pSystemTexture);
	}

	// No binding.. make a Direct3dTexture and get it in texture memory.
	pTexture = d3d_SetupNewTexture(pRender, pSystemTexture);
	if(pTexture)
	{
		d3d_SetupTextureSlot(pRender, pTexture);
		return TRUE;
	}
	else
	{
		pRender->m_pDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
		return FALSE;
	}
}

//(YF 1-10-01)

// Sets D3D up to use this texture slot.
static inline void d3d_SetupDetailTextureSlot(D3DRender *pRender, Direct3dTexture *pTexture)
{
	HRESULT hResult;

	// Move it to the end of the LRU.
	dl_Remove(&pTexture->m_Link);
	dl_Insert(pRender->m_Textures.m_pPrev, &pTexture->m_Link);

	hResult = pRender->m_pDevice->SetTexture( 1, pTexture->m_pTexture );

	//blend mode could be either D3DTOP_ADDSIGNED or D3DTOP_MODULATE. 
	if(GetApp()->GetOptions().GetDisplayOptions()->IsDetailTexAdditive())
	{
		pRender->m_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
	}
	else
	{
		pRender->m_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}

	pRender->m_pDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
}





// ------------------------------------------------------------------------ //
// Interface functions.
// ------------------------------------------------------------------------ //


//this function will, given the desired size for a texture, find the appropriate
//size that should be used
void d3d_FindTextureSize(uint32 nDesiredX, uint32 nDesiredY, uint32& nOutX, uint32& nOutY)
{
	nOutX = nOutY = 0;
	for(uint32 nCurrSize = 0; nCurrSize < NUM_VALID_TEXTURE_SIZES; nCurrSize++)
	{
		if((nOutX == 0) && (nDesiredX <= g_TextureSizes[nCurrSize]))
			nOutX = g_TextureSizes[nCurrSize];

		if((nOutY == 0) && (nDesiredY <= g_TextureSizes[nCurrSize]))
			nOutY = g_TextureSizes[nCurrSize];
	}
}


void d3d_UnbindFromTexture(void *pBinding)
{
	Direct3dTexture *pTexture;

	pTexture = (Direct3dTexture*)pBinding;
	d3d_UnloadTexture(pTexture, pTexture->m_pDrawMgr->m_BindingNumber);
}


BOOL d3d_InitTextureStuff(D3DRender *pRender)
{
	DWORD i, j;
	

	// Select a texture format.
	pRender->m_pDevice->EnumTextureFormats(d3d_TextureFormatCallback, pRender);

	// Print out the texture formats.
	pRender->m_pTextureFormat = d3d_SelectIdealTextureFormat(pRender);

	if(!pRender->m_pTextureFormat)
	{
		AddDebugMessage("Couldn't find a supported texture format!");
		return FALSE;
	}

	return TRUE;
}


void d3d_TermTextureStuff(D3DRender *pRender)
{
	DLink *pCur, *pNext;

	// Unload all the textures.
	pCur = pRender->m_Textures.m_pNext;
	while(pCur != &pRender->m_Textures)
	{
		pNext = pCur->m_pNext;
		d3d_UnloadTexture((Direct3dTexture*)pCur->m_pData, pRender->m_BindingNumber);
		pCur = pNext;
	}

	// Unload all the texture formats.
	pCur = pRender->m_TextureFormats.m_pNext;
	while(pCur != &pRender->m_TextureFormats)
	{
		pNext = pCur->m_pNext;
		dfree(pCur->m_pData);
		pCur = pNext;
	}

	dl_TieOff(&pRender->m_Textures);
	dl_TieOff(&pRender->m_TextureFormats);
}

void d3d_UnsetDetailTexture( D3DRender* pRender )
{
	//disable this stage
	pRender->m_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
}


BOOL d3d_SetupDetailTexture(D3DRender *pRender, DFileIdent* pDetailFile,
												DFileIdent* pBaseTexFile)
{
	ASSERT(pRender);

	//make sure that we have a valid texture
	if( !pDetailFile )
	{
		//this should be unneccessary
		//d3d_UnsetDetailTexture( pRender );
		return FALSE;
	}

	CTexture *pSystemTexture;
	Direct3dTexture *pTexture;

	// Get the system memory texture in memory.
	pSystemTexture = dib_GetDibTexture( pDetailFile );
	if( !pSystemTexture )
	{
		d3d_UnsetDetailTexture( pRender );
		return FALSE;
	}

	//set up the scaling information for the detail texture rendering
	CTexture* pBaseTex = dib_GetDibTexture( pBaseTexFile );
	if(pBaseTex)
	{
		COptionsDisplay* pDisplayOpt = GetApp()->GetOptions().GetDisplayOptions();

		g_fDetailTextureScale = pBaseTex->m_Header.GetDetailTextureScale() *
							pDisplayOpt->GetDetailTexScale();

		float fAngle = MATH_DEGREES_TO_RADIANS(	pDisplayOpt->GetDetailTexAngle() + 
												pBaseTex->m_Header.GetDetailTextureAngle());

		g_fDetailTextureAngleCos = (float)cos(fAngle);
		g_fDetailTextureAngleSin = (float)sin(fAngle);
	}

	// Get its binding.
	if(pRender->m_BindingNumber < pSystemTexture->m_nBindings)
	{
		pTexture = (Direct3dTexture*)pSystemTexture->m_Bindings[pRender->m_BindingNumber];
		if(pTexture)
		{
			d3d_SetupDetailTextureSlot(pRender, pTexture);
			return TRUE;
		}
	}
	else
	{
		// Add bindings for it..
		d3d_AddBinding(pRender, pSystemTexture);
	}

	// No binding.. make a Direct3dTexture and get it in texture memory.
	pTexture = d3d_SetupNewTexture(pRender, pSystemTexture);
	if(pTexture)
	{
		d3d_SetupDetailTextureSlot(pRender, pTexture);
		return TRUE;
	}
	else
	{
		d3d_UnsetDetailTexture( pRender );
		return FALSE;
	}
}

BOOL d3d_SetupPolyTexture(D3DRender *pRender, DFileIdent_t *pTextureFile, DFileIdent_t *pDetailFile)
{
	if(!pTextureFile)
		return FALSE;

	BOOL retval = d3d_SetupPolyTextureBase( pRender, pTextureFile );

	// begin yf 1-10-01
	// setup detail texture
	if( retval && GetApp()->GetOptions().GetDisplayOptions()->IsDetailTexEnabled() )
	{
		d3d_SetupDetailTexture( pRender, pDetailFile, pTextureFile );
	}
	else
	{
		d3d_UnsetDetailTexture( pRender );
	}

	return retval;
}




