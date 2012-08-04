//-------------------------------------------------------------------
//
//   MODULE    : CUISTATICTEXT.CPP
//
//   PURPOSE   : implements the CUIStaticText bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISTATICTEXT_H__
#include "cuistatictext.h"
#endif

#ifndef __CUISTATICTEXT_IMPL_H__
#include "cuistatictext_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIStaticText::CUIStaticText() 
{
	// this is only for subclasses.  do not use.
}


//  ---------------------------------------------------------------------------
CUIStaticText::CUIStaticText(CUIGUID guid) 
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUIStaticText_Impl(this, guid),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUIStaticText::~CUIStaticText()
{
	
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText::SetText(const char* text)
{
	return ((CUIStaticText_Impl*)m_pImpl)->SetText(text);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText::SetFont(CUIFont* font)
{
	return ((CUIStaticText_Impl*)m_pImpl)->SetFont(font);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText::SetCharHeight(uint8 height)
{
	return ((CUIStaticText_Impl*)m_pImpl)->SetCharHeight(height); 
}


//  ---------------------------------------------------------------------------
const char* CUIStaticText::GetText()
{
	return ((CUIStaticText_Impl*)m_pImpl)->GetText();
}


//  ---------------------------------------------------------------------------
CUIFont* CUIStaticText::GetFont()
{
	return ((CUIStaticText_Impl*)m_pImpl)->GetFont();
}			


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText::SetWrapWidth(uint16 wrap)
{
	return ((CUIStaticText_Impl*)m_pImpl)->SetWrapWidth(wrap);
}
