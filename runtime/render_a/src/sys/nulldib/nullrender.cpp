
#include <windows.h>
#include "ltbasedefs.h"
#include "ltbasetypes.h"
#include "colorops.h"
#include "renderstruct.h"
#include <malloc.h>
#include <string.h>

typedef uint32 DDWORD;

typedef struct DIB_BMI256_struct
{
	BITMAPINFOHEADER	hdr;
	RGBQUAD				colors[256];
} DIB_BMI;

typedef struct NullBuf_t
{
	DDWORD			m_Width, m_Height;
	unsigned short	m_Data[1];
} NullBuf;



DIB_BMI g_bmi;
HWND g_hWnd = NULL;
HBITMAP g_hBitmap = NULL;
void *g_pDibBytes = NULL;
DWORD g_DibWidth, g_DibHeight, g_DibPitchBytes;
bool g_bInOptimized2D=false;
bool g_bIn3D=false;


// ---------------------------------------------------------------- //
// Internal functions.
// ---------------------------------------------------------------- //

int nr_Init(struct RenderStructInit *pInit)
{
	RECT screenRect, wndRect;
	HDC hDC;

	

	pInit->m_RendererVersion = LTRENDER_VERSION;
	g_hWnd = (HWND)pInit->m_hWnd;
	
	// Size the window to how they want it.
	GetWindowRect(GetDesktopWindow(), &screenRect);
	
	// Setup the client rectangle.
	wndRect.left = 0;//((screenRect.right - screenRect.left) - pInit->m_Mode.m_Width) / 2;
	wndRect.top = 0;//((screenRect.right - screenRect.left) - pInit->m_Mode.m_Width) / 2;
	wndRect.right = 0;//wndRect.left + pInit->m_Mode.m_Width;
	wndRect.bottom = 0;//wndRect.top + pInit->m_Mode.m_Height;

	// Figure out the full window coordinates given the client coordinates.
	AdjustWindowRect(&wndRect, WS_OVERLAPPEDWINDOW, FALSE);

	SetWindowPos(g_hWnd, 0, wndRect.left, wndRect.top, wndRect.right-wndRect.left,
		wndRect.bottom-wndRect.top, SWP_NOREPOSITION);

	
	// Create our DIB buffer for the 'screen'.
	hDC = GetDC(NULL);
	if(hDC)
	{
		memset(&g_bmi.hdr, 0, sizeof(BITMAPINFOHEADER));

		g_bmi.hdr.biSize         = sizeof(BITMAPINFOHEADER);
		g_bmi.hdr.biWidth        = pInit->m_Mode.m_Width;
		g_bmi.hdr.biHeight       = -((int)pInit->m_Mode.m_Height);
		g_bmi.hdr.biBitCount     = 16;
		g_bmi.hdr.biPlanes       = 1;
		g_bmi.hdr.biCompression  = BI_RGB;
		g_bmi.hdr.biSizeImage    = 0L;
		g_bmi.hdr.biClrUsed      = 0;
		g_bmi.hdr.biClrImportant = 0;

		g_hBitmap = CreateDIBSection(hDC, (BITMAPINFO*)&g_bmi, DIB_PAL_COLORS, (void**)&g_pDibBytes, NULL, 0);
		if(g_hBitmap)
		{
			g_DibWidth = pInit->m_Mode.m_Width;
			g_DibHeight = pInit->m_Mode.m_Height;
			g_DibPitchBytes = g_DibWidth * 2;
		}

		ReleaseDC(NULL, hDC);
	}

	return RENDER_OK;
}


void nr_Term(bool bFullTerm)
{
	g_hWnd = 0;

	if(g_hBitmap)
	{
		DeleteObject(g_hBitmap);
		g_hBitmap = 0;
	}
	
	g_pDibBytes = NULL;
}


void nr_BindTexture(SharedTexture *pTexture, bool bTextureChanged)
{
}


void nr_UnbindTexture(SharedTexture *pTexture)
{
}


