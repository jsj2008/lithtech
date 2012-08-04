//------------------------------------------------------------------
//
//  FILE      : DtxViewView.cpp
//
//  PURPOSE   :	implementation of the CDtxViewDoc class
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "DtxView.h"

#include "DtxViewDoc.h"
#include "s3tc_compress.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif



//-------------------------------------------------------------------------------------------------
// DtxImageBuffer
//-------------------------------------------------------------------------------------------------


void DtxImageBuffer::Init(unsigned Width, unsigned Height)
{
	Term();

	const unsigned BufSize = Width * Height;
	m_Image = new uint32[BufSize];
	m_Alpha = new uint32[BufSize];

	m_Width = Width;
	m_Height = Height;
	m_Bytes = Width * Height * sizeof(uint32);
}



void DtxImageBuffer::Term()
{
	if (NULL != m_Image)
	{
		delete [] m_Image;
		m_Image = NULL;
	}

	if (NULL != m_Alpha)
	{
		delete [] m_Alpha;
		m_Alpha = NULL;
	}
}



//-------------------------------------------------------------------------------------------------
// CDtxViewDoc
//-------------------------------------------------------------------------------------------------


IMPLEMENT_DYNCREATE(CDtxViewDoc, CDocument)

BEGIN_MESSAGE_MAP(CDtxViewDoc, CDocument)
END_MESSAGE_MAP()


CDtxViewDoc::CDtxViewDoc()
	: m_ViewAlphaChannel(true),
	  m_ViewMipmapLevels(true),
	  m_DocTotalWidth(0),
	  m_DocTotalHeight(0),
	  m_DocImageWidth(0),
	  m_DocImageHeight(0)
{
}



CDtxViewDoc::~CDtxViewDoc()
{
	DestroyBuffers();
}



BOOL CDtxViewDoc::OnNewDocument()
{
	return TRUE;
}



BOOL CDtxViewDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	// Try to load the texture file.
	if (m_TextureProp.Init(lpszPathName))
	{
		if (CreateBuffers())
		{
			return TRUE;
		}
	}

	return FALSE;
}



BOOL CDtxViewDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	return FALSE;
}



void CDtxViewDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
    CDocument::SetPathName(lpszPathName, bAddToMRU);

	SetTitle(lpszPathName);
}



#ifdef _DEBUG
void CDtxViewDoc::AssertValid() const
{
	CDocument::AssertValid();
}



void CDtxViewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG



bool CDtxViewDoc::CreateBuffers()
{
	// Delete the old texture pixel data.
	DestroyBuffers();

	// Validate the texture data.
	TextureData *pTexture = m_TextureProp.m_pTexture;
	if (NULL == pTexture)
	{
		return false;
	}

	// Get the size of the texture.
	CMoDWordArray rgbData;
	if (!rgbData.SetSize(pTexture->m_Header.m_BaseWidth * pTexture->m_Header.m_BaseHeight))
	{
		return false;
	}

	FMConvertRequest cRequest;

	for (DWORD iMipmap = 0; iMipmap < pTexture->m_Header.m_nMipmaps; ++iMipmap)
	{
		TextureMipData *pMip = &pTexture->m_Mips[iMipmap];

		// Get us into PValue format.
		dtx_SetupDTXFormat(pTexture, cRequest.m_pSrcFormat);
		cRequest.m_pSrc 		= pMip->m_Data;
		cRequest.m_SrcPitch 	= pMip->m_Pitch;
		cRequest.m_pDestFormat->InitPValueFormat();
		cRequest.m_pDest 		= (BYTE*)rgbData.GetArray();
		cRequest.m_DestPitch 	= pMip->m_Width * sizeof(DWORD);
		cRequest.m_Width 		= pMip->m_Width;
		cRequest.m_Height 		= pMip->m_Height;

		if (pTexture->m_Header.GetBPPIdent() == BPP_32P)
		{
			DtxSection *pSection = dtx_FindSection(pTexture, "PALLETE32");
			if (pSection)
			{
				cRequest.m_pSrcPalette = (RPaletteColor*)pSection->m_Data;
			}
		}

		FormatMgr formatMgr;
		if (formatMgr.ConvertPixels(&cRequest) != LT_OK)
		{
			return false;
		}

		// Store this in the stack so it doesn't crash if they change it in the other thread.
		BPPIdent bppIdent = m_TextureProp.m_BPPIdent;

		// Possibly compress.
		if (IsBPPCompressed(bppIdent) && bppIdent != pTexture->m_Header.GetBPPIdent())
		{
			S3TC_Compressor compressor;

			compressor.m_Format = bppIdent;
			compressor.m_Width 	= pMip->m_Width;
			compressor.m_Height = pMip->m_Height;
			compressor.m_pData 	= rgbData.GetArray();
			compressor.m_Pitch 	= pMip->m_Width * sizeof(DWORD);
			compressor.m_DataFormat.InitPValueFormat();

			if (compressor.CompressUsingLibrary() == LT_OK)
			{
				// Now decompress into the PValue format.
				cRequest.m_pSrcFormat->Init(bppIdent, 0, 0, 0, 0);
				cRequest.m_pSrc 		= (BYTE*)compressor.m_pOutData;
				cRequest.m_pDestFormat->InitPValueFormat();
				cRequest.m_pDest 		= (BYTE*)rgbData.GetArray();
				cRequest.m_DestPitch 	= pMip->m_Width * sizeof(DWORD);
				cRequest.m_Width 		= pMip->m_Width;
				cRequest.m_Height 		= pMip->m_Height;

				formatMgr.ConvertPixels(&cRequest);

				delete compressor.m_pOutData;
			}
		}

		// If this is the first mipmap, then save the width and height for the window dims.
		if (iMipmap == 0)
		{
			m_DocImageWidth		= pMip->m_Width;
			m_DocImageHeight	= pMip->m_Height;
		}

		// Update the overall window dims.
		m_DocTotalWidth 	+= pMip->m_Width;
		m_DocTotalHeight	+= pMip->m_Height;

		// Create the image buffers.
		m_ImageBuffers[iMipmap].Init(pMip->m_Width, pMip->m_Height);

		DWORD *pInLine = rgbData.GetArray();
		DWORD a, r, g, b;

		unsigned iBuf = 0;
		for (DWORD y = 0; y < pMip->m_Height; ++y)
		{
			for (DWORD x = 0; x < pMip->m_Width; ++x)
			{
				PValue_Get(pInLine[x], a, r, g, b);

				m_ImageBuffers[iMipmap].m_Image[iBuf] = RGB(b, g, r);
				m_ImageBuffers[iMipmap].m_Alpha[iBuf] = RGB(a, a, a);

				++iBuf;
			}

			pInLine += pMip->m_Width;
		}
	}

	return true;
}



