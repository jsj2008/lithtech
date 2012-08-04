//
//
//

#include "bdefs.h"
#include "clientmgr.h"
#include "interface_helpers.h"
#include "colorops.h"
#include <d3d9.h>

//this needs to come last since the bink headers cause conflicts with a lot of other headers
#include "binkvideomgrimpl.h"


//
//	!!!!! BINK IS NOT ENABLED
//
//		IHAVEPURCHASEDBINK Define that allows bink video player to function. ( Separate license/SDK available from rad game tools http://www.radgametools.com/)
//
//		You must recompile Exe_Lithtech with IHAVEPURCHASEDBINK defined in the project settings: 
//
//    From the SDK you purchased from bink place bink.h rad.h radbase.h and smack.h into the Engine/runtime/kernel/src/sys/win directory.
// 	Also requires bink32.dll in your path when running 
//

#if defined( IHAVEPURCHASEDBINK )


//--------------------------------------------------------------------------------
//Video vertex format: No TnL, No coloring, 1 texture
struct SVideoVertex
{
	void	Init(float fX, float fY, float fU, float fV)
	{
		m_vPos.Init(fX, fY);
		m_fRHW	= 1.0f;
		m_fU	= fU;
		m_fV	= fV;
	}

	LTVector	m_vPos;
	float		m_fRHW;
	float		m_fU;
	float		m_fV;
};

//the vertex format for this
#define VIDEOVERTEX_FVF		(D3DFVF_XYZRHW | D3DFVF_TEX1)

//--------------------------------------------------------------------------------
// Utility functions

//given a size it will find the next largest valid texture size
uint32	GetTextureDim(uint32 nVal)
{
	uint32 nSize = 1;
	while(nSize < nVal)
		nSize <<= 1;

	return nSize;
}





//--------------------------------------------------------------------------------
// BinkVideoMgr

BinkVideoMgr::BinkVideoMgr()
{
	m_BinkSoundFn    = NULL;
	m_BinkDSndFn     = NULL;
	m_BinkOpen       = NULL;
	m_BinkClose      = NULL;
	m_BinkWait       = NULL;
	m_BinkToBuffer   = NULL;
	m_BinkDoFrame    = NULL;
	m_BinkNextFrame  = NULL;
	m_hBinkDLL       = NULL;
}

BinkVideoMgr::~BinkVideoMgr()
{
	if(m_hBinkDLL)
		FreeLibrary(m_hBinkDLL);
}


