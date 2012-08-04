//-------------------------------------------------------------------
//
//   MODULE    : CUICHECK.CPP
//
//   PURPOSE   : Implements the CUICheck bridge Class
//
//   CREATED   : 4/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUICHECK_H__
#include "cuicheck.h"
#endif

#ifndef __CUICHECK_IMPL_H__
#include "cuicheck_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUICheck::CUICheck() 
{
	// this is only for subclasses.  do not use.
}


//  ---------------------------------------------------------------------------
CUICheck::CUICheck(CUIGUID guid)
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUICheck_Impl(this, guid),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUICheck::~CUICheck()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------


//	---------------------------------------------------------------------------
bool CUICheck::GetValue()
{
	return ((CUICheck_Impl*)m_pImpl)->GetValue(); 
}


//	---------------------------------------------------------------------------
bool CUICheck::SetValue(bool value)
{
	return ((CUICheck_Impl*)m_pImpl)->SetValue(value); 
}


//	---------------------------------------------------------------------------
bool CUICheck::Toggle()
{
	return ((CUICheck_Impl*)m_pImpl)->Toggle(); 
}