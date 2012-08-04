//----------------------------------------------------------------------------------
// SelectHostCtrl.cpp
//----------------------------------------------------------------------------------
#include <assert.h>
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONGUI/TabCtrl.h"
#include "WONGUI/SimpleComponent.h"
#include "SelectHostCtrl.h"
#include "WONUpdateCtrl.h"
#include "CustomInfo.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// SelectHostCtrl Constructor.
//----------------------------------------------------------------------------------
SelectHostCtrl::SelectHostCtrl(void)
	: WizardCtrl()
{
	AddControls();
}

//----------------------------------------------------------------------------------
// SelectHostCtrl Destructor.
//----------------------------------------------------------------------------------
SelectHostCtrl::~SelectHostCtrl(void)
{
}

//----------------------------------------------------------------------------------
// AddControls: Initialize all of the sub-controls.
//----------------------------------------------------------------------------------
void SelectHostCtrl::AddControls(void)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();
	DisplayContext* pContext = GetWindow()->GetDisplayContext();

	// Background.
	NativeImagePtr pBackroundImg = pResMgr->GetBackgroundImage(IDB_SELECT_HOST_DLG_BACKGROUND);
	ImageComponentPtr pBackground = new ImageComponent(pBackroundImg, true);
	AddChildLayout(pBackground, CLI_SameSize, this);

	// Help Button (Navigation Control).
	m_pHelpButton = new MSButton(pResMgr->GetString(IDS_SELECT_HOST_DLG_HELP));
	m_pHelpButton->SetDesiredSize();
	AddChildLayout(m_pHelpButton, CLI_SameBottom | CLI_SameLeft, this, NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Next Button (Navigation Control).
	m_pNextButton = new MSButton(pResMgr->GetString(IDS_SELECT_HOST_DLG_NEXT));
	m_pNextButton->SetDesiredSize();
	AddChildLayout(m_pNextButton, CLI_SameBottom | CLI_SameRight, this, -NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Back Button (Navigation Control).
	m_pBackButton = new MSButton(pResMgr->GetString(IDS_SELECT_HOST_DLG_BACK));
	m_pBackButton->SetDesiredSize();
	AddChildLayout(m_pBackButton, CLI_SameBottom | CLI_Left, m_pNextButton, -NAV_BTN_SPACING, 0, 0, 0);

	// Separator Bar (Navigation Control).
	MSSeperatorPtr pSeparator = new MSSeperator(SEP_BAR_HEIGHT);
	AddChildLayout(pSeparator, CLI_Above, m_pNextButton, 0, -SEP_BAR_VERT_MARGIN, 0, 0);
	AddChildLayout(pSeparator, CLI_SameLeft | CLI_GrowToRight, this, SEP_BAR_HORIZ_MARGIN, 0, -SEP_BAR_HORIZ_MARGIN, 0);

	// Game Logo (Dialog-Specific Control).
	NativeImagePtr pBrandImg = pResMgr->GetGameLogoImage(IDB_SELECT_HOST_DLG_GAME_LOGO);
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

	// Configure Proxy Button (Dialog-Specific Control).
	m_pConfigProxyButton = new MSButton(pResMgr->GetString(IDS_SELECT_HOST_DLG_CONFIG_PROXY));
	m_pConfigProxyButton->SetDesiredSize();

	// Already Have Update Button (Dialog-Specific Control).
	m_pAlreadyHaveUpdateButton = new MSButton(pResMgr->GetString(IDS_SELECT_HOST_DLG_HAVE_PATCH));
	m_pAlreadyHaveUpdateButton->SetDesiredSize();

	// Center the previous two buttons in the remaining space (under the info text).
	LayoutPointPtr pButtonFrame = new LayoutPoint();
	pButtonFrame->SetHeight(m_pConfigProxyButton->Height());
	AddChildLayout(pButtonFrame, CLI_Above, pSeparator, 0, -STD_VERT_MARGIN, 0, 0);
	AddChildLayout(pButtonFrame, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(pButtonFrame, CLI_GrowToRight, m_pNextButton, 0, 0, 0, 0);
	AddLayoutPoint(pButtonFrame);
	CenterLayoutPtr pButtonCenterLayout = new CenterLayout(pButtonFrame, m_pConfigProxyButton, true, true);
	m_pAlreadyHaveUpdateButton->SetLeft(m_pConfigProxyButton->Width() + NAV_BTN_HORIZ_MARGIN);
	pButtonCenterLayout->Add(m_pAlreadyHaveUpdateButton);
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
	m_pHostList = new MSListCtrl();
	m_pHostList->SetComponentFlags(ComponentFlag_GrabBG, true);
	m_pHostList->SetDraw3DFrame(true);
	m_pHostList->SetScrollbarConditions(ScrollbarCondition_Disable, ScrollbarCondition_Conditional);
	AddChildLayout(m_pHostList, CLI_Below, pInfoBox, 0, STD_VERT_MARGIN, 0, 0);
	AddChildLayout(m_pHostList, CLI_Right, pBrandFrame, 0, 0, 0, 0);
	AddChildLayout(m_pHostList, CLI_GrowToRight, m_pNextButton, 0, 0, 0, 0);
	AddChildLayout(m_pHostList, CLI_GrowToTop, pButtonFrame, 0, 0, 0, -STD_VERT_MARGIN);
	MultiListArea* pListArea = m_pHostList->GetListArea();
	pListArea->SetNumColumns(3);
	pListArea->SetColumnWidth(0, 100);
	pListArea->SetColumnWidth(1, 50);
	pListArea->SetColumnWidth(2, 150);

	// Add the controls in the desired tab order.
	AddChild(pBackground);
	AddChild(pBrandLogo);
	AddChild(pInfoBox);
	AddChild(m_pHostList);
	AddChild(m_pConfigProxyButton);
	AddChild(m_pAlreadyHaveUpdateButton);
	AddChild(pSeparator);
	AddChild(m_pHelpButton);
	AddChild(m_pBackButton);
	AddChild(m_pNextButton);

	//????? Temp Code
	pListArea->InsertRow();
	pListArea->SetString(0, 0, "Sierra.com");
	pListArea->SetString(0, 1, "x");
	pListArea->SetString(0, 2, "x");

	pListArea->InsertRow();
	pListArea->SetString(1, 0, "File Planet");
	pListArea->SetString(1, 1, "x");
	pListArea->SetString(1, 2, "x");

	EnableControls();
}

//----------------------------------------------------------------------------------
// EnableControls: Enable or disable the controls as aproproiate.
//----------------------------------------------------------------------------------
void SelectHostCtrl::EnableControls(void)
{
	MultiListArea* pListArea = m_pHostList->GetListArea();
	m_pNextButton->Enable(pListArea->GetSelItem() != NULL);
}

//----------------------------------------------------------------------------------
// Show: Make this dialog visible (or invisible), and start (or stop) any threads 
// that are needed.
//----------------------------------------------------------------------------------
void SelectHostCtrl::Show(bool bShow)
{
	SetVisible(bShow);

	if (bShow)
	{
		// Some of the text changes dynamically based on patch selection, update it.
		ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

		// Informational (Main) text.
		GUIString sInfo = pResMgr->BuildInfoString(IDS_SELECT_HOST_DLG_INFO_1);
		m_pInfoText->Clear();
		m_pInfoText->AddFormatedText(sInfo);
		m_pInfoText->SetVertOffset(0); // Scroll to the top.

		// Make sure the sppropriate control has the focus.
		MultiListArea* pListArea = m_pHostList->GetListArea();
		if (pListArea->GetSelItem())
			m_pNextButton->RequestFocus();
		else
			m_pHostList->RequestFocus();
	}

	// Start ???
	//if (bShow && ! <???>)
	{
		// ?????
	}
}

//----------------------------------------------------------------------------------
// HandleAlreadyHaveUpdateButton: Process a click on the Already Have Update button.
//----------------------------------------------------------------------------------
bool SelectHostCtrl::HandleAlreadyHaveUpdateButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

	MSFileDialog Dlg;
	Dlg.SetFileMustExist(true);
	Dlg.AddFilter("*.exe", pResMgr->GetString(IDS_SELECT_HOST_DLG_PATCH_FILTER));
	Dlg.SetTitle(pResMgr->GetString(IDS_SELECT_HOST_DLG_FILE_OPEN_DLG_TITLE));
	//Dlg.SetOkTitle("Open");

	if (Dlg.DoDialog(GetWindow()))
	{
		//?????
	}

	return true;
}

//----------------------------------------------------------------------------------
// HandleConfigureProxyButton: Process a click on the Configure Proxy button.
//----------------------------------------------------------------------------------
bool SelectHostCtrl::HandleConfigProxyButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
		pUpdateControl->ShowScreen(WONUpdateCtrl::WS_ConfigProxy);

	return true;
}

