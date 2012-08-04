// *********************************************************************** //
//
// MODULE  : dshowVideomgrimpl.cpp
//
// PURPOSE : Uses direct show for video
//
// CREATED : 12/21/04
//
// (c) 2004-2005 Touchdown Entertainment Inc.  All Rights Reserved
//
// *********************************************************************** //

#include "bdefs.h"
#include "clientmgr.h"
#include "interface_helpers.h"
#include "colorops.h"
#include <d3d9.h>

#include "dshowvideomgrimpl.h"

//----------------------------------------------------------------------------
//
//		Helper structure - Video vertex format: No TnL, No coloring, 1 texture
//
//----------------------------------------------------------------------------

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



//----------------------------------------------------------------------------
//
//		The vertex baseline renderer
//
//----------------------------------------------------------------------------
#define VIDEOVERTEX_FVF		(D3DFVF_XYZRHW | D3DFVF_TEX1)



//----------------------------------------------------------------------------
//
//  Name:		GetTextureDim
//
//  Purpose:	given a size it will find the next largest valid texture size
//
//----------------------------------------------------------------------------
uint32	GetTextureDim(uint32 nVal)
{
	uint32 nSize = 1;
	while(nSize < nVal)
		nSize <<= 1;

	return nSize;
}



//----------------------------------------------------------------------------
//
//	Name:	IsWindowsMediaFile()
//
//	Purpose:	Check for a window media file which requires different filter
//				graph handling 
//
//----------------------------------------------------------------------------

bool IsWindowsMediaFile(LPTSTR lpszFile)
{
	TCHAR szFilename[MAX_PATH];

	// Copy the file name to a local string and convert to lowercase
	(void)StringCchCopy(szFilename, NUMELMS(szFilename), lpszFile);
	szFilename[MAX_PATH-1] = 0;
	_tcslwr(szFilename);

	if (_tcsstr(szFilename, TEXT(".asf")) ||
		_tcsstr(szFilename, TEXT(".wma")) ||
		_tcsstr(szFilename, TEXT(".wmv")))
		return true;
	else
		return false;
}



//----------------------------------------------------------------------------
//
//	Name: RenderOutputPins()
//
//	Purpose:	Render outpins ( used for windows media files )
//
//----------------------------------------------------------------------------
HRESULT RenderOutputPins(IGraphBuilder *pGB, IBaseFilter *pFilter)
{
	HRESULT         hr = S_OK;
	IEnumPins *     pEnumPin = NULL;
	IPin *          pConnectedPin = NULL, * pPin = NULL;
	PIN_DIRECTION   PinDirection;
	ULONG           ulFetched;

	// Enumerate all pins on the filter
	hr = pFilter->EnumPins(&pEnumPin);

	if(SUCCEEDED(hr))
	{
		// Step through every pin, looking for the output pins
		while (S_OK == (hr = pEnumPin->Next(1L, &pPin, &ulFetched)))
		{
			// Is this pin connected?  We're not interested in connected pins.
			hr = pPin->ConnectedTo(&pConnectedPin);
			if (pConnectedPin)
			{
				pConnectedPin->Release();
				pConnectedPin = NULL;
			}

			// If this pin is not connected, render it.
			if (VFW_E_NOT_CONNECTED == hr)
			{
				hr = pPin->QueryDirection(&PinDirection);
				if ((S_OK == hr) && (PinDirection == PINDIR_OUTPUT))
				{
					hr = pGB->Render(pPin);
				}
			}
			pPin->Release();

			// If there was an error, stop enumerating
			if (FAILED(hr))
				break;
		}
	}

	// Release pin enumerator
	pEnumPin->Release();
	return hr;
}



//----------------------------------------------------------------------------
//
//  Name:	DShowVideoMgr::DShowVideoMgr()
//
//  Purpose:	Constructor
//
//----------------------------------------------------------------------------
DShowVideoMgr::DShowVideoMgr()
{
}



//----------------------------------------------------------------------------
//  Name:		DShowVideoMgr::~DShowVideoMgr()
//
//  Purpose:	destructor
//
//----------------------------------------------------------------------------
DShowVideoMgr::~DShowVideoMgr()
{
}


