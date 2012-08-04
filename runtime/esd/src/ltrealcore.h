/****************************************************************************
;
;	MODULE:		LTRealCore (.H)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	10-20-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRealCore_H
#define LTRealCore_H

#define LITHTECH_ESD_INC 1
#include "lith.h"
#include "bdefs.h"
#undef LITHTECH_ESD_INC
#include "iltesd.h"
#include "pnwintyp.h"
#include "pncom.h"
#include "rmapckts.h"
#include "rmacomm.h"
#include "rmamon.h"
#include "rmafiles.h"
#include "rmaengin.h"
#include "rmacore.h"
#include "rmaclsnk.h"
#include "rmaerror.h"
#include "rmaauth.h"
#include "rmaausvc.h"
#include "rmawin.h"

#include "LTRClientContext.h"
#include "LTRAudioDevice.h"
#include "LTRClientEngineSetup.h"

#define LTREALCOREPLUGINFILENAME	"LITH3210.DLL"
#define LTREALCOREENGINEFILENAME	"PNEN3260.DLL"	// Rahul@real.com said this value shouldn't change [mds 1/20/2001]

//-----------------------------------------------------------------------------
// Main class for CLTRealCore
//-----------------------------------------------------------------------------
class CLTRealCore
{
public:
	// Default constructor
	CLTRealCore();

	// Default destructor (calls Term if it has not been called)
	~CLTRealCore();

	// Initialize core
	LTRESULT	Init();

	// Terminate core
	LTRESULT	Term();

	// Verify that Real Player is installed on the users system
	LTRESULT	IsRealPlayerInstalled(bool* bIsInstalled);

	// Verify that Real Player plugin is installed on the users system
	LTRESULT	IsRealPlayerPlugInInstalled(bool* bIsInstalled);

	// Replace Audio Device
	LTRESULT	ReplaceAudioDevice();

	// Create a Real Player object
	LTRESULT	CreatePlayer(IRMAPlayer** ppRMAPlayer);

	// Destroy a Real Player object
	LTRESULT	DestroyPlayer(IRMAPlayer** ppRMAPlayer);

	// Dump a Real error code to the console
	LTRESULT	DumpRealErrorCode(HRESULT hResult);

	// Use this class as a singleton
	static CLTRealCore& Instance()
	{
		static CLTRealCore Instance;
		return Instance;
	}

protected:
	LTBOOL					m_bInitialized;

	LTBOOL					m_bRealPlayerInstalled;
	LTBOOL					m_bRealPlayerPlugInInstalled;

    HINSTANCE				m_hDll;
    HINSTANCE				m_hFileSystemDll;
	
	FPRMCREATEENGINE		m_fpCreateEngine;
	FPRMCLOSEENGINE			m_fpCloseEngine;

    IRMAClientEngine*		m_pRMAEngine;
	CLTClientEngineSetup*	m_pClientEngineSetup;

	CLTAudioDevice*			m_pAudioDevice;
	IRMAAudioDeviceManager*	m_pRMAAudioDeviceManager;
	IRMAAudioPushdown*		m_pRMAAudioPushdown;

	// Real core handles
	HINSTANCE				m_hVSRC3260;
	HINSTANCE				m_hPNXR3260;
	HINSTANCE				m_hSMPL3260;
	HINSTANCE				m_hAUTH3260;
	HINSTANCE				m_hMETA3260;
	HINSTANCE				m_hMP3F3260;
	HINSTANCE				m_hMP3R3260;

	// Real core handles (video specific)
	HINSTANCE				m_hRNCO3260;
	HINSTANCE				m_hRV303260;
	HINSTANCE				m_hDRV33260;
	HINSTANCE				m_hRV203260;
	HINSTANCE				m_hDRV23260;
};

inline static CLTRealCore& CLTRealCore()
{
	return CLTRealCore::Instance();
}

#endif // LTRealCore_H
#endif // LITHTECH_ESD