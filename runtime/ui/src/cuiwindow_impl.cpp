//-------------------------------------------------------------------
//
//   MODULE    : CUIWINDOW_IMPL.CPP
//
//   PURPOSE   : implements the CUIWindow_Impl widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIWINDOW_IMPL_H__
#include "cuiwindow_impl.h"
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


// use the internal drawprim interface
static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);

static ILTTexInterface *pTexInterface;
define_holder(ILTTexInterface, pTexInterface);


//  ---------------------------------------------------------------------------
CUIWindow_Impl::CUIWindow_Impl (
	CUIBase* abstract, 
	CUIGUID guid, 
	HTEXTURE skin, 
	char* table	) : CUIWidget_Impl(abstract, guid)
{
	if (!skin || !pDrawPrimInternal) return;

	// most of the visible elements are empty
	// when 'skins' are implemented, assignment goes here	
}


//  ---------------------------------------------------------------------------
CUIWindow_Impl::~CUIWindow_Impl()
{
	// you shouldn't destroy a window directly with this!
	// call DestroyWindow()
}


//  ---------------------------------------------------------------------------
HTEXTURE	CUIWindow_Impl::GetTexture(CUI_ELEMENTTYPE elm)
{
	switch (elm) {
		
		// corners
		case CUIE_TR_CORNER:
			return m_DrawTR_CORNER.m_Texture;

		case CUIE_TL_CORNER:
			return m_DrawTL_CORNER.m_Texture;

		case CUIE_BR_CORNER:
			return m_DrawBR_CORNER.m_Texture;

		case CUIE_BL_CORNER:
			return m_DrawBL_CORNER.m_Texture;

		// edges
		case CUIE_RIGHT:
			return m_DrawRIGHT.m_Texture;

		case CUIE_LEFT:
			return m_DrawLEFT.m_Texture;

		case CUIE_TOP:
			return m_DrawTOP.m_Texture;

		case CUIE_BOTTOM:
			return m_DrawBOTTOM.m_Texture;
		
		// BG
		default:
			return CUIWidget_Impl::GetTexture(elm);
	}		
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIWindow_Impl::SetTexture(CUI_ELEMENTTYPE elm, 
									       HTEXTURE hTex, 
									       CUIRECT* pRect)
{
	CUIRECT		r;
	uint32		texw,texh;
	float		tw,th;
	float		x,y,w,h;
	CUIPolyTex*	pTex;

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

		// corners

		case CUIE_TR_CORNER:

			pTex = &m_DrawTR_CORNER;
			x = this->m_Rect.x + this->m_Rect.width - r.width;
			y = this->m_Rect.y;
			w = r.width;
			h = r.height;
			break;

		case CUIE_TL_CORNER:

			pTex = &m_DrawTL_CORNER;		
			x = this->m_Rect.x;
			y = this->m_Rect.y;
			w = r.width;
			h = r.height;
			break;

		case CUIE_BR_CORNER:

			pTex = &m_DrawBR_CORNER;		
			x = this->m_Rect.x + this->m_Rect.width - r.width;
			y = this->m_Rect.y + this->m_Rect.height - r.height;
			w = r.width;
			h = r.height;
			break;

		case CUIE_BL_CORNER:

			pTex = &m_DrawBL_CORNER;		
			x = this->m_Rect.x;
			y = this->m_Rect.y + this->m_Rect.height - r.height;
			w = r.width;
			h = r.height;
			break;

		// borders

		case CUIE_TOP:

			pTex = &m_DrawTOP;		
			x = this->m_Rect.x;
			y = this->m_Rect.y;
			w = this->m_Rect.width;
			h = r.height;
			break;
		
		case CUIE_BOTTOM:

			pTex = &m_DrawBOTTOM;		
			x = this->m_Rect.x;
			y = this->m_Rect.y + this->m_Rect.height - r.height;
			w = this->m_Rect.width;
			h = r.height;
			break;
			
		case CUIE_LEFT:

			pTex = &m_DrawLEFT;		
			x = this->m_Rect.x;
			y = this->m_Rect.y;
			w = r.width;
			h = this->m_Rect.height;
			break;
			
		case CUIE_RIGHT:

			pTex = &m_DrawRIGHT;		
			x = this->m_Rect.x + this->m_Rect.width - r.width;
			y = this->m_Rect.y;
			w = r.width;
			h = this->m_Rect.height;
			break;
		
		default:
			return CUIWidget_Impl::SetTexture(elm, hTex, pRect);
	}	

	// set the texture
	pTex->m_Texture = hTex;

	// set the X,Y coords.
	pDrawPrimInternal->SetXYWH(&pTex->m_Poly, x, y, w, h);

	// set the U,V coords.
	pDrawPrimInternal->SetUVWH(&pTex->m_Poly, 
					r.x / (float)tw,
					r.y / (float)th,
					r.width / (float)tw,
					r.height / (float)th);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIWindow_Impl::SetTexture(CUI_ELEMENTTYPE elm, 
							      HTEXTURE hTex, 
							      bool tile)
{
	float		tw, th;
	uint32		texw, texh;
	CUIPolyTex*	pTex;

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

		// borders

		case CUIE_TOP:
			pTex = &m_DrawTOP;
			break;
		
		case CUIE_BOTTOM:
			pTex = &m_DrawBOTTOM;
			break;
			
		case CUIE_LEFT:
			pTex = &m_DrawLEFT;
			break;
			
		case CUIE_RIGHT:
			pTex = &m_DrawRIGHT;
			break;
	
		// corners
	
		case CUIE_TR_CORNER:
			pTex = &m_DrawTR_CORNER;
			break;
		
		case CUIE_TL_CORNER:
			pTex = &m_DrawTL_CORNER;
			break;
		
		case CUIE_BR_CORNER:
			pTex = &m_DrawBR_CORNER;
			break;
		
		case CUIE_BL_CORNER:
			pTex = &m_DrawBL_CORNER;
			break;


		default:
			return CUIWidget_Impl::SetTexture(elm, hTex, tile);
	}	

	// set the texture
	pTex->m_Texture = hTex;

	// set the U,V coords.
	pDrawPrimInternal->SetUVWH(&pTex->m_Poly, 0, 0, tw, th);

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
void CUIWindow_Impl::Draw()
{
	// draw's only this window.  Called by Render().
	
	// draw the background
	CUIWidget_Impl::Draw();

	// draw the edges on top of the bg
	m_DrawTOP.Render();
	m_DrawBOTTOM.Render();
	m_DrawRIGHT.Render();
	m_DrawLEFT.Render();

	// draw the corners on top of it all
	m_DrawTR_CORNER.Render();
	m_DrawTL_CORNER.Render();
	m_DrawBR_CORNER.Render();
	m_DrawBL_CORNER.Render();
}

		
//  ---------------------------------------------------------------------------
void CUIWindow_Impl::Move(float x, float y)
{
	// handles the repositioning specifics
	int32	v;
	
	float 	dx = x - m_Rect.x;
	float 	dy = y - m_Rect.y;

	for (v=0; v<4; v++) {	

		// edges

		m_DrawTOP.m_Poly.verts[v].x += dx;
		m_DrawTOP.m_Poly.verts[v].y += dy;
	
		m_DrawBOTTOM.m_Poly.verts[v].x += dx;
		m_DrawBOTTOM.m_Poly.verts[v].y += dy;

		m_DrawRIGHT.m_Poly.verts[v].x += dx;
		m_DrawRIGHT.m_Poly.verts[v].y += dy;
	
		m_DrawLEFT.m_Poly.verts[v].x += dx;
		m_DrawLEFT.m_Poly.verts[v].y += dy;
	
		// corners

		m_DrawTR_CORNER.m_Poly.verts[v].x += dx;
		m_DrawTR_CORNER.m_Poly.verts[v].y += dy;
	
		m_DrawTL_CORNER.m_Poly.verts[v].x += dx;
		m_DrawTL_CORNER.m_Poly.verts[v].y += dy;
	
		m_DrawBR_CORNER.m_Poly.verts[v].x += dx;
		m_DrawBR_CORNER.m_Poly.verts[v].y += dy;
	
		m_DrawBL_CORNER.m_Poly.verts[v].x += dx;
		m_DrawBL_CORNER.m_Poly.verts[v].y += dy;
	}
		
	// move the BG (must do this last because it sets the rect to new values
	CUIWidget_Impl::Move(x, y);
}

	



//  ---------------------------------------------------------------------------
void CUIWindow_Impl::Resize(float w, float h)
{
	// handles the repositioning specifics
	float polyw, polyh;

	if (!pDrawPrimInternal) return;	

	// set m_Rect, resize m_DrawBG
	CUIWidget_Impl::Resize(w,h);
	

	// edges

	polyh = m_DrawTOP.m_Poly.verts[3].y - m_DrawTOP.m_Poly.verts[0].y;

	pDrawPrimInternal->SetXYWH(&m_DrawTOP.m_Poly, 
		m_Rect.x, m_Rect.y, m_Rect.width, polyh);


	polyh = m_DrawBOTTOM.m_Poly.verts[3].y - m_DrawBOTTOM.m_Poly.verts[0].y;

	pDrawPrimInternal->SetXYWH(&m_DrawBOTTOM.m_Poly, 
		m_Rect.x, m_Rect.y + m_Rect.height - polyh, m_Rect.width, polyh);

	
	polyw = m_DrawLEFT.m_Poly.verts[1].x - m_DrawLEFT.m_Poly.verts[0].x;

	pDrawPrimInternal->SetXYWH(&m_DrawLEFT.m_Poly, 
		m_Rect.x, m_Rect.y, polyw, m_Rect.height);

	
	polyw = m_DrawRIGHT.m_Poly.verts[1].x - m_DrawRIGHT.m_Poly.verts[0].x;

	pDrawPrimInternal->SetXYWH(&m_DrawRIGHT.m_Poly, 
		m_Rect.x + m_Rect.width - polyw, m_Rect.y, polyw, m_Rect.height);
	
	// corners

	polyw = m_DrawTR_CORNER.m_Poly.verts[1].x - m_DrawTR_CORNER.m_Poly.verts[0].x;
	polyh = m_DrawTR_CORNER.m_Poly.verts[3].y - m_DrawTR_CORNER.m_Poly.verts[0].y;

	pDrawPrimInternal->SetXYWH(&m_DrawTR_CORNER.m_Poly, 
		m_Rect.x + m_Rect.width - polyw, m_Rect.y, polyw, polyh);

	
	polyw = m_DrawTL_CORNER.m_Poly.verts[1].x - m_DrawTL_CORNER.m_Poly.verts[0].x;
	polyh = m_DrawTL_CORNER.m_Poly.verts[3].y - m_DrawTL_CORNER.m_Poly.verts[0].y;

	pDrawPrimInternal->SetXYWH(&m_DrawTL_CORNER.m_Poly, 
		m_Rect.x, m_Rect.y, polyw, polyh);

	
	polyw = m_DrawBR_CORNER.m_Poly.verts[1].x - m_DrawBR_CORNER.m_Poly.verts[0].x;
	polyh = m_DrawBR_CORNER.m_Poly.verts[3].y - m_DrawBR_CORNER.m_Poly.verts[0].y;

	pDrawPrimInternal->SetXYWH(&m_DrawBR_CORNER.m_Poly, 
		m_Rect.x + m_Rect.width - polyw, m_Rect.y + m_Rect.height - polyh, 
		polyw, polyh);

	
	polyw = m_DrawBL_CORNER.m_Poly.verts[1].x - m_DrawBL_CORNER.m_Poly.verts[0].x;
	polyh = m_DrawBL_CORNER.m_Poly.verts[3].y - m_DrawBL_CORNER.m_Poly.verts[0].y;

	pDrawPrimInternal->SetXYWH(&m_DrawBL_CORNER.m_Poly, 
		m_Rect.x, m_Rect.y + m_Rect.height - polyh, 
		polyw, polyh);

}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWindow_Impl::SetColors(CUI_ELEMENTTYPE elm, 
										 uint32 argb0,
										 uint32 argb1,
										 uint32 argb2,
										 uint32 argb3)
{
	switch (elm) {
			
		// edges
		case CUIE_TOP:
			m_DrawTOP.SetColors(argb0, argb1, argb2, argb3);
			break;
		case CUIE_BOTTOM:
			m_DrawBOTTOM.SetColors(argb0, argb1, argb2, argb3);
			break;
		case CUIE_LEFT:
			m_DrawLEFT.SetColors(argb0, argb1, argb2, argb3);
			break;
		case CUIE_RIGHT:
			m_DrawRIGHT.SetColors(argb0, argb1, argb2, argb3);
			break;	

		// corners
		case CUIE_TR_CORNER:
			m_DrawTR_CORNER.SetColors(argb0, argb1, argb2, argb3);
			break;
		case CUIE_TL_CORNER:
			m_DrawTL_CORNER.SetColors(argb0, argb1, argb2, argb3);
			break;
		case CUIE_BR_CORNER:
			m_DrawBR_CORNER.SetColors(argb0, argb1, argb2, argb3);
			break;
		case CUIE_BL_CORNER:
			m_DrawBL_CORNER.SetColors(argb0, argb1, argb2, argb3);
			break;	

		// relevant
		case CUIE_RELEVANT:
			m_DrawTOP.SetColors(argb0, argb1, argb2, argb3);
			m_DrawBOTTOM.SetColors(argb0, argb1, argb2, argb3);
			m_DrawLEFT.SetColors(argb0, argb1, argb2, argb3);
			m_DrawRIGHT.SetColors(argb0, argb1, argb2, argb3);
			m_DrawTR_CORNER.SetColors(argb0, argb1, argb2, argb3);
			m_DrawTL_CORNER.SetColors(argb0, argb1, argb2, argb3);
			m_DrawBR_CORNER.SetColors(argb0, argb1, argb2, argb3);
			m_DrawBL_CORNER.SetColors(argb0, argb1, argb2, argb3);
			break;	

		default:
			return CUIWidget_Impl::SetColors(elm, argb0, argb1, argb2, argb3);
	}
	
	return CUIR_OK;
}


