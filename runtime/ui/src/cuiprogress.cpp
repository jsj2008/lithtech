//-------------------------------------------------------------------
//
//   MODULE    : CUIPROGRESS.CPP
//
//   PURPOSE   : Implements the CUIProgress bridge Class
//
//   CREATED   : 3/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIPOROGRESS_H__
#include "cuiprogress.h"
#endif

#ifndef __CUIPOROGRESS_IMPL_H__
#include "cuiprogress_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIProgress::CUIProgress(CUIGUID guid)
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUIProgress_Impl(this, guid),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUIProgress::~CUIProgress()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIProgress::SetStretchMode(CUI_STRETCHMODE stretch)
{
	return ((CUIProgress_Impl*)m_pImpl)->SetStretchMode(stretch);
}


//  ---------------------------------------------------------------------------
CUI_STRETCHMODE CUIProgress::GetStretchMode()
{
	return ((CUIProgress_Impl*)m_pImpl)->GetStretchMode();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIProgress::SetFillMode(CUI_FILLMODE fill)
{
	return ((CUIProgress_Impl*)m_pImpl)->SetFillMode(fill);
}


//  ---------------------------------------------------------------------------
CUI_FILLMODE CUIProgress::GetFillMode()
{
	return ((CUIProgress_Impl*)m_pImpl)->GetFillMode();
}

