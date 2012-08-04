//-------------------------------------------------------------------
//
//   MODULE    : CUILIST.CPP
//
//   PURPOSE   : implements the CUIList bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUILIST_H__
#include "cuilist.h"
#endif

#ifndef __CUILIST_IMPL_H__
#include "cuilist_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIList::CUIList() 
{
	// this is only for subclasses.  do not use.
}


//  ---------------------------------------------------------------------------
CUIList::CUIList(CUIGUID guid) 
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUIList_Impl(this, guid),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUIList::~CUIList()
{
	// destroy resources
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------


//  ---------------------------------------------------------------------------
CUIFont* CUIList::GetFont()
{
	return ((CUIList_Impl*)m_pImpl)->GetFont();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList::SetFont(CUIFont* pFont)
{
	return ((CUIList_Impl*)m_pImpl)->SetFont(pFont);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList::SetCharHeight(uint8 height)
{
	return ((CUIList_Impl*)m_pImpl)->SetCharHeight(height); 
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList::AddItem(const char* text, int32 index)
{
	return ((CUIList_Impl*)m_pImpl)->AddItem(text, index);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList::RemoveItem(int32 index)
{
	return ((CUIList_Impl*)m_pImpl)->RemoveItem(index);
}


//  ---------------------------------------------------------------------------
int32 CUIList::FindItem(const char* text)
{
	return ((CUIList_Impl*)m_pImpl)->FindItem(text);
}


//  ---------------------------------------------------------------------------
const char* CUIList::GetItem(int32 index)
{
	return ((CUIList_Impl*)m_pImpl)->GetItem(index);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList::SetSelection(int32 index)
{
	return ((CUIList_Impl*)m_pImpl)->SetSelection(index);
}

	
//  ---------------------------------------------------------------------------
int32 CUIList::GetSelection()
{
	return ((CUIList_Impl*)m_pImpl)->GetSelection();
}


//  ---------------------------------------------------------------------------
uint32 CUIList::GetItemCount()
{
	return ((CUIList_Impl*)m_pImpl)->GetItemCount();
}


//  ---------------------------------------------------------------------------
int32 CUIList::QueryPoint(float x, float y)
{
	return ((CUIList_Impl*)m_pImpl)->QueryPoint(x, y);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIList::Scroll(int32 number)
{
	return ((CUIList_Impl*)m_pImpl)->Scroll(number);
}