//----------------------------------------------------------------------------
//  Name:		DShowVideoMgr::Init()
//
//  Purpose:	Initializer 
//
//----------------------------------------------------------------------------
LTRESULT DShowVideoMgr::Init()
{
	return LT_OK;
}



//----------------------------------------------------------------------------
//  Name:		DShowVideoMgr::CreateScreenVideo()
//
//  Purpose:	Create a new video stream 
//
//----------------------------------------------------------------------------
LTRESULT DShowVideoMgr::CreateScreenVideo(const char *pFilename, uint32 flags, VideoInst* &pVideo)
{
	LTRESULT dResult;
	DShowVideoInst * pRet;

	// Initialize the handle

	pVideo = NULL;

	// create a new video instance 

	LT_MEM_TRACK_ALLOC(pRet = new DShowVideoInst(this), LT_MEM_TYPE_MISC);

	if (!pRet) 
	{
		RETURN_ERROR(1, DShowVideoMgr::CreateScreenVideo, LT_OUTOFMEMORY );
	}

	// initialize the video

	dResult = pRet->InitVideo(pFilename, flags);

	if (dResult != LT_OK) 
	{
		pRet->Release();
		return dResult;
	}

	// add to the list of videos 
	m_Videos.AddTail(pRet, &pRet->m_Link);

	// return the handle of the video

	pVideo = pRet;

	return LT_OK;
}



//----------------------------------------------------------------------------
//  Name:		DShowVideoInst()
//
//  Purpose:	Video instance constructor
//
//----------------------------------------------------------------------------
DShowVideoInst::DShowVideoInst( DShowVideoMgr *pMgr )
{

	m_pMgr = pMgr;

	m_pGB = NULL;
	m_pMC = NULL;
	m_pMP = NULL;
	m_pME = NULL;

	m_pRenderer = NULL;
	m_pTextureRenderer = NULL;

	m_Status = LT_OK;

}



//----------------------------------------------------------------------------
//  Name:		DShowVideoInst()
//
//  Purpose:	destructor
//
//----------------------------------------------------------------------------
DShowVideoInst::~DShowVideoInst()
{
	Term();
}


//----------------------------------------------------------------------------
//  Name:		DShowVideoInst::Term()
//
//  Purpose:	Termination and cleanup a video instance
//
//----------------------------------------------------------------------------
void DShowVideoInst::Term()
{

#ifdef REGISTER_FILTERGRAPH
	// Pull graph from Running Object Table (Debug)
	RemoveFromROT();
#endif

	// Set the status as finished 
	m_Status = LT_FINISHED;

	// Shut down the graph
	if (!(!m_pMC)) m_pMC->Stop();

	// shut down the rest of the interfaces
	if (!(!m_pMC)) m_pMC.Release();
	if (!(!m_pME)) m_pME.Release();
	if (!(!m_pMP)) m_pMP.Release();
	if (!(!m_pGB)) m_pGB.Release();

	if (!(!m_pRenderer)) m_pRenderer.Release();

}



//----------------------------------------------------------------------------
//
//  Name:		DShowVideoInst::OnRenderTerm()
//
//  Purpose:	Renderer is terminating. Shutdown the video
//
//----------------------------------------------------------------------------

void DShowVideoInst::OnRenderTerm()
{
	Term();
}