void CDtxViewDoc::DestroyBuffers()
{
	for (unsigned i = 0; i < MAX_DTX_MIPMAPS; ++i)
	{
		m_ImageBuffers[i].Term();
	}

	m_DocTotalWidth 	= 0;
	m_DocTotalHeight	= 0;
	m_DocImageWidth		= 0;
	m_DocImageHeight	= 0;
}



bool CDtxViewDoc::GetImageInfoString(CString *pText) const
{
	if (NULL == pText)
	{
		return false;
	}

	// Validate the texture data.
	if (!IsValidTexture())
	{
		return false;
	}

	// Get the texture format string.
    char Format[30];
	switch (m_TextureProp.m_BPPIdent)
	{
		case BPP_8P:
			strcpy(Format, "8 bit palette");
			break;
		case BPP_8:
			strcpy(Format, "8 bit");
			break;
		case BPP_16:
			strcpy(Format, "16 bit");
			break;
		case BPP_32:
			strcpy(Format, "32 bit");
			break;
		case BPP_S3TC_DXT1:
			strcpy(Format, "DXT1");
			break;
		case BPP_S3TC_DXT3:
			strcpy(Format, "DXT3");
			break;
		case BPP_S3TC_DXT5:
			strcpy(Format, "DXT5");
			break;
		case BPP_32P:
			strcpy(Format, "32 bit palette");
			break;
		default:
			break;
	}

    char buf[100];
    wsprintf(buf, "%u x %u, %s", m_DocImageWidth, m_DocImageHeight, Format);
	*pText = buf;

	return true;
}



bool CDtxViewDoc::GetDocumentSize(CSize *pSize) const
{
	// Validate the texture data.
	if (!IsValidTexture())
	{
		return false;
	}

	// Get the width.
	unsigned Width = m_DocTotalWidth;
	if (!m_ViewAlphaChannel)
	{
		Width = m_DocImageWidth;
	}

	// Get the height.
	unsigned Height = m_DocTotalHeight;
	if (!m_ViewMipmapLevels)
	{
		Height = m_DocImageHeight;
	}

	// Validate the width and height.
	if (Width <= 0 || Height <= 0)
	{
		return false;
	}

	// Set the size.
	*pSize = CSize(Width, Height);

	return true;
}



void CDtxViewDoc::DrawTexture(CDC *pDC)
{
	// Validate the texture data.
	TextureData *pTexture = m_TextureProp.m_pTexture;
	if (NULL == pTexture)
	{
		return;
	}

	// Create an in-memory DC compatible with the display DC we're using to paint.
	CDC dcMemory;
	dcMemory.CreateCompatibleDC(pDC);

	DWORD outYCoord = 0;

	// Set the number of mipmap levels to display.
	DWORD NumMipsToDisplay = 1;
	if (m_ViewMipmapLevels)
	{
		NumMipsToDisplay = pTexture->m_Header.m_nMipmaps;
	}

	for (DWORD iMipmap=0; iMipmap < NumMipsToDisplay; ++iMipmap)
	{
		if (NULL == m_ImageBuffers[iMipmap].m_Image ||
			NULL == m_ImageBuffers[iMipmap].m_Alpha)
		{
			continue;
		}

		unsigned Width = m_ImageBuffers[iMipmap].m_Width;
		unsigned Height = m_ImageBuffers[iMipmap].m_Height;

		// Copy the image pixels into the bitmap.
		CBitmap mipImage;
		mipImage.CreateCompatibleBitmap(pDC, Width, Height);
		mipImage.SetBitmapBits(m_ImageBuffers[iMipmap].m_Bytes, m_ImageBuffers[iMipmap].m_Image);

		// Select the bitmap into the in-memory DC.
		CBitmap* pOldBitmap = dcMemory.SelectObject(&mipImage);

		// Blit the image.
		pDC->BitBlt(0, outYCoord, Width, Height, &dcMemory, 0, 0, SRCCOPY);

		if (m_ViewAlphaChannel)
		{
			// Copy the alpha pixels into the bitmap.
			mipImage.SetBitmapBits(m_ImageBuffers[iMipmap].m_Bytes, m_ImageBuffers[iMipmap].m_Alpha);

			// Select the alpha image into the in-memory DC.
			dcMemory.SelectObject(&mipImage);

			// Blit the alpha image.
			pDC->BitBlt(Width, outYCoord, Width, Height, &dcMemory, 0, 0, SRCCOPY);
		}

		// Select the original bitmap.
		dcMemory.SelectObject(pOldBitmap);

		outYCoord += Height;
	}
}
