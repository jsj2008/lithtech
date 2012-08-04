
#include "bdefs.h"
#include "ltmodule.h"

#include "clientmgr.h"
#include "iltclient.h"
#include "sysclientde_impl.h"
#include "interface_helpers.h"
#include "text_mgr.h"
#include "stringmgr.h"


// ----------------------------------------------------------------- //
// Globals.
// ----------------------------------------------------------------- //

// All the fonts.
static CGLinkedList<LTFont*> g_Fonts;

static HDC g_hTextDC = 0;
static HBITMAP g_hTextBitmap = 0;
static HBITMAP g_hOldTextBitmap = 0;
static HBRUSH g_hOldBrush=0;
static uint32 g_TextBitmapWidth=0, g_TextBitmapHeight=0, g_TextBitmapPitch=0;
static uint8 *g_pTextBitmapBits=LTNULL;


static void tmgr_DrawTextToSurface(CisSurface *pDest, LTRect *pSrcRect, LTRect *pDestRect,
	HLTCOLOR hForeColor, HLTCOLOR hBackColor);

//a IClientFormatMgr interface
#include "client_formatmgr.h"
static IClientFormatMgr *format_mgr;
define_holder(IClientFormatMgr, format_mgr);

//ILTClient game interface
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);



// ----------------------------------------------------------------- //
// Internal helpers.
// ----------------------------------------------------------------- //

static void tmgr_DeleteFont(LTFont *pFont)
{
	DeleteObject(pFont->m_hFont);
	g_Fonts.RemoveAt(pFont);
	LTMemFree(pFont);
}

static void tmgr_DeleteAllFonts()
{
	GPOS pos;
	LTFont *pFont;

	for(pos=g_Fonts; pos; )
	{
		pFont = g_Fonts.GetNext(pos);
	
		tmgr_DeleteFont(pFont);
	}
}

static void tmgr_SizeTextSurface(SIZE *pSize)
{
	struct {
		BITMAPINFOHEADER bmi;
	} bmi;
	int fullWidth;


	if(pSize->cx > (int)g_TextBitmapWidth || pSize->cy > (int)g_TextBitmapHeight)
	{
		// Get rid of the old stuff if it exists.
		if(g_hOldTextBitmap)
			SelectObject(g_hTextDC, g_hOldTextBitmap);
		
		if(g_hTextBitmap)
			DeleteObject(g_hTextBitmap);

		
		// Create the new bitmap.
		fullWidth = pSize->cx;
		if(fullWidth % 32 != 0)
			fullWidth += (32 - (fullWidth % 32));

		memset(&bmi, 0, sizeof(bmi));

		bmi.bmi.biSize         = sizeof(BITMAPINFOHEADER);
		bmi.bmi.biWidth        = fullWidth;
		bmi.bmi.biHeight       = - (int)pSize->cy;
//		bmi.bmi.biBitCount     = 1;
		bmi.bmi.biBitCount     = 16;
		bmi.bmi.biPlanes       = 1;
		bmi.bmi.biCompression  = BI_RGB;

		g_hTextBitmap = CreateDIBSection(g_hTextDC, 
			(BITMAPINFO*)&bmi, DIB_RGB_COLORS, (void**)&g_pTextBitmapBits, LTNULL, 0);

		g_hOldTextBitmap = (HBITMAP)SelectObject(g_hTextDC, g_hTextBitmap);
		g_TextBitmapWidth = fullWidth;
		g_TextBitmapHeight = pSize->cy;
//		g_TextBitmapPitch = fullWidth / 8;
		g_TextBitmapPitch = fullWidth * 2;
	}
}


template<class P>
inline void tmgr_RasterizeTextBackground_T(
	uint8 *pSrcLine, uint32 srcX, uint8 *pDestLine, 
	long destPitch, uint32 rectWidth, uint32 rectHeight, GenericColor bkColor,
	P *pixelType)
{
	uint16 *pSrcPos;
	P destPos;
	uint32 y, xCounter;

	for(y=0; y < rectHeight; y++)
	{
		pSrcPos = ((uint16*)pSrcLine) + srcX;
		destPos = pDestLine;

		xCounter = rectWidth;
		while(xCounter)
		{
			xCounter--;
		
			if(!(*pSrcPos))
				destPos = bkColor;

			++pSrcPos;
			++destPos;
		}

		pSrcLine += g_TextBitmapPitch;
		pDestLine += destPitch;
	}
}


