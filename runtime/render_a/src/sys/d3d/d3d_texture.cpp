#include "precompile.h"

#include "d3d_texture.h"
#include "d3d_convar.h"
#include "common_stuff.h"
#include "renderstruct.h"
#include "dtxmgr.h"
#include "common_draw.h"
#include "d3d_shell.h"
#include "colorops.h"
#include "rendererframestats.h"

// Globals.
CTextureManager			g_TextureManager;
FormatMgr				g_FormatMgr;
DECLARE_LTLINK(g_Textures);


static bool ShouldFreeSystemTexture(const SharedTexture* pTexture)
{
	assert(pTexture);

	//only free them if we have texture caching disabled
	if(g_CV_CacheTextures.m_Val)
		return false;

	//make sure that we could recreate this texture if needed
	if(pTexture->m_pFile == NULL)
		return false;

	//it is free to be discared
	return true;
}


void d3d_PrintFormatInfo(const char* pStart, D3DFORMAT Format)
{
	char spec[256]; 
	d3d_D3DFormatToString(Format, spec, sizeof(spec));
	g_pStruct->ConsolePrint("%s%s", pStart, spec);
}

// Transfers the system memory TextureData to the video testure.
bool d3d_TransferTexture(RTexture* pTexture, TextureData* pTextureData)
{
	uint32 iRTextureMipLevels = 0;
	if (pTexture->IsCubeMap()) 
	{	
		// Cube map texture...
		iRTextureMipLevels				= pTexture->m_pD3DCubeTexture->GetLevelCount(); 
	}
	else 
	{
		// Normal texture...
		iRTextureMipLevels				= pTexture->m_pD3DTexture->GetLevelCount(); 
	}

	BPPIdent SrcBPP						= pTextureData->m_Header.GetBPPIdent();

	if (iRTextureMipLevels > pTextureData->m_Header.m_nMipmaps) 
	{	
		// Check stuff...
		AddDebugMessage(1, "d3d_TransferTexture: mipmap count is invalid!"); return false; 
	}

	// Go thru each mipmap and copy it.
	for (uint32 i = 0; i < iRTextureMipLevels; ++i) 
	{
		uint32 iSrcLvl = i + pTexture->m_iStartMipmap;
		if (!g_TextureManager.UploadRTexture(pTextureData, iSrcLvl, pTexture, i)) 
		{ 
			return false; 
		} 
	} 

	return true;
}

// Updates the aspect ratio so it's valid for the current settings.
// Makes it square if D3DPTEXTURECAPS_SQUAREONLY is set.
// Makes its aspect ratio valid based on g_CV_MaxAspectRatio.
void AdjustAspectRatio(uint32 width, uint32 height, uint32 *outWidth, uint32 *outHeight)
{
	if (g_Device.GetDeviceCaps()->TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) 
	{
		width = height = LTMAX(width, height); 
	}
	else 
	{
		if (g_CV_MaxTexAspectRatio > 0) 
		{
			uint32* pMin = width > height ? &height : &width;
			uint32* pMax = width > height ? &width  : &height;
			if ((int)(*pMax / *pMin) > g_CV_MaxTexAspectRatio) 
			{
				*pMin = *pMax / (uint32)g_CV_MaxTexAspectRatio; 
			} 
		} 
	}
	*outWidth  = width;
	*outHeight = height;
}

// Returns true if it should use S3TC for a texture with this format.
bool d3d_ShouldUseS3TC(BPPIdent bpp)
{
	return (g_CV_S3TCEnable.m_Val && g_TextureManager.IsS3TCFormatSupported(bpp));
}

// Gets the first usable mipmap size the renderer can create.  Returns -1 if none can be used.
int32 d3d_GetFirstUsableMipmap(TextureData *pTexture)
{
	uint32 maxWidth  = LTMIN((uint32)g_CV_MaxTextureSize, g_Device.GetDeviceCaps()->MaxTextureWidth);
	uint32 maxHeight = LTMIN((uint32)g_CV_MaxTextureSize, g_Device.GetDeviceCaps()->MaxTextureHeight);
	if (!maxWidth)  maxWidth  = 256;		// Older 3DFX Voodoo1 drivers have a bug where these are 0.
	if (!maxHeight) maxHeight = 256;

	for (uint32 i=0; i < pTexture->m_Header.m_nMipmaps; ++i) {
		if (pTexture->m_Mips[i].m_Width <= maxWidth && pTexture->m_Mips[i].m_Height <= maxHeight)
			return (int)i; }

	return -1;
}

// Don't call this.. it's only called by d3d_SetTexture.
RTexture* d3d_CreateAndLoadTexture(SharedTexture *pSharedTexture)
{
	RTexture*	 pRet = NULL; 

	// Ask the engine to get the system memory version ready.
	TextureData* pTextureData = g_pStruct->GetTexture(pSharedTexture);
	if (pTextureData) 
	{
		// Setup the surfaces and copy the texture data over.
		pRet = g_TextureManager.CreateRTexture(pSharedTexture,pTextureData);
		if (pRet) 
		{
			// Alrighty!  Put the texture data in there and load it up.
			if (!d3d_TransferTexture(pRet, pTextureData)) 
			{
				AddDebugMessage(4, "Unable to transfer texture data to video memory.");
				g_TextureManager.FreeTexture(pRet); pRet = NULL; 
			}
			else
			{
				if (ShouldFreeSystemTexture(pSharedTexture))
					g_pStruct->FreeTexture(pSharedTexture); 
			}
		}
	}

	return pRet;
}

