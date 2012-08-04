//-------------------------------------------------------------------
//
//   MODULE    : CUICHECK_IMPL.CPP
//
//   PURPOSE   : Implements the CUICheck_Impl Utility Class
//
//   CREATED   : 4/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUICHECK_IMPL_H__
#include "cuicheck_impl.h"
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

#ifndef __ILTFONTMANAGER_H__
#include "iltfontmanager.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif


// Interface database
static ILTWidgetManager *pWidgetManager;
define_holder(ILTWidgetManager, pWidgetManager);

static ILTTexInterface *pTexInterface;
define_holder(ILTTexInterface, pTexInterface);

static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);

static ILTFontManager *pLTFontManager;
define_holder(ILTFontManager, pLTFontManager);


//  ---------------------------------------------------------------------------
CUICheck_Impl::CUICheck_Impl(CUIBase* abstract, CUIGUID guid) :
	CUIButton_Impl(abstract, guid)
{

}


//  ---------------------------------------------------------------------------
CUICheck_Impl::~CUICheck_Impl()
{
	// delete resources
}


//  ---------------------------------------------------------------------------
bool CUICheck_Impl::GetValue()
{
	// save the old value
	uint32 retVal = m_StateFlags & CUIS_PRESSED;

	return (retVal != 0);
}


//  ---------------------------------------------------------------------------
/*
void CUICheck_Impl::Move(float x, float y)
{
	// handles the repositioning specifics
	uint32 v;

	float dx = x - m_Rect.x;
	float dy = y - m_Rect.y;
	
	for (v=0; v<4; v++) {	
		m_drawCheckPressed.m_Poly.verts[v].x += dx;
		m_drawCheckPressed.m_Poly.verts[v].y += dy;

		m_drawCheckHighlighted.m_Poly.verts[v].x += dx;
		m_drawCheckHighlighted.m_Poly.verts[v].y += dy;
	
		m_drawCheckDisabled.m_Poly.verts[v].x += dx;
		m_drawCheckDisabled.m_Poly.verts[v].y += dy;
	}
	

	CUIWidget_Impl::Move(x,y);
}
*/


