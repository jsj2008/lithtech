//-------------------------------------------------------------------
//
//   MODULE    : CUIPROGRESS_IMPL.CPP
//
//   PURPOSE   : implements the CUIProgress_Impl Utility Class
//
//   CREATED   : 3/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIPROGRESS_IMPL_H__
#include "cuiprogress_impl.h"
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
CUIProgress_Impl::CUIProgress_Impl(CUIBase* abstract, CUIGUID guid) :
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
	// stretch or fill?
	m_Stretch = CUI_STRETCH_TRUE;
	// which direction?
	m_Fill = CUI_FILL_LEFTTORIGHT;
}


//  ---------------------------------------------------------------------------
CUIProgress_Impl::~CUIProgress_Impl()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIProgress_Impl::SetStretchMode(CUI_STRETCHMODE stretch)
{
	switch (stretch) {

		case CUI_STRETCH_FALSE:
			// fall through
		case CUI_STRETCH_TRUE:
			m_Stretch = stretch;
			break;

		default:
			return CUIR_ERROR;
	}	

	this->RepositionInterval();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIProgress_Impl::SetFillMode(CUI_FILLMODE fill)
{
	switch (fill) {

		case CUI_FILL_LEFTTORIGHT: // same as CUI_FILL_TOPTOBOTTOM
			// fall through
		case CUI_FILL_RIGHTTOLEFT: // same as CUI_FILL_BOTTOMTOTOP
			m_Fill = fill;
			break;

		default:
			return CUIR_ERROR;
	}	

	this->RepositionInterval();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIProgress_Impl::QueryPoint(int16 x, int16 y)
{
	// is the point within the progress indicator at all?
	if ( (x < m_Rect.x) || (x > m_Rect.x + m_Rect.width) ||
		 (y < m_Rect.y) || (y > m_Rect.y + m_Rect.height) )
	{
		return CUIR_ERROR;
	}

	if (m_Orientation == CUI_ORIENT_HORIZONTAL) {
		// is the point in the fill area?
		if (x < m_DrawInterval.m_Poly.verts[1].x) {
			return CUIR_PROGRESS_LESS;
		}

		// else it must be in the empty area
		return CUIR_PROGRESS_GREATER;
	}

	else { // m_Orientation == CUI_ORIENT_VERTICAL
		// is the point in the fill area?
		if (y > m_DrawInterval.m_Poly.verts[0].y) {
			return CUIR_PROGRESS_LESS;
		}

		// else it must be in the empty area
		return CUIR_PROGRESS_GREATER;
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIProgress_Impl::SetGutter(int16 left, int16 right, int16 top, int16 bottom)
{
	// set the gutter
	CUIWidget_Impl::SetGutter(left, right, top, bottom);
	
	// reposition with new gutter
	this->RepositionInterval();

	// return ok
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
void CUIProgress_Impl::RepositionInterval()
{
	//float w,h; //,x,y;
	float delta;

	// this function makes sure the progress indicator's fill is correctly positioned, 
	// given it's size, the widget's size, and the indicator value

	if (m_Stretch == CUI_STRETCH_TRUE) {
		pDrawPrimInternal->SetUVWH(&m_DrawInterval.m_Poly, 
			m_TexRect.x,
			m_TexRect.y,
			m_TexRect.width,
			m_TexRect.height);
	}


	if (m_Orientation == CUI_ORIENT_HORIZONTAL) {

		if (m_Fill == CUI_FILL_LEFTTORIGHT) {

			if (m_Stretch == CUI_STRETCH_FALSE) {
				pDrawPrimInternal->SetUVWH(&m_DrawInterval.m_Poly, 
					m_TexRect.x,
					m_TexRect.y,
					m_TexRect.width * m_CurrentValue / (m_MaxValue - m_MinValue),
					m_TexRect.height);
			}

			delta = (m_CurrentValue * (m_Rect.width - m_GutterRight - m_GutterLeft)) / (m_MaxValue - m_MinValue);

			pDrawPrimInternal->SetXYWH(&m_DrawInterval.m_Poly, 
				m_Rect.x + m_GutterLeft, 
				m_Rect.y + m_GutterTop,
				delta, 
				m_Rect.height - m_GutterTop - m_GutterBottom);

		}
		else { // CUI_FILL_LEFTTORIGHT

			if (m_Stretch == CUI_STRETCH_FALSE) {
				pDrawPrimInternal->SetUVWH(&m_DrawInterval.m_Poly, 
					m_TexRect.x + m_TexRect.width - (m_TexRect.width * m_CurrentValue / (m_MaxValue - m_MinValue)),
					m_TexRect.y,
					m_TexRect.width * m_CurrentValue / (m_MaxValue - m_MinValue),
					m_TexRect.height);
			}

			delta = (m_CurrentValue * (m_Rect.width - m_GutterRight - m_GutterLeft)) / (m_MaxValue - m_MinValue);

			pDrawPrimInternal->SetXYWH(&m_DrawInterval.m_Poly, 
				m_Rect.x + m_Rect.width - m_GutterRight - delta, 
				m_Rect.y + m_GutterTop,
				delta, 
				m_Rect.height - m_GutterTop - m_GutterBottom);
		}
	}
	else {  // CUI_ORIENT_VERTICAL

		if (m_Fill == CUI_FILL_TOPTOBOTTOM) {

			if (m_Stretch == CUI_STRETCH_FALSE) {
				pDrawPrimInternal->SetUVWH(&m_DrawInterval.m_Poly, 
					m_TexRect.x,
					m_TexRect.y,
					m_TexRect.width,
					m_TexRect.height * m_CurrentValue / (m_MaxValue - m_MinValue));
			}
			
			delta = (m_CurrentValue * (m_Rect.height - m_GutterTop - m_GutterBottom)) / (m_MaxValue - m_MinValue);

			pDrawPrimInternal->SetXYWH(&m_DrawInterval.m_Poly, 
				m_Rect.x + m_GutterLeft,
				m_Rect.y + m_GutterTop, 
				m_Rect.width - m_GutterLeft - m_GutterRight, 
				delta);
		}
		else { // CUI_FILL_TOPTOBOTTOM

			if (m_Stretch == CUI_STRETCH_FALSE) {
				pDrawPrimInternal->SetUVWH(&m_DrawInterval.m_Poly, 
					m_TexRect.x,
					m_TexRect.y + m_TexRect.height - (m_TexRect.height * m_CurrentValue / (m_MaxValue - m_MinValue)),
					m_TexRect.width,
					m_TexRect.height * m_CurrentValue / (m_MaxValue - m_MinValue));
			}
			
			delta = (m_CurrentValue * (m_Rect.height - m_GutterTop - m_GutterBottom)) / (m_MaxValue - m_MinValue);

			pDrawPrimInternal->SetXYWH(&m_DrawInterval.m_Poly, 
				m_Rect.x + m_GutterLeft,
				m_Rect.y + m_Rect.height - m_GutterTop - delta, 
				m_Rect.width - m_GutterLeft - m_GutterRight, 
				delta);
		}
	}
}