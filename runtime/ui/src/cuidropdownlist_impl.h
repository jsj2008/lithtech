//-------------------------------------------------------------------
//
//   MODULE    : CUIDropDownList_Impl.H
//
//   PURPOSE   : defines the CUIDropDownList_Impl widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIDROPDOWNLIST_IMPL_H__
#define __CUIDROPDOWNLIST_IMPL_H__


#ifndef __CUILIST_IMPL_H__
#include "cuilist_impl.h"
#endif


class CUIDropDownList_Impl : public CUIList_Impl
{
	public:
			
		CUIDropDownList_Impl(CUIBase* abstract, CUIGUID guid);
		virtual ~CUIDropDownList_Impl();


		virtual CUI_WIDGETTYPE	GetType()      { return CUIW_DROPDOWNLIST; }	
		virtual const char*		GetClassName() { return "CUIDropDownList"; }	
		
		// get data

		// set data
		virtual CUI_RESULTTYPE	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);

		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
				   						   HTEXTURE hTex, 
				   						   CUIRECT* pRect);

		virtual CUI_RESULTTYPE	SetFont(CUIFont* pFont);
		virtual CUI_RESULTTYPE	SetCharHeight(uint8 height);

		virtual CUI_RESULTTYPE	SetGutter(int16 left, int16 right, int16 top, int16 bottom);

		virtual CUI_RESULTTYPE	SetDisplayNumber(uint8 display);

		
		// actions
		virtual int32			QueryPoint(float x, float y);
		//virtual CUI_RESULTTYPE	Scroll(int32 number);
		virtual CUI_RESULTTYPE	Open();
		virtual CUI_RESULTTYPE	Close();

			
	protected:
			
		virtual void		Resize(float w, float h);
		virtual void		Move(float x, float y);
		virtual void		Draw();
		virtual void		AlignTextInWidget();
		virtual void		ResizeDropDown();
		
	protected:	

		CUIPolyTex		m_DrawDropDownBG;
		uint32			m_DisplayNumber;

};


#endif //__CUIDROPDOWNLIST_IMPL_H__
