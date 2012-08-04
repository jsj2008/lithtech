//----------------------------------------------------------------------------------
// MotdCtrl.cpp
//----------------------------------------------------------------------------------
#include <assert.h>
#include "MotdCtrl.h"
#include "WONUpdateCtrl.h"
#include "CustomInfo.h"
#include "WONAPI/WONCommon/StringUtil.h"
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONGUI/TabCtrl.h"
#include "WONGUI/SimpleComponent.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// Static member variables.
//----------------------------------------------------------------------------------
MotdCtrl* MotdCtrl::m_pInstance = NULL;


//----------------------------------------------------------------------------------
// MotdCtrl Constructor.
//----------------------------------------------------------------------------------
MotdCtrl::MotdCtrl(void)
	: WizardCtrl()
{
	m_pInstance = this;
	m_bDownloaded = false;
	AddControls();
}

//----------------------------------------------------------------------------------
// MotdCtrl Destructor.
//----------------------------------------------------------------------------------
MotdCtrl::~MotdCtrl(void)
{
	m_pInstance = NULL;
}

//----------------------------------------------------------------------------------
// MotdComplete: The MotD's have been fetched.  Extract them from the raw data.
//----------------------------------------------------------------------------------
void MotdCtrl::MotdComplete(GetMOTDOp* pGetMotdOp)
{
///	DebugLog(TEXT("Retrieved MotD.\n"));

	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

	ByteBufferPtr pRawGameMsg;
	ByteBufferPtr pRawSysMsg;
	GUIString sMotD;

	// Extract the Raw MotDs.
	if (pGetMotdOp->GameMOTDIsNew())
		pRawGameMsg = pGetMotdOp->GetGameMOTD();

	if (pGetMotdOp->SysMOTDIsNew())
		pRawSysMsg = pGetMotdOp->GetSysMOTD();

	// Start with the system-wide MotD.
	if (pRawSysMsg)
		sMotD = pRawSysMsg.get()->data();

	// Add the game specific MotD.
	if (pRawGameMsg)
	{
		if (sMotD.length())
			sMotD.append("\n\n");
		sMotD.append(pRawGameMsg.get()->data());
	}

	// Display the combined MotD.
	if (sMotD.length())
	{
		m_pInfoText->Clear();
		m_pInfoText->AddFormatedText(sMotD);
	}

	// Update the Next button to say 'next' instead of 'skip'.
	m_pNextButton->SetText(pResMgr->GetString(IDS_MOTD_DLG_NEXT));
	m_pNextButton->Invalidate();
	m_bDownloaded = true;
}

//----------------------------------------------------------------------------------
// MotdCallback: callback to allow us to receive the MotDs.
//----------------------------------------------------------------------------------
void MotdCtrl::MotdCallback(AsyncOp* pOp)
{
///	DebugLog(TEXT("MotD Callback.\n"));

	GetMOTDOp* pGetOp = dynamic_cast<GetMOTDOp*>(pOp);

	if (pGetOp->Killed())
		return;

	MotdComplete(pGetOp);

} //lint !e1746

//----------------------------------------------------------------------------------
// StartMotdDownload: Kick off the MotD request.
//----------------------------------------------------------------------------------
void MotdCtrl::StartMotdDownload(void)
{
///	DebugLog(TEXT("Starting MotD Download...\n"));

	CustomInfo* pCI = GetCustomInfo();

	// Prepare the download.
	m_pGetMotdOp = new GetMOTDOp(pCI->GetProductName());
	m_pGetMotdOp->SetCompletion(new MotDCompletion(MotdCallback));
	if (pCI->GetExtraConfig().length())
		m_pGetMotdOp->SetExtraConfig(pCI->GetExtraConfig());

	// Start the download.
	m_pGetMotdOp->Run(OP_MODE_ASYNC, pCI->GetMotdTimeout());
}

//----------------------------------------------------------------------------------
// StopMotdDownload: Kill the Motd thread if it is pending.
//----------------------------------------------------------------------------------
void MotdCtrl::StopMotdDownload(void)
{
	if (m_pGetMotdOp.get())
		m_pGetMotdOp->Kill();
}

//----------------------------------------------------------------------------------
// Show: Make this dialog visible (or invisible), and start (or stop) any threads 
// that are needed.
//----------------------------------------------------------------------------------
void MotdCtrl::Show(bool bShow)
{
	SetVisible(bShow);

	if (bShow)
	{
		// Do *not* refresh the info string, or it will blow away the downloaded MotD.

		// Make sure the Next button is the default button.
		m_pNextButton->RequestFocus();
	}

	// Start or Stop the MotD downloading if it has not already been fetched.
	if (! m_bDownloaded)
	{
		if (bShow)
			StartMotdDownload();
		else
			StopMotdDownload();
	}
}

