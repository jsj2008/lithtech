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
        //AfxMessageBox(IDS_ERROR_COULDNOTLOADFILE);
#ifdef _DEBUG
		AfxMessageBox("Could Not Load DTX");
#endif
		return false;
	}

	// Load the texture from file.
	DRESULT dResult = dtx_Create(pStream, &m_pTexture, TRUE);
	pStream->Release();
	if (dResult != DE_OK)
	{
		//CString str;
		//str.FormatMessage(IDS_ERROR_ERRORLOADINGDTX, pFilename);
		//AfxMessageBox(str);
		
#ifdef _DEBUG
		AfxMessageBox("Error Loading DTX");
#endif
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
