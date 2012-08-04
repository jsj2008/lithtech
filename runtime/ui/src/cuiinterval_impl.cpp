//-------------------------------------------------------------------
//
//   MODULE    : CUIINTERVAL_IMPL.CPP
//
//   PURPOSE   : Implements the CUIInterval_Impl Utility Class
//
//   CREATED   : 3/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIINTERVAL_IMPL_H__
#include "cuiinterval_impl.h"
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
CUIInterval_Impl::CUIInterval_Impl(CUIBase* abstract, CUIGUID guid) :
	CUIStaticImage_Impl(abstract, guid)
{
	// superclass sets this to left
	m_Halign = CUI_HALIGN_CENTER;

	// set up some reasonable defaults
	m_MinValue = 0;
	m_MaxValue = 100;
	m_CurrentValue = 50;

	// are we vertical or horizontal?
	m_Orientation = CUI_ORIENT_HORIZONTAL;

	memset(&m_TexRect, 0, sizeof(CUIRECT));
}


//  ---------------------------------------------------------------------------
CUIInterval_Impl::~CUIInterval_Impl()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIInterval_Impl::GetRange(int32* pMin, int32* pMax) 
{
	*pMin = m_MinValue;
	*pMax = m_MaxValue;

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIInterval_Impl::SetTexture(CUI_ELEMENTTYPE elm, 
										   HTEXTURE hTex, 
										   CUIRECT* pRect)
{
	CUIRECT r;
	float tw, th;
	uint32 texw, texh;

	// if no rect is provided, gracefully set texture size to 0
	memset (&r, 0, sizeof(CUIRECT));
	if (pRect) {
		memcpy(&r, pRect, sizeof(CUIRECT));
	}

	if (hTex) {
		pTexInterface->GetTextureDims(hTex, texw, texh);
		tw = (float)texw;
		th = (float)texh;
	}
	else {
		tw = 1;
		th = 1;
	}
	
	// now set the tex
	switch (elm) {

		case CUIE_SLIDER:
		case CUIE_PROGRESS_INDICATOR:

			m_TexRect.x      = r.x / tw;
			m_TexRect.y      = r.y / th;
			m_TexRect.width  = r.width / tw;
			m_TexRect.height = r.height / th;
			
			m_DrawInterval.m_Texture = hTex;

			pDrawPrimInternal->SetUVWH(&m_DrawInterval.m_Poly, 
					m_TexRect.x,
					m_TexRect.y,
					m_TexRect.width,
					m_TexRect.height);
			
			RepositionInterval();

			break;

		default:
			// CUIE_BG is handled by superclass
			return CUIStaticImage_Impl::SetTexture(elm, hTex, pRect);
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIInterval_Impl::SetColors(CUI_ELEMENTTYPE elm, 
										 uint32 argb0,
										 uint32 argb1,
										 uint32 argb2,
										 uint32 argb3)
{	
	switch (elm) {
		case CUIE_SLIDER:
		case CUIE_PROGRESS_INDICATOR:
			m_DrawInterval.SetColors(argb0, argb1, argb2, argb3);
			break;

		case CUIE_RELEVANT:
			m_DrawInterval.SetColors(argb0, argb1, argb2, argb3);
			m_DrawBG.SetColors(argb0, argb1, argb2, argb3);
			break;

		default:
			return CUIWidget_Impl::SetColors(elm, argb0, argb1, argb2, argb3);		
	}
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIInterval_Impl::SetMaxValue(int32 max) 
{
	if (max <= m_MinValue) return CUIR_ERROR;

	m_MaxValue = max;

	RepositionInterval();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIInterval_Impl::SetMinValue(int32 min) 
{
	if (min >= m_MaxValue) return CUIR_ERROR;

	m_MinValue = min;

	RepositionInterval();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIInterval_Impl::SetRange(int32 min, int32 max) 
{
	if (max <= min) return CUIR_ERROR;

	m_MinValue = min;
	m_MaxValue = max;

	RepositionInterval();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIInterval_Impl::SetCurrentValue(int32 val) 
{
	// clamp the current value to between min & max 
	m_CurrentValue = LTCLAMP(val, m_MinValue, m_MaxValue);

	RepositionInterval();

	// return an error if the value was clamped
	if ( (val < m_MinValue) || (val > m_MaxValue) ) return CUIR_ERROR;

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIInterval_Impl::SetOrientation(CUI_ORIENTATIONTYPE orient) 
{
	m_Orientation = orient;

	RepositionInterval();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIInterval_Impl::SetGutter(int16 left, int16 right, 
											int16 top, int16 bottom)
{
	CUIStaticImage_Impl::SetGutter(left, right, top, bottom);

	RepositionInterval();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIInterval_Impl::Increment(int32 inc) 
{
	m_CurrentValue += inc;

	// clamp the current value to between min & max 
	m_CurrentValue = LTCLAMP(m_CurrentValue, m_MinValue, m_MaxValue);

	RepositionInterval();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIInterval_Impl::IncrementPercent(int32 percent) 
{
	percent = LTCLAMP(percent, 0, 100);

	m_CurrentValue += (m_MaxValue - m_MinValue) * percent / 100;
	
	// clamp the current value to between min & max 
	m_CurrentValue = LTCLAMP(m_CurrentValue, m_MinValue, m_MaxValue);
	
	RepositionInterval();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
void CUIInterval_Impl::Move(float x, float y)
{
	// handles the repositioning specifics
	uint32 v;
	float dx = x - m_Rect.x;
	float dy = y - m_Rect.y;
	
	// move the slider nubbin
	for (v=0; v<4; v++) {	
		m_DrawInterval.m_Poly.verts[v].x += dx;
		m_DrawInterval.m_Poly.verts[v].y += dy;
	}

	CUIStaticImage_Impl::Move(x,y);
}


//  ---------------------------------------------------------------------------
void CUIInterval_Impl::Resize(float w, float h)
{
	// resize the background
	CUIStaticImage_Impl::Resize(w, h);

	// position the widget accordingly
	RepositionInterval();
}


//  ---------------------------------------------------------------------------
void CUIInterval_Impl::Draw()
{
	// draw the BG first (under the nubbin)
	CUIStaticImage_Impl::Draw();

	// render the nubbin
	m_DrawInterval.Render();
}

