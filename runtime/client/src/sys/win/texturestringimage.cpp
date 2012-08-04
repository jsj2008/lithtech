#include "bdefs.h"
#include "texturestringimage.h"
#include "d3d_texture.h"
#include <algorithm>

//----------------------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------------------

//the category that all allocations should go into
#define MEMORY_CATEGORY			LT_MEM_TYPE_TEXTURE

//the maximum texture size that we can use
#define MAXIMUM_TEXTURE_SIZE	1024

//an object bank for the texture string images
static ObjectBank<CTextureStringImage> g_TexStringImageBank(64, 64);



//----------------------------------------------------------------------------------------
// Interfaces
//----------------------------------------------------------------------------------------

// get the ILTTexInterface from the interface database
static ILTTexInterface *g_pILTTextureMgr = NULL;
define_holder(ILTTexInterface, g_pILTTextureMgr);



//----------------------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------------------

//finds the byte width of a bitmap
#define WIDTHBYTES( nBitCount, nPixelWidth ) (((( uint32 )nBitCount * nPixelWidth + 7) / 8 + 3) & ~3 )



//given a font info structure, this will convert it to an actual logical font that can be
//used for rendering and other tasks
static HFONT CreateHFont(const CFontInfo& FontInfo)
{
	//clear out the logical font structure first
	LOGFONTW LogFont;
	memset( &LogFont, 0, sizeof( LogFont ));

	//now fill in each property
	LogFont.lfHeight = FontInfo.m_nHeight;

	//setup the weight to be either normal or bold
	LogFont.lfWeight = FontInfo.IsStyleSet(CFontInfo::kStyle_Bold) ? FW_BOLD : FW_NORMAL;

	//setup any additional styles such as italics, underline, etc.
	LogFont.lfItalic = FontInfo.IsStyleSet(CFontInfo::kStyle_Italic);

	//now just some standard flags for truetype fonts
	LogFont.lfCharSet				= FontInfo.m_lfCharSet;  // default is machines charset
	LogFont.lfOutPrecision		= OUT_TT_PRECIS;
	LogFont.lfClipPrecision		= CLIP_DEFAULT_PRECIS;
	LogFont.lfQuality				= ANTIALIASED_QUALITY;
	LogFont.lfPitchAndFamily	= DEFAULT_PITCH;

	//and finally copy over the typeface name
	LTStrCpy( LogFont.lfFaceName, FontInfo.m_szTypeface, LTARRAYSIZE(LogFont.lfFaceName) );

	return ::CreateFontIndirectW(&LogFont);
}

//utility to create a bitmap that can be used by the passed in device context for rendering
//the font characters. The returned bitmap should be freed by the caller
static bool CreateGlyphBitmap( HDC hDC, uint32 nWidth, uint32 nHeight, BITMAPINFO& bmi, uint8*& pDibBits, HBITMAP& hBitmap )
{
	// Create a bitmap to hold all the font glyphs.
	memset( &bmi, 0, sizeof( bmi ));
	bmi.bmiHeader.biSize         = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth        = nWidth;

	// Use negative height since dibs are stored upside down.
	bmi.bmiHeader.biHeight       = -(int32)nHeight;
	bmi.bmiHeader.biBitCount     = 24;
	bmi.bmiHeader.biPlanes       = 1;
	bmi.bmiHeader.biCompression  = BI_RGB;
	bmi.bmiHeader.biSizeImage	 = WIDTHBYTES( 24, nWidth ) * nHeight;

	hBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS, ( void** )&pDibBits, NULL, 0 );
	if( !hBitmap )
	{
		DEBUG_PRINT( 1, ( "CreateGlyphBitmap: Could not create bitmap." ));
		return false;
	}

	return true;
}

//handles freeing all of the device objects at once in the event of a failure or shutdown
static void ReleaseWindowsObjects(HDC hDC, HFONT hFont, HBITMAP hBitmap)
{
	if(hFont)
		DeleteObject(hFont);
	if(hBitmap)
		DeleteObject(hBitmap);
	if(hDC)
		DeleteDC(hDC);
}

