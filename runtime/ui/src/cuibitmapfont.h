//-------------------------------------------------------------------
//
//   MODULE    : CUIBITMAPFONT.H
//
//   PURPOSE   : defines the CUIBitmapFont font class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIBITMAPFONT_H__
#define __CUIBITMAPFONT_H__


#ifndef __CUIFONT_IMPL_H__
#include "cuifont_impl.h"
#endif


class CUIBitmapFont : public CUIFont_Impl
{
	public:
			
		virtual const char* GetClassName() { return "CUIBitmapFont"; }
			
	public:
		
		// create a proportional font from HTEXTURE, font table
		CUIBitmapFont(HTEXTURE hTex, uint16* fTable, uint8 cheight, uint8* pMap);
		// create a monospace font from HTEXTURE, character width, height
		CUIBitmapFont(HTEXTURE hTex, uint8 cwidth, uint8 cheight, uint8* pMap);
		// create a font from an HTEXTURE only.  the system will attempt
		// to determine it's characteristics.
		CUIBitmapFont(HTEXTURE hTex, uint8* pMap);
		// destructor
		~CUIBitmapFont();
		
	private:
			
		bool 		CreatePropFont(const uint8* pData, uint32 nWidth, uint32 nHeight, uint32 nPitch);
		bool		BuildFontTable();

		// Creates a font map used for the ordering of charaters
		bool		BuildDefaultMap();
};


#endif //__CUIBITMAPFONT_H__