HRENDERCONTEXT nr_CreateContext()
{
	HRENDERCONTEXT p;
	LT_MEM_TRACK_ALLOC(p = (HRENDERCONTEXT)LTMemAlloc(1),LT_MEM_TYPE_RENDERER);
	return p;
}


void nr_DeleteContext(HRENDERCONTEXT hContext)
{
	if(hContext)
	{
		LTMemFree(hContext);
	}
}


void nr_Clear(LTRect *pRect, DDWORD flags, LTRGBColor& ClearColor)
{
	BYTE *pCurLine, *pEndLine;

	if(g_pDibBytes && g_hBitmap)
	{
		pCurLine = (BYTE*)g_pDibBytes;
		pEndLine = pCurLine + g_DibHeight*g_DibPitchBytes;
		while(pCurLine != pEndLine)
		{
			memset(pCurLine, 0, g_DibPitchBytes);
			pCurLine += g_DibPitchBytes;
		}
	}
}


int nr_RenderScene(struct SceneDesc *pScene)
{
	return 0;
}


void nr_RenderCommand(int argc, char **argv)
{
}


void* nr_GetHook(char *pHook)
{
	return 0;
}


void nr_SwapBuffers(uint flags)
{
	BOOL ret;
	HDC hDC;
	
	if(g_hBitmap && g_hWnd && g_pDibBytes)
	{
		hDC = GetDC(g_hWnd);
		if(hDC)
		{
			ret = StretchDIBits(hDC,
					 0, 0, g_DibWidth, g_DibHeight,
					 0, 0, g_DibWidth, g_DibHeight,
					 g_pDibBytes, (BITMAPINFO*)&g_bmi, DIB_RGB_COLORS, SRCCOPY);
			
			ReleaseDC(g_hWnd, hDC);
		}
	}
}

HLTBUFFER nr_CreateSurface(int width, int height)
{
	NullBuf *pBuf;

	LT_MEM_TRACK_ALLOC(pBuf = (NullBuf*)LTMemAlloc(sizeof(NullBuf) + ((width*height)-1) * sizeof(unsigned short)),LT_MEM_TYPE_RENDERER);
	if(pBuf)
	{
		pBuf->m_Width = width;
		pBuf->m_Height = height;
		return (HLTBUFFER)pBuf;
	}
	else
	{
		return LTNULL;
	}
}


void nr_DeleteSurface(HLTBUFFER hSurf)
{
	if(hSurf)
	{
		LTMemFree(hSurf);
	}
}


void nr_GetSurfaceInfo(HLTBUFFER hSurf, DDWORD *pWidth, DDWORD *pHeight)
{
	NullBuf *pBuf;

	pBuf = (NullBuf*)hSurf;
	
	if(pWidth) *pWidth = pBuf->m_Width;
	if(pHeight) *pHeight = pBuf->m_Height;
//	if(pPitchBytes) *pPitchBytes = pBuf->m_Width*2;
}


void* nr_LockSurface(HLTBUFFER hSurf, uint32& Pitch)
{
	NullBuf *pBuf;

	pBuf = (NullBuf*)hSurf;
	return pBuf->m_Data;
}


void nr_UnlockSurface(HLTBUFFER hSurf)
{
}


bool nr_LockScreen(int left, int top, int right, int bottom, void **pData, long *pPitch)
{
	BYTE *pStartLine;

	if(!g_pDibBytes)
		return LTFALSE;

	pStartLine = (BYTE*)g_pDibBytes;
	pStartLine += (DWORD)top * g_DibPitchBytes + (DWORD)(left << 1);
	*pData = pStartLine;
	*pPitch = g_DibPitchBytes;
	return LTTRUE;
}


void nr_UnlockScreen()
{
}


void nr_MakeScreenShot(const char *pFilename)
{
}


bool nr_Start3D()
{
	g_bIn3D = TRUE;
	return LTTRUE;
}


bool nr_End3D()
{
	g_bIn3D = FALSE;
	return LTTRUE;
}


bool nr_IsIn3D()
{
	return g_bIn3D;
}


bool nr_StartOptimized2D()
{
	g_bInOptimized2D = TRUE;
	return LTTRUE;
}


