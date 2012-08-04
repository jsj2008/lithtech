//----------------------------------------------------------------------------------
// WONUpdateCtrl.h
//----------------------------------------------------------------------------------
#ifndef __WONUpdateCtrl_H__
#define __WONUpdateCtrl_H__

#include "WelcomeCtrl.h"
#include "ConfigProxyCtrl.h"
#include "MotdCtrl.h"
#include "OptionalPatchCtrl.h"
#include "PatchDetailsCtrl.h"
#include "SelectHostCtrl.h"
#include "DownloadCtrl.h"
#include "WebLaunchCtrl.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// WONUpdateCtrl.
//----------------------------------------------------------------------------------
class WONUpdateCtrl : public Container
{
public:
	typedef enum
	{
		WS_None,
		WS_Welcome,
		WS_ConfigProxy,
		WS_Motd,
		WS_OptionalPatch,
		WS_PatchDetails,
		WS_SelectHost,
		WS_Download,
		WS_WebLaunch,

	} WIZARD_SCREEN;

protected:
	WIZARD_SCREEN        m_CurrentScreen;      // What screen is being displayed?
	WIZARD_SCREEN        m_PreviousScreen;     // What screen is displayed last?
	WelcomeCtrlPtr       m_pWelcomeCtrl;       // Welcome Screen.
	ConfigProxyCtrlPtr   m_pConfigProxyCtrl;   // Configure Proxy Screen.
	MotdCtrlPtr          m_pMotdCtrl;          // MotD Screen.
	OptionalPatchCtrlPtr m_pOptionalPatchCtrl; // Optional Patch Screen.
	PatchDetailsCtrlPtr  m_pPatchDetailsCtrl;  // Patch Details Screen.
	SelectHostCtrlPtr    m_pSelectHostCtrl;    // Select Host Screen.
	DownloadCtrlPtr      m_pDownloadCtrl;      // Download Screen.
	WebLaunchCtrlPtr     m_pWebLaunchCtrl;     // Web Launch Screen.

public:
	virtual void HandleComponentEvent(ComponentEvent* pEvent);
//	virtual bool TimerEvent(int theDelta);
	virtual void SizeChanged(void);
	virtual void AddedToParent(void);

public:
	WONUpdateCtrl(void);
	~WONUpdateCtrl(void);

	void AddControls(void);

	virtual bool KeyDown(int nKey);

	void ShowScreen(WIZARD_SCREEN WS);
//	inline WIZARD_SCREEN GetCurentScreen(void) const   { return m_CurrentScreen; }
//	inline WIZARD_SCREEN GetPreviousScreen(void) const { return m_PreviousScreen; }
	void ShowNextScreen(void);
	void ShowPreviousScreen(void);

	void ShowHelp(void);
};
typedef SmartPtr<WONUpdateCtrl> WONUpdateCtrlPtr;

};

#endif
