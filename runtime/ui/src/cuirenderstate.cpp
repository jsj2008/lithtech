//-------------------------------------------------------------------
//
//   MODULE    : CUIRENDERSTATE.CPP
//
//   PURPOSE   : implements the CUIRenderState Utility Class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIRENDERSTATE_H__
#include "cuirenderstate.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif


// use the internal drawprim interface
static ILTDrawPrim *pDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);

static ILTTexInterface *pTexInterface = NULL;
define_holder(ILTTexInterface, pTexInterface);


//	---------------------------------------------------------------------------
void	CUIRenderState::SetRenderState(HTEXTURE hTex)
{
	pDrawPrimInternal->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	pDrawPrimInternal->SetZBufferMode(DRAWPRIM_NOZ); 
	pDrawPrimInternal->SetClipMode(DRAWPRIM_NOCLIP);
	pDrawPrimInternal->SetFillMode(DRAWPRIM_FILL);

	// it's entirely possible that the texture is NULL
	if (hTex) {

		// use color from texture and polygon	
		pDrawPrimInternal->SetColorOp(DRAWPRIM_MODULATE);
	
		// what about alpha?
		ETextureType TexType;
		pTexInterface->GetTextureType(hTex,	TexType);

		switch (TexType) {

			case TEXTURETYPE_ARGB4444:		// PC Only
			case TEXTURETYPE_ARGB8888:		// PC Only
			case TEXTURETYPE_DXT3:			// PC Only
			case TEXTURETYPE_DXT5:			// PC Only
				
				// for texture types that support more than one bit of alpha, we
				// want to do an alpha blend.

				pDrawPrimInternal->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
				pDrawPrimInternal->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
				break;
			
			case TEXTURETYPE_ARGB1555:		// PC Only
			case TEXTURETYPE_RGB565:		// PC Only
			case TEXTURETYPE_DXT1:			// PC Only
					// or
			case TEXTURETYPE_INVALID: 
			default:

				// for texture types that support only one bit of alpha (or none!),
				// we want to do an alpha test.

				pDrawPrimInternal->SetAlphaTestMode(DRAWPRIM_ALPHATEST_GREATER);
				pDrawPrimInternal->SetAlphaBlendMode(DRAWPRIM_NOBLEND);
				break;

		}
		
	}
	else {
		pDrawPrimInternal->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		pDrawPrimInternal->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		pDrawPrimInternal->SetColorOp(DRAWPRIM_NOCOLOROP);
	}
	
	// set the texture that was passed in
	pDrawPrimInternal->SetTexture(hTex);
}