//----------------------------------------------------------------------------
//
//  Name:		DShowVideoInst::InitVideo()
//
//  Purpose:	Initialize the base video 
//
//----------------------------------------------------------------------------
LTRESULT DShowVideoInst::InitVideo( const char *pFilename, uint32 flags)
{
	HRESULT hr = S_OK;

	TCHAR strFileName[MAX_PATH];
	WCHAR wFileName[MAX_PATH];

	CComPtr<IBaseFilter>    pFSrc;          // Source Filter
	CComPtr<IPin>           pFSrcPinOut;    // Source Filter Output Pin


	// Renderer must be initialized and have a renderer available
	if (!r_IsRenderInitted()) 
	{ 
		RETURN_ERROR(1, DShowVideoInst::InitScreen, LT_NOTINITIALIZED); 
	}


	// Determine the file to load based the file type passed
	// Use the standard win32 API to do this.
	strFileName[0] = 0;

	(void)StringCchCat( strFileName, NUMELMS(strFileName), pFilename );

	strFileName[MAX_PATH-1] = 0;  // NULL-terminate

	wFileName[MAX_PATH-1] = 0;    // NULL-terminate

	USES_CONVERSION;

	(void)StringCchCopyW(wFileName, NUMELMS(wFileName), T2W(strFileName));


	// Create our filter graph
	if (FAILED(m_pGB.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC)))
	{
		RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "DShow Open failed");
	}

#ifdef REGISTER_FILTERGRAPH
	// Register the graph in the Running Object Table (for debug purposes)
	AddToROT(m_pGB);
#endif

	// Create the Texture Renderer object
	LT_MEM_TRACK_ALLOC(m_pTextureRenderer = new CTextureRenderer(NULL, &hr), LT_MEM_TYPE_MISC);

	if (FAILED(hr) || !m_pTextureRenderer)
	{
		RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "Could not create texture renderer object!");
	}

	// Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
	m_pRenderer = m_pTextureRenderer;

	if (FAILED(hr = m_pGB->AddFilter(m_pRenderer, L"TEXTURERENDERER")))
	{
		RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "Could not add renderer filter to graph!");
	}


	// Special case for windows media files -> asf wmv and wma
	if (IsWindowsMediaFile(strFileName))
	{
		CComPtr<IBaseFilter>    pReader;						// Source Reader ( will be released when it goes out of scope )
		CComPtr<IFileSourceFilter> pFileSource = NULL;	// SourceFile Loader ( will be released when it goes out of scope )

		// Load the improved ASF reader filter by CLSID ( qasf.dll )
		if(FAILED(hr = pReader.CoCreateInstance(CLSID_WMAsfReader, NULL, CLSCTX_INPROC )))
		{
			RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "Failed to create WMAsfFilter!");
		}

		// Add the ASF reader filter to the graph.  For ASF/WMV/WMA content,
		// this filter is NOT the default and must be added explicitly.
		if ( FAILED(hr = m_pGB->AddFilter( pReader, L"ASF Reader" ) ))
		{
			RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "Failed to add ASF reader filter to graph!");
		}

		// Set the source file filter 
		if (FAILED(hr = pReader->QueryInterface(IID_IFileSourceFilter, (void **) &pFileSource)))
		{
			RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "Failed to load source filter!");
		}

		// Attempt to load this file ( set the source filename )
		hr = pFileSource->Load(wFileName, NULL);

		// NOTE: We don't play licensed content 
		// Handle Digital Rights Management (DRM) errors
		if ((0xC00D0BBEL == hr) || 
				(0xC00D0BBDL == hr) )
		{
			GENERATE_ERROR(1, DShowVideoInst::InitBaseVideo, LT_ERROR, "This media file is protected by DRM and needs a license.");
			return hr;
		}
		else if (FAILED(hr))
		{
			GENERATE_ERROR(1, DShowVideoInst::InitBaseVideo, LT_ERROR, "Failed to load file into source filter.");
			return hr;
		}

		// Render the output pins of the ASF reader to build the
		// remainder of the graph automatically and connect the video stream to the
		// texture renderer 
		RenderOutputPins( m_pGB, pReader );

	}
	else // Non windows media files 
	{

		// Add the source filter to the graph. ( based on the filename ? )
		hr = m_pGB->AddSourceFilter (wFileName, L"SOURCE", &pFSrc);

		// If the media file was not found, inform the user.
		if (hr == VFW_E_NOT_FOUND)
		{
			RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "File not found!" );
		}
		else if(FAILED(hr))
		{
			RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "Could not find filter for file!" );
		}

		// Find output pin for the media file ( decompressor ? )
		if (FAILED(hr = pFSrc->FindPin(L"Output", &pFSrcPinOut)))
		{
			RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "Filter output missing!" );
		}

		// Render the source filter's output pin.  The Filter Graph Manager
		// will connect the video stream to the loaded CTextureRenderer
		// and will load and connect an audio renderer (if needed).

		if (FAILED(hr = m_pGB->Render(pFSrcPinOut)))
		{
			RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "Filter output missing!" );
			return hr;
		}

	}


	// Get the graph's media control, event & position interfaces
	m_pGB.QueryInterface(&m_pMC);
	m_pGB.QueryInterface(&m_pMP);
	m_pGB.QueryInterface(&m_pME);

	// Start the graph running;
	if (FAILED(hr = m_pMC->Run()))
	{
		RETURN_ERROR_PARAM(1, DShowVideoInst::InitBaseVideo, LT_MISSINGFILE, "Could not run the DirectShow graph!" );
	}

	m_flags = flags;

	return LT_OK;
}