//handles creating all device objects that are needed for the creation of a texture image. This includes
//the handle to the device context, the handle to the font, and the data associated with the bitmap
static bool	CreateWindowObjects(const CFontInfo& FontInfo, HDC& hDC, TEXTMETRICW& TextMetrics, 
								HFONT& hFont, BITMAPINFO& bmi, uint8*& pBitmapImage, HBITMAP& hBitmap)
{
	//default all the hanldes in case of failure
	hDC		= NULL;
	hFont	= NULL;
	hBitmap = NULL;

	//create the font from the provided font
	hFont = CreateHFont(FontInfo);
	if(!hFont)
	{
		ReleaseWindowsObjects(hDC, hFont, hBitmap);
		return false;
	}

	//create the device context
	hDC = CreateCompatibleDC(NULL);
	if(!hDC)
	{
		ReleaseWindowsObjects(hDC, hFont, hBitmap);
		return false;
	}

	// The font must be selected into the dc before the bitmap, because Win98 will refuse
	// to anti-alias without the font selected first.
	SelectObject( hDC, hFont );

	// Setup the dc.
	SetMapMode( hDC, MM_TEXT );
	SetTextColor( hDC, RGB( 255, 255, 255 ));
	SetBkColor( hDC, RGB( 0, 0, 0 ));
	SetBkMode( hDC, TRANSPARENT );

	//now obtain the text metrics since this has information on the widest character
	GetTextMetricsW(hDC, &TextMetrics);

	//create a bitmap to encompass all the characters we will be rendering
	if(!CreateGlyphBitmap(hDC, TextMetrics.tmMaxCharWidth, FontInfo.m_nHeight, bmi, pBitmapImage, hBitmap))
	{
		ReleaseWindowsObjects(hDC, hFont, hBitmap);
		return false;
	}

	// Select the bitmap into the context.
	SelectObject( hDC, hBitmap );

	//success
	return true;
}

//given a string and a buffer, this will fill the buffer with all the unique letters in the
//string
static void GetUniqueCharList(const wchar_t* pszString, wchar_t* pszBuffer, uint32 nBufferSize)
{
	//bail on an empty bufer
	if(nBufferSize == 0)
		return;

	LTStrCpy(pszBuffer, pszString, nBufferSize);
	pszBuffer[nBufferSize - 1] = (wchar_t)'\0';

	//determint the size of the string we will be modifying
	uint32 nStrLen = LTStrLen(pszBuffer);

	//only bother checking for duplicates if we have more than a single character
	if(nStrLen > 1)
	{
		//now sort the characters to place duplicates right next to each other
		std::sort(pszBuffer, pszBuffer + nStrLen);

		//the current end of the buffer
		uint32 nBufferEnd = nStrLen - 1;

		//now run through starting at the end, and if we find a duplicate, just move the last character over
		//us and bring in the end line
		for(uint32 nCurrChar = nBufferEnd; nCurrChar > 0; nCurrChar--)
		{
			//see if this is a duplicate character
			if(pszBuffer[nCurrChar] == pszBuffer[nCurrChar - 1])
			{
				//it is, move the last character over this one, and move the end string back
				pszBuffer[nCurrChar] = pszBuffer[nBufferEnd];
				pszBuffer[nBufferEnd] = '\0';
				nBufferEnd--;
			}
		}
	}

	//we now have the buffer filled with unique characters in only O(2N + lgN) time
}

//given a glyph that has the glyph character field filled in, this will fill in all of the dimension
//fields within that glyph that relate to the size and positioning of the glyph (except for the texture
//positioning).
static FIXED FixedFromFloat(double d)
{
    long l;

    l = (long) (d * 65536L);
    return *(FIXED *)&l;
}

