//-------------------------------------------------------------------
//
//   MODULE    : CUICHECK_IMPL.H
//
//   PURPOSE   : Defines the CUICheck_Impl widget class
//
//   CREATED   : 4/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUICHECK_IMPL_H__
#define __CUICHECK_IMPL_H__


#ifndef __CUIBUTTON_IMPL_H__
#include "cuibutton_impl.h"
#endif


class CUICheck_Impl : public CUIButton_Impl
{
	public:
			
		CUICheck_Impl(CUIBase* abstract, CUIGUID guid);
		virtual ~CUICheck_Impl();
		

		// get data		
		virtual CUI_WIDGETTYPE	GetType()      { return CUIW_CHECK; }	
		virtual const char*		GetClassName() { return "CUICheck"; }	

		virtual bool			GetValue();
		


		// set data
		virtual bool			SetValue(bool value);

		virtual CUI_RESULTTYPE	SetTexture(CUI_ELEMENTTYPE elm, 
				   						   HTEXTURE hTex, 
				   						   CUIRECT* pRect);

		virtual CUI_RESULTTYPE	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);

		virtual CUI_RESULTTYPE	SetAlignmentH(CUI_ALIGNMENTTYPE align);
		virtual CUI_RESULTTYPE	SetAlignmentV(CUI_ALIGNMENTTYPE align);

		virtual CUI_RESULTTYPE	SetGutter(int16 left, int16 right, int16 top, int16 bottom);
		
		virtual CUI_RESULTTYPE	SetText(const char* pText);

		// actions
		virtual bool			Toggle();
				
	protected:

		virtual void			Reposition(); 
		virtual void			Draw();

		// overload this so we can offset the text WRT the check texture
		virtual void			AlignTextInWidget();


		//virtual void			Move(float x, float y);
		//virtual void			Resize(float w, float h);
			
		//CUIPolyTex			m_DrawNormal; == same as m_DrawBG
		//CUIPolyTex			m_drawCheckPressed;
		//CUIPolyTex			m_drawCheckHighlighted;
		//CUIPolyTex			m_drawCheckDisabled;	
		
};

#endif //__CUICHECK_IMPL_H__
