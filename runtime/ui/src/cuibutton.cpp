//-------------------------------------------------------------------
//
//   MODULE    : CUIBUTTON.CPP
//
//   PURPOSE   : implements the CUIButton bridge Class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIBUTTON_H__
#include "cuibutton.h"
#endif

#ifndef __CUIBUTTON_IMPL_H__
#include "cuibutton_impl.h"
#endif

//  ---------------------------------------------------------------------------
CUIButton::CUIButton() 
{
	// this is only for subclasses.  do not use.
}


//  ---------------------------------------------------------------------------
CUIButton::CUIButton(CUIGUID guid)
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUIButton_Impl(this, guid),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUIButton::~CUIButton()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------
