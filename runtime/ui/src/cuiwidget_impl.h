
//-------------------------------------------------------------------
//
//   MODULE    : CUIWIDGET_IMPL.H
//
//   PURPOSE   : defines the CUIWdget_Impl widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIWIDGET_IMPL_H__
#define __CUIWIDGET_IMPL_H__


#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUIBASE_IMPL_H__
#include "cuibase_impl.h"
#endif

#ifndef __CUIPOLYTEX_H__
#include "cuipolytex.h"
#endif


class CUIWidget_Impl : public CUIBase_Impl
{
	public:
			
		CUIWidget_Impl(CUIBase* pAbstract, CUIGUID guid);
		virtual	~CUIWidget_Impl();
		
		// get data		
		virtual CUI_WIDGETTYPE	GetType()	   { return CUIW_WIDGET; }
		virtual const char*		GetClassName() { return "CUIWidget"; }	
		
		virtual HTEXTURE		GetTexture(CUI_ELEMENTTYPE elm);
		
		// set data
		virtual CUI_RESULTTYPE 	SetColor(CUI_ELEMENTTYPE elm, uint32 argb);
		virtual CUI_RESULTTYPE 	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);

		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
				   						   HTEXTURE hTex, 
				   						   bool bTile);

		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
				   						   HTEXTURE hTex, 
				   						   CUIRECT* pRect);

		virtual CUI_RESULTTYPE	SetAlignmentH(CUI_ALIGNMENTTYPE align);
		virtual CUI_RESULTTYPE	SetAlignmentV(CUI_ALIGNMENTTYPE align);


	protected:	
			
		virtual void		Draw();
		virtual void		Move(float x, float y);
		virtual void		Resize(float w, float h);
		
	protected:	
			
		bool				m_ZoneEvent;
		CUI_ALIGNMENTTYPE	m_Valign;
		CUI_ALIGNMENTTYPE	m_Halign;
		CUIPolyTex			m_DrawBG;	
			
};


#endif //__CUIWIDGET_IMPL_H__
