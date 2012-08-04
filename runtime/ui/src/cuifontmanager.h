//-------------------------------------------------------------------
//
//   MODULE    : CUIFONTMANAGER.H
//
//   PURPOSE   : defines the CUIFontManager class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIFONTMANAGER_H__
#define __CUIFONTMANAGER_H__


#ifndef __ILTFONTMANAGER_H__
#include "iltfontmanager.h"
#endif

class LTFontParams;


class CUIFontManager : public ILTFontManager
{
	public:
	
		// interface database	
		declare_interface(CUIFontManager);

		CUIFontManager();
		~CUIFontManager();

		// housekeeping
		void	Init();
		void	Term();
		bool	IsInitted()		{ return m_bInitted; }
			
		// font creation functions
		
		CUIFont* CreateFont(HTEXTURE hTex, 
							uint8  charw, 
							uint8  charh,
							uint8* pMap);
	
		CUIFont* CreateFont(HTEXTURE hTex, 
							uint16* pTable, 
							uint8  charh,
							uint8* pMap);

		CUIFont* CreateFont(HTEXTURE hTex,
							uint8* pMap);

		CUIFont* CreateFont(char const* pszFontFile, 
							char const* pszFontFace,
							uint32 pointSize,
							uint8  asciiStart,
							uint8  asciiEnd,
							LTFontParams* fontParams = NULL); 
		
		CUIFont* CreateFont(char const* pszFontFile, 
							char const* pszFontFace,
					  		uint32 pointSize,
					  		char*  pCharacters,
							LTFontParams* fontParams = NULL);


		// font destruction function

		void	 DestroyFont(CUIFont* pFont);

		// font info
		uint32	 GetNumFontsInUse();


		// single-byte char-to-texture routines
		//HTEXTURE	CharToTexture(uint8* pTTFData, uint32 dataSize, char character, uint32 pointSize);
		// multi-byte char-to-texture routines
		HTEXTURE	CharToTexture(uint8* pTTFData, uint32 dataSize, uint32 character, uint32 pointSize);

		// single-byte string-to-texture routines
		HTEXTURE	StringToTexture(uint8* pTTFData, uint32 dataSize, char* pCharacters, uint32 pointSize);
		// multi-byte string-to-texture routines
		HTEXTURE	StringToTexture(uint8* pTTFData, uint32 dataSize, uint32* pCharacters, uint32 pointSize);


		// Creating of PolyStrings

		virtual CUIPolyString*	CreatePolyString(CUIFont* pFont, 
											     char* pBuf,
											     float x,
											     float y);

		virtual CUIFormattedPolyString*	CreateFormattedPolyString(CUIFont* pFont, 
											     char* pBuf,
											     float x,
											     float y,
												 CUI_ALIGNMENTTYPE alignment);

		// PolyString destruction	
		
		virtual void DestroyPolyString(CUIPolyString* pPolystr);


	private:

		bool	m_bInitted;
		uint32	m_NumValidFonts;

};


#endif //__CUIFONTMANAGER_H__
