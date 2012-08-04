#ifndef __TEXTHELPER_H
#define __TEXTHELPER_H

class CBitmapFont;
class ILTClient;

// font structure and helper

struct FONT
{
	FONT (char* pStr, int w, int h, LTBOOL i = LTFALSE, LTBOOL u = LTFALSE, LTBOOL b = LTFALSE)
				{ SAFE_STRCPY(strFontName, pStr); nWidth = w; nHeight = h; bItalic = i; bUnderline = u; bBold = b; }

	FONT()		{ memset (strFontName, 0, 64); nWidth = 0; nHeight = 0; bItalic = LTFALSE; bUnderline = LTFALSE; bBold = LTFALSE; }

	char		strFontName[64];
	int			nWidth;
	int			nHeight;
	LTBOOL		bItalic;
	LTBOOL		bUnderline;
	LTBOOL		bBold;
};

#define SETFONT(font, str, w, h, i, u, b) SAFE_STRCPY(font.strFontName, str); font.nWidth = w; font.nHeight = h; font.bItalic = i; font.bUnderline = u; font.bBold = b

#define TH_ALIGN_LEFT		1
#define TH_ALIGN_CENTER		2
#define TH_ALIGN_RIGHT		3

// text helper class definition

class CTextHelper
{
public:

	static HSURFACE CreateSurfaceFromString (ILTClient* pClientDE, CBitmapFont* pFont, char* str, int nReplacementFont = 0);
	static HSURFACE CreateSurfaceFromString (ILTClient* pClientDE, CBitmapFont* pFont, int strID, int nReplacementFont = 0);
	static HSURFACE CreateWrappedStringSurface (ILTClient* pClientDE, int nWidth, CBitmapFont* pFont, char* str, int nAlignment = TH_ALIGN_LEFT, LTBOOL bCrop = LTTRUE);
	static HSURFACE CreateWrappedStringSurface (ILTClient* pClientDE, int nWidth, CBitmapFont* pFont, int strID, int nAlignment = TH_ALIGN_LEFT, LTBOOL bCrop = LTTRUE);

	static HSURFACE CreateSurfaceFromString (ILTClient* pClientDE, FONT* pFontDef, char* str, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);
	static HSURFACE CreateSurfaceFromString (ILTClient* pClientDE, HLTFONT HLTFONT, char* str, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);
	static HSURFACE CreateSurfaceFromString (ILTClient* pClientDE, FONT* pFontDef, int strID, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);
	static HSURFACE CreateSurfaceFromString (ILTClient* pClientDE, HLTFONT HLTFONT, int strID, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);
	
	static HSURFACE CreateWrappedStringSurface (ILTClient* pClientDE, int nWidth, FONT* pFontDef, char* str, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, int nAlignment = TH_ALIGN_LEFT, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);
	static HSURFACE CreateWrappedStringSurface (ILTClient* pClientDE, int nWidth, HLTFONT HLTFONT, char* str, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, int nAlignment = TH_ALIGN_LEFT, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);
	static HSURFACE CreateWrappedStringSurface (ILTClient* pClientDE, int nWidth, FONT* pFontDef, int strID, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, int nAlignment = TH_ALIGN_LEFT, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);
	static HSURFACE CreateWrappedStringSurface (ILTClient* pClientDE, int nWidth, HLTFONT HLTFONT, int strID, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, int nAlignment = TH_ALIGN_LEFT, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);

	static HSURFACE CreateShortenedStringSurface (ILTClient* pClientDE, int nWidth, FONT* pFontDef, char* str, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);
	static HSURFACE CreateShortenedStringSurface (ILTClient* pClientDE, int nWidth, HLTFONT HLTFONT, char* str, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);
	static HSURFACE CreateShortenedStringSurface (ILTClient* pClientDE, int nWidth, FONT* pFontDef, int strID, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);
	static HSURFACE CreateShortenedStringSurface (ILTClient* pClientDE, int nWidth, HLTFONT HLTFONT, int strID, HLTCOLOR foreColor, HLTCOLOR backColor = LTNULL, LTBOOL bCropped = LTFALSE, int nExtraX = 0, int nExtraY = 0);

protected:

	static HSURFACE CreateSurface (ILTClient* pClientDE, HLTFONT HLTFONT, HSTRING hString, HLTCOLOR foreColor, HLTCOLOR backColor, LTBOOL bCropped, int nExtraX, int nExtraY);
	static HSURFACE CreateWrappedSurface (ILTClient* pClientDE, int nWidth, HLTFONT HLTFONT, char* pString, HLTCOLOR foreColor, HLTCOLOR backColor, int nAlignment, LTBOOL bCropped, int nExtraX, int nExtraY);
	static HSURFACE CreateShortenedSurface (ILTClient* pClientDE, int nWidth, HLTFONT HLTFONT, char* pString, HLTCOLOR foreColor, HLTCOLOR backColor, LTBOOL bCropped, int nExtraX, int nExtraY);

	static HSURFACE CropSurface (ILTClient* pClientDE, HSURFACE hSurface, HLTCOLOR hBackColor);
};

LTBOOL TextHelperCheckStringID(ILTClient* pClientDE, int nStringID, const char* sCheck, LTBOOL bIgnoreCase = LTTRUE, LTBOOL bDefaultVal = LTFALSE);

LTFLOAT TextHelperGetLTFLOATValFromStringID(ILTClient* pClientDE, int nStringID, LTFLOAT nDefaultVal);

int TextHelperGetIntValFromStringID(ILTClient* pClientDE, int nStringID, int nDefaultVal);

#endif
