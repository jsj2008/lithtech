//------------------------------------------------------------------
//
//  FILE      : TextureProp.cpp
//
//  PURPOSE   :	Texture properties structure
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "TextureProp.h"
#include "resource.h"

#include "sysstreamsim.h"
#include "s3tc_compress.h"


TextureProp::~TextureProp()
{
	Term();
}



bool TextureProp::Init(const char *pFilename)
{
	if (m_Initted)
	{
		Term();
	}

	// Open the file.
	DStream *pStream = streamsim_Open(pFilename, "rb");
	if (!pStream)
	{
		return false;
	}

	// Load the texture from file.
	DRESULT dResult = dtx_Create(pStream, &m_pTexture, TRUE);
	pStream->Release();
	if (dResult != DE_OK)
	{
		return false;
	}

	if (m_pTexture->m_Header.m_Extra[1] == 0)
	{
		m_nMipmaps 			= 4;
	}
	else
	{
		m_nMipmaps 			= m_pTexture->m_Header.m_Extra[1];
	}

	if (m_pTexture->m_Header.m_Extra[2] & 0x80)
	{
		m_AlphaCutoff 		= m_pTexture->m_Header.m_Extra[2] & ~0x80;
	}
	else
	{
		m_AlphaCutoff 		= 8;
	}

	m_AverageAlpha 			= m_pTexture->m_Header.m_Extra[3];

	m_bFullBrights 			= !!(m_pTexture->m_Header.m_IFlags&DTX_FULLBRITE);
	m_b32BitSysCopy 		= !!(m_pTexture->m_Header.m_IFlags&DTX_32BITSYSCOPY);
	m_bPrefer4444 			= !!(m_pTexture->m_Header.m_IFlags & DTX_PREFER4444);
	m_bPrefer5551 			= !!(m_pTexture->m_Header.m_IFlags & DTX_PREFER5551);
	m_bPrefer16Bit 			= !!(m_pTexture->m_Header.m_IFlags & DTX_PREFER16BIT);
	m_bNoSysCache 			= !!(m_pTexture->m_Header.m_IFlags & DTX_NOSYSCACHE);
	m_TextureFlags 			= m_pTexture->m_Header.m_UserFlags;
	m_TextureGroup 			= m_pTexture->m_Header.m_Extra[0];
	m_BPPIdent 				= m_pTexture->m_Header.GetBPPIdent();
	m_NonS3TCMipmapOffset 	= m_pTexture->m_Header.GetNonS3TCMipmapOffset();
	m_UIMipmapOffset 		= m_pTexture->m_Header.GetUIMipmapOffset();
	LTStrCpy(m_CommandString, m_pTexture->m_Header.m_CommandString, sizeof(m_CommandString));
	m_TexturePriority 		= m_pTexture->m_Header.GetTexturePriority();
	m_DetailTextureScale 	= m_pTexture->m_Header.GetDetailTextureScale();
	m_DetailTextureAngle 	= m_pTexture->m_Header.GetDetailTextureAngle();

	m_Initted = true;

	return true;
}

bool TextureProp::Save(const char *pFilename)
{
	if (!m_Initted)
	{
		return false;
	}

	// Open the file.
	DStream *pStream = streamsim_Open(pFilename, "wb");
	if (!pStream)
	{
		return false;
	}

	// Load the texture from file.
	DRESULT dResult = dtx_Save(m_pTexture, pStream);
	pStream->Release();
	if (dResult != DE_OK)
	{
		return false;
	}
	return true;
}


void TextureProp::Term()
{
	if (!m_Initted)
	{
		return;
	}

	// Delete the texture.
	if (NULL != m_pTexture)
	{
		dtx_Destroy(m_pTexture);
		m_pTexture = NULL;
	}

	m_Initted = false;
}

#include <stdio.h>
bool TextureProp::Optimize()
{
	if(NULL == m_pTexture)
	{
		return false;
	}

	// Don't touch it if we're dealing with a non-standard texture
	if(m_pTexture->m_Header.m_IFlags & DTX_CUBEMAP)
	{
		return false;
	}
	else if(m_pTexture->m_Header.m_IFlags & DTX_BUMPMAP)
	{
		return false;
	}
	else if(m_pTexture->m_Header.m_IFlags & DTX_LUMBUMPMAP)
	{
		return false;
	}
	//else
	//It's a normal texture.. so let's process it.

	//Is it already compressed?
	if(IsBPPCompressed(m_pTexture->m_Header.GetBPPIdent()))
	{
		return false;
	}

	// first convert it to 32 bit so we can check out its alpha channel info
	if(!ConvertTextureData(m_pTexture, BPP_32))
	{
		return false;
	}

	bool bUsesAlpha = false;

	TextureMipData* pMipData = &m_pTexture->m_Mips[0];
	int nDataSize = pMipData->m_Width * pMipData->m_Height * 4;

	uint8 *pRawData = pMipData->m_Data;

	for(int i = 0; i < nDataSize; i+=4)
	{
		if(pRawData[i+3] != 0xFF)
		{
			bUsesAlpha = true;
			break;
		}
	}

	//if the alpha is actually used then DXT5 will work
	if(bUsesAlpha)
	{
		if(!ConvertTextureData(m_pTexture, BPP_S3TC_DXT5))
		{
			return false;
		}
		else
		{
			printf("DXT5 Compression Used...\n");
		}
	}
	else
	{
		//if the alpha is all white then DXT1 will work
		if(!ConvertTextureData(m_pTexture, BPP_S3TC_DXT1))
		{
			return false;
		}
		else
		{
			printf("DXT1 Compression Used...\n");
		}
	}

	return true;
}

bool TextureProp::OptimizeFor2D()
{
	if(!m_pTexture)
	{
		return false;
	}

	m_pTexture->m_Header.m_nMipmaps = 1;
	m_pTexture->m_Header.m_Extra[1] = 1;
	m_nMipmaps = 1;

	return true;
}
