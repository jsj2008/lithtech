
//-------------------------------------------------------------------
//
//   MODULE    : CUIWINDOW.H
//
//   PURPOSE   : defines the CUIWindow bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIWINDOW_H__
#define __CUIWINDOW_H__


#ifndef __CUIWIDGET_H__
#include "cuiwidget.h"
#endif


/*!  
CUIWindow class.

\see interface ILTWidgetManager

Used for: Text and UI.   */

class CUIWindow : public CUIWidget
{
	public:
			
#ifndef DOXYGEN_SHOULD_SKIP_THIS

		// CUIWidgets should not be created with new and delete, but should instead
		// be created via the ILTWindowManager's CreateWidget() and DestroyWidget()
		// management functions.

		CUIWindow(CUIGUID guid, 
			      HTEXTURE skin = NULL, 
			      char* table   = NULL);
	
		virtual ~CUIWindow();
		
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
		
};


#endif //__CUIWINDOW_H__
