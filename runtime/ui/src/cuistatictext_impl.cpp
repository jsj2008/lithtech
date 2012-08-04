//-------------------------------------------------------------------
//
//   MODULE    : CUISTATICTEXT_IMPL.CPP
//
//   PURPOSE   : implements the CUIStaticText_Impl widget class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUISTATICTEXT_IMPL_H__
#include "cuistatictext_impl.h"
#endif

#ifndef __ILTFONTMANAGER_H__
#include "iltfontmanager.h"
#endif


// use the ILTFontManager for polystring allocation
static ILTFontManager *pLTFontManager;
define_holder(ILTFontManager, pLTFontManager);


//  ---------------------------------------------------------------------------
CUIStaticText_Impl::CUIStaticText_Impl(CUIBase* abstract, CUIGUID guid) :
	CUIWidget_Impl(abstract, guid)
{
	m_pPolyStr		= NULL;
	m_pFont 		= NULL;

	m_WrapWidth		= 0;

	m_pTextColors[0] = 
		m_pTextColors[1] = 
			m_pTextColors[2] = 
				m_pTextColors[3] = 0x00000000 | CUI_SYSTEM_OPAQUE;

	m_CharHeight	= 0;
	m_bMultiLine	= true;
	m_Halign		= CUI_HALIGN_LEFT;
}


