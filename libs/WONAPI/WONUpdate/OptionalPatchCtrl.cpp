//----------------------------------------------------------------------------------
// OptionalPatchCtrl.cpp
//----------------------------------------------------------------------------------
#include <assert.h>
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONGUI/TabCtrl.h"
#include "WONGUI/SimpleComponent.h"
#include "OptionalPatchCtrl.h"
#include "WONUpdateCtrl.h"
#include "CustomInfo.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// OptionalPatchCtrl Constructor.
//----------------------------------------------------------------------------------
OptionalPatchCtrl::OptionalPatchCtrl(void)
	: WizardCtrl()
{
	AddControls();
}

//----------------------------------------------------------------------------------
// OptionalPatchCtrl Destructor.
//----------------------------------------------------------------------------------
OptionalPatchCtrl::~OptionalPatchCtrl(void)
{
}

//----------------------------------------------------------------------------------
// AddControls: Initialize all of the sub-controls.
//----------------------------------------------------------------------------------
void OptionalPatchCtrl::AddControls(void)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();
	DisplayContext* pContext = GetWindow()->GetDisplayContext();

	// Background.
	NativeImagePtr pBackroundImg = pResMgr->GetBackgroundImage(IDB_OPTIONAL_PATCH_DLG_BACKGROUND);
	ImageComponentPtr pBackground = new ImageComponent(pBackroundImg, true);
	AddChildLayout(pBackground, CLI_SameSize, this);

	// Help Button (Navigation Control).
	m_pHelpButton = new MSButton(pResMgr->GetString(IDS_OPTIONAL_PATCH_DLG_HELP));
	m_pHelpButton->SetDesiredSize();
	AddChildLayout(m_pHelpButton, CLI_SameBottom | CLI_SameLeft, this, NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Next Button (Navigation Control).
	m_pNextButton = new MSButton(pResMgr->GetString(IDS_OPTIONAL_PATCH_DLG_NEXT));
	m_pNextButton->SetDesiredSize();
	AddChildLayout(m_pNextButton, CLI_SameBottom | CLI_SameRight, this, -NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Back Button (Navigation Control).
	m_pBackButton = new MSButton(pResMgr->GetString(IDS_OPTIONAL_PATCH_DLG_BACK));
	m_pBackButton->SetDesiredSize();
	AddChildLayout(m_pBackButton, CLI_SameBottom | CLI_Left, m_pNextButton, -NAV_BTN_SPACING, 0, 0, 0);

	// Separator Bar (Navigation Control).
	MSSeperatorPtr pSeparator = new MSSeperator(SEP_BAR_HEIGHT);
	AddChildLayout(pSeparator, CLI_Above, m_pNextButton, 0, -SEP_BAR_VERT_MARGIN, 0, 0);
	AddChildLayout(pSeparator, CLI_SameLeft | CLI_GrowToRight, this, SEP_BAR_HORIZ_MARGIN, 0, -SEP_BAR_HORIZ_MARGIN, 0);

	// Game Logo (Dialog-Specific Control).
	NativeImagePtr pBrandImg = pResMgr->GetGameLogoImage(IDB_OPTIONAL_PATCH_DLG_GAME_LOGO);
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

	// Patch Details Button (Dialog-Specific Control).
	m_pPatchDetailsButton = new MSButton(pResMgr->GetString(IDS_OPTIONAL_PATCH_DLG_VIEW_DETAILS));
	m_pPatchDetailsButton->SetDesiredSize();

	// Center the button under the patch list.
	LayoutPointPtr pButtonFrame = new LayoutPoint();
	pButtonFrame->SetHeight(m_pPatchDetailsButton->Height());
	AddChildLayout(pButtonFrame, CLI_Above, pSeparator, 0, -STD_VERT_MARGIN, 0, 0);
	AddChildLayout(pButtonFrame, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(pButtonFrame, CLI_GrowToRight, m_pNextButton, 0, 0, 0, 0);
	AddLayoutPoint(pButtonFrame);
	CenterLayoutPtr pButtonCenterLayout = new CenterLayout(pButtonFrame, m_pPatchDetailsButton, true, true);
	AddChildLayout(pButtonCenterLayout);

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
//	AddChildLayout(pInfoBox, CLI_GrowToTop, pButtonFrame, 0, 0, 0, -STD_VERT_MARGIN);
	pInfoBox->SetHeight(100); //????

	// Patch Version List (Dialog-Specific Control).
	m_pPatchList = new MSListCtrl();
	m_pPatchList->SetComponentFlags(ComponentFlag_GrabBG, true);
	m_pPatchList->SetDraw3DFrame(true);
	m_pPatchList->SetScrollbarConditions(ScrollbarCondition_Disable, ScrollbarCondition_Conditional);
	AddChildLayout(m_pPatchList, CLI_Below, pInfoBox, 0, STD_VERT_MARGIN, 0, 0);
	AddChildLayout(m_pPatchList, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(m_pPatchList, CLI_GrowToRight, m_pNextButton, 0, 0, 0, 0);
	AddChildLayout(m_pPatchList, CLI_GrowToTop, pButtonFrame, 0, 0, 0, -STD_VERT_MARGIN);

	// Add the controls in the desired tab order.
	AddChild(pBackground);
	AddChild(pBrandLogo);
	AddChild(m_pPatchList);
	AddChild(pInfoBox);
	AddChild(m_pPatchDetailsButton);
	AddChild(pSeparator);
	AddChild(m_pHelpButton);
	AddChild(m_pBackButton);
	AddChild(m_pNextButton);
}

//----------------------------------------------------------------------------------
// Show: Make this dialog visible (or invisible), and start (or stop) any threads 
// that are needed.
//----------------------------------------------------------------------------------
void OptionalPatchCtrl::Show(bool bShow)
{
	SetVisible(bShow);

	if (bShow)
	{
		// Some of the text changes dynamically based on patch selection, update it.
		ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

		// Informational (Main) text.
		GUIString sInfo = pResMgr->BuildInfoString(IDS_OPTIONAL_PATCH_DLG_INFO_1);
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
// HandlePatchDetailsButton: Process a click on the Patch Details Button.
//----------------------------------------------------------------------------------
bool OptionalPatchCtrl::HandlePatchDetailsButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowScreen(WONUpdateCtrl::WS_PatchDetails);

	return true;
}

//----------------------------------------------------------------------------------
// HandleHelpButton: Process a click on the Help Button.
//----------------------------------------------------------------------------------
bool OptionalPatchCtrl::HandleHelpButton(ComponentEvent* pEvent)
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
// HandleBackButton: Process a click on the Back Button.
//----------------------------------------------------------------------------------
bool OptionalPatchCtrl::HandleBackButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowPreviousScreen();

	return true;
}

//----------------------------------------------------------------------------------
// HandleNextButton: Process a click on the Next Button.
//----------------------------------------------------------------------------------
bool OptionalPatchCtrl::HandleNextButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	//?????

	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowNextScreen();

	return true;
}

//----------------------------------------------------------------------------------
// HandleComponentEvent: Handle Component Events (Button pushes and the like).
//----------------------------------------------------------------------------------
void OptionalPatchCtrl::HandleComponentEvent(ComponentEvent* pEvent)
{
	if (pEvent->mComponent == m_pHelpButton && HandleHelpButton(pEvent))
		return;
	if (pEvent->mComponent == m_pBackButton && HandleBackButton(pEvent))
		return;
	if (pEvent->mComponent == m_pNextButton && HandleNextButton(pEvent))
		return;
	if (pEvent->mComponent == m_pPatchDetailsButton && HandlePatchDetailsButton(pEvent))
		return;

	Container::HandleComponentEvent(pEvent);
}
