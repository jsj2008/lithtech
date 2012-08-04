// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ButeEdit.h"

#include "MainFrm.h"
#include "LeftView.h"
#include "ButeEditView.h"
#include "ButeEditDoc.h"

#include "ModifyBoolDlg.h"
#include "ModifyByteDlg.h"
#include "ModifyDoubleDlg.h"
#include "ModifyDWordDlg.h"
#include "ModifyFloatDlg.h"
#include "ModifyIntegerDlg.h"
#include "ModifyPointDlg.h"
#include "ModifyRangeDlg.h"
#include "ModifyRectangleDlg.h"
#include "ModifyStringDlg.h"
#include "ModifyVectorDlg.h"
#include "NewTagDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, OnUpdateViewStyles)
	ON_COMMAND_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, OnViewStyle)

	ON_COMMAND_RANGE(ID_EDIT_NEW_STRING, ID_EDIT_NEW_TAG, OnNewTagKeyCommand)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_NEW_STRING, ID_EDIT_NEW_TAG, OnUpdateNewTagKeyCommands)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftView), CSize(300, 100), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CButeEditView), CSize(100, 100), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

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

CButeEditView* CMainFrame::GetRightPane()
{
	CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
	CButeEditView* pView = DYNAMIC_DOWNCAST(CButeEditView, pWnd);
	return pView;
}

CLeftView* CMainFrame::GetLeftPane()
{
	CWnd* pWnd = m_wndSplitter.GetPane(0, 0);
	CLeftView* pView = DYNAMIC_DOWNCAST(CLeftView, pWnd);
	return pView;
}

void CMainFrame::OnUpdateViewStyles(CCmdUI* pCmdUI)
{
	// TODO: customize or extend this code to handle choices on the
	// View menu.

	CButeEditView* pView = GetRightPane(); 

	// if the right-hand pane hasn't been created or isn't a view,
	// disable commands in our range

	if (pView == NULL)
		pCmdUI->Enable(FALSE);
	else
	{
		DWORD dwStyle = pView->GetStyle() & LVS_TYPEMASK;

		// if the command is ID_VIEW_LINEUP, only enable command
		// when we're in LVS_ICON or LVS_SMALLICON mode

		if (pCmdUI->m_nID == ID_VIEW_LINEUP)
		{
			if (dwStyle == LVS_ICON || dwStyle == LVS_SMALLICON)
				pCmdUI->Enable();
			else
				pCmdUI->Enable(FALSE);
		}
		else
		{
			// otherwise, use dots to reflect the style of the view
			pCmdUI->Enable();
			BOOL bChecked = FALSE;

			switch (pCmdUI->m_nID)
			{
			case ID_VIEW_DETAILS:
				bChecked = (dwStyle == LVS_REPORT);
				break;

			case ID_VIEW_SMALLICON:
				bChecked = (dwStyle == LVS_SMALLICON);
				break;

			case ID_VIEW_LARGEICON:
				bChecked = (dwStyle == LVS_ICON);
				break;

			case ID_VIEW_LIST:
				bChecked = (dwStyle == LVS_LIST);
				break;

			default:
				bChecked = FALSE;
				break;
			}

			pCmdUI->SetRadio(bChecked ? 1 : 0);
		}
	}
}


void CMainFrame::OnViewStyle(UINT nCommandID)
{
	// TODO: customize or extend this code to handle choices on the
	// View menu.
	CButeEditView* pView = GetRightPane();

	// if the right-hand pane has been created and is a CButeEditView,
	// process the menu commands...
	if (pView != NULL)
	{
		DWORD dwStyle = -1;

		switch (nCommandID)
		{
		case ID_VIEW_LINEUP:
			{
				// ask the list control to snap to grid
				CListCtrl& refListCtrl = pView->GetListCtrl();
				refListCtrl.Arrange(LVA_SNAPTOGRID);
			}
			break;

		// other commands change the style on the list control
		case ID_VIEW_DETAILS:
			dwStyle = LVS_REPORT;
			break;

		case ID_VIEW_SMALLICON:
			dwStyle = LVS_SMALLICON;
			break;

		case ID_VIEW_LARGEICON:
			dwStyle = LVS_ICON;
			break;

		case ID_VIEW_LIST:
			dwStyle = LVS_LIST;
			break;
		}

		// change the style; window will repaint automatically
		if (dwStyle != -1)
			pView->ModifyStyle(LVS_TYPEMASK, dwStyle);
	}
}

