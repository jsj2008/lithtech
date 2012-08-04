//-------------------------------------------------------------------
//
//   MODULE    : CUIWIDGET.CPP
//
//   PURPOSE   : implements the CUIWidget bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIWIDGET_H__
#include "cuiwidget.h"
#endif

#ifndef __CUIWIDGET_IMPL_H__
#include "cuiwidget_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIWidget::~CUIWidget()
{
	
}


//  ---------------------------------------------------------------------------
//	The following functions are all bridge functions.  The widget does nothing
//	on its own.  This is to keep all implementation out of the SDK directory.
//  ---------------------------------------------------------------------------


//  ---------------------------------------------------------------------------
HTEXTURE CUIWidget::GetTexture(CUI_ELEMENTTYPE elm)
{
	return m_pImpl->GetTexture(elm);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidget::SetColor(CUI_ELEMENTTYPE elm, uint32 argb)
{
	return m_pImpl->SetColor(elm, argb);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE 	CUIWidget::SetColors(CUI_ELEMENTTYPE elm, 
									 uint32 argb0,
									 uint32 argb1,
									 uint32 argb2,
									 uint32 argb3)
{
	return m_pImpl->SetColors(elm, argb0, argb1, argb2, argb3);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidget::SetTexture(CUI_ELEMENTTYPE elm, HTEXTURE hTex, bool tile)
{
	return m_pImpl->SetTexture(elm, hTex, tile);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidget::SetTexture(CUI_ELEMENTTYPE elm, HTEXTURE hTex, CUIRECT* pRect)
{
	return m_pImpl->SetTexture(elm, hTex, pRect);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidget::SetAlignmentH(CUI_ALIGNMENTTYPE align)
{
	return ((CUIWidget_Impl*)m_pImpl)->SetAlignmentH(align);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIWidget::SetAlignmentV(CUI_ALIGNMENTTYPE align)
{
	return ((CUIWidget_Impl*)m_pImpl)->SetAlignmentV(align);
}