template<class P>
inline void tmgr_RasterizeTextForeground_T(
	uint8 *pSrcLine, uint32 srcX, uint8 *pDestLine, 
	long destPitch, uint32 rectWidth, uint32 rectHeight, GenericColor fgColor,
	P *pixelType)
{
	uint16 *pSrcPos;
	P destPos;
	uint32 y, xCounter;

	for(y=0; y < rectHeight; y++)
	{
		pSrcPos = ((uint16*)pSrcLine) + srcX;
		destPos = pDestLine;

		xCounter = rectWidth;
		while(xCounter)
		{
			xCounter--;

			if(*pSrcPos)
				destPos = fgColor;

			++pSrcPos;
			++destPos;
		}

		pSrcLine += g_TextBitmapPitch;
		pDestLine += destPitch;
	}
}


template<class P>
inline void tmgr_RasterizeText_T(
	uint8 *pSrcLine, uint32 srcX, uint8 *pDestLine, 
	long destPitch, uint32 rectWidth, uint32 rectHeight, GenericColor fgColor, GenericColor bkColor,
	P *pixelType)
{
	uint16 *pSrcPos;
	P destPos;
	uint32 y, xCounter;

	
	for(y=0; y < rectHeight; y++)
	{
		pSrcPos = ((uint16*)pSrcLine) + srcX;
		destPos = pDestLine;

		xCounter = rectWidth;
		while(xCounter)
		{
			xCounter--;

			if(*pSrcPos)
				destPos = fgColor;
			else
				destPos = bkColor;

			++pSrcPos;
			++destPos;
		}

		pSrcLine += g_TextBitmapPitch;
		pDestLine += destPitch;
	}
}


static void tmgr_InternalDrawStringToSurface(HSURFACE hDest, HLTFONT hFont, HSTRING hString, 
	LTRect *pRect, HLTCOLOR hForeColor, HLTCOLOR hBackColor, SIZE *pSize)
{
	LTFont *pFont;
	CisSurface *pDest;
	HFONT hOldFont;
	int nChars;
	LTRect textRect, destRect;


	pFont = (LTFont*)hFont;
	pDest = (CisSurface*)hDest;

	// Make sure we even have a DC.
	if(!hString || !pFont || !pDest)
		return;
	
	tmgr_SizeTextSurface(pSize);

	// Did it size?
	if(!g_hTextBitmap || !g_pTextBitmapBits)
		return;


	// Clear the HDC and draw the text into it.
	hOldFont = (HFONT)SelectObject(g_hTextDC, pFont->m_hFont);
		nChars = str_GetNumStringCharacters(hString);
		Rectangle(g_hTextDC, 0, 0, pSize->cx, pSize->cy);
		TextOut(g_hTextDC, 0, 0, (char*)str_GetStringBytes(hString, LTNULL), nChars);	
	SelectObject(g_hTextDC, hOldFont);

	// Convert from that into the surface.
	if(pRect)
	{
		destRect = *pRect;
	}
	else
	{
		destRect.left = destRect.top = 0;
		destRect.right = pDest->m_Width;
		destRect.bottom = pDest->m_Height;
	}
	
	textRect.left = textRect.top = 0;
	textRect.right = pSize->cx;
	textRect.bottom = pSize->cy;
	tmgr_DrawTextToSurface(pDest, &textRect, &destRect, hForeColor, hBackColor);
}





// ----------------------------------------------------------------- //
// Interface implementation functions.
// ----------------------------------------------------------------- //

static HLTFONT tmgr_CreateFont(const char *pFontName, int width, int height, bool bItalic,
	bool bUnderline, bool bBold)
{
	HFONT hFont;
	LTFont *pFont;

	hFont = CreateFont(height, width, 0, 0, bBold?FW_BOLD:FW_NORMAL,
		bItalic, bUnderline, LTFALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, pFontName);

	if(!hFont)
		return LTNULL;

	 LT_MEM_TRACK_ALLOC(pFont = (LTFont*)LTMemAlloc(sizeof(LTFont)),LT_MEM_TYPE_MISC);
	 pFont->m_hFont = hFont;
	 g_Fonts.AddHead(pFont);
	 
	 return (HLTFONT)pFont;
}

static void tmgr_DeleteFont(HLTFONT hFont)
{
	LTFont *pFont;

	pFont = (LTFont*)hFont;
	if(pFont)
	{
		tmgr_DeleteFont(pFont);
	}
}

static LTRESULT tmgr_SetFontExtraSpace(HLTFONT hFont, int pixels)
{
	LTFont *pFont = (LTFont*)hFont;
	HFONT hOldFont;

	if(!pFont)
		return LT_ERROR;

	hOldFont = (HFONT)SelectObject(g_hTextDC, pFont->m_hFont);
	SetTextCharacterExtra(g_hTextDC, pixels);
	SelectObject(g_hTextDC, hOldFont);

	return LT_OK;
}