static bool FillGlyphDimensions(HDC hDC, const TEXTMETRICW& TextMetric, CTextureStringGlyph* pGlyphs, uint32 nNumGlyphs)
{
	// Setup the transform for the glyph.  Just use identity.
	MAT2 mat;
	mat.eM11 = FixedFromFloat( 1.0f );
	mat.eM12 = FixedFromFloat( 0.0f );
	mat.eM21 = FixedFromFloat( 0.0f );
	mat.eM22 = FixedFromFloat( 1.0f );

	//determine if this is a truetype font or not
	bool bTrueType = (TextMetric.tmPitchAndFamily & TMPF_TRUETYPE) != 0;

	// Get the individual widths of all chars in font, indexed by character values.
	GLYPHMETRICS GlyphMetrics;
	memset( &GlyphMetrics, 0, sizeof( GlyphMetrics ) );

	for( uint32 nCurrGlyph = 0; nCurrGlyph < nNumGlyphs; nCurrGlyph++ )
	{
		// Get the character for this glyph.
		CTextureStringGlyph& Glyph = pGlyphs[nCurrGlyph];

		if(bTrueType)
		{
			// Get the glyph metrics for this glyph.
			if( GDI_ERROR == GetGlyphOutlineW( hDC, Glyph.m_cGlyph, GGO_METRICS, &GlyphMetrics, 0, NULL, &mat ))
			{
				// Use the default character on anything that has issues.
				if( GDI_ERROR == GetGlyphOutlineW( hDC, TextMetric.tmDefaultChar, GGO_METRICS, &GlyphMetrics, 0, NULL, &mat ))
				{
					return false;
				}
			}

			//we have the glyph outline, so we now need to convert the data from that over to the glyph data
			//format
			Glyph.m_nTotalWidth			= GlyphMetrics.gmCellIncX;

			Glyph.m_rBlackBox.Left()	= GlyphMetrics.gmptGlyphOrigin.x;
			Glyph.m_rBlackBox.Top()		= TextMetric.tmAscent - GlyphMetrics.gmptGlyphOrigin.y;

			Glyph.m_rBlackBox.Right()	= Glyph.m_rBlackBox.Left() + GlyphMetrics.gmBlackBoxX + 1;
			Glyph.m_rBlackBox.Bottom()	= Glyph.m_rBlackBox.Top()  + GlyphMetrics.gmBlackBoxY + 1;
		}
		else
		{
			//this is not a truetype font, so we have to use a different approach for getting the glyph
			//sizes
			INT nWidth = 0;
			if(!GetCharWidth32W(hDC, Glyph.m_cGlyph, Glyph.m_cGlyph, &nWidth))
			{
				if(!GetCharWidth32W(hDC, TextMetric.tmDefaultChar, TextMetric.tmDefaultChar, &nWidth))
				{
					return false;
				}
			}

			//setup our character from that information
			Glyph.m_nTotalWidth			= nWidth;

			Glyph.m_rBlackBox.Left()	= 0;
			Glyph.m_rBlackBox.Top()		= 0;

			Glyph.m_rBlackBox.Right()	= nWidth;
			Glyph.m_rBlackBox.Bottom()	= TextMetric.tmHeight;
		}
	}

	//success
	return true;
}

//determines the smallest power of two that encompasses this value
inline uint32 GetTextureSize( uint32 nValue )
{
	//default it to the minimum texture size
	uint32 nPowerOfTwo = 32;

	//and now while the size of the texture is too small (including the upper
	//boundary since the provided value ranges from 0..nValue inclusive) increase
	//our texture size by a power of two
	while( nPowerOfTwo <= nValue )
	{
		nPowerOfTwo *= 2;
	}

	return nPowerOfTwo;
}

