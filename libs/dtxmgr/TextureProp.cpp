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
#include "bdefs.h"
#include "dtxmgr_lib.h"

#include "resource.h"

#include "sysstreamsim.h"
#include "s3tc_compress.h"

TextureProp::~TextureProp()
{
	Term();
}

bool TextureProp::New(unsigned short nWidth, unsigned short nHeight, unsigned short nMips)
{
	if (m_Initted)
	{
		Term();
	}

	uint32 allocSize = 0;
	uint32 textureDataSize = 0;

	m_pTexture = dtx_Alloc(BPP_32, nWidth, nHeight, nMips, &allocSize, &textureDataSize);

	if(m_pTexture)
	{
		m_Initted = true;
		return true;
	}

	return false;
}

bool TextureProp::Resize(unsigned short nWidth, unsigned short nHeight)
{
	if(m_pTexture)
	{
		LTRESULT  res = dtx_Resize( m_pTexture, m_pTexture->m_Header.GetBPPIdent(), nWidth, nHeight);
		if(res == LT_OK)
		{
			return true;
		}
	}

	return false;
}

// Pass in NULL  for the file name if using a file handle
bool TextureProp::Init(const char *pFilename, int nFileHandle)
{
	if (m_Initted)
	{
		Term();
		return false;
	}

	DStream *pStream = NULL;
	if(pFilename)
	{
		// Open the file.
		pStream = streamsim_Open(pFilename, "rb");
		if (!pStream)
		{
			return false;
		}
	}

	// Load the texture from file.
	DRESULT dResult = dtx_Create(pStream, nFileHandle, &m_pTexture, FALSE);

	if(pStream)
	{
		pStream->Release();
		pStream = NULL;
	}

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
	m_BPPIdent 				= (int)m_pTexture->m_Header.GetBPPIdent();
	m_NonS3TCMipmapOffset 	= m_pTexture->m_Header.GetNonS3TCMipmapOffset();
	m_UIMipmapOffset 		= m_pTexture->m_Header.GetUIMipmapOffset();
	LTStrCpy(m_CommandString, m_pTexture->m_Header.m_CommandString, sizeof(m_CommandString));
	m_TexturePriority 		= m_pTexture->m_Header.GetTexturePriority();
	m_DetailTextureScale 	= m_pTexture->m_Header.GetDetailTextureScale();
	m_DetailTextureAngle 	= m_pTexture->m_Header.GetDetailTextureAngle();

	m_Initted = true;

	// Are we compressed?
	BPPIdent bppIdent = m_pTexture->m_Header.GetBPPIdent();
	if (IsBPPCompressed(bppIdent))
	{
		if(!ConvertTextureData(m_pTexture, BPP_32))
		{
			return false;
		}
	}
	else
	{
		//copy the pixels into a buffer
	}

	return true;
}

bool TextureProp::Save(const char *pFilename, int nFileHandle)
{
	if (!m_Initted)
	{
		Term();
		return false;
	}

	if(!m_pTexture)
	{
		Term();
		return false;
	}

	DStream *pStream = NULL;
	if(pFilename)
	{
		// Open the file.
		pStream = streamsim_Open(pFilename, "rb");
		if (!pStream)
		{
			return false;
		}
	}

	LTRESULT res = dtx_Save(m_pTexture, pStream, nFileHandle);

	if(LT_OK != res)
	{
		return false;
	}

	return true;
}

void TextureProp::GetDims(uint16 &nWidth, uint16 &nHeight)
{
	if(NULL == m_pTexture)
	{
		nWidth = 0;
		nHeight = 0;
	}

	nWidth = m_pTexture->m_Mips[0].m_Width;
	nHeight = m_pTexture->m_Mips[0].m_Height;
}

