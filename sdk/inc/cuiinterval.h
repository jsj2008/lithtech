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


#ifndef __CUIINTERVAL_H__
#define __CUIINTERVAL_H__


#ifndef __CUISTATICIMAGE_H__
#include "cuistaticimage.h"
#endif


/*!  
CUIInterval is a class containing member data and functions which are
common to widgets such as CUISlider and CUIProgress.  This class cannot
be directly instantiated.  

\see ILTWidgetManager
\see CUISlider
\see CUIProgress

Used for: Text and UI.
*/


class CUIInterval : public CUIStaticImage
{
	public:
	
#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIWidgets should not be created with new and delete, but should instead
		// be created via the ILTWindowManager's CreateWidget() and DestroyWidget()
		// management functions.
		virtual ~CUIInterval();

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*!
\return the object's maximum value.

\see SetMaxValue()

Used for:  Text and UI.
*/
		virtual int32			GetMaxValue();

/*!
\return the object's minimum value.

\see SetMinValue()

Used for:  Text and UI.
*/
		virtual int32			GetMinValue();

/*!
\param pMin address of variable to receive the objet's minimum value
\param pMax address of variable to receive the objet's maximum value
\return CUIR_OK.

GetRange retrieves the maximum and  minimum values from the object.

\see SetRange()

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	GetRange(int32* pMin, int32* pMax);

/*!
\return the object's current value.

\see SetCurrentValue()

Used for:  Text and UI.
*/
		virtual int32			GetCurrentValue();

/*!
\return the object's screen orientation.

Returns either CUI_ORIENT_HORIZONTAL or CUI_ORIENT_VERTICAL.

\see SetOrientation()

Used for:  Text and UI.
*/
		virtual CUI_ORIENTATIONTYPE	GetOrientation();

/*!
\param max new maximum value.
\return CUI_RESULTTYPE.

Sets the object's maximum value.

\see GetMaxValue()

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	SetMaxValue(int32 max);

/*!
\param min new minimum value.
\return CUI_RESULTTYPE.

Sets the object's minimum value.

\see GetMinValue()

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	SetMinValue(int32 min);

/*!
\param min new minimum value.
\param max new maximum value.
\return CUI_RESULTTYPE.

Sets the object's minimum and maximum values.

\see GetRange()

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	SetRange(int32 min, int32 max);

/*!
\param val the object's new current value.
\return CUI_RESULTTYPE.

\see GetCurrentValue()

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	SetCurrentValue(int32 val);

/*!
\param orient the object's new screen orientation.
\return CUI_RESULTTYPE.

Specifies whether the object is drawn vertically (CUI_ORIENT_VERTICAL) or
horizontally (CUI_ORIENT_HORIZONTAL).

\see GetOrientation()

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	SetOrientation(CUI_ORIENTATIONTYPE orient);

/*!
\param inc the amount by which to increment the object's current value.
\return CUI_RESULTTYPE.

The object's current value will be incremented by \e inc.  \e Inc can be positive 
or negative.  An object's current value will never go outside the range of its 
maximum or minimum values.

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	Increment(int32 inc);

/*!
\param inc the percentage by which to increment the object's current value.
\return CUI_RESULTTYPE.

The object's current value will be incremented by (max_value - min_value) * \e percent.
\e Percent can be positive or negative.  An object's current value will never go outside 
the range of its maximum or minimum values.

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	IncrementPercent(int32 percent);

/*!
\param x x-coordinate to test.
\param y y-coordinate to test.
\return CUI_RESULTTYPE.

Use this function to test a point (screen coordinates) inside the object.  If the object is a CUISlider, 
possible return values are: 

\e CUIR_SLIDER_MIN if the point is in the area that reperesents the range from min_value to current_value.
\e CUIR_SLIDER_BAR if the point is within the slider bar's movable element (current_value).
\e CUIR_SLIDER_MAX if the point is in the area that reperesents the range from current_value to max_value.
\e CUIR_ERROR if the point is outside the confines of the CUISlider widget.

If the object is a CUIProgress, possible return values are:
\e CUIR_PROGRESS_LESS if the point is in the area that reperesents the range from min_value to current_value.
\e CUIR_PROGRESS_GREATER if the point is in the area that reperesents the range from current_value to max_value.
\e CUIR_ERROR if the point is outside the confines of the CUIProgress widget.

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	QueryPoint(int16 x, int16 y);

};

#endif //__CUIINTERVAL_H__