//uses the glyphs and determines the size of the texture needed to lay them out with a sigle pixel boundary
//around each character
static bool GetTextureSizeFromCharSizes(const CTextureStringGlyph* pGlyphs, uint32 nNumGlyphs, uint32 nMaxGlyphWidth, 
										uint32 nMaxGlyphHeight, uint32 nCharSpacing, SIZE& sizeTexture )
{
	//default the size
	sizeTexture.cx = sizeTexture.cy = 0;

	// Start the height off as one row.
	uint32 nRawHeight = nMaxGlyphHeight + nCharSpacing;

	// To find the height, keep putting characters into the rows until we reach the bottom.
	int nXOffset	= 0;
	int nMaxXOffset = 0;

	for( uint32 nGlyph = 0; nGlyph < nNumGlyphs; nGlyph++ )
	{
		// Get this character's width.
		int nCharWidthWithSpacing = pGlyphs[nGlyph].m_rBlackBox.GetWidth() + nCharSpacing;

		// See if this width fits in the current row.
		if( nXOffset + nCharWidthWithSpacing < MAXIMUM_TEXTURE_SIZE )
		{
			// Still fits in the current row.
			nXOffset += nCharWidthWithSpacing;

			//keep track of the maximum extent
			nMaxXOffset = LTMAX(nMaxXOffset, nXOffset);
		}
		else
		{
			// Doesn't fit in the current row.  Englarge by one row
			// and start at the left again.
			nXOffset = 0;
			nRawHeight += nMaxGlyphHeight + nCharSpacing;

			//keep track of the maximum extent
			nMaxXOffset = LTMAX(nMaxXOffset, nCharWidthWithSpacing);
		}
	}

	//if the nMaxOffset extends past the maximum texture size, then we have a glyph that is too
	//large to fit on a texture
	if(nMaxXOffset >= MAXIMUM_TEXTURE_SIZE)
		return false;

	//also see if the whole string couldn't fit on the texture
	if(nRawHeight >= MAXIMUM_TEXTURE_SIZE)
		return false;

	//adjust the offset to be a texture size
	sizeTexture.cx = GetTextureSize( nMaxXOffset );

	// Enlarge the height to the nearest power of two and use that as our final height.
	sizeTexture.cy = GetTextureSize( nRawHeight );

	//otherwise the texture size is valid
	return true;
}

//handles copying the image data from the associated bitmap to the 16 bit texture provided
static void CopyGlyphBitmapToPixelData( BITMAPINFO const& bmi, uint8 const* pDibBits, 
									   const LTRect2n& rectDest,
									   TEXTMETRICW const& textMetric,
									   uint8* pPixelData, SIZE const& sizeTexture )
{
	//pitches of each surface
	const int nBitmapPitch = WIDTHBYTES( bmi.bmiHeader.biBitCount, bmi.bmiHeader.biWidth );
	const int nPixelDataPitch = WIDTHBYTES( 16, sizeTexture.cx );

	//pixel byte widths of the bitmap surface
	const int nBitmapPixelWidth = (bmi.bmiHeader.biBitCount + 7) / 8;

	//calculate the width
	const int nGlyphWidth = rectDest.GetWidth();

	//calcualte the height
	const int nGlyphHeight = rectDest.GetHeight();

	// Copy the glyph pixels to the pixeldata.  Leave the rgb of the pixeldata set to white.
	// This lets the font antialias with any color background.
	for( int y = 0; y < nGlyphHeight; y++ )
	{
		// Find out where to start this row by offset by y using the passed in bitmap pitch.
		// We only need to copy the glyph pixels, so we advance into the bitmap to
		// where they start.  The source is 24 bpp.
		const uint8* pSrcPos = pDibBits + ( y * nBitmapPitch );

		// The pixeldata pointer will be pointing to some region in 
		// the middle of a larger bitmap.  So, we use the pixeldatapitch
		// passed in to calculate how much to advance per line.  We don't want
		// to write out any columns to the left or right of the actual glyph.
		uint16* pDestPos = ( uint16* )( pPixelData + (( y + rectDest.Top() ) * nPixelDataPitch ) + (rectDest.Left() * 2) );

		// Copy this row.
		for( int x = 0; x < nGlyphWidth; x++ )
		{
			// The source is 24 bits.  Just take r and use it as the alpha value.
			uint32 nVal = MulDiv( pSrcPos[0], 15, 255 );
			*pDestPos = ( *pDestPos & 0x0FFF ) | (( nVal & 0x0F ) << 12 );

			pDestPos++;
			pSrcPos += nBitmapPixelWidth;
		}
	}
}