LTRESULT BinkVideoMgr::Init()
{

	// load bink dll //

	m_hBinkDLL = LoadLibrary("Binkw32.dll");

	if(!m_hBinkDLL) 
	{
		dsi_ConsolePrint("Unable to find Binkw32.dll, video playback disabled.");
		RETURN_ERROR(1, BinkVideoMgr::Init, LT_MISSINGFILE);
	}

	// connect bink dll functions //

	m_BinkSoundFn         = (BinkSoundFn)GetProcAddress(m_hBinkDLL,         "_BinkSetSoundSystem@8");
	m_BinkDSndFn          = (BinkDSndFn)GetProcAddress(m_hBinkDLL,          "_BinkOpenDirectSound@4");
	m_BinkOpen            = (BinkOpenFn)GetProcAddress(m_hBinkDLL,          "_BinkOpen@8");
	m_BinkClose           = (BinkCloseFn)GetProcAddress(m_hBinkDLL,         "_BinkClose@4");
	m_BinkWait            = (BinkWaitFn)GetProcAddress(m_hBinkDLL,          "_BinkWait@4");
	m_BinkToBuffer        = (BinkToBufferFn)GetProcAddress(m_hBinkDLL,      "_BinkCopyToBuffer@28");
	m_BinkDoFrame         = (BinkDoFrameFn)GetProcAddress(m_hBinkDLL,       "_BinkDoFrame@4");
	m_BinkNextFrame       = (BinkNextFrameFn)GetProcAddress(m_hBinkDLL,     "_BinkNextFrame@4");
	m_BinkIsSoftCursorFn  = (BinkIsSoftCursorFn)GetProcAddress(m_hBinkDLL,  "_BinkIsSoftwareCursor@8");
	m_BinkCheckCursorFn   = (BinkCheckCursorFn)GetProcAddress(m_hBinkDLL,   "_BinkCheckCursor@20");
	m_BinkRestoreCursorFn = (BinkRestoreCursorFn)GetProcAddress(m_hBinkDLL, "_BinkRestoreCursor@4");
	m_BinkDX8SurfaceTypeFn= (BinkDX8SurfaceTypeFn)GetProcAddress(m_hBinkDLL,"_BinkDX8SurfaceType@4");

	// make sure functions connected //

	if( !m_BinkSoundFn || !m_BinkOpen || !m_BinkClose || !m_BinkWait ||
		!m_BinkToBuffer || !m_BinkDoFrame || !m_BinkNextFrame ||
		!m_BinkDSndFn || !m_BinkIsSoftCursorFn || 
		!m_BinkCheckCursorFn || !m_BinkDX8SurfaceTypeFn) 
	{
		dsi_ConsolePrint("Invalid Binkw32.dll, video playback disabled.");
		FreeLibrary(m_hBinkDLL);
		m_hBinkDLL = NULL;
		RETURN_ERROR(1, BinkVideoMgr::Init, LT_NOTINITIALIZED);
	}

	// setup sound

	// we just want to use direct sound here
	m_BinkSoundFn( m_BinkDSndFn, (u32) NULL );

	return LT_OK;
}

LTRESULT BinkVideoMgr::CreateScreenVideo(const char *pFilename, uint32 flags, VideoInst* &pVideo)
{
	LTRESULT dResult;
	BinkVideoInst *pRet;

	// NULL out return video //

	pVideo = NULL;

	// create a new video //

	LT_MEM_TRACK_ALLOC(pRet = new BinkVideoInst(this), LT_MEM_TYPE_MISC);
	if(!pRet) 
	{
		RETURN_ERROR(1, BinkVideoMgr, LT_OUTOFMEMORY);
	}

	// initialize "base video" //

	dResult = pRet->InitBaseVideo(pFilename, flags);
	if(dResult != LT_OK) 
	{
		pRet->Release();
		return dResult;
	}

	// initialize video //

	dResult = pRet->InitScreen();

	if(dResult != LT_OK) 
	{
		pRet->Release();
		return dResult;
	}

	// add to the list of videos //

	m_Videos.AddTail(pRet, &pRet->m_Link);

	// return the new video //

	pVideo = pRet;

	return LT_OK;
}

//--------------------------------------------------------------------------------
// BinkVideoInst

BinkVideoInst::BinkVideoInst(BinkVideoMgr *pMgr)
{
	m_pMgr				= pMgr;
	m_bnk				= NULL;
	m_pTexture			= NULL;
	m_BufferFormat		= 0;
	m_nTextureWidth		= 0;
	m_nTextureHeight	= 0;
}


BinkVideoInst::~BinkVideoInst()
{
	Term();
}


void BinkVideoInst::Term()
{
	// clean old data //
	OnRenderTerm();

	if( m_bnk ) 
	{
		// wait until the frame finishes //

		while( m_pMgr->m_BinkWait( m_bnk ) )
			;

		// close video //

		m_pMgr->m_BinkClose( m_bnk );
		m_bnk = NULL;
	}
}


