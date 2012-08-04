/****************************************************************************
;
;	 MODULE:		DIBPAL (.CPP)
;
;	PURPOSE:		DIB Palette Class
;
;	HISTORY:		02/18/96  [blg]  This file was created
;
;	COMMENT:		Copyright (c) 1996, Monolith Inc.
;
****************************************************************************/


// Includes...

#include "stdafx.h"
#include "dibpal.h"
#include "dibmgr.h"


// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibPal::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CDibPal::Init(PALETTEENTRY* pPes, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());
	ASSERT(pPes);


	// Set simple member variables...

	m_dwFlags = flags;


	// Fill in the logical palette structure...

	m_logPal.version    = 0x300;
	m_logPal.numEntries = 256;

	for (int i = 0; i < 256; i++)
	{
		m_logPal.pes[i]         = pPes[i];
		m_logPal.pes[i].peFlags = 0;
	}


	// Make an identity palette if necessary...

	if (IsPaletteDevice() && !(flags & DMPF_NOIDENTITY))
	{
		MakeIdentity();
		m_bIdentity = TRUE;
	}


	// Create the palette...

	m_hPal = CreatePalette((LOGPALETTE*)&m_logPal);

	if (!m_hPal)
	{
		return(FALSE);
	}


	// All done...

	return(TRUE);
}

BOOL CDibPal::Init(BYTE* pRgbs, DWORD flags)
{
	// Copy the rgb values into a pes...

	PALETTEENTRY	pe[256];

	int j = 0;

	for (int i = 0; i < 256; i++)
	{
		pe[i].peRed   = pRgbs[j++];
		pe[i].peGreen = pRgbs[j++];
		pe[i].peBlue  = pRgbs[j++];
	}


	// Let the main init function do the real work...

	return(Init((PALETTEENTRY*)&pe, flags));
}

BOOL CDibPal::Init(RGBQUAD* pQuads, DWORD flags)
{
	// Copy the rgb values into a pes...

	PALETTEENTRY	pe[256];

	for (int i = 0; i < 256; i++)
	{
		pe[i].peRed   = pQuads[i].rgbRed;
		pe[i].peGreen = pQuads[i].rgbGreen;
		pe[i].peBlue  = pQuads[i].rgbBlue;
	}


	// Let the main init function do the real work...

	return(Init((PALETTEENTRY*)&pe, flags));
}

BOOL CDibPal::Init(RGBTRIPLE* pTrips, DWORD flags)
{
	// Copy the rgb values into a pes...

	PALETTEENTRY	pe[256];

	for (int i = 0; i < 256; i++)
	{
		pe[i].peRed   = pTrips[i].rgbtRed;
		pe[i].peGreen = pTrips[i].rgbtGreen;
		pe[i].peBlue  = pTrips[i].rgbtBlue;
	}


	// Let the main init function do the real work...

	return(Init((PALETTEENTRY*)&pe, flags));
}

BOOL CDibPal::Init(const char* sFile, DWORD flags)
{
	// Determine how to init based on the file type...

	const char* pExt = strrchr(sFile, '.');

    if (pExt && stricmp(pExt, ".BMP") == 0)			// .BMP file?
	{
		return(InitBmp(sFile, flags));
	}
    else if (pExt && stricmp(pExt, ".PCX") == 0)	// .PCX file?
	{
		return(InitPcx(sFile, flags));
	}
    else if (pExt && stricmp(pExt, ".PAL") == 0)	// .PAL file?
	{
		return(InitPal(sFile, flags));
	}
	else											// assume .PAL data in .RES
	{
		return(InitRes(sFile, flags));		
	}
}


