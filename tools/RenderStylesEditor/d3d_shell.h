// D3D_Shell.h
//	The D3D Shell contains the Direct3D Object and related information.
// There's only one of these in the renderer. 

#ifndef __D3D_SHELL_H__
#define __D3D_SHELL_H__

#include <vector>

using namespace std;

// Forward declarations...
struct D3DDeviceInfo;
struct D3DModeInfo;

// Structure for holding information about an adapter, including a list
//	of devices available on this adapter (Unless you've got two cards, 
//	there's just one of these)...
struct D3DAdapterInfo
{
	uint32					iAdapterNum;	// This is D3D's adapter count number (e.g. 2nd adapter)
    D3DADAPTER_IDENTIFIER9	AdapterID;		// Adapter data
    uint32					iCurrentDevice;	// Device data
    uint32					iNumDevices;
    vector<D3DDeviceInfo>	Devices;
};

// Structure for holding information about a Direct3D device, including
//	a list of modes compatible with this device
struct D3DDeviceInfo
{
    // Device data
    D3DDEVTYPE				DeviceType;		// Reference, HAL, etc.
    D3DCAPS9				d3dCaps;		// Capabilities of this device
    TCHAR*					strDesc;		// Name of this device

	
	BOOL					bStereo;
    D3DMULTISAMPLE_TYPE		MultiSampleType;
    uint32					iCurrentMode;
    vector<D3DModeInfo>		Modes;
};

// Structure for holding information about a display mode
struct D3DModeInfo
{
    uint32					Width;			// Screen width in this mode
	uint32					Height;			// Screen height in this mode
    D3DFORMAT				Format;			// Pixel format in this mode
    bool					bHWTnL;			// Whether to use Hardware or Software vertex processing
	bool					bWindowed;      // Whether this mode can do windowed

	bool operator < (const D3DModeInfo& other) const { 
		if (Format < other.Format) return true; else if (Format > other.Format) return false;
		if (Width  < other.Width) return true;  else if (Width > other.Width) return false;
		if (Height < other.Height) return true; else if (Height > other.Height) return false;
		return false; }
};

class CD3D_Shell {
public:
	CD3D_Shell()			{ Reset(); }
	~CD3D_Shell()			{ FreeAll(); }

	bool					Create();		// Create the Sucka...
	void					Reset();		// Resets back to initial conditions (doesn't try to free anything)...
	void					FreeAll();		// Frees all the member vars and resets afterwards...

    bool					BuildDeviceList();
	D3DDeviceInfo*			PickDefaultDev(D3DAdapterInfo** pAdapterInfo, bool bUseRefRast);	// Pick default device from our list...
	D3DModeInfo*			PickDefaultMode(D3DDeviceInfo* pDeviceInfo,uint32 iBitDepth);		// Pick default mode from the passed devices mode list...

	D3DAdapterInfo*			GetAdapterInfo(int iAdapterID)							{ if (iAdapterID < (int)m_AdapterList.size()) return &m_AdapterList[iAdapterID]; return NULL; }
	D3DDeviceInfo*			GetDeviceInfo(int iAdapterID,int iDeviceID)				{ D3DAdapterInfo* pAdapter = GetAdapterInfo(iAdapterID); if (!pAdapter) return NULL; if (iDeviceID < (int)pAdapter->Devices.size()) return &pAdapter->Devices[iDeviceID]; return NULL; }
	D3DModeInfo*			GetModeInfo(int iAdapterID,int iDeviceID,int iModeID)	{ D3DDeviceInfo* pDevice = GetDeviceInfo(iAdapterID,iDeviceID); if (!pDevice) return NULL; if (iModeID < (int)pDevice->Modes.size()) return &pDevice->Modes[iModeID]; return NULL; }

	D3DDeviceInfo*			FindDevice(const char* strDesc,D3DAdapterInfo** pAdapterInfo);

	// Note: m_pD3D is public for quick access...
    LPDIRECT3D9				m_pD3D;			// The main D3D object

private:
	D3DDISPLAYMODE			m_DesktopFormat;// Desktop display mode (mainly for window mode)...
	vector<D3DAdapterInfo>	m_AdapterList;	// List of adaptors on this system (probably just one, unless you've got two cards)...
};

extern CD3D_Shell g_D3DShell;				// The global D3D Shell...
#define PDIRECT3D (g_D3DShell.m_pD3D)		// Use for quick access to the D3DDevice...

#endif