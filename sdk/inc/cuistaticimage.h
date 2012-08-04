//-------------------------------------------------------------------
//
//   MODULE    : CUISTATICIMAGE.H
//
//   PURPOSE   : defines the CUIStaticImage bridge class    
//
//   CREATED   : 1/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISTATICIMAGE_H__
#define __CUISTATICIMAGE_H__


#ifndef __CUIWIDGET_H__
#include "cuiwidget.h"
#endif


/*!  
The CUIStaticImage class is one of the CUI Widgets.  This class
holds a single static image.

CUIE_BG

\see interface ILTWidgetManager

Used for: Text and UI.   */

class CUIStaticImage : public CUIWidget
{
	public:
			
#ifndef DOXYGEN_SHOULD_SKIP_THIS
		CUIStaticImage();
		CUIStaticImage(CUIGUID guid);
		// virtual destructor
		virtual	~CUIStaticImage();
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


	protected:	
			
};


#endif //__CUISTATICIMAGE_H__
