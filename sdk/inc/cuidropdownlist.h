//-------------------------------------------------------------------
//
//   MODULE    : CUIList.H
//
//   PURPOSE   : defines the CUIList bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIDROPDOWNLIST_H__
#define __CUIDROPDOWNLIST_H__


#ifndef __CUILIST_H__
#include "cuilist.h"
#endif

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif


/*!  
CUIList class.  This class is uses a collection of CUIPolyStrings to display a
list of text items over a background.  If the list is expanded, an additional 
expansion image is drawn.

CUIE_BG
CUIE_CUIE_DROPDOWN_BG

\see class CUIPolyString

Used for: Text and UI.   */

class CUIDropDownList : public CUIList
{
	public:
			

#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIWidgets should not be created with new and delete, but should instead
		// be created via the ILTWindowManager's CreateWidget() and DestroyWidget()
		// management functions.
		//CUIDropDownList();
		CUIDropDownList(CUIGUID guid);
		virtual ~CUIDropDownList();
		
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*!  
\return CUIR_OK

Use this function to open (drop) a drop-down list.

Used for: Text and UI.   */		
		virtual CUI_RESULTTYPE	Open();

/*!  
\return CUIR_OK

Use this function to close (raise) a drop-down list.

Used for: Text and UI.   */		
		virtual CUI_RESULTTYPE	Close();

/*!  
\return CUIR_OK

Use this function to set the number of items displayed when a drop-down list
is open.

Used for: Text and UI.   */		
		virtual CUI_RESULTTYPE	SetDisplayNumber(uint8 display);
		
};

#endif //__CUILIST_H__
