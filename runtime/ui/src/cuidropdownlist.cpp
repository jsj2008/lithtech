//-------------------------------------------------------------------
//
//   MODULE    : CUIDROPDOWNLIST.CPP
//
//   PURPOSE   : implements the CUIDropDownList bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIDROPDOWNLIST_H__
#include "cuidropdownlist.h"
#endif

#ifndef __CUIDROPDOWNLIST_IMPL_H__
#include "cuidropdownlist_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIDropDownList::CUIDropDownList(CUIGUID guid) 
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUIDropDownList_Impl(this, guid),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUIDropDownList::~CUIDropDownList()
{
	// destroy resources
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIDropDownList::Open()
{
	return ((CUIDropDownList_Impl*)m_pImpl)->Open();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIDropDownList::Close()
{
	return ((CUIDropDownList_Impl*)m_pImpl)->Close();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIDropDownList::SetDisplayNumber(uint8 display)
{	
	return ((CUIDropDownList_Impl*)m_pImpl)->SetDisplayNumber(display);
}

