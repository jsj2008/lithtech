//-------------------------------------------------------------------
//
//   MODULE    : CUIBUTTON_IMPL.CPP
//
//   PURPOSE   : implements the CUIButton_Impl Utility Class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIBUTTON_IMPL_H__
#include "cuibutton_impl.h"
#endif

#ifndef __CUIPOLYTEX_H__
#include "cuipolytex.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __ILTWIDGETMANAGER_H__
#include "iltwidgetmanager.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif


static ILTWidgetManager *pWidgetManager;
define_holder(ILTWidgetManager, pWidgetManager);

static ILTTexInterface *pTexInterface;
define_holder(ILTTexInterface, pTexInterface);

static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);


//  ---------------------------------------------------------------------------
CUIButton_Impl::CUIButton_Impl(CUIBase* abstract, CUIGUID guid) :
	CUIStaticText_Impl(abstract, guid)
{

	// superclass sets this to left
	m_Halign		= CUI_HALIGN_CENTER;
}


//  ---------------------------------------------------------------------------
CUIButton_Impl::~CUIButton_Impl()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
void CUIButton_Impl::Move(float x, float y)
{
	// handles the repositioning specifics
	uint32 v;

	float dx = x - m_Rect.x;
	float dy = y - m_Rect.y;
	
	for (v=0; v<4; v++) {	
		m_drawButtonPressed.m_Poly.verts[v].x += dx;
		m_drawButtonPressed.m_Poly.verts[v].y += dy;

		m_drawButtonHighlighted.m_Poly.verts[v].x += dx;
		m_drawButtonHighlighted.m_Poly.verts[v].y += dy;
	
		m_drawButtonDisabled.m_Poly.verts[v].x += dx;
		m_drawButtonDisabled.m_Poly.verts[v].y += dy;
	}
	

	CUIWidget_Impl::Move(x,y);
}


//  ---------------------------------------------------------------------------
void CUIButton_Impl::Resize(float w, float h)
{
	CUIWidget_Impl::Resize(w, h);

	pDrawPrimInternal->SetXYWH(&m_drawButtonPressed.m_Poly, 
			m_Rect.x, m_Rect.y, m_Rect.width, m_Rect.height);

	pDrawPrimInternal->SetXYWH(&m_drawButtonHighlighted.m_Poly, 
			m_Rect.x, m_Rect.y, m_Rect.width, m_Rect.height);

	pDrawPrimInternal->SetXYWH(&m_drawButtonDisabled.m_Poly, 
			m_Rect.x, m_Rect.y, m_Rect.width, m_Rect.height);

}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIButton_Impl::SetColors(CUI_ELEMENTTYPE elm, 
										 uint32 argb0,
										 uint32 argb1,
										 uint32 argb2,
										 uint32 argb3)
{	
	switch (elm) {
		case CUIE_BUTTON_PRESSED:
			m_drawButtonPressed.SetColors(argb0, argb1, argb2, argb3);
			break;

		case CUIE_BUTTON_HIGHLIGHTED:
			m_drawButtonHighlighted.SetColors(argb0, argb1, argb2, argb3);
			break;

		case CUIE_BUTTON_DISABLED:
			m_drawButtonDisabled.SetColors(argb0, argb1, argb2, argb3);
			break;

		case CUIE_RELEVANT:
			m_drawButtonPressed.SetColors(argb0, argb1, argb2, argb3);
			m_drawButtonHighlighted.SetColors(argb0, argb1, argb2, argb3);
			m_drawButtonDisabled.SetColors(argb0, argb1, argb2, argb3);
			m_DrawBG.SetColors(argb0, argb1, argb2, argb3);
			break;

		default:
			return CUIStaticText_Impl::SetColors(elm, argb0, argb1, argb2, argb3);		
	}
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIButton_Impl::SetTexture(CUI_ELEMENTTYPE elm, 
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
	

	switch (elm) {

		case CUIE_BUTTON_PRESSED:
			m_drawButtonPressed.m_Texture = hTex;
			pDrawPrimInternal->SetUVWH(&m_drawButtonPressed.m_Poly, 
					r.x / tw,
					r.y / th,
					r.width / tw,
					r.height / th);
			break;

		case CUIE_BUTTON_HIGHLIGHTED:
			m_drawButtonHighlighted.m_Texture = hTex;
			pDrawPrimInternal->SetUVWH(&m_drawButtonHighlighted.m_Poly, 
					r.x / tw,
					r.y / th,
					r.width / tw,
					r.height / th);
			break;

		case CUIE_BUTTON_DISABLED:
			m_drawButtonDisabled.m_Texture = hTex;
			pDrawPrimInternal->SetUVWH(&m_drawButtonDisabled.m_Poly, 
					r.x / tw,
					r.y / th,
					r.width / tw,
					r.height / th);
			break;

		default:
			// CUIE_BG is handled by superclass
			return CUIStaticText_Impl::SetTexture(elm, hTex, pRect);
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
void CUIButton_Impl::Draw()
{
	// has no children
	// draw the window BG
	if (m_StateFlags & CUIS_ENABLED) {

		if (m_StateFlags & CUIS_PRESSED) {
			m_drawButtonPressed.Render();

		}
		else if (m_StateFlags & CUIS_HIGHLIGHTED) {
			m_drawButtonHighlighted.Render();
		}
		else {
			// draw the regular old bg
			m_DrawBG.Render();
		}
	}
	else {
		m_drawButtonDisabled.Render();
	}
		
	// draw the text
	if (m_pPolyStr) m_pPolyStr->RenderClipped(&m_Rect);
}
