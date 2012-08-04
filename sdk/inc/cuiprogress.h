//-------------------------------------------------------------------
//
//   MODULE    : CUIPROGRESS.H
//
//   PURPOSE   : defines the interface for The CUIProgress Widget
//
//   CREATED   : 3/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIPROGRESS_H__
#define __CUIPROGRESS_H__


#ifndef __CUIINTERVAL_H__
#include "cuiinterval.h"
#endif


/*!  
CUIProgress is a CUI widgets which displays a filling or stretching image
over a background to indicate progress.

CUIProgress starts with a default \e min of 0, \e max of 100, and a \e current
\e value of 50.

CUIE_BG
CUIE_PROGRESS_INDICATOR

\see ILTWidgetManager
\see CUI_ELEMENTTYPE

Used for: Text and UI.
*/


class CUIProgress : public CUIInterval
{
	public:
	
#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIWidgets should not be created with new and delete, but should instead
		// be created via the ILTWindowManager's CreateWidget() and DestroyWidget()
		// management functions.
		CUIProgress(CUIGUID guid);
		virtual ~CUIProgress();

#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*!
\param stretch CUI_STRETCHMODE.
\return CUI_RESULTTYPE.

If \e stretch is CUI_STRETCH_TRUE, the CUIE_PROGRESS_INDICATOR element's texture stretches to fit
the width of the indicator, and grows or shrinks based on the object's current value.

If \e stretch is CUI_STRETCH_FALSE, the CUIE_PROGRESS_INDICATOR element draws a partial texture.
The amount of the texture drawn is based on the object's current value.

\see SetTexture()

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	SetStretchMode(CUI_STRETCHMODE stretch);

/*!
\return CUI_STRETCH_TRUE or CUI_STRETCH_FALSE.

Returns whether the objects texture is drawn in a stretch or fill mode.

\see SetStretchMode()

Used for:  Text and UI.
*/
		virtual CUI_STRETCHMODE			GetStretchMode();


/*!
\param stretch CUI_FILLMODE.
\return CUI_RESULTTYPE.

If \e fill is CUI_FILL_LEFTTORIGHT, the CUIE_PROGRESS_INDICATOR element draws
from left to right when the orientation is CUI_ORIENT_HORIZONTAL.

If \e fill is CUI_FILL_RIGHTTOLEFT, the CUIE_PROGRESS_INDICATOR element draws
from right to left when the orientation is CUI_ORIENT_HORIZONTAL.

If \e fill is CUI_FILL_TOPTOBOTTOM, the CUIE_PROGRESS_INDICATOR element draws
from top to bottom when the orientation is CUI_ORIENT_VERTICAL.

If \e fill is CUI_FILL_BOTTOMTOTOP, the CUIE_PROGRESS_INDICATOR element draws
from bottom to top when the orientation is CUI_ORIENT_VERTICAL.

Used for:  Text and UI.
*/
		virtual CUI_RESULTTYPE	SetFillMode(CUI_FILLMODE fill);

/*!
\return CUI_FILLMODE.

Returns whether the objects texture is drawn in a stretch or fill mode.

\see SetFillMode()

Used for:  Text and UI.
*/
		virtual CUI_FILLMODE			GetFillMode();

};

#endif //__CUIPROGRESS_H__
