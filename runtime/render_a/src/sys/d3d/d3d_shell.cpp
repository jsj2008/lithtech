// D3D_Shell.h
//	The D3D Shell contains the Direct3D Object and related information.
// There's only one of these in the renderer. 

#include "precompile.h"

#include "d3d_shell.h"
#include "common_stuff.h"
#include "renderstruct.h"
#include "rendererconsolevars.h"
#include <algorithm>

CD3D_Shell g_D3DShell;						// The global D3D Shell...

D3DFORMAT gKnownFormats[] = {
		D3DFMT_X8R8G8B8, 
		D3DFMT_A8R8G8B8, 
		D3DFMT_A2B10G10R10, 
		D3DFMT_X1R5G5B5, 
		D3DFMT_A1R5G5B5,
		D3DFMT_R5G6B5
};

#define NUM_KNOWN_FORMATS (sizeof(gKnownFormats) / sizeof(gKnownFormats[0]))


// Create the Sucka...
bool CD3D_Shell::Create()
{
	FreeAll();								// Make sure everything is all clean before we start...

	// Create the D3D Object (it lets us Query/Create devices)...
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_pD3D) return false;

	// Get the Desktop display mode...
	if (FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_DesktopFormat))) { FreeAll(); return false; }

	// Build the device lists (Adapters, Devices, Modes)...
	if (!BuildDeviceList())					{ FreeAll(); return false; }
	return true;
}

// Resets back to initial conditions (doesn't try to free anything)...
void CD3D_Shell::Reset()
{
	m_pD3D = NULL;

	m_AdapterList.clear();
}

// Frees all the member vars and resets afterwards...
void CD3D_Shell::FreeAll()
{
	if (m_pD3D) {
		uint32 iRefCnt = m_pD3D->Release(); } // assert(iRefCnt==0);

	Reset();
}

