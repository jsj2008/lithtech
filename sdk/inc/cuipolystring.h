//-------------------------------------------------------------------
//
//   MODULE    : CUIPOLYSTRING.H
//
//   PURPOSE   : defines the CUIPolyString bridge Class
//
//   CREATED   : 7/00 
//
//-------------------------------------------------------------------


#ifndef __CUIPOLYSTRING_H__
#define __CUIPOLYSTRING_H__

 

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif


#ifndef DOXYGEN_SHOULD_SKIP_THIS
// forward declaration for the CUIBase Implementation class 
class CUIPolyString_Impl;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*!  CUIPolyString class.  This class is used to create and store
collections of polygons that have been mapped to font textures via 
the CUIFont class.

\see class CUIFont
\see class CUIFormattedPolyString

Used for: Text and UI.   */

class CUIPolyString
{
	public:
			
#ifndef DOXYGEN_SHOULD_SKIP_THIS
		
		// CUIPolyString should not be created with new and delete, but should instead
		// be created via the ILTFontManager's CreatePolyString() and DestroyPolyString()
		// management functions.

		CUIPolyString();

		CUIPolyString(CUIFont* pFont, 
					  const char* pBuffer = NULL,
					  float x = 0.0,
					  float y = 0.0);

		// virtual destructor
		virtual ~CUIPolyString();


		// all CUI classes support this
		virtual const char* GetClassName();
		
		
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
	
	public:
			
/*!
\param pBuffer  A NULL-terminated string of text (no formatting characters).
\return	CUI_RESULTTYPE.

Sets the text of a CUIPolyString.  When function is used, the entire text of the
CUIPolyString will be formatted with the last CUIFont which was applied to the
CUIPolyString.

Returns CUIR_OK if successful, CUIR_NO_FONT if there is no associated CUIFont, 
CUIR_OUT_OF_MEMORY if there are memory allocation problems, otherwise returns
CUIR_ERROR.

\see ApplyFont()

Used for: Text and UI.  */
		virtual CUI_RESULTTYPE	SetText(const char* pBuffer);

/*!
\param argb uint32 color of the form 0xAARRGGBB.

Sets a single color for each character of the entire CUIPolyString.

Used for: Text and UI.  */
		virtual CUI_RESULTTYPE	SetColor(uint32 argb);

		
/*!
\param argb0 uint32 color of the form 0xAARRGGBB.
\param argb1 uint32 color of the form 0xAARRGGBB.
\param argb2 uint32 color of the form 0xAARRGGBB.
\param argb3 uint32 color of the form 0xAARRGGBB.
\param span true or false

Sets the vertex colors for each character of the entire CUIPolyString (each character is
represented internally as an LT_POLYGT4).  

\see struct LT_POLYGT4
\see interface ILTDrawPrim

Vertices are numbered clockwise beginning with vertex 0 at the top left corner.

Used for: Text and UI.  */
		virtual CUI_RESULTTYPE	SetColors(uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);



/*!
\param height desired screen height (in pixels) of each character.
\param width (optional) desired screen width (in pixels) of each character.

\return	CUIR_OK if successful.

Sets the screen size of the CUIPolyString's characters.

Used for: Text and UI.  */
		virtual CUI_RESULTTYPE	SetCharScreenSize(uint8 height, uint8 width = 0);

/*!
\param width desired screen width (in pixels) of each character.

\return	CUIR_OK if successful.

Sets the screen width of the CUIPolyString's characters.

Used for: Text and UI.  */
		virtual CUI_RESULTTYPE	SetCharScreenWidth(uint8 width);

/*!
\param height desired screen height (in pixels) of each character.

\return	CUIR_OK if successful.

Sets the screen height of the CUIPolyString's characters.

Used for: Text and UI.  */		
		virtual CUI_RESULTTYPE	SetCharScreenHeight(uint8 height);

/*!
\return	character screen width.

retrieves the screen width of the CUIPolyString's characters.

Used for: Text and UI.  */
		virtual uint8			GetCharScreenWidth();

/*!
\return	character screen height.

Sets the screen height of the CUIPolyString's characters.

Used for: Text and UI.  */
		virtual uint8			GetCharScreenHeight();



/*!
\param x the new x coordinate of the CUIPolyString (onscreen, in pixels) 
\param y the new y coordinate of the CUIPolyString (onscreen, in pixels)
\return	CUI_RESULTTYPE.

Sets new screen coordinates for the CUIPolyString.

Returns CUIR_OK if successful.

Used for: Text and UI.  */
		virtual CUI_RESULTTYPE	SetPosition(float x, float y);

/*!
\param pFont the new font for the CUIPolyString 
\return	CUI_RESULTTYPE.

Sets a new font for the entire CUIPolyString.

Returns CUIR_OK if successful.

Used for: Text and UI.  */
		virtual CUI_RESULTTYPE	SetFont(CUIFont* pFont);

				
/*!
\return	const char*.

Returns the ascii text of a CUIPolyString.  This string cannot be modified.
To change a CUIPolyString's text, use CuiPolyString::SetText().

\see SetText()

Used for: Text and UI.  */
		virtual const char*	GetText();
		
/*!
\return	LT_POLYGT4*.

Returns a pointer to the polygons which make up a CUIPolyString.  These
polygons can be directly manipulated at runtime for various visual effects. 

\see class  ILTDrawPrim
\see struct LT_POLYGT4

Used for: Text and UI.  */
		virtual LT_POLYGT4*	GetPolys();
		
/*!
\return	CUIFont*.

Returns a pointer to the current font associated with the CUIPolyString.  
If you wish to change the font, use CUIFont::Apply()

\see CUIFont::Apply()

Used for: Text and UI.  */
		virtual CUIFont*	GetFont();

/*!
\param  pWidth address of variable to receive the width (in pixels)
\param  pHeight address of variable to receive the height (in pixels)
\return	CUI_RESULTTYPE.

Gets the current screen dimensions of the CUIPolyStirng

\see GetPosition()
\see GetRect()

Returns CUIR_OK.

Used for: Text and UI.  */		
		virtual CUI_RESULTTYPE	GetDims(float* pWidth, float* pHeight);

/*!
\param  pX address of variable to receive the x coord. (in pixels)
\param  pY address of variable to receive the y coord. (in pixels)
\return	CUI_RESULTTYPE.

For a CUIPolyStirng, returns the (x,y) screen coordinates of the upper left corner
of the string.  For a CUIFormattedPolyString, (x,y) can be the top-left, top-center,
or top-right, depending on the alignment.
  
\see CUIFormattedPolyString::SetAlignmentH()
\see GetDims()
\see GetRect()

Returns CUIR_OK.

Used for: Text and UI.  */	
		virtual CUI_RESULTTYPE	GetPosition(float* pX, float* pY);
		
/*!
\param pRect address of variable to receive the bounding rectangle (in pixels)
\return	CUI_RESULTTYPE.

Gets the current bounding box of the CUIPolyString.  If you are querying a
CUIFormattedPolyString, be aware that based on the alignment of the string,
the resulting \em pRect->x and \em pRect->y values might be different from
the (x,y) coords. returned by GetPosition(), GetX(), and GetY().  

\see CUIFormattedPolyString::SetAlignmentH()
\see GetDims()
\see GetPosition()

Returns CUIR_OK.

Used for: Text and UI.  */	
		virtual CUI_RESULTTYPE	GetRect(CUIRECT* pRect);
		
/*!
\return	float.

Gets the current screen height of the CUIPolyString (in pixels).

Used for: Text and UI.  */	
		virtual float		GetHeight();

/*!
\return	float.

Gets the current screen width of the CUIPolyString (in pixels).

Used for: Text and UI.  */	
		virtual float		GetWidth();
		
/*!
\return	float.

Gets the current screen x coordinate of the CUIPolyString (in pixels).

Used for: Text and UI.  */	
		virtual float		GetX();
		
/*!
\return	float.

Gets the current screen y coordinate of the CUIPolyString (in pixels).

Used for: Text and UI.  */	
		virtual float		GetY();
		
/*!
\return	int16.

Gets the length of the ascii representation of the CUIPolyString (in characters).

Used for: Text and UI.  */	
		virtual uint16		GetLength();

		
/*!
\param pFont CUIFont to use.
\param index (optional) index of the first character to modify.
\param num (optional) number of characters to modify.
\param bProcessRemainder (optional) adjust positions of chars following the modification.

Applies a CUIFont to a CUIPolyString.  If \em index is not specified, it will be
the first character of the string.  If \em num is not specified, all characters
from \em index to the end of the string will be modified.  If \b ProcessRemainder is
not specified, all characters following the modification will be adjusted horizontally.

This version of ApplyFont allows you to modify all or part of a string.  If you modify only part of a
CUIPolyString, however, you may \em only change the color, size, or attributes of the font. 
If you change the font's texture or font table and then apply the font to only a
portion of the string, the results could be garbled on screen.

Used for: Text and UI.  */

