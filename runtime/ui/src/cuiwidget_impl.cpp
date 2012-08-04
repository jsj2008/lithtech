//-------------------------------------------------------------------
//
//   MODULE    : CUIWIDGET_IMPL.CPP
//
//   PURPOSE   : implements the CUIWidget_Impl widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIWIDGET_IMPL_H__
#include "cuiwidget_impl.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif


// use the internal drawprim interface
static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);

static ILTTexInterface *pTexInterface;
define_holder(ILTTexInterface, pTexInterface);


//  ---------------------------------------------------------------------------
CUIWidget_Impl::CUIWidget_Impl(CUIBase* abstract, CUIGUID guid) :
	CUIBase_Impl(abstract, guid)
{
	m_ZoneEvent = false;

	m_BGColor = CUI_DEFAULT_WIDGET_COLOR | CUI_SYSTEM_OPAQUE;
	m_Valign  = CUI_VALIGN_CENTER;
	m_Halign  = CUI_HALIGN_CENTER;
}


//  ---------------------------------------------------------------------------
CUIWidget_Impl::~CUIWidget_Impl()
{
	// delete resources
}

//  ---------------------------------------------------------------------------
void CUIWidget_Impl::Draw()
{
	// draw's only this window.  Called by Render().	
	m_DrawBG.Render();
}


//  ---------------------------------------------------------------------------
void CUIWidget_Impl::Resize(float w, float h)
{
	// handles the repositioning specifics
	m_Rect.width  = w;
	m_Rect.height = h;
	
	if (pDrawPrimInternal) {
		pDrawPrimInternal->SetXYWH(&m_DrawBG.m_Poly, 
			m_Rect.x, m_Rect.y, m_Rect.width, m_Rect.height);
	}
	
}


//  ---------------------------------------------------------------------------
void CUIWidget_Impl::Move(float x, float y)
{
	// handles the repositioning specifics
	uint32 v;

	float dx = x - m_Rect.x;
	float dy = y - m_Rect.y;
	
	for (v=0; v<4; v++) {	
		m_DrawBG.m_Poly.verts[v].x += dx;
		m_DrawBG.m_Poly.verts[v].y += dy;
	}

	// must set these after the move, so that dx, dy will be valid!
	m_Rect.x = x;
	m_Rect.y = y;
}


//  ---------------------------------------------------------------------------
HTEXTURE	CUIWidget_Impl::GetTexture(CUI_ELEMENTTYPE elm)
{
	switch (elm) {
		case CUIE_BG:
			return m_DrawBG.m_Texture;
			break;
	}

	return NULL;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIWidget_Impl::SetTexture(CUI_ELEMENTTYPE elm, 
							      HTEXTURE hTex, 
							      bool tile)
{

	float	tw, th;
	uint32	texw, texh;

	if (tile && hTex) {
		pTexInterface->GetTextureDims(hTex, texw, texh);
		tw = (m_DrawBG.m_Poly.verts[1].x - m_DrawBG.m_Poly.verts[0].x) / (float)texw;
		th = (m_DrawBG.m_Poly.verts[3].y - m_DrawBG.m_Poly.verts[0].y) / (float)texh;
	}
	else {
		tw = 1;
		th = 1;
	}

	switch (elm) {
		case CUIE_BG:
			m_DrawBG.m_Texture = hTex;
			pDrawPrimInternal->SetUVWH(&m_DrawBG.m_Poly, 
				0,0,tw,th);
			break;
		
		default:
			return CUIR_UNKNOWN_ELEMENT;
	}

	return CUIR_OK;

}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIWidget_Impl::SetTexture(CUI_ELEMENTTYPE elm, 
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
		case CUIE_BG:
			m_DrawBG.m_Texture = hTex;
			pDrawPrimInternal->SetUVWH(&m_DrawBG.m_Poly, 
					r.x / tw,
					r.y / th,
					(r.width)  / tw,
					(r.height) / th);
			break;

		default:
			return CUIR_UNKNOWN_ELEMENT;
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidget_Impl::SetColor(CUI_ELEMENTTYPE elm, uint32 argb)
{	
	return this->SetColors(elm, argb, argb, argb, argb);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidget_Impl::SetColors(CUI_ELEMENTTYPE elm, 
										 uint32 argb0,
										 uint32 argb1,
										 uint32 argb2,
										 uint32 argb3)
{	
	switch (elm) {
		case CUIE_RELEVANT:
			// same as BG... fall through
		case CUIE_BG:
			m_DrawBG.SetColors(argb0, argb1, argb2, argb3);
			break;

		default:
			return CUIR_UNKNOWN_ELEMENT;	
	}
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidget_Impl::SetAlignmentH(CUI_ALIGNMENTTYPE align)
{
	m_Halign = align;
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidget_Impl::SetAlignmentV(CUI_ALIGNMENTTYPE align)
{
	m_Valign = align;
	return CUIR_OK;
}