void d3d_BindTexture(SharedTexture *pSharedTexture, bool bTextureChanged)
{
	if (!pSharedTexture) 
	{
		return; 
	}

	if (pSharedTexture->m_pRenderData) 
	{	
		// Convert it and stuff.
		if (bTextureChanged) 
		{
			RTexture* pRTexture		  = (RTexture*)pSharedTexture->m_pRenderData; 

			TextureData* pTextureData = g_pStruct->GetTexture(pSharedTexture);
			if (pTextureData) 
			{
				// Alrighty!  Put the texture data in there and load it up.
				if(!d3d_TransferTexture(pRTexture, pTextureData)) 
				{
					AddDebugMessage(4, "Unable to transfer texture data to video memory."); 
				}
				else
				{			
					if (ShouldFreeSystemTexture(pSharedTexture))
						g_pStruct->FreeTexture(pSharedTexture); 
				}
			} 
		} 
	}
	else 
	{
		d3d_CreateAndLoadTexture(pSharedTexture); 
	}
}

void d3d_UnbindTexture(SharedTexture *pSharedTexture)
{
	RTexture *pTexture;
	if (pSharedTexture->m_pRenderData) 
	{
		pTexture = (RTexture*)pSharedTexture->m_pRenderData;
		g_TextureManager.FreeTexture(pTexture); 
	}
}

static void CalcRTextureMemoryUse(TextureData* pTexture, RTexture* pRTexture, uint32 nNumMipMaps)
{
	//now we need to figure out how much memory this texture is actually taking up. We need this to
	//be as accurate as possible so that we can get a reasonable estimate of texture usage per scene
	assert(pTexture);
	assert(pRTexture);

	//assume at first that we aren't compressed, so it expands to a 32 bit texture
	uint32 nBaseUncompressedMemory	= CalcImageSize(BPP_32, pRTexture->m_BaseWidth, pRTexture->m_BaseHeight);
	uint32 nBaseMemory				= nBaseUncompressedMemory;

	//see if we are actually compressed though
	if(g_TextureManager.IsS3TCFormatSupported(pTexture->m_Header.GetBPPIdent()))
	{
		//we are a compressed texture, so use that size
		nBaseMemory = CalcImageSize(pTexture->m_Header.GetBPPIdent(), pRTexture->m_BaseWidth, pRTexture->m_BaseHeight);
	}

	//handle adjustments for cube maps
	if(pRTexture->IsCubeMap())
	{
		nBaseUncompressedMemory *= 6;
		nBaseMemory *= 6;
	}
	
	//now we need to add in support for the mip maps (note we start at 1, since 0 is the base image)
	uint32 nMipMemory				= 0;
	uint32 nUncompressedMipMemory	= 0;

	float fMipScale = 0.25f;
	for(uint32 nCurrMip = 1; nCurrMip < nNumMipMaps; nCurrMip++)
	{
		nMipMemory += (uint32)(nBaseMemory * fMipScale);
		nUncompressedMipMemory += (uint32)(nBaseUncompressedMemory * fMipScale);
		fMipScale *= 0.25f;
	}

	//now figure out our totals
	pRTexture->m_TextureMem.m_nMemory				= nBaseMemory + nMipMemory;
	pRTexture->m_TextureMem.m_nUncompressedMemory	= nBaseUncompressedMemory + nUncompressedMipMemory;
}


// Exposed functions.
bool CTextureManager::Init(bool bFullInit)
{
	if (bFullInit) 
	{
		// Still using this puffy g_Textures list...
		dl_TieOff(&g_Textures);	
	}

	memset(m_TextureFormats, 0, sizeof(m_TextureFormats));

	m_RTextureBank.Init(64, 0);

	if (!SelectTextureFormats())			
	{ 
		// Select our prefered texture formats...
		return false; 
	}				

	return (m_bInitialized = true);
}

void CTextureManager::Term(bool bFullTerm)
{
	if (bFullTerm) {
		FreeAllTextures(); }												// Free all the Textures...

	m_RTextureBank.Term();

	memset(m_TextureFormats, 0, sizeof(m_TextureFormats));
	m_bInitialized = false;
}

bool CTextureManager::QueryDDSupport(PFormat* Format)
{
	D3DAdapterInfo* pAdapterInfo = g_Device.GetAdapterInfo();
	D3DDeviceInfo*	pDeviceInfo  = g_Device.GetDeviceInfo();
	D3DModeInfo*	pDisplayMode = g_Device.GetModeInfo();
	D3DFORMAT		D3DSrcFormat = d3d_PFormatToD3DFormat(Format);

	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DSrcFormat) == D3D_OK) { return true; }
	return false;
}

