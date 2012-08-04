// *********************************************************************** //
//
// MODULE  : dshowVideomgrimpl.h
//
// PURPOSE : direct show video manager 
//
// CREATED : 12/21/04
//
// (c) 2004 Touchdown Entertainment Inc.  All Rights Reserved
//
// *********************************************************************** //

#ifndef __DSHOWVIDEOMGRIMPL_H__
#define __DSHOWVIDEOMGRIMPL_H__

#ifndef __VIDEOMGR_H__
#include "videomgr.h"
#endif

#ifndef __DDRAW_H__
#include <ddraw.h>
#define __DDRAW_H__
#endif

#ifndef __RENDERSTRUCT_H__
#include "renderstruct.h"
#endif

#ifndef __DTXMGR_H__
#include "dtxmgr.h"
#endif

#ifndef __DE_WORLD_H__
#include "de_world.h"
#endif

#ifndef __RENDER_H__
#include "render.h"
#endif

#include "streams.h"


class DShowVideoInst;
class CTextureRenderer;

// -------------------------------------------------------------------------------- //
// BinkVideoMgr.
// -------------------------------------------------------------------------------- //
class DShowVideoMgr : public VideoMgr
{
public:
                
						DShowVideoMgr();
						~DShowVideoMgr();

    LTRESULT			Init();

    virtual LTRESULT	CreateScreenVideo(const char *pFilename, uint32 flags, VideoInst* &pVideo);

public:

	//video flags
    uint32						m_VideoFlags;

};


// -------------------------------------------------------------------------------- //
// DShowVideoInst.
// -------------------------------------------------------------------------------- //
class DShowVideoInst : public VideoInst
{
public:

					    DShowVideoInst(DShowVideoMgr *pMgr);
					    ~DShowVideoInst();
    
    LTRESULT		InitVideo(const char *pFilename, uint32 flags);
    void				Term();

	 void				OnRenderInit() {};
	 void				OnRenderTerm();

    virtual LTRESULT	Update();
    virtual LTRESULT	DrawVideo();
    virtual LTRESULT	GetVideoStatus();
    virtual void		Release();

    LTRESULT		UpdateOnScreen();

private:

	//---- fills in our texture with black
	bool				ClearSurface();

	//---- pointer to the manager that created us
   DShowVideoMgr		*m_pMgr;

	//---- flags for the video
	uint32						m_flags;

	CComPtr<IGraphBuilder>  m_pGB;          // GraphBuilder
	CComPtr<IMediaControl>  m_pMC;          // Media Control
	CComPtr<IMediaPosition> m_pMP;          // Media Position
	CComPtr<IMediaEvent>    m_pME;          // Media Event

	CComPtr<IBaseFilter>    m_pRenderer;			// our custom renderer
	CTextureRenderer *		m_pTextureRenderer;	// Pointer to the renderer

	LTRESULT						m_Status;		// Video status 


};




//-----------------------------------------------------------------------------
// Define GUID for Texture Renderer
// {71771540-2017-11cf-AE26-0020AFD79767}
//-----------------------------------------------------------------------------
struct __declspec(uuid("{71771540-2017-11cf-ae26-0020afd79767}")) CLSID_TextureRenderer;

//-----------------------------------------------------------------------------
// CTextureRenderer Class Declarations
//-----------------------------------------------------------------------------
class CTextureRenderer : public CBaseVideoRenderer
{

public:
	CTextureRenderer(LPUNKNOWN pUnk,HRESULT *phr);
	~CTextureRenderer();

public:
	HRESULT CheckMediaType(const CMediaType *pmt );     // Format acceptable?
	HRESULT SetMediaType(const CMediaType *pmt );       // Video format notification
	HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample


	//the dimensions of the surface that we have allocated
	uint32				m_nTextureWidth;
	uint32				m_nTextureHeight;
	D3DFORMAT         m_TextureFormat; 

   // The on-screen rendering surface
   LPDIRECT3DTEXTURE9  m_pTexture;

	// Using dynamix texture ?
	BOOL m_bUseDynamicTextures;

	LONG m_lVidWidth;   // Video width
	LONG m_lVidHeight;  // Video Height
	LONG m_lVidPitch;   // Video Pitch

	bool	m_StartedSampling; 

};


#endif