//  ---------------------------------------------------------------------------
/*
void CUICheck_Impl::Resize(float w, float h)
{
	CUIWidget_Impl::Resize(w, h);

	pDrawPrimInternal->SetXYWH(&m_drawCheckPressed.m_Poly, 
			m_Rect.x, m_Rect.y, m_Rect.width, m_Rect.height);

	pDrawPrimInternal->SetXYWH(&m_drawCheckHighlighted.m_Poly, 
			m_Rect.x, m_Rect.y, m_Rect.width, m_Rect.height);

	pDrawPrimInternal->SetXYWH(&m_drawCheckDisabled.m_Poly, 
			m_Rect.x, m_Rect.y, m_Rect.width, m_Rect.height);

}
*/


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUICheck_Impl::SetColors(CUI_ELEMENTTYPE elm, 
										 uint32 argb0,
										 uint32 argb1,
										 uint32 argb2,
										 uint32 argb3)
{	
	switch (elm) {
		case CUIE_CHECK_ON:
			elm = CUIE_BUTTON_PRESSED;
			//return CUIButton_Impl::SetColors(CUIE_BUTTON_PRESSED, argb0, argb1, argb2, argb3);
			break;

		case CUIE_CHECK_OFF:
			elm = CUIE_BUTTON_HIGHLIGHTED;
			//return CUIButton_Impl::SetColors(CUIE_BUTTON_HIGHLIGHTED, argb0, argb1, argb2, argb3);
			break;

		case CUIE_CHECK_DISABLED:
			elm = CUIE_BUTTON_DISABLED;
			//return CUIButton_Impl::SetColors(CUIE_BUTTON_DISABLED, argb0, argb1, argb2, argb3);
			break;

		//default:			
	}

	return CUIButton_Impl::SetColors(elm, argb0, argb1, argb2, argb3);	
	
	//return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUICheck_Impl::SetGutter(int16 left, int16 right, int16 top, int16 bottom)
{
	// set the gutter
	CUIWidget_Impl::SetGutter(left, right, top, bottom);

	// reposition our elements
	this->Reposition();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUICheck_Impl::SetTexture(CUI_ELEMENTTYPE elm, 
										   HTEXTURE hTex, 
										   CUIRECT* pRect)
{
	CUIPolyTex* pTex;
	CUIRECT r;
	float tw, th;
	uint32 texw, texh;

	switch (elm) {

		case CUIE_CHECK_ON:
			pTex = &m_drawButtonPressed;
			break;

		case CUIE_CHECK_OFF:
			pTex = &m_drawButtonHighlighted;
			break;

		case CUIE_CHECK_DISABLED:
			pTex = &m_drawButtonDisabled;
			break;

		default:
			// CUIE_BG is handled by superclass
			return CUIButton_Impl::SetTexture(elm, hTex, pRect);
	}


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


	// set the texture
	pTex->m_Texture = hTex;

	// set the U,V Coords.
	pDrawPrimInternal->SetUVWH(&pTex->m_Poly, 
		r.x / tw,
		r.y / th,
		r.width / tw,
		r.height / th);

	// set X,Y coords. (this element is the same size as its texture
	pDrawPrimInternal->SetXYWH(&pTex->m_Poly, 
		m_Rect.x,
		m_Rect.y,
		r.width,
		r.height);	

	this->Reposition();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
bool CUICheck_Impl::SetValue(bool value)
{
	// save the old value
	uint32 retVal = m_StateFlags & CUIS_PRESSED;

	// set the new value
	if (value) {
		this->SetState(CUIS_PRESSED);
	}
	else {
		this->UnsetState(CUIS_PRESSED);
	}

	// return the old value
	return (retVal != 0);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUICheck_Impl::SetAlignmentH(CUI_ALIGNMENTTYPE align)
{
	// go all the way back to widget, don't use parent method
	CUIWidget_Impl::SetAlignmentH(align);
	this->Reposition();
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUICheck_Impl::SetAlignmentV(CUI_ALIGNMENTTYPE align)
{
	// go all the way back to widget, don't use parent method
	CUIWidget_Impl::SetAlignmentV(align);
	this->Reposition();
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUICheck_Impl::SetText(const char* pText)
{
	float x		= m_Rect.x + m_GutterLeft; 
	float y		= m_Rect.y + m_GutterTop; 
	uint32 wrap	= (uint32) (m_Rect.width - m_GutterRight - m_GutterLeft);

	int32 len = strlen(pText);

	// if not polystring exists, make one
	if (!m_pPolyStr) {
		m_pPolyStr = pLTFontManager->CreateFormattedPolyString(m_pFont, NULL, m_Rect.x, m_Rect.y, m_Halign);
	}

	// if the 'new' was successful...
	if (!m_pPolyStr) return CUIR_OUT_OF_MEMORY;

	// make the polystring conform to the label
	//m_pPolyStr->SetWrapWidth(wrap);

	m_pPolyStr->SetAlignmentH(CUI_HALIGN_LEFT);
	m_pPolyStr->SetCharScreenHeight(m_CharHeight);
	m_pPolyStr->SetColors(m_pTextColors[0], m_pTextColors[1], m_pTextColors[2], m_pTextColors[3]);
		
	// if we set the text after all the formatting is set, we save time because
	// we're not re-visiting polys over and over
	m_pPolyStr->SetText(pText);

	AlignTextInWidget();
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
bool CUICheck_Impl::Toggle()
{
	// save the old value
	uint32 retVal = m_StateFlags & CUIS_PRESSED;

	// set the new value to the opposite
	if (!retVal) {
		this->SetState(CUIS_PRESSED);
	}
	else {
		this->UnsetState(CUIS_PRESSED);
	}

	// return the old value
	return (retVal != 0);
}


//  ---------------------------------------------------------------------------
void CUICheck_Impl::Draw()
{
	// unfortunately we can't borrow the draw routine from CUIButton since 
	// buttons draw only one image, where we draw the BG plus some checked
	// state.

	// draw the BG
	m_DrawBG.Render();

	// draw the check box
	if (m_StateFlags & CUIS_ENABLED) {

		if (m_StateFlags & CUIS_PRESSED) {
			m_drawButtonPressed.Render();

		}
		else { // if (m_StateFlags & CUIS_HIGHLIGHTED) {
			m_drawButtonHighlighted.Render();
		}
	}
	else {
		m_drawButtonDisabled.Render();
	}
		
	// draw the text
	if (m_pPolyStr) m_pPolyStr->RenderClipped(&m_Rect);
}


//  ---------------------------------------------------------------------------
void CUICheck_Impl::Reposition()
{
	// depending on the alignment of the checkbox, we need to set the position
	// of the text and the texture elements.  The text is always a LEFT-aligned 
	// polystring.
	uint32 i;

	float x, y;
	float cx, cy, cw, ch;

	//CUIRECT	TextRect;
	//float textx, texty;

	CUIPolyTex*		pArray[3];

	pArray[0] = &m_drawButtonPressed;
	pArray[1] = &m_drawButtonDisabled;
	pArray[2] = &m_drawButtonHighlighted;	

	// get the width & height of the textured element
	cx = pArray[0]->m_Poly.verts[0].x;
	cy = pArray[0]->m_Poly.verts[0].y;

	cw = pArray[0]->m_Poly.verts[1].x - pArray[0]->m_Poly.verts[0].x;
	ch = pArray[0]->m_Poly.verts[3].y - pArray[0]->m_Poly.verts[0].y;
		
	// compute the new coords based on alignment style and dims
	switch (m_Halign) {

		case CUI_HALIGN_RIGHT:
			x = m_Rect.x + m_Rect.width - m_GutterRight - cw;
			break;

		case CUI_HALIGN_LEFT:
			// fall through...
		case CUI_HALIGN_CENTER:
			// fall through...
		case CUI_HALIGN_JUSTIFY:
			// fall through...
		default:
			x = m_Rect.x + m_GutterLeft;
	}

	// compute the new coords based on alignment style and dims
	switch (m_Valign) {

		case CUI_VALIGN_TOP:
			y = m_Rect.y + m_GutterTop;
			break;

		case CUI_VALIGN_BOTTOM:
			y = m_Rect.y + m_Rect.height - m_GutterBottom - ch;
			break;
		
		default: // center
			y = m_Rect.y + (m_Rect.height - ch) / 2;
	}


	// set the textures new x,y coords
	
	for (i=0; i<3; i++) {
		pDrawPrimInternal->SetXYWH(&pArray[i]->m_Poly, x, y, cw, ch);
	}

	this->AlignTextInWidget();	
}


//  ---------------------------------------------------------------------------
void CUICheck_Impl::AlignTextInWidget()
{
	float w, h;
	float ypos, xpos;

	if (m_pPolyStr) {
		
		h = m_pPolyStr->GetHeight();
		w = m_pPolyStr->GetWidth();

		switch (m_Valign) {

			case CUI_VALIGN_TOP:
				ypos = m_Rect.y + m_GutterTop;
				break;

			case CUI_VALIGN_CENTER:
				ypos = m_Rect.y + (m_Rect.height - h) / 2;
				break;

			case CUI_VALIGN_BOTTOM:
				ypos = m_Rect.y + (m_Rect.height - h - m_GutterBottom);
				break;

		}

		switch (m_Halign) {

			case CUI_HALIGN_CENTER:
				// fall through...
			case CUI_HALIGN_JUSTIFY:
				// fall through...
			case CUI_HALIGN_LEFT:
				xpos = m_Rect.x + m_Rect.width - m_GutterRight - w;
				break;

			case CUI_HALIGN_RIGHT:				
				xpos = m_Rect.x + m_GutterLeft;
				break;

		}

		m_pPolyStr->SetPosition(xpos, ypos);
	}
}