//----------------------------------------------------------------------------
//  Name:		DShowVideoInst::ClearSurface()
//
//  Purpose:	fills in our texture with black
//
//----------------------------------------------------------------------------
bool DShowVideoInst::ClearSurface()
{
	//make sure everything is valid
	if( !m_pTextureRenderer->m_pTexture )
		return false;

	//get the actual surface
	IDirect3DSurface9*	pSurface;

	if(FAILED(m_pTextureRenderer->m_pTexture->GetSurfaceLevel(0, &pSurface)))
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

	// run through and clear out each line, 
	// NOTE: This assumes a 32 bit texture, which is a fairly safe assumption, but if anything
	// ever changes in the texture creation process, this should change as well
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



//----------------------------------------------------------------------------
//  Name:		DShowVideoInst::Update()
//
//  Purpose:	Called in clientmgr during frame update. Update the player 
//					status.
//
//----------------------------------------------------------------------------
LTRESULT DShowVideoInst::Update()
{
	long lEventCode;
	LONG_PTR lParam1, lParam2;
	HRESULT hr;

	// If we dont have the media event for the graph we must be done
	if ( !m_pME )
	{
		m_Status = LT_FINISHED;
	}
	else // Check the videos status 
	{

		// Check for completion events
		hr = m_pME->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
		if (SUCCEEDED(hr))
		{
			// If we have reached the end of the media file, reset to beginning
			if (EC_COMPLETE == lEventCode)
			{
				m_Status = LT_FINISHED;
			}

		}

		// Free any memory associated with this event
		hr = m_pME->FreeEventParams(lEventCode, lParam1, lParam2);
	}

	return m_Status;

}



//----------------------------------------------------------------------------
//
//  Name:		DShowVideoInst::DrawVideo()
//
//  Purpose:	
//
//----------------------------------------------------------------------------
LTRESULT DShowVideoInst::DrawVideo()
{
	// Check the graph
	if ( !m_pGB )
	{
		return LT_FINISHED;
	}

	return (m_pTextureRenderer->m_pTexture)?(UpdateOnScreen()):(LT_FINISHED);
}


//----------------------------------------------------------------------------
//  Name:		DShowVideoInst::GetVideoStatus()
//
//  Purpose:	Return the current player status. Status is updated in Update()
//
//			LT_NOTINITIALIZED - Video module not initialized. 
//			LT_INVALIDPARAMS - hVideo is invalid.
//			LT_FINISHED - Finished playing.
//			LT_OK - Still playing.
//----------------------------------------------------------------------------
LTRESULT DShowVideoInst::GetVideoStatus()
{
	return m_Status;
}



//----------------------------------------------------------------------------
//  Name:		DShowVideoInst::Release()
//
//  Purpose:	
//
//----------------------------------------------------------------------------
void DShowVideoInst::Release()
{
	// remove ourselves from the video manager
	if (m_pMgr->m_Videos.FindElement(this) != BAD_INDEX)
	{
		 m_pMgr->m_Videos.RemoveAt(&m_Link);
	}

	// Remove our instance 
	LT_MEM_TRACK_FREE( delete this );

}



//----------------------------------------------------------------------------
//  Name:		DShowVideoInst::UpdateOnScreen()
//
//  Purpose:	
//
//----------------------------------------------------------------------------
LTRESULT DShowVideoInst::UpdateOnScreen()
{
	HRESULT hr;

	if ( m_Status == LT_FINISHED )
	{
		return LT_OK;
	}

	// Check Media pointers render etc 
	if (!m_pGB || !r_IsRenderInitted() || !m_pRenderer) 
	{ 
		RETURN_ERROR(1, DShowVideoInst::UpdateOnScreen, LT_NOTINITIALIZED); 
	}

	// Don't render unless we have something to sample
	if ( !m_pTextureRenderer->m_StartedSampling )
	{	
		return LT_OK;
	}


	// We want to get the rendering device
	IDirect3DDevice9* pD3DDevice = r_GetRenderStruct()->GetD3DDevice();

	// make sure we have a device
	if ( !pD3DDevice )
	{ 
		RETURN_ERROR(1, DShowVideoInst::InitScreen, LT_NOTINITIALIZED); 
	}

	// check to see if we are in 2d or 3d mode 
	bool bSet3DState = !r_GetRenderStruct()->IsIn3D();

	// Set 3d mode if needed
	if (bSet3DState)
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

	//setup our texture
	hr = pD3DDevice->SetTexture(0, m_pTextureRenderer->m_pTexture );

	//setup our vertices
	hr = pD3DDevice->SetVertexShader(NULL);
	hr = pD3DDevice->SetFVF(VIDEOVERTEX_FVF);

	//we now need to figure out the position and dimensions of the video since we want to 
	//maintain the aspect ratio
	uint32 nWidth, nHeight;

	uint32 nScreenWidth	= r_GetRenderStruct()->m_Width;
	uint32 nScreenHeight	= r_GetRenderStruct()->m_Height;

	//we can basically fit either the width or height to the screen width or height and maintain
	//aspect ratio, so we will simply try both
	nWidth = nScreenWidth;
	nHeight = nWidth * m_pTextureRenderer->m_lVidHeight / m_pTextureRenderer->m_lVidWidth;

	if ( nHeight > nScreenHeight )
	{
		nHeight = nScreenHeight;
		nWidth  = nHeight * m_pTextureRenderer->m_lVidWidth / m_pTextureRenderer->m_lVidHeight;
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
	float fXBilinearOffset = 0.5f / (float)m_pTextureRenderer->m_nTextureWidth;
	float fYBilinearOffset = 0.5f / (float)m_pTextureRenderer->m_nTextureHeight;

	//the texture locations since the video didn't necessarily fill the entire texture
	float fTexLeft		= fXBilinearOffset;
	float fTexTop		= fYBilinearOffset;
	float fTexRight	= (float)m_pTextureRenderer->m_lVidWidth / (float)m_pTextureRenderer->m_nTextureWidth - fXBilinearOffset;
	float fTexBottom	= (float)m_pTextureRenderer->m_lVidHeight / (float)m_pTextureRenderer->m_nTextureHeight - fYBilinearOffset;
	
	// setup our vertices (static to avoid possible memory transfer issues)
	static SVideoVertex Verts[4];

	Verts[0].Init(fVidLeft, fVidTop, fTexLeft, fTexBottom);
	Verts[1].Init(fVidRight, fVidTop, fTexRight, fTexBottom);
	Verts[2].Init(fVidRight, fVidBottom, fTexRight, fTexTop);
	Verts[3].Init(fVidLeft, fVidBottom, fTexLeft, fTexTop);

	//now setup the texture and the texture stage states
	pD3DDevice->SetTexture(0, m_pTextureRenderer->m_pTexture);
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

	if (bSet3DState)
	{
		r_GetRenderStruct()->End3D();
	}

	return LT_OK;

}



//----------------------------------------------------------------------------
//  Name:		CTextureRenderer()
//
//  Purpose:	Constructor
//
//----------------------------------------------------------------------------
CTextureRenderer::CTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr )
                                  : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer),
                                    NAME("Texture Renderer"), pUnk, phr),
                                    m_bUseDynamicTextures(FALSE)
{
	// Store and AddRef the texture for our use.
	ASSERT(phr);
	if (phr)
	{
        *phr = S_OK;
	}

	// Initialize other varibles here ? 
	m_pTexture			= NULL;

	m_TextureFormat	= D3DFMT_UNKNOWN;

	m_nTextureWidth	= 0;
	m_nTextureHeight	= 0;

	m_StartedSampling = false;

}


