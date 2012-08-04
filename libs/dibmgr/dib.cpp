/****************************************************************************
;
;	 MODULE:		DIB (.CPP)
;
;	PURPOSE:		DIB (Device Independent Bitmap) Class
;
;	HISTORY:		02/18/96  [blg]  This file was created
;
;	COMMENT:		Copyright (c) 1996, Monolith Inc.
;
****************************************************************************/


// Includes...

#include "stdafx.h"
#include "dib.h"
#include "dibmgr.h"

#define RGB_TO_16(x)	(WORD)( ((x.peRed>>3)<<10) | ((x.peGreen>>3)<<5) | (x.peBlue>>3) )



// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CDib::Init(HDC hDC, int width, int height, int depth, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());
	ASSERT(hDC);
	ASSERT(width > 0);
//	ASSERT(height > 0);  // Can be < 0 if they don't want it flipped upside down!
//	ASSERT(width % 4 == 0);
//	ASSERT(depth == 8);

	/* This is not the case anymore!  SHP 8/20/98
	if (width % 4 != 0)
	{
		TRACE("CDib::Init() ERROR - Width must be a multiple of 4!\n");
		return(FALSE);
	}
	*/

	// Set simple member variables...

	m_dwFlags      = 0;
	m_nWidth       = width;
	m_nHeight      = ((height < 0) ? -height : height);
	m_nDepth       = depth;
	if( depth == 8)
	{
		// scanlines are DWORD aligned
		m_nPitch       = ((width+3) / 4) * 4;
	}
	else
	{
		// DEdit uses 16-bit dibs.
		//ASSERT(depth == 24);

		// 24 bit depth (actually 32 bit) is automatically DWORD aligned...
		m_nPitch = m_nWidth;
	}

	m_nStride      = m_nPitch - m_nWidth;
	m_bPalOwner    = FALSE;
	m_pPal         = NULL;
	m_bTransparent = TRUE;


	// Fill in the bitmap info structure...

	memset(&m_bmi.hdr, 0, sizeof(BITMAPINFOHEADER));

	m_bmi.hdr.biSize         = sizeof(BITMAPINFOHEADER);
	m_bmi.hdr.biWidth        = m_nWidth;
	m_bmi.hdr.biHeight       = height;
	m_bmi.hdr.biBitCount     = m_nDepth;
	m_bmi.hdr.biPlanes       = 1;
	m_bmi.hdr.biCompression  = BI_RGB;
	m_bmi.hdr.biSizeImage    = 0L;
	m_bmi.hdr.biClrUsed      = 0;
	m_bmi.hdr.biClrImportant = 0;


	// Set the colors to be an array of 16-bit indices...

	int i;
	WORD* pDW = (WORD*)m_bmi.colors;
	if (m_nDepth == 8)
	{
		for (i = 0; i < 256; i++)
		{
			*pDW++ = i;
		}
		m_hBmp = CreateDIBSection(hDC, (BITMAPINFO*)&m_bmi, DIB_PAL_COLORS, (void**)&m_pBytes, NULL, 0);
	}
	else
	{
		m_hBmp = CreateDIBSection(hDC, (BITMAPINFO*)&m_bmi, DIB_RGB_COLORS, (void**)&m_pBytes, NULL, 0);
	}

	if (!m_hBmp)
	{
		return(FALSE);
	}


	// Create the line look-up table...

	m_pLines = new DWORD [GetHeight()];

	for (i = 0; i < GetHeight(); i++)
	{
		m_pLines[i] = ((GetHeight() - 1) - i) * GetPitch() * (m_nDepth/8);
	}


	// All done...

	return(TRUE);
}

