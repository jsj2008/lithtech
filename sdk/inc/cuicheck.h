//-------------------------------------------------------------------
//
//   MODULE    : CUICHECK.H
//
//   PURPOSE   : Defines the interface for The CUICheck Widget
//
//   CREATED   : 4/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUICHECK_H__
#define __CUICHECK_H__


#ifndef __CUIBUTTON_H__
#include "cuibutton.h"
#endif


/*!  
CUICheck is one of the CUI widgets.  It displays a string of text
(using an internal CUIPolyString), a background image and one of
the following CUI_ELEMENTTYPEs based on its state:

CUIE_BG,
CUIE_CHECK_ON,
CUIE_CHECK_OFF,
CUIE_CHECK_DISABLED.

\see ILTWidgetManager
\see CUI_ELEMENTTYPE
\see SetState()

Used for: Text and UI.
*/


class CUICheck : public CUIButton
{
	public:
	
#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIWidgets should not be created with new and delete, but should instead
		// be created via the ILTWindowManager's CreateWidget() and DestroyWidget()
		// management functions.
		CUICheck();
		CUICheck(CUIGUID guid);
		virtual ~CUICheck();


#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*!
\param value true or false
\return the previous value

Use this function to set a CUICheck widget's state.  If \e value is \b true, this function
is equivalent to calling CUIBase::SetState(CUIS_PRESSED).  if \e value is \b false, this
function is equivalent to calling CUIBase::UnsetState(CUIS_PRESSED).
*/
		virtual bool SetValue(bool value);

/*!
\return the current value

Use this function to query a CUICheck widget's state.  The return is \b true if the widget
has the state CUIS_PRESSED set, or \b false otherwise.
*/
		virtual bool GetValue();

/*!
\return the previous value

Use this function to toggle a CUICheck widget's state.  If the widget currently has the
state CUIS_PRESSED set, the return will be \b true and the state will be unset.  If 
CUIS_PRESSED is not currently set, \b false will be returned and the state will be set.
*/
		virtual bool Toggle();

};

#endif //__CUICHECK_H__
