//-------------------------------------------------------------------
//
//   MODULE    : CUIWINDOW_IMPL.H
//
//   PURPOSE   : defines the CUIWindowe_Impl widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIWINDOW_IMPL_H__
#define __CUIWINDOW_IMPL_H__


#ifndef __CUIWIDGET_IMPL_H__
#include "cuiwidget_impl.h"
#endif

#ifndef __CUIPOLYTEX_H__
#include "cuipolytex.h"
#endif


class CUIWindow_Impl : public CUIWidget_Impl
{
	public:
			
		CUIWindow_Impl(CUIBase* abstract,
					   CUIGUID guid, 
					   HTEXTURE skin = NULL, 
					   char* table   = NULL);
	
		~CUIWindow_Impl();
		
				
		// get data	
		virtual CUI_WIDGETTYPE			GetType()      { return CUIW_WINDOW;  }
		virtual const char*				GetClassName() { return "CUIWindow"; }	
		
		virtual HTEXTURE				GetTexture(CUI_ELEMENTTYPE elm);
		

		// set data
		virtual CUI_RESULTTYPE			SetTexture(CUI_ELEMENTTYPE elm, 
				   								   HTEXTURE hTex, 
				   								   bool tile);

		virtual CUI_RESULTTYPE			SetTexture(CUI_ELEMENTTYPE elm, 
				   								   HTEXTURE hTex, 
				   								   CUIRECT* pRect);
		
		//virtual CUI_RESULTTYPE 			SetColor(CUI_ELEMENTTYPE elm, uint32 argb);
		virtual CUI_RESULTTYPE 			SetColors(CUI_ELEMENTTYPE elm, 
												  uint32 argb0,
												  uint32 argb1,
												  uint32 argb2,
												  uint32 argb3);
		
		// actions
		
	protected:	
			
		virtual void		Draw();
		virtual void		Move(float x, float y);
		virtual void		Resize(float w, float h);
	
		
	protected:

		// window polygons		
		CUIPolyTex			m_DrawTOP;
		CUIPolyTex			m_DrawBOTTOM;
		CUIPolyTex			m_DrawLEFT;
		CUIPolyTex			m_DrawRIGHT;
		
		CUIPolyTex			m_DrawTR_CORNER;
		CUIPolyTex			m_DrawTL_CORNER;
		CUIPolyTex			m_DrawBR_CORNER;
		CUIPolyTex			m_DrawBL_CORNER;		
};


#endif //__CUIWINDOW_IMPL_H__
