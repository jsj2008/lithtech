//-------------------------------------------------------------------
//
//   MODULE    : CUISTATICTEXT_IMPL.H
//
//   PURPOSE   : defines the CUIStaticText_Impl widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISTATICTEXT_IMPL_H__
#define __CUISTATICTEXT_IMPL_H__


#ifndef __CUIWIDGET_IMPL_H__
#include "cuiwidget_impl.h"
#endif

#ifndef __CUIFORMATTEDPOLYSTRING_H__
#include "cuiformattedpolystring.h"
#endif

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif


class CUIStaticText_Impl : public CUIWidget_Impl
{
	public:
			
		CUIStaticText_Impl(CUIBase* abstract, CUIGUID guid);
		virtual ~CUIStaticText_Impl();


		virtual CUI_WIDGETTYPE	GetType()      { return CUIW_STATICTEXT; }	
		virtual const char*		GetClassName() { return "CUIStaticText"; }	
		
		// set data
		virtual CUI_RESULTTYPE	SetText(const char* pText);
		virtual CUI_RESULTTYPE	SetFont(CUIFont* pFont);
		
		//virtual CUI_RESULTTYPE	SetColor(CUI_ELEMENTTYPE elm, uint32 argb);
		virtual CUI_RESULTTYPE	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);

		virtual CUI_RESULTTYPE	SetGutter(int16 left, int16 right, int16 top, int16 bottom);

		//virtual CUI_RESULTTYPE	SetMultiLine(bool multi);
		virtual CUI_RESULTTYPE	SetWrapWidth(uint16 wrap);
		virtual CUI_RESULTTYPE	SetCharHeight(uint8 height);

		virtual CUI_RESULTTYPE	SetAlignmentH(CUI_ALIGNMENTTYPE align);
		virtual CUI_RESULTTYPE	SetAlignmentV(CUI_ALIGNMENTTYPE align);
		
		// get data
		virtual const char*		GetText();
		virtual CUIFont*		GetFont() { return m_pFont; } 

		// actions
			
	protected:
			
		virtual void		Resize(float w, float h);
		virtual void		Move(float x, float y);
		virtual void		Draw();
		virtual void		AlignTextInWidget();
		
	protected:	
			
		CUIFormattedPolyString*	m_pPolyStr;
		CUIFont*    			m_pFont;
		bool					m_bMultiLine;
		uint16					m_WrapWidth;
		uint8					m_CharHeight;
		uint32					m_pTextColors[4];
};


#endif //__CUISTATICTEXT_IMPL_H__
