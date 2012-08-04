//-------------------------------------------------------------------
//
//   MODULE    : CUISTATICTEXT.H
//
//   PURPOSE   : defines the CUIStaticText bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISTATICTEXT_H__
#define __CUISTATICTEXT_H__


#ifndef __CUIWIDGET_H__
#include "cuiwidget.h"
#endif

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif


// #define CUI_STATICTEXT_MAXLEN	255


/*!  
CUIStaticText class.  This class is uses a CUIFormattedPolyString to provide
a way of displaying formatted text within the widget hierarchy.

\see class CUIPolyString

Used for: Text and UI.   */

class CUIStaticText : public CUIWidget
{
	public:
			

#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIWidgets should not be created with new and delete, but should instead
		// be created via the ILTWindowManager's CreateWidget() and DestroyWidget()
		// management functions.
		CUIStaticText();
		CUIStaticText(CUIGUID guid);
		virtual ~CUIStaticText();
		
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*!
\return NULL-terminated, immutable string

This function gets the ascii text stored in a static text widget

Used for: Text and UI.
*/
		virtual const char*		GetText();


/*!
\return CUIFont.

This function returns the CUIFont associated with a static text widget

Used for: Text and UI.
*/
		virtual CUIFont*		GetFont();
		

/*!
\param pBuffer string to set.

\return CUIR_OK if successful.

This function sets the ascii text stored in a static text widget.  The
font must be set before calling SetText()

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE	SetText(const char* pBuffer);


/*!
\param pFont CUIFont to set.

\return CUIR_OK if successful.

This function sets the CUIFont associated with a static text widget.  The
font must be set before calling SetText().

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE	SetFont(CUIFont* pFont);


/*!
\param height desired screen height of characters.

\return CUIR_OK if successful.

This function sets the height of the text contained in the widget.

Used for: Text and UI.
*/
		virtual CUI_RESULTTYPE	SetCharHeight(uint8 height); 


/*!
\param wrap wrap-width.
\return	CUI_RESULTTYPE.

Sets the wrap-width of the widget.  This defines where a multi-line
CUIStaticText widget will break text. 

Returns CUIR_OK if successful.

Used for: Text and UI.  */
		virtual CUI_RESULTTYPE	SetWrapWidth(uint16 wrap);
			
};

#endif //__CUISTATICTEXT_H__