//----------------------------------------------------------------------------------
// HandleHelpButton: Process a click on the Help button.
//----------------------------------------------------------------------------------
bool SelectHostCtrl::HandleHelpButton(ComponentEvent* pEvent)
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
bool SelectHostCtrl::HandleBackButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	// Send the user to the previous screen.
	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowPreviousScreen();

	return true;
}

//----------------------------------------------------------------------------------
// HandleNextButton: Process a click on the Next button.
//----------------------------------------------------------------------------------
bool SelectHostCtrl::HandleNextButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	// Extract the selected patch.
	MultiListArea* pListArea = m_pHostList->GetListArea();
	//????? Convert to a patch

	// Tag this patch so we know what we're dealing with later.
	//????? Save to wherever

	// Send the user to the next screen (it will vary based on the selected host).
	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowNextScreen();

	return true;
}

//----------------------------------------------------------------------------------
// HandleHostList: Process a change in the Host List.
//----------------------------------------------------------------------------------
bool SelectHostCtrl::HandleHostList(ComponentEvent* pRawEvent)
{
	// Check for a simple click or selection change.
	if (pRawEvent->mType == ComponentEvent_ListSelChanged)
	{
		EnableControls();
		return true;
	}

	// Look for a double click.
	ListItemClickedEvent* pEvent = dynamic_cast<ListItemClickedEvent*>(pRawEvent);
	if (pEvent && pEvent->mType == ComponentEvent_ListItemClicked && pEvent->mWasDoubleClick)
	{
		FireNextButton();
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------------
// HandleComponentEvent: Handle Component Events (Button pushes and the like).
//----------------------------------------------------------------------------------
void SelectHostCtrl::HandleComponentEvent(ComponentEvent* pEvent)
{
	if ((pEvent->mComponent == m_pHelpButton && HandleHelpButton(pEvent)) ||
		(pEvent->mComponent == m_pBackButton && HandleBackButton(pEvent)) ||
		(pEvent->mComponent == m_pNextButton && HandleNextButton(pEvent)) ||
		(pEvent->mComponent == m_pConfigProxyButton && HandleConfigProxyButton(pEvent)) ||
		(pEvent->mComponent == m_pAlreadyHaveUpdateButton && HandleAlreadyHaveUpdateButton(pEvent)) ||
		(pEvent->mComponent == m_pHostList->GetListArea() && HandleHostList(pEvent)))
	{
		return;
	}

	Container::HandleComponentEvent(pEvent);
}