// Figures out what format we'll use based on the flags...
D3DFORMAT CTextureManager::QueryDDFormat1(BPPIdent BPP, uint32 iFlags)
{
	uint32 iFormat = NULL;				// Pick a texture format.

	if (d3d_ShouldUseS3TC(BPP)) 
		return g_TextureManager.S3TCFormatConv(BPP);

	//handle bumpmap formats first, then normal texture formats
	if (iFlags & DTX_LUMBUMPMAP)
		iFormat = FORMAT_LUMBUMPMAP;
	else if (iFlags & DTX_BUMPMAP)
		iFormat = FORMAT_BUMPMAP;
	else if (!(iFlags & DTX_PREFER16BIT) && g_CV_32BitTextures && g_TextureManager.m_TextureFormats[FORMAT_32BIT].m_bValid) 
		iFormat = FORMAT_32BIT; 
	else if (iFlags & DTX_PREFER5551) 
		iFormat = FORMAT_FULLBRITE; 
	else if (iFlags & DTX_PREFER4444) 
		iFormat = FORMAT_4444; 
	else if (iFlags & DTX_FULLBRITE)  
		iFormat = FORMAT_FULLBRITE;	// If they didn't choose a specific alpha mode but chose fullbrite, then use 5551.
	else 
		iFormat = FORMAT_NORMAL;

	//see if we stumbled across an invalid format
	if(!g_TextureManager.m_TextureFormats[iFormat].m_bValid)
		return D3DFMT_UNKNOWN;

	return g_TextureManager.m_TextureFormats[iFormat].m_PF;
}

bool CTextureManager::ConvertTexDataToDD(uint8* pSrcData, PFormat* SrcFormat, uint32 SrcWidth, uint32 SrcHeight, uint8* pDstData, PFormat* DstFormat, BPPIdent eDstType, uint32 nDstFlags, uint32 DstWidth, uint32 DstHeight)
{
 	D3DFORMAT D3DSrcFormat = d3d_PFormatToD3DFormat(SrcFormat); assert(D3DSrcFormat != D3DFMT_UNKNOWN);
	D3DFORMAT D3DDstFormat = QueryDDFormat1(eDstType, nDstFlags); assert(D3DDstFormat != D3DFMT_UNKNOWN);
	
	// Create a quick little surface to convert into....
	LPDIRECT3DSURFACE9 pD3DDstSurface = NULL; LPDIRECT3DTEXTURE9 pD3DDstTexture = NULL;
	HRESULT hResult = PD3DDEVICE->CreateTexture(DstWidth,DstHeight,1,NULL,D3DDstFormat,D3DPOOL_SYSTEMMEM,&pD3DDstTexture);
	if (hResult != D3D_OK || !pD3DDstTexture) 
		return false;

	hResult = pD3DDstTexture->GetSurfaceLevel(0,&pD3DDstSurface);
	if (hResult != D3D_OK || !pD3DDstSurface) 
		return false;

	uint32 SrcPitch = g_TextureManager.GetPitch(D3DSrcFormat,SrcWidth);
	RECT SrcRect; SrcRect.left = 0; SrcRect.top = 0; SrcRect.right = SrcWidth; SrcRect.bottom = SrcHeight;

	hResult = D3DXLoadSurfaceFromMemory(pD3DDstSurface,NULL,NULL,pSrcData,D3DSrcFormat,SrcPitch,NULL,&SrcRect,D3DX_FILTER_LINEAR,0);

	// Lock it and copy out it's data...
	D3DLOCKED_RECT LockedRect; pD3DDstSurface->LockRect(&LockedRect,NULL,NULL); 
	uint8* pD3DSrcData = (uint8*)LockedRect.pBits;
	for (uint y = 0; y < DstHeight; ++y) 
	{
		memcpy(pDstData,pD3DSrcData,DstWidth*DstFormat->GetBytesPerPixel());
		pDstData    += DstWidth * DstFormat->GetBytesPerPixel();
		pD3DSrcData += LockedRect.Pitch; 
	}
	pD3DDstSurface->UnlockRect();

	pD3DDstSurface->Release();
	pD3DDstTexture->Release();

	if (hResult != D3D_OK) 
		return false;

	return true;
}

