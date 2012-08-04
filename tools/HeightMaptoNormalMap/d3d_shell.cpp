// D3D_Shell.h
//	The D3D Shell contains the Direct3D Object and related information.
// There's only one of these in the renderer. 

#include "stdafx.h"
#include "tdguard.h"

#include "d3d_shell.h"
#include <algorithm>

// GLOBALS
CD3D_Shell		g_D3DShell;					// The global D3D Shell...
bool			g_bRunWindowed = true;

// EXTERNS
extern uint32	g_ScreenWidth;				// Screen Size...
extern uint32	g_ScreenHeight;

// Create the Sucka...
bool CD3D_Shell::Create()
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		return false;
	}

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
	// which adapter formats are we looking for  
    const D3DFORMAT allowedAdapterFormatArray[] = 
    {   
        D3DFMT_X8R8G8B8, 
        D3DFMT_X1R5G5B5, 
        D3DFMT_R5G6B5 //, 
//        D3DFMT_A2R10G10B10
    };
    const UINT allowedAdapterFormatArrayCount  = sizeof(allowedAdapterFormatArray) / sizeof(allowedAdapterFormatArray[0]);


	if (!m_pD3D) return false;

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
		
//		uint32 iNumAdapterModes = m_pD3D->GetAdapterModeCount(iAdapter);

		D3DDISPLAYMODE DesktopMode;		// Add the current desktop format to list of formats

        m_pD3D->GetAdapterDisplayMode(iAdapter, &DesktopMode);

		formats.push_back(DesktopMode.Format);

/* OLD         for (uint32 iMode = 0; iMode < iNumAdapterModes; iMode++) 
		{
			D3DDISPLAYMODE d3dDisplayMode; // Get the display mode attributes
			m_pD3D->EnumAdapterModes(iAdapter,iMode, &d3dDisplayMode);
			D3DModeInfo DisplayMode; 
			DisplayMode.Width	 = d3dDisplayMode.Width; 
			DisplayMode.Height	 = d3dDisplayMode.Height;
			DisplayMode.Format	 = d3dDisplayMode.Format;

			// Filter out low-resolution modes
			if (DisplayMode.Width < 640 || DisplayMode.Height < 480) continue;

            // Check if the mode already exists (to filter out refresh rates)
            for (uint32 m=0L; m<modes.size(); ++m) 
			{
                if ((modes[m].Width == DisplayMode.Width) && (modes[m].Height == DisplayMode.Height) && (modes[m].Format == DisplayMode.Format)) break; 
			}

            // If we found a new mode, add it to the list of modes
            if (m == modes.size()) 
			{
                // Check if the mode's format already exists (and add it if isn't new)...
                for (uint32 f=0; f<formats.size(); ++f) 
				{
                    if (DisplayMode.Format == formats[f]) break; 
				}

                if (f==formats.size()) formats.push_back(DisplayMode.Format);
				modes.push_back(DisplayMode); 
			}
		}
*/


        for( UINT iFormatList = 0; iFormatList < allowedAdapterFormatArrayCount; iFormatList++ )
        {
            D3DFORMAT allowedAdapterFormat = allowedAdapterFormatArray[iFormatList];

			// Check the modes for this format 
            UINT numAdapterModes = m_pD3D->GetAdapterModeCount( iAdapter, allowedAdapterFormat );

            for (UINT mode = 0; mode < numAdapterModes; mode++)
            {
                D3DDISPLAYMODE d3dDisplayMode;
                m_pD3D->EnumAdapterModes( iAdapter, allowedAdapterFormat, mode, &d3dDisplayMode );

				D3DModeInfo DisplayMode; 
				DisplayMode.Width	 = d3dDisplayMode.Width; 
				DisplayMode.Height	 = d3dDisplayMode.Height;
				DisplayMode.Format	 = d3dDisplayMode.Format;

				// Filter out low-resolution modes
				if ( DisplayMode.Width < 640 || DisplayMode.Height < 480 ) continue;

	            // Check if the mode already exists (to filter out refresh rates)
		        for (uint32 m=0L; m < modes.size(); ++m) 
				{
				    if ( (modes[m].Width == DisplayMode.Width) && 
						 (modes[m].Height == DisplayMode.Height) && 
						 (modes[m].Format == DisplayMode.Format)) 
					{
						 break;
					}
				}

	            // If we found a new mode, add it to the list of modes
		        if ( m == modes.size() ) 
				{
				    // Check if the mode's format already exists (and add it if isn't new)...
	                for (uint32 f=0; f<formats.size(); ++f) 
					{
			            if (DisplayMode.Format == formats[f]) break; 
					}

					if (f==formats.size()) 
					{
						formats.push_back(DisplayMode.Format);
					}

					modes.push_back(DisplayMode); 
				}
			}
		}


