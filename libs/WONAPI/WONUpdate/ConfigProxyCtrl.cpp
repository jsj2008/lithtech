//----------------------------------------------------------------------------------
// ConfigProxyCtrl.cpp
//----------------------------------------------------------------------------------
#include <assert.h>
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONGUI/TabCtrl.h"
#include "WONGUI/ImageDecoder.h"
#include "WONGUI/SimpleComponent.h"
#include "ConfigProxyCtrl.h"
#include "WONUpdateCtrl.h"
#include "CustomInfo.h"

using namespace WONAPI;


//----------------------------------------------------------------------------------
// ConfigProxyCtrl Constructor.
//----------------------------------------------------------------------------------
ConfigProxyCtrl::ConfigProxyCtrl(void)
	: WizardCtrl()
{
	AddControls();
}

//----------------------------------------------------------------------------------
// ConfigProxyCtrl Destructor.
//----------------------------------------------------------------------------------
ConfigProxyCtrl::~ConfigProxyCtrl(void)
{
}

//----------------------------------------------------------------------------------
// LoadProxySettings: Fetch the proxy settings from the API (its buried deep in 
// there).
//----------------------------------------------------------------------------------
void ConfigProxyCtrl::LoadProxySettings(void)
{
	HTTPGetOp::ReadProxyHostFile();
	std::string sProxy = HTTPGetOp::GetStaticProxyHost();
	
	if (sProxy == "")
	{
		m_pUseProxyCheckBox->SetCheck(false);
		m_pHostInputBox->SetText("");
		m_pPortInputBox->SetText("");
	}
	else
	{
		std::string sHost = sProxy;
		std::string sPort = sProxy;

		char nPos = sHost.rfind(':');
		if (nPos != -1)
		{
			sHost.erase(nPos, sHost.length() - nPos);
			sPort.erase(0, nPos + 1);
		}
		else
			sPort = "80";

		m_pUseProxyCheckBox->SetCheck(true);
		m_pHostInputBox->SetText(sHost);
		m_pPortInputBox->SetText(sPort);
	}
}

//----------------------------------------------------------------------------------
// SaveProxySettings: Stuff the proxy setting back into the API (which will save 
// them for us).
//----------------------------------------------------------------------------------
void ConfigProxyCtrl::SaveProxySettings(void)
{
	std::string sHost;
	std::string sPort;
	
	if (m_pUseProxyCheckBox->IsChecked())
	{
		sHost = m_pHostInputBox->GetText();
		sPort = m_pPortInputBox->GetText();
	}

	std::string sProxy = sHost;
	if (sPort != "")
		sProxy += ":" + sPort;

	HTTPGetOp::WriteProxyHostFile(sProxy);
}