bool CTextureManager::UploadRTexture(TextureData* pSrcTexture, uint32 iSrcLvl, RTexture* pDstTexture, uint32 iDstLvl)
{
	TextureMipData* pSrcMip = &pSrcTexture->m_Mips[iSrcLvl];
	PFormat SrcFormat; 
	pSrcTexture->SetupPFormat(&SrcFormat);

	D3DFORMAT D3DSrcFormat = QueryDDFormat1(pSrcTexture->m_Header.GetBPPIdent(), pSrcTexture->m_Header.m_IFlags);

	if (pDstTexture->IsCubeMap()) 
	{
		// Cube map texture...
		LPDIRECT3DCUBETEXTURE9 pD3DDstTexture = pDstTexture->m_pD3DCubeTexture;

		uint8* pSrcData = pSrcMip->m_Data; 
		uint32 SrcPitch = GetPitch(D3DSrcFormat,pSrcMip->m_Width);

		RECT SrcRect; 
		SrcRect.left	= 0; 
		SrcRect.top		= 0; 
		SrcRect.right	= pSrcMip->m_Width; 
		SrcRect.bottom	= pSrcMip->m_Height;

		for (uint32 i = 0; i < 6; ++i) 
		{
			LPDIRECT3DSURFACE9 pDstSurface = NULL; 

			pD3DDstTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)i,iDstLvl,&pDstSurface);
			if (!pDstSurface) 
				return false;

			HRESULT hResult = D3DXLoadSurfaceFromMemory(pDstSurface,NULL,NULL,pSrcData,D3DSrcFormat,SrcPitch,NULL,&SrcRect,D3DX_FILTER_NONE,0);

			uint32 iRefCnt = pDstSurface->Release();
			if (hResult != D3D_OK) 
				return false; 
	
			pSrcData += pSrcMip->m_dataSize;
		} 
	}
	else 
	{
		// Normal texture..
		LPDIRECT3DTEXTURE9 pD3DDstTexture = pDstTexture->m_pD3DTexture;

		LPDIRECT3DSURFACE9 pDstSurface = NULL; 
		pD3DDstTexture->GetSurfaceLevel(iDstLvl,&pDstSurface);
		if (!pDstSurface) return false;

		uint8* pSrcData = pSrcMip->m_Data; 
		uint32 SrcPitch = GetPitch(D3DSrcFormat,pSrcMip->m_Width);
		
		RECT SrcRect; 
		SrcRect.left = 0; 
		SrcRect.top = 0; 
		SrcRect.right = pSrcMip->m_Width; 
		SrcRect.bottom =  pSrcMip->m_Height;

		HRESULT hResult;
		LT_MEM_TRACK_ALLOC(hResult = D3DXLoadSurfaceFromMemory(pDstSurface,NULL,NULL,pSrcData,D3DSrcFormat,SrcPitch,NULL,&SrcRect,D3DX_FILTER_NONE,0), LT_MEM_TYPE_RENDERER);
		
		pDstSurface->Release();
		if (hResult != D3D_OK) 
			return false; 
	}

	return true;
}

