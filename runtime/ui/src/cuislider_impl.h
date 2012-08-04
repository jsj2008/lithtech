//-------------------------------------------------------------------
//
//   MODULE    : CUISLIDER_IMPL.H
//
//   PURPOSE   : defines the CUISliderImage_Impl widget class
//
//   CREATED   : 3/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISLIDER_IMPL_H__
#define __CUISLIDER_IMPL_H__


#ifndef __CUI_H__
#include "cui.h"
#endif

#ifndef __CUIINTERVAL_IMPL_H__
#include "cuiinterval_impl.h"
#endif


class CUISlider_Impl : public CUIInterval_Impl
{
	public:
			
		CUISlider_Impl(CUIBase* abstract, CUIGUID guid);
		virtual	~CUISlider_Impl();
		
		// get data		
		virtual CUI_WIDGETTYPE	GetType()	   { return CUIW_SLIDER; }
		virtual const char*		GetClassName() { return "CUISlider"; }	

		virtual uint32			GetSliderSizePercent() { return m_SliderSizePercent; }
		virtual CUI_RESULTTYPE	SetSliderSizePercent(uint16 percent);

		virtual CUI_RESULTTYPE	QueryPoint(int16 x, int16 y);
		

	protected:	

		virtual void RepositionInterval();


	protected:
		
		uint16				m_SliderSizePercent;

};


#endif //__CUISLIDER_IMPL_H__