/* FROM NEW CODE

                if( displayMode.Width < 640 ||
                    displayMode.Height < 480  )
				{
//                    displayMode.Width > m_nMaxWidth ||
//                    displayMode.Height > m_nMaxHeight || 
//                    displayMode.RefreshRate < m_nRefreshMin ||
//                    displayMode.RefreshRate > m_nRefreshMax )
                {
                    continue;
                }

                pAdapterInfo->displayModeList.Add( displayMode );
                
                if( !adapterFormatList.Contains(displayMode.Format) )
                    adapterFormatList.Add( displayMode.Format );
            }

        }

        D3DDISPLAYMODE displayMode;
        pD3D->GetAdapterDisplayMode( adapterOrdinal, &displayMode );
        if( !adapterFormatList.Contains(displayMode.Format) )
            adapterFormatList.Add( displayMode.Format );
*/
			

        sort(modes.begin(),modes.end()); // Sort the list of display modes...

        // Add devices to adapter (these are the device types we care about)...
		uint32		iNumDeviceTypes = 2L;
		TCHAR*		strDeviceDescs[] = { "HAL", "REF" };
		D3DDEVTYPE	DeviceTypes[]    = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };
		for (UINT iDevice = 0; iDevice < iNumDeviceTypes; ++iDevice) 
		{
			D3DDeviceInfo Device;		// Fill in device info
			Device.DeviceType		 = DeviceTypes[iDevice];
			m_pD3D->GetDeviceCaps(iAdapter, DeviceTypes[iDevice], &Device.d3dCaps);
			Device.strDesc			 = strDeviceDescs[iDevice];
			Device.iCurrentMode		 = 0;
//			Device.bCanDoWindowed	 = FALSE;
//			Device.bWindowed		 = FALSE;
			Device.bStereo			 = FALSE;
			Device.MultiSampleType	 = D3DMULTISAMPLE_NONE;

			// Go through the formats, check if we can render and do HW T&L...
			vector<bool> bConfirmedFormats;
			vector<bool> bCanDoHWTnL;
			vector<bool> bCanDoWindowed;

			for (DWORD f=0; f<formats.size(); ++f) 
			{
				bool bConfirmed = false; 
				bool bHWTnL = false;
				bool bWindowed = false;

				// Is it compat with rendering...
				if (FAILED(m_pD3D->CheckDeviceType(iAdapter,Device.DeviceType,formats[f],formats[f],FALSE))) 
				{
					bConfirmed = false; 
				}
				else 
				{
					bConfirmed = true;
					
					// Check if this mode is windowed 
					if ( FAILED(m_pD3D->CheckDeviceType(iAdapter,Device.DeviceType,formats[f],formats[f],TRUE)) )
					{
						bWindowed = false;
					}
					else
					{
						bWindowed = true;
					}

					// Confirm the device for HW vertex processing
					if (Device.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) 
					{ 
						bHWTnL = true; 
					}
					else 
					{ 
						bHWTnL = false; 
					} 
				}

				bConfirmedFormats.push_back(bConfirmed);
				bCanDoHWTnL.push_back(bHWTnL); 
				bCanDoWindowed.push_back(bWindowed);
			}

			// Add all enumerated display modes with confirmed formats to the device's list of valid modes
			for (uint32 m=0; m < modes.size(); ++m) 
			{
				for (uint32 f=0; f < formats.size(); ++f) 
				{
					if (modes[m].Format == formats[f]) 
					{
						if ( bConfirmedFormats[f] ) 
						{
							// Add this mode to the device's list of valid modes
							modes[m].bHWTnL = bCanDoHWTnL[m];
							modes[m].bWindowed = bCanDoWindowed[m];
							Device.Modes.push_back(modes[m]); 
						} 
					} 
				} 
			}

			// Select any 640x480 mode for default (but prefer a 16-bit mode)
			for (m=0; m<Device.Modes.size(); ++m) 
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
			// (which was added initially as formats[0])
//			if (bConfirmedFormats[0] && (Device.d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED)) 
//			{
//				Device.bCanDoWindowed = TRUE;
//				Device.bWindowed      = TRUE; 
//			}

			// If valid modes were found, keep this device...
			AdapterInfo.Devices.push_back(Device); 
		}

		// If valid devices were found, keep this adapter (add it to our list)...
		m_AdapterList.push_back(AdapterInfo); }

	// Return an error if no compatible devices were found
	if (!m_AdapterList.size())
	{
		return false;
	}

	return true;
}