void nr_EndOptimized2D()
{
	g_bInOptimized2D = FALSE;
}


bool nr_IsInOptimized2D()
{
	return g_bInOptimized2D;
}


bool nr_OptimizeSurface(HLTBUFFER hBuffer, DDWORD transparentColor)
{
	return LTFALSE;
}


void nr_UnoptimizeSurface(HLTBUFFER hBuffer)
{
}


bool nr_QueryDeletePalette(struct DEPalette_t *pPalette)
{
	return LTTRUE;
}


void nr_ReadConsoleVariables()
{
}


LTBOOL nr_SetMasterPalette(SharedTexture *pTexture)
{
	return FALSE;
}


bool nr_GetScreenFormat(PFormat *pFormat)
{
	pFormat->Init(BPP_16, 0, RGB555_RMASK, RGB555_GMASK, RGB555_BMASK);
	return true;
}


// ---------------------------------------------------------------- //
// DLL export functions.
// ---------------------------------------------------------------- //

extern "C"
{
	void RenderDLLSetup(RenderStruct *pStruct);
	RMode* GetSupportedModes();
	void FreeModeList(RMode *pModes);
};


void rdll_RenderDLLSetup(RenderStruct *pStruct)
{
	pStruct->Start3D = nr_Start3D;
	pStruct->End3D = nr_End3D;
	pStruct->IsIn3D = nr_IsIn3D;
	pStruct->StartOptimized2D = nr_StartOptimized2D;
	pStruct->EndOptimized2D = nr_EndOptimized2D;
	pStruct->IsInOptimized2D = nr_IsInOptimized2D;
	pStruct->OptimizeSurface = nr_OptimizeSurface;
	pStruct->UnoptimizeSurface = nr_UnoptimizeSurface;
	pStruct->Init = nr_Init;
	pStruct->Term = nr_Term;
	pStruct->BindTexture = nr_BindTexture;
	pStruct->UnbindTexture = nr_UnbindTexture;
	pStruct->CreateContext = nr_CreateContext;
	pStruct->DeleteContext = nr_DeleteContext;
	pStruct->Clear = nr_Clear;
	pStruct->RenderScene = nr_RenderScene;
	pStruct->RenderCommand = nr_RenderCommand;
	pStruct->SwapBuffers = nr_SwapBuffers;
	pStruct->CreateSurface = nr_CreateSurface;
	pStruct->DeleteSurface = nr_DeleteSurface;
	pStruct->GetSurfaceInfo = nr_GetSurfaceInfo;
	pStruct->LockSurface = nr_LockSurface;
	pStruct->UnlockSurface = nr_UnlockSurface;
	pStruct->LockScreen = nr_LockScreen;
	pStruct->UnlockScreen = nr_UnlockScreen;
	pStruct->MakeScreenShot = nr_MakeScreenShot;
	pStruct->ReadConsoleVariables = nr_ReadConsoleVariables;
	pStruct->GetScreenFormat = nr_GetScreenFormat;
	pStruct->BlitToScreen = NULL;
}


RMode* rdll_GetSupportedModes()
{
	RMode *pMode;

	LT_MEM_TRACK_ALLOC(pMode = new RMode, LT_MEM_TYPE_RENDERER);
	if(pMode)
	{
		LTStrCpy(pMode->m_Description, "NullRender: (debug renderer)", sizeof(pMode->m_Description));
		LTStrCpy(pMode->m_InternalName, "NullRender", sizeof(pMode->m_InternalName));

		pMode->m_Width		= 640;
		pMode->m_Height		= 480;
		pMode->m_BitDepth	= 32;
		pMode->m_bHWTnL		= true;
		pMode->m_pNext		= LTNULL;

		return pMode;
	}
	else
	{
		return LTNULL;
	}
}


void rdll_FreeModeList(RMode *pModes)
{
//	free(pModes);
	RMode* pCur = pModes;
	while (pCur) {
		RMode* pNext = pCur->m_pNext;
		LTMemFree(pCur);
		pCur = pNext; }
}
