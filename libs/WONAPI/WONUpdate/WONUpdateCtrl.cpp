//----------------------------------------------------------------------------------
// WONUpdateCtrl.cpp
//----------------------------------------------------------------------------------
#include "WONAPI/WONAPI.h"
#include "WONGUI/SimpleComponent.h"
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONUpdateCtrl.h"
#include "MessageDlg.h"
#include "CustomInfo.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// Local Variables.
//----------------------------------------------------------------------------------
WONAPICore* g_pCore = new WONAPICore(true);
static WONUpdateCtrl* g_pUpdateCtrl = NULL;


//----------------------------------------------------------------------------------
// WONUpdateCtrl Constructor.
//----------------------------------------------------------------------------------
WONUpdateCtrl::WONUpdateCtrl(void)
{
	g_pUpdateCtrl = this;
	m_CurrentScreen = WS_None;
	m_PreviousScreen = WS_None;
	AddControls();
}

//----------------------------------------------------------------------------------
// WONUpdateCtrl Destructor.
//----------------------------------------------------------------------------------
WONUpdateCtrl::~WONUpdateCtrl(void)
{
	g_pUpdateCtrl = NULL;
}

//----------------------------------------------------------------------------------
// AddedToParent: ???
//----------------------------------------------------------------------------------
void WONUpdateCtrl::AddedToParent(void)
{
	Container::AddedToParent();
///	RequestTimer(true);
}

/*
//----------------------------------------------------------------------------------
// TimerEvent: ???
//----------------------------------------------------------------------------------
bool WONUpdateCtrl::TimerEvent(int theDelta)
{
	Container::TimerEvent(theDelta);
	gAccum += theDelta;
	if(gAccum < gNextChatEvent)
		return true;

	gAccum = 0;
	gNextChatEvent = rand() % 5000;
	return true;
}
*/

//----------------------------------------------------------------------------------
// HandleComponentEvent: Handle Component Events (Button pushes and the like).
//----------------------------------------------------------------------------------
void WONUpdateCtrl::HandleComponentEvent(ComponentEvent* pEvent)
{
/*
	if((theEvent->mType==ComponentEvent_InputReturn && theEvent->mComponent==mChatInput)
		|| (theEvent->mType==ComponentEvent_ButtonPressed && theEvent->mComponent==mSendButton))
	{
		if(mChatInput->GetText().empty())
			return;

		mChatOutput->AddSegment("Brian: ",0x800000);
		mChatOutput->AddSegment(mChatInput->GetText(),true);
		mChatInput->AddInputHistory(mChatInput->GetText());
		mChatInput->SetText("");
	}
	else
*/
		Container::HandleComponentEvent(pEvent);
}

//----------------------------------------------------------------------------------
// SizeChanged: Handle a Size Changed message.
//----------------------------------------------------------------------------------
void WONUpdateCtrl::SizeChanged(void)
{
	Container::SizeChanged();
}

