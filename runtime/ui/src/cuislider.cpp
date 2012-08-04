//-------------------------------------------------------------------
//
//   MODULE    : CUISLIDER.CPP
//
//   PURPOSE   : Implements the CUISlider bridge Class
//
//   CREATED   : 3/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISLIDER_H__
#include "cuislider.h"
#endif

#ifndef __CUISLIDER_IMPL_H__
#include "cuislider_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUISlider::CUISlider(CUIGUID guid)
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUISlider_Impl(this, guid),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUISlider::~CUISlider()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------


//  ---------------------------------------------------------------------------
uint32 CUISlider::GetSliderSizePercent()
{
	return ((CUISlider_Impl*)m_pImpl)->GetSliderSizePercent();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUISlider::SetSliderSizePercent(uint16 percent)
{
	return ((CUISlider_Impl*)m_pImpl)->SetSliderSizePercent(percent);
}