//  ---------------------------------------------------------------------------
CUIStaticText_Impl::~CUIStaticText_Impl()
{
	// delete the polystring
	if (m_pPolyStr) {
		pLTFontManager->DestroyPolyString(m_pPolyStr);
		m_pPolyStr = NULL;
	}
}

	
//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText_Impl::SetText(const char* pText)
{
	float x		= m_Rect.x + m_GutterLeft; 
	float y		= m_Rect.y + m_GutterTop; 
	uint32 wrap	= (uint32) (m_Rect.width - m_GutterRight - m_GutterLeft);

	int32 len = strlen(pText);

	// make sure there's a font
	if (!m_pFont) return CUIR_NO_FONT;

	y = m_Rect.y; 
	
	switch (m_Halign) {
				
		case CUI_HALIGN_CENTER:
			x = m_Rect.x + m_Rect.width/2; 
			break;

		case CUI_HALIGN_RIGHT:
			x = m_Rect.x + m_Rect.width; 
			break;

		default:
			x = m_Rect.x; 			
	}
	

	if (!m_pPolyStr) {
		m_pPolyStr = pLTFontManager->CreateFormattedPolyString(m_pFont, NULL, x, y, m_Halign);
	}

	// if the 'new' was successful...
	if (!m_pPolyStr) return CUIR_OUT_OF_MEMORY;

	// make the polystring conform to the label
	m_pPolyStr->SetWrapWidth(wrap);
	m_pPolyStr->SetCharScreenHeight(m_CharHeight);
	m_pPolyStr->SetColors(m_pTextColors[0], m_pTextColors[1], m_pTextColors[2], m_pTextColors[3]);
	m_pPolyStr->SetText(pText);

	AlignTextInWidget();
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIStaticText_Impl::SetGutter(int16 left, int16 right, int16 top, int16 bottom)
{
	// set the gutter
	CUIBase_Impl::SetGutter(left, right, top, bottom);

	m_WrapWidth = (uint16) (m_Rect.width - m_GutterLeft - m_GutterRight);

	if (m_pPolyStr) {
		m_pPolyStr->SetWrapWidth(m_WrapWidth);
	}
	
	AlignTextInWidget();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText_Impl::SetFont(CUIFont* pFont)
{
	m_pFont = pFont;

	if (pFont) {
		// if no charheight, set ones
		if (!m_CharHeight) m_CharHeight = pFont->GetDefCharScreenHeight();
	}
	else {
		return CUIR_NO_FONT;
	}

	// change the font in the polystring
	if (m_pPolyStr) {
		m_pPolyStr->SetFont(pFont);
	}

	// (re)align the text
	AlignTextInWidget();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText_Impl::SetCharHeight(uint8 height)
{
	m_CharHeight = height;

	if (m_pPolyStr) {
		m_pPolyStr->SetCharScreenHeight(height);
	}

	AlignTextInWidget();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText_Impl::SetWrapWidth(uint16 wrap)
{
	// is this a necessary function??
	m_WrapWidth = wrap;

	if (m_pPolyStr) {
		m_pPolyStr->SetWrapWidth(m_WrapWidth);
	}

	AlignTextInWidget();

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText_Impl::SetColors(CUI_ELEMENTTYPE elm, 
											 uint32 argb0,
											 uint32 argb1,
											 uint32 argb2,
											 uint32 argb3)
{
	switch (elm) {

		case CUIE_TEXT:
			if (m_pPolyStr) {
				m_pPolyStr->SetColors(argb0, argb1, argb2, argb3);
			}
			m_pTextColors[0] = argb0;
			m_pTextColors[1] = argb1;
			m_pTextColors[2] = argb2;
			m_pTextColors[3] = argb3;

			break;

		default:
			return CUIWidget_Impl::SetColors(elm, argb0, argb1, argb2, argb3);			
	}
	
	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText_Impl::SetAlignmentH(CUI_ALIGNMENTTYPE align)
{
	CUIWidget_Impl::SetAlignmentH(align);

	if (m_pPolyStr) {
		m_pPolyStr->SetAlignmentH(align);	
		this->AlignTextInWidget();
	}

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIStaticText_Impl::SetAlignmentV(CUI_ALIGNMENTTYPE align)
{
	CUIWidget_Impl::SetAlignmentV(align);

	if (m_pPolyStr) {
		this->AlignTextInWidget();
	}	

	return CUIR_OK;
}


//  ---------------------------------------------------------------------------
const char* CUIStaticText_Impl::GetText()
{
	return m_pPolyStr->GetText();
}


//  ---------------------------------------------------------------------------
void CUIStaticText_Impl::Resize(float w, float h)
{
	// resize the control
	CUIWidget_Impl::Resize(w, h);

	m_WrapWidth = (uint16)w - m_GutterLeft - m_GutterRight;

	if (m_pPolyStr) {
		m_pPolyStr->SetWrapWidth(m_WrapWidth);
	}
	
	AlignTextInWidget();	
}


//  ---------------------------------------------------------------------------
void CUIStaticText_Impl::Move(float x, float y)
{
	float oldx, oldy;
	float dx, dy;
	
	// and move the text string
	if (m_pPolyStr) {
		m_pPolyStr->GetPosition(&oldx, &oldy);
		dx = oldx - m_Rect.x;
		dy = oldy - m_Rect.y;				
		m_pPolyStr->SetPosition(x + dx, y + dy);
	}
	
	// move the control
	CUIWidget_Impl::Move(x, y);
}


//  ---------------------------------------------------------------------------
void CUIStaticText_Impl::Draw()
{
	// has no children
	// draw the window BG
	CUIWidget_Impl::Draw();

	if (m_pPolyStr) m_pPolyStr->RenderClipped(&m_Rect);
}


//  ---------------------------------------------------------------------------
void CUIStaticText_Impl::AlignTextInWidget()
{
	float w, h;
	float ypos, xpos;	

	if (m_pPolyStr) {
		
		h = m_pPolyStr->GetHeight();
		w = m_pPolyStr->GetWidth();
		xpos = m_pPolyStr->GetX();

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

			case CUI_HALIGN_JUSTIFY:
			case CUI_HALIGN_LEFT:
				xpos = m_Rect.x + m_GutterLeft;
				break;

			case CUI_HALIGN_CENTER:
				xpos = m_Rect.x + m_Rect.width/2;
				break;

			case CUI_HALIGN_RIGHT:
				xpos = m_Rect.x + m_Rect.width - m_GutterRight;
				break;
		}

		m_pPolyStr->SetPosition(xpos, ypos);
	}
}

		
