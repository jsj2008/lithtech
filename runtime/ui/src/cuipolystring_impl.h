//-------------------------------------------------------------------
//
//   MODULE    : CUIPolyString_Impl.h
//
//   PURPOSE   : defines the CUIPolyString_Impl Class
//
//   CREATED   : 7/00 
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIPOLYSTRING_IMPL_H__
#define __CUIPOLYSTRING_IMPL_H__


#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif


class CUIPolyString_Impl
{
	public:
			
		const char* GetClassName() { return "CUIPolyString"; }
	
	public:

		// constructor / destructor
		CUIPolyString_Impl();

		CUIPolyString_Impl(CUIFont* pFont, 
					  const char* pBuf = NULL,
					  float x   = 0.0,
					  float y   = 0.0);

		virtual ~CUIPolyString_Impl();


		// set
			
		virtual CUI_RESULTTYPE	SetText(const char* pBuf);

		CUI_RESULTTYPE	SetPosition(float x, float y);
		
		CUI_RESULTTYPE	SetColor(uint32 argb);
		CUI_RESULTTYPE	SetColors(uint32 argb0,
							  uint32 argb1,
							  uint32 argb2,
							  uint32 argb3);

		virtual CUI_RESULTTYPE	SetCharScreenSize(uint8 width, uint8 height);
		virtual CUI_RESULTTYPE	SetCharScreenHeight(uint8 height);
		virtual CUI_RESULTTYPE	SetCharScreenWidth(uint8 width);
		
		virtual CUI_RESULTTYPE	SetFont(CUIFont* pFont);

		// get

		const char*	GetText() 	{ return m_pChars; }
		LT_POLYGT4*	GetPolys()  { return m_pPolys; }
		CUIFont*	GetFont() 	{ return m_pFont; }
		float		GetHeight() { return m_Rect.height; }
		float		GetWidth()  { return m_Rect.width;  }
		float		GetX() 		{ return m_X; }
		float		GetY() 		{ return m_Y; }
		uint16		GetLength() { return m_Length; }

		uint8		GetCharScreenWidth()	{ return m_CharScreenWidth; }
		uint8		GetCharScreenHeight()	{ return m_CharScreenHeight; }

		CUI_RESULTTYPE	GetDims(float* pWidth, float* pHeight);
		CUI_RESULTTYPE	GetPosition(float* pX, float* pY);
		CUI_RESULTTYPE	GetRect(CUIRECT* pRect);

		// actions

		void	ApplyColors();

		CUI_RESULTTYPE	ApplyFont(CUIFont* pFont, 
							  int16 index, 
							  int16 num, 
							  bool bProcessRemainder);

		CUI_RESULTTYPE	Render(int32 start, int32 end);		
		CUI_RESULTTYPE	RenderClipped(CUIRECT* pClip, int32 start, int32 end);

		bool			IsValid() { return m_Valid; }
		
	protected:

		CUI_RESULTTYPE	GetExtents(CUIRECT* pRect);

		// helper funcs for ApplyFont 
		void		MapCharacter(CUIFont* pFont, 
								 uint8 cidx,
								 LT_POLYGT4* pPoly,
								 int32 *pW, 
								 uint32 texturew, 
								 uint32 textureh);

		void		MakeBlankPoly(LT_POLYGT4* pPoly);
		
		void		SetPolyXYZ(LT_POLYGT4* pPoly, float cx, float cy, 
							   int32 w, int32 h, float em, float bold);


		// void		SetRenderState(HTEXTURE hTex);


	protected:
				
		CUIFont*	m_pFont;
		uint32		m_pColors[4];
		
		uint32		m_Flags;

		uint8		m_CharScreenWidth;
		uint8		m_CharScreenHeight;
		int8		m_Bold;
		int8		m_Italic;

		char*		m_pChars;
		LT_POLYGT4* m_pPolys;
		
		uint32		m_NumAllocatedChars;
		uint32		m_NumAllocatedPolys;

		
		float		m_X;
		float		m_Y;
		CUIRECT		m_Rect;

		
		uint16 		m_Length;	
		bool		m_Valid;
		
};


#endif //__CUIPOLYSTRING_IMPL_H__
