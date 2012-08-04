// D3D_Device.h
//	Contains the active D3D Device. CD3D_Shell & CD3D_Device are the main containers of D3D shtuff...

#ifndef __D3D_DEVICE_H__
#define __D3D_DEVICE_H__

#ifndef __D3D9_H__
#include <d3d9.h>
#define __D3D9_H__
#endif

#ifndef __RENDEROBJECT_H__
#include "renderobject.h"
#endif

#ifndef __D3D_RENDERSTYLE_H__
#include "d3d_renderstyle.h"
#endif

#include "d3d_device_wrapper.h"
#include "lthread.h"


// PROTOTYPES
struct D3DAdapterInfo;
struct D3DDeviceInfo;
struct D3DModeInfo;
class CD3D_RenderWorld;

class CDirect3DDevice9Wrapper;

struct ExtraDeviceCaps 
{
	bool		m_bLightAddPolyCapable;
	bool		m_bUsingTableFog;				// Using Table or Vertex fog...

	bool		m_bHasStencilBuffer;			// Do we have a stencil buffer?
	D3DFORMAT	m_DepthStencilBufferFormat;		// What's the format of this guy (if we've got one) - Note: it's the Z & Stencil buffer...
};

class CD3D_Device 
{
public:
	CD3D_Device():
	 m_pCurrentFrame(NULL),
	 m_pPreviousFrame(NULL),
	 m_pRenderTarget(NULL),
	 m_pBackBuffer(NULL),
	 m_pDefaultRenderTarget(NULL),
	 m_pDefaultDepthStencilBuffer(NULL)

	{ 
		Reset(); 
	}

	~CD3D_Device()	
	{ 
		FreeAll(); 
	}

	// Creation/Destruction Routines...
	bool					CreateDevice(D3DAdapterInfo* pAdapter,D3DDeviceInfo* pDevice,D3DModeInfo* pMode);	// Create the Sucka...
	void					TryFallingBack_OnFailedDevCreate(D3DPRESENT_PARAMETERS* PresentationParam);			// If device create fails - try falling back (this function tries to scale back)...
	bool					SetMode(D3DModeInfo* pMode);
	void					FreeDevice();		// Releases the device (and resets all that should be reset on device release)...
	bool					ReleaseDevObjects(bool bFullRelease);// Release all the device objects (the Render Objects)...
	bool					RestoreDevObjects();// Restores the objects (calls ReCreateObject on all the Render Objects)...
	void					Reset();			// Resets back to initial conditions (doesn't try to free anything)...
	void					ResetDeviceVars();	// Reset all the device vars (this is a partial reset - for ALT-Tab type stuff)...
	void					FreeAll();			// Frees all the member vars and resets afterwards...
	void					FreeFrameTextures(); // Frees the previous frame and current frame texture buffers.
	static CRenderObject*	CreateRenderObject(CRenderObject::RENDER_OBJECT_TYPES ObjectType);
	static bool				DestroyRenderObject(CRenderObject* pObject);
	CD3DRenderStyle*		CreateRenderStyle();
	void					DestroyRenderStyle(CRenderStyle* pRenderStyle);
	bool					Standby();

	// State Set/Get Routines...
	D3DAdapterInfo*			GetAdapterInfo()	{ return m_pAdapter; }
	D3DDeviceInfo*			GetDeviceInfo()		{ return m_pDevice; }
	D3DModeInfo*			GetModeInfo()		{ return m_pMode; }
	void					SetDefaultRenderStates();
	void					SetupViewport(uint32 iLeft, uint32 iRight, uint32 iTop, uint32 iBottom, float fMinZ = 0.0f, float fMaxZ = 1.0f);

	// Render Routines...
	static bool				Start3D();			// Call before you start rendering a frame...
	static bool				End3D();			// Call when you are done rendering a frame...
	static bool				IsIn3D();

	void					PreventFrameBuffering();

	// Cap info (some precomputed caps)...
	D3DCAPS9*				GetDeviceCaps()		{ return &m_DeviceCaps; }
	ExtraDeviceCaps*		GetExtraDevCaps()	{ return &m_ExtraDevCaps; }
	void					CheckSpecialCards();// Check for some specific cards and disable/enable stuff...
	void					PreCalcSomeDeviceCaps();
	bool					CanDrawPortals()	{ return m_ExtraDevCaps.m_bHasStencilBuffer; }	// Returns TRUE if the current device is capable of rendering portals through a stencil buffer.

	// Debug/Helper Functions...
	void					ListDeviceCaps();	// ConsolePrint the device caps...

	// Load the world data
	bool					LoadWorldData(ILTStream *pStream);

	// Access to the loading/rendering critical section
	LCriticalSection&		GetLoadRenderCS() const { return m_hLoadRenderCS; }

	D3DFORMAT				GetDefaultDepthStencilFormat(uint32 iZBitDepth,uint32 iStencilBitDepth);

	// Note: m_pD3DDevice is public for quick access...
    CDirect3DDevice9Wrapper *m_pD3DDevice;		// The D3D rendering device

	// Pointer to the active rendering world
	CD3D_RenderWorld		*m_pRenderWorld;

	IDirect3DTexture9*		m_pCurrentFrame;
	IDirect3DTexture9*		m_pPreviousFrame;
	IDirect3DTexture9*		m_pRenderTarget;
	IDirect3DSurface9*		m_pBackBuffer;

	IDirect3DSurface9*		m_pDefaultRenderTarget;
	IDirect3DSurface9*		m_pDefaultDepthStencilBuffer;
	
private:
	D3DMULTISAMPLE_TYPE		GetDefaultMultiSampleType(uint32 Samples);
	void					SetPresentationParams(D3DPRESENT_PARAMETERS& PresentationParam,D3DModeInfo* pMode);

	// Device Info/Data...
	D3DCAPS9				m_DeviceCaps;		// Device Caps...
	ExtraDeviceCaps			m_ExtraDevCaps;		// Lith defined extra Caps...
	D3DAdapterInfo*			m_pAdapter;			// My Adaptor Info Pointer (Points into g_D3DShell's device list)
    D3DDeviceInfo*			m_pDevice;			// My Device Info Pointer (Points into g_D3DShell's device list)
    D3DModeInfo*			m_pMode;			// My Mode Info Pointer (Points into g_D3DShell's mode list)
    bool					m_bWindowed;
	RECT					m_rcViewport;		// Cache the viewport...

	CRenderObject*			m_pRenderObjectList_Head;	// List of all the render objects (kept around so that we can notify them of stuff)...
	CD3DRenderStyle*		m_pRenderStyleList_Head;	// List of all the render styles (kept around so that we can notify them of stuff)...
	bool					m_bIn3D;			// Are we in between the Start3D and End3D calls...

	// The critical section restricting access to the rendering device while loading
	mutable LCriticalSection m_hLoadRenderCS;

	IDirect3DQuery9*		m_pEndOfFrameQuery;
};

extern CD3D_Device g_Device;					// The global D3D Device...
#define PD3DDEVICE (g_Device.m_pD3DDevice)		// Use for quick access to the D3DDevice...

static LPDIRECT3DDEVICE9 d3d_GetD3DDevice() { return PD3DDEVICE ? PD3DDEVICE->GetDevice() : 0; }	// For the RenderStruct...

inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }

#endif
