//----------------------------------------------------------------------------------
// SierraUp.cpp : Defines the entry point for the application.
//----------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <assert.h>
#include "WONGUI/GUICompat.h"
#include "WONGUI/ChildLayouts.h"
#include "AboutDlg.h"
#include "Resource.h"
#include "../WONUpdateCtrl.h"
#include "../AbortDlg.h"
#include "../CustomInfo.h"

using namespace WONAPI;

//----------------------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------------------
const int IDM_ABOUTBOX = 0x0110;

//----------------------------------------------------------------------------------
// Global Variables.
//----------------------------------------------------------------------------------
PlatformWindowManagerPtr g_pWindowMgr;
PlatformWindowPtr        g_pWindow;
ContainerPtr             g_pMainScreen;
AboutDlgPtr              g_pAboutDlg;
WNDPROC                  g_pGUIProc = NULL;


/*
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
static void HandleMinimizeButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return;
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
static void HandleCloseButton(ComponentEvent* pEvent)
{
	if (pEvent->mType != ComponentEvent_ButtonPressed)
		return;
}
*/

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
void ShowScreen(Container* pScreen)
{
	g_pWindow->EndPopup();
	g_pWindow->GetRoot()->RemoveAllChildren();
	pScreen->SetPosSize(0, 0, g_pWindow->GetRoot()->Width(), g_pWindow->GetRoot()->Height());
	pScreen->SetLayout(new LayoutManager(0, 0, 100, 100));
	g_pWindow->AddComponent(pScreen);
//	if (pScreen==gLoginScreen)
//		gLoginCtrl->RequestFocus();
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
void CreateMainScreen(void)
{
	int nWidth = g_pWindow->GetRoot()->Width();
	int nHeight = g_pWindow->GetRoot()->Height();
	g_pMainScreen = new Container;
	g_pMainScreen->SetSize(nWidth, nHeight);

/*
	// Close Button (System Control).
	MSCloseButtonPtr pSysCloseButton = new MSCloseButton;
	pSysCloseButton->SetSize(SYS_BTN_WIDTH, SYS_BTN_HEIGHT);
	pSysCloseButton->SetComponentFlags(ComponentFlag_WantFocus, false);
	pSysCloseButton->SetListener(new ComponentListenerFunc(::HandleCloseButton));
	AddChildLayout(pSysCloseButton, CLI_SameTop | CLI_SameRight, this, -SYS_BTN_HORIZ_MARGIN, SYS_BTN_VERT_MARGIN, 0, 0);

	// Minimize Button (System Control).
	MSArrowButtonPtr pSysMinimizeButton = new MSArrowButton(MSArrowButton::Direction_Down);
	pSysMinimizeButton->SetSize(SYS_BTN_WIDTH, SYS_BTN_HEIGHT);
	pSysMinimizeButton->SetComponentFlags(ComponentFlag_WantFocus, false);
	pSysMinimizeButton->SetListener(new ComponentListenerFunc(::HandleMinimizeButton));
	AddChildLayout(pSysMinimizeButton, CLI_SameTop | CLI_Left, pSysCloseButton, -SYS_BTN_SPACING, 0, 0, 0);
*/

	WONUpdateCtrlPtr pCtrl = new WONUpdateCtrl();
	g_pMainScreen->AddChildLayout(pCtrl, CLI_SameSize, g_pMainScreen);

/*
	AddChild(pSysMinimizeButton);
	AddChild(pSysCloseButton);
*/
	g_pMainScreen->AddChild(pCtrl);
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
void KeyPressedFunc(int nKey, int nEventType)
{
	if (g_pWindow->GetWindowManager()->GetPopupOwnerWindow() != NULL)
		return;

	static bool escapeDown = false;
	if (nKey != KEYCODE_ESCAPE || nEventType > 1)
		return;

	if (nEventType == 0 && escapeDown)
		return;

	if (nEventType == 1)
	{
		escapeDown = false;
		return;
	}

	escapeDown = true;

//	if(gLoginScreen->GetParent()!=NULL)
//		g_pWindow->Close();
//	else if(g_pMainScreen->GetParent()!=NULL)
//		ShowScreen(gLoginScreen);
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
void OptionsDlgListener(ComponentEvent* pEvent)
{
	if (pEvent->mType == ComponentEvent_ButtonPressed)
		g_pWindowMgr->EndDialog(pEvent->mComponent->GetControlId());
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
void InitWindow(Window* pWindow)
{
	// Create Window
	WONRectangle aScreenRect;
	g_pWindowMgr->GetScreenRect(aScreenRect);
//	int aCreateFlags = CreateWindow_Centered | CreateWindow_SizeSpecClientArea | CreateWindow_Popup;
	int aCreateFlags = CreateWindow_Centered | CreateWindow_SizeSpecClientArea;
	int aWidth = 460;
	int aHeight = 320;
//	if (gFullScreen)
//	{
//		aWidth = aScreenRect.Width();
//		aHeight = aScreenRect.Height();
//		aCreateFlags |= CreateWindow_Popup;
//	}

	pWindow->SetTitle("EmpireEarth Lobby");
	pWindow->SetCreateFlags(aCreateFlags);
	pWindow->Create(WONRectangle(0, 0, aWidth, aHeight));

	// Make sure the user cannot make it smaller than the original starting size.
	WONRectangle rScreen;
	pWindow->GetScreenPos(rScreen);
	pWindow->SetMinWindowSize(rScreen.Width(), rScreen.Height());
}

//----------------------------------------------------------------------------------
// InitColorScheme: Prepare the Color Scheme for the GUI.
//----------------------------------------------------------------------------------
void InitColorScheme(ColorScheme* pColorScheme)
{
	// ?????
	pColorScheme->SetStandardColor(StandardColor_3DDarkShadow, 0x000000);
	pColorScheme->SetStandardColor(StandardColor_3DFace, 0xEFE7C6);
	pColorScheme->SetStandardColor(StandardColor_3DHilight, 0xFFFFFF);
	pColorScheme->SetStandardColor(StandardColor_3DShadow, 0x848484);
	pColorScheme->SetStandardColor(StandardColor_Scrollbar, 0xFFF6CD);

	pColorScheme->SetStandardColor(StandardColor_ButtonText, 0x000000);
	pColorScheme->SetStandardColor(StandardColor_GrayText, 0x848484);
	pColorScheme->SetStandardColor(StandardColor_Hilight, 0x08246B);
	pColorScheme->SetStandardColor(StandardColor_HilightText, 0xFFFFFF);

	pColorScheme->SetStandardColor(StandardColor_ToolTipBack, 0xFFFFDD);
	pColorScheme->SetStandardColor(StandardColor_ToolTipText, 0x000000);

	pColorScheme->SetStandardColor(StandardColor_MenuBack, 0xD0D0D0);
	pColorScheme->SetStandardColor(StandardColor_MenuText, 0);

	pColorScheme->SetStandardColor(StandardColor_Back, 0xFFFBE7);
	pColorScheme->SetStandardColor(StandardColor_Text, 0x000000);
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
void UpdateSystemMenu(void)
{
	assert((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	assert(IDM_ABOUTBOX < 0xF000);

	// Add 'About...' menu item to system menu.
	HWND hWnd = g_pWindow->GetHWND();
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);

	AppendMenu(hMenu, MF_SEPARATOR, 0, 0L);
	AppendMenu(hMenu, MF_STRING, IDM_ABOUTBOX, "About");
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
void UpdateIcon(void)
{
/*
	// Set the customized icon.
	HICON hIcon = gApp.GetAppInfo().GetIcon(IDI_APP);
	if (hIcon)
	{
		SetIcon(hIcon, TRUE);  // Set big icon.
		SetIcon(hIcon, FALSE); // Set small icon.
	}
*/
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Process the user's selection of the About Menu item (on the System Menu).
	if (uMsg == WM_SYSCOMMAND)
	{
		if ((wParam & 0xFFF0) == IDM_ABOUTBOX)
		{
			CustomInfo* pCI = GetCustomInfo();
			AboutDlgPtr pAboutDlg = new AboutDlg(pCI->GetCurVersion());
			pAboutDlg->DoDialog(g_pWindow);
			return false;
		}
	}

	if (g_pGUIProc)
		return CallWindowProc(g_pGUIProc, hWnd, uMsg, wParam, lParam);

	return false;
}		

//----------------------------------------------------------------------------------
// WONGUI_ENTRY_POINT: Cross platform executable entry point.  AKA WinMain on
// Windows systems.
//----------------------------------------------------------------------------------
WONGUI_ENTRY_POINT
{
	Container::SetMouseWheelGoesToFocusChildDef(false);

//	int anArgc;
//	LPWSTR *anArgv;
//	anArgv = CommandLineToArgvW(GetCommandLineW(),&anArgc);
//	if(anArgc>1)
//		gFullScreen = true;
//
//	GlobalFree(anArgv);

	g_pWindowMgr = new PlatformWindowManager;
	g_pWindow = new PlatformWindow;

	// Set the default resources.
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();
	HMODULE hModule = (HMODULE)g_pWindowMgr->GetHinstance();
	pResMgr->SetDefaultResourceModule(hModule, IDF_SIERRA_UP_WRF);

	// InitColorScheme
	ColorScheme* aColorScheme = g_pWindowMgr->GetColorScheme();
	InitColorScheme(aColorScheme);

	// InitWindow
	InitWindow(g_pWindow);
	g_pWindow->GetRoot()->SetKeyboardListener(new KeyboardListener(KeyPressedFunc));
	CreateMainScreen();
	ShowScreen(g_pMainScreen);

	// Customizations.
	UpdateSystemMenu();
	UpdateIcon();

	HWND hWnd = g_pWindow->GetHWND();
	g_pGUIProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
	SetWindowLong(hWnd, GWL_WNDPROC, (LONG)WindowProc);

	return g_pWindowMgr->MessageLoop();
}