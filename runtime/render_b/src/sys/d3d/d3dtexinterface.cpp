
#include "bdefs.h"
#include "ilttexinterface.h"
#include "d3dtexinterface.h"
#include "Render.h"
#include "dtxmgr.h"
#include "colorops.h"
#include "clientmgr.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

// interface database
define_interface(CSysTexInterface, ILTTexInterface); 


extern CClientMgr*			g_pClientMgr;

//---------------------------------------------------------------------
// Utility functions
//---------------------------------------------------------------------

static bool ConvertFormatToType(PFormat& Format, ETextureType &eTextureType) 
{
	switch (Format.GetType()) 
	{
	case BPP_S3TC_DXT1 : eTextureType = TEXTURETYPE_DXT1; break;
	case BPP_S3TC_DXT3 : eTextureType = TEXTURETYPE_DXT3; break;
	case BPP_S3TC_DXT5 : eTextureType = TEXTURETYPE_DXT5; break;
	case BPP_32		   : eTextureType = TEXTURETYPE_ARGB8888; break;
	case BPP_16		   : 
		if (Format.m_Masks[CP_ALPHA] == 0x00000000 && Format.m_Masks[CP_RED]  == 0x0000F800 && Format.m_Masks[CP_GREEN] == 0x000007E0 && Format.m_Masks[CP_BLUE] == 0x0000001F) eTextureType = TEXTURETYPE_RGB565;
		else if (Format.m_Masks[CP_ALPHA] == 0x00008000 && Format.m_Masks[CP_RED]  == 0x00007C00 && Format.m_Masks[CP_GREEN] == 0x000003E0 && Format.m_Masks[CP_BLUE] == 0x0000001F) eTextureType = TEXTURETYPE_ARGB1555;
		else if (Format.m_Masks[CP_ALPHA] == 0x0000F000 && Format.m_Masks[CP_RED]  == 0x00000F00 && Format.m_Masks[CP_GREEN] == 0x000000F0 && Format.m_Masks[CP_BLUE] == 0x0000000F) eTextureType = TEXTURETYPE_ARGB4444;
		return LT_ERROR; break; 
	default			   : return false; 
	}

	return true;
}

// Find texture in memory if it exists and return its handle.
// If it does not exist, this function should return an error.
LTRESULT CSysTexInterface::FindTextureFromName(HTEXTURE &hTexture, const char *pFilename) 
{
	FileRef ref;
	hTexture				= NULL;
	ref.m_FileType			= TYPECODE_TEXTURE;
	ref.m_pFilename			= pFilename;

	FileIdentifier* pIdent	= client_file_mgr->GetFileIdentifier(&ref, TYPECODE_TEXTURE);
	if (pIdent == LTNULL) 
	{ 
		return LT_ERROR; 
	}

	if (pIdent->m_pData) 
	{
		(SharedTexture*)hTexture = (SharedTexture*)pIdent->m_pData; 
		return LT_OK; 
	}

	return LT_ERROR;
}

// Create texture from a dtx file on disk.
LTRESULT CSysTexInterface::CreateTextureFromName(HTEXTURE &hTexture, const char *pFilename)
{
	FileRef ref; 
	LTRESULT dResult					 = LT_ERROR;
	hTexture							 = NULL;
	ref.m_FileType						 = TYPECODE_TEXTURE;
	ref.m_pFilename						 = pFilename;

	FileIdentifier* pIdent = client_file_mgr->GetFileIdentifier(&ref, TYPECODE_TEXTURE);
	
	if (pIdent) 
	{
		//see if the texture is already loaded and bound to this file identifier
		if(pIdent->m_pData)
		{
			//it is already bound
			hTexture = (HTEXTURE)pIdent->m_pData;
			hTexture->SetRefCount(hTexture->GetRefCount() + 1);
		}
		else
		{
			//it isn't already loaded, so create one and load it
		hTexture = g_pClientMgr->m_SharedTextureBank.Allocate();
		
		if (hTexture) 
		{
			memset(hTexture, 0, sizeof(*hTexture));
			hTexture->m_pFile = pIdent;
			pIdent->m_pData	  = hTexture;

			dResult	 = r_LoadSystemTexture(hTexture);

			if (dResult == LT_OK) 
			{
				r_BindTexture(hTexture, false);
				hTexture->SetRefCount(hTexture->GetRefCount() + 1);
				dl_AddHead(&g_pClientMgr->m_SharedTextures, &hTexture->m_Link, hTexture); 
			} 
			else 
			{
				g_pClientMgr->m_SharedTextureBank.Free(hTexture); 
			} 
		} 
	}
	}

	return dResult;
}