LTRESULT BinkVideoInst::InitBaseVideo( const char *pFilename, uint32 flags)
{
	u32 BinkFlags;

	BinkFlags = 0;

	// set bink flags //

	if( flags & PLAYBACK_YINTERLACE ) BinkFlags |= BINKCOPY2XHI;

	if( flags & PLAYBACK_FROMHANDLE ) BinkFlags |= BINKFILEHANDLE;

	if( flags & PLAYBACK_FROMMEMORY ) BinkFlags |= BINKFROMMEMORY;

	// open bink file //

	m_bnk = m_pMgr->m_BinkOpen(pFilename, BinkFlags);
	if (!m_bnk) 
	{
		RETURN_ERROR_PARAM(1, BinkVideoInst::Init, LT_MISSINGFILE, "BinkOpen failed");
	}

	m_Flags = flags;
	return LT_OK;
}

//fills in our texture with black
bool BinkVideoInst::ClearSurface()
{
	//make sure everything is valid
	if(!m_pTexture)
		return false;

	//get the actual surface
	IDirect3DSurface9*	pSurface;

	if(FAILED(m_pTexture->GetSurfaceLevel(0, &pSurface)))
	{
		return false;
	}

	//get the surface information
	D3DSURFACE_DESC	SurfDesc;
	if(FAILED(pSurface->GetDesc(&SurfDesc)))
	{
		pSurface->Release();
		return false;
	}

	//we have the surface, we need to lock it and fill it in
	D3DLOCKED_RECT LockedRect;
	if(FAILED(pSurface->LockRect(&LockedRect, NULL, 0)))
	{
		pSurface->Release();
		return false;
	}

	//run through and clear out each line, 
	//NOTE: This assumes a 32 bit texture, which is a fairly safe assumption, but if anything
	//ever changes in the texture creation process, this should change as well
	uint8* pData = (uint8*)LockedRect.pBits;

	for(uint32 nCurrLine = 0; nCurrLine < SurfDesc.Height; nCurrLine++)
	{
		memset(pData, 0, SurfDesc.Width * 4);
		pData += LockedRect.Pitch;
	}

	//alright, unlock and return success
	pSurface->UnlockRect();
	pSurface->Release();

	return true;	
}

//the correspoinding bink format
uint32		nBinkFormatList[] = {	BINKSURFACE32,
									BINKSURFACE565,
									BINKSURFACE24 };

LTRESULT BinkVideoInst::InitScreen()
{
	// clean old data //

	OnRenderTerm();

	// Renderer must be initialized and have a renderer available
	if (!m_bnk || !r_IsRenderInitted()) 
	{ 
		RETURN_ERROR(1, BinkVideoInst::InitScreen, LT_NOTINITIALIZED); 
	}

	// We want to get the rendering device and create a texture that will fit the movie
	IDirect3DDevice9* pD3DDevice = r_GetRenderStruct()->GetD3DDevice();
	//make sure that the device is properly setup
	if(!pD3DDevice)
	{ 
		RETURN_ERROR(1, BinkVideoInst::InitScreen, LT_NOTINITIALIZED); 
	}

	//alright, now we need to figure out the size and format of the texture
	uint32 nTexWidth  = GetTextureDim(m_bnk->Width);
	uint32 nTexHeight = GetTextureDim(m_bnk->Height);

	D3DFORMAT TexFormat = r_GetRenderStruct()->GetTextureDDFormat1(BPP_32, 0);

	//now try and actually allocate this texture
	if(FAILED(pD3DDevice->CreateTexture(nTexWidth, nTexHeight, 1, 0, TexFormat, D3DPOOL_MANAGED, &m_pTexture, NULL)))
	{
		//we may want to put a fallback for cards that don't support dynamic textures, but I don't
		//think we will run across any of those...

		//we completely failed.
		RETURN_ERROR_PARAM(1, BinkVideoInst::InitScreen, LT_ERROR, "Error allocating dynamic texture for video"); 
	}

	//alright, we now have the texture, lets clear it out
	if(!ClearSurface())
	{
		m_pTexture->Release();
		m_pTexture = NULL;

		RETURN_ERROR_PARAM(1, BinkVideoInst::InitScreen, LT_ERROR, "Error clearing the video surface"); 
	}
	

	if( m_bnk->Width > g_Render.m_Width || m_bnk->Height > g_Render.m_Height )
		m_Flags |= PLAYBACK_FULLSCREEN;

	//save our dimensions
	m_nTextureWidth = nTexWidth;
	m_nTextureHeight = nTexHeight;

	//save our bink image format
	m_BufferFormat = nBinkFormatList[0];

	// decompress next frame so that we will be ready on our first update
    m_pMgr->m_BinkDoFrame( m_bnk );

	return LT_OK;
}

