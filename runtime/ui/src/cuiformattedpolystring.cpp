//-------------------------------------------------------------------
//
//   MODULE    : CUIFORMATTEDPOLYSTRING.CPP
//
//   PURPOSE   : implements the CUFormattedPolyString bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIFORMATTEDPOLYSTRING_H__
#include "cuiformattedpolystring.h"
#endif

#ifndef __CUIFORMATTEDPOLYSTRING_IMPL_H__
#include "cuiformattedpolystring_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIFormattedPolyString::CUIFormattedPolyString(CUIFont* pFont, 
							 const char* pText,
							 float x,
							 float y,
							 CUI_ALIGNMENTTYPE alignment)
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUIFormattedPolyString_Impl(pFont, pText, x, y, alignment),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUIFormattedPolyString::~CUIFormattedPolyString() 
{
	// release uinique resources
}
	

//  ---------------------------------------------------------------------------
const char*	CUIFormattedPolyString::GetClassName()
{
	return m_pImpl->GetClassName();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIFormattedPolyString::SetAlignmentH(CUI_ALIGNMENTTYPE align)
{
	return ((CUIFormattedPolyString_Impl*)m_pImpl)->SetAlignmentH(align);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIFormattedPolyString::SetWrapWidth(uint16 wrap)
{
	return ((CUIFormattedPolyString_Impl*)m_pImpl)->SetWrapWidth(wrap);
}