static LTRESULT tmgr_GetFontExtraSpace(HLTFONT hFont, int &pixels)
{
	LTFont *pFont = (LTFont*)hFont;
	HFONT hOldFont;

	if(!pFont)
		return LT_ERROR;

	hOldFont = (HFONT)SelectObject(g_hTextDC, pFont->m_hFont);
	pixels = GetTextCharacterExtra(g_hTextDC);
	SelectObject(g_hTextDC, hOldFont);

	return LT_OK;
}

static void tmgr_GetStringDimensions(HLTFONT hFont, HSTRING hString, int *sizeX, int *sizeY)
{
	LTFont *pFont;
	int nChars;
	SIZE size;
	HFONT hOldFont;

	// Make sure they have valid parameters.
	pFont = (LTFont*)hFont;
	if(!pFont || !hString)
	{
		*sizeX = *sizeY = 0;
		return;
	}

	hOldFont = (HFONT)SelectObject(g_hTextDC, pFont->m_hFont);
		nChars = str_GetNumStringCharacters(hString);
		GetTextExtentPoint32(g_hTextDC, (char*)str_GetStringBytes(hString, LTNULL), nChars, &size);
	SelectObject(g_hTextDC, hOldFont);

	*sizeX = size.cx;
	*sizeY = size.cy;
}



static void tmgr_DrawTextToSurface(CisSurface *pDest, LTRect *pSrcRect, LTRect *pDestRect,
	HLTCOLOR hForeColor, HLTCOLOR hBackColor)
{
	LTRect srcRect, destRect;
	LTBOOL bIsVisible;
	uint8 *pSrcLine;
	uint8 *pDestLine;
	long destPitch;
	GenericColor gcForeColor, gcBackColor;


	if(!g_pTextBitmapBits)
		return;


	// Clip the rectangles..
	bIsVisible = cis_ClipRectsNonScaled(
		g_TextBitmapWidth, g_TextBitmapHeight,
		pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom,
		pDest->m_Width, pDest->m_Height,
		pDestRect->left, pDestRect->top, &srcRect, &destRect);
	if(!bIsVisible)
		return;

	// Draw!
	pDestLine = (uint8*)cis_LockSurface(pDest, destPitch, TRUE);
	if(!pDestLine)
		return;

	pSrcLine = (uint8*)g_pTextBitmapBits;
	pSrcLine += srcRect.top*g_TextBitmapPitch;
	pDestLine += destRect.top*destPitch + destRect.left*g_nScreenPixelBytes;

	format_mgr->Mgr()->PValueToFormatColor(&g_ScreenFormat, hForeColor, gcForeColor);
	format_mgr->Mgr()->PValueToFormatColor(&g_ScreenFormat, hBackColor, gcBackColor);

	// Handle the four transparency cases..
	if(IsColorTransparent(hForeColor) && IsColorTransparent(hBackColor))
	{
		// Nothing would get drawn!
	}
	else if(IsColorTransparent(hForeColor))
	{
		if(g_ScreenFormat.GetType() == BPP_16)
		{
			tmgr_RasterizeTextBackground_T(
				pSrcLine, srcRect.left, pDestLine, destPitch, 
				srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, 
				gcBackColor, (Pixel16*)LTNULL);
		}
		else if(g_ScreenFormat.GetType() == BPP_32)
		{
			tmgr_RasterizeTextBackground_T(
				pSrcLine, srcRect.left, pDestLine, destPitch, 
				srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, 
				gcBackColor, (Pixel32*)LTNULL);
		}
	}
	else if(IsColorTransparent(hBackColor))
	{
		if(g_ScreenFormat.GetType() == BPP_16)
		{
			tmgr_RasterizeTextForeground_T(
				pSrcLine, srcRect.left, pDestLine, destPitch, 
				srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, 
				gcForeColor, (Pixel16*)LTNULL);
		}
		else if(g_ScreenFormat.GetType() == BPP_32)
		{
			tmgr_RasterizeTextForeground_T(
				pSrcLine, srcRect.left, pDestLine, destPitch, 
				srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, 
				gcForeColor, (Pixel32*)LTNULL);
		}
	}
	else
	{
		if(g_ScreenFormat.GetType() == BPP_16)
		{
			tmgr_RasterizeText_T(
				pSrcLine, srcRect.left, pDestLine, destPitch, 
				srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, 
				gcForeColor, gcBackColor, (Pixel16*)LTNULL);
		}
		else if(g_ScreenFormat.GetType() == BPP_32)
		{
			tmgr_RasterizeText_T(
				pSrcLine, srcRect.left, pDestLine, destPitch, 
				srcRect.right - srcRect.left, srcRect.bottom - srcRect.top, 
				gcForeColor, gcBackColor, (Pixel32*)LTNULL);
		}
	}
	
	cis_UnlockSurface(pDest);
}


