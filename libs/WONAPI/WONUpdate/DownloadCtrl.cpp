//----------------------------------------------------------------------------------
// DownloadCtrl.cpp
//----------------------------------------------------------------------------------
#include <assert.h>
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONGUI/TabCtrl.h"
#include "WONGUI/SimpleComponent.h"
#include "DownloadCtrl.h"
#include "WONUpdateCtrl.h"
#include "AbortDlg.h"
#include "CustomInfo.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// DownloadCtrl Constructor.
//----------------------------------------------------------------------------------
DownloadCtrl::DownloadCtrl(void)
	: WizardCtrl()
{
	AddControls();
}

//----------------------------------------------------------------------------------
// DownloadCtrl Destructor.
//----------------------------------------------------------------------------------
DownloadCtrl::~DownloadCtrl(void)
{
}

//----------------------------------------------------------------------------------
// AddControls: Initialize all of the sub-controls.
//----------------------------------------------------------------------------------
void DownloadCtrl::AddControls(void)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();
	DisplayContext* pContext = GetWindow()->GetDisplayContext();

	// Background.
	NativeImagePtr pBackroundImg = pResMgr->GetBackgroundImage(IDB_DOWNLOAD_DLG_BACKGROUND);
	ImageComponentPtr pBackground = new ImageComponent(pBackroundImg, true);
	AddChildLayout(pBackground, CLI_SameSize, this);

	// Help Button (Navigation Control).
	m_pHelpButton = new MSButton(pResMgr->GetString(IDS_DOWNLOAD_DLG_HELP));
	m_pHelpButton->SetDesiredSize();
	AddChildLayout(m_pHelpButton, CLI_SameBottom | CLI_SameLeft, this, NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Next Button (Navigation Control).
	m_pNextButton = new MSButton(pResMgr->GetString(IDS_DOWNLOAD_DLG_NEXT));
	m_pNextButton->SetDesiredSize();
	AddChildLayout(m_pNextButton, CLI_SameBottom | CLI_SameRight, this, -NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Back Button (Navigation Control).
	m_pBackButton = new MSButton(pResMgr->GetString(IDS_DOWNLOAD_DLG_BACK));
	m_pBackButton->SetDesiredSize();
	AddChildLayout(m_pBackButton, CLI_SameBottom | CLI_Left, m_pNextButton, -NAV_BTN_SPACING, 0, 0, 0);

	// Separator Bar (Navigation Control).
	MSSeperatorPtr pSeparator = new MSSeperator(SEP_BAR_HEIGHT);
	AddChildLayout(pSeparator, CLI_Above, m_pNextButton, 0, -SEP_BAR_VERT_MARGIN, 0, 0);
	AddChildLayout(pSeparator, CLI_SameLeft | CLI_GrowToRight, this, SEP_BAR_HORIZ_MARGIN, 0, -SEP_BAR_HORIZ_MARGIN, 0);

	// Game Logo (Dialog-Specific Control).
	NativeImagePtr pBrandImg = pResMgr->GetGameLogoImage(IDB_DOWNLOAD_DLG_GAME_LOGO);
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

	// Visit Host Label (Dialog-Specific Control).
	m_pVisitHostLabel = new Label;
	m_pVisitHostLabel->SetText(pResMgr->GetString(IDS_DOWNLOAD_DLG_HOST));
	m_pVisitHostLabel->SetDesiredSize();
	m_pVisitHostLabel->SetBackground(-1);	
	AddChildLayout(m_pVisitHostLabel, CLI_Right | CLI_SameTop, pBrandFrame, 0, STD_VERT_MARGIN, 0, 0);

	// Visit Host Button (Dialog-Specific Control).
	m_pVisitHostButton = new MSButton(pResMgr->GetString(IDS_DOWNLOAD_DLG_VISIT_HOST));
//	m_pVisitHostButton->SetDesiredSize();
	m_pVisitHostButton->SetSize(308, 40);
	AddChildLayout(m_pVisitHostButton, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(m_pVisitHostButton, CLI_Below, m_pVisitHostLabel, 0, INPUT_VERT_SPACING, 0, 0);

	// Time Left Text (Dialog-Specific Control).
	m_pTimeLeftLabel = new Label;
	m_pTimeLeftLabel->SetText(pResMgr->GetString(IDS_DOWNLOAD_DLG_ESTIMATING));
	m_pTimeLeftLabel->SetDesiredSize();
	m_pTimeLeftLabel->SetBackground(-1);	
	AddChildLayout(m_pTimeLeftLabel, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(m_pTimeLeftLabel, CLI_GrowToRight, m_pNextButton, 0, 0, 0, 0);
	AddChildLayout(m_pTimeLeftLabel, CLI_Above, pSeparator, 0, -STD_VERT_MARGIN, 0, 0);

	// Progress Status Text (Dialog-Specific Control).
	m_pProgressStatusLabel = new Label;
	m_pProgressStatusLabel->SetText(pResMgr->GetString(IDS_DOWNLOADING_INFO));
	m_pProgressStatusLabel->SetDesiredSize();
	m_pProgressStatusLabel->SetBackground(-1);	
	AddChildLayout(m_pProgressStatusLabel, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(m_pProgressStatusLabel, CLI_GrowToRight, m_pNextButton, 0, 0, 0, 0);
	AddChildLayout(m_pProgressStatusLabel, CLI_Above, m_pTimeLeftLabel, 0, -INPUT_VERT_SPACING, 0, 0);

	// Progress Bar Frame (Dialog-Specific Control).
	MS3DFramePtr pProgressFrame = new MS3DFrame(3, false);
	pProgressFrame->SetHeight(m_pNextButton->Height());
	AddChildLayout(pProgressFrame, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(pProgressFrame, CLI_Above, m_pProgressStatusLabel, 0, -INPUT_VERT_SPACING, 0, 0);
	AddChildLayout(pProgressFrame, CLI_GrowToRight, m_pNextButton, 0, 0, 0, 0);
//	pProgressFrame->SetVisible(false);
	
	// Progress Bar (Dialog-Specific Control).
	m_pProgressBar = new ProgressBarComponent();
///	m_pProgressBar->SetRange(0, 100); //???
///	m_pProgressBar->SetPosition(75); //???
	if (pProgressFrame->IsVisible())
		AddChildLayout(m_pProgressBar, CLI_SameSize | CLI_SameLeft | CLI_SameTop, pProgressFrame, PROG_BAR_FRAME_WD, PROG_BAR_FRAME_WD, -2 * PROG_BAR_FRAME_WD, -2 * PROG_BAR_FRAME_WD);
	else
		AddChildLayout(m_pProgressBar, CLI_SameSize | CLI_SameLeft | CLI_SameTop, pProgressFrame, 0, 0, 0, 0);

	// Progress Label (Dialog-Specific Control).
	LabelPtr pProgressLabel = new Label;
	pProgressLabel->SetText(pResMgr->GetString(IDS_DOWNLOAD_DLG_PROGRESS));
	pProgressLabel->SetDesiredSize();
	pProgressLabel->SetBackground(-1);	
	AddChildLayout(pProgressLabel, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(pProgressLabel, CLI_Above, pProgressFrame, 0, -INPUT_VERT_SPACING, 0, 0);

	// Information Text (Dialog-Specific Control).
	m_pInfoText = new TextArea;
	m_pInfoText->SetComponentFlags(ComponentFlag_GrabBG, true);
	m_pInfoText->SetBackground(-1);
	m_pInfoText->SetBorderPadding(TEXT_AREA_LEFT_PAD, TEXT_AREA_TOP_PAD, TEXT_AREA_RIGHT_PAD, TEXT_AREA_BOTTOM_PAD);
	MSScrollContainerPtr pInfoBox = new MSScrollContainer(m_pInfoText);
	pInfoBox->SetScrollbarConditions(ScrollbarCondition_Disable, ScrollbarCondition_Conditional);
	pInfoBox->SetDraw3DFrame(true);
	pInfoBox->SetSize(10, 10); // Needs to be big enough to avoid a GPF.
	AddChildLayout(pInfoBox, CLI_SameTop, m_pVisitHostButton, 0, m_pVisitHostButton->Height() + STD_VERT_MARGIN, 0, 0);
	AddChildLayout(pInfoBox, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(pInfoBox, CLI_GrowToRight, m_pNextButton, 0, 0, 0, 0);
	AddChildLayout(pInfoBox, CLI_GrowToTop, pProgressLabel, 0, 0, 0, -STD_VERT_MARGIN);

	// Add the controls in the desired tab order.
	AddChild(pBackground);
	AddChild(pBrandLogo);
	AddChild(m_pVisitHostLabel);
	AddChild(m_pVisitHostButton);
	AddChild(pInfoBox);
	AddChild(pProgressLabel);
	AddChild(pProgressFrame);
	AddChild(m_pProgressBar);
	AddChild(m_pProgressStatusLabel);
	AddChild(m_pTimeLeftLabel);
	AddChild(pSeparator);
	AddChild(m_pHelpButton);
	AddChild(m_pBackButton);
	AddChild(m_pNextButton);
}

//----------------------------------------------------------------------------------
// Show: Make this dialog visible (or invisible), and start (or stop) any threads 
// that are needed.
//----------------------------------------------------------------------------------
void DownloadCtrl::Show(bool bShow)
{
	SetVisible(bShow);

	if (bShow)
	{
		// Some of the text changes dynamically based on patch selection, update it.
		ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

		// Informational (Main) text.
		GUIString sInfo = pResMgr->BuildInfoString(IDS_DOWNLOAD_DLG_INFO_1);
		m_pInfoText->Clear();
		m_pInfoText->AddFormatedText(sInfo);
		m_pInfoText->SetVertOffset(0); // Scroll to the top.

		// Visit Host prompt.
		m_pVisitHostLabel->SetText(pResMgr->GetString(IDS_DOWNLOAD_DLG_HOST));
		m_pVisitHostLabel->SetDesiredSize();

		// Visit Host button.
		m_pVisitHostButton = new MSButton(pResMgr->GetString(IDS_DOWNLOAD_DLG_VISIT_HOST));

		// Time Remaining text.
		m_pTimeLeftLabel->SetText(pResMgr->GetString(IDS_DOWNLOAD_DLG_ESTIMATING));

		// Download Progress text.
		m_pProgressStatusLabel->SetText(pResMgr->GetString(IDS_DOWNLOADING_INFO));

		// Reset the Progress Bar.
		m_pProgressBar->SetRange(0, 100); //???
		m_pProgressBar->SetPosition(75); //???

		// Make sure the Next button is the default button.
		m_pNextButton->RequestFocus();
	}

	// Start ???
	//if (bShow && ! <???>)
	{
		// ?????
	}
}

//----------------------------------------------------------------------------------
// HandleVisitHostButton: Process a click on the Visit Host button.
//----------------------------------------------------------------------------------
bool DownloadCtrl::HandleVisitHostButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	//?????

	return true;
}

//----------------------------------------------------------------------------------
// HandleHelpButton: Process a click on the Help button.
//----------------------------------------------------------------------------------
bool DownloadCtrl::HandleHelpButton(ComponentEvent* pEvent)
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
//----------------------------------------------------------------------------------
bool DownloadCtrl::HandleBackButton(ComponentEvent* pEvent)
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
//----------------------------------------------------------------------------------
bool DownloadCtrl::HandleNextButton(ComponentEvent* pEvent)
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
void DownloadCtrl::HandleComponentEvent(ComponentEvent* pEvent)
{
	if (pEvent->mComponent == m_pHelpButton && HandleHelpButton(pEvent))
		return;
	if (pEvent->mComponent == m_pBackButton && HandleBackButton(pEvent))
		return;
	if (pEvent->mComponent == m_pNextButton && HandleNextButton(pEvent))
		return;
	if (pEvent->mComponent == m_pVisitHostButton && HandleVisitHostButton(pEvent))
		return;

	Container::HandleComponentEvent(pEvent);
}