// Called by the init function to pick and create the device (g_Device)...
//	Pick Device Named szDevName if there's one of this name, otherwise, will pick what it thinks is best...
D3DDeviceInfo* CD3D_Shell::PickDefaultDev(D3DAdapterInfo** pAdapterInfo, bool bUseRefRast)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter) 
	{
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice) 
		{
			if (bUseRefRast    && itDevice->DeviceType != D3DDEVTYPE_REF) continue;
//			if (g_CV_SWRast    && itDevice->DeviceType != D3DDEVTYPE_SW)  continue;
//			if (g_bRunWindowed && !itDevice->bWindowed) continue;

			*pAdapterInfo = &(*itAdapter);

            return &(*itDevice); 
		} 
	} 
	return NULL;
}

D3DModeInfo* CD3D_Shell::PickDefaultMode(D3DDeviceInfo* pDeviceInfo,uint32 iBitDepth)
{
	for (vector<D3DModeInfo>::iterator itMode = pDeviceInfo->Modes.begin(); itMode != pDeviceInfo->Modes.end(); ++itMode) 
	{
		// Can this format run in a window
		if ( g_bRunWindowed && ( !itMode->bWindowed || itMode->Format != m_DesktopFormat.Format ))
		{
				continue; 
		}

		if ( !g_bRunWindowed ) 
		{
			if (itMode->Width  != g_ScreenWidth)  continue;
			if (itMode->Height != g_ScreenHeight) continue;

			switch (iBitDepth) 
			{
			case 32 : if (itMode->Format != D3DFMT_X8R8G8B8 && itMode->Format != D3DFMT_R8G8B8)   continue; break;
			case 24 : if (itMode->Format != D3DFMT_X8R8G8B8 && itMode->Format != D3DFMT_R8G8B8)   continue; break;
			case 16 : if (itMode->Format != D3DFMT_R5G6B5   && itMode->Format != D3DFMT_X1R5G5B5) continue; break; 
			} 
		}

		return &(*itMode); 
	}

	return NULL;
}

D3DDeviceInfo* CD3D_Shell::FindDevice(const char* strDesc,D3DAdapterInfo** pAdapterInfo)
{
	for (vector<D3DAdapterInfo>::iterator itAdapter = m_AdapterList.begin(); itAdapter != m_AdapterList.end(); ++itAdapter) {
		for (vector<D3DDeviceInfo>::iterator itDevice = itAdapter->Devices.begin(); itDevice != itAdapter->Devices.end(); ++itDevice) {
			if (strcmp(strDesc,itDevice->strDesc)==0) {
				*pAdapterInfo = &(*itAdapter); return &(*itDevice); } } }
	return NULL;
}