//----------------------------------------------------------------------------------
// AddControls: Initialize all of the sub-controls.
//----------------------------------------------------------------------------------
void WONUpdateCtrl::AddControls(void)
{
	// Wizard screens (sub controls).
	m_pWelcomeCtrl = new WelcomeCtrl();
	AddChildLayout(m_pWelcomeCtrl, CLI_SameSize, this);
	AddChild(m_pWelcomeCtrl);
	m_pWelcomeCtrl->SetVisible(false);
	int nBackBtnWd = m_pWelcomeCtrl->GetBackButtonWidth();
	int nNextBtnWd = m_pWelcomeCtrl->GetNextButtonWidth();

	m_pConfigProxyCtrl = new ConfigProxyCtrl();
	AddChildLayout(m_pConfigProxyCtrl, CLI_SameSize, this);
	AddChild(m_pConfigProxyCtrl);
	m_pConfigProxyCtrl->SetVisible(false);
	nBackBtnWd = max(nBackBtnWd, m_pConfigProxyCtrl->GetBackButtonWidth());
	nNextBtnWd = max(nNextBtnWd, m_pConfigProxyCtrl->GetBackButtonWidth());

	m_pMotdCtrl = new MotdCtrl();
	AddChildLayout(m_pMotdCtrl, CLI_SameSize, this);
	AddChild(m_pMotdCtrl);
	m_pMotdCtrl->SetVisible(false);
	nBackBtnWd = max(nBackBtnWd, m_pMotdCtrl->GetBackButtonWidth());
	nNextBtnWd = max(nNextBtnWd, m_pMotdCtrl->GetBackButtonWidth());

	m_pOptionalPatchCtrl = new OptionalPatchCtrl();
	AddChildLayout(m_pOptionalPatchCtrl, CLI_SameSize, this);
	AddChild(m_pOptionalPatchCtrl);
	m_pOptionalPatchCtrl->SetVisible(false);
	nBackBtnWd = max(nBackBtnWd, m_pOptionalPatchCtrl->GetBackButtonWidth());
	nNextBtnWd = max(nNextBtnWd, m_pOptionalPatchCtrl->GetBackButtonWidth());

	m_pPatchDetailsCtrl = new PatchDetailsCtrl();
	AddChildLayout(m_pPatchDetailsCtrl, CLI_SameSize, this);
	AddChild(m_pPatchDetailsCtrl);
	m_pPatchDetailsCtrl->SetVisible(false);
	nBackBtnWd = max(nBackBtnWd, m_pPatchDetailsCtrl->GetBackButtonWidth());
	nNextBtnWd = max(nNextBtnWd, m_pPatchDetailsCtrl->GetBackButtonWidth());

	m_pSelectHostCtrl = new SelectHostCtrl();
	AddChildLayout(m_pSelectHostCtrl, CLI_SameSize, this);
	AddChild(m_pSelectHostCtrl);
	m_pSelectHostCtrl->SetVisible(false);
	nBackBtnWd = max(nBackBtnWd, m_pSelectHostCtrl->GetBackButtonWidth());
	nNextBtnWd = max(nNextBtnWd, m_pSelectHostCtrl->GetBackButtonWidth());

	m_pDownloadCtrl = new DownloadCtrl();
	AddChildLayout(m_pDownloadCtrl, CLI_SameSize, this);
	AddChild(m_pDownloadCtrl);
	m_pDownloadCtrl->SetVisible(false);
	nBackBtnWd = max(nBackBtnWd, m_pDownloadCtrl->GetBackButtonWidth());
	nNextBtnWd = max(nNextBtnWd, m_pDownloadCtrl->GetBackButtonWidth());

	m_pWebLaunchCtrl = new WebLaunchCtrl();
	AddChildLayout(m_pWebLaunchCtrl, CLI_SameSize, this);
	AddChild(m_pWebLaunchCtrl);
	m_pWebLaunchCtrl->SetVisible(false);
	// Do not use the button widths in the Web Launch dialog, the 'next' button 
	// is too wide and will mess up the other dialogs.
	//nBackBtnWd = max(nBackBtnWd, m_pDownloadCtrl->GetBackButtonWidth());
	//nNextBtnWd = max(nNextBtnWd, m_pDownloadCtrl->GetBackButtonWidth());

	m_pWelcomeCtrl->SetBackButtonWidth(nBackBtnWd);
	m_pWelcomeCtrl->SetNextButtonWidth(nNextBtnWd);
	m_pConfigProxyCtrl->SetBackButtonWidth(nBackBtnWd);
	m_pConfigProxyCtrl->SetNextButtonWidth(nNextBtnWd);
	m_pMotdCtrl->SetBackButtonWidth(nBackBtnWd);
	m_pMotdCtrl->SetNextButtonWidth(nNextBtnWd);
	m_pOptionalPatchCtrl->SetBackButtonWidth(nBackBtnWd);
	m_pOptionalPatchCtrl->SetNextButtonWidth(nNextBtnWd);
	m_pPatchDetailsCtrl->SetBackButtonWidth(nBackBtnWd);
	m_pPatchDetailsCtrl->SetNextButtonWidth(nBackBtnWd);
	m_pSelectHostCtrl->SetBackButtonWidth(nBackBtnWd);
	m_pSelectHostCtrl->SetNextButtonWidth(nNextBtnWd);
	m_pDownloadCtrl->SetBackButtonWidth(nBackBtnWd);
	m_pDownloadCtrl->SetNextButtonWidth(nNextBtnWd);
	// Do NOT set the standard widths for the Web Launch dialog, the 'next' button 
	// is too wide.
	//m_pWebLaunchCtrl->SetBackButtonWidth(nBackBtnWd);
	//m_pWebLaunchCtrl->SetNextButtonWidth(nNextBtnWd);

	ShowScreen(WS_Welcome);
}

//----------------------------------------------------------------------------------
// ShowScreen: Display the specified dialog.
//----------------------------------------------------------------------------------
void WONUpdateCtrl::ShowScreen(WIZARD_SCREEN NewScreen)
{
	m_pWelcomeCtrl->Show(NewScreen == WS_Welcome);
	m_pConfigProxyCtrl->Show(NewScreen == WS_ConfigProxy);
	m_pMotdCtrl->Show(NewScreen == WS_Motd);
	m_pOptionalPatchCtrl->Show(NewScreen == WS_OptionalPatch);
	m_pPatchDetailsCtrl->Show(NewScreen == WS_PatchDetails);
	m_pSelectHostCtrl->Show(NewScreen == WS_SelectHost);
	m_pDownloadCtrl->Show(NewScreen == WS_Download);
	m_pWebLaunchCtrl->Show(NewScreen == WS_WebLaunch);

	m_PreviousScreen = m_CurrentScreen;
	m_CurrentScreen = NewScreen;

	Invalidate();
}