//----------------------------------------------------------------------------------
// AddControls: Initialize all of the sub-controls.
//----------------------------------------------------------------------------------
void ConfigProxyCtrl::AddControls(void)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();
	DisplayContext* pContext = GetWindow()->GetDisplayContext();

	// Background.
	NativeImagePtr pBackroundImg = pResMgr->GetBackgroundImage(IDB_CONFIG_PROXY_DLG_BACKGROUND);
	ImageComponentPtr pBackground = new ImageComponent(pBackroundImg, true);
	AddChildLayout(pBackground, CLI_SameSize, this);

	// Help Button (Navigation Control).
	m_pHelpButton = new MSButton(pResMgr->GetString(IDS_CONFIG_PROXY_DLG_HELP));
	m_pHelpButton->SetDesiredSize();
	AddChildLayout(m_pHelpButton, CLI_SameBottom | CLI_SameLeft, this, NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Ok Button (Navigation Control).
	m_pOkButton = new MSButton(pResMgr->GetString(IDS_COMMON_DLG_OK));
	m_pOkButton->SetDesiredSize();
	AddChildLayout(m_pOkButton, CLI_SameBottom | CLI_SameRight, this, -NAV_BTN_HORIZ_MARGIN, -NAV_BTN_VERT_MARGIN, 0, 0);

	// Cancel (Close) Button (Navigation Control).
	m_pCancelButton = new MSButton(pResMgr->GetString(IDS_COMMON_DLG_CANCEL));
	m_pCancelButton->SetDesiredSize();
	AddChildLayout(m_pCancelButton, CLI_SameBottom | CLI_Left, m_pOkButton, -NAV_BTN_SPACING, 0, 0, 0);

	// Separator Bar (Navigation Control).
	MSSeperatorPtr pSeparator = new MSSeperator(SEP_BAR_HEIGHT);
	AddChildLayout(pSeparator, CLI_Above, m_pOkButton, 0, -SEP_BAR_VERT_MARGIN, 0, 0);
	AddChildLayout(pSeparator, CLI_SameLeft | CLI_GrowToRight, this, SEP_BAR_HORIZ_MARGIN, 0, -SEP_BAR_HORIZ_MARGIN, 0);

	// Game Logo (Dialog-Specific Control).
	NativeImagePtr pBrandImg = pResMgr->GetGameLogoImage(IDB_CONFIG_PROXY_DLG_GAME_LOGO);
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

	// Proxy Port Label (Dialog-Specific Control).
	m_pPortLabel = new MSLabel;
	m_pPortLabel->SetText(pResMgr->GetString(IDS_CONFIG_PROXY_DLG_PORT));
	m_pPortLabel->SetDesiredSize();
	m_pPortLabel->SetTransparent(true);	
	AddChildLayout(m_pPortLabel, CLI_Right, pBrandFrame, STD_HORIZ_MARGIN, 0, 0, 0);

	// Proxy Host Label (Dialog-Specific Control).
	m_pHostLabel = new MSLabel;
	m_pHostLabel->SetText(pResMgr->GetString(IDS_CONFIG_PROXY_DLG_HOST));
	m_pHostLabel->SetDesiredSize();
	m_pHostLabel->SetTransparent(true);	
	AddChildLayout(m_pHostLabel, CLI_Right, pBrandFrame, STD_HORIZ_MARGIN, 0, 0, 0);

	// Proxy Port Input Box (Dialog-Specific Control).
	m_pPortInputBox = new MSInputBox();
	AddChildLayout(m_pPortInputBox, CLI_SameHeight, m_pHostLabel, 0, 0, 0, INPUT_VERT_SPACING * 2);
	AddChildLayout(m_pPortInputBox, CLI_Above, pSeparator, 0, -STD_VERT_MARGIN, 0, 0);
	AddChildLayout(m_pPortInputBox, CLI_GrowToRight, m_pOkButton, 0, 0, -STD_HORIZ_MARGIN, 0);

	// Proxy Host Input Box (Dialog-Specific Control).
	m_pHostInputBox = new MSInputBox();
	AddChildLayout(m_pHostInputBox, CLI_SameHeight, m_pPortInputBox, 0, 0, 0, 0);
	AddChildLayout(m_pHostInputBox, CLI_Above, m_pPortInputBox, 0, -INPUT_VERT_SPACING, 0, 0);
	AddChildLayout(m_pHostInputBox, CLI_GrowToRight, m_pPortInputBox, 0, 0, 0, 0);

	// Line up the Input Boxes.
	AddChildLayout((new ClearRightLayout(m_pHostInputBox, INPUT_HORIZ_SPACING))->Add(m_pHostLabel)->Add(m_pPortLabel));
	AddChildLayout(m_pPortInputBox, CLI_SameLeft, m_pHostInputBox, 0, 0, 0, 0);

	// Adjust the top of the Labels to line up with the input boxes.
	AddChildLayout(m_pHostLabel, CLI_SameTop, m_pHostInputBox, 0, INPUT_VERT_OFFSET, 0, 0);
	AddChildLayout(m_pPortLabel, CLI_SameTop, m_pPortInputBox, 0, INPUT_VERT_OFFSET, 0, 0);

	// Have Proxy Check Box (Dialog-Specific Control).
	m_pUseProxyCheckBox = new MSCheckbox;
	m_pUseProxyCheckBox->IncrementHorzTextPad(CHECKBOX_EXTRA_SPACING, 0);
	m_pUseProxyCheckBox->SetText(pResMgr->GetString(IDS_CONFIG_PROXY_DLG_USE_PROXY));
	m_pUseProxyCheckBox->SetDesiredSize();
	m_pUseProxyCheckBox->SetComponentFlags(ComponentFlag_GrabBG, true);
	m_pUseProxyCheckBox->SetTransparent(true);
//	mRememberPasswordCheck = aRememberPassword;
//	mRememberPasswordCheck->SetControlId(ID_RememberPassword);
	AddChildLayout(m_pUseProxyCheckBox, CLI_Above, m_pHostInputBox, 0, -INPUT_VERT_SPACING, 0, 0);
	AddChildLayout(m_pUseProxyCheckBox, CLI_Right, pBrandFrame, 0, 0, 0, 0);

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
	AddChildLayout(pInfoBox, CLI_GrowToRight, m_pOkButton, 0, 0, 0, 0);
	AddChildLayout(pInfoBox, CLI_GrowToTop, m_pUseProxyCheckBox, 0, 0, 0, -STD_VERT_MARGIN);

	// Add the controls in the desired tab order.
	AddChild(pBackground);
	AddChild(pBrandLogo);
	AddChild(pInfoBox);
	AddChild(m_pUseProxyCheckBox);
	AddChild(m_pHostLabel);
	AddChild(m_pHostInputBox);
	AddChild(m_pPortLabel);
	AddChild(m_pPortInputBox);
	AddChild(pSeparator);
	AddChild(m_pHelpButton);
	AddChild(m_pCancelButton);
	AddChild(m_pOkButton);

	LoadProxySettings();

	EnableControls();
}