//handles actually creating the associated texture given the list of glyphs that have their positioning
//and dimension information filled out
static bool CreateGlyphTexture(CTextureStringGlyph* pGlyphs, uint32 nNumGlyphs, HDC hDC, 
							   uint8* pDibBits, const BITMAPINFO& bmi, const TEXTMETRICW& textMetric,
							   HTEXTURE & hTexture)
{
	//run through and build up the maximum extents from the glyph black boxes
	uint32 nMaxWidth = 0;
	uint32 nMaxHeight = 0;

	for(uint32 nCurrGlyph = 0; nCurrGlyph < nNumGlyphs; nCurrGlyph++)
	{
		nMaxWidth = LTMAX(nMaxWidth, pGlyphs[nCurrGlyph].m_rBlackBox.GetWidth());
		nMaxHeight = LTMAX(nMaxHeight, pGlyphs[nCurrGlyph].m_rBlackBox.GetHeight());
	}

	//the spacing added to each glyph dimension
	const uint32 knCharSpacing = 2;

	//determine the size of this texture that we will need
	SIZE sizeTexture;
	if(!GetTextureSizeFromCharSizes(pGlyphs, nNumGlyphs, nMaxWidth, nMaxHeight, knCharSpacing, sizeTexture ))
	{
		//the font is to big to fit into a texture, we must fail
		return false;
	}



	// This will be filled in with the pixel data of the font.
	uint8* pImageData = NULL;

	//lock down our texture for writing
//	uint32 nWidth, nHeight, 
	uint32 nPitch;
//	uint8* pImageData;

	// Calculate the pixeldata pitch.
	nPitch = WIDTHBYTES( 16, sizeTexture.cx );
	int nPixelDataSize = nPitch * sizeTexture.cy;


	// Allocate an array to copy the font into.
	LT_MEM_TRACK_ALLOC( pImageData = new uint8[ nPixelDataSize ],LT_MEM_TYPE_UI );
	if ( pImageData == NULL )
	{
		DEBUG_PRINT( 1, ("CreateGlyphTexture:  Failed to allocate pixeldata." ));
		return false;
	}


	// set the whole font texture to pure white, with alpha of 0.  When
	// we copy the glyph from the bitmap to the pixeldata, we just
	// affect the alpha, which allows the font to antialias with any color.
	uint16* pData = (uint16*)pImageData;
	uint16* pPixelDataEnd = (uint16*)(pImageData + nPixelDataSize);
	while( pData < pPixelDataEnd )
	{
		pData[0] = 0x0FFF;
		pData++;
	}

	// This will hold the UV offset for the font texture.
	POINT sizeOffset;
	sizeOffset.x = 0;
	sizeOffset.y = 0;

	//success flag
	bool bSuccess = true;

	// Iterate over the characters.
	for( uint32 nGlyph = 0; nGlyph < nNumGlyphs; nGlyph++ )
	{
		// Clear the bitmap out for this glyph if it's not the first.  The first glyph
		// gets a brand new bitmap to write on.
		if( nGlyph != 0 )
		{
			memset( pDibBits, 0, bmi.bmiHeader.biSizeImage );
		}

		//cache the glyph we will be operating on
		CTextureStringGlyph& Glyph = pGlyphs[nGlyph];

		// Get this character's width.
		wchar_t cChar = Glyph.m_cGlyph;
		int nCharWidthWithSpacing = Glyph.m_rBlackBox.GetWidth() + knCharSpacing;

		// See if this width fits in the current row.
		int nCharRightSide = sizeOffset.x + nCharWidthWithSpacing;
		if( nCharRightSide >= sizeTexture.cx )
		{
			// Doesn't fit in the current row.  Go to the next row.
			sizeOffset.x = 0;
			sizeOffset.y += nMaxHeight + knCharSpacing;
		}

		// Write the glyph out so that the smallest box around the glyph starts
		// at the bitmap's 0,0.
		POINT ptTextOutOffset;
		ptTextOutOffset.x = -Glyph.m_rBlackBox.Left();
		ptTextOutOffset.y = -Glyph.m_rBlackBox.Top();

		// Write out the glyph.  We can't use GetGlyphOutline to get the bitmap since
		// it has a lot of corruption bugs with it.  

		if( !TextOutW( hDC, ptTextOutOffset.x, ptTextOutOffset.y, &cChar, 1 ))
		{
			bSuccess = false;
			break;
		}

		// Make sure the GDI is done with our bitmap.
		GdiFlush( );

		LTRect2n rCopyTo;
		rCopyTo.Left()		= sizeOffset.x + (knCharSpacing / 2);
		rCopyTo.Top()		= sizeOffset.y + (knCharSpacing / 2);
		rCopyTo.Right()		= rCopyTo.Left() + Glyph.m_rBlackBox.GetWidth();
		rCopyTo.Bottom()	= rCopyTo.Top() + Glyph.m_rBlackBox.GetHeight();

		// Find pointer to region within the pixel data to copy the glyph
		// and copy the glyph into the pixeldata.
		CopyGlyphBitmapToPixelData( bmi, pDibBits, rCopyTo, textMetric, pImageData, sizeTexture );

		//setup the UV coordinates for this glyph
		Glyph.m_fU = (float)(rCopyTo.Left() + 0.5f) / (float)sizeTexture.cx;
		Glyph.m_fV = (float)(rCopyTo.Top() + 0.5f) / (float)sizeTexture.cy;
		Glyph.m_fTexWidth  = rCopyTo.GetWidth() / (float)sizeTexture.cx;
		Glyph.m_fTexHeight = rCopyTo.GetHeight() / (float)sizeTexture.cy;

		// Update to the next offset for the next character.
		sizeOffset.x += nCharWidthWithSpacing;
	}


	//if we succeeded in rendering all the characters, convert it to a texture
	if(bSuccess)
	{

		// turn pixeldata into a texture
		g_pILTTextureMgr->CreateTextureFromData(
				hTexture, 
				TEXTURETYPE_ARGB4444,
				TEXTUREFLAG_PREFER16BIT | TEXTUREFLAG_PREFER4444,
				pImageData, 
				sizeTexture.cx,
				sizeTexture.cy );

		if( !hTexture )
		{
			DEBUG_PRINT( 1, ("CreateGlyphTexture:  Couldn't create texture." ));
			bSuccess = false;
		}

	}

	// Don't need pixel data any more.
	if( pImageData )
	{
		delete[] pImageData;
		pImageData = NULL;
	}


	//return the success code
	return bSuccess;
}