//-----------------------------------------------------------------------------
//  Name:		~CTextureRenderer()
//
//  Purpose:	destructor
//
//----------------------------------------------------------------------------
CTextureRenderer::~CTextureRenderer()
{
	if ( m_pTexture )
	{
		m_pTexture->Release();
		m_pTexture = NULL;
		m_TextureFormat	= D3DFMT_UNKNOWN;

		m_nTextureWidth	= 0;
		m_nTextureHeight	= 0;
	}

	m_StartedSampling = false;
}



//----------------------------------------------------------------------------
//  Name:		CTextureRenderer::CheckMediaType()
//
//  Purpose:	This method forces the graph to give us an R8G8B8 video
//					type, making our copy to texture memory trivial.
//
//----------------------------------------------------------------------------
HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
	HRESULT   hr = E_FAIL;
	VIDEOINFO *pvi=0;

	CheckPointer(pmt,E_POINTER);

	// Reject the connection if this is not a video type
	if( *pmt->FormatType() != FORMAT_VideoInfo ) 
	{
		return E_INVALIDARG;
	}

	// Only accept RGB24 video
	pvi = (VIDEOINFO *)pmt->Format();

	if ( IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video) )
	{
		if ( IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24) )
		{
			hr = S_OK;
		}
	}

	return hr;
}



