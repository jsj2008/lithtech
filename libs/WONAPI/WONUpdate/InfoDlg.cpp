//----------------------------------------------------------------------------------
// InfoDlg.cpp
//
// This dialog is intended to be displayed while a non-cancelable activity is going 
// on.  It does not have any buttons (or other mechanism) allowing the user to 
// cancel what is going on.
//----------------------------------------------------------------------------------
#include <assert.h>
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONGUI/SimpleComponent.h"
#include "InfoDlg.h"
#include "WizardCtrl.h"
#include "DownloadCtrl.h"
#include "WONUpdateCtrl.h"
#include "CustomInfo.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// InfoDlg Constructor
//----------------------------------------------------------------------------------
InfoDlg::InfoDlg(const GUIString& sInfo, const GUIString& sTitle)
{
	m_sInfo = sInfo;
//%%%	mTitle = sTitle;
	AddControls();
//%%%	mSystemMenu = false;
}

//----------------------------------------------------------------------------------
// InfoDlg Destructor
//----------------------------------------------------------------------------------
InfoDlg::~InfoDlg(void)
{
}

//----------------------------------------------------------------------------------
// AddControls: Initialize all of the sub-controls.
//----------------------------------------------------------------------------------
void InfoDlg::AddControls(void)
{
	SetSize(300, 100);

	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();
	DisplayContext* pContext = GetWindow()->GetDisplayContext();

	// Background.
	NativeImagePtr pBackroundImg = pResMgr->GetBackgroundImage(IDB_INFO_DLG_BACKGROUND);
	ImageComponentPtr pBackground = new ImageComponent(pBackroundImg, true);
	AddChildLayout(pBackground, CLI_SameSize, this);

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
	AddChildLayout(pInfoBox, CLI_SameTop | CLI_SameLeft | CLI_SameSize, this, STD_HORIZ_MARGIN, STD_VERT_MARGIN, -STD_HORIZ_MARGIN, -STD_VERT_MARGIN);

	// Add the controls in the desired tab order.
	AddChild(pBackground);
	AddChild(pInfoBox);

	m_pInfoText->AddFormatedText(m_sInfo);
	m_pInfoText->SetVertOffset(0); // Scroll to the top.
}

//----------------------------------------------------------------------------------
// HandleComponentEvent: Handle Component Events (Button pushes and the like).
//----------------------------------------------------------------------------------
void InfoDlg::HandleComponentEvent(ComponentEvent* pEvent)
{
	Container::HandleComponentEvent(pEvent);
}