LTRESULT CSysTexInterface::CreateTextureFromData(HTEXTURE &hTexture, ETextureType eTextureType, uint32 TextureFlags, 
	uint8 *pData, uint32 nWidth, uint32 nHeight, uint32 nAutoGenMipMaps) 
{
	if (nAutoGenMipMaps == 0) 
	{
		// We need to autogen them all the way down, so set nAutoGenMipMaps to the number we're going to want.
 		nAutoGenMipMaps = 1; uint32 iTmp = MAX(nWidth,nHeight); 
		while (iTmp > 2 && nAutoGenMipMaps < 8) 
		{ 
			iTmp /= 2; ++nAutoGenMipMaps; 
		} 
	}
 
	// Make our SysMem RGBA copy...
	PFormat TheFormat;
	switch (eTextureType) 
	{
	case TEXTURETYPE_ARGB8888 : TheFormat.Init(BPP_32,0xFF000000,0x00FF0000,0x0000FF00,0x000000FF); break;
	case TEXTURETYPE_ARGB4444 : TheFormat.Init(BPP_16,0xF000,0x0F00,0x00F0,0x000F); break;
	case TEXTURETYPE_ARGB1555 : TheFormat.Init(BPP_16,0x8000,RGB555_RMASK,RGB555_GMASK,RGB555_BMASK); break;
	case TEXTURETYPE_RGB565	  : TheFormat.Init(BPP_16,0x0000,RGB565_RMASK,RGB565_GMASK,RGB565_BMASK); break;
	case TEXTURETYPE_DXT1	  : TheFormat.Init(BPP_S3TC_DXT1,0,0,0,0); break;
	case TEXTURETYPE_DXT3	  : TheFormat.Init(BPP_S3TC_DXT3,0,0,0,0); break;
	case TEXTURETYPE_DXT5	  : TheFormat.Init(BPP_S3TC_DXT5,0,0,0,0); break;
	default : assert(0 && "Unsupported Format"); return LT_ERROR; 
	}

	//allocate our shared texture
	hTexture = g_pClientMgr->m_SharedTextureBank.Allocate();

	//check the allocation
	if(!hTexture)
		return LT_ERROR;

	//clear the texture out, and add it to the global list of shared textures
	memset(hTexture, 0, sizeof(*hTexture)); 
	dl_AddHead(&g_pClientMgr->m_SharedTextures, &hTexture->m_Link, hTexture);

	//there is no file reference for this shared texture since it is entirely user created
	hTexture->m_pFile = NULL;

	//allocate and initialize our actual texture data
	hTexture->SetTextureInfo(nWidth, nHeight, TheFormat);

	hTexture->m_pEngineData				= dtx_Alloc(TheFormat.GetType(), nWidth, nHeight, nAutoGenMipMaps, LTNULL, LTNULL);

	TextureData* pTextureData			= (TextureData*)hTexture->m_pEngineData;
	pTextureData->m_PFormat				= TheFormat;
	pTextureData->m_Flags				= TextureFlags;
	pTextureData->m_Header.m_Extra[2]	= TheFormat.GetType();
	pTextureData->m_Header.m_IFlags		= TextureFlags;
	pTextureData->m_pSharedTexture		= hTexture;

	// Add the new texture to the list of texture data
	dl_AddHead(&g_SysCache.m_List, &pTextureData->m_Link, pTextureData);
	g_SysCache.m_CurMem += pTextureData->m_AllocSize;

	if (((SharedTexture*)hTexture)->m_pEngineData) 
	{
		for (uint i = 0; i < nAutoGenMipMaps; ++i) 
		{
			if (!r_GetRenderStruct()->ConvertTexDataToDD(pData,&TheFormat,nWidth,nHeight,pTextureData->m_Mips[i].m_Data,&TheFormat,pTextureData->m_Header.GetBPPIdent(), pTextureData->m_Header.m_IFlags, pTextureData->m_Mips[i].m_Width,pTextureData->m_Mips[i].m_Height)) 
			{
				r_UnloadSystemTexture(pTextureData); hTexture = NULL;
				return LT_ERROR; 
			} 
		}

		hTexture->SetRefCount(hTexture->GetRefCount() + 1);
		if (!hTexture->m_pRenderData) 
		{
			// Bind it to the renderer.
			r_BindTexture(hTexture, true); 
		}
		return LT_OK; 
	}
	else 
	{
		r_UnloadSystemTexture((TextureData*)((SharedTexture*)hTexture)->m_pEngineData); hTexture = NULL;
		return LT_ERROR; 
	} 

	return LT_ERROR;
}

