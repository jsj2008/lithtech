//-------------------------------------------------------------------
//
//   MODULE    : CUIFONT_IMPL.CPP
//
//   PURPOSE   : implements the CUIFont Common Font Class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __CUIFONT_IMPL_H__
#include "cuifont_impl.h"
#endif

#ifndef __CUIPOLYSTRING_H__
#include "cuipolystring.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif

#ifndef __ILTFONTMANAGER_H__
#include "iltfontmanager.h"
#endif

#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif


// interface database
static ILTTexInterface *pTexInterface = NULL;
define_holder(ILTTexInterface, pTexInterface);

static ILTFontManager *pFontManager = NULL;
define_holder(ILTFontManager, pFontManager);

static ILTDrawPrim *pDrawPrimInternal = NULL;
define_holder_to_instance(ILTDrawPrim, pDrawPrimInternal, Internal);


// initialization of static data members
CUIPolyString* CUIFont_Impl::sm_pDrawStr = NULL;


//	--------------------------------------------------------------------------
CUIFont_Impl::CUIFont_Impl()
{
	m_PointSize					= 0;

	m_CharTexWidth 				= 0;
	m_CharTexHeight 			= 0;

	m_DefaultCharScreenWidth	= 0;
	m_DefaultCharScreenHeight	= 0;
				
	m_DefaultHorizontalSpacing	= 0;
	m_DefaultVerticalSpacing	= 0;

	m_Flags						= 0;

	m_DefaultSlant				= 3;
	m_DefaultBold				= 3;

	m_DefaultColors[0]			= CUI_DEFAULT_FONT_COLOR | CUI_SYSTEM_OPAQUE;
	m_DefaultColors[1]			= CUI_DEFAULT_FONT_COLOR | CUI_SYSTEM_OPAQUE;
	m_DefaultColors[2]			= CUI_DEFAULT_FONT_COLOR | CUI_SYSTEM_OPAQUE;
	m_DefaultColors[3]			= CUI_DEFAULT_FONT_COLOR | CUI_SYSTEM_OPAQUE;	

	m_bAllocatedTable			= false;
	m_bAllocatedMap				= false;
	m_Valid						= false;

	m_Texture					= NULL;

	m_pFontTable = NULL;
	m_pFontMap = NULL;
}


//	--------------------------------------------------------------------------
CUIFont_Impl::~CUIFont_Impl()
{
	// free any used resources
	if (m_bAllocatedTable && m_pFontTable)
	{
		delete [] m_pFontTable;
		m_pFontTable = NULL;
	}

	if (m_bAllocatedMap && m_pFontMap)
	{
		delete [] m_pFontMap;
		m_pFontMap = NULL;
	}

	m_CharTexWidth 		= 0;
	m_CharTexHeight 	= 0;

	// font is no longer valid
	m_Valid	= false;

	if (pFontManager->GetNumFontsInUse() == 0) {
		// I am the last valid font, so clean up sm_pDrawStr
		if (sm_pDrawStr) {
			delete sm_pDrawStr;
			sm_pDrawStr = NULL;
		}
	}
}

//	--------------------------------------------------------------------------
void CUIFont_Impl::ReleaseDrawString()
{
	// when the last font is destroyed, this should be called.
	if (sm_pDrawStr) {
		pFontManager->DestroyPolyString(sm_pDrawStr);
		sm_pDrawStr = NULL;
	}
}


//	--------------------------------------------------------------------------
CUI_RESULTTYPE	CUIFont_Impl::SetTexture(HTEXTURE hTex, 
							 	uint8 cwidth, 
								uint8 cheight) 
{
	m_Texture = hTex;
	
	if (cwidth) m_CharTexWidth   = cwidth;
	if (cheight) m_CharTexHeight = cheight;
	
	return CUIR_OK;
}


//	--------------------------------------------------------------------------
CUI_RESULTTYPE	CUIFont_Impl::SetTexture(HTEXTURE hTex, 
		 						uint16* pTable,
								uint8  cheight) 
{
	m_Texture = hTex;
	
	if (pTable)  m_pFontTable    = pTable;
	if (cheight) m_CharTexHeight = cheight;
	
	return CUIR_OK;
}


//	--------------------------------------------------------------------------
void CUIFont_Impl::SetAttributes(uint32 attrs)
{
	m_Flags |= attrs;
}


//	--------------------------------------------------------------------------
void CUIFont_Impl::UnsetAttributes(uint32 attrs)
{
	m_Flags &= (~attrs);
}



//	--------------------------------------------------------------------------
void CUIFont_Impl::SetDefCharWidth(uint8 width)
{
	m_DefaultCharScreenWidth = width;
}


//	--------------------------------------------------------------------------
void CUIFont_Impl::SetDefCharHeight(uint8 height)
{
	m_DefaultCharScreenHeight = height;
}



//	--------------------------------------------------------------------------
void CUIFont_Impl::SetDefSpacingH(uint8 hspacing)
{
	m_DefaultHorizontalSpacing = hspacing;
}


//	--------------------------------------------------------------------------
void CUIFont_Impl::SetDefSpacingV(uint8 vspacing)
{
	m_DefaultVerticalSpacing = vspacing;
}
 

//	--------------------------------------------------------------------------
void CUIFont_Impl::SetDefSlant(int8 slant)
{
	m_DefaultSlant = slant;
}
		

//	--------------------------------------------------------------------------
void CUIFont_Impl::SetDefBold(int8 bold)
{
	m_DefaultBold = bold;
}


//	--------------------------------------------------------------------------
void CUIFont_Impl::SetDefColor(uint32 argb)
{
	m_DefaultColors[0] = argb;
	m_DefaultColors[1] = argb;
	m_DefaultColors[2] = argb;
	m_DefaultColors[3] = argb;
}


//	--------------------------------------------------------------------------
void CUIFont_Impl::SetDefColors(uint32 argb0,
					  uint32 argb1,
					  uint32 argb2,
					  uint32 argb3)
{
	m_DefaultColors[0] = argb0;
	m_DefaultColors[1] = argb1;
	m_DefaultColors[2] = argb2;
	m_DefaultColors[3] = argb3;
}		


//	--------------------------------------------------------------------------
void CUIFont_Impl::SetMap(uint8* pMap)
{
	if (m_bAllocatedMap && m_pFontMap)
	{
		delete [] m_pFontMap;
	}

	m_pFontMap      = pMap;
	m_bAllocatedMap = false;
}

		
//	--------------------------------------------------------------------------
void CUIFont_Impl::DrawString(float x, float y, char* text)
{
	if (!sm_pDrawStr) {
		sm_pDrawStr = pFontManager->CreatePolyString(this, NULL, x, y);
	}

	if (sm_pDrawStr->GetFont() != this) {
		sm_pDrawStr->SetFont(this);
	}

	sm_pDrawStr->SetPosition(x,y);
	sm_pDrawStr->SetText(text);
	sm_pDrawStr->SetColors(m_DefaultColors[0], m_DefaultColors[1],
						   m_DefaultColors[2], m_DefaultColors[3]);

	sm_pDrawStr->Render(0, -1);
}
	
	
//	--------------------------------------------------------------------------
//	DEPRECATE THIS API
void CUIFont_Impl::Apply(CUIPolyString* pPolyStr, int16 index, int16 num) 
{
	pPolyStr->ApplyFont(this, index, num);
}




