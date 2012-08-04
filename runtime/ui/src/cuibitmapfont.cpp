//-------------------------------------------------------------------
//
//   MODULE    : CUIBITMAPFONT.CPP
//
//   PURPOSE   : implements the CUIBitmapFont Font Class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __CUIBITMAPFONT_H__
#include "cuibitmapfont.h"
#endif

#ifndef __CUIPOLYSTRING_IMPL_H__
#include "cuipolystring_impl.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif


// get the ILTTexInterface from the interface database
ILTTexInterface* pTexInterface = NULL;
define_holder(ILTTexInterface, pTexInterface);

	
//	--------------------------------------------------------------------------
// create a proportional font with user-supplied font table
CUIBitmapFont::CUIBitmapFont(HTEXTURE hTex, 
							 uint16*  pTable, 
							 uint8    cheight,
							 uint8*   pMap) : CUIFont_Impl()
{
	m_Texture 					= hTex;
	m_pFontTable 				= pTable;
	m_pFontMap					= NULL;
	m_Proportional 				= true;
	m_DefaultCharScreenWidth 	= (uint8)pTable[0];
	m_DefaultCharScreenHeight	= cheight;	
	m_CharTexWidth 				= 0;
	m_CharTexHeight				= cheight;	
	m_DefaultVerticalSpacing	= m_DefaultCharScreenHeight / 4;
	m_Valid						= true;

	if(pMap)
	{
		m_pFontMap = pMap;
	}
	else if(!BuildDefaultMap())
	{
		CUI_ERR("Cannot create default font map.\n");
	}
}


// create a monospace font with user-supplied width & height
CUIBitmapFont::CUIBitmapFont(HTEXTURE hTex, 
							 uint8    cwidth, 
							 uint8    cheight,
							 uint8*   pMap) : CUIFont_Impl()
{
	m_Texture 					= hTex;
	m_pFontTable 				= NULL;
	m_pFontMap					= NULL;
	m_Proportional 				= false;
	m_DefaultCharScreenWidth 	= cwidth;
	m_DefaultCharScreenHeight 	= cheight;	
	m_CharTexWidth				= cwidth;
	m_CharTexHeight				= cheight;	
	m_DefaultVerticalSpacing	= m_DefaultCharScreenHeight / 4;
	m_Valid						= true;

	if(pMap)
	{
		m_pFontMap = pMap;
	}
	else if(!BuildDefaultMap())
	{
		CUI_ERR("Cannot create default font map.\n");
	}
}


// create a font from an HTEXTURE only.  the system will attempt
// to determine it's characteristics.
CUIBitmapFont::CUIBitmapFont(HTEXTURE hTex, 
							 uint8* pMap) : CUIFont_Impl()
{
	m_Texture 		= hTex;
	m_pFontTable 	= NULL;
	m_pFontMap		= NULL;
	m_Proportional 	= true;
	m_CharTexWidth	= 0;
	
	if(pMap)
	{
		m_pFontMap = pMap;
	}
	else if(!BuildDefaultMap())
	{
		CUI_ERR("Cannot create default font map.\n");
	}

	if ( !BuildFontTable() ) {
		CUI_ERR("Cannot create font table.  Invalid texture format?\n");
		m_DefaultCharScreenWidth	= 0;
		m_DefaultCharScreenHeight	= 0;
		m_pFontTable				= NULL;
		// font is not valid
	}
	else {		
		m_DefaultCharScreenHeight  = m_CharTexHeight;

		// !!!! THIS assumes the 36 index in the bitmap is the character 'E' ( used for whitespace etc ) 
		m_DefaultCharScreenWidth   = (uint8)m_pFontTable[36*3];	 
		m_DefaultVerticalSpacing   = m_DefaultCharScreenHeight / 4;

		// I must delete this table when the font is destroyed
		m_bAllocatedTable		   = true;
		m_Valid					   = true;
	}
}


CUIBitmapFont::~CUIBitmapFont()
{
	// Parent class will deallocate font resources (FontTable and FontMap)
}


