//-------------------------------------------------------------------
//
//   MODULE    : CUIDROPDOWNLIST_IMPL.CPP
//
//   PURPOSE   : implements the CUIDropDownList_Impl widget class
//
//   CREATED   : 4/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIDROPDOWNLIST_IMPL_H__
#include "cuidropdownlist_impl.h"
#endif

#ifndef __ILTFONTMANAGER_H__
#include "iltfontmanager.h"
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
CUIDropDownList_Impl::CUIDropDownList_Impl(CUIBase* abstract, CUIGUID guid) :
	CUIList_Impl(abstract, guid)
{
	m_DisplayNumber = 5;
	ResizeDropDown();
}


//  ---------------------------------------------------------------------------
CUIDropDownList_Impl::~CUIDropDownList_Impl()
{
	// free any allocated resources
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIDropDownList_Impl::Open()
{
	// open the menu
	this->SetState(CUIS_OPEN);

	return CUIR_OK;
}

//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIDropDownList_Impl::Close()
{
	// close the menu
	this->UnsetState(CUIS_OPEN);
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIDropDownList_Impl::SetDisplayNumber(uint8 display)
{
	m_DisplayNumber = display;

	// set the size of the drop-down bg
	this->AlignTextInWidget();
	this->ResizeDropDown();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIDropDownList_Impl::SetGutter(int16 left, int16 right, int16 top, int16 bottom)
{
	CUI_RESULTTYPE retval;

	// parent call
	retval = CUIList_Impl::SetGutter(left, right, top, bottom);

	this->AlignTextInWidget();
	this->ResizeDropDown();

	return retval;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIDropDownList_Impl::SetCharHeight(uint8 height)
{
	CUI_RESULTTYPE retval;

	// call parent class
	retval = CUIList_Impl::SetCharHeight(height);

	// set the size of the drop-down bg
	this->AlignTextInWidget();
	this->ResizeDropDown();

	return retval;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIDropDownList_Impl::SetFont(CUIFont* pFont)
{
	CUI_RESULTTYPE retval;

	// call parent class
	retval = CUIList_Impl::SetFont(pFont);

	// set the size of the drop-down bg
	this->AlignTextInWidget();
	this->ResizeDropDown();

	return retval;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIDropDownList_Impl::SetColors(CUI_ELEMENTTYPE elm, 
									   uint32 argb0,
									   uint32 argb1,
									   uint32 argb2,
									   uint32 argb3)
{
	switch (elm) {

		case CUIE_DROPDOWN_BG:
			m_DrawDropDownBG.SetColors(argb0, argb1, argb2, argb3);
			break;			

		default:
			return CUIList_Impl::SetColors(elm, argb0, argb1, argb2, argb3);			
	}
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIDropDownList_Impl::SetTexture(CUI_ELEMENTTYPE elm, 
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
		case CUIE_DROPDOWN_BG:
			m_DrawDropDownBG.m_Texture = hTex;
			pDrawPrimInternal->SetUVWH(&m_DrawDropDownBG.m_Poly, 
					r.x / tw,
					r.y / th,
					r.width / tw,
					r.height / th);
			break;

		default:
			return CUIList_Impl::SetTexture(elm, hTex, pRect);
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
int32 CUIDropDownList_Impl::QueryPoint(float x, float y)
{
	CUIListNode*	pNode;
	CUIPolyString*	pStr;
	CUIRECT			r;
	int32			curindex = 0;
	int32			displaymax;

	// get the list of strings
	if (m_pList) {
		pNode = m_pList->GetHead();

		// am I open?
		if (m_StateFlags & CUIS_OPEN) {
			m_DrawDropDownBG.Render();
			displaymax = m_WindowStart + m_DisplayNumber;
		}
		else {
			displaymax = m_WindowStart + 1;
		}
		
		// and test them one by one
		while (pNode) {
			pStr = (CUIPolyString*) pNode->m_pData;

			if (pStr) {
				// ignore items that are not shown
				if (curindex >= m_WindowStart) {

					if (curindex >= displaymax) break;

					pStr->GetRect(&r);

					if (x > r.x && x < r.x + r.width) 
						if (y > r.y && y < r.y + r.height)
							return curindex;
				}
			}

			pNode = pNode->m_pNext;
			curindex++;
		}
	}

	return -1;
}


//  ---------------------------------------------------------------------------
void CUIDropDownList_Impl::Draw()
{
	CUIListNode*	pNode;
	CUIPolyString*	pStr;
	int32			curindex = 0;
	uint32			displayindex = 0;
	uint32			displaymax;
	CUIRECT			clipRect;

	// draw the bg image
	CUIWidget_Impl::Draw();

	// get the list of strings
	if (m_pList) {
		pNode = m_pList->GetHead();
		
		// am I open?
		if (m_StateFlags & CUIS_OPEN) {
			m_DrawDropDownBG.Render();
			displaymax = m_DisplayNumber;

			clipRect.x = m_DrawDropDownBG.m_Poly.verts[0].x;
			clipRect.y = m_DrawDropDownBG.m_Poly.verts[0].y;

			clipRect.width  = m_DrawDropDownBG.m_Poly.verts[1].x - m_DrawDropDownBG.m_Poly.verts[0].x - m_GutterRight;
			clipRect.height = m_DrawDropDownBG.m_Poly.verts[3].y - m_DrawDropDownBG.m_Poly.verts[0].y - m_GutterBottom;
		}
		else {
			displaymax = 1;

			clipRect = m_Rect;
			clipRect.x += m_GutterLeft*2;
			clipRect.width -= (m_GutterLeft*2) + (m_GutterRight*2);
		}

		// and draw them one by one
		while (pNode) {
			if (curindex >= m_WindowStart) {
				if (displayindex < displaymax) {							
					pStr = (CUIPolyString*) pNode->m_pData;
					if (pStr) pStr->RenderClipped(&clipRect);
					displayindex++;
				}
			}
			pNode = pNode->m_pNext;
			curindex++;
		}
	}
}


//  ---------------------------------------------------------------------------
void CUIDropDownList_Impl::Move(float x, float y)
{
	CUIWidget_Impl::Move(x,y);

	this->AlignTextInWidget();
	this->ResizeDropDown();
}


//  ---------------------------------------------------------------------------
void CUIDropDownList_Impl::Resize(float w, float h)
{
	CUIWidget_Impl::Resize(w,h);
}


//  ---------------------------------------------------------------------------
void CUIDropDownList_Impl::AlignTextInWidget()
{
	CUIListNode*	pNode;
	CUIPolyString*	pStr;
	float			x,y;
	int32			curindex = 0;

	x = m_Rect.x + m_GutterLeft*2;
	y = m_Rect.y + m_GutterTop*2;

	// get the list of strings
	if (m_pList) {
		pNode = m_pList->GetHead();
		
		// and setpos() them one by one
		while (pNode) {
			if (curindex >= m_WindowStart) {
				pStr = (CUIPolyString*) pNode->m_pData;
				if (pStr) pStr->SetPosition(x,y);
				y += m_CharHeight + m_pFont->GetDefSpacingV();
			}
			pNode = pNode->m_pNext;
			curindex++;
		}
	}	
}


//  ---------------------------------------------------------------------------
void CUIDropDownList_Impl::ResizeDropDown()
{
	float height = (float) m_CharHeight;

	if (m_pFont) height += m_pFont->GetDefSpacingV();

	// left side
	m_DrawDropDownBG.m_Poly.verts[0].x = m_Rect.x + m_GutterLeft;
	m_DrawDropDownBG.m_Poly.verts[3].x = m_DrawDropDownBG.m_Poly.verts[0].x;

	// right side
	m_DrawDropDownBG.m_Poly.verts[1].x = m_Rect.x + m_Rect.width - m_GutterRight;
	m_DrawDropDownBG.m_Poly.verts[2].x = m_DrawDropDownBG.m_Poly.verts[1].x;

	// top
	m_DrawDropDownBG.m_Poly.verts[0].y = m_Rect.y + m_GutterTop;
	m_DrawDropDownBG.m_Poly.verts[1].y = m_DrawDropDownBG.m_Poly.verts[0].y;

	// bottom
	m_DrawDropDownBG.m_Poly.verts[2].y = m_Rect.y + m_GutterTop + m_GutterBottom + (m_DisplayNumber * height);
	m_DrawDropDownBG.m_Poly.verts[3].y = m_DrawDropDownBG.m_Poly.verts[2].y;
}
