// ProjectTabControlBar.cpp: implementation of the CProjectTabControlBar class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "mrcext.h"
#include "mainfrm.h"
#include "projectbar.h"
#include "projecttabcontrolbar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProjectTabControlBar::CProjectTabControlBar()
{

}

CProjectTabControlBar::~CProjectTabControlBar()
{

}

/************************************************************************/
// This gets called when the "hide" button is pressed
void CProjectTabControlBar::OnButtonHide()
{
	HWND hBar=GetSafeHwnd();

	// Get the project bar
	CProjectBar *pProjectBar=GetMainFrame()->GetProjectBar();

	// Get the frame window
	CFrameWnd* pFrameWnd = (CFrameWnd*)m_pDockSite; //GetParentFrame();
	ASSERT(pFrameWnd);

	// Make sure that the control bar gets hidden
	if (!pProjectBar->IsFloating())
	{		
		pFrameWnd->ShowControlBar(this, FALSE, FALSE);
	}

	// Properties dialog
	if (hBar == GetPropertiesDlg()->GetSafeHwnd())
	{		
		pFrameWnd->FloatControlBar(this, CPoint(0,0));
		pProjectBar->DockTab(this, CMainFrame::CB_PROPERTIESVIEW);
		return;
	}
	// Class dialog
	else if (hBar == GetClassListDlg()->GetSafeHwnd())
	{
		pFrameWnd->FloatControlBar(this, CPoint(0,0));
		pProjectBar->DockTab(this, CMainFrame::CB_CLASSVIEW);
		return;
	}
	// Texture dialog
	else if (hBar == GetTextureDlg()->GetSafeHwnd())
	{
		pFrameWnd->FloatControlBar(this, CPoint(0,0));
		pProjectBar->DockTab(this, CMainFrame::CB_TEXTUREVIEW);
		return;
	}
	// Model dialog
	else if (hBar == GetModelDlg()->GetSafeHwnd())
	{
		pFrameWnd->FloatControlBar(this, CPoint(0,0));
		pProjectBar->DockTab(this, CMainFrame::CB_MODELVIEW);
		return;
	}
	// Worlds dialog
	else if (hBar == GetWorldsDlg()->GetSafeHwnd())
	{
		pFrameWnd->FloatControlBar(this, CPoint(0,0));
		pProjectBar->DockTab(this, CMainFrame::CB_WORLDSVIEW);
		return;
	}
	// Sound dialog
	else if (hBar == GetSoundDlg()->GetSafeHwnd())
	{
		pFrameWnd->FloatControlBar(this, CPoint(0,0));
		pProjectBar->DockTab(this, CMainFrame::CB_SOUNDVIEW);
		return;
	}
	// Sprite dialog
	else if (hBar == GetSpriteDlg()->GetSafeHwnd())
	{
		pFrameWnd->FloatControlBar(this, CPoint(0,0));
		pProjectBar->DockTab(this, CMainFrame::CB_SPRITEVIEW);
		return;
	}
	// Nodeview dialog
	else if (hBar == GetNodeView()->GetSafeHwnd())
	{
		pFrameWnd->FloatControlBar(this, CPoint(0,0));
		pProjectBar->DockTab(this, CMainFrame::CB_NODESVIEW);
		return;
	}
	else if (hBar == GetPrefabDlg()->GetSafeHwnd())
	{
		pFrameWnd->FloatControlBar(this, CPoint(0,0));
		pProjectBar->DockTab(this, CMainFrame::CB_PREFABVIEW);
		return;
	}

	CMRCSizeControlBar::OnButtonHide();
}