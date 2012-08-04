//-------------------------------------------------------------------
//
//   MODULE    : ILTFONTMANAGER.H
//
//   PURPOSE   : defines the interface for The CUI FontManager Class
//
//   CREATED   : 7/00 
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//-------------------------------------------------------------------


//	This interface is available through the Interface Database!


#ifndef __ILTFONTMANAGER_H__
#define __ILTFONTMANAGER_H__


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __CUIFONT_H__
#include "cuifont.h"
#endif

#ifndef __CUIPOLYSTRING_H__
#include "cuipolystring.h"
#endif

#ifndef __CUIFORMATTEDPOLYSTRING_H__
#include "cuiformattedpolystring.h"
#endif

#ifndef _LTFONT_PARAMS_H_
class LTFontParams;
#endif

/*!  
The ILTFontManager interface is used to create and destroy monospace and 
proportional bitmap fonts, as well as vector (TrueType) fonts, CUIPolyStrings
and CUIFormattedPolyStrings.  

For bitmap fonts, an HTEXTURE is used to hold the font graphics, and must be
256x256 pixels or smaller.

TrueType font support is compiled into the engine by default.  If you do not
wish to use TrueType fonts, open up the file 
\b engine\engine\ui\src\freetype-2.0.1\include\freetype\config\ftoption.h
and \e #undef the macro FT_ENABLE_FREETYPE.  This will save you about 80K in
code/data size.

ILTFontmanager is available from the Interface Manager.  

\see class IBase
\see class ILTTexInterface
\see class CUIFont
\see class CUIPolyString
\see class CUIFormattedPolyString


Used for: Text and UI.
*/

class ILTFontManager : public IBase
{

