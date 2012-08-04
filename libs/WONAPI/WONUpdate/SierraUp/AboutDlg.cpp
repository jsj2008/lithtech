//----------------------------------------------------------------------------------
// AboutDlg.cpp
//----------------------------------------------------------------------------------
#include <assert.h>
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONGUI/SimpleComponent.h"
#include "AboutDlg.h"
#include "../CustomInfo.h"
#include "../WizardCtrl.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// AboutDlg Constructor
//----------------------------------------------------------------------------------
AboutDlg::AboutDlg(const GUIString& sVer)
{
	m_sVersion = sVer;
	AddControls();
}

//----------------------------------------------------------------------------------
// AboutDlg Destructor
//----------------------------------------------------------------------------------
AboutDlg::~AboutDlg(void)
{
}

//----------------------------------------------------------------------------------
// AddControls: Initialize all of the sub-controls.
//----------------------------------------------------------------------------------
void AboutDlg::AddControls(void)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();
	DisplayContext* pContext = GetWindow()->GetDisplayContext();

	int nDesiredWd = 0;

	// Background.
	NativeImagePtr pBackroundImg = pResMgr->GetBackgroundImage(IDB_ABOUT_DLG_BACKGROUND);
	ImageComponentPtr pBackground = new ImageComponent(pBackroundImg, true);
	AddChildLayout(pBackground, CLI_SameSize, this);

	// MPS Logo (Dialog-Specific Control).
	NativeImagePtr pMpsImg = pResMgr->GetMpsLogoImage();
	ImageComponentPtr pMpsLogo = new ImageComponent(pMpsImg);

	LayoutPointPtr pMpsFrame = new LayoutPoint();
	if (pMpsLogo->Height())
	{
		nDesiredWd = pMpsLogo->Width();
		pMpsFrame->SetHeight(pMpsLogo->Height() + 2 * STD_VERT_MARGIN);
	}
	else
		pMpsFrame->SetHeight(STD_VERT_MARGIN);
	AddChildLayout(pMpsFrame, CLI_SameTop | CLI_SameLeft | CLI_GrowToRight, this, 0, 0, 0, 0);
	AddLayoutPoint(pMpsFrame);
	CenterLayoutPtr pMpsCenterLayout = new CenterLayout(pMpsFrame, pMpsLogo, true, true);
	AddChildLayout(pMpsCenterLayout);

	// Version Info.
	m_pVersionLabel = new MSLabel();
	GUIString sVersion = pResMgr->GetString(IDS_ABOUT_DLG_VERSION);
	sVersion.append(m_sVersion);
	m_pVersionLabel->SetText(sVersion);
	m_pVersionLabel->SetDesiredSize();
	m_pVersionLabel->SetTransparent(true);
	if (nDesiredWd < m_pVersionLabel->Width())
		nDesiredWd = m_pVersionLabel->Width();

	LayoutPointPtr pVersionFrame = new LayoutPoint();
	pVersionFrame->SetHeight(m_pVersionLabel->Height());
	AddChildLayout(pVersionFrame, CLI_Below, pMpsFrame, 0, 0, 0, 0);
	AddChildLayout(pVersionFrame, CLI_SameLeft | CLI_GrowToRight, this, 0, 0, 0, 0);
	AddLayoutPoint(pVersionFrame);
	CenterLayoutPtr pVersionCenterLayout = new CenterLayout(pVersionFrame, m_pVersionLabel, true, true);
	AddChildLayout(pVersionCenterLayout);

	// Copyright Info.
	m_pCopyrightLabel = new MSLabel();
	m_pCopyrightLabel->SetText(pResMgr->GetString(IDS_ABOUT_DLG_COPYRIGHT));
	m_pCopyrightLabel->SetDesiredSize();
	m_pCopyrightLabel->SetTransparent(true);
	if (nDesiredWd < m_pCopyrightLabel->Width())
		nDesiredWd = m_pCopyrightLabel->Width();

	LayoutPointPtr pCopyrightFrame = new LayoutPoint();
	pCopyrightFrame->SetHeight(m_pCopyrightLabel->Height());
	AddChildLayout(pCopyrightFrame, CLI_Below, pVersionFrame, 0, INPUT_VERT_SPACING, 0, 0);
	AddChildLayout(pCopyrightFrame, CLI_SameLeft | CLI_GrowToRight, this, 0, 0, 0, 0);
	AddLayoutPoint(pCopyrightFrame);
	CenterLayoutPtr pCopyrightCenterLayout = new CenterLayout(pCopyrightFrame, m_pCopyrightLabel, true, true);
	AddChildLayout(pCopyrightCenterLayout);

	// Close Button.
	m_pCloseButton = new MSButton(pResMgr->GetString(IDS_COMMON_DLG_CLOSE));
	m_pCloseButton->SetDesiredSize();
	if (nDesiredWd < m_pCloseButton->Width())
		nDesiredWd = m_pCloseButton->Width();

	LayoutPointPtr pButtonFrame = new LayoutPoint();
	pButtonFrame->SetHeight(m_pCloseButton->Height());
	AddChildLayout(pButtonFrame, CLI_Below, pCopyrightFrame, 0, STD_VERT_MARGIN, 0, 0);
	AddChildLayout(pButtonFrame, CLI_SameLeft | CLI_GrowToRight, this, 0, 0, 0, 0);
	AddLayoutPoint(pButtonFrame);
	CenterLayoutPtr pButtonCenterLayout = new CenterLayout(pButtonFrame, m_pCloseButton, true, true);
	AddChildLayout(pButtonCenterLayout);

	// Enable the Escape (and upper right 'X' button).
	SetEndOnEscape(1);

	int nDesiredHt = pMpsFrame->Height() + // Includes upper and lower white space.
					 m_pVersionLabel->Height() + 
					 INPUT_VERT_SPACING +
					 m_pCopyrightLabel->Height() + 
					 STD_VERT_MARGIN +
					 m_pCloseButton->Height() + 
					 STD_VERT_MARGIN;

	// Update the dialog dimensions to use the size we calculated.
	SetSize(nDesiredWd + 2 * STD_HORIZ_MARGIN, nDesiredHt);

	// Add the controls in the desired tab order.
	AddChild(pBackground);
	AddChild(pMpsLogo);
	AddChild(m_pVersionLabel);
	AddChild(m_pCopyrightLabel);
	AddChild(m_pCloseButton);
}

//----------------------------------------------------------------------------------
// DoDialog: Display this dialog as a modal dialog.
//----------------------------------------------------------------------------------
int AboutDlg::DoDialog(Window *pParent)
{
	PlatformWindowPtr pWindow = new PlatformWindow;
	pWindow->SetParent(pParent);
	pWindow->SetTitle(GetCustomInfo()->GetResourceManager()->GetString(IDS_ABOUT_DLG_TITLE));
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
	m_pCloseButton->RequestFocus();

	int nResult = pWindow->DoDialog(this);
	pWindow->Close();

	return nResult;
}

//----------------------------------------------------------------------------------
// HandleCloseButton: Close the dialog box.
//----------------------------------------------------------------------------------
bool AboutDlg::HandleCloseButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	GetWindow()->EndDialog();

	return true;
}

//----------------------------------------------------------------------------------
// HandleComponentEvent: Handle Component Events (Button pushes and the like).
//----------------------------------------------------------------------------------
void AboutDlg::HandleComponentEvent(ComponentEvent* pEvent)
{
	if (pEvent->mComponent == m_pCloseButton && HandleCloseButton(pEvent))
		return;

	MSDialog::HandleComponentEvent(pEvent);
}
