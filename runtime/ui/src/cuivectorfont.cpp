//-------------------------------------------------------------------
//
//   MODULE    : CUIVECTORFONT.CPP
//
//   PURPOSE   : implements the CUIVectorFont font class
//
//   CREATED   : 1/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------

#include "bdefs.h"
#include "dtxmgr.h"
#include "sysstreamsim.h"

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __CUIVECTORFONT_H__
#include "cuivectorfont.h"
#endif

#ifndef __LTSYSOPTIM_H__
#include "ltsysoptim.h"
#endif

#include "LTFontParams.h"

#include "iltclient.h"
#include "interface_helpers.h"

#ifdef _MSC_VER
#include <TCHAR.h>
#else
#include <string>
#endif

// get the ILTTexInterface from the interface database
static ILTTexInterface *pTexInterface = NULL;
define_holder(ILTTexInterface, pTexInterface);

extern HDC g_hTextDC;

//ILTClient game interface
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);





// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WIDTHBYTES
//
//  PURPOSE:	Find the pitch of a bitmap.
//
//	nBitCount -	Number of bits per pixel.
//
//	nPixelWidth	-	Width in pixels.
//
// ----------------------------------------------------------------------- //

#define WIDTHBYTES( nBitCount, nPixelWidth ) (((( uint32 )nBitCount * nPixelWidth + 7) / 8 + 3) & ~3 )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WriteBmp
//
//  PURPOSE:	Writes a bitmap to disk.
//
//	pszFileName -	Filename to use.
//
//	bmi	-			Filled in bitmap info.
//
//	pDibBites -		Bitmap bits.
//
// ----------------------------------------------------------------------- //

bool WriteBmp( char const* pszFileName, BITMAPINFO const& bmi, uint8 const* pDibBits )
{
	// Save the bitmap.
	FILE* fp = fopen( pszFileName, "wb");
	if (!fp)
	{
		return false;
	}

	BITMAPFILEHEADER fileHeader;
	memset(&fileHeader, 0, sizeof(fileHeader));

	// Fix up the bitmap height so it works on win9x, which doesn't like
	// negative heights.
	BITMAPINFO bmiTopDown = bmi;
	bmiTopDown.bmiHeader.biHeight = abs( bmiTopDown.bmiHeader.biHeight );

	int nPitch = WIDTHBYTES( bmiTopDown.bmiHeader.biBitCount, bmiTopDown.bmiHeader.biWidth );
	int nImageByteSize = nPitch * bmiTopDown.bmiHeader.biHeight;

	fileHeader.bfType = ('M' << 8) | 'B';
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	fileHeader.bfSize = nImageByteSize + fileHeader.bfOffBits;

	fwrite(&fileHeader, sizeof(fileHeader), 1, fp);
	fwrite(&bmi.bmiHeader, sizeof(bmiTopDown.bmiHeader), 1, fp);


	fwrite( pDibBits, nImageByteSize, 1, fp );

	fclose(fp);

	return true;
}


// ----------------------------------------------------------------------- //
//
//  Class:	InstalledFontFace
//
//  PURPOSE:	Container for HFONT and font resource file.
//
// ----------------------------------------------------------------------- //

class InstalledFontFace
{
	public:

		InstalledFontFace( )
		{
			m_hFont = NULL;
			m_bDeleteFile = false;
			m_nHeight = 0;
		}

		~InstalledFontFace( )
		{
			Term( );
		}

		// Intialize the font face, from optional font file.
		bool Init( char const* pszFontFile, char const* pszFontFace, int nHeight, LTFontParams *fontParams);
		void Term( );

		HFONT			GetHFont( ) { return m_hFont; }
		char const*		GetFontFile( ) { return m_sFontFile.c_str( ); }
		char const*		GetFontFace( ) { return m_sFontFace.c_str( ); }
		int				GetHeight( ) { return m_nHeight; }

	private:

		std::string		m_sFontFile;
		std::string		m_sFontFace;
		HFONT			m_hFont;
		bool			m_bDeleteFile;
		int				m_nHeight;
};

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	InstalledFontFace::Init
//
//  PURPOSE:	Intialize the font face, from optional font file.
//
//	pszFontFile	-	Specifies font resource file to install.  If left
//					NULL, then the font face must already be installed on
//					the system.
//
//	pszFontFace -	Font face to create.
//
//	nHeight	-		Height of the font.
//
// ----------------------------------------------------------------------- //

