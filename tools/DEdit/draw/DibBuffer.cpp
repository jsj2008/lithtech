//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#include "bdefs.h"
#include "dibbuffer.h"
#include "oldtypes.h"



static HPALETTE _CreateIdentityPalette(RGBQUAD aRGB[], int nColors)
{
	int i;
	struct {
		WORD Version;
		WORD NumberOfEntries;
		PALETTEENTRY aEntries[256];
	} Palette =
	{
		0x300,
		256
	};

 
	//*** Just use the screen DC where we need it
	HDC hdc = GetDC(NULL);


	//*** For SYSPAL_NOSTATIC, just copy the color table into
	//*** a PALETTEENTRY array and replace the first and last entries
	//*** with black and white
	if (GetSystemPaletteUse(hdc) == SYSPAL_NOSTATIC)

	{
		//*** Fill in the palette with the given values, marking each
		//*** as PC_NOCOLLAPSE
		for(i = 0; i < nColors; i++)
		{
			Palette.aEntries[i].peRed = aRGB[i].rgbRed;
			Palette.aEntries[i].peGreen = aRGB[i].rgbGreen;
			Palette.aEntries[i].peBlue = aRGB[i].rgbBlue;
			Palette.aEntries[i].peFlags = PC_NOCOLLAPSE;
		}

		//*** Mark any unused entries PC_NOCOLLAPSE
		for (; i < 256; ++i)
		{
			Palette.aEntries[i].peFlags = PC_NOCOLLAPSE;

		}

		//*** Make sure the last entry is white
		//*** This may replace an entry in the array!
		Palette.aEntries[255].peRed = 255;
		Palette.aEntries[255].peGreen = 255;
		Palette.aEntries[255].peBlue = 255;
		Palette.aEntries[255].peFlags = 0;

		//*** And the first is black
		//*** This may replace an entry in the array!
		Palette.aEntries[0].peRed = 0;
		Palette.aEntries[0].peGreen = 0;
		Palette.aEntries[0].peBlue = 0;
		Palette.aEntries[0].peFlags = 0;

	}
	else
	//*** For SYSPAL_STATIC, get the twenty static colors into
	//*** the array, then fill in the empty spaces with the
	//*** given color table
	{
		int nStaticColors;
		int nUsableColors;

		//*** Get the static colors from the system palette
		nStaticColors = GetDeviceCaps(hdc, NUMCOLORS);
		GetSystemPaletteEntries(hdc, 0, 256, Palette.aEntries);

		//*** Set the peFlags of the lower static colors to zero
		nStaticColors = nStaticColors / 2;

		for (i=0; i<nStaticColors; i++)
			Palette.aEntries[i].peFlags = 0;

		//*** Fill in the entries from the given color table
		nUsableColors = nColors - nStaticColors;
		for (; i<nUsableColors; i++)
		{
			Palette.aEntries[i].peRed = aRGB[i].rgbRed;
			Palette.aEntries[i].peGreen = aRGB[i].rgbGreen;
			Palette.aEntries[i].peBlue = aRGB[i].rgbBlue;
			Palette.aEntries[i].peFlags = PC_NOCOLLAPSE;
		}

		//*** Mark any empty entries as PC_NOCOLLAPSE

		for (; i<256 - nStaticColors; i++)
			Palette.aEntries[i].peFlags = PC_NOCOLLAPSE;

		//*** Set the peFlags of the upper static colors to zero
		for (i = 256 - nStaticColors; i<256; i++)
			Palette.aEntries[i].peFlags = 0;
	}

	//*** Remember to release the DC!
	ReleaseDC(NULL, hdc);

	//*** Return the palette
	return CreatePalette((LOGPALETTE *)&Palette);
}



CDibImageBuffer::CDibImageBuffer()
{
	m_Width = m_Height = m_Pitch = m_BitDepth = 0;
	m_pBuffer = NULL;
	
	m_hBitmap = NULL;
	m_hPalette = NULL;
		
	memset( m_Palette, 0, sizeof(m_Palette) );
}



CDibImageBuffer::~CDibImageBuffer()
{
	DestroySurface();
}



