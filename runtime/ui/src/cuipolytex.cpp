//-------------------------------------------------------------------
//
//   MODULE    : CUIPOLYTEX.CPP
//
//   PURPOSE   : implements the CUIPolyTex utility class.  
//				 Essentially, an LT_POLYGT4 plus an HTEXTURE.
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIPOLYTEX_H__
#include "cuipolytex.h"
#endif

#ifndef __CUIRENDERSTATE_H__
#include "cuirenderstate.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

// use the internal drawprim interface
static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);


//	--------------------------------------------------------------------------
CUIPolyTex::CUIPolyTex()
{
	int32 v;
	
	for (v=0; v<4; v++) {
		// initialize the x,y coords, and set Z to be the front of the
		// view 
		m_Poly.verts[v].x = 0;
		m_Poly.verts[v].y = 0;
		m_Poly.verts[v].z = SCREEN_NEAR_Z;

		// initialize the u,v coords
		m_Poly.verts[v].u = 0;
		m_Poly.verts[v].v = 0;

		// set a default color, make it opaque (system dependent #define)
		m_Poly.verts[v].rgba.r = (CUI_DEFAULT_WIDGET_COLOR >> 16) & 0xFF;
		m_Poly.verts[v].rgba.g = (CUI_DEFAULT_WIDGET_COLOR >> 8)  & 0xFF;
		m_Poly.verts[v].rgba.b = (CUI_DEFAULT_WIDGET_COLOR >> 0)  & 0xFF;
		m_Poly.verts[v].rgba.a = CUI_SYSTEM_OPAQUE >> 24;		
	}		
		
	// we don't start with a texture
	m_Texture = NULL;
}


//	--------------------------------------------------------------------------
void CUIPolyTex::SetColor(uint32 argb)
{
	this->SetColors(argb, argb, argb, argb);
}


//	--------------------------------------------------------------------------
void CUIPolyTex::SetColors(uint32 argb0, uint32 argb1, uint32 argb2, uint32 argb3)
{
	// vertex 0
	m_Poly.verts[0].rgba.b = argb0 & 0xFF;
	m_Poly.verts[0].rgba.g = (argb0 >> 8)  & 0xFF;
	m_Poly.verts[0].rgba.r = (argb0 >> 16) & 0xFF;
	m_Poly.verts[0].rgba.a = (argb0 >> 24) & 0xFF;

	// vertex 1
	m_Poly.verts[1].rgba.b = argb1 & 0xFF;
	m_Poly.verts[1].rgba.g = (argb1 >> 8)  & 0xFF;
	m_Poly.verts[1].rgba.r = (argb1 >> 16) & 0xFF;
	m_Poly.verts[1].rgba.a = (argb1 >> 24) & 0xFF;

	// vertex 2
	m_Poly.verts[2].rgba.b = argb2 & 0xFF;
	m_Poly.verts[2].rgba.g = (argb2 >> 8)  & 0xFF;
	m_Poly.verts[2].rgba.r = (argb2 >> 16) & 0xFF;
	m_Poly.verts[2].rgba.a = (argb2 >> 24) & 0xFF;

	// vertex 3
	m_Poly.verts[3].rgba.b = argb3 & 0xFF;
	m_Poly.verts[3].rgba.g = (argb3 >> 8)  & 0xFF;
	m_Poly.verts[3].rgba.r = (argb3 >> 16) & 0xFF;
	m_Poly.verts[3].rgba.a = (argb3 >> 24) & 0xFF;
}


//	--------------------------------------------------------------------------
void CUIPolyTex::Render()
{
	// set up the render state	
	CUIRenderState::SetRenderState(m_Texture);

	// draw our primitive
	pDrawPrimInternal->DrawPrim(&m_Poly);
}