bool TextureProp::GetTextureRGBAData(void* pBuffer, int nMaxBufferSize)
{
	if(NULL == m_pTexture)
	{
		return false;
	}

	if(NULL == pBuffer)
	{
		return false;
	}

	//Get the true buffer size
	int nMipBufferSize = m_pTexture->m_Mips[0].m_Width * m_pTexture->m_Mips[0].m_Height * 4; 

	// Use the smaller of the two values as out max buffer size. 
	int nUsableBufferSize = (nMaxBufferSize < nMipBufferSize) ? nMaxBufferSize : nMipBufferSize;

	
	//Get the RGB out of the mip data
	uint8* ppBuffer = (uint8*)pBuffer;

	// Get the size of the texture.
	CMoDWordArray rgbData;
	if (!rgbData.SetSize(m_pTexture->m_Header.m_BaseWidth * m_pTexture->m_Header.m_BaseHeight))
	{
		return false;
	}

	FMConvertRequest cRequest;

	TextureMipData *pMip = &m_pTexture->m_Mips[0];

	// Get us into PValue format.
	dtx_SetupDTXFormat(m_pTexture, cRequest.m_pSrcFormat);
	cRequest.m_pSrc 		= pMip->m_Data;
	cRequest.m_SrcPitch 	= pMip->m_Pitch;
	cRequest.m_pDestFormat->InitPValueFormat();
	cRequest.m_pDest 		= (BYTE*)rgbData.GetArray();
	cRequest.m_DestPitch 	= pMip->m_Width * sizeof(DWORD);
	cRequest.m_Width 		= pMip->m_Width;
	cRequest.m_Height 		= pMip->m_Height;

	FormatMgr formatMgr;
	if (formatMgr.ConvertPixels(&cRequest) != LT_OK)
	{
		return false;
	}

	DWORD *pInLine = rgbData.GetArray();
	DWORD a, r, g, b;

	unsigned iBuf = 0;
	for (DWORD y = 0; y < pMip->m_Height; ++y)
	{
		for (DWORD x = 0; x < pMip->m_Width; ++x)
		{
			PValue_Get(pInLine[x], a, r, g, b);

			//R
			ppBuffer[iBuf]   = (uint8)r;
			//G
			ppBuffer[iBuf+1] = (uint8)g;
			//B
			ppBuffer[iBuf+2] = (uint8)b;
			//A
			ppBuffer[iBuf+3] = (uint8)a;

			iBuf += 4;
		}

		pInLine += pMip->m_Width;
	}
	
	return true;
}

void* TextureProp::GetDataBuffer()
{
	if(!m_pTexture)
	{
		return NULL;
	}

	return (void*)m_pTexture->m_pDataBuffer;
}

bool TextureProp::ConvertToFormat(int nBPPIdent)
{
	if(!ConvertTextureData(m_pTexture, (BPPIdent)nBPPIdent))
	{
		return false;
	}

	return true;
}

void TextureProp::RebuildMipMaps()
{
	//Make sure we rebuild the mip maps
	dtx_BuildMipmaps(m_pTexture);
}

void TextureProp::FillMembers()
{
	//fill the member vars from the header
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
	m_BPPIdent 				= (int)m_pTexture->m_Header.GetBPPIdent();
	m_NonS3TCMipmapOffset 	= m_pTexture->m_Header.GetNonS3TCMipmapOffset();
	m_UIMipmapOffset 		= m_pTexture->m_Header.GetUIMipmapOffset();
	LTStrCpy(m_CommandString, m_pTexture->m_Header.m_CommandString, sizeof(m_CommandString));
	m_TexturePriority 		= m_pTexture->m_Header.GetTexturePriority();
	m_DetailTextureScale 	= m_pTexture->m_Header.GetDetailTextureScale();
	m_DetailTextureAngle 	= m_pTexture->m_Header.GetDetailTextureAngle();

}

void TextureProp::StoreMembers()
{
	//save the member vars off to the header
	m_pTexture->m_Header.m_Extra[1] = m_nMipmaps;
	m_pTexture->m_Header.m_nMipmaps = m_nMipmaps;

	if(m_bFullBrights) {m_pTexture->m_Header.m_IFlags |= DTX_FULLBRITE;}
	if(m_b32BitSysCopy) {m_pTexture->m_Header.m_IFlags |= DTX_32BITSYSCOPY;}
	if(m_bPrefer4444) {m_pTexture->m_Header.m_IFlags |= DTX_PREFER4444;}
	if(m_bPrefer5551) {m_pTexture->m_Header.m_IFlags |= DTX_PREFER5551;}
	if(m_bPrefer16Bit) {m_pTexture->m_Header.m_IFlags |= DTX_PREFER16BIT;}
	if(m_bNoSysCache) {m_pTexture->m_Header.m_IFlags |= DTX_NOSYSCACHE;}
	m_pTexture->m_Header.m_UserFlags = m_TextureFlags;
	m_pTexture->m_Header.m_Extra[0] = m_TextureGroup;

	//don't do this here! Use m_BPPIdent to convert to this format later
	//m_pTexture->m_Header.SetBPPIdent((BPPIdent)m_BPPIdent);

	m_pTexture->m_Header.SetNonS3TCMipmapOffset(m_NonS3TCMipmapOffset);
	m_pTexture->m_Header.SetUIMipmapOffset(m_UIMipmapOffset);
	LTStrCpy(m_pTexture->m_Header.m_CommandString, m_CommandString, sizeof(m_CommandString));

	m_pTexture->m_Header.SetTexturePriority(m_TexturePriority);
	m_pTexture->m_Header.SetDetailTextureScale(m_DetailTextureScale);
	m_pTexture->m_Header.SetDetailTextureAngle(m_DetailTextureAngle);
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