//-----------------------------------------------------------------------------
//  Name:		CTextureRenderer::SetMediaType()
//
//  Purpose:	Graph Connection has been made. Create everything 
//
//----------------------------------------------------------------------------
HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
	HRESULT hr;

	UINT uintWidth = 2;
	UINT uintHeight = 2;

	// Retrive the size of this media type
	D3DCAPS9 caps;
	VIDEOINFO *pviBmp;                      // Bitmap info header
	pviBmp = (VIDEOINFO *)pmt->Format();

	m_lVidWidth  = pviBmp->bmiHeader.biWidth;
	m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);
	m_lVidPitch  = (m_lVidWidth * 3 + 3) & ~(3); // We are forcing RGB24

	IDirect3DDevice9* pD3DDevice = r_GetRenderStruct()->GetD3DDevice();

	//make sure that the device is properly setup
	if (!pD3DDevice)
	{ 
		GENERATE_ERROR( 1, CTextureRenderer::SetMediaType, LT_ERROR, "Could not get 3D Device");
		return E_UNEXPECTED;
	}

	// here let's check if we can use dynamic textures
	ZeroMemory( &caps, sizeof(D3DCAPS9));

	hr = pD3DDevice->GetDeviceCaps( &caps );

	if ( caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES )
	{
		m_bUseDynamicTextures = TRUE;
	}

	// Create a texture the next power of 2 larger 
	if ( caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
	{
		while( (LONG)uintWidth < m_lVidWidth )
		{
			uintWidth = uintWidth << 1;
		}

		while( (LONG)uintHeight < m_lVidHeight )
		{
			uintHeight = uintHeight << 1;
		}
	}
	else // otherwise just use the video width to create the texture 
	{
		uintWidth = m_lVidWidth;
		uintHeight = m_lVidHeight;
	}

	// Create the texture that maps to this media type
	hr = E_UNEXPECTED;

	if ( m_bUseDynamicTextures )
	{
		hr = pD3DDevice->CreateTexture( uintWidth, uintHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,
													&m_pTexture, NULL);
		if ( FAILED(hr) )
		{
			m_bUseDynamicTextures = FALSE;
		}
	}

	if ( FALSE == m_bUseDynamicTextures )
	{
		hr = pD3DDevice->CreateTexture( uintWidth, uintHeight, 1, 0,	D3DFMT_X8R8G8B8,D3DPOOL_MANAGED,
													&m_pTexture, NULL);
	}

	if ( FAILED(hr))
	{
		GENERATE_ERROR(1, CTextureRenderer::SetMediaType, LT_ERROR, "Could not create texture");
		return hr;
	}

	// CreateTexture can silently change the parameters on us
	D3DSURFACE_DESC ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));

	if ( FAILED( hr = m_pTexture->GetLevelDesc( 0, &ddsd ) ) ) 
	{
		GENERATE_ERROR(1, CTextureRenderer::SetMediaType, LT_ERROR, "Could not get level description of texture");
		return hr;
	}


	CComPtr<IDirect3DSurface9> pSurf;

	if (SUCCEEDED(hr = m_pTexture->GetSurfaceLevel(0, &pSurf)))
	{
		pSurf->GetDesc(&ddsd);
	}

	// Save the texture format info and dimensions
	m_TextureFormat = ddsd.Format;

	m_nTextureWidth = uintWidth;
	m_nTextureHeight = uintHeight;


	// Check the texture format 
	if ( m_TextureFormat != D3DFMT_X8R8G8B8 &&
			m_TextureFormat != D3DFMT_A1R5G5B5 ) 
	{
		GENERATE_ERROR(1, CTextureRenderer::SetMediaType, LT_ERROR, "Texture is a format we can't handle!");
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	return S_OK;
}


