// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ltaview.h"
#include "tdguard.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_SETFOCUS()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(ID_FILECLOSE, OnFileClose)
	ON_COMMAND(ID_FILEOPEN, OnFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILECLOSE, OnUpdateFileClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame():
	m_bDocOpen(FALSE)
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}


	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
	CFrameWnd::SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_TreeMgr.Create(	WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CHILD | WS_BORDER,
						CRect(0, 0, 400, 200),	//arbitrary
						this,
						ID_LTATREE
					);

	m_TreeMgr.AddLTAIcon("file-info",		IDI_FILE, IDI_FILE);
	m_TreeMgr.AddLTAIcon("filename",		IDI_FILE, IDI_FILE);
	m_TreeMgr.AddLTAIcon("provenance",		IDI_FILE, IDI_FILE);
	m_TreeMgr.AddLTAIcon("properties",		IDI_FILE, IDI_FILE);
	m_TreeMgr.AddLTAIcon("proplist",		IDI_FILE, IDI_FILE);
	m_TreeMgr.AddLTAIcon("globalproplist",	IDI_FILE, IDI_FILE);
	m_TreeMgr.AddLTAIcon("versioncode",		IDI_FILE, IDI_FILE);
	m_TreeMgr.AddLTAIcon("type",			IDI_FILE, IDI_FILE);
	m_TreeMgr.AddLTAIcon("hierarchy",		IDI_HIERARCHY, IDI_HIERARCHY);
	m_TreeMgr.AddLTAIcon("childlist",		IDI_HIERARCHY, IDI_HIERARCHY);
	m_TreeMgr.AddLTAIcon("nodehierarchy",	IDI_HIERARCHY, IDI_HIERARCHY);
	m_TreeMgr.AddLTAIcon("children",		IDI_HIERARCHY, IDI_HIERARCHY);
	m_TreeMgr.AddLTAIcon("shape",			IDI_SHAPE, IDI_SHAPE);
	m_TreeMgr.AddLTAIcon("transform",		IDI_TRANSFORM, IDI_TRANSFORM);
	m_TreeMgr.AddLTAIcon("matrix",			IDI_MATRIX, IDI_MATRIX);
	m_TreeMgr.AddLTAIcon("geometry",		IDI_GEOMETRY, IDI_GEOMETRY);
	m_TreeMgr.AddLTAIcon("mesh",			IDI_GEOMETRY, IDI_GEOMETRY);
	m_TreeMgr.AddLTAIcon("vertex",			IDI_VERTEX, IDI_VERTEX);
	m_TreeMgr.AddLTAIcon("navigatorposlist",IDI_VERTEX, IDI_VERTEX);
	m_TreeMgr.AddLTAIcon("pointlist",		IDI_VERTEX, IDI_VERTEX);
	m_TreeMgr.AddLTAIcon("point",			IDI_VERTEX, IDI_VERTEX);
	m_TreeMgr.AddLTAIcon("dist",			IDI_VERTEX, IDI_VERTEX);
	m_TreeMgr.AddLTAIcon("tri-fs",			IDI_TRIFAN, IDI_TRIFAN);
	m_TreeMgr.AddLTAIcon("tex-fs",			IDI_TRIFAN, IDI_TRIFAN);
	m_TreeMgr.AddLTAIcon("nrm-fs",			IDI_NORMALFAN, IDI_NORMALFAN);
	m_TreeMgr.AddLTAIcon("normals",			IDI_NORMAL, IDI_NORMAL);
	m_TreeMgr.AddLTAIcon("normal",			IDI_NORMAL, IDI_NORMAL);
	m_TreeMgr.AddLTAIcon("uvs",				IDI_UV, IDI_UV);
	m_TreeMgr.AddLTAIcon("times",			IDI_TIME, IDI_TIME);
	m_TreeMgr.AddLTAIcon("keyframe",		IDI_KEYFRAME, IDI_KEYFRAME);
	m_TreeMgr.AddLTAIcon("animset",			IDI_ANIMSET, IDI_ANIMSET);
	m_TreeMgr.AddLTAIcon("anims",			IDI_ANIMSET, IDI_ANIMSET);
	m_TreeMgr.AddLTAIcon("target",			IDI_TARGET, IDI_TARGET);
	m_TreeMgr.AddLTAIcon("weightsets",		IDI_WEIGHTSET, IDI_WEIGHTSET);
	m_TreeMgr.AddLTAIcon("add-deformer",	IDI_ADDDEFORMER, IDI_ADDDEFORMER);
	m_TreeMgr.AddLTAIcon("skel-deformer",	IDI_SKELDEFORMER, IDI_SKELDEFORMER);
	m_TreeMgr.AddLTAIcon("influences",		IDI_INFLUENCES, IDI_INFLUENCES);
	m_TreeMgr.AddLTAIcon("posquat",			IDI_POSQUAT, IDI_POSQUAT);
	m_TreeMgr.AddLTAIcon("frames",			IDI_FRAME, IDI_FRAME);
	m_TreeMgr.AddLTAIcon("world",			IDI_WORLD, IDI_FILE);
	m_TreeMgr.AddLTAIcon("worldnode",		IDI_WORLD, IDI_FILE);
	m_TreeMgr.AddLTAIcon("header",			IDI_HEADER, IDI_HEADER);
	m_TreeMgr.AddLTAIcon("polyhedronlist",	IDI_GEOMETRY, IDI_GEOMETRY);
	m_TreeMgr.AddLTAIcon("polyhedron",		IDI_GEOMETRY, IDI_GEOMETRY);
	m_TreeMgr.AddLTAIcon("polyhedron",		IDI_GEOMETRY, IDI_GEOMETRY);
	m_TreeMgr.AddLTAIcon("polylist",		IDI_POLYGON, IDI_POLYGON);
	m_TreeMgr.AddLTAIcon("editpoly",		IDI_POLYGON, IDI_POLYGON);
	m_TreeMgr.AddLTAIcon("basepoly",		IDI_POLYGON, IDI_POLYGON);
	m_TreeMgr.AddLTAIcon("textureinfo",		IDI_UV, IDI_UV);
	m_TreeMgr.AddLTAIcon("flags",			IDI_FLAG, IDI_FLAG);
	m_TreeMgr.AddLTAIcon("shade",			IDI_SHADES, IDI_SHADES);
	m_TreeMgr.AddLTAIcon("color",			IDI_COLOR, IDI_COLOR);
	m_TreeMgr.AddLTAIcon("data",			IDI_DATA, IDI_DATA);
	m_TreeMgr.AddLTAIcon("indices",			IDI_INDEX, IDI_INDEX);



	//set the icon
	m_hTitleIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	return 0;
}

