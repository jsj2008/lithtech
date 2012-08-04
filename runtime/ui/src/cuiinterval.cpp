//-------------------------------------------------------------------
//
//   MODULE    : CUISLIDER.CPP
//
//   PURPOSE   : Implements the CUIInterval bridge Class
//
//   CREATED   : 3/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIINTERVAL_H__
#include "cuiinterval.h"
#endif

#ifndef __CUIINTERVAL_IMPL_H__
#include "cuiinterval_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIInterval::~CUIInterval()
{

}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIInterval::GetRange(int32* pMin, int32* pMax)
{
	return ((CUIInterval_Impl*)m_pImpl)->GetRange(pMin, pMax);
}


//  ---------------------------------------------------------------------------
int32 CUIInterval::GetMinValue()
{
	return ((CUIInterval_Impl*)m_pImpl)->GetMinValue();
}


//  ---------------------------------------------------------------------------
int32 CUIInterval::GetMaxValue()
{
	return ((CUIInterval_Impl*)m_pImpl)->GetMaxValue();
}


//  ---------------------------------------------------------------------------
int32 CUIInterval::GetCurrentValue()
{
	return ((CUIInterval_Impl*)m_pImpl)->GetCurrentValue();
}


//  ---------------------------------------------------------------------------
CUI_ORIENTATIONTYPE CUIInterval::GetOrientation()
{
	return ((CUIInterval_Impl*)m_pImpl)->GetOrientation();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIInterval::SetMaxValue(int32 max)
{
	return ((CUIInterval_Impl*)m_pImpl)->SetMaxValue(max);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIInterval::SetMinValue(int32 min)
{
	return ((CUIInterval_Impl*)m_pImpl)->SetMinValue(min);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIInterval::SetRange(int32 min, int32 max)
{
	return ((CUIInterval_Impl*)m_pImpl)->SetRange(min, max);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIInterval::SetCurrentValue(int32 val)
{
	return ((CUIInterval_Impl*)m_pImpl)->SetCurrentValue(val);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIInterval::SetOrientation(CUI_ORIENTATIONTYPE orient)
{
	return ((CUIInterval_Impl*)m_pImpl)->SetOrientation(orient);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIInterval::Increment(int32 inc)
{
	return ((CUIInterval_Impl*)m_pImpl)->Increment(inc);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIInterval::IncrementPercent(int32 percent)
{
	return ((CUIInterval_Impl*)m_pImpl)->IncrementPercent(percent);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIInterval::QueryPoint(int16 x, int16 y)
{
	return ((CUIInterval_Impl*)m_pImpl)->QueryPoint(x, y);
}
