//-------------------------------------------------------------------
//
//   MODULE    : CUISLIDER_IMPL.CPP
//
//   PURPOSE   : implements the CUISlider_Impl Utility Class
//
//   CREATED   : 3/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISLIDER_IMPL_H__
#include "cuislider_impl.h"
#endif

#ifndef __CUIPOLYTEX_H__
#include "cuipolytex.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif


// interface database
static ILTTexInterface *pTexInterface;
define_holder(ILTTexInterface, pTexInterface);

static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);


//  ---------------------------------------------------------------------------
CUISlider_Impl::CUISlider_Impl(CUIBase* abstract, CUIGUID guid) :
	CUIInterval_Impl(abstract, guid)
{
	// superclass sets this to left
	m_Halign = CUI_HALIGN_CENTER;

	// set up some reasonable defaults
	m_MinValue = 0;
	m_MaxValue = 100;
	m_CurrentValue = 50;

	// are we vertical or horizontal?
	m_Orientation = CUI_ORIENT_HORIZONTAL;

	m_SliderSizePercent = 25;	
}


//  ---------------------------------------------------------------------------
CUISlider_Impl::~CUISlider_Impl()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUISlider_Impl::SetSliderSizePercent(uint16 percent) 
{
	m_SliderSizePercent = LTCLAMP(percent, 0, 100);
	//m_SliderSizePercent = percent;

	this->RepositionInterval();

	// return an error if the value was clamped
	if ( (percent < 0) || (percent > 100) ) return CUIR_ERROR;

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUISlider_Impl::QueryPoint(int16 x, int16 y)
{
	// is the point within the slider at all?
	if ( (x < m_Rect.x) || (x > m_Rect.x + m_Rect.width) ||
		 (y < m_Rect.y) || (y > m_Rect.y + m_Rect.height) )
	{
		return CUIR_ERROR;
	}

	if (m_Orientation == CUI_ORIENT_HORIZONTAL) {
		// is the point in the minimal area?
		if (x < m_DrawInterval.m_Poly.verts[0].x) {
			return CUIR_SLIDER_MIN;
		}

		// is the point in the slider nubbin?
		if ( (x >= m_DrawInterval.m_Poly.verts[0].x) && 
			 (x < m_DrawInterval.m_Poly.verts[1].x) ) 
		{
			return CUIR_SLIDER_BAR;
		}

		// else it must be in the maximal area
		return CUIR_SLIDER_MAX;
	}

	else { // m_Orientation == CUI_ORIENT_VERTICAL
		// is the point in the minimal area?
		if (y < m_DrawInterval.m_Poly.verts[0].y) {
			return CUIR_SLIDER_MIN;
		}

		// is the point in the slider nubbin?
		if ( (y >= m_DrawInterval.m_Poly.verts[0].y) && 
			 (y < m_DrawInterval.m_Poly.verts[3].y) ) 
		{
			return CUIR_SLIDER_BAR;
		}

		// else it must be in the maximal area
		return CUIR_SLIDER_MAX;
	}
}


//  ---------------------------------------------------------------------------
void CUISlider_Impl::RepositionInterval()
{
	float w,h;
	float delta;

	// this function makes sure the slider nubbin is correctly positioned, 
	// given it's size, the widget's size, and the slider value
	
	if (m_Orientation == CUI_ORIENT_HORIZONTAL) {
		
		w     = (m_Rect.width - m_GutterLeft - m_GutterRight) * m_SliderSizePercent / 100;
		
		delta = (m_CurrentValue * (m_Rect.width - m_GutterLeft - m_GutterRight - w)) / (m_MaxValue - m_MinValue);

		pDrawPrimInternal->SetXYWH(&m_DrawInterval.m_Poly, 
			m_Rect.x + delta + m_GutterRight, 
			m_Rect.y + m_GutterTop, 
			w, 
			m_Rect.height - m_GutterTop - m_GutterBottom);

	}
	else {  // CUI_ORIENT_VERTICAL
	
		h     = (m_Rect.height - m_GutterTop - m_GutterBottom) * m_SliderSizePercent / 100;

		delta = (m_CurrentValue * (m_Rect.height - m_GutterTop - m_GutterBottom - h)) / (m_MaxValue - m_MinValue);

		pDrawPrimInternal->SetXYWH(&m_DrawInterval.m_Poly, 
			m_Rect.x + m_GutterRight, 
			m_Rect.y + delta + m_GutterTop, 
			m_Rect.width - m_GutterLeft - m_GutterRight, 
			h);

	}	
}