		virtual CUI_RESULTTYPE	ApplyFont(CUIFont* pFont, 
									  int16 index = 0, 
									  int16 num = 0,
									  bool	bProcessRemainder = true);

/*!
\param start (optional) drawing starts from this character index.
\param end (optional) drawing ends at this character index.
\return	none.

Draws the CUIPolyString to the screen.  If \e start and \e end are not
specified, the entire string is drawn.

Used for: Text and UI.  */	
		virtual CUI_RESULTTYPE		Render(int32 start = 0, int32 end = -1);
		
/*!
\param clip Rectangle to clip to.
\param start (optional) drawing starts from this character index.
\param end (optional) drawing ends at this character index.
\return	none.

Clips the CUIPolyString to the rectangle defined by \em clip.
If \e start and \e end are not
specified, the entire string is drawn.

\b NOTE: precise clipping is performed (i.e., you will see partial letters if the clip
rectangle intersects character polygons).

Used for: Text and UI.  */	
		virtual CUI_RESULTTYPE		RenderClipped(CUIRECT* pClipRect, 
												  int32 start = 0, 
												  int32 end = -1);

	
/*!
\return true or false.

Use this function to test whether a polystring is valid.  The most likely reason for
a polystring to be invalid is to call ILTFontManager::CreatePolyString() with a NULL
font.
  
Used for: Text and UI.  */	
		virtual	bool				IsValid();

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	
		// this code makes the bridge between the base interface and
		// its implementation.  This allows us to neatly cross the
		// EXE/DLL boundary and also keep utility code in the ui/src
		// directory where it belongs (i.e., not exposed in the
		// SDK header files).

		CUIPolyString_Impl*		GetImpl() { return m_pImpl; }
	
	protected:

		CUIPolyString_Impl*		m_pImpl;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
		
						
};


#endif //__CUIPOLYSTRING_H__