//----------------------------------------------------------------------------------
// AddControls: Initialize all of the sub-controls.
//----------------------------------------------------------------------------------
void MotdCtrl::AddControls(void)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();
	DisplayContext* pContext = GetWindow()->GetDisplayContext();

	// Background.
	NativeImagePtr pBackroundImg = pResMgr->GetBackgroundImage(IDB_MOTD_DLG_BACKGROUND);
	ImageComponentPtr pBackground = new ImageComponent(pBackroundImg, true);
	AddChildLayout(pBackground, CLI_SameSize, this);

	// Help Button (Navigation Control).
	m_pHelpButton = new MSButton(pResMgr->GetString(IDS_MOTD_DLG_HELP));
	m_pHelpButton->SetDesiredSize();
	AddChildLayout(m_pHelpButton, CLI_SameBottom | CLI_SameLeft, this, NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Next (AKA Skip) Button (Navigation Control).
	m_pNextButton = new MSButton(pResMgr->GetString(IDS_MOTD_DLG_NEXT));
	m_pNextButton->SetDesiredSize();
	int nWd = m_pNextButton->Width();
	m_pNextButton->SetText(pResMgr->GetString(IDS_MOTD_DLG_SKIP));
	m_pNextButton->SetDesiredSize();
	if (nWd > m_pNextButton->Width())
		m_pNextButton->SetWidth(nWd);
	AddChildLayout(m_pNextButton, CLI_SameBottom | CLI_SameRight, this, -NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Back Button (Navigation Control).
	m_pBackButton = new MSButton(pResMgr->GetString(IDS_MOTD_DLG_BACK));
	m_pBackButton->SetDesiredSize();
	AddChildLayout(m_pBackButton, CLI_SameBottom | CLI_Left, m_pNextButton, -NAV_BTN_SPACING, 0, 0, 0);

	// Separator Bar (Navigation Control).
	MSSeperatorPtr pSeparator = new MSSeperator(SEP_BAR_HEIGHT);
	AddChildLayout(pSeparator, CLI_Above, m_pNextButton, 0, -SEP_BAR_VERT_MARGIN, 0, 0);
	AddChildLayout(pSeparator, CLI_SameLeft | CLI_GrowToRight, this, SEP_BAR_HORIZ_MARGIN, 0, -SEP_BAR_HORIZ_MARGIN, 0);

	// Game Logo (Dialog-Specific Control).
	NativeImagePtr pBrandImg = pResMgr->GetGameLogoImage(IDB_MOTD_DLG_GAME_LOGO);
	ImageComponentPtr pBrandLogo = new ImageComponent(pBrandImg, true);

	LayoutPointPtr pBrandFrame = new LayoutPoint();
	if (pBrandLogo->Width())
		pBrandFrame->SetWidth(pBrandLogo->Width() + 2 * STD_HORIZ_MARGIN);
	else
		pBrandFrame->SetWidth(STD_HORIZ_MARGIN);
	AddChildLayout(pBrandFrame, CLI_SameLeft | CLI_SameTop, this, 0, 0, 0, 0);
	AddChildLayout(pBrandFrame, CLI_GrowToTop, pSeparator, 0, 0, 0, 0);
	AddLayoutPoint(pBrandFrame);
	CenterLayoutPtr pBrandCenterLayout = new CenterLayout(pBrandFrame, pBrandLogo, true, true);
	AddChildLayout(pBrandCenterLayout);

	// Information Text (Dialog-Specific Control).
	m_pInfoText = new TextArea;
	m_pInfoText->SetComponentFlags(ComponentFlag_GrabBG, true);
	m_pInfoText->SetBackground(-1);
	m_pInfoText->SetBorderPadding(TEXT_AREA_LEFT_PAD, TEXT_AREA_TOP_PAD, TEXT_AREA_RIGHT_PAD, TEXT_AREA_BOTTOM_PAD);
	MSScrollContainerPtr pInfoBox = new MSScrollContainer(m_pInfoText);
	pInfoBox->SetScrollbarConditions(ScrollbarCondition_Disable, ScrollbarCondition_Conditional);
	pInfoBox->SetDraw3DFrame(true);
	pInfoBox->SetSize(10, 10); // Needs to be big enough to avoid a GPF.
	AddChildLayout(pInfoBox, CLI_SameTop, this, 0, STD_VERT_MARGIN, 0, 0);
	AddChildLayout(pInfoBox, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(pInfoBox, CLI_GrowToRight, m_pNextButton, 0, 0, 0, 0);
	AddChildLayout(pInfoBox, CLI_GrowToTop, pSeparator, 0, 0, 0, -STD_VERT_MARGIN);

	// Add the controls in the desired tab order.
	AddChild(pBackground);
	AddChild(pBrandLogo);
	AddChild(pInfoBox);
	AddChild(pSeparator);
	AddChild(m_pHelpButton);
	AddChild(m_pBackButton);
	AddChild(m_pNextButton);

	GUIString sInfo = pResMgr->BuildInfoString(IDS_MOTD_DLG_INFO_1);
	m_pInfoText->AddFormatedText(sInfo);
	m_pInfoText->SetVertOffset(0); // Scroll to the top.
}

//----------------------------------------------------------------------------------
// HandleHelpButton: Process a click on the Help button.
//----------------------------------------------------------------------------------
bool MotdCtrl::HandleHelpButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	// Invoke the Help callback for this dialog.
	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowHelp();

	return true;
}

//----------------------------------------------------------------------------------
// HandleBackButton: Process a click on the Back button.
//
// Note:  If the Motd has not finished downloading, it will be shut down when the 
// 'Show' method is invoced to hide this window.
//----------------------------------------------------------------------------------
bool MotdCtrl::HandleBackButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowPreviousScreen();

	return true;
}

//----------------------------------------------------------------------------------
// HandleNextButton: Process a click on the Next button.
//
// Note:  If the Motd has not finished downloading, it will be shut down when the 
// 'Show' method is invoced to hide this window.
//----------------------------------------------------------------------------------
bool MotdCtrl::HandleNextButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowNextScreen();

	return true;
}

//----------------------------------------------------------------------------------
// HandleComponentEvent: Handle Component Events (Button pushes and the like).
//----------------------------------------------------------------------------------
void MotdCtrl::HandleComponentEvent(ComponentEvent* pEvent)
{
	if (pEvent->mComponent == m_pHelpButton && HandleHelpButton(pEvent))
		return;
	if (pEvent->mComponent == m_pBackButton && HandleBackButton(pEvent))
		return;
	if (pEvent->mComponent == m_pNextButton && HandleNextButton(pEvent))
		return;

	Container::HandleComponentEvent(pEvent);
}