//----------------------------------------------------------------------------------------
// CTextureStringImage
//----------------------------------------------------------------------------------------

CTextureStringImage::CTextureStringImage() :
	m_pGlyphList(NULL),
	m_nNumGlyphs(0),
	m_nRowHeight(0)
{
}

CTextureStringImage::~CTextureStringImage()
{
	FreeData();
}

//called to allocate a new texture string image object
CTextureStringImage* CTextureStringImage::Allocate()
{
	return g_TexStringImageBank.Allocate();
}

void CTextureStringImage::Free(CTextureStringImage* pImage)
{
	g_TexStringImageBank.Free(pImage);
}

//called to create a texture given a font and a string
bool CTextureStringImage::CreateBitmapFont(const wchar_t* pszString, const CFontInfo& Font)
{
	//clear up any previous data
	FreeData();

	// Create our windows objects
	HDC			hDC = NULL;
	HFONT		hFont = NULL;
	HBITMAP		hBitmap = NULL;
	uint8*		pBitmapImage = NULL;
	BITMAPINFO	bmi;
	TEXTMETRICW	TextMetric;

	if(!CreateWindowObjects(Font, hDC, TextMetric, hFont, bmi, pBitmapImage, hBitmap))
	{
		return false;
	}

	//first off create our unique glyph list
	if(!SetupUniqueGlyphList(pszString))
	{
		ReleaseWindowsObjects(hDC, hFont, hBitmap);
		return false;
	}

	//and now fill in the dimensions of those glyphs
	if(!FillGlyphDimensions(hDC, TextMetric, m_pGlyphList, m_nNumGlyphs))
	{
		ReleaseWindowsObjects(hDC, hFont, hBitmap);
		FreeData();
		return false;
	}

	//create the glyph texture
	if(!CreateGlyphTexture(m_pGlyphList, m_nNumGlyphs, hDC, pBitmapImage, bmi, TextMetric, m_hTexture))
	{
		ReleaseWindowsObjects(hDC, hFont, hBitmap);
		FreeData();
		return false;
	}

	//determine the height of a single row of text
	m_nRowHeight = TextMetric.tmHeight;

	//release our windows objects now that we are done with them
	ReleaseWindowsObjects(hDC, hFont, hBitmap);

	//success
	return true;
}