//-----------------------------------------------------------------------------
//  Name:		CTextureRenderer::DoRenderSample()
//
//  Purpose:	A sample has been delivered. Copy it to the texture
//
//----------------------------------------------------------------------------
HRESULT CTextureRenderer::DoRenderSample( IMediaSample * pSample )
{
	BYTE  *pBmpBuffer, *pTxtBuffer; // Bitmap buffer, texture buffer
	LONG  lTxtPitch;                // Pitch of bitmap, texture
	HRESULT hr;

	BYTE  * pbS = NULL;
	DWORD * pdwS = NULL;
	DWORD * pdwD = NULL;
	UINT row, col, dwordWidth;

	CheckPointer(pSample,E_POINTER);
	CheckPointer(m_pTexture,E_UNEXPECTED);

	// Get the video bitmap buffer
	pSample->GetPointer( &pBmpBuffer );

	// Lock the Texture
	D3DLOCKED_RECT d3dlr;

	if ( m_bUseDynamicTextures )
	{
		if( FAILED(hr = m_pTexture->LockRect(0, &d3dlr, 0, D3DLOCK_DISCARD)))
		{
			GENERATE_ERROR(1, CTextureRenderer::DoRenderSample, LT_ERROR, "LockRect Failed!");
			return E_FAIL;
		}
	}
	else
	{
		if (FAILED(m_pTexture->LockRect(0, &d3dlr, 0, 0)))
		{
			GENERATE_ERROR(1, CTextureRenderer::DoRenderSample, LT_ERROR, "LockRect Failed!");
			return E_FAIL;
		}

	}

	// Get the texture buffer & pitch
	pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
	lTxtPitch = d3dlr.Pitch;


	// Copy the bits

	if (m_TextureFormat == D3DFMT_X8R8G8B8)
	{
		// Instead of copying data bytewise, we use DWORD alignment here.
		// We also unroll loop by copying 4 pixels at once.
		//
		// original BYTE array is [b0][g0][r0][b1][g1][r1][b2][g2][r2][b3][g3][r3]
		//
		// aligned DWORD array is     [b1 r0 g0 b0][g2 b2 r1 g1][r3 g3 b3 r2]
		//
		// We want to transform it to [ff r0 g0 b0][ff r1 g1 b1][ff r2 g2 b2][ff r3 b3 g3]
		// below, bitwise operations do exactly this.

		dwordWidth = m_lVidWidth / 4; // aligned width of the row, in DWORDS
		// (pixel by 3 bytes over sizeof(DWORD))

		for( row = 0; row< (UINT)m_lVidHeight; row++)
		{
			pdwS = ( DWORD*)pBmpBuffer;
			pdwD = ( DWORD*)pTxtBuffer;

			for( col = 0; col < dwordWidth; col ++ )
			{
				pdwD[0] =  pdwS[0] | 0xFF000000;
				pdwD[1] = ((pdwS[1]<<8)  | 0xFF000000) | (pdwS[0]>>24);
				pdwD[2] = ((pdwS[2]<<16) | 0xFF000000) | (pdwS[1]>>16);
				pdwD[3] = 0xFF000000 | (pdwS[2]>>8);
				pdwD +=4;
				pdwS +=3;
			}

			// we might have remaining (misaligned) bytes here
			pbS = (BYTE*) pdwS;
			for( col = 0; col < (UINT)m_lVidWidth % 4; col++)
			{
				*pdwD = 0xFF000000     |
					(pbS[2] << 16) |
					(pbS[1] <<  8) |
					(pbS[0]);
				pdwD++;
				pbS += 3;
			}

			pBmpBuffer  += m_lVidPitch;
			pTxtBuffer += lTxtPitch;
		}// for rows
	} 
	else if (m_TextureFormat == D3DFMT_A1R5G5B5)
	{
		for(int y = 0; y < m_lVidHeight; y++ )
		{
			BYTE *pBmpBufferOld = pBmpBuffer;
			BYTE *pTxtBufferOld = pTxtBuffer;

			for (int x = 0; x < m_lVidWidth; x++)
			{
				*(WORD *)pTxtBuffer = (WORD)
					(0x8000 +
					((pBmpBuffer[2] & 0xF8) << 7) +
					((pBmpBuffer[1] & 0xF8) << 2) +
					(pBmpBuffer[0] >> 3));

				pTxtBuffer += 2;
				pBmpBuffer += 3;
			}

			pBmpBuffer = pBmpBufferOld + m_lVidPitch;
			pTxtBuffer = pTxtBufferOld + lTxtPitch;
		}
	}
	else
	{
		GENERATE_ERROR(1, CTextureRenderer::DoRenderSample, LT_ERROR, "Unknown texture format!");
	}

	// Unlock the Texture
	if (FAILED(m_pTexture->UnlockRect(0)))
		return E_FAIL;

	// signal that we have something to render
	m_StartedSampling = true;

	return S_OK;
}



