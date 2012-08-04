//-------------------------------------------------------------------
//
//   MODULE    : CUIPOLYTEX.H
//
//   PURPOSE   : defines the CUIPolyTex utility class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


//	Essentially, a POLYGT4 that also keeps track of a handle to its
//	HTEXTURE

// 	NOTES:

//	If you want a texture to tile, this is possible with a polytex.
//	Set your uv coords larger than the actual texture, and it will
//	tile on a single polygon.


#ifndef __CUIPOLYTEX_H__
#define __CUIPOLYTEX_H__


#ifndef __CUI_H__
#include "cui.h"
#endif


class CUIPolyTex
{

	public:
		// making & breaking
		CUIPolyTex();

		// debug
		const char*		GetClassName() { return "CUIPolyTex"; }	
			
		// get/set
		void SetColor(uint32 argb);
		void SetColors(uint32 argb0, uint32 argb1, uint32 argb2, uint32 argb3);

		// actions
		void Render();
		
	public:
	
		LT_POLYGT4	m_Poly;
		HTEXTURE	m_Texture;
}; 


#endif // __CUIPOLYTEX_H__