void CMainFrame::OnDestroy() 
{
	m_TreeMgr.DeleteTree();

	CFrameWnd::OnDestroy();
	
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);

	m_TreeMgr.MoveWindow(0, 0, cx, cy);
}

void CMainFrame::OnFileClose() 
{
	BeginWaitCursor();

	//delete the tree
	m_TreeMgr.DeleteTree();
	
	//set the title bar so it doesn't say a filename
	CString sTitle;
	sTitle.LoadString(AFX_IDS_APP_TITLE);

	SetWindowText(sTitle);

	m_bDocOpen = FALSE;

	EndWaitCursor();

}

void CMainFrame::OnFileOpen() 
{
	//load the neccessary strings
	CString sDefaultExt;
	sDefaultExt.LoadString(IDS_DEF_FILE_EXT);

	CString sDefaultFile;
	sDefaultFile.LoadString(IDS_DEF_FILE_NAME);

	CString sFileFilter;
	sFileFilter.LoadString(IDS_FILE_FILTER);

	//create the dialog
	CFileDialog FileDlg(TRUE, sDefaultExt, sDefaultFile, OFN_FILEMUSTEXIST, sFileFilter);

	if(FileDlg.DoModal() == IDOK)
	{

		BeginWaitCursor();

		CDialog WaitDlg;

		WaitDlg.Create(IDD_LOADINGLTA, this);

		//clear out the old tree
		m_TreeMgr.DeleteTree();

		//load in the new tree
		m_TreeMgr.LoadLTA(FileDlg.GetPathName());

		//setup the title to include the filename
		CString sTitle;
		sTitle.LoadString(AFX_IDS_APP_TITLE);
		sTitle += " - ";
		sTitle += FileDlg.GetFileName();

		SetWindowText(sTitle);

		WaitDlg.DestroyWindow();

		EndWaitCursor();

		m_bDocOpen = TRUE;
	}
	
}


void CMainFrame::OpenFileCmdLine ( LPTSTR lpCmdLine )
{
	SetIcon(m_hTitleIcon, FALSE);
		CString sTitle;
		sTitle.LoadString(AFX_IDS_APP_TITLE);
		sTitle += " - ";
		sTitle += lpCmdLine;

		SetWindowText(sTitle);



	BeginWaitCursor();

	CDialog WaitDlg;

	WaitDlg.Create(IDD_LOADINGLTA, this);

	//clear out the old tree
	m_TreeMgr.DeleteTree();

	//load in the new tree
	if ( m_TreeMgr.LoadLTA( lpCmdLine ) )
	{
		//setup the title to include the filename
		CString sTitle;
		sTitle.LoadString(AFX_IDS_APP_TITLE);
		sTitle += " - ";
		sTitle += lpCmdLine;

		SetWindowText(sTitle);

		m_bDocOpen = TRUE;
	}

	WaitDlg.DestroyWindow();

	EndWaitCursor();

	
}


void CMainFrame::OnUpdateFileClose(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bDocOpen);	
}