BOOL CDibPal::Init(BYTE *pData, DWORD dataLen, int type, DWORD flags)
{
	if( type == DIB_DT_PCX )
		return InitPcx( pData, dataLen, flags );
	else
		return FALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibPal::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CDibPal::Term()
{
	if (m_hPal)
	{
		DeleteObject(m_hPal);
		m_hPal    = NULL;
	}

	m_dwFlags = 0;
}


// ---------------------------------------------------------------------- //
//
//	ROUTINE:	CDibPal::IsPaletteDevice
//
//	PURPOSE:	Static function to determine if the system's display
//				device supports a palette.
//
// ---------------------------------------------------------------------- //

BOOL CDibPal::IsPaletteDevice()
{
	HDC		hDC;

	hDC = CreateIC("DISPLAY", NULL, NULL, NULL);

	if (hDC)
	{
		BOOL bPal = GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE;
		DeleteDC(hDC);
		return(bPal);
	}
	else
	{
		return(FALSE);
	}
}


// ---------------------------------------------------------------------- //
//
//	ROUTINE:	MakeIdentity
//
//	PURPOSE:	Makes the logical palette an identity palette
//
// ---------------------------------------------------------------------- //

void CDibPal::MakeIdentity()
{
	ClearSystemPalette();

	HDC	hDC = CreateDC("DISPLAY", NULL, NULL, NULL);

	int nNumPalColors = GetDeviceCaps(hDC, SIZEPALETTE);
	int nNumSysColors = GetDeviceCaps(hDC, NUMRESERVED);

	LOGPALETTE* pLogPal = (LOGPALETTE*)&m_logPal;

	GetSystemPaletteEntries(hDC, 0, nNumSysColors / 2, &(pLogPal->palPalEntry[0]));
	GetSystemPaletteEntries(hDC, nNumPalColors - nNumSysColors / 2, nNumSysColors / 2, &(pLogPal->palPalEntry[pLogPal->palNumEntries - nNumSysColors / 2])); 
	
	for (int i = (nNumSysColors / 2); i < nNumPalColors - (nNumSysColors / 2); i++)
	{
		pLogPal->palPalEntry[i].peFlags = PC_RESERVED;
	}
	
	DeleteDC(hDC);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClearSystemPalette
//
//	PURPOSE:	Clears the system palette
//
// ----------------------------------------------------------------------- //

void CDibPal::ClearSystemPalette()
{
	DIB_LOGPAL256	sysPal;
	HPALETTE		hScreenPal = NULL;
	HDC				hScreenDC  = GetDC(NULL);
	

	// Reset everything in the system palette to black...
	
	sysPal.version    = 0x300;
	sysPal.numEntries = 256;
	
	for(int iPal = 0; iPal < 256; iPal++)
	{
		sysPal.pes[iPal].peRed   = 0;
		sysPal.pes[iPal].peGreen = 0;
		sysPal.pes[iPal].peBlue  = 0;
		sysPal.pes[iPal].peFlags = PC_NOCOLLAPSE;
	}
	

	// Create, select, realize, deselect, and delete the palette...
	
	hScreenPal = CreatePalette((LOGPALETTE *)&sysPal);
	
	if (hScreenPal)
	{
		hScreenPal = SelectPalette(hScreenDC, hScreenPal, FALSE);
		RealizePalette(hScreenDC);
		hScreenPal = SelectPalette(hScreenDC, hScreenPal, FALSE);
		DeleteObject(hScreenPal);
	}
	
	ReleaseDC(NULL, hScreenDC);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibPal::InitPal
//
//	PURPOSE:	Initialization for .PAL data
//
// ----------------------------------------------------------------------- //

BOOL CDibPal::InitPal(const char* sFile, DWORD flags)
{
	// Open and load the file...

	CFile file;

	if (!file.Open(sFile, CFile::modeRead))
	{
		return(FALSE);
	}

	if (file.GetLength() != 768) return(FALSE);

	BYTE	pRgbs[768];

	file.Read(pRgbs, 768);


	// Use the raw data initializer...

	return(Init(pRgbs, flags));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibPal::InitPcx
//
//	PURPOSE:	Initialization for .PCX data
//
// ----------------------------------------------------------------------- //

BOOL CDibPal::InitPcx(const char* sFile, DWORD flags)
{
	// Open the file...
	
	CFile file;
	
	if (!file.Open(sFile, CFile::modeRead))
	{
		return(FALSE);
	}


	// Seek to the colors at the end of the file...

	file.Seek(-768L, CFile::end);
	

	// Read the colors...

	BYTE	rgbs[768];
	
	if (!file.Read(rgbs, 768L))
	{
		return(FALSE);
	}


	// Copy the colors into a palette entry structure...

	PALETTEENTRY	pe[256];

	int j = 0;

	for (int i = 0; i < 256; i++)
	{
		pe[i].peRed   = rgbs[j++];
		pe[i].peGreen = rgbs[j++];
		pe[i].peBlue  = rgbs[j++];
		pe[i].peFlags = 0;
	}


	// Use another initializer...

	return(Init(pe, flags));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibPal::InitPcx
//
//	PURPOSE:	Initialization for .Pcx
//
// ----------------------------------------------------------------------- //

BOOL CDibPal::InitPcx(BYTE *pData, DWORD dataLen, DWORD flags)
{
	if( dataLen < 768 )
		return FALSE;
	
	// Read the colors...
	BYTE	*pRgbs = &pData[dataLen-768];
	

	// Copy the colors into a palette entry structure...

	PALETTEENTRY	pe[256];

	int j = 0;

	for (int i = 0; i < 256; i++)
	{
		pe[i].peRed   = pRgbs[j++];
		pe[i].peGreen = pRgbs[j++];
		pe[i].peBlue  = pRgbs[j++];
		pe[i].peFlags = 0;
	}


	// Use another initializer...

	return(Init(pe, flags));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibPal::InitBmp
//
//	PURPOSE:	Initialization for .BMP data
//
// ----------------------------------------------------------------------- //

BOOL CDibPal::InitBmp(const char* sFile, DWORD flags)
{
	// Open the file...
	
	CFile file;
	
	if (!file.Open(sFile, CFile::modeRead))
	{
		return(FALSE);
	}


	// Read the headers...

	BITMAPFILEHEADER	BmfHdr;
	BITMAPINFOHEADER	BmiHdr;

	if (!file.Read((LPSTR)&BmfHdr, sizeof(BITMAPFILEHEADER)))
	{
		return(FALSE);
	}
	
	if (!file.Read((LPSTR)&BmiHdr, sizeof(BITMAPINFOHEADER)))
	{
		return(FALSE);
	}


	// Read the colors...

	RGBQUAD	rgb[256];

	if (!file.Read(rgb, 256 * sizeof(RGBQUAD)))
	{
		return(FALSE);
	}


	// Copy the colors into a palette entry structure...

	PALETTEENTRY	pe[256];

	for (int i = 0; i < 256; i++)
	{
		pe[i].peRed   = rgb[i].rgbRed;
		pe[i].peGreen = rgb[i].rgbGreen;
		pe[i].peBlue  = rgb[i].rgbBlue;
		pe[i].peFlags = 0;
	}


	// Create the direct draw palette...

	return(Init(pe, flags));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibPal::InitRes
//
//	PURPOSE:	Initialization for .PAL data in a windows resource
//
// ----------------------------------------------------------------------- //

BOOL CDibPal::InitRes(const char* sFile, DWORD flags)
{
	// Sanity checks... 

	ASSERT(sFile);


	// Make sure we have an instance handle...

	HINSTANCE hInst = CDibMgr::GetGlobalInstanceHandle();
	if (!hInst) return(FALSE);


	// Find the resource...

	HRSRC hRes = ::FindResource(hInst, sFile, "PALETTE");
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

	BOOL bRet = Init(pData, flags);


	// Free the resource stuff...

	UnlockResource(hMem);
	FreeResource(hMem);


	// All done...

	return(bRet);
}