//----------------------------------------------------------------------------------
// EnableControls: Enable or disable the controls as aproproiate.
//----------------------------------------------------------------------------------
void ConfigProxyCtrl::EnableControls(void)
{
	bool bUseProxy = m_pUseProxyCheckBox->IsChecked();

	m_pHostLabel->Enable(bUseProxy);
	m_pHostInputBox->Enable(bUseProxy);
	m_pPortLabel->Enable(bUseProxy);
	m_pPortInputBox->Enable(bUseProxy);
}

//----------------------------------------------------------------------------------
// Show: Make this dialog visible (or invisible), and start (or stop) any threads 
// that are needed.
//----------------------------------------------------------------------------------
void ConfigProxyCtrl::Show(bool bShow)
{
	SetVisible(bShow);

	if (bShow)
	{
		// Some of the text changes dynamically based on patch selection, update it.
		ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

		// Informational (Main) text.
		GUIString sInfo = pResMgr->BuildInfoString(IDS_CONFIG_PROXY_DLG_INFO_1);
		m_pInfoText->Clear();
		m_pInfoText->AddFormatedText(sInfo);
		m_pInfoText->SetVertOffset(0); // Scroll to the top.

		// Make sure the Next button is the default button.
		m_pOkButton->RequestFocus();
	}

	// No threads to worry about in this dialog.
}

//----------------------------------------------------------------------------------
// HandleUseProxyCheckBox: Process a click on the Use Proxt Check Box.
//----------------------------------------------------------------------------------
bool ConfigProxyCtrl::HandleUseProxyCheckBox(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	EnableControls();

	return true;
}

//----------------------------------------------------------------------------------
// HandleHelpButton: Process a click on the Help Button.
//----------------------------------------------------------------------------------
bool ConfigProxyCtrl::HandleHelpButton(ComponentEvent* pEvent)
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
// HandleCancelButton: Process a click on the Cancel Button.
//----------------------------------------------------------------------------------
bool ConfigProxyCtrl::HandleCancelButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowPreviousScreen();

	return true;
}

//----------------------------------------------------------------------------------
// HandleOkButton: Process a click on the Ok Button.
//----------------------------------------------------------------------------------
bool ConfigProxyCtrl::HandleOkButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return false;

	// Save the user's settings.
	SaveProxySettings();

	WONUpdateCtrl* pUpdateControl = reinterpret_cast<WONUpdateCtrl*>(GetParent());
	assert(pUpdateControl);
	pUpdateControl->ShowPreviousScreen();

	return true;
}

//----------------------------------------------------------------------------------
// HandleComponentEvent: Handle Component Events (Button pushes and the like).
//----------------------------------------------------------------------------------
void ConfigProxyCtrl::HandleComponentEvent(ComponentEvent* pEvent)
{
	if ((pEvent->mComponent == m_pUseProxyCheckBox && HandleUseProxyCheckBox(pEvent)) ||
		(pEvent->mComponent == m_pHelpButton && HandleHelpButton(pEvent)) ||
		(pEvent->mComponent == m_pCancelButton && HandleCancelButton(pEvent)) ||
		(pEvent->mComponent == m_pOkButton && HandleOkButton(pEvent)))
	{
		return;
	}

	Container::HandleComponentEvent(pEvent);
}
