//-------------------------------------------------------------------
//
//   MODULE    : CUIFormattedPolyString_Impl.h
//
//   PURPOSE   : defines the CUIFormattedPolyString_Impl Class
//
//   CREATED   : 7/00 
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIFORMATTEDPOLYSTRING_IMPL_H__
#define __CUIFORMATTEDPOLYSTRING_IMPL_H__


#ifndef __CUIFORMATTEDPOLYSTRING_H__
#include "cuiformattedpolystring.h"
#endif

#ifndef __CUIPOLYSTRING_IMPL_H__
#include "cuipolystring_impl.h"
#endif

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif


class CUIFormattedPolyString_Impl : public CUIPolyString_Impl
{
	public:
			
		const char* GetClassName() { return "CUIFormattedPolyString"; }
	
	public:

		// constructor / destructor
		CUIFormattedPolyString_Impl(CUIFont* pFont, 
					  const char* pBuf,
					  float x,
					  float y,
					  CUI_ALIGNMENTTYPE alignment);

		~CUIFormattedPolyString_Impl();


		// set data
		CUI_RESULTTYPE	SetText(const char* pBuf);
		CUI_RESULTTYPE	SetCharScreenSize(uint8 height, uint8 width);
		CUI_RESULTTYPE	SetCharScreenHeight(uint8 height);
		CUI_RESULTTYPE	SetCharScreenWidth(uint8 width);

		CUI_RESULTTYPE	SetWrapWidth(uint16 width);
		CUI_RESULTTYPE	SetAlignmentH(CUI_ALIGNMENTTYPE align);

		CUI_RESULTTYPE	SetFont(CUIFont* pFont);
		
	private:

		// applying of various attributes
		void			Parse();
		void			ApplyXYZ();
		void			ApplySizeAndFX();
		void			ApplyFont();

	private:
		
		uint16				m_WordCount;
		uint16				m_LineCount;

		uint16 				m_pWords[MAX_POLYSTRING_WORDS * 3];
		uint16 				m_pLines[MAX_POLYSTRING_LINES * 2];

		uint8*				m_pLetters;
	
		CUI_ALIGNMENTTYPE	m_Alignment;

		uint16				m_WrapWidth;
};


#endif //__CUIFORMATTEDPOLYSTRING_IMPL_H__