// Routines for debugging the filter graph - Never tested 



#ifdef REGISTER_FILTERGRAPH

//-----------------------------------------------------------------------------
// Running Object Table functions: Used to debug. By registering the graph
// in the running object table, GraphEdit is able to connect to the running
// graph. This code should be removed before the application is shipped in
// order to avoid third parties from spying on your graph.
//-----------------------------------------------------------------------------
DWORD dwROTReg = 0xfedcba98;

HRESULT AddToROT(IUnknown *pUnkGraph)
{
	IMoniker * pmk;
	IRunningObjectTable *pROT;
	if (FAILED(GetRunningObjectTable(0, &pROT))) {
		return E_FAIL;
	}

	WCHAR wsz[256];
	(void)StringCchPrintfW(wsz, NUMELMS(wsz),L"FilterGraph %08x  pid %08x\0", (DWORD_PTR) 0, GetCurrentProcessId());

	HRESULT hr = CreateItemMoniker(L"!", wsz, &pmk);
	if (SUCCEEDED(hr))
	{
		// Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
		// to the object.  Using this flag will cause the object to remain
		// registered until it is explicitly revoked with the Revoke() method.
		//
		// Not using this flag means that if GraphEdit remotely connects
		// to this graph and then GraphEdit exits, this object registration
		// will be deleted, causing future attempts by GraphEdit to fail until
		// this application is restarted or until the graph is registered again.
		hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,
			pmk, &dwROTReg);
		pmk->Release();
	}

	pROT->Release();
	return hr;
}


void RemoveFromROT(void)
{
	IRunningObjectTable *pirot=0;

	if (SUCCEEDED(GetRunningObjectTable(0, &pirot)))
	{
		pirot->Revoke(dwROTReg);
		pirot->Release();
	}
}

#endif