	public:
			
#ifndef DOXYGEN_SHOULD_SKIP_THIS
		// add ILTFontManager to the interface database	
		interface_version(ILTFontManager, 0);
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
		
/*!
\b Required \b function \b call.  Called to initialize the ILTFontManager's 
services.  You cannot call any of the ILTFontManager's functions until
you have initialized it.  Currently this function is called within the engine.
You can check the initialization status of the ILTFontManager with a call
to IsInitted().  Calling this function again after the ILTFontManager has been
initialized has no effect.

Used for: Text and UI.
*/
	virtual void Init() = 0;

/*!
\b Required \b function \b call. Called to close down the ILTFontManager's 
services.  If you do not call Term() when the client shell is 
terminating, you risk memory leaks or worse.  Currently this function is 
called within the engine.  You can check the initialization status of the
ILTFontManager with a call to IsInitted().  Calling this function again after
the ILTFontManager has been terminated has no effect.

Used for: Text and UI.	
*/
	virtual void Term() = 0;


/*!
\return whether ILTFontManager was initialized.

Returns \b true if the ILTFontManager is initialized, or \b false if it has
not yet been initialized.

\see Init()
\see Term()

Used for: Text and UI.
*/
	virtual bool IsInitted() = 0;


/*!
\param hTex HTEXTURE containing a monospace bitmap font.
\param charw width (in pixels) of each character in the font.
\param charh height (in pixels) of each character in the font.
\param pMap (optional) specifies a character map.
\return pointer to a CUIFont object

Creates a monospace bitmap font.  No font table is necessary when creating a
monospace font.

Used for: Text and UI.
*/
		virtual CUIFont* CreateFont(HTEXTURE hTex, 
									uint8    charw, 
									uint8    charh,
									uint8*   pMap = NULL) = 0;

/*!
\param hTex HTEXTURE containing a proportional bitmap font.
\param pTable pointer to a font table (an array of uint16).  
\param charh height (in pixels) of each character in the font.
\param pMap (optional) specifies a character map.
\return pointer to a CUIFont object

Each character in the
font is represented by 3 uint16 valuess in the font table.  The first uint16 
is the width of the character in pixels, the second and third uint16 are (u,v) 
texture coordinates of the character within the texture specified by hTex.

Used for: Text and UI.
*/
		virtual CUIFont* CreateFont(HTEXTURE hTex, 
									uint16*	 pTable, 
									uint8	 charh,
									uint8*   pMap = NULL) = 0;
									
/*!
\param hTex HTEXTURE containing a proportional bitmap font.
\param pMap (optional) specifies a character map.
\return pointer to a CUIFont object

Creates a proportional bitmap font.  Instead of relying on a user-supplied
font table, the system will attempt to scan the provided texture and 
programmatically determine the extents of each letter.  The HTEXTURE parameter
must therefore be a specially prepared font graphic. Each letter is demarcated
by a single transparent green pixel (ARGB = 0x0000FF00) at its lower right 
corner.  See the images included with the CUIFont sample project.

\b Note: the HTEXTURE parameter must be a 32-bit texture for this version of 
CreateFont().

Used for: Text and UI.
*/
		virtual CUIFont* CreateFont(HTEXTURE hTex,
									uint8*   pMap = NULL) = 0;

/*!
\param pszFontFile Filename of TTF.
\param pszFontFace Font face of TTF.
\param pointSize desired point size for the new font.
\param asciiStart (optional) specifies the beginning of the range of 
characters present in the font.
\param asciiEnd (optional) specifies the end of the range of characters 
present in the font.
\return pointer to a CUIFont object

This function uses a TrueType font to create an anti-aliased bitmap font.
When the documentation refers to Vector Fonts, this is what is meant.  All
fonts are stored internally as bitmaps, but fonts can be created from TrueType
resources.

Additionally, when the font is created with this version of CreateFont(),  you
can optionally specify an ascii range of characters to include in your font
image.  The default creates a font containing characters from '!' to '~' which
gives everything generally needed for North American English.  To create a
font containing extended ascii characters, choose different values for
\e asciiStart and \e asciiEnd, or use CreateFont(uint8*, uint32, uint32, char*).

Used for: Text and UI. */
		virtual CUIFont* 	CreateFont( char const* pszFontFile, 
										char const* pszFontFace,
								  	   uint32 pointSize,
								  	   uint8  asciiStart = 33,	      /* '!' */
								  	   uint8  asciiEnd   = 126,
									   LTFontParams* fontParams = NULL) = 0;  /* '~' */
									  


/*!
\param pszFontFile Filename of TTF.
\param pszFontFace Font face of TTF.
\param pointSize desired point size for the new font.
\param characters a string containig the characters which should be present
in the font.
\return pointer to a CUIFont object

This function uses a TrueType font to create an anti-aliased bitmap font.
When the documentation refers to Vector Fonts, this is what is meant.  All
fonts are stored internally as bitmaps, but fonts can be created from TrueType
resources.

This method of creating a vector font allows for easy generation of
nonstandard fonts (fonts whose characters are not in ascii order)

All vector font creation functions create a font table, and this one creates
a font map as well.

\see CreateFont( char const*, char const*, uint32, uint32, uint8, uint8)
\see CUIFont::SetMap()

Used for: Text and UI. */
		virtual CUIFont*	CreateFont( char const* pszFontFile, 
										char const* pszFontFace,
					  				   uint32 pointSize,
					  				   char*  pCharacters,
									   LTFontParams* fontParams = NULL) = 0;

		
		
/*!
\param pFont pointer to the font to be destroyed.
\return none.

Destroys a font, and frees associated memory.

Used for: Text and UI.
*/
		virtual void		DestroyFont(CUIFont* pFont) = 0;
		
/*!
\return number of fonts currently in use.

Use this function to query the ILTFontManager about how many fonts are currently 
in use (i.e., how many valid fonts have been created but not yet destroyed).
Remember that the engine uses a CUIFont for the console, so the number returned
by GetNumFontsInUse() may be one higher than you first expect.

Used for: Text and UI.
*/		
		virtual uint32		GetNumFontsInUse() = 0;

		
		/*!
\param pTTFData a pointer to a TrueType resource.
\param dataSize the size (in bytes) of the resource pointed to by \e pTTFData.
\param character the character to render (multi-byte characters are supported)
\param pointSize the point size at which to render \e character
\return HTEXTURE containing \e character.

Use this function to render a single character into an HTEXTURE.

Used for: Text and UI.
*/
		virtual HTEXTURE	CharToTexture(uint8* pTTFData, uint32 dataSize, uint32 character, uint32 pointSize) = 0;

/*!
\param pTTFData a pointer to a TrueType resource.
\param dataSize the size (in bytes) of the resource pointed to by \e pTTFData.
\param pCharacters an array of 8-bit characters to render
\param pointSize the point size at which to render \e pCharacters
\return HTEXTURE containing \e character.

Use this function to render a string of characters into an HTEXTURE.

Used for: Text and UI.
*/
		virtual HTEXTURE	StringToTexture(uint8* pTTFData, uint32 dataSize, char* pCharacters, uint32 pointSize) = 0;

/*!
\param pTTFData a pointer to a TrueType resource.
\param dataSize the size (in bytes) of the resource pointed to by \e pTTFData.
\param pCharacters an array of multi-byte characters to render
\param pointSize the point size at which to render \e pCharacters
\return HTEXTURE containing \e character.

Use this function to render a string of characters into an HTEXTURE.

Used for: Text and UI.
*/
		virtual HTEXTURE	StringToTexture(uint8* pTTFData, uint32 dataSize, uint32* pCharacters, uint32 pointSize) = 0;

/*!
\param pFont pointer to a CUIFont.
\param pBuffer (optional) a null-terminated string of characters.
\param x (optional) the x-coordinate on screen.
\param y (optional) the y-coordinate on screen.
\return pointer to a CUIPolyString object

This function creates a CUIPolyString

\see CUIFontManager::CreateFormattedPolyString()
\see CUIFontManager::DestroyPolyString()

Used for: Text and UI. */
		virtual CUIPolyString*	CreatePolyString(CUIFont* pFont, 
											     char* pBuffer = NULL,
											     float x = 0.0,
											     float y = 0.0) = 0;

/*!
\param pFont pointer to a CUIFont.
\param pBuffer (optional) a null-terminated string of characters.
\param x (optional) the x-coordinate on screen.
\param y (optional) the y-coordinate on screen.
\param alignment the alignment of the new text.
\return pointer to a CUIFormattedPolyString object

This function creates a formatted (or, format-capable) CUIPolyString.  Unlike a regular
CUIPolyString, a formatted string understands newlines, word-wrap, justified text, and left-center-right alignment. 

\see CUIFontManager::CreatePolyString()
\see CUIFontManager::DestroyPolyString()

Used for: Text and UI. */
		virtual CUIFormattedPolyString*	CreateFormattedPolyString(CUIFont* pFont, 
											     char* pBuffer = NULL,
											     float x = 0.0,
											     float y = 0.0,
												 CUI_ALIGNMENTTYPE alignment = CUI_HALIGN_LEFT) = 0;
					  
/*!
\param pPolystr pointer to the CUIPolyString to be destroyed.
\return none

This function destroys a CUIPolyString and frees associated memory.

\see CUIFontManager::CreatePolyString()

Used for: Text and UI. */
		virtual void	 DestroyPolyString(CUIPolyString* pPolystr) = 0;
				
};

#endif //__ILTFONTMANAGER_H__
