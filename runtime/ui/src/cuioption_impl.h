//-------------------------------------------------------------------
//
//   MODULE    : CUIOPTION_IMPL.H
//
//   PURPOSE   : Defines the CUIOption_Impl widget class
//
//   CREATED   : 4/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIOPTION_IMPL_H__
#define __CUIOPTION_IMPL_H__


#ifndef __CUICHECK_IMPL_H__
#include "cuicheck_impl.h"
#endif


class CUIOption_Impl : public CUICheck_Impl
{
	public:
			
		CUIOption_Impl(CUIBase* abstract, CUIGUID guid);
		virtual ~CUIOption_Impl();
		

		// get data		
		virtual CUI_WIDGETTYPE	GetType()      { return CUIW_OPTION; }	
		virtual const char*		GetClassName() { return "CUIOption"; }	


		// set data
		virtual CUI_RESULTTYPE	SetState(uint32 flags);
		//virtual CUI_RESULTTYPE	UnsetState(uint32 flags);

		virtual CUI_RESULTTYPE	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);

		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
				   						   HTEXTURE hTex, 
				   						   CUIRECT* pRect);
				
	protected:

		//virtual void			Reposition(); 
		//virtual void			Draw();
		//virtual void			Move(float x, float y);
		//virtual void			Resize(float w, float h);
			
		//CUIPolyTex	m_DrawNormal; == same as m_DrawBG
		//CUIPolyTex		m_drawOptionPressed;
		//CUIPolyTex		m_drawOptionHighlighted;
		//CUIPolyTex		m_drawOptionDisabled;	
		
};

#endif //__CUIOPTION_IMPL_H__