BOOL CDibImageBuffer::CreateSurface( DWORD width, DWORD height, DWORD depth, HDC hDC )
{
	DWORD	pitch;
	
	pitch = width + (width%4);
	
	if( m_hBitmap )
		DestroySurface();

	if( depth != 8 && depth != 16 && depth != 24 && depth != 32 )
		return FALSE;

	// Setup the bitmap info.
	m_BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_BitmapInfo.bmiHeader.biWidth = pitch;
	m_BitmapInfo.bmiHeader.biHeight = -((SDWORD)height);
	m_BitmapInfo.bmiHeader.biPlanes = 1;
	m_BitmapInfo.bmiHeader.biBitCount = (WORD)depth;
	m_BitmapInfo.bmiHeader.biCompression = BI_RGB;
	m_BitmapInfo.bmiHeader.biSizeImage = 0;
	m_BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	m_BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	m_BitmapInfo.bmiHeader.biClrUsed = 0;
	m_BitmapInfo.bmiHeader.biClrImportant = 0;

	// Initialize our color table (in case it's 8-bit).
	memset( m_BitmapInfo.m_Colors, 0, sizeof(m_BitmapInfo.m_Colors) );


	// Create the Dib section.
	m_hBitmap = CreateDIBSection( hDC, (BITMAPINFO*)&m_BitmapInfo, DIB_RGB_COLORS, &m_pBuffer, NULL, 0 );
	if( m_hBitmap )
	{
		m_Width = width;
		m_Height = height;
		m_Pitch = pitch;
		m_BitDepth = depth;

		// Set the palette to black initially.
		memset(m_Palette, 0, 768);
		SetPalette(m_Palette);

		return TRUE;
	}
	else
		return FALSE;
}



BOOL CDibImageBuffer::DestroySurface()
{
	if( !m_hBitmap )
		return FALSE;


	if( m_hPalette )
	{
		DeleteObject( m_hPalette );
		m_hPalette = NULL; 
	}
	
	DeleteObject( m_hBitmap );
	m_hBitmap = NULL;

	m_Width = m_Height = m_Pitch = m_BitDepth = 0;
	return TRUE;
}



BOOL CDibImageBuffer::SetPalette( BYTE *pPalette )
{
	WORD		i;

	if( !m_hBitmap || m_BitDepth != 8 )
		return FALSE;

	memcpy( m_Palette, pPalette, 768 );

/*
	// Hard code the first and last 10 in this palette to
	// ugly green, so they'll totally know if they use it!
	for( i=0; i < 10; i++ )
	{
		m_Palette[i*3] = 0;
		m_Palette[i*3+1] = 255;
		m_Palette[i*3+2] = 0;
	}

	for( i=245; i < 255; i++ )
	{
		m_Palette[i*3] = 0;
		m_Palette[i*3+1] = 255;
		m_Palette[i*3+2] = 0;
	}
*/

	if( m_hPalette != 0 )
		DeleteObject( m_hPalette );

	// Put the colors in a bunch of RGBQUADs.
	for( i=0; i < 256; i++ )
	{
		m_BitmapInfo.m_Colors[i].rgbRed = m_Palette[i*3];
		m_BitmapInfo.m_Colors[i].rgbGreen = m_Palette[i*3+1];
		m_BitmapInfo.m_Colors[i].rgbBlue = m_Palette[i*3+2];
	}

	m_hPalette = _CreateIdentityPalette(m_BitmapInfo.m_Colors, 256);
	return TRUE;
}



BOOL CDibImageBuffer::Display( HDC hDC, CRect rect, CPoint offset )
{
	BOOL			bRetVal;
	HPALETTE		hOldPalette;


	ASSERT( rect.left >= 0 && rect.left <= (int)m_Width && rect.right >= 0 && rect.right <= (int)m_Width &&
			rect.top >= 0 && rect.top <= (int)m_Height && rect.bottom >= 0 && rect.bottom <= (int)m_Height );

	if( !m_hBitmap )
		return FALSE;

	hOldPalette = SelectPalette( hDC, m_hPalette, FALSE );
	RealizePalette( hDC );
	
	bRetVal = SetDIBitsToDevice( hDC, offset.x, offset.y,
								rect.Width(), rect.Height(),
								offset.x, offset.y,
								0, m_Height,
								m_pBuffer,
								(BITMAPINFO*)&m_BitmapInfo,
								DIB_RGB_COLORS );

	SelectPalette( hDC, hOldPalette, FALSE );
	return bRetVal;
}



	