//frees all data associated with this object
void CTextureStringImage::FreeData()
{
	//free the glyph list
	delete [] m_pGlyphList;
	m_pGlyphList = NULL;
	m_nNumGlyphs = 0;
    
	//free the image
	m_hTexture = NULL;

	//clear out any data
	m_nRowHeight = 0;
}

//accesses a glyph in the list
const CTextureStringGlyph* CTextureStringImage::GetGlyphByIndex(uint32 nGlyph) const
{
	if(nGlyph < m_nNumGlyphs)
		return &m_pGlyphList[nGlyph];

	return NULL;
}

const CTextureStringGlyph* CTextureStringImage::GetGlyph(wchar_t cGlyph) const
{
	//find the glyph
	for(uint32 nCurrGlyph = 0; nCurrGlyph < m_nNumGlyphs; nCurrGlyph++)
	{
		if(m_pGlyphList[nCurrGlyph].m_cGlyph == cGlyph)
			return &m_pGlyphList[nCurrGlyph];
	}
	return NULL;
}

//------------------------------------------
// Creation utilities
//------------------------------------------

//called during the creation to extract all the unique glyphs from a string, allocate the
//glyph list, and set them up with the characters they reference
bool CTextureStringImage::SetupUniqueGlyphList(const wchar_t* pszString)
{
	//determine the length of the string
	uint32 nStrLen = LTStrLen(pszString);

	//the first thing to do is build up the unique character list
	wchar_t* pszUniqueList;
	LT_MEM_TRACK_ALLOC(pszUniqueList = new wchar_t[nStrLen + 1], MEMORY_CATEGORY);
	if(!pszUniqueList)
		return false;

	//get the unique character list
	GetUniqueCharList(pszString, pszUniqueList, nStrLen + 1);

	//determine the number of unique characters
	uint32 nNumGlyphs = LTStrLen(pszUniqueList);

	//now allocate our glyph list
	LT_MEM_TRACK_ALLOC(m_pGlyphList = new CTextureStringGlyph[nNumGlyphs], MEMORY_CATEGORY);

	//check the allocation
	if(!m_pGlyphList)
	{
		delete [] pszUniqueList;
		return false;
	}

	//now copy over the data
	m_nNumGlyphs = nNumGlyphs;

	for(uint32 nCurrGlyph = 0; nCurrGlyph < nNumGlyphs; nCurrGlyph++)
	{
		m_pGlyphList[nCurrGlyph].m_cGlyph = pszUniqueList[nCurrGlyph];
	}
	
	//free up the unique string list
	delete [] pszUniqueList;
	pszUniqueList = NULL;

	//and success
	return true;
}



