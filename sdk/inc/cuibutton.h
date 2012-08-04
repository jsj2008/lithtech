//-------------------------------------------------------------------
//
//   MODULE    : CUIBUTTON.H
//
//   PURPOSE   : defines the interface for The CUI Button Widget
//
//   CREATED   : 1/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIBUTTON_H__
#define __CUIBUTTON_H__


#ifndef __CUISTATICTEXT_H__
#include "cuistatictext.h"
#endif


/*!  
CUIButton is one of the CUI widgets.  It displays a string of text
(using an internal CUIPolyString) over one of the following 
CUI_ELEMENTTYPEs based on its state:

CUIE_BG,
CUIE_BUTTON_PRESSED,
CUIE_BUTTON_HIGHLIGHTED,
CUIE_BUTTON_DISABLED.

\see ILTWidgetManager
\see CUI_ELEMENTTYPE
\see SetState()

Used for: Text and UI.
*/


class CUIButton : public CUIStaticText
{
	public:
	
#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIWidgets should not be created with new and delete, but should instead
		// be created via the ILTWindowManager's CreateWidget() and DestroyWidget()
		// management functions.
		CUIButton();
		CUIButton(CUIGUID guid);
		virtual ~CUIButton();

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

};

#endif //__CUIBUTTON_H__