// Build the Device List - Here's the basic steps:
//	1. Loop through all the adapters on the system (usually just one),
//	2. Enumerate all display modes and formats on this adapter.
//	3. Add all the devices to adapter.
//	4. Select a default one...
bool CD3D_Shell::BuildDeviceList()
{
	if (!m_pD3D) 
		return false;

	m_AdapterList.clear();				// Clear the Adapter List First...

    // Loop through all the adapters on the system (usually, there's just one
    //	unless more than one graphics card is present).
    for (UINT iAdapter = 0; iAdapter < m_pD3D->GetAdapterCount(); ++iAdapter) 
	{
        D3DAdapterInfo AdapterInfo;		// Fill in adapter info
		AdapterInfo.iAdapterNum		= iAdapter;
        m_pD3D->GetAdapterIdentifier(iAdapter,0,&AdapterInfo.AdapterID);
        AdapterInfo.iNumDevices		= 0;
        AdapterInfo.iCurrentDevice	= 0;

		// Enumerate all display modes and formats on this adapter...
        vector<D3DModeInfo>		modes;	
        vector<D3DFORMAT>		formats;

		D3DDISPLAYMODE DesktopMode;		// Add the current desktop format to list of formats
        m_pD3D->GetAdapterDisplayMode(iAdapter, &DesktopMode);
		formats.push_back(DesktopMode.Format);

		// DX9 requires you to pass in modes.
		for (uint32 iKnownModes = 0; iKnownModes< NUM_KNOWN_FORMATS; iKnownModes++)
		{
			uint32 iNumAdapterModes = m_pD3D->GetAdapterModeCount(iAdapter, gKnownFormats[iKnownModes]);

        for (uint32 iMode = 0; iMode < iNumAdapterModes; iMode++) 
		{
			D3DDISPLAYMODE d3dDisplayMode; // Get the display mode attributes
				m_pD3D->EnumAdapterModes(iAdapter, gKnownFormats[iKnownModes], iMode, &d3dDisplayMode);
			D3DModeInfo DisplayMode; 
			DisplayMode.Width	 = d3dDisplayMode.Width; 
			DisplayMode.Height	 = d3dDisplayMode.Height;
			DisplayMode.Format	 = d3dDisplayMode.Format;

			// Filter out low-resolution modes
			if (DisplayMode.Width < 640 || DisplayMode.Height < 480) 
				continue;

            // Check if the mode already exists (to filter out refresh rates)
			uint32 m;
            for (m=0L; m<modes.size(); ++m) 
			{
                if ((modes[m].Width == DisplayMode.Width) && (modes[m].Height == DisplayMode.Height) && (modes[m].Format == DisplayMode.Format)) 
					break; 
			}

            // If we found a new mode, add it to the list of modes
            if (m == modes.size()) 
			{
				uint32 f;
                // Check if the mode's format already exists (and add it if isn't new)...
                for (f=0; f<formats.size(); ++f) 
				{
                    if (DisplayMode.Format == formats[f]) 
						break; 
				}

                if (f==formats.size()) 
				{
					LT_MEM_TRACK_ALLOC(formats.push_back(DisplayMode.Format), LT_MEM_TYPE_RENDERER);
				}

				LT_MEM_TRACK_ALLOC(modes.push_back(DisplayMode), LT_MEM_TYPE_RENDERER); 
			} 
		}
		}

        sort(modes.begin(),modes.end()); // Sort the list of display modes...

        // Add devices to adapter (these are the device types we care about)...
		TCHAR*		strDeviceDescs[] = { "HAL", "REF" };
		D3DDEVTYPE	DeviceTypes[]    = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };
		uint32		iNumDeviceTypes	 = sizeof(DeviceTypes) / sizeof(DeviceTypes[0]);

        for (UINT iDevice = 0; iDevice < iNumDeviceTypes; ++iDevice) 
		{
            D3DDeviceInfo Device;		// Fill in device info
            Device.DeviceType		 = DeviceTypes[iDevice];
            m_pD3D->GetDeviceCaps(iAdapter, DeviceTypes[iDevice], &Device.d3dCaps);
            Device.strDesc			 = strDeviceDescs[iDevice];
            Device.iCurrentMode		 = 0;
            Device.bCanDoWindowed	 = FALSE;
            Device.bWindowed		 = FALSE;
            Device.bStereo			 = FALSE;
            Device.MultiSampleType	 = D3DMULTISAMPLE_NONE;

            // Go through the formats, check if we can render and do HW T&L...
			vector<bool> bConfirmedFormats;
			vector<bool> bCanDoHWTnL;

            for (DWORD f=0; f<formats.size(); ++f) 
			{
				bool bConfirmed = false; 
				bool bHWTnL = false;

                // Is it compat with rendering...
                if (FAILED(m_pD3D->CheckDeviceType(iAdapter,Device.DeviceType,formats[f],formats[f],FALSE))) 
				{
					bConfirmed = false; 
				}
				else 
				{
					bConfirmed = true;
					// Confirm the device for HW vertex processing or that the Intel 865
					// which is fast enough for Tron.
					if (Device.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) 
					{
						bHWTnL = true;
					}
					else if ( AdapterInfo.AdapterID.VendorId == 0x8086 ) // Intel product
					{
						if( ( AdapterInfo.AdapterID.DeviceId != 0x2562 ) &&
							( AdapterInfo.AdapterID.DeviceId != 0x1132 ) &&
							( AdapterInfo.AdapterID.DeviceId != 0x7121 ) &&
							( AdapterInfo.AdapterID.DeviceId != 0x7123 ) &&
							( AdapterInfo.AdapterID.DeviceId != 0x7125 ) &&
							( AdapterInfo.AdapterID.DeviceId != 0x7127 ) &&
							( AdapterInfo.AdapterID.DeviceId != 0x3577 ) &&
							( AdapterInfo.AdapterID.DeviceId != 0x3582 ) )  // Not Previous Intel Products
						{
							bHWTnL = true;  // Support Intel 865 and above products
						}
					else 
						{
						bHWTnL = false; 
				}
					}
					else
					{
						bHWTnL = false; 
					}
				}

				LT_MEM_TRACK_ALLOC(bConfirmedFormats.push_back(bConfirmed), LT_MEM_TYPE_RENDERER);
				LT_MEM_TRACK_ALLOC(bCanDoHWTnL.push_back(bHWTnL), LT_MEM_TYPE_RENDERER); 
			}

            // Add all enumerated display modes with confirmed formats to the device's list of valid modes
            for (uint32 m=0; m<modes.size(); ++m) 
			{
                for (uint32 f=0; f<formats.size(); ++f) 
				{
                    if ((modes[m].Format == formats[f]) && bConfirmedFormats[f])
					{
						modes[m].bHWTnL = bCanDoHWTnL[f];
                        LT_MEM_TRACK_ALLOC(Device.Modes.push_back(modes[m]), LT_MEM_TYPE_RENDERER); 
					} 
				} 
			}

            // Select any 640x480 mode for default (but prefer a 16-bit mode)
            for (uint32 m=0; m<Device.Modes.size(); ++m) 
			{
                if (Device.Modes[m].Width==640 && Device.Modes[m].Height==480) 
				{
                    Device.iCurrentMode = m;
                    if (Device.Modes[m].Format == D3DFMT_R5G6B5 || Device.Modes[m].Format == D3DFMT_X1R5G5B5 || Device.Modes[m].Format == D3DFMT_A1R5G5B5) 
					{
                        break; 
					} 
				} 
			}

            // Check if the device is compatible with the desktop display mode
            // (which was added initially as formats[0]), second test is DX9 way
			// of testing for windowed mode?
            if (bConfirmedFormats[0] && (m_pD3D->CheckDeviceType(iAdapter, Device.DeviceType, 
				DesktopMode.Format, DesktopMode.Format, TRUE) == D3D_OK))
			{
                Device.bCanDoWindowed = TRUE;
                Device.bWindowed      = TRUE; 
			}

            // If valid modes were found, keep this device...
			LT_MEM_TRACK_ALLOC(AdapterInfo.Devices.push_back(Device), LT_MEM_TYPE_RENDERER); 
		}
    
        // If valid devices were found, keep this adapter (add it to our list)...
        LT_MEM_TRACK_ALLOC(m_AdapterList.push_back(AdapterInfo), LT_MEM_TYPE_RENDERER); 
	}

    // Return an error if no compatible devices were found
    if (!m_AdapterList.size()) 
		return false;

    return true;
}