/************************************************************************/
// One of the "new" commands was selected
void CMainFrame::OnNewTagKeyCommand(UINT nCommandID)
{
	// Get the document
	CButeEditDoc* pDoc = (CButeEditDoc*)GetActiveDocument();
	ASSERT_VALID(pDoc);

	// Get the bute manager
	CButeMgr *pButeMgr=pDoc->GetButeMgr();

	// Get the selected tag
	CString sTag=pDoc->GetSelectedTag();

	// Indicates if the key has been added
	BOOL bAdded=FALSE;

	// The name of the new value
	int nValueIndex=1;
	CString sNewValueName="NewValue1";

	// Make sure that the new value name is unique
	while (pButeMgr->Exist(sTag, sNewValueName))
	{
		nValueIndex++;
		sNewValueName.Format("NewValue%d", nValueIndex);
	}

	// Modify the key depending on the type
	switch (nCommandID)
	{
	case ID_EDIT_NEW_TAG:
		{
			// Create the tag dialog
			CNewTagDlg tagDlg;
			if (tagDlg.DoModal() == IDOK)
			{
				// Add the tag
				pDoc->AddTag(tagDlg.m_sName);
				GetLeftPane()->BuildTreeControl();

				// Select the tag
				pDoc->SetSelectedTag(tagDlg.m_sName);
				GetLeftPane()->SelectTag(tagDlg.m_sName);
			}			
			break;
		}
	case ID_EDIT_NEW_INTEGER:
		{
			// Modify the integer type
			CModifyIntegerDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				pButeMgr->SetInt(sTag, modifyDlg.m_sName, modifyDlg.m_nValue);
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	case ID_EDIT_NEW_DWORD:
		{
			// Modify the DWORD type
			CModifyDWordDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				pButeMgr->SetDword(sTag, modifyDlg.m_sName, modifyDlg.m_dwValue);
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	case ID_EDIT_NEW_BYTE:
		{
			// Modify the BYTE type
			CModifyByteDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				pButeMgr->SetByte(sTag, modifyDlg.m_sName, modifyDlg.m_nValue);
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	case ID_EDIT_NEW_BOOL:
		{	
			// Modify the BOOL type
			CModifyBoolDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				pButeMgr->SetBool(sTag, modifyDlg.m_sName, modifyDlg.GetValue());
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	case ID_EDIT_NEW_DOUBLE:
		{
			// Modify the double type
			CModifyDoubleDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				pButeMgr->SetDouble(sTag, modifyDlg.m_sName, modifyDlg.m_fValue);
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	case ID_EDIT_NEW_FLOAT:
		{
			// Modify the float type
			CModifyFloatDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				pButeMgr->SetFloat(sTag, modifyDlg.m_sName, modifyDlg.m_fValue);
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	case ID_EDIT_NEW_STRING:
		{
			// Modify the string type
			CModifyStringDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);
			
			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				// Convert \r and \n to \\r and \\n
				modifyDlg.m_sValue.Replace("\r", "\\r");
				modifyDlg.m_sValue.Replace("\n", "\\n");
				
				pButeMgr->SetString(sTag, modifyDlg.m_sName, modifyDlg.m_sValue);
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	case ID_EDIT_NEW_RECT:
		{
			// Modify the rectangle type
			CModifyRectangleDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				pButeMgr->SetRect(sTag, modifyDlg.m_sName, modifyDlg.GetRect());
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	case ID_EDIT_NEW_POINT:
		{
			// Modify the point type				
			CModifyPointDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				pButeMgr->SetPoint(sTag, modifyDlg.m_sName, modifyDlg.GetPoint());
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	case ID_EDIT_NEW_VECTOR:
		{
			// Modify the vector type			
			CModifyVectorDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				pButeMgr->SetVector(sTag, modifyDlg.m_sName, modifyDlg.GetVector());
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	case ID_EDIT_NEW_RANGE:
		{
			// Modify the range type
			CModifyRangeDlg modifyDlg;
			modifyDlg.m_sName=sNewValueName;
			modifyDlg.EnableModifyName(TRUE);

			// Display the dialog
			if (modifyDlg.DoModal() == IDOK)
			{
				pButeMgr->SetRange(sTag, modifyDlg.m_sName, modifyDlg.GetRange());
				ASSERT(pButeMgr->Success());
				bAdded=TRUE;
			}
			break;
		}
	default:
		{
			// The switch statement should have been handled in one of the cases above!
			ASSERT(FALSE);
		}
	}

	// Set the modified flag
	if (bAdded)
	{
		pDoc->SetModifiedFlag();
		pDoc->UpdateAllViews(NULL);
	}
}

/************************************************************************/
// The command update handler for the "new" commands
void CMainFrame::OnUpdateNewTagKeyCommands(CCmdUI* pCmdUI)
{
	// Get the document
	CButeEditDoc* pDoc = (CButeEditDoc*)GetActiveDocument();

	// Indicates if the command should be enabled
	BOOL bEnable=FALSE;

	// Update the command based on its ID
	switch (pCmdUI->m_nID)
	{	
	case ID_EDIT_NEW_TAG:
		{
			bEnable=TRUE;
			break;
		}
	case ID_EDIT_NEW_STRING:
	case ID_EDIT_NEW_BYTE:
	case ID_EDIT_NEW_DWORD:
	case ID_EDIT_NEW_INTEGER:
	case ID_EDIT_NEW_FLOAT:
	case ID_EDIT_NEW_DOUBLE:
	case ID_EDIT_NEW_BOOL:
	case ID_EDIT_NEW_POINT:
	case ID_EDIT_NEW_RECT:
	case ID_EDIT_NEW_VECTOR:
	case ID_EDIT_NEW_RANGE:
		{
			// See if there is a selected tag
			if (pDoc->GetSelectedTag().GetLength() > 0)
			{
				bEnable=TRUE;
			}
			break;
		}
	default:
		{
			ASSERT(FALSE);
		}
	}

	// Update the command ID
	pCmdUI->Enable(bEnable);
}

/************************************************************************/
// Updates the status bar text with the currently selected tag/key
void CMainFrame::UpdateStatusBarText()
{
	// Get the document
	CButeEditDoc* pDoc = (CButeEditDoc*)GetActiveDocument();
	ASSERT_VALID(pDoc);

	// Get the list control
	CListCtrl &listControl=GetRightPane()->GetListCtrl();

	// Get the currently selected key
	POSITION pos = listControl.GetFirstSelectedItemPosition();
	if (pos)
	{
		// Get the selected index
		int nIndex=listControl.GetNextSelectedItem(pos);

		// Get the selected item text
		CString sSelectedKey=listControl.GetItemText(nIndex, 0);

		// Set the message text
		SetMessageText(pDoc->GetSelectedTag()+"\\"+sSelectedKey);
	}
	else
	{
		SetMessageText("Ready");
	}
}