// This sets up pTexture with all the Direct3D stuff it needs and copies
// in the texture data from pTextureData.
// pBuild should have m_pSharedTexture, m_pTextureData, and m_iStage initted.
RTexture* CTextureManager::CreateRTexture(SharedTexture* pSharedTexture, TextureData* pTextureData)
{
	// [KLS] 10/17/01 Removed assert if not initialized, this can happen when you
	// ALT-TAB (the assert was really annoying when debugging and didn't represent 
	// a real problem).
	if (!m_bInitialized) return NULL;

	//flags for the render texture
	uint32 nFlags = 0;

	//add any custom flags
	if(pTextureData->m_Header.m_IFlags & DTX_CUBEMAP)
		nFlags |= RT_CUBEMAP;

	if(pTextureData->m_Header.m_IFlags & DTX_FULLBRITE)
		nFlags |= RT_FULLBRITE;

	if(pTextureData->m_Header.m_IFlags & DTX_BUMPMAP)
		nFlags |= RT_BUMPMAP;

	if(pTextureData->m_Header.m_IFlags & DTX_LUMBUMPMAP)
		nFlags |= RT_LUMBUMPMAP;


	// Figure out which format we're going to use...
	BPPIdent bpp		= pTextureData->m_Header.GetBPPIdent();
	D3DFORMAT iFormat	= QueryDDFormat1(bpp,pTextureData->m_Header.m_IFlags);

	uint32 baseMipmapOffset = 0;

	// If not using S3TC, add a mipmap offset.
	if (!d3d_ShouldUseS3TC(pTextureData->m_Header.GetBPPIdent()))		
		baseMipmapOffset += pTextureData->m_Header.GetNonS3TCMipmapOffset();

	baseMipmapOffset	= LTMIN(baseMipmapOffset, pTextureData->m_Header.m_nMipmaps-1);

	// Make sure we're using a valid size.
	int32 firstUsable	= d3d_GetFirstUsableMipmap(pTextureData);				

	if (firstUsable == -1) 
		return NULL;

	baseMipmapOffset	= LTMAX(baseMipmapOffset, firstUsable);

	int32 nMipmaps		= pTextureData->m_Header.m_Extra[1];
	if (nMipmaps == 0) 
		nMipmaps = NUM_MIPMAPS;

	int32 maxMipmaps	= pTextureData->m_Header.m_nMipmaps - baseMipmapOffset;	
	if (maxMipmaps == 0) 
		return NULL;

	float fAngle		= MATH_DEGREES_TO_RADIANS((float)(pTextureData->m_Header.GetDetailTextureAngle()));
	int32 nMipsToCreate	= (uint8)LTCLAMP(nMipmaps, 1, maxMipmaps);

	// Create the RTexture.
	RTexture* pRTexture;
	LT_MEM_TRACK_ALLOC(pRTexture = m_RTextureBank.Allocate(), LT_MEM_TYPE_RENDERER);
	
	if (!pRTexture)	
		return NULL; 
	
	pRTexture->m_Flags				 = (uint8)nFlags;
	pRTexture->m_BaseWidth			 = pTextureData->m_Mips[baseMipmapOffset].m_Width;
	pRTexture->m_BaseHeight			 = pTextureData->m_Mips[baseMipmapOffset].m_Height;
	pRTexture->m_DetailTextureScale	 = pTextureData->m_Header.GetDetailTextureScale();
	pRTexture->m_DetailTextureAngleC = (float)cos(fAngle);
	pRTexture->m_DetailTextureAngleS = (float)sin(fAngle);
	pRTexture->m_iStartMipmap		 = (uint8)baseMipmapOffset;

	ConParse cParse; 
	
	cParse.Init(pTextureData->m_Header.m_CommandString);	// Use alpha reference value.
	if (cParse.ParseFind("AlphaRef", false, 1)) 
	{ 
		pRTexture->m_AlphaRef = (uint16)atoi(cParse.m_Args[1]); 
	}
	else 
	{ 
		pRTexture->m_AlphaRef = ALPHAREF_NONE; 
	}

	//read in the mipmap offset value
	cParse.Init(pTextureData->m_Header.m_CommandString);	
	if (cParse.ParseFind("MipMapBias", false, 1)) 
	{ 
		pRTexture->m_fMipMapBias = (float)atof(cParse.m_Args[1]);
	}

	// Adjust size for cards that require square textures...
	uint32 iTexWidth  = pRTexture->m_BaseWidth; 
	uint32 iTexHeight = pRTexture->m_BaseHeight;

	if (iFormat == D3DFMT_DXT1 || iFormat == D3DFMT_DXT3 || iFormat == D3DFMT_DXT5) 
	{	
		// Force it to be square?  Compressed textures don't need this.
		AdjustAspectRatio(iTexWidth, iTexHeight, &iTexWidth, &iTexHeight); 
	}

	// Create the D3D Texture...

	if (pRTexture->IsCubeMap()) 
	{			
		// It's a cubemap...

		// Cube maps must be square...
		assert(iTexWidth == iTexHeight);			

		// Don't create mipmaps if cube mapping and mipmapping aren't supported together
		if ((g_Device.GetDeviceCaps()->TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP) == 0)
			nMipsToCreate = 0;

		HRESULT hResult = PD3DDEVICE->CreateCubeTexture(max(iTexWidth,iTexHeight),nMipsToCreate,NULL,iFormat,D3DPOOL_MANAGED,&pRTexture->m_pD3DCubeTexture);
		if (hResult != D3D_OK) 
		{
			AddDebugMessage(4, "Unable to create (%d) cube texture surface.", max(iTexWidth,iTexHeight));
			return NULL; 
		}

		// Little double check...
		D3DSURFACE_DESC MipDesc;					
		pRTexture->m_pD3DCubeTexture->GetLevelDesc(0,&MipDesc);
		assert(MipDesc.Width == pTextureData->m_Mips[pRTexture->m_iStartMipmap].m_Width && MipDesc.Height == pTextureData->m_Mips[pRTexture->m_iStartMipmap].m_Height);

		// Set priority!
		pRTexture->m_pD3DCubeTexture->SetPriority(pTextureData->m_Header.GetTexturePriority()); 
	}
	else 
	{	
		// It's just a normal texture...
		HRESULT hResult = PD3DDEVICE->CreateTexture(iTexWidth,iTexHeight,nMipsToCreate,NULL,iFormat,D3DPOOL_MANAGED,&pRTexture->m_pD3DTexture);
		if (hResult != D3D_OK) 
		{
			AddDebugMessage(4, "Unable to create (%dx%d) texture surface.", iTexWidth, iTexHeight);
			return NULL; 
		}

		// Little double check...
		D3DSURFACE_DESC MipDesc;					
		pRTexture->m_pD3DTexture->GetLevelDesc(0,&MipDesc);
		assert(MipDesc.Width == pTextureData->m_Mips[pRTexture->m_iStartMipmap].m_Width && MipDesc.Height == pTextureData->m_Mips[pRTexture->m_iStartMipmap].m_Height);
		
		// Set priority!
		pRTexture->m_pD3DTexture->SetPriority(pTextureData->m_Header.GetTexturePriority()); 
	}

	// Setup the uv coordinate multipliers.
	uint32 realWidth,realHeight;
	AdjustAspectRatio(pTextureData->m_Header.m_BaseWidth, pTextureData->m_Header.m_BaseHeight, &realWidth, &realHeight);

	CalcRTextureMemoryUse(pTextureData, pRTexture, nMipsToCreate);

	g_pStruct->m_SystemTextureMemory += pRTexture->GetMemoryUse();

	// Associate the texture.
	pRTexture->m_pSharedTexture		= pSharedTexture;
	pSharedTexture->m_pRenderData	= pRTexture; 
	
	pRTexture->m_Link.m_pData = pRTexture;
	dl_Insert(&g_Textures, &pRTexture->m_Link);

	return pRTexture;
}

void CTextureManager::FreeAllTextures()
{
	LTLink* pListHead = &g_Textures;
	LTLink* pCur	  = pListHead->m_pNext;
	while (pCur != pListHead) 
	{
		LTLink* pNext = pCur->m_pNext;
		FreeTexture((RTexture*)pCur->m_pData);
		pCur = pNext; 
	}
	dl_TieOff(pListHead);
}

