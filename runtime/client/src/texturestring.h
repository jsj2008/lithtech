//-------------------------------------------------------------------
// TextureString.h
//
// Provides the definition for a texture string object. This object
// holds a list of character information and any associated formatting
// information associated with the string. The actual glyph characters
// for this string are found in the associated texture string image
// which holds onto the texture and glyph information.
//
//-------------------------------------------------------------------

#ifndef __TEXTURESTRING_H__
#define __TEXTURESTRING_H__

#ifndef __TEXTURESTRINGIMAGE_H__
#	include "texturestringimage.h"
#endif

class CTextureString :
	public ILTRefCount
{
public:

	CTextureString();
	~CTextureString();

	//called to create a string given a string and an associated font
	LTRESULT	Create(const wchar_t* pszString, const CFontInfo& Font);

	//called to create a string given a string and an existing texture string to create it from
	LTRESULT	CreateSubstring(const wchar_t* pszString, const CTextureString& CreateFrom);

	//frees everything associated with this string. This discards all characters, image data, etc.
	void		FreeData();

	//applies word wrapping to the string so that all characters should be in the range of 0..nWidth in 
	//the X axis and in the range [0...+inf] in the Y axis. This undoes any previous word wrapping or
	//formatting
	bool		WordWrap(uint32 nWidth);

	//calling this will undo any formatting associated with the string and will place characters in
	//a single row
	bool		ClearFormatting();

	//called to get the extents of the string. These extents encompass all the characters
	//within the string
	LTRect2n						GetExtents() const				{ return m_rExtents; }

	//called to access the string associated with this
	const wchar_t*				GetString() const				{ return m_pszString; }

	//called to access the associated texture image reference. Note that if this is to be held onto
	//this should be stored in an image reference
	const CTextureStringImage*	GetTextureImage() const			{ return m_TextureImage; }

	//called to get the length of the associated string
	uint32							GetStringLength() const			{ return m_nNumCharacters; }

	//called to get information about a character. This assumes that the index is within range
	void						GetCharInfo(uint32 nIndex, int32& nX, int32& nY, bool& bVisible, const CTextureStringGlyph*& pGlyph);

	//given a character index, this will return the string space bounding rectangle of that string
	LTRect2n					GetCharRect(uint32 nIndex);

private:

	//internal creation called once a string and a bitmap font image have been properly
	//setup
	LTRESULT	InternalCreate(const wchar_t* pszString, CTextureStringImage* pTextureImage);

	//called to update the extents of the string
	void		UpdateExtents();

	//called to allocate a string. This will allocate the characters, the string, and handle copying
	//them over
	bool	AllocateString(const wchar_t* pszString);

	//frees all string data allocated by the allocate string
	void	FreeString();

	//called when the reference count goes to zero
	virtual void Free();

	//class that represents a single character within the string. It has
	//a placement position that is in string space, and therefore by moving
	//this around it repositions the character. In addition this points to
	//the associated glyph inside of the texture string image
	class CTextureStringChar
	{
	public:

		CTextureStringChar() :
			m_nXPos(0),
			m_nYPos(0),
			m_pGlyph(NULL),
			m_bVisible(true)
		{
		}

		//the current position in string space
		int32						m_nXPos;
		int32						m_nYPos;

		//the glyph that this character uses for rendering
		const CTextureStringGlyph*	m_pGlyph;

		//boolean indicating whether or not this glyph should be rendered
		bool						m_bVisible;
	};

	//the extents that encompass each character in the string
	LTRect2n				m_rExtents;

	//a reference to our texture string image
	TTextureStringImageRef	m_TextureImage;

	//The list of characters in this string, their placement, and their associated glyphs
	CTextureStringChar*		m_pCharacters;

	//the number of characters in the string
	uint32					m_nNumCharacters;

	//a flat copy of the string. This allows for easier debugging, and also faster access to straight
	//string data
	wchar_t*				m_pszString;

};


#endif
