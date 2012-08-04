//-------------------------------------------------------------------
//
//   MODULE    : CUIFONT_IMPL.H
//
//   PURPOSE   : defines the CUIFont_Impl font Class
//
//   CREATED   : 7/00 
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIFONT_IMPL_H__
#define __CUIFONT_IMPL_H__


#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif

class CUIFont_Impl : public CUIFont
{
	public:
			
		CUIFont_Impl();
		virtual ~CUIFont_Impl();

		// all CUI calsses support this
		virtual const char* GetClassName() = 0;

	public:
		

		bool			IsValid()				{ return m_Valid; }
		bool			IsProportional()		{ return m_Proportional; }
	
		HTEXTURE		GetTexture()			{ return m_Texture; }
		uint32			GetAttributes()			{ return m_Flags; }

		uint8			GetDefCharScreenWidth()	{ return m_DefaultCharScreenWidth; }
		uint8			GetDefCharScreenHeight(){ return m_DefaultCharScreenHeight; }

		uint8			GetCharTexWidth()		{ return m_CharTexWidth; }
		uint8			GetCharTexHeight()		{ return m_CharTexHeight; }

		uint8			GetDefSpacingH()		{ return m_DefaultHorizontalSpacing; }
		uint8			GetDefSpacingV()		{ return m_DefaultVerticalSpacing; }

		int8			GetDefSlant()			{ return m_DefaultSlant; }
		int8			GetDefBold()			{ return m_DefaultBold; }

		uint16*			GetFontTable()			{ return m_pFontTable; }
		uint8*			GetFontMap()			{ return m_pFontMap; }
		uint32*			GetDefColors()			{ return m_DefaultColors; }
			
		CUI_RESULTTYPE	SetTexture(HTEXTURE hTex, 
							   uint8 cwidth  = 0,
							   uint8 cheight = 0);
		
		CUI_RESULTTYPE	SetTexture(HTEXTURE hTex, 
							   uint16* pTable  = NULL,
							   uint8  cheight = 0);
							   
							   
		void		SetMap(uint8* fMap);
		void		SetAttributes(uint32 attrs);
		void		UnsetAttributes(uint32 attrs);
		
		void		SetDefCharWidth(uint8 width);
		void		SetDefCharHeight(uint8 height);

		void		SetDefSpacingH(uint8 spacing);
		void		SetDefSpacingV(uint8 vspacing);
		
		
		void		SetDefSlant(int8 slant);
		void		SetDefBold(int8 bold);

		void		SetDefColor(uint32 argb);

		void		SetDefColors(uint32 argb0,
							  uint32 argb1,
							  uint32 argb2,
							  uint32 argb3);


		void		DrawString(float x, float y, char* text);

		void 		Apply(CUIPolyString* pPolyStr,
						  int16 index = 0,
						  int16 num   = 0);
	
		
	protected:
		void		ReleaseDrawString();

	protected:
	
		int32		m_PointSize;
		bool		m_Valid;
		bool		m_Proportional;		
		bool		m_bAllocatedTable;
		bool		m_bAllocatedMap;	
		uint32		m_Flags;	
		HTEXTURE	m_Texture;
		uint16*		m_pFontTable;
		uint8*		m_pFontMap;

		uint8		m_CharTexWidth;
		uint8		m_CharTexHeight;
		uint8		m_DefaultCharScreenWidth;
		uint8		m_DefaultCharScreenHeight;
				
		uint8		m_DefaultHorizontalSpacing;
		uint8		m_DefaultVerticalSpacing;

		int8		m_DefaultSlant;
		int8		m_DefaultBold;

		uint32		m_DefaultColors[4];

		static CUIPolyString*	sm_pDrawStr;
};


#endif //__CUIFONT_IMPL_H__