// Frees up the given texture.
void CTextureManager::FreeTexture(RTexture* pTexture)
{
	// Disassociates texture from its SharedTexture (if any)
	if (pTexture->m_pSharedTexture) 
	{
		pTexture->m_pSharedTexture->m_pRenderData = NULL;
		pTexture->m_pSharedTexture = NULL; 
	}

	// Update memory usage...
	g_pStruct->m_SystemTextureMemory -= pTexture->GetMemoryUse();

	if (pTexture->m_pD3DTexture) 
	{
		uint32 iRefCnt			= pTexture->m_pD3DTexture->Release(); 
		pTexture->m_pD3DTexture = NULL; 
	}

	// Remove it from the list and free it
	if (pTexture->m_Link.m_pData) 
		dl_Remove(&pTexture->m_Link);
	m_RTextureBank.Free(pTexture);
}

uint32 CTextureManager::GetPitch(D3DFORMAT Format, uint32 iWidth)
{
	uint32 iPitch;
	if (Format == D3DFMT_DXT1 || Format == D3DFMT_DXT3 || Format == D3DFMT_DXT5) {
		switch (Format) {
		case D3DFMT_DXT1 : iPitch = iWidth*2; break;
		case D3DFMT_DXT3 : iPitch = iWidth*4; break;
		case D3DFMT_DXT5 : iPitch = iWidth*4; break; } }
	else { 
		uint32 iBitCount, iAlphaMask, iRedMask, iGreenMask, iBlueMask;
		d3d_GetColorMasks(Format,iBitCount,iAlphaMask,iRedMask,iGreenMask,iBlueMask);
		iPitch = iWidth * iBitCount/8; }
	return iPitch;
}

// Returns true if the card natively supports creation of a texture with the specified format.
bool CTextureManager::IsS3TCFormatSupported(BPPIdent bpp)
{
	if      (bpp == BPP_S3TC_DXT1)	{ return m_bSupportsDXT1; }
	else if (bpp == BPP_S3TC_DXT3)	{ return m_bSupportsDXT3; }
	else if (bpp == BPP_S3TC_DXT5)	{ return m_bSupportsDXT5; }
	else							{ return false; }
}

D3DFORMAT CTextureManager::S3TCFormatConv(BPPIdent BPP)
{
	switch (BPP) {
	case BPP_S3TC_DXT1 : return D3DFMT_DXT1; break;
	case BPP_S3TC_DXT3 : return D3DFMT_DXT3; break;
	case BPP_S3TC_DXT5 : return D3DFMT_DXT5; break;
	default: assert(0 && "Invalid Format for S3TC"); }
	return (D3DFORMAT)NULL;
}

BPPIdent CTextureManager::S3TCFormatConv(D3DFORMAT Format)
{
	switch (Format) {
	case D3DFMT_DXT1 : return BPP_S3TC_DXT1; break;
	case D3DFMT_DXT3 : return BPP_S3TC_DXT3; break;
	case D3DFMT_DXT5 : return BPP_S3TC_DXT5; break;
	default: assert(0 && "Invalid Format for S3TC"); }
	return (BPPIdent)NULL;
}

