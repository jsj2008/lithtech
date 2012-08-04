//-------------------------------------------------------------------
//
//   MODULE    : CUIFONTMANAGER.CPP
//
//   PURPOSE   : implements the CUIFontManager Interface
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIFONTMANAGER_H__
#include "cuifontmanager.h"
#endif

#ifndef __CUIBITMAPFONT_H__
#include "cuibitmapfont.h"
#endif

#ifndef __CUIVECTORFONT_H__
#include "cuivectorfont.h"
#endif

#ifndef __CUIPOLYSTRING_H__
#include "cuipolystring.h"
#endif

#ifndef __CUIFORMATTEDPOLYSTRING_H__
#include "cuiformattedpolystring.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif


// interface database
define_interface(CUIFontManager, ILTFontManager); 


//	--------------------------------------------------------------------------
CUIFontManager::CUIFontManager()
{
	m_bInitted = false;	
}


//	--------------------------------------------------------------------------
CUIFontManager::~CUIFontManager()
{
	if (m_bInitted) {
		CUI_ERR("ILTFontManager was destroyed without calling ::Term()!\n");
	}
}


//	--------------------------------------------------------------------------
void CUIFontManager::Init()
{
	// for the moment, you may wonder why is this empty?  Its use is
	// reserved for later.  But now that ILTFontManager is availible
	// via the Interface Manager, we have no guarantee of the order in
	// which interfaces are destroyed.  Thus the need for Init/Shutdown
	// functions to be called from OnEngineTerm()

	if (m_bInitted) return;

	m_NumValidFonts = 0;

	m_bInitted = true;
}


//	--------------------------------------------------------------------------
void CUIFontManager::Term()
{
	// for the moment, you may wonder why is this empty?  Its use is
	// reserved for later.  But now that ILTFontManager is availible
	// via the Interface Manager, we have no guarantee of the order in
	// which interfaces are destroyed.  Thus the need for Init/Shutdown
	// functions to be called from OnEngineTerm()

	if (!m_bInitted) return;

	if (m_NumValidFonts) {
		CUI_ERR("ILTFontManager was terminated, but not all fonts were destroyed!\n");
	}

	m_bInitted = false;
}


//	--------------------------------------------------------------------------
CUIFont* CUIFontManager::CreateFont(HTEXTURE hTex, 
									uint8	 charw, 
									uint8	 charh,
									uint8*	 pMap)	// optional
{
	CUIFont*	pFont;

	// Create a monospace bitmap font
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	LT_MEM_TRACK_ALLOC(pFont = new CUIBitmapFont(hTex, charw, charh, pMap),LT_MEM_TYPE_UI);
	
	if (pFont) {
		if (!pFont->IsValid()) {
			delete pFont;
			pFont = NULL;
			CUI_ERR("Font creation failed\n");
		}
		else {
			m_NumValidFonts++;
		}
	}

	return pFont; // could be NULL
}


//	--------------------------------------------------------------------------
CUIFont* CUIFontManager::CreateFont(HTEXTURE hTex, 
									uint16*  pTable, 
									uint8    charh,
									uint8*   pMap)	// optional
{
	CUIFont*	pFont;

	// Create a proportional bitmap font
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	LT_MEM_TRACK_ALLOC(pFont = new CUIBitmapFont(hTex, pTable, charh, pMap),LT_MEM_TYPE_UI);	

	if (pFont) {
		if (!pFont->IsValid()) {
			delete pFont;
			pFont = NULL;
			CUI_ERR("Font creation failed\n");
		}
		else {
			m_NumValidFonts++;
		}
	}

	return pFont; // could be NULL
}


//	--------------------------------------------------------------------------
CUIFont* CUIFontManager::CreateFont(HTEXTURE hTex, 
									uint8*   pMap)	// optional
{
	CUIFont*	pFont;

	// Create a proportional bitmap font and font table
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	LT_MEM_TRACK_ALLOC(pFont = new CUIBitmapFont(hTex, pMap),LT_MEM_TYPE_UI);	

	if (pFont) {
		if (!pFont->IsValid()) {
			delete pFont;
			pFont = NULL;
			CUI_ERR("Font creation failed\n");
		}
		else {
			m_NumValidFonts++;
		}
	}

	return pFont; // could be NULL
}


