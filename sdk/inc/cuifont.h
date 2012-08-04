//-------------------------------------------------------------------
//
//   MODULE    : CUIFONT.H
//
//   PURPOSE   : defines the inteface for the CUIFont Class
//
//   CREATED   : 7/00 
//
//-------------------------------------------------------------------


#ifndef __CUIFONT_H__
#define __CUIFONT_H__

 
#ifndef __CUI_H__
#include "cui.h"
#endif


/*! 
FX bitmasks that can be applied to CUIFonts.
*/
typedef enum {
/*! 
Turns \b Italics on or off.
*/
	CUI_FONT_ITALIC		= 1,
/*! 
Turns \b Bold on or off.
*/
	CUI_FONT_BOLD		= 2,

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	// currently unsupported
	CUI_FONT_UNDERLINE	= 4,
	CUI_FONT_STRIKEOUT	= 8,
	CUI_FONT_DROPSHADOW = 16
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

}	CUI_FONTFXTYPE;


#ifndef DOXYGEN_SHOULD_SKIP_THIS
// forward decl needed to avoid circular include
class CUIPolyString;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*! 
The maximum number of charcters allowed in a font.
\b Note:  For efficiency reasons, the CUI font system is largely based on a uint8, 
and simply changing this \e #define will not have the effect of making fonts capable
of holding more than 255 elements.
*/
#define CUI_MAX_BITMAPFONT_CHARS	255


/*!
The CUIFont class is used to store font resources.  Fonts can be monospace or
proportional, and can be created from HTEXTURES or TrueType resources.

\see interface ILTFontManager
\see class CUIPolyString

Used for: Text and UI.   */

class CUIFont
{
	public:
	
#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// all CUI classes support this
		virtual const char* GetClassName() = 0;
		
		// virtual destructor
		virtual ~CUIFont() {};

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

