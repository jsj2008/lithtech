//-------------------------------------------------------------------
//
//   MODULE    : CUIPOLYSTRING.CPP
//
//   PURPOSE   : implements the CUIPolyString bridge class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIPOLYSTRING_H__
#include "cuipolystring.h"
#endif

#ifndef __CUIPOLYSTRING_IMPL_H__
#include "cuipolystring_impl.h"
#endif


//  ---------------------------------------------------------------------------
CUIPolyString::CUIPolyString()
{
	// does nothing
}


//  ---------------------------------------------------------------------------
CUIPolyString::CUIPolyString(CUIFont* font, 
							 const char* buf,
							 float x,
							 float y)
{
	LT_MEM_TRACK_ALLOC(m_pImpl = new CUIPolyString_Impl(font, buf, x, y),LT_MEM_TYPE_UI);
}


//  ---------------------------------------------------------------------------
CUIPolyString::~CUIPolyString() 
{
	if (m_pImpl) 
		delete m_pImpl;
	m_pImpl = NULL;
}


//  ---------------------------------------------------------------------------
const char*	CUIPolyString::GetClassName()
{
	return m_pImpl->GetClassName();
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString::SetPosition(float x, float y)
{
	return m_pImpl->SetPosition(x,y);
}	
	

//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString::GetPosition(float* x, float* y)
{
	return m_pImpl->GetPosition(x,y);
}


//  ---------------------------------------------------------------------------
uint16	CUIPolyString::GetLength()
{
	return m_pImpl->GetLength();
}


//  ---------------------------------------------------------------------------
float	CUIPolyString::GetX()
{
	return m_pImpl->GetX();
}


//  ---------------------------------------------------------------------------
float	CUIPolyString::GetY()
{
	return m_pImpl->GetY();
}


//  ---------------------------------------------------------------------------
float	CUIPolyString::GetWidth()
{
	return m_pImpl->GetWidth();
}


//  ---------------------------------------------------------------------------
float	CUIPolyString::GetHeight()
{
	return m_pImpl->GetHeight();
}


//  ---------------------------------------------------------------------------
CUIFont* CUIPolyString::GetFont()
{
	return m_pImpl->GetFont();
}


//  ---------------------------------------------------------------------------
const char*	CUIPolyString::GetText()
{
	return m_pImpl->GetText();
}


//  ---------------------------------------------------------------------------
LT_POLYGT4*	CUIPolyString::GetPolys()
{
	return m_pImpl->GetPolys();
}


//  ---------------------------------------------------------------------------
uint8	CUIPolyString::GetCharScreenWidth()
{
	return m_pImpl->GetCharScreenWidth();
}


//  ---------------------------------------------------------------------------
uint8	CUIPolyString::GetCharScreenHeight()
{
	return m_pImpl->GetCharScreenHeight();
}



//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString::GetRect(CUIRECT* rect)
{
	return m_pImpl->GetRect(rect);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString::GetDims(float* width, float* height)
{
	return m_pImpl->GetDims(width,height);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIPolyString::SetCharScreenWidth(uint8 width)
{
	return m_pImpl->SetCharScreenWidth(width);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIPolyString::SetCharScreenHeight(uint8 height)
{
	return m_pImpl->SetCharScreenHeight(height);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE CUIPolyString::SetCharScreenSize(uint8 height, uint8 width)
{
	return m_pImpl->SetCharScreenSize(height, width);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString::SetColor(uint32 argb)
{
	return m_pImpl->SetColor(argb);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString::SetColors(uint32 argb0, uint32 argb1,
										 uint32 argb2, uint32 argb3)
{
	return m_pImpl->SetColors(argb0, argb1, argb2, argb3);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString::SetText(const char* buf)
{
	return m_pImpl->SetText(buf);
}


//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIPolyString::SetFont(CUIFont* pFont)
{
	return  m_pImpl->SetFont(pFont);
}

//	---------------------------------------------------------------------------
CUI_RESULTTYPE CUIPolyString::ApplyFont(
	CUIFont* pFont, 
	int16 index, 
	int16 num,
	bool  bProcessRemainder)
{
	return m_pImpl->ApplyFont(pFont, index, num, bProcessRemainder);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString::Render(int32 start, int32 end)
{
	return m_pImpl->Render(start, end);
}


//  ---------------------------------------------------------------------------
CUI_RESULTTYPE	CUIPolyString::RenderClipped(CUIRECT* clip, 
										  int32 start, 
										  int32 end)
{
	return m_pImpl->RenderClipped(clip, start, end);
}


//  ---------------------------------------------------------------------------
bool	CUIPolyString::IsValid()
{
	return m_pImpl->IsValid();
}


