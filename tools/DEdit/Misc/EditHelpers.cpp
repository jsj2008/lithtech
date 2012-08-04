//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditHelpers.cpp
//
//	PURPOSE	  :
//
//	CREATED	  : December 14 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "dedit.h"
#include "edithelpers.h"
#include "editprojectmgr.h"
#include "projectbar.h"
#include "regiondoc.h"
#include "nodeview.h"
#include "regionview.h"
#include "mainfrm.h"
#include "levelerrordlg.h"
#include "renameresourcedlg.h"
#include "leveltexturesdlg.h"
#include "objectsearchdlg.h"
#include "levelitemsdlg.h"
#include "objectselfilterdlg.h"

static CColorSelectDlg		g_ColorSelectDlg;
static CLevelErrorDlg		g_LevelErrorDlg;
static CLevelItemsDlg		g_LevelItemsDlg;
static CLevelTexturesDlg	g_LevelTexturesDlg;
static CObjectSearchDlg		g_ObjectSearchDlg;
static CRenameResourceDlg	g_RenameResourceDlg(NULL);
static CObjectSelFilterDlg	g_ObjectSelFilterDlg;
static uint32				g_nCurrTexture = 0;
CMainFrame					*g_pMainFrame=NULL;


CDEditApp* GetApp()
{
	return (CDEditApp*)AfxGetApp();
}


// So code using the console can work...
void con_PrintString(unsigned long color, int debugLevel, char *pMsg)
{
	GetDebugDlg()->AddMessage(pMsg);
}


void AddDebugMessage(char *pMsg, ...)
{
	va_list marker;

	va_start(marker, pMsg);
	GetDebugDlg()->AddMessage2(pMsg, marker);
	va_end(marker);
}

CDebugDlg* GetDebugDlg()
{
	return &GetProjectBar()->m_DebugDlg;
}

CPropertiesDlg* GetPropertiesDlg()
{
	return GetMainFrame()->GetPropertiesDlg();	
}

CNodeView * GetNodeView( )
{
	return GetMainFrame()->GetNodeView();
}

ClassListDlg* GetClassListDlg()
{
	return GetMainFrame()->GetClassListDlg();
}

CTextureDlg* GetTextureDlg()
{
	return GetMainFrame()->GetTextureDlg();
}

CModelDlg* GetModelDlg()
{
	return GetMainFrame()->GetModelDlg();
}

CWorldsDlg*	GetWorldsDlg()
{
	return GetMainFrame()->GetWorldsDlg();
}

CSoundDlg*	GetSoundDlg()
{
	return GetMainFrame()->GetSoundDlg();
}

CSpriteDlg*	GetSpriteDlg()
{
	return GetMainFrame()->GetSpriteDlg();
}

CPrefabDlg* GetPrefabDlg()
{
	return GetMainFrame()->GetPrefabDlg();
}

CColorSelectDlg* GetColorSelectDlg()
{
	return &g_ColorSelectDlg;
}

CLevelErrorDlg* GetLevelErrorDlg()
{
	return &g_LevelErrorDlg;
}

CLevelItemsDlg* GetLevelItemsDlg()
{
	return &g_LevelItemsDlg;
}

CLevelTexturesDlg* GetLevelTexturesDlg()
{
	return &g_LevelTexturesDlg;
}

CObjectSearchDlg* GetObjectSearchDlg()
{
	return &g_ObjectSearchDlg;
}

CObjectSelFilterDlg* GetObjectSelFilterDlg()
{
	return &g_ObjectSelFilterDlg;
}

CRenameResourceDlg* GetRenameResourceDlg()
{
	return &g_RenameResourceDlg;
}


CMainFrame* GetMainFrame()
{
	return g_pMainFrame;
}


CProjectBar* GetProjectBar()
{
	return GetMainFrame()->GetProjectBar();
}


CEditProjectMgr* GetProject()
{
	return GetMainFrame()->GetProjectBar()->GetProject();
}


struct DFileMgr_t* GetFileMgr()
{
	return g_pMainFrame->GetProjectBar()->GetProject()->m_hFileMgr;
}

CRegionDoc* GetActiveRegionDoc()
{
	CProjectBar *pBar;
	CView *pView;
	CFrameWnd *pFrameWnd;
	CRegionDoc *pDoc;

	// If a new document is currently being opened don't return the active one
	if( GetProjectBar()->m_pNewRegionDoc )
		return GetProjectBar()->m_pNewRegionDoc;

	if((pFrameWnd = GetMainFrame()->GetActiveFrame()) &&
		(pView = pFrameWnd->GetActiveView()) &&
			(pView->IsKindOf(RUNTIME_CLASS(CRegionView))))
	{
		pDoc = ((CRegionView*)pView)->GetRegionDoc();
		return pDoc;
	}
	
	return NULL;
}

// Redraws all of the documents
void RedrawAllDocuments()
{
	CMultiDocTemplate *pTemplate=GetApp()->m_pWorldTemplate;

	POSITION pos=pTemplate->GetFirstDocPosition();
	while (pos)
	{
		CRegionDoc *pDoc=(CRegionDoc *)pTemplate->GetNextDoc(pos);
		pDoc->RedrawAllViews();
	}	
}

int AppMessageBox( UINT idString, UINT nType )
{
	CString		str;
	
	str.LoadString( idString );
	return AfxGetMainWnd()->MessageBox( str, AfxGetAppName(), nType );
}


int AppMessageBox( const char *pStr, UINT nType )
{
	return AfxGetMainWnd()->MessageBox( pStr, AfxGetAppName(), nType );
}

//Gets which texture layer is currently being manipulated
uint32 GetCurrTexture()
{
	return g_nCurrTexture;		
}

void SetCurrTexture(uint32 nCurrTex)
{
	//make sure that this is within range
	if((nCurrTex < CEditPoly::NUM_TEXTURES) && (g_nCurrTexture != nCurrTex))
	{
		//save this value
		g_nCurrTexture = nCurrTex;

		//redraw all documents
		RedrawAllDocuments();
	}
}



CHelperFileDlg::CHelperFileDlg(
	BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName, 
	DWORD dwFlags, LPCTSTR lpszFilter, 
	CWnd* pParentWnd )
: CFileDialog( bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd )
{
	m_szFileName[0] = '\0';
}

int CHelperFileDlg::DoModal()
{
	//DON'T DO THIS....YOU NEED TO SET m_szFileName...besides the constructor already does this
//	m_ofn.lpstrFile = m_szFileName;
	m_ofn.nMaxFile = 50000;

	return CFileDialog::DoModal();
}