// Called by the init function to pick and create the device (g_Device)...
//	Pick Device Named szDevName if there's one of this name, otherwise, will pick what it thinks is best...
D3DDeviceInfo* CD3D_Shell::PickDefaultDev(D3DAdapterInfo** pAdapterInfo)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter) 
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice) 
		{
			//if we want the reference rasterizer, only look for those device types
			if (g_CV_RefRast && (itDevice->DeviceType != D3DDEVTYPE_REF)) 
				continue;

			//if we aren't running reference rasterizer though, don't ever fall back to one non-intentionally
			if(!g_CV_RefRast && (itDevice->DeviceType == D3DDEVTYPE_REF))
				continue;

			//if we are running windowed, only use devices that support it
			if (g_bRunWindowed && !itDevice->bWindowed) 
				continue;

			*pAdapterInfo = &(*itAdapter); 
            return &(*itDevice); 
		} 
	}
	
	return NULL;
}

D3DModeInfo* CD3D_Shell::PickDefaultMode(D3DDeviceInfo* pDeviceInfo, uint32 nWidth, uint32 nHeight, uint32 iBitDepth, bool bHWTnL)
{
	for (vector<D3DModeInfo>::iterator itMode = pDeviceInfo->Modes.begin(); itMode != pDeviceInfo->Modes.end(); ++itMode) 
	{
		//make sure the dimensions match
		if ((itMode->Width != nWidth) || (itMode->Height != nHeight))
			continue;

		//if we are in windowed mode, make sure this format matches the desktop
		if (g_bRunWindowed && (itMode->Format != m_DesktopFormat.Format))
			continue; 

		//if they want a HW TnL mode, and we aren't in ref rast mode, make sure it is HW
		if((!g_CV_RefRast && bHWTnL) && !itMode->bHWTnL)
			continue;

		//if we aren't in windowed mode, make sure that the bit depth matches
		if (!g_bRunWindowed) 
		{
			switch (iBitDepth) 
			{
			case 32 : if (itMode->Format != D3DFMT_X8R8G8B8 && itMode->Format != D3DFMT_R8G8B8)   continue; break;
			case 24 : if (itMode->Format != D3DFMT_X8R8G8B8 && itMode->Format != D3DFMT_R8G8B8)   continue; break;
			case 16 : if (itMode->Format != D3DFMT_R5G6B5   && itMode->Format != D3DFMT_X1R5G5B5) continue; break; 
			} 
		}

		//we found a matching device
		return &(*itMode); 
	}

	//no matching device
	return NULL;
}

// Go through all the Adapters/Devices and print them out...
void CD3D_Shell::ListDevices()
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter) 
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice) 
		{
			g_pStruct->ConsolePrint("Device: %s", itDevice->strDesc); 
		} 
	}
}

// List out the modes for the game...
void CD3D_Shell::GetSupportedModes(RMode*& pGlobalModeList)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter) 
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice) 
		{
			for (vector<D3DModeInfo>::iterator itMode = itDevice->Modes.begin(); itMode != itDevice->Modes.end(); ++itMode) 
			{
				RMode* pMode = NULL;
				LT_MEM_TRACK_ALLOC(pMode = new RMode, LT_MEM_TYPE_RENDERER);

				pMode->m_Width	= itMode->Width;
				pMode->m_Height	= itMode->Height;
				pMode->m_bHWTnL	= itMode->bHWTnL;

				LTStrCpy(pMode->m_InternalName, itAdapter->AdapterID.Driver,      sizeof(pMode->m_InternalName));
				LTStrCpy(pMode->m_Description,  itAdapter->AdapterID.Description, sizeof(pMode->m_Description));

				uint32 iBitDepth   = 0;
				switch (itMode->Format) 
				{
				case D3DFMT_R8G8B8	 : iBitDepth = 24; break;
				case D3DFMT_X8R8G8B8 : 
				case D3DFMT_A8R8G8B8 : iBitDepth = 32; break;
				case D3DFMT_R5G6B5   : iBitDepth = 16; break; 
				default				 : assert(0); 
				}

				pMode->m_BitDepth  = iBitDepth;

				pMode->m_pNext	   = pGlobalModeList;
				pGlobalModeList    = pMode; 
			} 
		} 
	}
}

D3DDeviceInfo* CD3D_Shell::FindDevice(const char* strDesc,D3DAdapterInfo** pAdapterInfo)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter) {
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice) {
			if (strcmp(strDesc,itDevice->strDesc)==0) {
				*pAdapterInfo = &(*itAdapter); return &(*itDevice); } } }
	return NULL;
}