static void tmgr_DrawStringToSurface(HSURFACE hDest, HLTFONT hFont, HSTRING hString, 
	LTRect *pRect, HLTCOLOR hForeColor, HLTCOLOR hBackColor)
{
	LTFont *pFont;
	SIZE size;
	int nChars;
	HFONT hOldFont;


	// Parameter validation.
	if(!hFont || !hString || !hDest || !g_hTextDC)
		return;
	
	pFont = (LTFont*)hFont;

	// Make sure the HDC is big enough..
	hOldFont = (HFONT)SelectObject(g_hTextDC, pFont->m_hFont);
		nChars = str_GetNumStringCharacters(hString);
		GetTextExtentPoint32(g_hTextDC, (char*)str_GetStringBytes(hString, LTNULL), nChars, &size);
	SelectObject(g_hTextDC, hOldFont);

	tmgr_InternalDrawStringToSurface(hDest, hFont, hString, pRect, hForeColor, hBackColor, &size);
}


static HSURFACE tmgr_CreateSurfaceFromString(HLTFONT hFont, HSTRING hString,
	HLTCOLOR hForeColor, HLTCOLOR hBackColor, int extraPixelsX, int extraPixelsY)
{
	CisSurface *pDest;
	int nChars;
	LTFont *pFont;
	HFONT hOldFont;
	SIZE size;

	// Parameter validation.
	if(!hFont || !g_hTextDC)
		return LTNULL;
	
	pFont = (LTFont*)hFont;

	// Find out how big the surface needs to be.
	if(hString)
	{
		hOldFont = (HFONT)SelectObject(g_hTextDC, pFont->m_hFont);
			nChars = str_GetNumStringCharacters(hString);
			GetTextExtentPoint32(g_hTextDC, (char*)str_GetStringBytes(hString, LTNULL), nChars, &size);
		SelectObject(g_hTextDC, hOldFont);
	}
	else
	{
		size.cx = size.cy = 1;
	}

	size.cx += extraPixelsX;
	size.cy += extraPixelsY;
	pDest = cis_InternalCreateSurface(size.cx, size.cy);
	if(!pDest)
		return LTNULL;

	tmgr_InternalDrawStringToSurface((HSURFACE)pDest, hFont, hString, LTNULL, hForeColor, hBackColor, &size);
	return (HSURFACE)pDest;
}




// ----------------------------------------------------------------- //
// Main exposed functions.
// ----------------------------------------------------------------- //

void tmgr_Init() {
	// Setup interface pointers.
	ilt_client->CreateFont = tmgr_CreateFont;
	ilt_client->DeleteFont = tmgr_DeleteFont;
	ilt_client->SetFontExtraSpace = tmgr_SetFontExtraSpace;
	ilt_client->GetFontExtraSpace = tmgr_GetFontExtraSpace;
	ilt_client->GetStringDimensions = tmgr_GetStringDimensions;
	ilt_client->DrawStringToSurface = tmgr_DrawStringToSurface;
	ilt_client->CreateSurfaceFromString = tmgr_CreateSurfaceFromString;

	
	// Setup the HDC we will draw all the text into.
	g_hTextDC = CreateCompatibleDC(LTNULL);
	g_hOldBrush = (HBRUSH)SelectObject(g_hTextDC, GetStockObject(BLACK_BRUSH));
	SetTextColor(g_hTextDC, RGB(255,255,255));

//	SetBkMode(g_hTextDC, TRANSPARENT);
	SetBkMode(g_hTextDC, OPAQUE);
	SetTextColor(g_hTextDC, RGB(255,255,255));
	SetBkColor(g_hTextDC, RGB(0,0,0));

	g_hTextBitmap = 0;
	g_hOldTextBitmap = 0;
	g_TextBitmapWidth = g_TextBitmapHeight = 0;
	g_pTextBitmapBits = LTNULL;
}


void tmgr_Term()
{
	tmgr_DeleteAllFonts();

	// Get rid of the text DC and bitmaps.
	if(g_hTextDC)
	{
		SelectObject(g_hTextDC, g_hOldBrush);
		SelectObject(g_hTextDC, g_hOldTextBitmap);
		DeleteDC(g_hTextDC);
	}

	if(g_hTextBitmap)
	{
		DeleteObject(g_hTextBitmap);
	}

	g_hOldTextBitmap = LTNULL;
	g_hTextBitmap = LTNULL;
	g_hTextDC = LTNULL;
}