bool CTextureManager::SelectTextureFormats()
{
	D3DAdapterInfo* pAdapterInfo = g_Device.GetAdapterInfo();
	D3DDeviceInfo*	pDeviceInfo  = g_Device.GetDeviceInfo();
	D3DModeInfo*	pDisplayMode = g_Device.GetModeInfo();

 	// Check for a valid FORMAT_32BIT...
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A8R8G8B8) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_32BIT,D3DFMT_A8R8G8B8); }
	else { AddDebugMessage(0, "FORMAT_32BIT texture format missing."); return false; }	// It's a sad thing, but not a failable offense...

	// Check for a valid FORMAT_FULLBRITE...
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A1R5G5B5) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_FULLBRITE,D3DFMT_A1R5G5B5); }
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A4R4G4B4) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_FULLBRITE,D3DFMT_A4R4G4B4); }
	else { AddDebugMessage(0, "FORMAT_FULLBRITE texture format missing."); return false; }

	// Check for a valid FORMAT_4444...
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A4R4G4B4) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_4444,D3DFMT_A4R4G4B4); }
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A1R5G5B5) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_4444,D3DFMT_A1R5G5B5); }
	else { AddDebugMessage(0, "FORMAT_4444 texture format missing."); return false; }

	// Check for a valid FORMAT_NORMAL...
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_R5G6B5) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_NORMAL,D3DFMT_R5G6B5); }
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A1R5G5B5) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_NORMAL,D3DFMT_A1R5G5B5); }
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A4R4G4B4) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_NORMAL,D3DFMT_A4R4G4B4); }
	else { AddDebugMessage(0, "FORMAT_NORMAL texture format missing."); return false; }

	// Check for a valid FORMAT_INTERFACE...
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A8R8G8B8) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_INTERFACE,D3DFMT_A8R8G8B8); }
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A1R5G5B5) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_INTERFACE,D3DFMT_A1R5G5B5); }
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A4R4G4B4) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_INTERFACE,D3DFMT_A4R4G4B4); }
	else { AddDebugMessage(0, "FORMAT_INTERFACE texture format missing."); return false; }

	// Check for a valid FORMAT_LIGHTMAP...
	if (g_CV_32BitLightmaps && PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_R8G8B8) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_LIGHTMAP,D3DFMT_R8G8B8); }
	else if (g_CV_32BitLightmaps && PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_X8R8G8B8) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_LIGHTMAP,D3DFMT_X8R8G8B8); }
	else if (g_CV_32BitLightmaps && PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A8R8G8B8) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_LIGHTMAP,D3DFMT_A8R8G8B8); }
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_X1R5G5B5) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_LIGHTMAP,D3DFMT_X1R5G5B5); }
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_A1R5G5B5) == D3D_OK) {
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_LIGHTMAP,D3DFMT_A1R5G5B5); }
	else { AddDebugMessage(0, "FORMAT_LIGHTMAP texture format missing."); return false; }

	// Check for a valid FORMAT_BUMPMAP...
	bool bFoundBumpMapFormat = true;
	
	//Note that the following formats are commented out currently. The reason for this is that the image
	//conversion code does not handle channels with over 8 bits correctly. This will have to be fixed,
	//and these uncommented again...
	/*
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_V16U16) == D3D_OK) 
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_BUMPMAP,D3DFMT_V16U16); 
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_W11V11U10) == D3D_OK) 
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_BUMPMAP,D3DFMT_W11V11U10);
	else*/ 
	
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_V8U8) == D3D_OK) 
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_BUMPMAP,D3DFMT_V8U8);
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_Q8W8V8U8) == D3D_OK) 
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_BUMPMAP,D3DFMT_Q8W8V8U8);
	else 
	{
		bFoundBumpMapFormat = false;
		AddDebugMessage(2, "FORMAT_BUMPMAP texture format missing."); // If we can't find one, no big deal...
	}

	// Check for a valid FORMAT_LUM_BUMPMAP...
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_V8U8) == D3D_OK) 
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_LUMBUMPMAP,D3DFMT_X8L8V8U8); 
	else if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_V16U16) == D3D_OK) 
		SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_LUMBUMPMAP,D3DFMT_L6V5U5);
	else 
	{
		//see if we found a bumpmap format
		if(bFoundBumpMapFormat)
		{
			//we did, so just fall back to that
			AddDebugMessage(2, "FORMAT_LUMBUMPMAP texture format missing, falling back to FORMAT_BUMPMAP");
			SetTextureFormatFromD3DFormat(CTextureManager::FORMAT_LUMBUMPMAP, GetTextureFormat(CTextureManager::FORMAT_BUMPMAP)->m_PF);
		}
		else
		{
			//no luck finding any format
			AddDebugMessage(2, "FORMAT_LUMBUMPMAP texture format missing");
		}
	}
	
	// Check for DXT Formats...
	m_bSupportsDXT1 = m_bSupportsDXT3 = m_bSupportsDXT5 = false;
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_DXT1) == D3D_OK) { m_bSupportsDXT1 = true; }
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_DXT3) == D3D_OK) { m_bSupportsDXT3 = true; }
	if (PDIRECT3D->CheckDeviceFormat(pAdapterInfo->iAdapterNum,pDeviceInfo->DeviceType,pDisplayMode->Format,NULL,D3DRTYPE_TEXTURE,D3DFMT_DXT5) == D3D_OK) { m_bSupportsDXT5 = true; }

	return true;
}

 
void CTextureManager::SetTextureFormatFromD3DFormat(ETEXTURE_FORMATS eFormat,D3DFORMAT iFormat)
{
	assert((int32)eFormat < (int32)NUM_TEXTUREFORMATS);
	TextureFormat* pFormat = &m_TextureFormats[eFormat]; 
	
	pFormat->m_PF = iFormat; 
	pFormat->m_bValid = true;

	//get the bit count of the format
	uint32 iBitCount, AMask, RMask, GMask, BMask;
	d3d_GetColorMasks(iFormat, iBitCount, AMask, RMask, GMask, BMask);

	pFormat->m_BytesPP = iBitCount / 8;
}

void CTextureManager::ListTextureFormats()
{
	d3d_PrintFormatInfo("[FULLBRITE] - ",	m_TextureFormats[FORMAT_FULLBRITE].m_PF);
	d3d_PrintFormatInfo("[4444] - ",		m_TextureFormats[FORMAT_4444].m_PF);
	d3d_PrintFormatInfo("[NORMAL] - ",		m_TextureFormats[FORMAT_NORMAL].m_PF);
	d3d_PrintFormatInfo("[INTERFACE] - ",	m_TextureFormats[FORMAT_INTERFACE].m_PF);
	d3d_PrintFormatInfo("[LIGHTMAP] - ",	m_TextureFormats[FORMAT_LIGHTMAP].m_PF);
	d3d_PrintFormatInfo("[BUMPMAP] - ",		m_TextureFormats[FORMAT_BUMPMAP].m_PF);
	d3d_PrintFormatInfo("[LUMBUMPMAP] - ",	m_TextureFormats[FORMAT_LUMBUMPMAP].m_PF);
}