LTRESULT CSysTexInterface::GetTextureData(const HTEXTURE hTexture, const uint8* &pData, uint32 &nPitch, uint32& nWidth, uint32& nHeight, ETextureType& eType) 
{
	SharedTexture* pTexture = (SharedTexture*)hTexture;
 	if (!pTexture)						 
	{
		return LT_ERROR; 
	}

 	TextureData* pTextureData = r_GetTextureData(pTexture);
 	if (!pTextureData)					 
	{ 
		return LT_ERROR; 
	}

	if(!ConvertFormatToType(pTextureData->m_PFormat, eType))
	{
		return LT_ERROR;
	}

	nWidth  = pTextureData->m_Mips[0].m_Width;
	nHeight = pTextureData->m_Mips[0].m_Height;
	pData   = pTextureData->m_Mips[0].m_Data;
	nPitch  = pTextureData->m_Mips[0].m_Width * pTextureData->m_PFormat.GetBytesPerPixel();

	return LT_OK;
}

// Let the engine know that we are finished modifing the texture
LTRESULT CSysTexInterface::FlushTextureData (const HTEXTURE hTexture, ETextureMod eChanged, uint32 nMipMap) 
{
	r_BindTexture(hTexture, true);		 // Bind it to the renderer.
	return (LT_OK);
}

LTRESULT CSysTexInterface::GetTextureType(const HTEXTURE hTexture, ETextureType &eTextureType)
{
	SharedTexture* pTexture = (SharedTexture*)hTexture;
 	if (!pTexture)						 
	{
		return LT_ERROR; 
	}

	uint32 nWidth, nHeight;
	PFormat Format;
	if(!r_GetTextureInfo(hTexture, nWidth, nHeight, Format))
	{
		return LT_ERROR;
	}

	if(!ConvertFormatToType(Format, eTextureType))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

// Get information about the texture
LTRESULT CSysTexInterface::GetTextureDims(const HTEXTURE hTexture, uint32 &nWidth, uint32 &nHeight) 
{
	SharedTexture* pTexture = (SharedTexture*)hTexture;
	if (!pTexture) 
	{ 
		return LT_ERROR; 
	}

	PFormat Format;
	if(!r_GetTextureInfo(hTexture, nWidth, nHeight, Format))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

bool CSysTexInterface::ReleaseTextureHandle(const HTEXTURE hTexture)
{
	FN_NAME(CSysTexInterface::ReleaseTextureHandle);
	SharedTexture* pTexture = hTexture;
	if (!pTexture) 
		return false;

	if (!r_GetRenderStruct()->m_bInitted) 
		return false;

	pTexture->SetRefCount(pTexture->GetRefCount() - 1);

	// [dlk] 12/21/2005 - uncommented this block, as it caused a 64k memory leak on the console font.
	// each time "restartrender" was called. (change carried over from Jupiter_Xbox)
	//
	// (OLD comment) KLS - The texture will be cleaned up on level switch, don't delete it now
	// since both the engine and game code may be reference this texture.
	
	if(pTexture->GetRefCount() == 0) 
	{
		g_pClientMgr->FreeSharedTexture(hTexture); 
	}

	return true;
}

uint32 CSysTexInterface::AddRefTextureHandle(const HTEXTURE hTexture)
{
	FN_NAME(LTTexMod::ReleaseTextureHandle);
	SharedTexture* pTexture = hTexture;
	if (!pTexture) return false;

	uint32 nNewRefCount = pTexture->GetRefCount() + 1;
	pTexture->SetRefCount(nNewRefCount);

	return nNewRefCount;
}