bool InstalledFontFace::Init( char const* pszFontFile, char const* pszFontFace, int nHeight, LTFontParams *fontParams )
{
	// Check inputs.
	if( !pszFontFace || !pszFontFace[0] )
	{
		ASSERT( !"InstalledFontFace::Init:  Invalid parameters." );
		return false;
	}

	// Start fresh.
	Term( );

	char szOutName[_MAX_PATH] = "";

	// If they specified a font file, install it.
	if( pszFontFile && pszFontFile[0] )
	{
		// Get the path to a real file.
		bool bFileCopied = false;
		if( GetOrCopyClientFile( pszFontFile, szOutName, sizeof( szOutName ), bFileCopied ) != LT_OK )
		{
			DEBUG_PRINT( 1, ( "InstalledFontFace::Init:  Could not get font file %s.", szOutName ));
			return false;
		}

		// Signal ourselves to delete the temporary copy of the font file.
		m_bDeleteFile = bFileCopied;

		// Add the font resource.
		if( !AddFontResource( szOutName ))
		{
			DEBUG_PRINT( 1, ( "InstalledFontFace::Init: Could not install font file %s", szOutName ));
			return false;
		}
	}

	// Create the font they asked for.
	LOGFONT logFont;
	memset( &logFont, 0, sizeof( logFont ));
	logFont.lfHeight = nHeight;
	
	if(fontParams)
	{
		DEBUG_PRINT( 1, ("InstalledFontFace::Init Using User Defined Font Params!" ));
		logFont.lfWeight = fontParams->Weight;
		logFont.lfCharSet = fontParams->CharSet;
		logFont.lfOutPrecision = fontParams->OutPrecision;
		logFont.lfClipPrecision = fontParams->ClipPrecision;
		logFont.lfQuality = fontParams->Quality;
		logFont.lfPitchAndFamily = fontParams->PitchAndFamily;
		logFont.lfItalic = fontParams->Italic;
		logFont.lfUnderline = fontParams->Underline;
		logFont.lfStrikeOut = fontParams->StrikeOut;
	}
	else
	{
		logFont.lfWeight = FW_NORMAL;
		logFont.lfCharSet = ANSI_CHARSET;
		logFont.lfOutPrecision = OUT_TT_PRECIS;
		logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		logFont.lfQuality = ANTIALIASED_QUALITY;
		logFont.lfPitchAndFamily = DEFAULT_PITCH;
	}
	strncpy( logFont.lfFaceName, pszFontFace, LF_FACESIZE );
	logFont.lfFaceName[LF_FACESIZE - 1] = 0;

	m_hFont = ::CreateFontIndirect( &logFont );
	if( !m_hFont )
	{
		DEBUG_PRINT( 1, ( "InstalledFontFace::Init:  Could not create font face %s.", pszFontFace ));
		return false;
	}

	m_sFontFile = szOutName;
	m_sFontFace = pszFontFace;
	m_nHeight = nHeight;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	InstalledFontFace::Term
//
//  PURPOSE:	Destroy the font and clean up resources.
//
// ----------------------------------------------------------------------- //

void InstalledFontFace::Term( )
{
	// Delete the font.
	if( m_hFont )
	{
		DeleteObject( m_hFont );
		m_hFont = NULL;
	}

	// Uninstall the font file.
	if( !m_sFontFile.empty( ))
	{
		RemoveFontResource( m_sFontFile.c_str( ));

		// Check if we should delete the file.
		if( m_bDeleteFile )
		{
			DeleteFile( m_sFontFile.c_str( ));
		}

		m_sFontFile = "";
	}

	m_sFontFace = "";
	m_bDeleteFile = false;
	m_nHeight = 0;
}




//	--------------------------------------------------------------------------
// create a proportional bitmap font from a TrueType resource

CUIVectorFont::CUIVectorFont( )
{
}

CUIVectorFont::~CUIVectorFont()
{
	Term( );
}


bool CUIVectorFont::Init(
		char const* pszFontFile,
		char const* pszFontFace,
		uint32 pointSize,
		uint8  asciiStart,
		uint8  asciiEnd,
		LTFontParams* fontParams
		)
{
	char szChars[256];
	int i;
	for( i = asciiStart; i <= asciiEnd; i++ )
	{
		szChars[i - asciiStart] = i;
	}
	szChars[i - asciiStart] = 0;

	bool bOk = Init( pszFontFile, pszFontFace, pointSize, szChars, fontParams);

	return bOk;
}


// create a proportional font from TTF, and string
bool CUIVectorFont::Init(char const* pszFontFile,
						 char const* pszFontFace,
						 uint32 pointSize,
						 char const* pszCharacters,
						 LTFontParams* fontParams
						 )
{
	// Check inputs.
	if( !pszCharacters || !pszCharacters[0] || !pszFontFace || !pszFontFace[0] )
		return false;

	// Start fresh.
	Term( );

	// set the font defaults
	m_Proportional 		= true;

	InstalledFontFace installedFontFace;
	if( !installedFontFace.Init( pszFontFile, pszFontFace, pointSize, fontParams ))
		return false;

	// when the font is created, this can be
	// slightly different than pointSize
	m_PointSize 				= pointSize;

	// Create a Texture and a Font table
	bool bOk = CreateFontTextureAndTable( installedFontFace, pszCharacters, true );
	if( !bOk )
	{
		return false;
	}

	// font is valid
	m_Valid	= true;
	return true;
}

void CUIVectorFont::Term( )
{
	// free any used resources
	if (m_bAllocatedTable && m_pFontTable)
	{
		delete[] m_pFontTable;
		m_pFontTable = NULL;
	}

	if (m_bAllocatedMap && m_pFontMap)
	{
		delete[] m_pFontMap;
		m_pFontMap = NULL;
	}

	// release the HTEXTURE
	if (m_Texture)
	{
		pTexInterface->ReleaseTextureHandle(m_Texture);
		m_Texture = NULL;
	}

	m_CharTexWidth 		= 0;
	m_CharTexHeight 	= 0;

	// font is no longer valid
	m_Valid	= false;
}

// Spacing between each character in font map.
const int kCharSpacing = 2;


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetPowerOfTwo
//
//  PURPOSE:	Get the smallest power of two that is greater than the
//				input value.
//
//	nValue -	Value to start with.
//
// ----------------------------------------------------------------------- //

inline int GetPowerOfTwo( int nValue )
{
	int nPowerOfTwo = 32;
	while( nPowerOfTwo < nValue )
	{
		nPowerOfTwo *= 2;
	}

	return nPowerOfTwo;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetTextureSizeFromCharSizes
//
//  PURPOSE:	Finds the texture size that will fit character glyphs.
//
//	pGlyphMetrics -	Array of GLYPHMETRICS of each glpyh in font.
//
//	sizeMaxGlyphSize - Size of largest glyph.
//
//	nLen -			Number of characters in pCharSizes.
//
//	sizeTexture -	Function fills in with texture size.
//
// ----------------------------------------------------------------------- //

static void GetTextureSizeFromCharSizes( GLYPHMETRICS const* pGlyphMetrics, SIZE const& sizeMaxGlyphSize,
										int nLen, SIZE& sizeTexture )
{
	// Get the total area of the pixels of all the characters.  We use the largest glyph size
	// rather than the exact values because this is just a rough
	// guess and if we overestimate in width, we just get a shorter texture.
	int nTotalPixelArea = ( sizeMaxGlyphSize.cx + kCharSpacing ) * nLen *
		( sizeMaxGlyphSize.cy + kCharSpacing );

	// Use the square root of the area guess at the width.
	int nRawWidth = static_cast<int>( sqrtf(static_cast<float>(nTotalPixelArea)) + 0.5f );

	// Englarge the width to the nearest power of two and use that as our final width.
	sizeTexture.cx = GetPowerOfTwo( nRawWidth );

	// Start the height off as one row.
	int nRawHeight = sizeMaxGlyphSize.cy + kCharSpacing;

	// To find the height, keep putting characters into the rows until we reach the bottom.
	int nXOffset = 0;
	for( int nGlyph = 0; nGlyph < nLen; nGlyph++ )
	{
		// Get this character's width.
		int nCharWidth = pGlyphMetrics[ nGlyph ].gmBlackBoxX;

		// See if this width fits in the current row.
		int nNewXOffset = nXOffset + nCharWidth + kCharSpacing;
		if( nNewXOffset < sizeTexture.cx )
		{
			// Still fits in the current row.
			nXOffset = nNewXOffset;
		}
		else
		{
			// Doesn't fit in the current row.  Englarge by one row
			// and start at the left again.
			nXOffset = 0;
			nRawHeight += sizeMaxGlyphSize.cy + kCharSpacing;
		}
	}

	// Enlarge the height to the nearest power of two and use that as our final height.
	sizeTexture.cy = GetPowerOfTwo( nRawHeight );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CreateGlyphBitmap
//
//  PURPOSE:	Creates the bitmap to store individual glyphs.
//
//	nGlyphWidth -	Width of glyph bitmap.
//
//	nGlyphHeight -	Height of glyph bitmap.
//
//	bmi	-			Bitmap info to fill in.
//
//	pDibBits -		Pointer to bitmap bits to fill in.
//
//	hBitmap -		Bitmap handle to fill in.
//
// ----------------------------------------------------------------------- //

static bool CreateGlyphBitmap( HDC hDC, int nGlyphWidth, int nGlyphHeight, BITMAPINFO& bmi, uint8*& pDibBits,
					   HBITMAP& hBitmap )
{
	// Create a bitmap to hold all the font glyphs.
	memset( &bmi, 0, sizeof( bmi ));
	bmi.bmiHeader.biSize         = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth        = nGlyphWidth;
	// Use negative height since dibs are stored upside down.
	bmi.bmiHeader.biHeight       = -( int )nGlyphHeight;
	bmi.bmiHeader.biBitCount     = 24;
	bmi.bmiHeader.biPlanes       = 1;
	bmi.bmiHeader.biCompression  = BI_RGB;
	bmi.bmiHeader.biSizeImage	 = WIDTHBYTES( 24, nGlyphWidth ) * nGlyphHeight;

	hBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS, ( void** )&pDibBits, LTNULL, 0 );
	if( !hBitmap )
	{
		DEBUG_PRINT( 1, ( "CreateGlyphBitmap: Could not create bitmap." ));
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CopyGlyphBitmapToPixelData
//
//  PURPOSE:	Copies glphy bitmap to region of pixel data.
//
//	bmi	-			Bitmap info of source.
//
//	pDibBits -		Bitmap bits of glyph.
//
//	glyphMetrics -	Glyph metrics of this glyph.
//
//	pPixelData -	Pointer to beginning of pixel data region.
//
//	nPixelDataPitch -	Pitch of pixel data.
//
//	nGlyphWidth -	Width of glyph pixels.
//
//	nGlyphHeight -	Height of glyph pixels.
//
// ----------------------------------------------------------------------- //

static void CopyGlyphBitmapToPixelData( BITMAPINFO const& bmi, uint8 const* pDibBits,
									   GLYPHMETRICS const& glyphMetrics,
									   TEXTMETRIC const& textMetric,
									   uint8* pPixelData, SIZE const& sizeTexture )
{
	int nBitmapPitch = WIDTHBYTES( bmi.bmiHeader.biBitCount, bmi.bmiHeader.biWidth );
	int nPixelDataPitch = WIDTHBYTES( 16, sizeTexture.cx );

	// Calculate the position of the glyph within the source bitmap.
	RECT rectSrc;
	rectSrc.left = 0;
	rectSrc.top = 0;
	rectSrc.right = rectSrc.left + glyphMetrics.gmBlackBoxX;
	rectSrc.bottom = rectSrc.top + glyphMetrics.gmBlackBoxY;

	int nDestYOffset = textMetric.tmAscent - glyphMetrics.gmptGlyphOrigin.y;

	// Copy the glyph pixels to the pixeldata.  Leave the rgb of the pixeldata set to white.
	// This lets the font antialias with any color background.
	for( int y = rectSrc.top; y < rectSrc.bottom; y++ )
	{
		// Find out where to start this row by offset by y using the passed in bitmap pitch.
		// We only need to copy the glyph pixels, so we advance into the bitmap to
		// where they start.  The source is 24 bpp.
		uint8 const* pSrcPos = pDibBits + ( y * nBitmapPitch ) + ( rectSrc.left * 3 );

		// The pixeldata pointer will be pointing to some region in
		// the middle of a larger bitmap.  So, we use the pixeldatapitch
		// passed in to calculate how much to advance per line.  We don't want
		// to write out any columns to the left or right of the actual glyph.
		uint16* pDestPos = ( uint16* )( pPixelData + (( y + nDestYOffset ) * nPixelDataPitch ));

		// Copy this row.
		for( int x = 0; x < static_cast<int>(glyphMetrics.gmBlackBoxX); x++ )
		{
			// The source is 24 bits.  Just take r and use it as the alpha value.
			uint32 nVal = MulDiv( pSrcPos[0], 15, 255 );
			*pDestPos = ( *pDestPos & 0x0FFF ) | (( nVal & 0x0F ) << 12 );

			pDestPos++;
			pSrcPos += 3;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WritePixelDataBitmap
//
//  PURPOSE:	Writes pixel data texture to a bitmap file.  For debugging.
//
//	pPixelData -	pixel data bits.
//
//	sizeTexture -	size of pixel data.
//
//	pszFilename -	Filename to use.
//
// ----------------------------------------------------------------------- //

static void WritePixelDataBitmap( uint8* pPixelData, SIZE& sizeTexture, char const* pszFilename )
{
	int nPixelDataPitch = WIDTHBYTES( 16, sizeTexture.cx );
	int nBitmapPitch = WIDTHBYTES( 16, sizeTexture.cx );

	// Create a bitmap to hold all the font glyphs.
	BITMAPINFO bmi;
	uint8* pDibBits = new uint8[ nBitmapPitch * sizeTexture.cy ];
	memset( pDibBits, 0, nBitmapPitch * sizeTexture.cy );
	memset( &bmi, 0, sizeof( bmi ));
	bmi.bmiHeader.biSize         = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth        = sizeTexture.cx;
	// Use negative height since dibs are stored upside down.
	bmi.bmiHeader.biHeight       = -( int )sizeTexture.cy;
	bmi.bmiHeader.biBitCount     = 16;
	bmi.bmiHeader.biPlanes       = 1;
	bmi.bmiHeader.biCompression  = BI_RGB;

	for( int y = 0; y < sizeTexture.cy; y++ )
	{
		uint16 const* pSrcPos = ( uint16* )( pPixelData + y * nPixelDataPitch );
		uint16* pDestPos = ( uint16* )( pDibBits + ( y * nBitmapPitch ));

		for( int x = 0; x < sizeTexture.cx; x++ )
		{
			if ( pSrcPos[0] == 0x00F0 )
			{
				pDestPos[0] = 0x83E0;
			}
			else
			{

				// Get the alpha value and scale it into 5 bits.
				uint32 nVal = (( pSrcPos[0] & 0xF000 ) >> 12 );

				nVal = MulDiv( nVal, 0x1F, 0x0F );

				// Set the rgb values to the alpha value.
				pDestPos[0] = ( uint16 )(( nVal ) | ( nVal << 5 ) | ( nVal << 10 ));
			}
			pDestPos++;
			pSrcPos++;
		}
	}

	WriteBmp( pszFilename, bmi, pDibBits );

	delete[] pDibBits;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WritePixelDataTGA
//
//  PURPOSE:	Writes pixel data texture to a TGA file.  For texture bitmaps
//
//	pPixelData -	pixel data bits sixteen bit ARGB pixels. 
//
//	sizeTexture -	size of pixel data.
//
//	pszFilename -	Filename to use.
//
// ----------------------------------------------------------------------- //

static void WritePixelDataTGA( uint8* pPixelData, SIZE& sizeTexture, char const* pszFilename )
{

	// I don't like to do this but ...
	// TGA header.
#pragma pack(1)
    class TGAHeader
    {
    public:
        uint8   m_IDLength;
        uint8   m_ColorMapType;
        uint8   m_ImageType;
        uint16  m_CMapStart;
        uint16  m_CMapLength;
        uint8   m_CMapDepth;
        uint16  m_XOffset;
        uint16  m_YOffset;
        uint16  m_Width;
        uint16  m_Height;
        uint8   m_PixelDepth;
        uint8   m_ImageDescriptor;
    };
#pragma pack()


	TGAHeader hdr;

	int nPixelDataPitch = WIDTHBYTES( 16, sizeTexture.cx );
	int nBitmapPitch = WIDTHBYTES( 16, sizeTexture.cx );


	// Save the bitmap.
	FILE* fp = fopen( pszFilename, "wb");
	if (!fp)
	{
		return;
	}


	// Fill in TGA header.
	memset(&hdr, 0, sizeof(hdr));
	hdr.m_Width = (WORD)sizeTexture.cx;
	hdr.m_Height = (WORD)sizeTexture.cy;
	hdr.m_PixelDepth = 32;
	hdr.m_ImageType = 2;
	hdr.m_ImageDescriptor = (1<<5);  // says image is flipped origin 0,1
 
	fwrite(&hdr, sizeof(hdr), 1, fp);

	for( int y = 0; y < sizeTexture.cy; y++ )
	{
		uint16 const* pSrcPos = ( uint16* )( pPixelData + y * nPixelDataPitch );
		uint32 Dest;

		for( int x = 0; x < sizeTexture.cx; x++ )
		{
			// Special for green dot 
			if ( pSrcPos[0] == 0x00F0 )
			{
				Dest = 0x0000FF00;
			}
			else
			{

				// Get the alpha value and scale it into 5 bits.
				uint32 nVal = (( pSrcPos[0] & 0xF000 ) >> 12 );

				nVal = MulDiv( nVal, 0xFF, 0x0F );
				
				if ( nVal )
				{
					Dest = (nVal << 24 ) | 0x00FFFFFF;
				}
				else
				{
					Dest = 0;  // no pixel here 
				}

			}

			// write out the pixel

			fwrite ( &Dest , sizeof(Dest), 1, fp );


			pSrcPos++;
		}
	}

	fclose(fp);


}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	FixedFromFloat
//
//  PURPOSE:	Converts floating point to FIXED value.
//
//	hDC			- Device context.
//
//	pszChars	- Characters in font.
//
//	nLen		- Number of chars in pszChars.
//
//	pGlyphSizes	- Array of glyph sizes.  Must be nLen long.
//
//	sizMaxGlyphSize - Maximum size of pGlyphSizes
//
// ----------------------------------------------------------------------- //

FIXED FixedFromFloat(double d)
{
    long l;

    l = (long) (d * 65536L);
    return *(FIXED *)&l;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GetGlyphSizes
//
//  PURPOSE:	Get the glyph sizes for a font.
//
//	hDC			- Device context.
//
//	pszChars	- Characters in font.
//
//	nLen		- Number of chars in pszChars.
//
//	pGlyphSizes	- Array of glyph sizes.  Must be nLen long.
//
//	sizMaxGlyphSize - Maximum size of pGlyphSizes
//
// ----------------------------------------------------------------------- //

static bool GetGlyphSizes( HDC hDC, char const* pszChars, int nLen, TEXTMETRIC const& textMetric,
						  GLYPHMETRICS* pGlyphMetrics, SIZE& sizeMaxGlyphSize )
{
	// Setup the transform for the glyph.  Just use identity.
	MAT2 mat;
	mat.eM11 = FixedFromFloat( 1.0f );
	mat.eM12 = FixedFromFloat( 0.0f );
	mat.eM21 = FixedFromFloat( 0.0f );
	mat.eM22 = FixedFromFloat( 1.0f );

	// Get the individual widths of all chars in font, indexed by character values.
	memset( pGlyphMetrics, 0, sizeof( GLYPHMETRICS ) * nLen );
	sizeMaxGlyphSize.cx = sizeMaxGlyphSize.cy = 0;
	for( int nGlyph = 0; nGlyph < nLen; nGlyph++ )
	{
		// Get the character for this glyph.
		char nChar = pszChars[ nGlyph ];
		GLYPHMETRICS& glyphMetrics = pGlyphMetrics[ nGlyph ];

		uint32 uChar = *(( uint8* )&nChar );

		// Get the glyph metrics for this glyph.
		if( GDI_ERROR == GetGlyphOutline( hDC, uChar, GGO_METRICS, &glyphMetrics,
			0, NULL, &mat ))
		{
			// Use the default character on anything that has issues.
			if( GDI_ERROR == GetGlyphOutline( hDC, textMetric.tmDefaultChar, GGO_METRICS, &glyphMetrics,
				0, NULL, &mat ))
			return false;
		}

		sizeMaxGlyphSize.cx = Max( sizeMaxGlyphSize.cx, ( long )glyphMetrics.gmBlackBoxX );

		// The glyph bitmap will be offset into the texture character slot in the
		// y direction.  The maximum it will take up in the font texture needs to include
		// the amount it's offset.
		sizeMaxGlyphSize.cy = Max( sizeMaxGlyphSize.cy, ( long )( textMetric.tmAscent -
			glyphMetrics.gmptGlyphOrigin.y + glyphMetrics.gmBlackBoxY ));
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CUIVectorFont::CreateFontTextureAndTable
//
//  PURPOSE:	Creates font pixel data and mapping and size tables.
//
//	installedFontFace - Font face to use.
//
//	pszChars -			Chars to use in font.
//
//	bMakeMap -			Make a character map.
//
// ----------------------------------------------------------------------- //

bool CUIVectorFont::CreateFontTextureAndTable( InstalledFontFace& installedFontFace,
											  char const* pszChars, bool bMakeMap )
{
	bool bOk = true;

	// sanity check
	if (!pTexInterface)
	{
		DEBUG_PRINT( 1, ( "CUIVectorFont::CreateFontTextureAndTable: No Interface" ));
		return false;
	}

	// Check inputs.
	if( !pszChars || !pszChars[0] )
	{
		DEBUG_PRINT( 1, ("CUIVectorFont::CreateFontTextureAndTable:  Invalid parameters" ));
		return false;
	}

	// Get the font to use.
	HFONT hFont = installedFontFace.GetHFont( );
	if( !hFont )
	{
		DEBUG_PRINT( 1, ("CUIVectorFont::CreateFontTextureAndTable:  Invalid font." ));
		return false;
	}

	// Create a DC.
	HDC hDC = CreateCompatibleDC( NULL );
	if( !hDC )
	{
		DEBUG_PRINT( 1, ("CUIVectorFont::CreateFontTextureAndTable:  Couldn't create DC." ));
		return false;
	}

	// Get the number of characters to put in font.
	int nLen = strlen( pszChars );

	// These will hold the info on the bitmap.
	HBITMAP hBitmap = NULL;
	uint8* pDibBits = NULL;
	BITMAPINFO bmi;
	int nBufferSize = 0;

	// This will hold the size of the texture used for rendering.
	SIZE sizeTexture;

	// This will hold the sizes of each glyph in the font.  Index 0 of aGlyphSizes is for index 0 of pszChars.
	GLYPHMETRICS aGlyphMetrics[256];
	SIZE sizeMaxGlyphSize;

	// This will hold the GDI font metrics.
	TEXTMETRIC textMetric;

	if( bOk )
	{
		// Create the bitmap to hold the glyph.  We have to create the bitmap before we
		// get any font dimensions.  It seems that GDI needs to have a bitmap selected
		// to do any GetTextExtentPoint32 or GetTextMetrics calls.  So, we have to guess
		// at the size of the bitmap.  We only need one glyph in at a time, so we'll make
		// the bitmap twice the height for the width and the height.
		bOk = CreateGlyphBitmap( hDC, installedFontFace.GetHeight( ) * 2, installedFontFace.GetHeight( ) * 2,
			bmi, pDibBits, hBitmap );
		if( !bOk )
		{
			DEBUG_PRINT( 1, ("CUIVectorFont::CreateFontTextureAndTable:  Failed to create glyph bitmap." ));
		}
	}

	if( bOk )
	{
		// The font must be selected into the dc before the bitmap, because Win98 will refuse
		// to anti-alias without the font selected first.
		SelectObject( hDC, hFont );

		// Select the bitmap into the context.
		SelectObject( hDC, hBitmap );

		// Setup the dc.
		SetMapMode( hDC, MM_TEXT );
		SetTextColor( hDC, RGB( 255, 255, 255 ));
		SetBkColor( hDC, RGB( 0, 0, 0 ));
		SetBkMode( hDC, TRANSPARENT );

		// Get the metric info on the font.
		GetTextMetrics( hDC, &textMetric );

		// Get the sizes of each glyph.
		bOk = GetGlyphSizes( hDC, pszChars, nLen, textMetric, aGlyphMetrics, sizeMaxGlyphSize );
	}

	if( bOk )
	{

		// Get the size of the default character.
		SIZE sizeTextExtent;
		GetTextExtentPoint32( hDC, " ", 1, &sizeTextExtent );
		m_DefaultCharScreenWidth 	= static_cast<uint8>(sizeTextExtent.cx);
		m_DefaultCharScreenHeight 	= static_cast<uint8>(sizeTextExtent.cy);
		m_DefaultVerticalSpacing	= ( uint32 )((( float )m_DefaultCharScreenHeight / 4.0f ) + 0.5f );

		// Get the average info on the characters.  The width isn't used
		// for proportional fonts, so using an average is ok.
		m_CharTexWidth = ( uint8 )textMetric.tmAveCharWidth;
		m_CharTexHeight = static_cast<uint8>(textMetric.tmHeight);

		// Get the size our font texture should be to hold all the characters.
		GetTextureSizeFromCharSizes( aGlyphMetrics, sizeMaxGlyphSize, nLen, sizeTexture );

		// allocate the font table.  This contains the texture offsets for
		// the characters.
		LT_MEM_TRACK_ALLOC(m_pFontTable = new uint16[nLen * 3],LT_MEM_TYPE_UI);
		bOk = m_bAllocatedTable = ( m_pFontTable != NULL );
	}

	if( bOk )
	{
		// allocate the font map.  This contains the character mappings.
		if (bMakeMap)
		{
			LT_MEM_TRACK_ALLOC(m_pFontMap = new uint8[256],LT_MEM_TYPE_UI);
			bOk = m_bAllocatedMap = ( m_pFontMap != NULL );
		}
	}

	// This will be filled in with the pixel data of the font.
	uint8* pPixelData = NULL;

	// Calculate the pixeldata pitch.
	int nPixelDataPitch = WIDTHBYTES( 16, sizeTexture.cx );
	int nPixelDataSize = nPixelDataPitch * sizeTexture.cy;

	if( bOk )
	{
		// Allocate an array to copy the font into.
		LT_MEM_TRACK_ALLOC( pPixelData = new uint8[ nPixelDataSize ],LT_MEM_TYPE_UI );
		bOk = ( pPixelData != NULL );
		if( !bOk )
			DEBUG_PRINT( 1, ("CUIVectorFont::CreateFontTextureAndTable:  Failed to create pixeldata." ));
	}

	if( bOk )
	{
		// set the whole font texture to pure white, with alpha of 0.  When
		// we copy the glyph from the bitmap to the pixeldata, we just
		// affect the alpha, which allows the font to antialias with any color.
		uint16* pData = ( uint16* )pPixelData;
		uint16* pPixelDataEnd = ( uint16* )( pPixelData + nPixelDataSize );
		while( pData < pPixelDataEnd )
		{
			pData[0] = 0x0FFF;
			pData++;
		}

		// This will hold the UV offset for the font texture.
		POINT sizeOffset;
		sizeOffset.x = sizeOffset.y = 0;


		// Iterate over the characters.
		for( int nGlyph = 0; nGlyph < nLen; nGlyph++ )
		{
			// Clear the bitmap out for this glyph if it's not the first.  The first glyph
			// gets a brand spankin new bitmap to write on.
			if( nGlyph != 0 )
			{
				memset( pDibBits, 0, bmi.bmiHeader.biSizeImage );
			}


			// Get this character's width.
			char nChar = pszChars[nGlyph];
			GLYPHMETRICS& glyphMetrics = aGlyphMetrics[ nGlyph ];
			int nCharWidthWithSpacing = glyphMetrics.gmBlackBoxX + kCharSpacing;

			// See if this width fits in the current row.
			int nCharRightSide = sizeOffset.x + nCharWidthWithSpacing;
			if( nCharRightSide >= sizeTexture.cx )
			{
				// Doesn't fit in the current row.  Go to the next row.
				sizeOffset.x = 0;
				sizeOffset.y += sizeMaxGlyphSize.cy + kCharSpacing;
			}

			// Write the glyph out so that the smallest box around the glyph starts
			// at the bitmap's 0,0.
			POINT ptTextOutOffset;
			ptTextOutOffset.x = -glyphMetrics.gmptGlyphOrigin.x;
			ptTextOutOffset.y = -( textMetric.tmAscent - glyphMetrics.gmptGlyphOrigin.y );

			// Write out the glyph.  We can't use GetGlyphOutline to get the bitmap since
			// it has a lot of corruption bugs with it.
			if( !TextOut( hDC, ptTextOutOffset.x, ptTextOutOffset.y, &nChar, 1 ))
			{
				bOk = false;
				break;
			}

			// Make sure the GDI is done with our bitmap.
			GdiFlush( );

			// Fill in the font character map if we have one.
			if( m_pFontMap )
				m_pFontMap[( uint8 )nChar ] = nGlyph;

			// Char width.
			m_pFontTable[ nGlyph * 3 ]  = nCharWidthWithSpacing;

			// X Offset.
			m_pFontTable[ nGlyph * 3 + 1] = ( uint16 )sizeOffset.x;

			// Y Offset.
			m_pFontTable[ nGlyph * 3 + 2] = (uint16)sizeOffset.y;


			// Find pointer to region within the pixel data to copy the glyph
			// and copy the glyph into the pixeldata.
			uint8* pPixelDataRegion = pPixelData + ( sizeOffset.y * nPixelDataPitch ) + ( sizeOffset.x * 2 );
			CopyGlyphBitmapToPixelData( bmi, pDibBits, glyphMetrics, textMetric, pPixelDataRegion, sizeTexture );

			// Update to the next offset for the next character.
			sizeOffset.x += nCharWidthWithSpacing;
		}



		// See if we should write out this bitmap.
		bool bWriteFontBitmap = false;
		HCONSOLEVAR hConVar = ilt_client->GetConsoleVar( "WriteFontBitmap" );
		if( hConVar )
		{
			bWriteFontBitmap = (( int )ilt_client->GetVarValueFloat( hConVar ) != 0 );
		}



		// See if we should write green dots for inputing a bitmap.
		bool bGreenDot = false;
		HCONSOLEVAR hConVar2 = ilt_client->GetConsoleVar( "GreenDot" );
		if( hConVar2 )
		{
			bGreenDot = (( int )ilt_client->GetVarValueFloat( hConVar2 ) != 0 );
		}


		// See if we should write out a TGA font
		bool bWriteFontTGA = false;
		HCONSOLEVAR hConVar3 = ilt_client->GetConsoleVar( "WriteFontTGA" );
		if( hConVar3 )
		{
			bWriteFontTGA = (( int )ilt_client->GetVarValueFloat( hConVar3 ) != 0 );
		}



		// Do they want to see green dots ?
		if ( (bWriteFontBitmap || bWriteFontTGA) && bGreenDot )
		{


			// Iterate over the characters.
			for( int nGlyph = 0; nGlyph < nLen; nGlyph++ )
			{
				uint32 w;
				uint32 x;
				uint32 y;

				// Get this character's width.
				char nChar = pszChars[nGlyph];

				// Fill in the font character map if we have one.
				if( m_pFontMap )
				{
					m_pFontMap[( uint8 )nChar ] = nGlyph;
				}

				// Char width.
				w = m_pFontTable[ nGlyph * 3 ];

				// X Offset.
				x = m_pFontTable[ nGlyph * 3 + 1];

				// Y Offset.
				y = m_pFontTable[ nGlyph * 3 + 2];

				y += sizeMaxGlyphSize.cy + kCharSpacing - 1;
				x += w - 1;

				uint8 * pPixel = pPixelData + ( y * nPixelDataPitch ) + ( x * 2);

				uint16* pData = ( uint16* )pPixel;
				uint16* pPixelDataEnd = ( uint16* )( pPixelData + nPixelDataSize );

				if ( pData < pPixelDataEnd )
				{
						*pData = 0x00F0;    // Set green pixel ( pixel format is ARGB ) 
				}

			}	
		}



		// Write BMP
		if ( bWriteFontBitmap )
		{
			char szTextFace[_MAX_PATH];
			GetTextFace( hDC, sizeof( szTextFace ), szTextFace );
			char szFileName[_MAX_PATH];
			sprintf( szFileName, "Font_%s.bmp", szTextFace );

			WritePixelDataBitmap( pPixelData, sizeTexture, szFileName );
		}



		// Write TGA
		if ( bWriteFontTGA )
		{
			char szTextFace[_MAX_PATH];
			GetTextFace( hDC, sizeof( szTextFace ), szTextFace );
			char szFileName[_MAX_PATH];
			sprintf( szFileName, "Font_%s.tga", szTextFace );

			WritePixelDataTGA( pPixelData, sizeTexture, szFileName );
		}

		
	}  // end if bOk

	if( bOk )
	{
		// turn pixeldata into a texture
		pTexInterface->CreateTextureFromData(
				m_Texture, 
				TEXTURETYPE_ARGB4444,
				TEXTUREFLAG_PREFER16BIT | TEXTUREFLAG_PREFER4444,
				pPixelData, 
				sizeTexture.cx,
				sizeTexture.cy );
		if( !m_Texture )
		{
			DEBUG_PRINT( 1, ("CUIVectorFont::CreateFontTextureAndTable:  Couldn't create texture." ));
			bOk = false;
		}
	}

	// Don't need pixel data any more.
	if( pPixelData )
	{
		delete[] pPixelData;
		pPixelData = NULL;

	}

	// Don't need bitmap anymore.
	if( hBitmap )
	{
		DeleteObject( hBitmap );
		hBitmap = NULL;
	}

	// Don't nee font anymore.
	if( hFont )
	{
		DeleteObject( hFont );
		hFont = NULL;
	}

	// Don't need dc anymore.
	if( hDC )
	{
		DeleteDC( hDC );
		hDC = NULL;
	}

	// Clean up if we had an error.
	if( !bOk )
	{
		if( m_pFontTable )
		{
			delete[] m_pFontTable;
			m_pFontTable = NULL;
		}

		if( m_pFontMap )
		{
			delete[] m_pFontMap;
			m_pFontMap = NULL;
		}

		if( m_Texture )
		{
			pTexInterface->ReleaseTextureHandle( m_Texture );
			m_Texture = NULL;
		}
	}

	return bOk;
}