BOOL CDib::Init(BYTE* pBytes, HDC hDC, int width, int height, int depth, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());
	ASSERT(hDC);
	ASSERT(pBytes);


	// Let the main init do the real work...

	if (!Init(hDC, width, height, depth, flags))
	{
		return(FALSE);
	}


	// Copy the bytes...

	if (IsStrideless())
	{
		memcpy(m_pBytes, pBytes, GetBufferSize()*depth/8);
	}
	else
	{
		for (int y = 0; y < GetHeight(); y++)
		{
			memcpy(&m_pBytes[GetIndex(y)], pBytes, GetWidth());
			pBytes += GetWidth();
		}
	}


	// All done...

	return(TRUE);
}

BOOL CDib::Init(BYTE* pBytes, int type, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());


	// Determine what type of data we are dealing with...

	switch (type)
	{
		case DIB_DT_PCX:
		{
			return(InitPcx(pBytes, hDC, flags));
		}

		case DIB_DT_BMP:
		{
			return(InitBmp(pBytes, hDC, flags));
		}

		case DIB_DT_RID:
		{
			return(InitRid(pBytes, hDC, flags));
		}

		case DIB_DT_PID:
		{
			return(InitPid(pBytes, hDC, flags));
		}
	}

	return(FALSE);
}

BOOL CDib::Init(const char* sFile, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());


	// Determine how to load based on the file type...

	const char* pExt = strrchr(sFile, '.');

    if (pExt && stricmp(pExt, ".BMP") == 0)			// .BMP file?
	{
		return(InitBmp(sFile, hDC, flags));
	}
    else if (pExt && stricmp(pExt, ".PCX") == 0)	// .PCX file?
	{
		return(InitPcx(sFile, hDC, flags));
	}
    else if (pExt && stricmp(pExt, ".RID") == 0)	// .RID file?
	{
		return(InitRid(sFile, hDC, flags));
	}
    else if (pExt && stricmp(pExt, ".PID") == 0)	// .PID file?
	{
		return(InitPid(sFile, hDC, flags));
	}
	else											// assume .BMP in .RES
	{
		return(InitRes(sFile, hDC, flags));
	}
}



// ----------------------------------------------------------------------- //
//	ROUTINE:	CDib::Init
//	PURPOSE:	Inits by copying the source dib and converting to
//              the given bit depth if necessary.
// ----------------------------------------------------------------------- //

