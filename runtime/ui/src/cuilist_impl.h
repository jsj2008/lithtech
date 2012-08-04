//-------------------------------------------------------------------
//
//   MODULE    : CUIList_Impl.H
//
//   PURPOSE   : defines the CUIList_Impl widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUILIST_IMPL_H__
#define __CUILIST_IMPL_H__


#ifndef __CUIWIDGET_IMPL_H__
#include "cuiwidget_impl.h"
#endif

#ifndef __CUIPOLYSTRING_H__
#include "cuipolystring.h"
#endif

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif


class CUIList_Impl : public CUIWidget_Impl
{
	public:
			
		CUIList_Impl(CUIBase* abstract, CUIGUID guid);
		virtual ~CUIList_Impl();


		virtual CUI_WIDGETTYPE	GetType()      { return CUIW_LIST; }	
		virtual const char*		GetClassName() { return "CUIList"; }	
		
		// get data
		virtual CUIFont*		GetFont()		{ return m_pFont; }

		// set data
		virtual CUI_RESULTTYPE	SetFont(CUIFont* pFont);
		virtual CUI_RESULTTYPE	SetCharHeight(uint8 height); 
		virtual CUI_RESULTTYPE	SetColors(CUI_ELEMENTTYPE elm, 
										  uint32 argb0,
										  uint32 argb1,
										  uint32 argb2,
										  uint32 argb3);
		
		// actions
		virtual CUI_RESULTTYPE	AddItem(const char* item, int32 index);
		virtual CUI_RESULTTYPE	RemoveItem(int32 index);
		virtual int32			FindItem(const char* pText);
		virtual const char*		GetItem(int32 index);
		virtual CUI_RESULTTYPE	SetSelection(int32 index);
		virtual int32			GetSelection() { return m_Selection; }
		virtual uint32			GetItemCount() { return m_ItemCount; }
		virtual int32			QueryPoint(float x, float y);
		virtual CUI_RESULTTYPE	Scroll(int32 number);

			
	protected:
			
		virtual void		Resize(float w, float h);
		virtual void		Move(float x, float y);
		virtual void		Draw();
		virtual void		AlignTextInWidget();
		
	protected:	

		CUIFont*		m_pFont;
		CUILinkList*	m_pList;
		
		uint32			m_pTextColors[4];
		uint32			m_pSelectedColors[4];
		uint32			m_CharHeight;
		int32			m_Selection;
		int32			m_WindowStart;
		int32			m_ItemCount;


};


#endif //__CUILIST_IMPL_H__