//----------------------------------------------------------------------------------
// ShowNextScreen: Handle a click on the 'Next' button.  The dialog will enable it 
// when it is a valid option.
//----------------------------------------------------------------------------------
void WONUpdateCtrl::ShowNextScreen(void)
{
	CustomInfo* pCI = GetCustomInfo();

	switch (m_CurrentScreen)
	{
		case WS_Welcome:       ShowScreen(WS_Motd);          break;
		case WS_ConfigProxy:   ShowScreen(m_PreviousScreen); break;
		case WS_Motd:
			if (pCI->GetOptionalUpgrade())
				ShowScreen(WS_OptionalPatch);
			else
				ShowScreen(WS_SelectHost);
			break;
		case WS_OptionalPatch: ShowScreen(WS_SelectHost);    break;
		case WS_PatchDetails:  ShowScreen(WS_SelectHost);    break;
		case WS_SelectHost:
			if (pCI->GetSelectedPatch() && pCI->GetSelectedPatch()->GetMustVisitHost())
				ShowScreen(WS_WebLaunch);
			else
				ShowScreen(WS_Download);
			break;
		case WS_Download:      GetWindow()->Close();         break;
	}
}

//----------------------------------------------------------------------------------
// ShowPreviousScreen: Handle a click on the 'Back' button (the dialog will handle
// any shutdown/cleanup issues).
//----------------------------------------------------------------------------------
void WONUpdateCtrl::ShowPreviousScreen(void)
{
	switch (m_CurrentScreen)
	{
		case WS_Welcome:       GetWindow()->Close();         break;
		case WS_ConfigProxy:   ShowScreen(m_PreviousScreen); break;
		case WS_Motd:          ShowScreen(WS_Welcome);       break;
		case WS_SelectHost:
			if (GetCustomInfo()->GetOptionalUpgrade())
				ShowScreen(WS_OptionalPatch);
			else
				ShowScreen(WS_Motd);
			break;
		case WS_OptionalPatch: ShowScreen(WS_Motd);          break;
		case WS_PatchDetails:  ShowScreen(WS_OptionalPatch); break;
		case WS_Download:      ShowScreen(WS_SelectHost);    break;
		case WS_WebLaunch:     ShowScreen(WS_SelectHost);    break;
	}
}

//----------------------------------------------------------------------------------
// GetBrandImage: Fetch the large (left side) custom image for the window being 
// shown.
//----------------------------------------------------------------------------------
NativeImage* WONUpdateCtrl::GetBrandImage(void)
{
	switch (m_CurrentScreen)
	{
		case WS_Welcome:       break;
		case WS_ConfigProxy:   break;
		case WS_Motd:          break;
		case WS_SelectHost:    break;
		case WS_OptionalPatch: break;
		case WS_PatchDetails:  break;
		case WS_Download:      break;
		case WS_WebLaunch:     break;
	}

	//?????
	return NULL;
}

//----------------------------------------------------------------------------------
// ShowHelp: Invoke the Help callback for the current window.
//----------------------------------------------------------------------------------
void WONUpdateCtrl::ShowHelp(void)
{
	//?????

	char* sMsg = "";
	switch (m_CurrentScreen)
	{
		case WS_Welcome:       sMsg = "Welcome Help Not Ready";        break;
		case WS_ConfigProxy:   sMsg = "Config Proxy Help Not Ready";   break;
		case WS_Motd:          sMsg = "MotD Help Not Ready";           break;
		case WS_SelectHost:    sMsg = "Select Host Help Not Ready";    break;
		case WS_OptionalPatch: sMsg = "Optional Patch Help Not Ready"; break;
		case WS_PatchDetails:  sMsg = "Patch Details Help Not Ready";  break;
		case WS_Download:      sMsg = "Download Help Not Ready";       break;
		case WS_WebLaunch:     sMsg = "Web Launch Help Not Ready";     break;
	}

	MessageBox(GetWindow(), sMsg, "Temp Help", MD_OK);
}

//----------------------------------------------------------------------------------
// KeyDown: Proces a keystroke.
//----------------------------------------------------------------------------------
bool WONUpdateCtrl::KeyDown(int nKey)
{
	// Make the escape key work like the back button.
	if (nKey == KEYCODE_ESCAPE)
	{
		switch (m_CurrentScreen)
		{
			case WS_Welcome:       m_pWelcomeCtrl->FireBackButton();       return true;
			case WS_ConfigProxy:   m_pConfigProxyCtrl->FireBackButton();   return true;
			case WS_Motd:          m_pMotdCtrl->FireBackButton();          return true;
			case WS_OptionalPatch: m_pOptionalPatchCtrl->FireBackButton(); return true;
			case WS_PatchDetails:  m_pPatchDetailsCtrl->FireBackButton();  return true;
			case WS_SelectHost:    m_pSelectHostCtrl->FireBackButton();    return true;
			case WS_Download:      m_pDownloadCtrl->FireBackButton();      return true;
			case WS_WebLaunch:     m_pWebLaunchCtrl->FireBackButton();     return true;
		}
	}

	return Container::KeyDown(nKey);
}