//-----------------------------------------------------------------------------------------------------------
//
// Setting Texture functions
//
// Eventually these should be gathered into a class so we don't have ugly global data, but for now...
//
//-----------------------------------------------------------------------------------------------------------

//Structure holding data that should be cached for each texture stage
class CCachedStageInfo
{
public:
	CCachedStageInfo() :
		m_pTexture(NULL),
		m_fMipMapBias(0.0f)
	{
	}

	//the currently installed texture
	LPDIRECT3DBASETEXTURE9	m_pTexture;

	//the current mip map offset
	float					m_fMipMapBias;
};

//the list of information for each stage
CCachedStageInfo	g_CachedStageInfo[MAX_TEXTURESTAGES];

//called to handle tracking of memory from a texture
static void d3d_TrackTextureMemory(CTrackedTextureMem& TextureMem, ERendererFrameStats eMemType)
{
#ifndef _FINAL
	if (TextureMem.m_nTextureFrameCode != g_CurFrameCode) 
	{
		IncFrameStat(eMemType, TextureMem.m_nMemory);
		IncFrameStat(eFS_TotalUncompressedTexMemory, TextureMem.m_nUncompressedMemory);

		TextureMem.m_nTextureFrameCode = g_CurFrameCode; 
	}
#endif
}

//called to do the actual texture setting
static void d3d_InternalSetCurrentTextureDirect(LPDIRECT3DBASETEXTURE9 pTexture, uint32 nStage, float fMipMapBias, uint32 nAlphaRef)
{
	//make sure that the stage is valid
	assert(nStage < MAX_TEXTURESTAGES);

	CCachedStageInfo& StageInfo = g_CachedStageInfo[nStage];

	//clear out any states if we need to
	if(pTexture)
	{
		//offset the mipmap bias by the global value
		fMipMapBias += g_CV_MipMapBias.m_Val;

		//see if we need to modify the mipmap
		if(fMipMapBias != StageInfo.m_fMipMapBias)
		{
			PD3DDEVICE->SetSamplerState(nStage, D3DSAMP_MIPMAPLODBIAS, *((DWORD*)&fMipMapBias));
			StageInfo.m_fMipMapBias = fMipMapBias;
		}

		PD3DDEVICE->SetRenderState(D3DRS_ALPHAREF, nAlphaRef);
	}

	//finally setup the texture
	if(pTexture != StageInfo.m_pTexture)
	{
		//keep track of the number of texture changes
		IncFrameStat(eFS_TextureChanges, 1); 
		PD3DDEVICE->SetTexture(nStage, pTexture);
		StageInfo.m_pTexture = pTexture;
	}
	else
	{
		IncFrameStat(eFS_TextureChangeSaves, 1);
	}
}

// Call this to have it set the texture.
bool d3d_SetTexture(SharedTexture* pTexture, uint32 iStage, ERendererFrameStats eMemType) 
{
	//see if we actually have a valid texture
	if (pTexture) 
	{
		//we do, make sure that the render texture is correct
		RTexture* pRTexture = (RTexture*)pTexture->m_pRenderData;
	
		if(!pRTexture) 
		{
			//It is currently not setup, lets try and create the texture
			pRTexture = d3d_CreateAndLoadTexture(pTexture);
			if (!pRTexture)
			{
				//failed to create it, just clear out the stage
				d3d_InternalSetCurrentTextureDirect(NULL, iStage, 0.0f, ALPHAREF_NONE);
				return false; 
			}
		}

		//the render texture is valid, set it up
		
		// Track texture memory usage, we only need this during development though
		d3d_TrackTextureMemory(pRTexture->m_TextureMem, eMemType);

		//now actually setup the texture
		d3d_InternalSetCurrentTextureDirect(pRTexture->m_pD3DTexture, iStage, pRTexture->m_fMipMapBias, pRTexture->m_AlphaRef);
	}	
	else 
	{
		//we didn't have a texture, just clear it out
		d3d_InternalSetCurrentTextureDirect(NULL, iStage, 0.0f, ALPHAREF_NONE);
	}

	return true; 
}

// Call to set a D3D texture 
bool d3d_SetTextureDirect(LPDIRECT3DBASETEXTURE9 pTexture, uint32 nStage)
{
	d3d_InternalSetCurrentTextureDirect(pTexture, nStage, 0.0f, ALPHAREF_NONE);
	return true;
}

bool d3d_DisableTexture(uint32 nStage)
{
	d3d_InternalSetCurrentTextureDirect(NULL, nStage, 0.0f, ALPHAREF_NONE);
	return true;
}

bool d3d_SetTextureDirect(LPDIRECT3DBASETEXTURE9 pTexture, uint32 nStage, CTrackedTextureMem& TextureMem, ERendererFrameStats eMemType)
{
	// Track texture memory usage
	d3d_TrackTextureMemory(TextureMem, eMemType);

	//now setup the texture
	d3d_InternalSetCurrentTextureDirect(pTexture, nStage, 0.0f, ALPHAREF_NONE);
	return true;
}

