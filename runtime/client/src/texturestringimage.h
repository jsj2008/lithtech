//-------------------------------------------------------------------
// TextureStringImage.h
//
// Provides the definition of the texture string image class which
// serves as a bitmap font for the texture strings. This bitmap font
// contains a sequence of glyphs that are pieced together by the
// texture strings themselves to form final strings
//
//-------------------------------------------------------------------

#ifndef __TEXTURESTRINGIMAGE_H__
#define __TEXTURESTRINGIMAGE_H__

#ifndef __ILTREFCOUNT_H__
#	include "iltrefcount.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#	include "ilttexinterface.h"
#endif

#ifndef __ILTTEXTURESTRING_H__
#	include "ilttexturestring.h"
#endif

//class that represents an actual glyph. A glyph has an associated character
//and also a rectangle into the texture
class CTextureStringGlyph
{
public:

	//the glyph that this rectangle holds
	wchar_t		m_cGlyph;

	//the black box of this glyph. This is the rectangle that encompases the area
	//that should be rendered, and is relative to 0, 0 being the upper left of the 
	//glyph space, so if it is 1, it has a spacing of one on the left, or if it is -1
	//it backs up some, etc.
	LTRect2n	m_rBlackBox;

	//the total width and height that should be used for skipping to the next character of
	//the string.
	uint32		m_nTotalWidth;

	//the actual rectangle in the texture
	float		m_fU;
	float		m_fV;
	float		m_fTexWidth;
	float		m_fTexHeight;
};

//the actual texture string image
class CTextureStringImage :
	public ILTRefCount
{
public:

	CTextureStringImage();
	~CTextureStringImage();

	//called to allocate a new texture string image object
	static CTextureStringImage*	Allocate();
	static void							Free(CTextureStringImage* pImage);

	//called to create a texture given a font and a string
	bool						CreateBitmapFont(const wchar_t* pszString, const CFontInfo& Font);

	//frees all data associated with this object
	void						FreeData();

	//accesses a glyph in the list
	const CTextureStringGlyph*	GetGlyphByIndex(uint32 nGlyph) const;
	const CTextureStringGlyph*	GetGlyph(wchar_t cGlyph) const;

	//provides access to the list of glyphs
	uint32						GetNumGlyphs() const			{ return m_nNumGlyphs; }

	//provides access to the height of a single row
	uint32						GetRowHeight() const			{ return m_nRowHeight; }

	//provides access to the font used to create the image
	const CFontInfo&			GetFont() const					{ return m_FontInfo; }

	//provides access to the texture
	HTEXTURE						GetTexture() const				{ return m_hTexture; }

private:

	//------------------------------------------
	// Creation utilities
	//------------------------------------------

	//called during the creation to extract all the unique glyphs from a string, allocate the
	//glyph list, and set them up with the characters they reference
	bool						SetupUniqueGlyphList(const wchar_t* pszString);

	//called by the base ILTRefCount
	virtual void			Free()		{ Free(this); }

	//the list of the glyphs associated with the bitmap font
	CTextureStringGlyph*	m_pGlyphList;

	//the height of a single row in pixels
	uint32					m_nRowHeight;

	//the number of the glyphs in the texture
	uint32					m_nNumGlyphs;

	//the font associated with this texture string bitmap font
	CFontInfo				m_FontInfo;

	//handle to the texture that holds the actual image data
	HTEXTURE					m_hTexture;
};

typedef CLTReference<CTextureStringImage>	TTextureStringImageRef;

#endif