	public:
		
/*!
\return	bool.

This function is used to query whether on not the font in question is
valid.  This is used internally by ILFontManager.

Used for: Text and UI.  */
		virtual bool		IsValid() = 0;


/*!
\return	bool.

This function is used to query whether on not the font in question is
proportional (as opposed to monospace).

Used for: Text and UI.  */
		virtual bool		IsProportional() = 0;
	

/*!
\return	HTEXTURE.

This function returns a handle to the texture associated with the
font.

\see ILTTexInterface

Used for: Text and UI.  */
		virtual HTEXTURE	GetTexture() = 0;
		

/*!
\return	uint32 flags.

This function is used to query the attribute flags associated with a font.

\see SetAttributes()
\see UnsetAttributes()
\see CUI_FONTFXTYPE

Used for: Text and UI.  */
		virtual uint32		GetAttributes() = 0;


/*!
return	default width of one character on screen (in pixels).

For proportional fonts, function returns the default width of
a space character.  This width can be set with the CUIFont::SetDefCharWidth()
function.

Used for: Text and UI.  */
		virtual uint8		GetDefCharScreenWidth() = 0;


/*!
\return	default height of one character on screen (in pixels).

This is not a per-character value.  This is the font's height.  The height can
be set with the CUIFont::SetDefCharHeight() function. 

Used for: Text and UI.  */
		virtual uint8		GetDefCharScreenHeight() = 0;

/*!
\return	width of one character's texture (in pixels).

This is the value that is passed in when creating monospace bitmap fonts.  

Used for: Text and UI.  */
		virtual uint8		GetCharTexWidth() = 0;

/*!
\return	height of one character's texture (in pixels).

This is the value that is passed in when creating monospace or proportional
bitmap fonts.

Used for: Text and UI.  */
		virtual uint8		GetCharTexHeight() = 0;

/*!
\return	default horizontal spacing between characters (in pixels).

\see CUIFont::SetDefSpacing()

Used for: Text and UI.  */
		virtual uint8		GetDefSpacingH() = 0;

/*!
\return	default vertical spacing between characters (in pixels).

\see CUIFont::SetDefSpacingV()

Used for: Text and UI.  */
		virtual uint8		GetDefSpacingV() = 0;	

/*!
\return	The font's default italic slant (in pixels).

\see CUIFont::SetDefSlant()

Used for: Text and UI.  */
		virtual int8		GetDefSlant() = 0;	

/*!
\return	The font's default bold value (in pixels).

\see CUIFont::SetDefBold()

Used for: Text and UI.  */
		virtual int8		GetDefBold() = 0;	

/*!
\return The font's character uv lookup table.

\see ILTFontManager

Used for: Text and UI.  */
		virtual uint16*		GetFontTable() = 0;
		
/*!
\return	The font's character map.

\see ILTFontManager

Used for: Text and UI.  */
		virtual uint8*		GetFontMap() = 0;		

/*!
\return	pointer to an array of 4 default uint32 colors of the form 0xAARRGGBB.

\see CUIFont::SetDefColor()
\see CUIFont::SetDefColors()

Used for: Text and UI.  */
		virtual uint32*		GetDefColors()	= 0;


/*!
\param hTex New texture.
\param cwidth (optional) New character width (in pixels, relative to the texture map).
\param cheight (optional) New character height (in pixels, relative to the texture map).
\return CUIR_OK if successful.

This function is used to set a monospace font's texture map and optionally
set new character dimensions (relative to the texture map, not on-screen).
If \em cwidth or \em cheight are not specified, the character dimensions will
not be changed from their original values.

Used for: Text and UI.  */
		virtual  CUI_RESULTTYPE	SetTexture(HTEXTURE hTex, 
							   uint8 cwidth  = 0,
							   uint8 cheight = 0) = 0;
		

/*!
\param hTex New texture.
\param pTable (optional) Address of new font table.
\param cheight (optional) New character height (in pixels, relative to the texture map).
\return	CUIR_OK if successful.

This function is used to set a proportional font's texture map and optionally
set a new character height (within that texture, not onscreen).  If \em cheight is not specified, the characters'
texture height coordinates will not be changed from their original values.

\see ILTFontManager::CreateFont(HTEXTURE, uint8*, uint8)

Used for: Text and UI.  */
		virtual CUI_RESULTTYPE	SetTexture(HTEXTURE hTex, 
							   uint16* pTable  = NULL,
							   uint8  cheight = 0) = 0;
							   
							   
/*!
\param pMap pointer to a font map.
\return	none.

This function is used to set a font's map.  A font map is an array of uint8
which maps characters to their positions in font table (which in turn detemrines
their u,v coordinates).  For example, if the element [32] of the font map array was 
equal to 10 ,then ascii character '!' would map to entry [10] in the font
table.

This is useful in such cases as using European characters with accents and
umlauts--such characters are sprinkled throughout the upper 128 ascii indices
and you may not wish to use up valuable VRAM by including unprintable or unused
characters in your font.  Or, you may wish to map all lowercase characters to
their capital counterparts for example.

\b NOTE: If you provide a font map, you are responsible for allocating and freeing the
memory.

Used for: Text and UI.  */
		virtual  void		SetMap(uint8* pMap) = 0;


/*!
\param attrs New font flags.

This function is used to set the attribute flags for a font.  More than one
flag can be set at a time, e.g.:

\code
CUIFont::SetAttributes(CUI_FONT_ITALIC | CUI_FONT_BOLD);
\endcode
		
\see GetAttributes()
\see UnsetAttributes()
\see CUI_FONTFXTYPE

Used for: Text and UI.  */
		virtual void		SetAttributes(uint32 attrs) = 0;


/*!
\param attrs Font flags to unset.

This function is used to unset the attribute flags for a font.  More than one
flag can be unset at a time, e.g.:

\code
CUIFont::UnsetAttributes(CUI_FONT_ITALIC | CUI_FONT_BOLD);
\endcode
		
\see GetAttributes()
\see SetAttributes()
\see CUI_FONTFXTYPE

Used for: Text and UI.  */
		virtual void		UnsetAttributes(uint32 attrs) = 0;


/*!
\param width Sets the font's default character width on screen (in pixels).

For proportional fonts, this function sets the number of pixels that a 'space' character uses.
For monospace fonts, the screen width of every chracter is set to \em width.  When a 
CUIPolyString or a CUI widget that displays text is created, this is default character width.

Used for: Text and UI.  */
		virtual void		SetDefCharWidth(uint8 width) = 0;

/*!
\param height Sets the font's default character height on screen (in pixels).

This function affects both monospace and proportional fonts.  When a CUIPolyString or a CUI
widget that displays text is created with this font, this is default character height.  

Used for: Text and UI.  */
		virtual void		SetDefCharHeight(uint8 height) = 0;


/*!
\param spacing Sets the font's default horizontal character spacing on screen (in pixels).

\em Spacing is the distance between the left edge of a character and the right edge
of the preceeding character.

Used for: Text and UI.  */
		virtual void		SetDefSpacingH(uint8 hspacing) = 0;
		

/*!
\param spacing Sets the font's default vertical character spacing on screen (in pixels).

\em Spacing is the distance between the top edge of a character and the bottom edge
of the character above (in a multi-line string).

Used for: Text and UI.  */
		virtual void		SetDefSpacingV(uint8 vspacing) = 0;

		
/*!
\param slant Sets the font's default italic slant (in pixels).

\em Slant is the distance that a character's top is offset horizontally from
its bottom when italic characters are being drawn.  A positive value for 
\em slant will cause the characters to lean to the right, while a negative
value for \em slant will cause the characters to lean to the left.

\b Note: Simply setting the \em slant of a font will not cause the font to be
italicized.  You must set the font attribute CUI_FONT_ITALIC.

\see SetAttributes()

Used for: Text and UI.  */
		virtual void		SetDefSlant(int8 slant) = 0;


/*!
\param bold Sets the font's default bold amount (in pixels).

\em Bold is the amount that a character's width is increased (or decreased)
from the normal character width.

\b Note: Simply setting \em bold for a font will not cause the font to be
emboldened.  You must set the font attribute CUI_FONT_BOLD.

\see SetAttributes()

Used for: Text and UI.  */
		virtual void		SetDefBold(int8 bold) = 0;


/*!
\param argb Sets the font's default color.

Sets a single color for each character of the entire font.

Used for: Text and UI.  */
		virtual void		SetDefColor(uint32 argb) = 0;

		
/*!
\param argb0 Sets the font's character color for vertex 0.
\param argb1 Sets the font's character color for vertex 1.
\param argb2 Sets the font's character color for vertex 2.
\param argb3 Sets the font's character color for vertex 3.

Sets the default vertex colors for each character of the entire font (each character
is represented internally as an LT_POLYGT4).  Vertices are numbered clockwise
beginning with vertex 0 at the top left corner.

\see struct LT_POLYGT4
\see interface ILTDrawPrim


Used for: Text and UI.  */
		virtual void		SetDefColors(uint32 argb0,
							  uint32 argb1,
							  uint32 argb2,
							  uint32 argb3) = 0;


/*!
\param x horizontal screen coordinate (in pixels).
\param y vertical screen coordinate (in pixels).
\param text a string of text.

Draws a string of text to the screen.  This function essentially allocates a
CUIPolyString, draws it, and then frees it.  For persistent text, it is 
recommended to create and store your own CUIPolyString objects -- it will be
much more resource friendly.  

\see class CUIPolyString
\see ILTFontManager::CreatePolyString()

Used for: Text and UI.  */
		virtual void		DrawString(float x, float y, char* text) = 0;


/*!
\param pPolyString CUIPolyString to modify.
\param index (optional) index of the first character to modify.
\param num (optional) number of characters to modify.

Applies a CUIFont to a CUIPolyString.  If \em index is not specified, it will be
the first character of the string.  If \em num is not specified, all characters
from \em index to the end of the string will be modified.

Apply allows you to modify all or part of a string.  If you modify only part of a
CUIPolyString, however, you may \em only change the color, size, or attributes of the font. 
If you change the font's texture or font table and then apply the font to only a
portion of the string, the results could be garbled on screen.

\b Note:  This function is deprecated and will be removed in a future release.
Use CUIPolyString::ApplyFont() instead.

\see CUIPolyString

Used for: Text and UI.  */
		virtual void 		Apply(CUIPolyString* pPolyStr,
								  int16 index = 0,
								  int16 num   = 0) = 0;
				
};


#endif //__CUIFONT_H__