BOOL CDib::Init( HDC hDC, CDib *pDib, CDibPal *pPal ) 
{
	BYTE			*pSrcBuf;
	WORD			*pDestBuf;
	int				x, y;
	PALETTEENTRY	*pEntries, entry;


	if( !pPal )
		return FALSE;

	pEntries = pPal->GetPes();
	if( !pEntries )
		return FALSE;
	
	if( !Init(hDC, pDib->GetWidth(), pDib->GetHeight(), 16, 0) )
		return FALSE;

	for( y=0; y < GetHeight(); y++ )
	{
		pSrcBuf = &pDib->GetBytes()[ y*pDib->GetPitch() ];
		pDestBuf = &GetBuf16()[ y*GetPitch() ];
		
		for( x=0; x < GetWidth(); x++ )
		{
			entry = pEntries[*pSrcBuf];
			*pDestBuf = RGB_TO_16( entry );

			pSrcBuf++;
			pDestBuf++;
		}
	}

	return TRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CDib::Term()
{
	if (m_hBmp)
	{
		DeleteObject(m_hBmp);
		m_hBmp = NULL;
	}

	if (m_pLines)
	{
		delete[] m_pLines;
		m_pLines = NULL;
	}

	m_pBytes = NULL;
	m_pPal   = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::Resize
//
//	PURPOSE:	Resizes the dib
//
// ----------------------------------------------------------------------- //

BOOL CDib::Resize(HDC hDC, int width, int height, int depth, DWORD flags)
{
	// Check if this is a request for the same size...

	if (IsValid())
	{
		if (GetWidth() == width && GetHeight() == height)
		{
			return(TRUE);
		}
	}


	// Term existing dib stuff...

	Term();


	// Init with new info...

	return(Init(hDC, width, height, depth, flags));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::Fill
//
//	PURPOSE:	Fills the dib with the given color
//
// ----------------------------------------------------------------------- //

void CDib::Fill(BYTE pix)
{
	// Sanity checks...

	ASSERT(IsValid());


	// Do the fill as fast as possible...

	if (IsStrideless())
	{
		memset(m_pBytes, pix, GetBufferSize());
	}
	else
	{
		for (int y = 0; y < GetHeight(); y++)
		{
			memset(&m_pBytes[GetIndex(y)], pix, GetWidth());
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::InitBmp
//
//	PURPOSE:	Initialization for BMP data
//
// ----------------------------------------------------------------------- //

BOOL CDib::InitBmp(BYTE* pBytes, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());
	ASSERT(pBytes);


	// Get the header info...

	BYTE*				pData = pBytes;
	BYTE*				pStart;
	BITMAPINFOHEADER*	pBmiHdr;

	pStart   = pData;
	pBmiHdr  = (BITMAPINFOHEADER*)pData;
	pData   += sizeof(BITMAPINFO);
	
	int width  = pBmiHdr->biWidth;
	int height = pBmiHdr->biHeight;
	int depth  = pBmiHdr->biBitCount;

	
	// Get a pointer to the pixels in the buffer...

	if ( depth == 8 )
	{
		pData = &pStart[pBmiHdr->biSize + 1024L];
	}
	else
	{
		ASSERT(depth == 24);

		// if were handling anything besides 8 or 24 bit bmp's,
		// then we may need another case for computing the start
		// of the pixel data... just a reminder...
	}

	// Let the raw data initializer do all of the real work...

	BOOL bRet = Init(pData, hDC, width, height, depth, flags);


	// All done...

	return(bRet);
}

BOOL CDib::InitBmp(const char* sFile, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());
	ASSERT(sFile);


	// Open the bitmap file...

	CFile	file;
	DWORD	bufSize;
	
	BITMAPFILEHEADER	BmfHdr;
	BITMAPINFOHEADER	BmiHdr;
	
	if (!file.Open(sFile, CFile::modeRead)) return(FALSE);
	if (!file.Read((LPSTR)&BmfHdr, sizeof(BITMAPFILEHEADER))) return(FALSE);
	if (!file.Read((LPSTR)&BmiHdr, sizeof(BITMAPINFOHEADER))) return(FALSE);


	// Init an empty dib...

	int width  = BmiHdr.biWidth;
	int height = BmiHdr.biHeight;
	int depth  = BmiHdr.biBitCount;

	if (!Init(hDC, width, height, depth, flags))
	{
		return(FALSE);
	}

	
	// Seek to the pixels in the file...

	file.Seek(BmfHdr.bfOffBits, CFile::begin);


	// Get a pointer to the bytes...

	BYTE* pBytes = GetBytes();
	
	
	// Read the pixels into the buffer...

//	if (IsStrideless())	// can we ignore stride?
	{
		bufSize = m_nPitch * height * (depth/8);

		if (file.Read(pBytes, bufSize) != bufSize)
		{
			return(FALSE);
		}
	}
//	else				// deal with stride
//	{
//		for (int i = GetHeight() - 1; i >= 0; i--)
//		{
//			file.Read(&pBytes[GetIndex(i)], GetWidth());
//		}
//	}
			

	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::InitPcx
//
//	PURPOSE:	Initialization for PCX data
//
// ----------------------------------------------------------------------- //

BOOL CDib::InitPcx(BYTE* pBytes, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());
	ASSERT(pBytes);


	// Get the header info...

	BYTE*       pStart  = pBytes;
	DIB_PCXHDR* pPcxHdr = (DIB_PCXHDR*)pStart;

	
	// Get the size info and allocate memory for the buffer...

	int width  = (pPcxHdr->x1 - pPcxHdr->x0 + 1);
	int height = (pPcxHdr->y1 - pPcxHdr->y0 + 1);

	DWORD bufSize = width * height;

	if (pPcxHdr->bitsPerPixel != 8) 
		return(FALSE);

	// Create an empty dib...

	if (!Init(hDC, width, height, 8 * pPcxHdr->planes, flags))
	{
		return(FALSE);
	}

	
	// Get the packed pixels...
	
	DWORD offset  = sizeof(DIB_PCXHDR);
	BYTE* pPacked = &pStart[offset];


	// Unpack the pixels...
	
	int		i, j, n, y;
	BYTE	b;
	BYTE*	pSrcBuf = pPacked;
	BYTE*	pDestBuf;
	BYTE*   pScanlineBuf;

	pScanlineBuf = new BYTE[(width * pPcxHdr->bitsPerPixel * pPcxHdr->planes) / 8];
	
	for (y = 0; y < height; y++)
	{
		pDestBuf = GetAddress(y);

		n = width * pPcxHdr->planes;
		
		while (n > 0)
		{
			b = *pSrcBuf++;
			
			if ((b & 0xC0) == 0xC0)
			{
				i = b & 0x3F;
				b = *pSrcBuf++;
				
				for (j = 0; j < i; j++)
				{
					pScanlineBuf[--n] = b;
				}
			}
			else
			{
				pScanlineBuf[--n] = b;
			}
		}
		if (pPcxHdr->planes == 1)
		{
			for (i = width; i; i--)
				*pDestBuf++ = pScanlineBuf[i - 1];
		}
		else if (pPcxHdr->planes == 3)
		{
			for (i = width; i; i--)
			{
				*pDestBuf++ = pScanlineBuf[i - 1];
				*pDestBuf++ = pScanlineBuf[width + i - 1];
				*pDestBuf++ = pScanlineBuf[2 * width + i - 1];
			}
		}
	}

	delete pScanlineBuf;
	// All done...

	return(TRUE);
}

BOOL CDib::InitPcx(const char* sFile, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(sFile);


	// Load the file into memory...

	CFile file;

	if (!file.Open(sFile, CFile::modeRead))
	{
		return(FALSE);
	}

	DWORD size = static_cast<DWORD>(file.GetLength());
	if (size == 0) return(FALSE);

	BYTE* pData = new BYTE[size];
	if (!pData) return(FALSE);

	file.Read(pData, size);


	// Init using the raw data...

	BOOL bRet = InitPcx(pData, hDC, flags);


	// Clean up...

	delete pData;


	// All done...

	return(bRet);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::InitRid
//
//	PURPOSE:	Initialization for RID data
//
// ----------------------------------------------------------------------- //

BOOL CDib::InitRid(BYTE* pBytes, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());
	ASSERT(pBytes);


	// Get the header info...

	DWORD* pDWord = (DWORD*)pBytes;
	DWORD id      = *pDWord++;
	DWORD flags2  = *pDWord++;
	DWORD width   = *pDWord++;
	DWORD height  = *pDWord++;
	DWORD x       = *pDWord++;
	DWORD y       = *pDWord++;
	DWORD user1   = *pDWord++;
	DWORD user2   = *pDWord++;


	// Seek to the pixels in the file...

	BYTE* pData = (BYTE*)pDWord;


	// Let the raw data initializer do all of the real work...

	BOOL bRet = Init(pData, hDC, width, height, 8, flags);

	if (!(flags & 1))
	{
		SetTransparent(FALSE);
	}


	// All done...

	return(bRet);
}

BOOL CDib::InitRid(const char* sFile, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(sFile);


	// Load the file into memory...

	CFile file;

	if (!file.Open(sFile, CFile::modeRead))
	{
		return(FALSE);
	}

	DWORD size = static_cast<DWORD>(file.GetLength());
	if (size == 0) return(FALSE);

	BYTE* pData = new BYTE[size];
	if (!pData) return(FALSE);

	file.Read(pData, size);


	// Init using the raw data...

	BOOL bRet = InitRid(pData, hDC, flags);


	// Clean up...

	delete pData;


	// All done...

	return(bRet);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::InitPid
//
//	PURPOSE:	Initialization for PID data
//
// ----------------------------------------------------------------------- //

BOOL CDib::InitPid(BYTE* pBytes, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());
	ASSERT(pBytes);


	// Get the header info...

	DWORD* pDWord = (DWORD*)pBytes;
	DWORD id      = *pDWord++;
	DWORD flags2  = *pDWord++;
	DWORD width   = *pDWord++;
	DWORD height  = *pDWord++;
	DWORD dwX     = *pDWord++;
	DWORD dwY     = *pDWord++;
	DWORD user1   = *pDWord++;
	DWORD user2   = *pDWord++;


	// Create an empty dib...

	if (!Init(hDC, width, height, 8, flags))
	{
		return(FALSE);
	}

	if (!(flags & 1))
	{
		SetTransparent(FALSE);
	}

	// Get the packed pixels...
	
	BYTE* pPacked = (BYTE*)pDWord;

	int nTransparentIndex;
	if(flags2 & IDF_KEYINDEX)
		nTransparentIndex = LOWORD(user1);
	else
		nTransparentIndex = 0;


	// If this is an rle pid, decode into the buffer...

	if (flags2 & IDF_RLECOMPRESSED)
	{
		SetTransparent(TRUE);

		int   x=0;
		int   y=0;
		DWORD dwOffset=0;

		BYTE* pDestBuf = GetAddress(y);

		while (y < GetHeight())
		{		
			// Check to see if the run is a transparent run
			if (pPacked[dwOffset] & 128)
			{			
				memset(pDestBuf+x, nTransparentIndex, pPacked[dwOffset]-128 );
				x+=pPacked[dwOffset]-128;
				dwOffset++;
			}
			else
			{
				memcpy(pDestBuf+x, pPacked+dwOffset+1, pPacked[dwOffset]);				
				x+=pPacked[dwOffset];
				dwOffset+=pPacked[dwOffset]+1;
			}

			if (x >= GetWidth())
			{			
				y++;							
				x=0;
				if (y < GetHeight()) pDestBuf = GetAddress(y);
			}		
		}
	}

	
	// Unpack the pixels...

	else
	{
		int		i, j, n;
		DWORD	y;
		BYTE	b;
		BYTE*	pSrcBuf = pPacked;
		BYTE*	pDestBuf;
		
		for (y = 0; y < height; y++)
		{
			pDestBuf = GetAddress(y);

			n = width;
			
			while (n > 0)
			{
				b = *pSrcBuf++;
				
				if ((b & 0xC0) == 0xC0)
				{
					i = b & 0x3F;
					b = *pSrcBuf++;
					
					for (j = 0; j < i; j++)
					{
						*pDestBuf++ = b;
					}
					
					n -= i;
				}
				else
				{
					*pDestBuf++ = b;
					n--;
				}
			}
		}
	}


	// All done...

	return(TRUE);
}

BOOL CDib::InitPid(const char* sFile, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(sFile);


	// Load the file into memory...

	CFile file;

	if (!file.Open(sFile, CFile::modeRead))
	{
		return(FALSE);
	}

	DWORD size = static_cast<DWORD>(file.GetLength());
	if (size == 0) return(FALSE);

	BYTE* pData = new BYTE[size];
	if (!pData) return(FALSE);

	file.Read(pData, size);


	// Init using the raw data...

	BOOL bRet = InitPid(pData, hDC, flags);


	// Clean up...

	delete pData;


	// All done...

	return(bRet);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::InitRes
//
//	PURPOSE:	Initialization for BMP data in a windows resource
//
// ----------------------------------------------------------------------- //

BOOL CDib::InitRes(const char* sFile, HDC hDC, DWORD flags)
{
	// Sanity checks...

	ASSERT(sFile);


	// Make sure we have an instance handle...

	HINSTANCE hInst = CDibMgr::GetGlobalInstanceHandle();
	if (!hInst) return(FALSE);


	// Find the resource...

	HRSRC hRes = ::FindResource(hInst, sFile, RT_BITMAP);
	if (!hRes) return(FALSE);


	// Load the resource...

	HGLOBAL hMem = ::LoadResource(hInst, hRes);
	if (!hMem) return(FALSE);


	// Lock the resource...

	BYTE* pData = (BYTE*)LockResource(hMem);
	if (!pData)
	{
		FreeResource(hMem);
		return(FALSE);
	}


	// Init the data using the raw bmp data initializer...

	BOOL bRet = InitBmp(pData, hDC, flags);


	// Free the resource stuff...

	UnlockResource(hMem);
	FreeResource(hMem);


	// All done...

	return(bRet);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::Invert
//
//	PURPOSE:	Inverts the dib data
//
// ----------------------------------------------------------------------- //

void CDib::Invert()
{
	ASSERT(IsValid());
	
	if (GetHeight() <= 1) return;
	
	BYTE* pLine = new BYTE [GetWidth()];
	ASSERT(pLine);
	if (!pLine)
	{
		return;
	}
	
	DWORD k, s, d;
	    
	int j;	    
	int width  = GetWidth();
	int height = GetHeight();
	
	for (int i = 0; i < height / 2; i++)
	{
		k = i * width;
		for	(j = 0; j < width; j++) pLine[j] = m_pBytes[k++];
			
		s = (height - 1 - i) * width;
		d = i * width;
		for	(j = 0; j < (int)width; j++) m_pBytes[d++] = m_pBytes[s++];
			
		d = (height - 1 - i) * width;
		for	(j = 0; j < (int)width; j++) m_pBytes[d++] = pLine[j];
	}	
	
	delete [] pLine;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::Blt
//
//	PURPOSE:	Blits the given source dib onto the dib
//
// ----------------------------------------------------------------------- //

int CDib::Blt(CDib* pDib, int xDst, int yDst)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(pDib && pDib->IsValid());
	
	// Probably will support 16-to-16 and 8-to-16 sometime..
	ASSERT( (GetDepth() == 8) && (pDib->GetDepth() == 8) );


	// Set some stuff up...

	int srcWidth  = pDib->GetWidth();
	int srcHeight = pDib->GetHeight();
	int dstWidth  = GetWidth();
	int dstHeight = GetHeight();

	if (xDst < 0)
	{
		srcWidth -= -xDst;
		xDst      = 0;
	}

	if (xDst + srcWidth - 1 >= dstWidth)
	{
		srcWidth -= (xDst + srcWidth) - dstWidth;
	}

	if (yDst < 0)
	{
		srcHeight -= -yDst;
		yDst       = 0;
	}

	if (yDst + srcHeight - 1 >= dstHeight)
	{
		srcHeight -= (yDst + srcHeight) - dstHeight;
	}


	// If the source is transparent, deal with it...

	if (pDib->IsTransparent())
	{
		for (int y = 0; y < srcHeight; y++)
		{
			BYTE* pDst = GetAddress(xDst, yDst + y);
			BYTE* pSrc = pDib->GetAddress(y);

			for (int x = 0; x < srcWidth; x++)
			{
				if (*pSrc != 0) *pDst = *pSrc;
				pSrc++;
				pDst++;
			}
		}
	}
	else
	{
		for (int y = 0; y < srcHeight; y++)
		{
			memcpy(GetAddress(xDst, yDst + y), pDib->GetAddress(y), srcWidth);
		}
	}

	return(srcHeight);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::SetPalette
//
//	PURPOSE:	Sets the palette for this dib
//
// ----------------------------------------------------------------------- //

void CDib::SetPalette(CDibPal* pPal, BOOL bOwner)
{
	// Sanity checks...

	ASSERT(IsValid());


	// Set member variables...

#ifdef _DEBUG
	if (m_pPal && m_bPalOwner)
	{
		TRACE("WARNING - Owner palette not removed\n");
	}
#endif

	m_pPal      = pPal;
	m_bPalOwner = bOwner;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::Scale
//
//	PURPOSE:	Scales the dib to the new dimensions
//
// ----------------------------------------------------------------------- //

BOOL CDib::Scale(int nNewWidth, int nNewHeight, int nNewDepth, DWORD flags)
{
	// Sanity checks...

	ASSERT(IsValid());


	// TODO: Add scale code, update all appropriate member variables...

	return(FALSE);	// (temp) remove this line when this function works!


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::Save
//
//	PURPOSE:	Saves the dib to a file
//
// ----------------------------------------------------------------------- //

BOOL CDib::Save(const char* sFile, CDibPal* pPal)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(sFile);


	// Saved based on bit-depth...

	switch (GetDepth())
	{
		case 8:
		{
			return(Save8(sFile, pPal));
		}

		case 16:
		{
			return(FALSE);
		}

		case 24:
		{
			return(FALSE);
		}

		default:
		{
			return(FALSE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::Save8
//
//	PURPOSE:	Saves the 8-bit dib to a file
//
// ----------------------------------------------------------------------- //

BOOL CDib::Save8(const char* sFile, CDibPal* pPal)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(sFile);
	

	// Make sure we have a palette...

	if (!pPal)
	{
		pPal = m_pPal;
	}

	if (!pPal)
	{
		return(FALSE);
	}


	// Construct the bitmap info header...

	DIB_BMI256 bmi;
	memset(&bmi, 0, sizeof(DIB_BMI256));

	bmi.hdr.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.hdr.biWidth       = GetWidth();
	bmi.hdr.biHeight      = GetHeight();
	bmi.hdr.biPlanes      = 1;
	bmi.hdr.biBitCount    = 8;
	bmi.hdr.biCompression = BI_RGB;
	bmi.hdr.biSizeImage   = 0;


	// Fill in the palette info...

	PALETTEENTRY* pPes = pPal->GetPes();
	if (!pPes) return(FALSE);

	for (int i = 0; i < 256; i++)
	{
		bmi.colors[i].rgbRed   = pPes[i].peRed;
		bmi.colors[i].rgbGreen = pPes[i].peGreen;
		bmi.colors[i].rgbBlue  = pPes[i].peBlue;
	}


	// Construct the bitmap file header...

	BITMAPFILEHEADER	bmf;
	memset(&bmf, 0, sizeof(BITMAPFILEHEADER));
	strcpy((char*)&bmf.bfType, "BM");
	bmf.bfSize    = sizeof(BITMAPFILEHEADER) + sizeof(DIB_BMI256) + (GetWidth() * GetHeight());
	bmf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(DIB_BMI256);


	// Write the file...

	BYTE* pBytes = GetBytes();
	if (!pBytes) return(FALSE);

	CFile	file;

	if (!file.Open(sFile, CFile::modeCreate | CFile::modeWrite))
	{
		return(FALSE);
	}

	file.Write(&bmf, sizeof(BITMAPFILEHEADER));
	file.Write(&bmi, sizeof(DIB_BMI256));

	for (int y = GetHeight() - 1; y >= 0; y--)
	{
		DWORD i = GetIndex(y);
		file.Write(&pBytes[i], GetWidth());
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDib::FillRect
//
//	PURPOSE:	Fills the given destination rect with the given color.
//
// ----------------------------------------------------------------------- //

void CDib::FillRect(RECT* pRect, DWORD dwColor)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(pRect);


	// Copy the color to the rectangle...

	int fillWidth = pRect->right - pRect->left;

	for (int y = pRect->top; y <= pRect->bottom; y++)
	{
		memset(&m_pBytes[GetIndex(y) + pRect->left], dwColor, fillWidth);
	}
} 

void CDib::FillRect(int xDest, int yDest, RECT* pSrcRect, DWORD dwColor)
{
	CRect rcFill;

	rcFill.left   = xDest;
	rcFill.top    = yDest;
	rcFill.right  = xDest + (pSrcRect->right - pSrcRect->left);
	rcFill.bottom = yDest + (pSrcRect->bottom - pSrcRect->top);

	FillRect(&rcFill, dwColor);
}

