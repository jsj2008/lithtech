
#include "bdefs.h"
#include "clientmgr.h"
#include "iltclient.h"
#include "sysclientde_impl.h"
#include "sysinterface_helpers.h"
#include "text_mgr.h"
#include "stringmgr.h"



// ----------------------------------------------------------------- //
// Globals.
// ----------------------------------------------------------------- //

// All the fonts.
static CGLinkedList<DeFont*> g_Fonts;

// static HDC g_hTextDC = 0;
// static HBITMAP g_hTextBitmap = 0;
// static HBITMAP g_hOldTextBitmap = 0;
// static HBRUSH g_hOldBrush=0;
// static uint32 g_TextBitmapWidth=0, g_TextBitmapHeight=0, g_TextBitmapPitch=0;
// static uint8 *g_pTextBitmapBits=NULL;


static void tmgr_DrawTextToSurface(CisSurface *pDest, LTRect *pSrcRect, LTRect *pDestRect,
	HLTCOLOR hForeColor, HLTCOLOR hBackColor);



// ----------------------------------------------------------------- //
// Internal helpers.
// ----------------------------------------------------------------- //

static void tmgr_DeleteFont(DeFont *pFont)
{
}

static void tmgr_DeleteAllFonts()
{
}

static void tmgr_SizeTextSurface(int *pSize)
{
}


template<class P>
static inline void tmgr_RasterizeTextBackground_T(
	uint8 *pSrcLine, uint32 srcX, uint8 *pDestLine,
	long destPitch, uint32 rectWidth, uint32 rectHeight, GenericColor bkColor,
	P *pixelType)
{
}


template<class P>
static inline
void tmgr_RasterizeTextForeground_T(
	uint8 *pSrcLine, uint32 srcX, uint8 *pDestLine,
	long destPitch, uint32 rectWidth, uint32 rectHeight, GenericColor fgColor,
	P *pixelType)
{
}


template<class P>
static inline void tmgr_RasterizeText_T(
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
	LTRect *pRect, HLTCOLOR hForeColor, HLTCOLOR hBackColor, int *pSize)
{
}





// ----------------------------------------------------------------- //
// Interface implementation functions.
// ----------------------------------------------------------------- //

static HLTFONT tmgr_CreateFont(char *pFontName, int width, int height, LTBOOL bItalic,
	LTBOOL bUnderline, LTBOOL bBold)
{
return NULL;      // DAN - temporary
}

static void tmgr_DeleteFont(HLTFONT hFont)
{
}

static void tmgr_GetStringDimensions(HLTFONT hFont, HSTRING hString, int *sizeX, int *sizeY)
{
}



static void tmgr_DrawTextToSurface(CisSurface *pDest, LTRect *pSrcRect, LTRect *pDestRect,
	HLTCOLOR hForeColor, HLTCOLOR hBackColor)
{
}


static void tmgr_DrawStringToSurface(HSURFACE hDest, HLTFONT hFont, HSTRING hString,
	LTRect *pRect, HLTCOLOR hForeColor, HLTCOLOR hBackColor)
{
}


static HSURFACE tmgr_CreateSurfaceFromString(HLTFONT hFont, HSTRING hString,
	HLTCOLOR hForeColor, HLTCOLOR hBackColor, int extraPixelsX, int extraPixelsY)
{
return NULL;      // DAN - temporary
}




// ----------------------------------------------------------------- //
// Main exposed functions.
// ----------------------------------------------------------------- //

void tmgr_Init()
{
}


void tmgr_Term()
{
}








