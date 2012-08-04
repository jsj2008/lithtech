// DtxShellExDoc.cpp : implementation of the CDtxShellExDoc class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
//#include "DtxShellEx.h"

#include "DtxShellExDoc.h"
#include "resource.h"
#include "s3tc_compress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MIPMAP_LEVEL_ONE 0

/////////////////////////////////////////////////////////////////////////////
// CDtxShellExDoc

IMPLEMENT_DYNCREATE(CDtxShellExDoc, CDocument)

BEGIN_MESSAGE_MAP(CDtxShellExDoc, CDocument)
	//{{AFX_MSG_MAP(CDtxShellExDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



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



/////////////////////////////////////////////////////////////////////////////
// CDtxShellExDoc construction/destruction

CDtxShellExDoc::CDtxShellExDoc():
m_bBuffersCreated(false)
{
	// TODO: add one-time construction code here

}

CDtxShellExDoc::~CDtxShellExDoc()
{
	if(m_bBuffersCreated)
	{
		//delete the buffers
		m_ImageBuffer.Term();
		m_bBuffersCreated = false;
	}
}

BOOL CDtxShellExDoc::OnNewDocument()
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDtxShellExDoc serialization

void CDtxShellExDoc::Serialize(CArchive& ar)
{
	// TODO: Serialize your document here
	if (ar.IsStoring())
	{
	//	ar << m_sizeDoc;
	}
	else
	{
	//	ar >> m_sizeDoc;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CDtxShellExDoc data load
void CDtxShellExDoc::LoadDTX(const char* szFullFilePath)
{
	// Try to load the texture file.
	if (m_TextureProp.Init(szFullFilePath))
	{
		CreateBuffers();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDtxShellExDoc diagnostics

#ifdef _DEBUG
void CDtxShellExDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDtxShellExDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDtxShellExDoc commands

BOOL CDtxShellExDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	return TRUE;
}

void CDtxShellExDoc::DeleteContents()
{
	// TODO: Delete the contents of your documents here
	m_sizeDoc = CSize(100,100);
	CDocument::DeleteContents();
}

// copied from CDtxShellExView, replace GetDocument() with this
void CDtxShellExDoc::OnDraw(CDC* pDC)
{
	CDtxShellExDoc* pDoc = this; //GetDocument();
	ASSERT_VALID(pDoc);

	if(m_bBuffersCreated)
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

		unsigned Width = m_ImageBuffer.m_Width;
		unsigned Height = m_ImageBuffer.m_Height;

		// Copy the image pixels into the bitmap.
		CBitmap mipImage;
		mipImage.CreateCompatibleBitmap(pDC, Width, Height);
		mipImage.SetBitmapBits(m_ImageBuffer.m_Bytes, m_ImageBuffer.m_Image);

		// Select the bitmap into the in-memory DC.
		CBitmap* pOldBitmap = dcMemory.SelectObject(&mipImage);

		pDC->SetStretchBltMode(HALFTONE);
		//pDC->SetBrushOrgEx() // you should use this if you're going to write to the image now.

		// Blit the image.
		pDC->StretchBlt(0, 0, 100, 100, &dcMemory, 0, 0, Width, Height, SRCCOPY);		

		// Select the original bitmap.
		dcMemory.SelectObject(pOldBitmap);
	}

}

bool CDtxShellExDoc::CreateBuffers()
{
	m_ImageBuffer.Term();

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
	
		TextureMipData *pMip = &pTexture->m_Mips[MIPMAP_LEVEL_ONE];

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

		// Create the image buffers.
		m_ImageBuffer.Init(pMip->m_Width, pMip->m_Height);

		DWORD *pInLine = rgbData.GetArray();
		DWORD a, r, g, b;

		unsigned iBuf = 0;
		for (DWORD y = 0; y < pMip->m_Height; ++y)
		{
			for (DWORD x = 0; x < pMip->m_Width; ++x)
			{
				PValue_Get(pInLine[x], a, r, g, b);

				m_ImageBuffer.m_Image[iBuf] = RGB(b, g, r);
				m_ImageBuffer.m_Alpha[iBuf] = RGB(a, a, a);

				++iBuf;
			}

			pInLine += pMip->m_Width;
		}

	m_bBuffersCreated = true;
	return true;
}