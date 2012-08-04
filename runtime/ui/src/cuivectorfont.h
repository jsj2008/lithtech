//-------------------------------------------------------------------
//
//   MODULE    : CUIVECTORFONT.H
//
//   PURPOSE   : defines the CUIVectorFont class
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUIVECTORFONT_H__
#define __CUIVECTORFONT_H__


#ifndef __CUIFONT_H__
#include "cuifont_impl.h"
#endif

class InstalledFontFace;
class LTFontParams;

class CUIVectorFont : public CUIFont_Impl
{
	public:
			
		virtual const char* GetClassName() { return "CUIVectorFont"; }

	public:

		CUIVectorFont( );
		~CUIVectorFont();
		
		// create a proportional font from TTF and ascii range
		bool Init( char const* pszFontFile,
					char const* pszFontFace,
					uint32 pointSize,
					uint8  asciiStart,
					uint8  asciiEnd, 
					LTFontParams* fontParams = NULL);
	
		// create a proportional font from TTF, and string
		bool Init( char const* pszFontFile,
					char const* pszFontFace,
					uint32 pointSize,
					char const* pszCharacters,
					LTFontParams* fontParams = NULL);
	
		
		void Term( );
		
	private:

		bool 					CreateFontTextureAndTable( InstalledFontFace& installedFontFace, 
									char const* pszChars, bool bMakeMap);
};


#endif //__CUIVECTORFONT_H__


