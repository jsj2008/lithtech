//-------------------------------------------------------------------
//
//   MODULE    : CUIBUTTON_IMPL.H
//
//   PURPOSE   : defines the CUIButton_Impl widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIBUTTON_IMPL_H__
#define __CUIBUTTON_IMPL_H__


#ifndef __CUISTATICTEXT_IMPL_H__
#include "cuistatictext_impl.h"
#endif


class CUIButton_Impl : public CUIStaticText_Impl
{
	public:
			
		CUIButton_Impl(CUIBase* abstract, CUIGUID guid);
		virtual ~CUIButton_Impl();
		

		// get data		
		virtual CUI_WIDGETTYPE	GetType()      { return CUIW_BUTTON; }	
		virtual const char*		GetClassName() { return "CUIButton"; }	


		// set data
		virtual CUI_RESULTTYPE	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);

		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
				   						   HTEXTURE hTex, 
				   						   CUIRECT* pRect);
				
	protected:

		virtual void			Draw();
		virtual void			Move(float x, float y);
		virtual void			Resize(float w, float h);
			
		//CUIPolyTex	m_DrawNormal; == same as m_DrawBG
		CUIPolyTex		m_drawButtonPressed;
		CUIPolyTex		m_drawButtonHighlighted;
		CUIPolyTex		m_drawButtonDisabled;	
		
};

#endif //__CUIBUTTON_IMPL_H__
