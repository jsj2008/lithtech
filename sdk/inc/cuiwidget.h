//-------------------------------------------------------------------
//
//   MODULE    : CUIWIDGET.H
//
//   PURPOSE   : defines the CUIWidget bridge class.    
//
//   CREATED   : 1/01
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIWIDGET_H__
#define __CUIWIDGET_H__


#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUIBASE_H__
#include "cuibase.h"
#endif


/*!  
CUIWidget class.
A widget cannot be instantiated.  It is the base class for other widget types

\see interface ILTWidgetManager

Used for: Text and UI.   */

class CUIWidget : public CUIBase
{
	public:
			
#ifndef DOXYGEN_SHOULD_SKIP_THIS
		// virtual destructor
		virtual	~CUIWidget();
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*!
\param elm CUI Element to query
\return HTEXTURE

This function retrieves the current Texture associated with a CUI_ELEMENTTYPE.
CUI_ELEMENTTYPEs are defined in cuitypes.h, and break a widget down into texturable
components.  For example, a CUIWindow has the following CUI Elements:

CUIE_BG			
CUIE_TOP		
CUIE_BOTTOM		
CUIE_LEFT		
CUIE_RIGHT		
CUIE_TR_CORNER	
CUIE_TL_CORNER	
CUIE_BR_CORNER	
CUIE_BL_CORNER	

Used for: Text and UI.
*/
		virtual HTEXTURE	GetTexture(CUI_ELEMENTTYPE elm);
		

/*!
\param elm CUI Element whose color should be set.
\param argb 32-bit color value of the form 0xAARRGGBB.
\return CUI_RESULTTYPE

Use this function to set the background color of a CUI_ELEMENTTYPE.  CUI_ELEMENTTYPEs are
defined in cuitypes.h.

Used for: Text and UI.
*/	
		virtual CUI_RESULTTYPE 	SetColor(CUI_ELEMENTTYPE elm, uint32 argb);


/*!
\param elm CUI Element to set the color of.
\param argb0 32-bit color value of the form 0xAARRGGBB.
\param argb1 32-bit color value of the form 0xAARRGGBB.
\param argb2 32-bit color value of the form 0xAARRGGBB.
\param argb3 32-bit color value of the form 0xAARRGGBB.
\return CUI_RESULTTYPE

Use this function to set separate vertex colors for a CUI_ELEMENTTYPE.  CUI_ELEMENTTYPEs are
defined in cuitypes.h.

Used for: Text and UI.
*/		
		virtual CUI_RESULTTYPE 	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);


/*!
\param elm  CUI_ELEMENTTYPE to set the texture of.
\param hTex Texture to use.
\param tile if true, the texture tiles, if false, it stretches to fir the element
\return CUI_RESULTTYPE

Use this function to set the texture of a CUI_ELEMENTTYPE.  CUI Elements are
defined in cuitypes.h.  Note that the amount of times a texture will tile is limited by
the system architecture, and further note that not all CUI_ELEMENTTYPEs support tiled
textures.

Used for: Text and UI.
*/	
		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
				   					   HTEXTURE hTex, 
				   					   bool tile);

									   
/*!
\param elm  CUI Element to set the texture of.
\param hTex Texture to use.
\param pRect address of CUIRECT containing uv coords. of within hTex
\return CUI_RESULTTYPE

Use this function to set the texture of a CUI Element.  CUI Elements are
defined in cuitypes.h.  Use the CUIRECT to specify a portion of the HTEXTURE to
use for the element.  Furthermore, the height and width of the element will be
changed to reflect the rectangle size.

Used for: Text and UI.
*/	
		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
				   					   HTEXTURE hTex, 
				   					   CUIRECT* pRect);


/*!
\param align  new alignment.
\return CUIR_OK if successful

Use this function to set the horizontal alignment of a widget (not all widgets support
the concept of alignment.  Furthermore, of those that do support alignment, not all support every
kind of alignment.  i.e., setting a CUIStaticImage's alignment to CUI_HALIGN_JUSTIFY makes no sense,
but setting a CUIStaticText widget's alignment to CUI_HALIGN_JUSTIFY will work).

valid horizontal alignments are:
CUI_HALIGN_LEFT
CUI_HALIGN_CENTER
CUI_HALIGN_RIGHT
CUI_HALIGN_JUSTIFY

\see CUI_ALIGNMENTTYPE

Used for: Text and UI.		*/	
		virtual CUI_RESULTTYPE	SetAlignmentH(CUI_ALIGNMENTTYPE align);


/*!
\param align  new alignment.
\return CUIR_OK if successful

Use this function to set the vertical alignment of a widget (not all widgets support
the concept of alignment.  Furthermore, of those that do support alignment, not all support every
kind of alignment).

valid vertical alignments are:
CUI_VALIGN_TOP
CUI_VALIGN_CENTER
CUI_VALIGN_BOTTOM

\see CUI_ALIGNMENTTYPE

Used for: Text and UI.		*/
		virtual CUI_RESULTTYPE	SetAlignmentV(CUI_ALIGNMENTTYPE align);


	protected:	
			
};


#endif //__CUIWIDGET_H__