void BinkVideoInst::OnRenderInit()
{
	if( InitScreen() != LT_OK )
		Term();
}

void BinkVideoInst::OnRenderTerm()
{
	// release any textures if present
	if(m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = NULL;
	}

	m_nTextureWidth = 0;
	m_nTextureHeight = 0;
}

LTRESULT BinkVideoInst::Update()
{
	return LT_FINISHED;
}


LTRESULT BinkVideoInst::DrawVideo()
{
	if( !m_bnk )
		return LT_FINISHED;

	return (m_pTexture)?(UpdateOnScreen()):(LT_FINISHED);
}


LTRESULT BinkVideoInst::GetVideoStatus()
{
	if(!m_bnk)
		LT_FINISHED;

	return IsAtLastFrame() ? LT_FINISHED : LT_OK; 
}

void BinkVideoInst::Release()
{
	if(m_pMgr->m_Videos.FindElement(this) != BAD_INDEX)
		m_pMgr->m_Videos.RemoveAt(&m_Link);

	delete this;
}


bool BinkVideoInst::IsAtLastFrame()
{ 
	return (m_bnk)?(m_bnk->FrameNum == (m_bnk->Frames-1)):(LTTRUE); 
}

LTRESULT BinkVideoInst::UpdateOnScreen()
{
	// abort if at last frame
	if (IsAtLastFrame())
		return LT_OK;

	if (!m_bnk || !r_IsRenderInitted() || !m_pTexture) 
	{ 
		RETURN_ERROR(1, BinkVideoInst::UpdateOnScreen, LT_NOTINITIALIZED); 
	}

	// We want to get the rendering device
	IDirect3DDevice9* pD3DDevice = r_GetRenderStruct()->GetD3DDevice();
	//make sure that the device is properly setup
	if(!pD3DDevice)
	{ 
		RETURN_ERROR(1, BinkVideoInst::InitScreen, LT_NOTINITIALIZED); 
	}

	// wait until it is time for next frame
	while( m_pMgr->m_BinkWait(m_bnk) )
		Sleep(1);

	// Let bink handle the cursor
	m_pMgr->m_BinkCheckCursorFn((HWND)dsi_GetMainWindow(), 0, 0, m_bnk->Width, m_bnk->Height);

	//get the surface that we will be rendering to
	IDirect3DSurface9*	pSurface;
	if(FAILED(m_pTexture->GetSurfaceLevel(0, &pSurface)))
	{
		RETURN_ERROR_PARAM(1, BinkVideoInst::InitScreen, LT_ERROR, "Error obtaining the video texture surface"); 
	}

	//we have the surface, we need to lock it and fill it in
	D3DLOCKED_RECT LockedRect;
	if(FAILED(pSurface->LockRect(&LockedRect, NULL, 0)))
	{
		pSurface->Release();
		RETURN_ERROR_PARAM(1, BinkVideoInst::InitScreen, LT_ERROR, "Error locking the video texture surface"); 
	}


	//figure out what format we need bink to decompress to
//	int32 nBinkFormat = m_pMgr->m_BinkDX8SurfaceTypeFn(pSurface);


    // copy bink's decompressed pixels to destination
    m_pMgr->m_BinkToBuffer( m_bnk, LockedRect.pBits,
							LockedRect.Pitch,
							m_bnk->Height,
							0, 0,
							m_BufferFormat );

	//we are done with our surface handle
	pSurface->UnlockRect();
	pSurface->Release();

	// decompress next frame
    m_pMgr->m_BinkDoFrame( m_bnk );

	bool bSet3DState = !r_GetRenderStruct()->IsIn3D();

	//alright, now we need to actually display to the screen
	if(bSet3DState)
	{
		//startup 3d mode
		r_GetRenderStruct()->Start3D();

		//setup a full screen display
		D3DVIEWPORT9 vp;
		vp.X = 0;
		vp.Y = 0;
		vp.Width = r_GetRenderStruct()->m_Width;
		vp.Height = r_GetRenderStruct()->m_Height;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		pD3DDevice->SetViewport(&vp);

		//make sure to clear the screen
		LTRGBColor Color;
		Color.dwordVal = 0x00000000;

		r_GetRenderStruct()->Clear(NULL, CLEARSCREEN_SCREEN, Color);	
	}

	HRESULT hr;

	//setup our texture
	hr = pD3DDevice->SetTexture(0, m_pTexture);

	//setup our vertices
	hr = pD3DDevice->SetVertexShader(NULL);
	hr = pD3DDevice->SetFVF(VIDEOVERTEX_FVF);

	//we now need to figure out the position and dimensions of the video since we want to 
	//maintain the aspect ratio
	uint32 nWidth, nHeight;

	uint32 nScreenWidth		= r_GetRenderStruct()->m_Width;
	uint32 nScreenHeight	= r_GetRenderStruct()->m_Height;

	//we can basically fit either the width or height to the screen width or height and maintain
	//aspect ratio, so we will simply try both
	nWidth = nScreenWidth;
	nHeight = nWidth * m_bnk->Height / m_bnk->Width;

	if(nHeight > nScreenHeight)
	{
		nHeight = nScreenHeight;
		nWidth  = nHeight * m_bnk->Width / m_bnk->Height;
	}

	//sanity check
	assert(nWidth <= nScreenWidth);
	assert(nHeight <= nScreenHeight);

	//now figure out the offsets
	uint32 nXOffset = (nScreenWidth - nWidth) / 2;
	uint32 nYOffset = (nScreenHeight - nHeight) / 2;

	//and the rectangle for the actual video to be rendered to
	float fVidLeft		= (float)nXOffset;
	float fVidTop		= (float)nYOffset;
	float fVidRight		= (float)(nXOffset + nWidth);
	float fVidBottom	= (float)(nYOffset + nHeight);

	//figure out our bilinear offset
	float fXBilinearOffset = 0.5f / (float)m_nTextureWidth;
	float fYBilinearOffset = 0.5f / (float)m_nTextureHeight;

	//the texture locations since the video didn't necessarily fill the entire texture
	float fTexLeft		= fXBilinearOffset;
	float fTexTop		= fYBilinearOffset;
	float fTexRight		= (float)m_bnk->Width / (float)m_nTextureWidth - fXBilinearOffset;
	float fTexBottom	= (float)m_bnk->Height / (float)m_nTextureHeight - fYBilinearOffset;
	
	//setup our vertices (static to avoid possible memory transfer issues)
	static SVideoVertex Verts[4];

	Verts[0].Init(fVidLeft, fVidTop, fTexLeft, fTexTop);
	Verts[1].Init(fVidRight, fVidTop, fTexRight, fTexTop);
	Verts[2].Init(fVidRight, fVidBottom, fTexRight, fTexBottom);
	Verts[3].Init(fVidLeft, fVidBottom, fTexLeft, fTexBottom);

	//now setup the texture and the texture stage states
	pD3DDevice->SetTexture(0, m_pTexture);
	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pD3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	pD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_DISABLE);
	pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);

	//and we are ready to render
	hr = pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Verts, sizeof(SVideoVertex));

	//disable that texture
	pD3DDevice->SetTexture(0, NULL);

	if(bSet3DState)
		r_GetRenderStruct()->End3D();

	// advance frame
	if( !IsAtLastFrame() )
		m_pMgr->m_BinkNextFrame(m_bnk);

	return LT_OK;
}



#endif