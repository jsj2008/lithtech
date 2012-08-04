//-------------------------------------------------------------------
//
//   MODULE    : CUISLIDER.H
//
//   PURPOSE   : defines the interface for The CUISlider Widget
//
//   CREATED   : 3/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISLIDER_H__
#define __CUISLIDER_H__


#ifndef __CUIINTERVAL_H__
#include "cuiinterval.h"
#endif


/*!  
CUISlider is a CUI widgets that can be used as a scroll-bar, a volume slider, or
anything else that is adjustable and has a min and a max value.  This widget
displays a background and a slider object.

CUISlider starts with a default \e min of 0, \e max of 100, \e current \e value
of 50, and \e slider \e size \e percent of 25. 

CUIE_BG
CUIE_SLIDER

\see ILTWidgetManager
\see CUI_ELEMENTTYPE

Used for: Text and UI.
*/


class CUISlider : public CUIInterval
{
	public:
	
#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIWidgets should not be created with new and delete, but should instead
		// be created via the ILTWindowManager's CreateWidget() and DestroyWidget()
		// management functions.
		CUISlider(CUIGUID guid);
		virtual ~CUISlider();

#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*!
\return the size of the slider element.

Returns the size of the slider element as a percentage of the current width of the CUISlider object.

\see SetSliderSizePercent()

Used for:  Text and UI.
*/
		virtual uint32			GetSliderSizePercent();

/*!
\param percent sets the size of the slider element.
\return CUI_RESULTTYPE.

Sets the size of the slider element as a percentage of the current width of the CUISlider object.

\see GetSliderSizePercent()

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	SetSliderSizePercent(uint16 percent);

};

#endif //__CUISLIDER_H__