//	--------------------------------------------------------------------------
bool CUIBitmapFont::BuildFontTable()
{
	// attempt to figure out the font table
	// currently, this will produce a proportional font, even from
	// a monospace input image
	
	const uint8		*pData;
	uint32 			nPitch;
	uint32			nWidth;
	uint32			nHeight;
	ETextureType 	type;

	if (!pTexInterface || !m_Texture) return false;
		
	// pData is a pointer to the actual texture data.  So I don't need
	// to free it (or flush it, since currently I do not modify it)
	if(pTexInterface->GetTextureData(m_Texture, pData, nPitch, nWidth, nHeight, type) != LT_OK)
		return false;
			
	if (!pData) return false;
	
	// ok, now I should have the font data in Platform-specific format
	// but this is ok, because the Green is in the same place, and that
	// will let me calculate the position of the characters
	
	// I don't particularly want to do the bitmap madness for all compression
	// types, so I'm going to convert it
	
	switch (type) 
	{
		
		case TEXTURETYPE_ARGB8888:		// PC Only
			// no changes
			break;
		
		case TEXTURETYPE_ARGB4444:		// PC Only
		case TEXTURETYPE_ARGB1555:		// PC Only
		case TEXTURETYPE_RGB565:		// PC Only
		case TEXTURETYPE_DXT1:			// PC Only
		case TEXTURETYPE_DXT3:			// PC Only
		case TEXTURETYPE_DXT5:			// PC Only
			

		case TEXTURETYPE_INVALID: 
		default:

			//unsupported types
			return false;
			break;
	}
			
	return CreatePropFont(pData, nWidth, nHeight, nPitch);
		
}


// ---------------------------------------------------------------------------
bool CUIBitmapFont::CreatePropFont(const uint8* pData, uint32 nWidth, uint32 nHeight, uint32 nPitch)
{
	uint32	x, y;
	uint8	curindex =  0;

	bool	opaque   = false;
	bool	green    = false;
	bool	reading  = false;

	uint32	u        = 0; 
	uint32 	v        = 0; 

	LT_MEM_TRACK_ALLOC(m_pFontTable = new uint16[CUI_MAX_BITMAPFONT_CHARS * 3],LT_MEM_TYPE_UI);
	if (!m_pFontTable) return false;

	memset(m_pFontTable, 0, CUI_MAX_BITMAPFONT_CHARS * 3 * 2);

	// first, find a line of text
	int32 ytop = 0;
	int32 ybot = 0;
	
	// max text height
	m_CharTexHeight = 0;

	// look for the green marker dots generated by fontmaker or from pc version which will write green dots
	
	// I use green (255) because it's in the same place in PS2
	// and PC pixels, and it's max value is constant (unlike alpha)
	// between all platforms.
	
	v = 0;
	curindex = 0;
	uint32* rgba;
	

	for (y=0; y < nHeight; y++) 
	{		 
		 
		u       = 0;
				
		rgba = (uint32*)(pData + y * nPitch);

		for (x=0; x < nWidth; x++) 
		{
			// Check for a full green pixel that does not contain any opacity ( alpha )
			if (rgba[x] == 0x0000FF00) 
			{				
				green = true;

				// try and set the max character height
				if ( (y - v - 1) > m_CharTexHeight)
					m_CharTexHeight = y - v - 1;

				m_pFontTable[curindex*3] = x - u + 1;
				m_pFontTable[curindex*3+1] = u; 
				m_pFontTable[curindex*3+2] = v; 

				u = x + 1;
				curindex++;

				if (curindex == CUI_MAX_BITMAPFONT_CHARS) 
				{
					CUI_ERR("Character Index is too high!\n");
					return false;
				}
			}
		}
		 
		if (green) 
		{
			v = y+1;
			green = false;			
		}
	}
	

	return true;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CUIBitmapFont::BuildDefaultMap
//
//	PURPOSE:	Creates a default font map used for ordering of characters 
//
// ----------------------------------------------------------------------- //
bool CUIBitmapFont::BuildDefaultMap()
{
	if (m_bAllocatedMap && m_pFontMap)
	{
		delete [] m_pFontMap;
		m_pFontMap = NULL;
	}

	LT_MEM_TRACK_ALLOC(m_pFontMap = new uint8[256],LT_MEM_TYPE_UI);
	if (!m_pFontMap)
	{
		return false;
	}

	// clear the other index so they point to first character in texure - usually a !
	memset ( m_pFontMap , 0, 256 );

	int i = 0;
	for(int r = 33; r < 127; r++)
	{
		m_pFontMap[r] = i;
		i++;
	}

	m_bAllocatedMap = true;

	return true;
}
