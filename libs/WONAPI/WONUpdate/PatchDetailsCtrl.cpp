//----------------------------------------------------------------------------------
// PatchDetailsCtrl.cpp
//----------------------------------------------------------------------------------
#include <assert.h>
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONGUI/TabCtrl.h"
#include "WONGUI/SimpleComponent.h"
#include "PatchDetailsCtrl.h"
#include "WONUpdateCtrl.h"
#include "CustomInfo.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// PatchDetailsCtrl Constructor.
//----------------------------------------------------------------------------------
PatchDetailsCtrl::PatchDetailsCtrl(void)
	: WizardCtrl()
{
	AddControls();
}

//----------------------------------------------------------------------------------
// PatchDetailsCtrl Destructor.
//----------------------------------------------------------------------------------
PatchDetailsCtrl::~PatchDetailsCtrl(void)
{
}

//----------------------------------------------------------------------------------
// AddControls: Initialize all of the sub-controls.
//----------------------------------------------------------------------------------
void PatchDetailsCtrl::AddControls(void)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();
	DisplayContext* pContext = GetWindow()->GetDisplayContext();

	// Background.
	NativeImagePtr pBackroundImg = pResMgr->GetBackgroundImage(IDB_PATCH_DETAILS_DLG_BACKGROUND);
	ImageComponentPtr pBackground = new ImageComponent(pBackroundImg, true);
	AddChildLayout(pBackground, CLI_SameSize, this);

	// Help Button (Navigation Control).
	m_pHelpButton = new MSButton(pResMgr->GetString(IDS_PATCH_DETAILS_DLG_HELP));
	m_pHelpButton->SetDesiredSize();
	AddChildLayout(m_pHelpButton, CLI_SameBottom | CLI_SameLeft, this, NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Next (AKA Skip) Button (Navigation Control).
	m_pNextButton = new MSButton(pResMgr->GetString(IDS_PATCH_DETAILS_DLG_NEXT));
	m_pNextButton->SetDesiredSize();
	AddChildLayout(m_pNextButton, CLI_SameBottom | CLI_SameRight, this, -NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Back Button (Navigation Control).
	m_pBackButton = new MSButton(pResMgr->GetString(IDS_PATCH_DETAILS_DLG_BACK));
	m_pBackButton->SetDesiredSize();
	AddChildLayout(m_pBackButton, CLI_SameBottom | CLI_Left, m_pNextButton, -NAV_BTN_SPACING, 0, 0, 0);

	// Separator Bar (Navigation Control).
	MSSeperatorPtr pSeparator = new MSSeperator(SEP_BAR_HEIGHT);
	AddChildLayout(pSeparator, CLI_Above, m_pNextButton, 0, -SEP_BAR_VERT_MARGIN, 0, 0);
	AddChildLayout(pSeparator, CLI_SameLeft | CLI_GrowToRight, this, SEP_BAR_HORIZ_MARGIN, 0, -SEP_BAR_HORIZ_MARGIN, 0);

	// Game Logo (Dialog-Specific Control).
	NativeImagePtr pBrandImg = pResMgr->GetGameLogoImage(IDB_PATCH_DETAILS_DLG_GAME_LOGO);
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
}

//----------------------------------------------------------------------------------
// Show: Make this dialog visible (or invisible), and start (or stop) any threads 
// that are needed.
//----------------------------------------------------------------------------------
void PatchDetailsCtrl::Show(bool bShow)
{
	SetVisible(bShow);

	if (bShow)
	{
		// Some of the text changes dynamically based on patch selection, update it.
		ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

		// Informational (Main) text.
		GUIString sInfo = pResMgr->BuildInfoString(IDS_PATCH_DETAILS_DLG_INFO_1);
		m_pInfoText->Clear();
		m_pInfoText->AddFormatedText(sInfo);
		m_pInfoText->SetVertOffset(0); // Scroll to the top.

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
// HandleHelpButton: Process a click on the Help button.
//----------------------------------------------------------------------------------
bool PatchDetailsCtrl::HandleHelpButton(ComponentEvent* pEvent)
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
bool PatchDetailsCtrl::HandleBackButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	// If the Patch Details have not been downloaded, close the threads.
	//?????

	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowPreviousScreen();

	return true;
}

//----------------------------------------------------------------------------------
// HandleNextButton: Process a click on the Next button.
//----------------------------------------------------------------------------------
bool PatchDetailsCtrl::HandleNextButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	// If the Patch Details have not been downloaded, close the threads.
	//?????

	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowNextScreen();

	return true;
}

//----------------------------------------------------------------------------------
// HandleComponentEvent: Handle Component Events (Button pushes and the like).
//----------------------------------------------------------------------------------
void PatchDetailsCtrl::HandleComponentEvent(ComponentEvent* pEvent)
{
	if (pEvent->mComponent == m_pHelpButton && HandleHelpButton(pEvent))
		return;
	if (pEvent->mComponent == m_pBackButton && HandleBackButton(pEvent))
		return;
	if (pEvent->mComponent == m_pNextButton && HandleNextButton(pEvent))
		return;

	Container::HandleComponentEvent(pEvent);
}
