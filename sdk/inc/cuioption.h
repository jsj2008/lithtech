//-------------------------------------------------------------------
//
//   MODULE    : CUIOPTION.H
//
//   PURPOSE   : Defines the interface for The CUIOption Widget
//
//   CREATED   : 4/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIOPTION_H__
#define __CUIOPTION_H__


#ifndef __CUICHECK_H__
#include "cuicheck.h"
#endif


/*!  
CUIOption is one of the CUI widgets.  It displays a string of text
(using an internal CUIPolyString) and displays the following 
CUI_ELEMENTTYPEs based on its state:

CUIE_BG,
CUIE_OPTION_ON,
CUIE_OPTION_OFF,
CUIE_OPTION_DISABLED.

CUIOption is almost identical to CUICkeck, except that only one sibling at a time
can be set to \e true.  Calling SetValue(\b true) will cause all other CUIOption siblings
to change their value to false.

\see ILTWidgetManager
\see CUI_ELEMENTTYPE
\see SetState()
\see SetValue()

Used for: Text and UI.
*/


class CUIOption : public CUICheck
{
	public:
	
#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIWidgets should not be created with new and delete, but should instead
		// be created via the ILTWindowManager's CreateWidget() and DestroyWidget()
		// management functions.
		CUIOption(CUIGUID guid);
		virtual ~CUIOption();

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

};

#endif //__CUIOPTION_H__
