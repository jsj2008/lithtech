// D3D_Device.h
//	Contains the active D3D Device. CD3D_Shell & CD3D_Device are the main containers of D3D shtuff...

#ifndef __D3D_DEVICE_H__
#define __D3D_DEVICE_H__

#include <d3d9.h>
#include "renderobject.h"

// PROTOTYPES
struct D3DAdapterInfo;
struct D3DDeviceInfo;
struct D3DModeInfo;
struct ModelSpecularStateBackup;

struct ExtraDeviceCaps {
	bool		m_bSrcBlendSrcColor;			// Primitive drawing capabilities...
	bool		m_bDestBlendSrcAlpha;
	bool		m_bTBlendAdd;
	bool		m_bTBlendModulate;
	bool		m_bCanDither;

	bool		m_bDrawPrimEnabled;
	bool		m_bTexturesFromVideo;			// Can textures be in video memory?
	bool		m_bTexturesFromSystem;			// Can textures be in system memory?
	bool		m_bTexturesFromAGP;				// Can textures be in AGP memory?
	bool		m_bModelSpecularCapable;
	bool		m_bDetailTextureCapable;

	bool		m_bLightmapCapable;
	bool		m_bGouraudFullbriteCapable;
	bool		m_bSinglePassLM;
	bool		m_bLightAddPolyCapable;
	bool		m_bFullbriteCapable;
	bool		m_bShadowCapable;
	bool		m_bPowerVR;
	bool		m_bDrawPortals;					// This is what is used to determine whether or not to draw portals.

	bool		m_bHasStencilBuffer;			// Do we have a stencil buffer?
	bool		m_bHasZBuffer;					// Do we have a Z Buffer (That's a Zed buffer for you Canadians out there)?
	D3DFORMAT	m_DepthStencilBufferFormat;		// What's the format of this guy (if we've got one) - Note: it's the Z & Stencil buffer...
};

class CD3D_Device {
public:
	CD3D_Device()			{ Reset(); }
	~CD3D_Device()			{ FreeAll(); }

	// Creation/Destruction Routines...
	bool					CreateDevice(D3DAdapterInfo* pAdapter,D3DDeviceInfo* pDevice,D3DModeInfo* pMode);	// Create the Sucka...
	void					TryFallingBack_OnFailedDevCreate(D3DPRESENT_PARAMETERS* PresentationParam);			// If device create fails - try falling back (this function tries to scale back)...
	bool					SetMode(D3DModeInfo* pMode);
	void					FreeDevice();		// Releases the device (and resets all that should be reset on device release)...
	bool					ReleaseDevObjects();// Release all the device objects (the Render Objects)...
	bool					RestoreDevObjects();// Restores the objects (calls ReCreateObject on all the Render Objects)...
	void					Reset();			// Resets back to initial conditions (doesn't try to free anything)...
	void					ResetDeviceVars();	// Reset all the device vars (this is a partial reset - for ALT-Tab type stuff)...
	void					FreeAll();			// Frees all the member vars and resets afterwards...
	static CRenderObject*	CreateRenderObject(CRenderObject::RENDER_OBJECT_TYPES ObjectType);
	static bool				DestroyRenderObject(CRenderObject* pObject);
	D3DAdapterInfo*			GetAdapterInfo()	{ return m_pAdapter; }
	D3DDeviceInfo*			GetDeviceInfo()		{ return m_pDevice; }
	D3DModeInfo*			GetModeInfo()		{ return m_pMode; }

	// State Set/Get Routines...
	void					SetDefaultRenderStates();
	void					SetupViewport(int32 iWidth, int32 iHeight, float fMinZ = 0.0f, float fMaxZ = 1.0f);

	// Render Routines...
	static bool				Start3D();			// Call before you start rendering a frame...
	static bool				End3D();			// Call when you are done rendering a frame...
	static bool				IsIn3D();

	// Cap info (some precomputed caps)...
	D3DCAPS9*				GetDeviceCaps()		{ return &m_DeviceCaps; }
	ExtraDeviceCaps*		GetExtraDevCaps()	{ return &m_ExtraDevCaps; }
	void					PreCalcSomeDeviceCaps();
	bool					CanDrawPortals()	{ return m_ExtraDevCaps.m_bHasStencilBuffer; }	// Returns TRUE if the current device is capable of rendering portals through a stencil buffer.

	// Note: m_pD3DDevice is public for quick access...
    LPDIRECT3DDEVICE9		m_pD3DDevice;		// The D3D rendering device
private:
	D3DFORMAT				GetDefaultDepthStencilFormat(uint32 iZBitDepth,uint32 iStencilBitDepth);
	D3DMULTISAMPLE_TYPE		GetDefaultMultiSampleType(uint32 Samples);
	void					SetPresentationParams(D3DPRESENT_PARAMETERS& PresentationParam,D3DModeInfo* pMode);

	// Device Info/Data...
	D3DCAPS9				m_DeviceCaps;		// Device Caps...
	ExtraDeviceCaps			m_ExtraDevCaps;		// Lith defined extra Caps...
	D3DAdapterInfo*			m_pAdapter;			// My Adaptor Info Pointer (Points into g_D3DShell's device list)
    D3DDeviceInfo*			m_pDevice;			// My Device Info Pointer (Points into g_D3DShell's device list)
    D3DModeInfo*			m_pMode;			// My Mode Info Pointer (Points into g_D3DShell's mode list)
    bool					m_bWindowed;

	CRenderObject*			m_pRenderObjectList_Head;	// List of all the render objects (kept around so that we can notify them of stuff)...
	bool					m_bIn3D;			// Are we in between the Start3D and End3D calls...
};
extern CD3D_Device g_Device;					// The global D3D Device...
#define PD3DDEVICE (g_Device.m_pD3DDevice)		// Use for quick access to the D3DDevice...
static LPDIRECT3DDEVICE9 d3d_GetD3DDevice() { return PD3DDEVICE; }	// For the RenderStruct...

// ModelSpecularStateBackup (For Saving ModelSpecular States)
struct ModelSpecularStateBackup { uint32 m_OldColorOp; uint32 m_OldTAddress; };

#endif