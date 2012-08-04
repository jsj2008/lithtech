//----------------------------------------------------------------------------------
// AbortDlg.cpp
//----------------------------------------------------------------------------------
#include <assert.h>
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONGUI/SimpleComponent.h"
#include "AbortDlg.h"
#include "WizardCtrl.h"
#include "CustomInfo.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// AbortDlg Constructor
//----------------------------------------------------------------------------------
AbortDlg::AbortDlg(const GUIString& sInfo, const GUIString& sTitle)
{
	m_sInfo = sInfo;
	m_sTitle = sTitle;
	AddControls();
}

//----------------------------------------------------------------------------------
// AbortDlg Destructor
//----------------------------------------------------------------------------------
AbortDlg::~AbortDlg(void)
{
}

//----------------------------------------------------------------------------------
// AddControls: Initialize all of the sub-controls.
//----------------------------------------------------------------------------------
void AbortDlg::AddControls(void)
{
	SetSize(300, 100);

	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();
	DisplayContext* pContext = GetWindow()->GetDisplayContext();

	// Background.
	NativeImagePtr pBackroundImg = pResMgr->GetBackgroundImage(IDB_ABORT_DLG_BACKGROUND);
	ImageComponentPtr pBackground = new ImageComponent(pBackroundImg, true);
	AddChildLayout(pBackground, CLI_SameSize, this);

	// Abort Button.
	m_pAbortButton = new MSButton(pResMgr->GetString(IDS_ABORT_DLG_ABORT));
	m_pAbortButton->SetDesiredSize();

	LayoutPointPtr pButtonFrame = new LayoutPoint();
	pButtonFrame->SetHeight(m_pAbortButton->Height());
	AddChildLayout(pButtonFrame, CLI_SameBottom | CLI_SameLeft | CLI_GrowToRight, this, 0, -STD_VERT_MARGIN, 0, 0);
	AddLayoutPoint(pButtonFrame);
	CenterLayoutPtr pButtonCenterLayout = new CenterLayout(pButtonFrame, m_pAbortButton, true, true);
	AddChildLayout(pButtonCenterLayout);

	// Information Text.
	m_pInfoText = new TextArea;
	m_pInfoText->SetComponentFlags(ComponentFlag_GrabBG, true);
	m_pInfoText->SetBackground(-1);
	m_pInfoText->SetLineAlignment(HorzAlign_Center);
	m_pInfoText->SetBorderPadding(TEXT_AREA_LEFT_PAD, TEXT_AREA_TOP_PAD, TEXT_AREA_RIGHT_PAD, TEXT_AREA_BOTTOM_PAD);
	MSScrollContainerPtr pInfoBox = new MSScrollContainer(m_pInfoText);
	pInfoBox->SetScrollbarConditions(ScrollbarCondition_Disable, ScrollbarCondition_Conditional);
	pInfoBox->SetDraw3DFrame(true);
	pInfoBox->SetSize(10, 10); // Needs to be big enough to avoid a GPF.
	AddChildLayout(pInfoBox, CLI_SameTop | CLI_SameLeft | CLI_GrowToRight, this, STD_HORIZ_MARGIN, STD_VERT_MARGIN, -STD_HORIZ_MARGIN, STD_VERT_MARGIN);
	AddChildLayout(pInfoBox, CLI_GrowToTop, m_pAbortButton, 0, 0, 0, -STD_VERT_MARGIN);

	// Add the controls in the desired tab order.
	AddChild(pBackground);
	AddChild(pInfoBox);
	AddChild(m_pAbortButton);

	m_pInfoText->AddFormatedText(m_sInfo);
	m_pInfoText->SetVertOffset(0); // Scroll to the top.

	// Enable the Escape (and upper right 'X' button).
	SetEndOnEscape(1);

	m_pAbortButton->RequestFocus();
}

//----------------------------------------------------------------------------------
// DoDialog: Display this dialog as a modal dialog.
//----------------------------------------------------------------------------------
int AbortDlg::DoDialog(Window *pParent)
{
	PlatformWindowPtr pWindow = new PlatformWindow;
	pWindow->SetParent(pParent);
	pWindow->SetTitle(m_sTitle);
	pWindow->SetCreateFlags(CreateWindow_SizeSpecClientArea | CreateWindow_NotSizeable);

	WONRectangle aRect;
	if (pParent != NULL)
		pParent->GetScreenPos(aRect);
	else
		pWindow->GetWindowManager()->GetScreenRect(aRect);

	// Center it on top of its parent (or the desktop).
	pWindow->Create(WONRectangle(aRect.Left() + (aRect.Width() - Width()) / 2,
								 aRect.Top() + (aRect.Height() - Height()) / 2,
								 Width(), Height()));

	pWindow->AddComponent(this);
	m_pAbortButton->RequestFocus();

	int nResult = pWindow->DoDialog(this);
	pWindow->Close();

	return nResult;
}

//----------------------------------------------------------------------------------
// HandleCloseButton: Close the dialog box.
//----------------------------------------------------------------------------------
bool AbortDlg::HandleAbortButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	GetWindow()->EndDialog();

	return true;
}

//----------------------------------------------------------------------------------
// HandleComponentEvent: Handle Component Events (Button pushes and the like).
//----------------------------------------------------------------------------------
void AbortDlg::HandleComponentEvent(ComponentEvent* pEvent)
{
	if (pEvent->mComponent == m_pAbortButton && HandleAbortButton(pEvent))
		return;

	MSDialog::HandleComponentEvent(pEvent);
}