//	--------------------------------------------------------------------------
CUIFont* CUIFontManager::CreateFont( char const* pszFontFile,
									char const* pszFontFace,
								  	uint32 pointSize,
								  	uint8  asciiStart,
								  	uint8  asciiEnd,
									LTFontParams* fontParams)	
{

	CUIVectorFont*	pFont;

	// Create a vector (truetype) font
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	LT_MEM_TRACK_ALLOC(pFont = new CUIVectorFont( ),LT_MEM_TYPE_UI);	
	if( pFont )
	{
		if( !pFont->Init( pszFontFile, pszFontFace, pointSize, asciiStart, asciiEnd, fontParams ))
		{
			if (!pFont->IsValid()) 
			{
				delete pFont;
				pFont = NULL;
				CUI_ERR("Font creation failed\n");
			}
		}
		else
		{
			m_NumValidFonts++;
		}
	}
	
	return pFont; // could be NULL
}


//	--------------------------------------------------------------------------
CUIFont* CUIFontManager::CreateFont( char const* pszFontFile,
									char const* pszFontFace,
					  				uint32 pointSize,
					  				char*  pCharacters,
									LTFontParams* fontParams){
	CUIVectorFont*	pFont;

	// Create a vector (truetype) font
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	LT_MEM_TRACK_ALLOC(pFont = new CUIVectorFont( ),LT_MEM_TYPE_UI);	
	if( pFont )
	{
		if( !pFont->Init( pszFontFile, pszFontFace, pointSize, pCharacters, fontParams ))
		{
			delete pFont;
			pFont = NULL;
			CUI_ERR("Font creation failed\n");
		}
		else 
		{
			m_NumValidFonts++;
		}
	}

	return pFont; // could be NULL
}


//	--------------------------------------------------------------------------
void CUIFontManager::DestroyFont(CUIFont* font)
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return;
	}

	if (font) {
		if (font->IsValid()) {
			m_NumValidFonts--;
		}
		delete font;
	}
}


//	--------------------------------------------------------------------------
uint32 CUIFontManager::GetNumFontsInUse()
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return 0;
	}

	return m_NumValidFonts; 
}


//	--------------------------------------------------------------------------
HTEXTURE CUIFontManager::CharToTexture(uint8* pTTFData, uint32 dataSize, uint32 character, uint32 pointSize)
{

	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	return NULL; // CUIVectorFont::CharToTexture(pTTFData, dataSize, character, pointSize);
}


//	--------------------------------------------------------------------------
HTEXTURE CUIFontManager::StringToTexture(uint8* pTTFData, uint32 dataSize, char* pCharacters, uint32 pointSize)
{

	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	return NULL; // CUIVectorFont::StringToTexture(pTTFData, dataSize, pCharacters, pointSize);

}


//	--------------------------------------------------------------------------
HTEXTURE CUIFontManager::StringToTexture(uint8* pTTFData, uint32 dataSize, uint32* pCharacters, uint32 pointSize)
{

	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	return NULL; // CUIVectorFont::StringToTexture(pTTFData, dataSize, pCharacters, pointSize);

}


//	--------------------------------------------------------------------------
CUIPolyString* CUIFontManager::CreatePolyString(CUIFont* font, 
										  char* buf,
										  float x,
										  float y)
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	CUIPolyString* p;
	LT_MEM_TRACK_ALLOC(p = new CUIPolyString(font, buf, x, y),LT_MEM_TYPE_UI);	
	return p;
}



//	--------------------------------------------------------------------------
CUIFormattedPolyString*	CUIFontManager::CreateFormattedPolyString(CUIFont* font, 
											     char* buf,
											     float x,
											     float y,
												 CUI_ALIGNMENTTYPE alignment)
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return NULL;
	}

	CUIFormattedPolyString*	p;
	LT_MEM_TRACK_ALLOC(p = new CUIFormattedPolyString(font, buf, x, y, alignment),LT_MEM_TYPE_UI);
	return p;
}


//	--------------------------------------------------------------------------
void CUIFontManager::DestroyPolyString(CUIPolyString* polystring)
{
	if (!m_bInitted) {
		CUI_ERR("ILTFontManager was never initialized!\n");
		return;
	}

	if (polystring) delete polystring;
}
