//-------------------------------------------------------------------
//
//   MODULE    : CUIFORMATTEDPOLYSTRING.H
//
//   PURPOSE   : defines the CUIFormattedPolyString bridge Class
//
//   CREATED   : 7/00 
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIFORMATTEDPOLYSTRING_H__
#define __CUIFORMATTEDPOLYSTRING_H__


#ifndef __CUIPOLYSTRING_H__
#include "cuipolystring.h"
#endif


/*!  MAX_POLYSTRING_LINES determines how many lines can be displayed by a CUIFormattedPolyString.
decreasing this value will save memory (less pre-allocated formatting data).  If you try to create
a polystring whose lines exceed this value, you'll see an assertion failure--(in DEBUG mode) and your
string will be capped.
*/
#define		MAX_POLYSTRING_LINES	64

/*!  MAX_POLYSTRING_WORDS determines how many words can be displayed by a CUIFormattedPolyString.
decreasing this value will save memory (less pre-allocated formatting data).  If you try to create
a polystring whose words exceed this value, you'll see an assertion failure (in DEBUG mode) and your
string will be capped.
*/
#define		MAX_POLYSTRING_WORDS	128


/*!  CUIFormattedPolyString class.  This class is used to create and store
collections of polygons that have been mapped to font textures via 
the CUIFont class.  In addition to the functionality of the regular CUIPolyString,
the CUIFormattedPolyString adds support for newlines, word-wrap, and justification.

\see class CUIFont

Used for: Text and UI.   
*/
class CUIFormattedPolyString : public CUIPolyString
{
	public:
			
#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIPolyString should not be created with new and delete, but should instead
		// be created via the ILTFontManager's CreatePolyString() and DestroyPolyString()
		// management functions.

		CUIFormattedPolyString(CUIFont* pFont, 
				  const char* pBuf = NULL,
				  float x   = 0.0,
				  float y   = 0.0,
  				  CUI_ALIGNMENTTYPE alignment = CUI_HALIGN_LEFT);
		
		// all CUI classes support this
		virtual const char* GetClassName();
		
		// virtual destructor
		virtual ~CUIFormattedPolyString();
		
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
	
	public:

/*!
\param align new text alignment.
\return	CUIR_OK if successful.

Sets the horizontal alignment of a CUIFormattedPolyString.  For a CUIFormattedPolyString,
the horizontal alignment is based on the string's (x,y) coordinate.  If the \em align is
\b CUI_HALIGN_LEFT, the (x,y) will refer to the upper left corner of the string.  If 
\em align is \b CUI_HALIGN_CENTER, the string's x coordinate will refer to the horizontal
midpoint of the string.  If \em align is \b CUI_HALIGN_RIGHT, (x,y) will be the upper right
corner of the string.  if \em align is \b CUI_HALIGN_JUSTIFIED, (x,y) will be the upper left
corner and the string's text will spread out to fill the space between x and (x + wrapwidth)

\see SetWrapWidth()

Used for: Text and UI.  */
		virtual	CUI_RESULTTYPE	SetAlignmentH(CUI_ALIGNMENTTYPE halign);

/*!
\param wrap wrap-width for each line.
\return	CUIR_OK if successful.

Sets the word-wrap width (in pixels) for a CUIFormattedPolyString.  The string
will honor newlines ('\n') and will attempt to perform word wrapping. If
\e wrap is zero, word wrap will be turned off for this poly string (and as a 
side-effect, the string will no longer honor newline characters).  The wrap width
also detemines the right margin when a CUIFormattedPolystring's alignment is set to
\b CUI_HALIGN_JUSTIFIED.

\see SetAlignmentH()

Used for: Text and UI.  */
		virtual CUI_RESULTTYPE	SetWrapWidth(uint16 wrap);
		
};


#endif //__CUIFORMATTEDPOLYSTRING_H__
