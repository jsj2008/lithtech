//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "bdefs.h"
#include "dedit.h"

#include "mainfrm.h"
#include "regiondoc.h"
#include "splash.h"

#include "projectbar.h"
#include "regionview.h"
#include "regionframe.h"
#include "regmgr.h"

#include "resource.h"
#include "optionssheet.h"

#include "advancedselect.h"
#include "advancedselectdlg.h"
#include "sysstreamsim.h"
#include "paletteedit.h"
#include "dedit_concommand.h"
#include "texture.h"
#include <dos.h>
#include <direct.h>
#include <io.h>

#include "optionsrun.h"
#include "propertiesdlg.h"
#include "classlist.h"
#include "texturedlg.h"
#include "modeldlg.h"
#include "worldsdlg.h"
#include "sounddlg.h"
#include "spritedlg.h"
#include "prefabdlg.h"
#include "optionswindows.h"
#include "optionsmodels.h"
#include "optionsmisc.h"
#include "texturedlg.h"
#include "levelerrordlg.h"
#include "renameresourcedlg.h"
#include "prefabtrackerdlg.h"
#include "objecttrackerdlg.h"
#include "texturetrackerdlg.h"
#include "leveltexturesdlg.h"
#include "texturesearchreplacedlg.h"
#include "objectsearchdlg.h"
#include "levelitemsdlg.h"
#include "objectselfilterdlg.h"

//for the Rez file creation
#include "rezutils.h"

//for model rendering
#include "eventnames.h"
#include "modelmgr.h"
#include "meshshapelist.h"
#include "meshshape.h"

//for updating world objects
#include "UpdateAllObjects.h"



extern CMainFrame *g_pMainFrame;

// Defines
#define MODE_MENU_INSERT_POS	8	// The menu position to insert the mode menus at
#define MODE_MENU_BRUSH_INDEX	0	// The index to the brush menu in the IDR_MAINFRAME_MODE_MENUS menu

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMRCMDIFrameWndSizeDock)

BEGIN_MESSAGE_MAP(CMainFrame, CMRCMDIFrameWndSizeDock)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_COMMAND_RANGE(ID_SELECT_TAB_WORLDSVIEW, ID_SELECT_TAB_PREFABVIEW, OnSelectTab)
	ON_COMMAND(ID_RUN_WORLD, OnRunWorld)
	ON_UPDATE_COMMAND_UI(ID_RUN_WORLD, OnUpdateRunWorld)
	ON_COMMAND(ID_SHOW_WORLD_CLASSES, OnShowWorldClasses)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DEBUGWINDOW, OnUpdateViewDebugWindow)
	ON_COMMAND(ID_VIEW_DEBUGWINDOW, OnViewDebugWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTUREPALETTE, OnUpdateViewTexturePalette)
	ON_COMMAND(ID_VIEW_TEXTUREPALETTE, OnViewTexturePalette)
	ON_UPDATE_COMMAND_UI(ID_SINGLE_NODE_SELECTION, OnUpdateSingleNodeSelection)
	ON_UPDATE_COMMAND_UI(ID_MULTI_NODE_SELECTION, OnUpdateMultiNodeSelection)
	ON_COMMAND(ID_SINGLE_NODE_SELECTION, OnSingleNodeSelection)
	ON_COMMAND(ID_MULTI_NODE_SELECTION, OnMultiNodeSelection)
	ON_COMMAND(ID_FILE_CREATEREZFILE, OnCreateRezFile)
	ON_COMMAND(ID_FILE_RELOADPALETTEDAT, OnReloadPalettes)
	ON_COMMAND(ID_FILE_CLOSE, OnClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_CREATEREZFILE, OnUpdateProjectOpen)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_PROJECT_WINDOW, OnViewProjectWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROJECT_WINDOW, OnUpdateViewProjectWindow)
	ON_COMMAND(ID_PREFAB_TRACKER, OnPrefabTracker)
	ON_UPDATE_COMMAND_UI(ID_PREFAB_TRACKER, OnUpdatePrefabTracker)
	ON_COMMAND(ID_OBJECT_TRACKER, OnObjectTracker)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_TRACKER, OnUpdateObjectTracker)
	ON_COMMAND(ID_TEXTURE_TRACKER, OnTextureTracker)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_TRACKER, OnUpdateTextureTracker)
	ON_COMMAND(ID_SEARCH_REPLACE_IN_TEXTURES, OnSearchReplaceInTextures)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_REPLACE_IN_TEXTURES, OnUpdateSearchReplaceInTextures)
	ON_COMMAND(ID_VIEW_COLORSELECTION, OnViewColorSelection)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COLORSELECTION, OnUpdateViewColorSelection)
	ON_COMMAND(ID_VIEW_LEVEL_ERROR, OnViewLevelError)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LEVEL_ERROR, OnUpdateViewLevelError)
	ON_COMMAND(ID_VIEW_LEVEL_ITEMS, OnViewLevelItems)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LEVEL_ITEMS, OnUpdateViewLevelItems)
	ON_COMMAND(ID_VIEW_LEVEL_TEXTURES, OnViewLevelTextures)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LEVEL_TEXTURES, OnUpdateViewLevelTextures)
	ON_COMMAND(ID_VIEW_OBJECT_SEARCH, OnViewObjectSearch)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECT_SEARCH, OnUpdateViewObjectSearch)
	ON_COMMAND(ID_OBJECT_SEL_FILTER, OnObjectSelFilter)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_SEL_FILTER, OnUpdateObjectSelFilter)
	ON_WM_DESTROY()
	ON_COMMAND(ID_GEOMETRY_EDITMODE, OnGeometryEditMode)
	ON_UPDATE_COMMAND_UI(ID_GEOMETRY_EDITMODE, OnUpdateGeometryEditMode)
	ON_COMMAND(ID_OBJECT_EDITMODE, OnObjectEditMode)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_EDITMODE, OnUpdateObjectEditMode)
	ON_COMMAND(ID_BRUSH_EDITMODE, OnBrushEditMode)
	ON_UPDATE_COMMAND_UI(ID_BRUSH_EDITMODE, OnUpdateBrushEditMode)
	ON_COMMAND(ID_TEXTURE_PASS1, OnTexturePass1)
	ON_COMMAND(ID_TEXTURE_PASS2, OnTexturePass2)
	ON_COMMAND(ID_CENTERONSELECTION, OnCenterOnSelection)
	ON_UPDATE_COMMAND_UI(ID_CENTERONSELECTION, OnUpdateCenterOnSelection)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_VIEW_TOOLTIPS, OnViewTooltips)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLTIPS, OnUpdateViewTooltips)
	ON_COMMAND(ID_EDIT_OPTIONS, OnOptions)
	ON_COMMAND(ID_EDIT_PALETTE, OnPaletteEdit )
	ON_UPDATE_COMMAND_UI(ID_WINDOW_NEXT, OnUpdateMDINext)
	ON_COMMAND(ID_TOGGLE_EDIT_MODE, OnToggleEditMode)
	ON_COMMAND(ID_CENTER_ON_MARKER, OnCenterOnMarker)
	ON_UPDATE_COMMAND_UI(ID_CENTER_ON_MARKER, OnUpdateCenterOnMarker)
	ON_UPDATE_COMMAND_UI(ID_FILE_RELOADPALETTEDAT, OnUpdateProjectOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateProjectOpen)
	ON_COMMAND_EX(CG_ID_VIEW_PROJECTBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(CG_ID_VIEW_PROJECTBAR, OnUpdateControlBarMenu)
	ON_COMMAND_EX(CG_ID_VIEW_CREZBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(CG_ID_VIEW_CREZBAR, OnUpdateControlBarMenu)
	ON_UPDATE_COMMAND_UI(IDW_NODEVIEW_TOOLBAR, OnUpdateControlBarMenu)
	ON_COMMAND_EX(IDW_NODEVIEW_TOOLBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(IDW_WORLDEDIT_TOOLBAR, OnUpdateControlBarMenu)
	ON_COMMAND_EX(IDW_WORLDEDIT_TOOLBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PALETTE, OnUpdateProjectOpen)
	ON_COMMAND(ID_WINDOW_NEXT,MDINext)
	ON_COMMAND(ID_VIEW_TOGGLE_FLOATING_TOOLBARS, OnViewToggleFloatingToolbars)
	ON_MESSAGE(WM_USER_MODEL_LOADED, OnModelLoaded)
	ON_COMMAND(ID_RENAME_RESOURCES, OnRenameResources)
	ON_UPDATE_COMMAND_UI(ID_RENAME_RESOURCES, OnUpdateRenameResources)
	ON_COMMAND(ID_UPDATE_WORLD_OBJECTS, OnUpdateWorldObjects)
	ON_UPDATE_COMMAND_UI(ID_UPDATE_WORLD_OBJECTS, OnUpdateUpdateWorldObjects)
	ON_COMMAND(ID_WORLD_SPECIAL_IGNORE_MSGS_IN_PREFABS, OnIgnoreMsgsInPrefabs)
	ON_UPDATE_COMMAND_UI(ID_WORLD_SPECIAL_IGNORE_MSGS_IN_PREFABS, OnUpdateIgnoreMsgsInPrefabs)
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_BRUSHNAME,
	ID_INDICATOR_SELECTION,
	ID_INDICATOR_TEXTURE,
	ID_INDICATOR_VIEWTYPE,
	ID_INDICATOR_GRID,
	ID_INDICATOR_CAMERA,
	ID_INDICATOR_CURSOR,
	ID_INDICATOR_BOUNDING_BOX
};



/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	g_pMainFrame = this;

	m_pProjectBar = new CProjectBar;
	m_pPropertiesDlg = new CPropertiesDlg;
	m_pNodeView = new CNodeView;
	m_pClassListDlg = new ClassListDlg;
	m_pTextureDlg = new CTextureDlg;
	m_pModelDlg = new CModelDlg;
	m_pWorldsDlg = new CWorldsDlg;
	m_pSoundDlg = new CSoundDlg;
	m_pSpriteDlg = new CSpriteDlg;
	m_pPrefabDlg = new CPrefabDlg;

	m_WorldEditMode = BRUSH_EDITMODE;
	m_NodeSelectionMode = SINGLE_SELECTION;
	m_bShown = FALSE;
	m_bToolTips = TRUE;
	m_bClosing = FALSE;
	m_bIgnoreMsgsInPrefabs = TRUE;

	m_projectControls.SetSize(0);
}

CMainFrame::~CMainFrame()
{
}

//------------------------------------------------------------------------------------
//
//  CMainFrame::OnCreate
//
//  Purpose:	Creates Main Frame.
//
//------------------------------------------------------------------------------------
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CString			str;
	CFlatToolbar		*pNV = GetNodeViewToolbar();
	CFlatToolbar		*pWE = GetWorldEditToolbar();
	
	
	if (CMRCMDIFrameWndSizeDock::OnCreate(lpCreateStruct) == -1)
		return -1;

	//SCHLEGZ 12/31/97 New toolbars
	// Setup the standard toolbar...

	if (!m_wndMainBar.Create(this, /*WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC |*/
		CBRS_TOP, AFX_IDW_TOOLBAR ) ||
		!m_wndMainBar.LoadToolBar( IDR_MAINFRAME ))
	{
		TRACE0("Failed to create main toolbar\n");
		return -1;      // fail to create
	}
	str.LoadString( IDS_STANDARD_TOOLBAR );
	m_wndMainBar.SetWindowText( str );
	m_wndMainBar.EnableDocking( CBRS_ALIGN_ANY );


	// Setup the Node View toolbar.
	if( !pNV->Create( this, /*WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC |*/
		CBRS_TOP, IDW_NODEVIEW_TOOLBAR ) ||
		!pNV->LoadToolBar( IDR_NODEVIEW_TOOLBAR ))
	{
		TRACE0("Failed to create Node View toolbar\n");
		return -1;
	}

	str.LoadString( IDS_NODEVIEWTOOLBAR_TEXT );
	pNV->SetWindowText( str );
	pNV->EnableDocking( CBRS_ALIGN_ANY );

	pNV->SetButtonStyle( 0, TBBS_GROUP );
	pNV->SetButtonStyle( 1, TBBS_GROUP );
//	pNV->SetButtonStyle( 2, TBBS_GROUP );
//	pNV->SetButtonStyle( 3, TBBS_GROUP );


	// Setup the world editing toolbar.
	if( !pWE->Create(this, /*WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC |
			*/CBRS_TOP, IDW_WORLDEDIT_TOOLBAR ) ||
			!pWE->LoadToolBar( IDR_WORLDEDIT_TOOLBAR ))
	{
		TRACE0("Failed to create world edit toolbar\n");
		return -1;
	}

	str.LoadString( IDS_WORLDEDITTOOLBAR_TEXT );
	pWE->SetWindowText( str );
	pWE->EnableDocking( CBRS_ALIGN_ANY );

	pWE->SetButtonStyle( 0, TBBS_GROUP );
	pWE->SetButtonStyle( 1, TBBS_GROUP );
	pWE->SetButtonStyle( 2, TBBS_GROUP );


	// Setup status bar...

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	UINT nPaneID, nPaneStyle;
	int nPaneWidth, nPaneIndex;
	CString szStatusText;
	CSize szExtent;

	// Get a dc and a font...
	CClientDC dc( &m_wndStatusBar );
	CFont* pOldFont = dc.SelectObject( m_wndStatusBar.GetFont( ));

	// Setup the message pane...
    m_wndStatusBar.SetPaneInfo( 0, m_wndStatusBar.GetItemID( 0 ),
           SBPS_STRETCH, NULL );
	
	// Setup the current selected texture...
	nPaneIndex = m_wndStatusBar.CommandToIndex( ID_INDICATOR_TEXTURE );
	szStatusText.LoadString( IDS_NOSELTEXTURE );
	szExtent = dc.GetTextExtent( str );
	m_wndStatusBar.GetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, nPaneWidth );
	m_wndStatusBar.SetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, szExtent.cx );

	// Setup the view type pane...
	nPaneIndex = m_wndStatusBar.CommandToIndex( ID_INDICATOR_VIEWTYPE );
	szStatusText.LoadString( IDS_VIEW_PERSP );
	szExtent = dc.GetTextExtent( str );
	m_wndStatusBar.GetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, nPaneWidth );
	m_wndStatusBar.SetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, szExtent.cx );

	// Setup the grid pane...
	nPaneIndex = m_wndStatusBar.CommandToIndex( ID_INDICATOR_GRID );
	szStatusText.LoadString( IDS_GRID );
	szStatusText += "0000";
	szExtent = dc.GetTextExtent( szStatusText );
	m_wndStatusBar.GetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, nPaneWidth );
	m_wndStatusBar.SetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, szExtent.cx );

	// Setup the camera pane...
	nPaneIndex = m_wndStatusBar.CommandToIndex( ID_INDICATOR_CAMERA );
	szStatusText = _T( "(-1.0,-1.0,-1.0)@(-9999,-9999,-9999)" );
	szExtent = dc.GetTextExtent( szStatusText );
	m_wndStatusBar.GetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, nPaneWidth );
	m_wndStatusBar.SetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, szExtent.cx );

	// Setup the cursor pane...
	nPaneIndex = m_wndStatusBar.CommandToIndex( ID_INDICATOR_CURSOR );
	szStatusText = _T( "@(-9999,-9999,-9999)" );
	szExtent = dc.GetTextExtent( szStatusText );
	m_wndStatusBar.GetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, nPaneWidth );
	m_wndStatusBar.SetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, szExtent.cx );

	// Setup the bounding box pane
	nPaneIndex = m_wndStatusBar.CommandToIndex( ID_INDICATOR_BOUNDING_BOX );
	szStatusText = _T("(-9999, -9999, -9999)");
	szExtent = dc.GetTextExtent( szStatusText );
	m_wndStatusBar.GetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, nPaneWidth );
	m_wndStatusBar.SetPaneInfo( nPaneIndex, nPaneID, nPaneStyle, szExtent.cx );

	// Restore the dc's old font...
	dc.SelectObject(pOldFont);

	
	// Setup the color dialog...
	if( !GetColorSelectDlg()->Create(IDD_COLORSELECT_DLG, this) )
	{
		TRACE0("Failed to create the color selection dialog.");
		return -1;
	}
	GetColorSelectDlg()->ShowWindow( SW_HIDE );

	// Setup the level error dialog...
	if( !GetLevelErrorDlg()->Create(IDD_LEVEL_ERROR, this) )
	{
		TRACE0("Failed to create the level error dialog.");
		return -1;
	}
	GetLevelErrorDlg()->ShowWindow( SW_HIDE );

	// Setup the level items dialog...
	if( !GetLevelItemsDlg()->Create(IDD_LEVEL_ITEMS, this) )
	{
		TRACE0("Failed to create the level items dialog.");
		return -1;
	}
	GetLevelItemsDlg()->ShowWindow( SW_HIDE );


	// Setup the level textures dialog...
	if( !GetLevelTexturesDlg()->Create(IDD_LEVEL_TEXTURES, this) )
	{
		TRACE0("Failed to create the level textures dialog.");
		return -1;
	}
	GetLevelTexturesDlg()->ShowWindow( SW_HIDE );

	// Setup the object search dialog...
	if( !GetObjectSearchDlg()->Create(IDD_OBJECTSEARCH, this) )
	{
		TRACE0("Failed to create the object search dialog.");
		return -1;
	}
	GetObjectSearchDlg()->ShowWindow( SW_HIDE );

	// Setup the object selection filter dialog...
	if( !GetObjectSelFilterDlg()->Create(IDD_OBJECTSELFILTER, this) )
	{
		TRACE0("Failed to create the object selection filter dialog.");
		return -1;
	}
	GetObjectSelFilterDlg()->ShowWindow( SW_HIDE );

	// Setup the rename resource dialog
	if(!GetRenameResourceDlg()->Create(IDD_RENAMERESOURCE, this) )
	{
		TRACE0("Failed to create the rename resource dialog.");
		return -1;
	}
	GetRenameResourceDlg()->ShowWindow( SW_HIDE );

	// Create Project Bar...
	InitProjectBar();

	// Restore registry settings for frame and components...
	RestoreRegistryInfo( );

	// Restore registry settings for project bar...
	GetProjectBar( )->RestoreRegistryInfo( _T( "General" ));

	// Dock everything...
	EnableDocking(CBRS_ALIGN_ANY, RUNTIME_CLASS(CDEDockFrameWnd));
	DockControlBar( &m_wndMainBar, AFX_IDW_DOCKBAR_TOP );
	DockControlBar( &m_NodeViewToolbar, AFX_IDW_DOCKBAR_TOP );
	DockControlBar( &m_WorldEditToolbar, AFX_IDW_DOCKBAR_TOP );
	DockControlBar( m_pProjectBar, AFX_IDW_DOCKBAR_LEFT );
	ShowControlBar( &m_wndMainBar, (m_wndMainBar.GetStyle() & WS_VISIBLE) == 0, FALSE);
	ShowControlBar( &m_NodeViewToolbar, (m_NodeViewToolbar.GetStyle() & WS_VISIBLE) == 0, FALSE);
	ShowControlBar( &m_WorldEditToolbar, (m_WorldEditToolbar.GetStyle() & WS_VISIBLE) == 0, FALSE);
	
	// Get the window options
	COptionsWindows *pOptions=GetApp()->GetOptions().GetWindowsOptions();

	// Make sure that all of our control bars are visible
	int32 i;
	for (i=0; i < m_projectControls.GetSize(); i++)
	{
		ShowControlBar( m_projectControls[i]->m_pWnd, TRUE, FALSE );
	}

	// Create the project bars	
	for (i=0; i < pOptions->GetNumMainControls(); i++)
	{
		CMRCSizeDialogBar *pControlBar=CreateProjectControlBar(pOptions->GetMainControl(i));

		// Make sure that the window gets properly sized
		pControlBar->RepositionControls();
	}	

	// Load toolbar states from registry...
	LoadBarState(_T("General"));
	LoadSizeBarState(_T("General"));

	CSplashWnd::ShowSplashScreen(this);

	return 0;
}

/************************************************************************/
// Call this to add a project control info structure based on the
// control type.
// Returns: pointer to the allocated control info structure.
CProjectControlBarInfo *CMainFrame::AddProjectControlInfo(CMoArray<CProjectControlBarInfo *> &controlArray,
														  CMainFrame::ProjectControl controlType)
{
	// The tab info structure
	CProjectControlBarInfo *pControlInfo=NULL;

	// Fill in the tab info structure with the appropriate data
	switch (controlType)
	{
	case CB_WORLDSVIEW:		
			pControlInfo = new CProjectControlBarInfo(GetWorldsDlg(), IDS_WORLDS, IDD_WORLDS_TABDLG, ID_SELECT_TAB_WORLDSVIEW, controlType);
			break;		
	case CB_TEXTUREVIEW:		
			pControlInfo = new CProjectControlBarInfo(GetTextureDlg(), IDS_TEXTURES, IDD_TEXTURE_TABDLG, ID_SELECT_TAB_TEXTUREVIEW, controlType);	
			break;		
	case CB_NODESVIEW:		
			pControlInfo = new CProjectControlBarInfo(GetNodeView(), IDS_NODES, IDD_NODEVIEW_TABDLG, ID_SELECT_TAB_NODESVIEW, controlType);	
			break;		
	case CB_PROPERTIESVIEW:		
			pControlInfo = new CProjectControlBarInfo(GetPropertiesDlg(), IDS_PROPERTIES, IDD_PROPERTIES, ID_SELECT_TAB_PROPERTIESVIEW, controlType);
			break;		
	case CB_MODELVIEW:		
			pControlInfo = new CProjectControlBarInfo(GetModelDlg(), IDS_MODELS, IDD_MODEL_TABDLG, ID_SELECT_TAB_MODELVIEW, controlType);
			break;		
	case CB_SOUNDVIEW:		
			pControlInfo = new CProjectControlBarInfo(GetSoundDlg(), IDS_SOUNDS, IDD_SOUND_TABDLG, ID_SELECT_TAB_SOUNDVIEW, controlType);
			break;	
	case CB_SPRITEVIEW:
			pControlInfo = new CProjectControlBarInfo(GetSpriteDlg(), IDS_SPRITES, IDD_SPRITE_TABDLG, ID_SELECT_TAB_SPRITEVIEW, controlType);	
			break;		
	case CB_CLASSVIEW:		
			pControlInfo = new CProjectControlBarInfo(GetClassListDlg(), IDS_CLASSLIST, IDD_CLASSES_TABDLG, ID_SELECT_TAB_CLASSVIEW, controlType);
			break;
	case CB_PREFABVIEW:
			pControlInfo = new CProjectControlBarInfo(GetPrefabDlg(), IDS_PREFABS, IDD_PREFAB_TABDLG, ID_SELECT_TAB_PREFABVIEW, controlType);
			break;
	default:
		{
			return NULL;
		}
	}

	ASSERT(pControlInfo);

	// Add the tab info
	if (pControlInfo)
	{
		controlArray.Append(pControlInfo);
	}

	// Return the tab info
	return pControlInfo;
}

/************************************************************************/
// This creates a project bar window (from the control type) and returns
// a pointer to the created window.  Note that it doesn't allocate a new
// CMRCSizeDialogBar, it is just returning a pointer to the window that
// is assocated with the control type.
CMRCSizeDialogBar *CMainFrame::CreateProjectControlBar(CMainFrame::ProjectControl controlType)
{
	// Make sure that this control type hasn't already been created
	int i;
#ifdef _DEBUG
	for (i=0; i < m_projectControls.GetSize(); i++)
	{
		ASSERT(m_projectControls[i]->m_ControlType != controlType);
	}
#endif

	// Add the project control info
	CProjectControlBarInfo *pInfo=AddProjectControlInfo(m_projectControls, controlType);

	// Get the window
	CMRCSizeDialogBar *pBar=pInfo->m_pWnd;

	// Create a new control bar window and attach it to the mainframe
	pBar->Create(this, pInfo->m_DialogID, CBRS_RIGHT, pInfo->m_SelectID);
	pBar->ShowWindow(SW_SHOW);
	pBar->EnableDocking( CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT );	

	// Set the title for the control bar
	CString sTitle;
	sTitle.LoadString(pInfo->m_TitleStringID);
	pBar->SetWindowText(sTitle);

	return pBar;
}

/************************************************************************/
// This destroys a project bar window and returns a pointer to the
// destroyed window.
CMRCSizeDialogBar *CMainFrame::DestroyProjectControlBar(CMainFrame::ProjectControl controlType)
{
	// Find the window
	CMRCSizeDialogBar *pWnd=NULL;

	int i;
	for (i=0; i < m_projectControls.GetSize(); i++)
	{
		if (m_projectControls[i]->m_ControlType == controlType)
		{
			// Store the window
			pWnd=m_projectControls[i]->m_pWnd;

			// Delete the control from the array
			delete m_projectControls[i];
			m_projectControls.Remove(i);

			break;
		}
	}

	ASSERT(pWnd);
	if (pWnd)
	{
		pWnd->DestroyWindow();
	}

	// Save the settings in the registry
	GetApp()->GetOptions().GetWindowsOptions()->SetMainControlArray(m_projectControls);

	return pWnd;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UpdateStatusText
//
//	PURPOSE:	Central function for updating status bar text strings
//
// ----------------------------------------------------------------------- //

void CMainFrame::UpdateStatusText(int nIndex, const char* szText)
{
	UINT nID=0,nStyle=0;
	int nWidth=0;
	CSize size;

	int nPane = m_wndStatusBar.CommandToIndex(nIndex);

	m_wndStatusBar.GetPaneInfo(nPane,nID,nStyle,nWidth);
	{//compute size of status string
		HGDIOBJ hOldFont = NULL;
		HFONT hFont = (HFONT)m_wndStatusBar.SendMessage(WM_GETFONT);
		CClientDC dc(NULL);
		if (hFont != NULL) 
			hOldFont = dc.SelectObject(hFont);
		size = dc.GetTextExtent(szText,strlen(szText));
		if (hOldFont != NULL) 
			dc.SelectObject(hOldFont);
	}
	m_wndStatusBar.SetPaneInfo(nPane,nID,nStyle,size.cx);
	m_wndStatusBar.SetPaneText(nPane,szText,TRUE);

	return;
}

//--------------------------------------------------------------------------------
//
//  CMainFrame::DockControlBarRightOf
//
//  Purpose:	Puts Bar to the right of RightOf.
//
//--------------------------------------------------------------------------------
void CMainFrame::DockControlBarRightOf( CToolBar* Bar,CToolBar* RightOf )
{
	CRect rect;
	DWORD dw;
	UINT n;

	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout();
	RightOf->GetWindowRect(&rect);
	rect.OffsetRect(1,0);
	dw=RightOf->GetBarStyle();
	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;

	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line.  By calculating a rectangle, we in effect
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar(Bar,n,&rect);
}


//-----------------------------------------------------------------------------
//
//  Registry data
//
//-----------------------------------------------------------------------------
static TCHAR szWindowFormat[] = _T("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");
static TCHAR szColorDlgFormat[] = _T( "%d %d %d %d %d" );
extern TCHAR szRegKeyCompany[];
extern TCHAR szRegKeyApp[];
extern TCHAR szRegKeyVer[];
static TCHAR szMainWindow[] = _T( "MainWindow" );
static TCHAR szColorSelectDlg[] = _T( "ColorSelectDlg" );
static TCHAR szToolTips[] = _T( "ToolTips" );
static TCHAR szUndoPreferences[] = _T("Undo Preferences");
static TCHAR szUndoPreferencesFormat[] = _T("%d %d %d %d");

//-----------------------------------------------------------------------------
//
//  CMainFrame::RestoreRegistryInfo()
//
//  Purpose:	Reads registry for window placement and toolbar info.
//
//-----------------------------------------------------------------------------
void CMainFrame::RestoreRegistryInfo()
{
	CRegMgr			regMgr;
	UINT32			bufSize;
	char			str[501];
	TCHAR			temp[256];

	if( regMgr.Init( szRegKeyCompany, szRegKeyApp, szRegKeyVer, NULL, HKEY_CURRENT_USER ))
	{
		// Place the main window.
		bufSize=500;
		if( regMgr.Get( szMainWindow, str, bufSize) )
		{
			WINDOWPLACEMENT wp;
			int nRead = _stscanf(str, szWindowFormat,
				&wp.flags, &wp.showCmd,
				&wp.ptMinPosition.x, &wp.ptMinPosition.y,
				&wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
				&wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
				&wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

			if( nRead == 10 )
			{
				wp.length = sizeof wp;
				GetMainFrame( )->SetWindowPlacement( &wp );
			}
		}

		// Get tooltips flag...
		if( regMgr.Get( szToolTips, str, bufSize ))
		{
			_stscanf( str, "%u", &m_bToolTips );
		}

		// Set the tooltips flags...
		SetToolTips( );

		// Place the color dialog.
		bufSize=500;
		if( regMgr.Get( szColorSelectDlg, str, bufSize) )
		{
			BOOL bVisible;
			int left, top, width, height;

			sscanf( str, szColorDlgFormat, &bVisible, &left, &top, &width, &height );

			GetColorSelectDlg()->MoveWindow( left, top, width, height );
			GetColorSelectDlg()->ShowWindow( bVisible ? SW_SHOW : SW_HIDE );
		}
	
		// Get the undo preferences...
		bufSize=256;
		SetUndoPreferencesDefault( m_iUndoGeometry, m_iUndoTextures, m_iUndoProperties, m_iUndoGeneral );
		SetUndoPreferencesDefault( m_iCurUndoGeometry, m_iCurUndoTextures, m_iCurUndoProperties, m_iCurUndoGeneral );
		if( regMgr.Get( szUndoPreferences, str, bufSize) )
		{
			sscanf( str, szUndoPreferencesFormat, &m_iUndoGeometry, &m_iUndoTextures, 
				&m_iUndoProperties, &m_iUndoGeneral );
		}
	
	}	
}

//-----------------------------------------------------------------------------
//
//  CMainFrame::SaveRegistryInfo()
//
//  Purpose:	Saves window placement and toolbar info to registry.
//
//-----------------------------------------------------------------------------
void CMainFrame::SaveRegistryInfo()
{
	CRegMgr				regMgr;
	char				keyVal[501];
	CRect				rect;


	if( regMgr.Init( szRegKeyCompany, szRegKeyApp, szRegKeyVer, NULL, HKEY_CURRENT_USER ))
	{
		// Window position.
		WINDOWPLACEMENT wp;
		wp.length = sizeof wp;
		if (GetMainFrame( )->GetWindowPlacement( &wp ))
		{
			wp.flags = 0;
			if( GetMainFrame( )->IsZoomed( ))
				wp.flags |= WPF_RESTORETOMAXIMIZED;
			// and write it to the .INI file
			wsprintf(keyVal, szWindowFormat,
				wp.flags, wp.showCmd,
				wp.ptMinPosition.x, wp.ptMinPosition.y,
				wp.ptMaxPosition.x, wp.ptMaxPosition.y,
				wp.rcNormalPosition.left, wp.rcNormalPosition.top,
				wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);
				regMgr.Set( szMainWindow, keyVal);
		}

		// Save tooltips flag...
		wsprintf( keyVal, "%u", m_bToolTips );
		regMgr.Set( szToolTips, keyVal );

		// Color select dialog position.
		GetColorSelectDlg()->GetWindowRect( &rect );
		sprintf( keyVal, szColorDlgFormat, GetColorSelectDlg()->IsWindowVisible(), rect.left, rect.top, rect.Width(), rect.Height() );
		regMgr.Set( szColorSelectDlg, keyVal);
	
		sprintf( keyVal, szUndoPreferencesFormat, m_iUndoGeometry, m_iUndoTextures, m_iUndoProperties, m_iUndoGeneral );
		regMgr.Set( szUndoPreferences, keyVal );
	}

	// Save the settings in the registry
	GetApp()->GetOptions().GetWindowsOptions()->SetMainControlArray(m_projectControls);
}

void CMainFrame::InitProjectBar()
{
	m_pProjectBar->Init(this);
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style |= WS_VISIBLE;
	return CMRCMDIFrameWndSizeDock::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMRCMDIFrameWndSizeDock::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMRCMDIFrameWndSizeDock::Dump(dc);
}

#endif //_DEBUG





// ---------------------------------------------------- //
// Helpers
// ---------------------------------------------------- //

int CMainFrame::GetNumRegionDocs()
{
	return (int)m_pProjectBar->m_RegionDocs.GetSize();
}


void CMainFrame::UpdateAllRegions( LPARAM hint )
{
	CDocTemplate		*pTemp = GetApp()->m_pWorldTemplate;
	POSITION			pos;

	for( pos=pTemp->GetFirstDocPosition(); pos != NULL; )
		pTemp->GetNextDoc(pos)->UpdateAllViews( NULL, hint );
}




/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

static BOOL _FindNameInList(char *pTestName, CLinkedList<CString*> &names)
{
	LPOS pos;
	CString *pName;

	for(pos=names; pos; )
	{
		pName = names.GetNext(pos);
	
		if(pName->CompareNoCase(pTestName) == 0)
			return TRUE;
	}

	return FALSE;
}


static void _InsertStringIntoList(char *pString, CLinkedList<CString*> &theList)
{
	LPOS pos;
	CString *pNewString, *pTest;

	pNewString = new CString;
	*pNewString = pString;

	for(pos=theList; pos; )
	{
		pTest = theList.GetAt(pos);

		if(pTest->CompareNoCase(pString) == 1)
		{
			theList.InsertBefore(pos, pNewString);
			return;
		}

		theList.GetNext(pos);
	}

	theList.Append(pNewString);
}

void CMainFrame::OnShowWorldClasses()
{
	CDocTemplate *pTemp = GetApp()->m_pWorldTemplate;
	POSITION pos;
	CRegionDoc *pDoc;
	CString *pString, *pName;
	uint32 iObject;
	LPOS classPos;
	CBaseEditObj *pObject;
	char *pClassName;
	CLinkedList<CString*> classNames;

	// For each region doc.
	for( pos=pTemp->GetFirstDocPosition(); pos != NULL; )
	{
		pDoc = (CRegionDoc*)pTemp->GetNextDoc(pos);

		if(pDoc->IsKindOf(RUNTIME_CLASS(CRegionDoc)))
		{	
			// Build the list of class names.
			for(iObject=0; iObject < pDoc->m_Region.m_Objects.GetSize(); ++iObject )
			{
				pObject = pDoc->m_Region.m_Objects[iObject];

				pClassName = (char*)pObject->GetClassName();
				if(!_FindNameInList(pClassName, classNames))
				{
					_InsertStringIntoList(pClassName, classNames);
				}
			}

			// Print it to the debug window.
			AddDebugMessage("");
			AddDebugMessage("Classes for world %s", pDoc->m_FileName);
			for(classPos=classNames; classPos; )
			{
				pName = classNames.GetNext(classPos);
				AddDebugMessage("  %s  %s", (LPCTSTR)(*pName), (GetProject()->FindClassDef(*pName) != NULL) ? "" : "(Invalid)");
			}
			
			AddDebugMessage("");

			DeleteAndRemoveElements(classNames);
		}
	}
}

BOOL FindName(CMoArray<char*> &textureNames, char *pName)
{
	for(DWORD i=0; i < textureNames; i++)
	{
		if(stricmp(textureNames[i], pName) == 0)
			return TRUE;
	}

	return FALSE;
}

void CMainFrame::OnViewProjectWindow() 
{
	if( m_pProjectBar && ::IsWindow(m_pProjectBar->m_hWnd) )
	{
		if( m_pProjectBar->IsWindowVisible() )
			ShowControlBar( m_pProjectBar, FALSE, FALSE );
		else
			ShowControlBar( m_pProjectBar, TRUE, FALSE );

	}

	RecalcLayout();
}


void CMainFrame::OnUpdateViewProjectWindow(CCmdUI* pCmdUI) 
{
	if( m_pProjectBar && ::IsWindow(m_pProjectBar->m_hWnd) )
	{
		pCmdUI->SetCheck( m_pProjectBar->IsWindowVisible() );
	}
}

void CMainFrame::OnPrefabTracker() 
{
	CPrefabTrackerDlg Dlg(this);
	Dlg.DoModal();	
}

void CMainFrame::OnUpdatePrefabTracker(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetProjectBar() && GetProjectBar()->IsProjectOpen());
}

void CMainFrame::OnObjectTracker() 
{
	CObjectTrackerDlg Dlg(this);
	Dlg.DoModal();	
}

void CMainFrame::OnUpdateObjectTracker(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetProjectBar() && GetProjectBar()->IsProjectOpen());
}

void CMainFrame::OnTextureTracker() 
{
	CTextureTrackerDlg Dlg(this);
	Dlg.DoModal();	
}

void CMainFrame::OnUpdateTextureTracker(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetProjectBar() && GetProjectBar()->IsProjectOpen());
}

void CMainFrame::OnSearchReplaceInTextures() 
{
	CTextureSearchReplaceDlg Dlg;
	Dlg.DoModal();	
}


void CMainFrame::OnUpdateSearchReplaceInTextures(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetProjectBar() && GetProjectBar()->IsProjectOpen());
}


void CMainFrame::OnOptions() 
{
	COptionsSheet	OptDlg("DEdit Options",this,0);

	// Get the options
	CDEditOptions *pOptions=&GetApp()->GetOptions();

	// Set the autosave time
	OptDlg.m_undoPage.m_dwAutoSaveTime		= pOptions->GetAutoSaveTime();
	OptDlg.m_undoPage.m_bEnableAutoSave		= pOptions->IsAutoSave();
	OptDlg.m_undoPage.m_nNumBackups			= pOptions->GetNumBackups();
	OptDlg.m_undoPage.m_sBackupPath			= pOptions->GetAutoSavePath();
	OptDlg.m_undoPage.m_bDeleteOnClose		= pOptions->DeleteOnClose();

	if(OptDlg.DoModal() == IDOK)
	{
		// Check if the user changed the size of the undo buffer in the dialog.
		// If so, resize the CMoArray, and set the new value in the options.
		if (OptDlg.m_miscPage.DidUndoValueChange()) 
		{
			for(int i=0; i < m_pProjectBar->m_RegionDocs; i++ )
			{
				// Change array size
				m_pProjectBar->m_RegionDocs[i]->m_UndoMgr.SetUndoLimit(pOptions->GetMiscOptions()->GetNumUndos());
			}

			// Save the number of undos in the options
			//pOptions->SetNumUndos(OptDlg.m_miscPage.m_dwUndos);
		}

		//resize the memory of the model manager if applicable
		if(pOptions->GetModelsOptions()->IsLimitMemoryUse())
		{
			GetApp()->GetModelMgr().SetMaxMemoryUsage(pOptions->GetModelsOptions()->GetMaxMemoryUse());
		}
		//we also need to have all the region objects update their class icons, and titles
		for(uint32 nCurrDoc = 0; nCurrDoc < m_pProjectBar->m_RegionDocs; nCurrDoc++ )
		{
			//update class icons
			m_pProjectBar->m_RegionDocs[nCurrDoc]->GetRegion()->UpdateObjectClassIcons();
			//update the title
			m_pProjectBar->m_RegionDocs[nCurrDoc]->SetTitleKeepModified();
		}


		BOOL offset = !!OptDlg.m_d3dPage.m_bBlurryTextures;

		// Set the mipmap offset
		pOptions->GetDisplayOptions()->SetMipMapOffset(offset);
		
		// Set the autosave time
		pOptions->SetAutoSaveTime(OptDlg.m_undoPage.m_dwAutoSaveTime);

		// Set the number of backups
		pOptions->SetNumBackups(OptDlg.m_undoPage.m_nNumBackups);

		// Set whether or not backups are enabled
		pOptions->EnableAutoSave(OptDlg.m_undoPage.m_bEnableAutoSave);

		//set the path to where autosaves are stored
		pOptions->SetAutoSavePath(OptDlg.m_undoPage.m_sBackupPath);

		//set whether or not files should be deleted on close
		pOptions->SetDeleteOnClose(OptDlg.m_undoPage.m_bDeleteOnClose);

		// Update the run options
		OptDlg.m_runPage.SaveOptions();
	}
}

void CMainFrame::OnPaletteEdit() 
{
	CPaletteEdit dlgPaletteEdit( this );

	dlgPaletteEdit.DoModal( );
}

void CMainFrame::OnViewColorSelection() 
{
	if( GetColorSelectDlg()->IsWindowVisible() )
		GetColorSelectDlg()->ShowWindow( SW_HIDE );
	else
		GetColorSelectDlg()->ShowWindow( SW_SHOW );
}

void CMainFrame::OnUpdateViewColorSelection(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( GetColorSelectDlg()->IsWindowVisible() );
}

void CMainFrame::OnViewLevelError() 
{
	if( GetLevelErrorDlg()->IsWindowVisible() )
		GetLevelErrorDlg()->ShowWindow( SW_HIDE );
	else
		GetLevelErrorDlg()->ShowWindow( SW_SHOW );
}

void CMainFrame::OnUpdateViewLevelError(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( GetLevelErrorDlg()->IsWindowVisible() );
}

void CMainFrame::OnViewLevelItems() 
{
	if( GetLevelItemsDlg()->IsWindowVisible() )
		GetLevelItemsDlg()->ShowWindow( SW_HIDE );
	else
		GetLevelItemsDlg()->ShowWindow( SW_SHOW );
}

void CMainFrame::OnUpdateViewLevelItems(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( GetLevelItemsDlg()->IsWindowVisible() );
}

void CMainFrame::OnViewLevelTextures() 
{
	if( GetLevelTexturesDlg()->IsWindowVisible() )
		GetLevelTexturesDlg()->ShowWindow( SW_HIDE );
	else
		GetLevelTexturesDlg()->ShowWindow( SW_SHOW );
}

void CMainFrame::OnUpdateViewLevelTextures(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetNumRegionDocs() > 0 );
	pCmdUI->SetCheck( GetLevelTexturesDlg()->IsWindowVisible() );
}

void CMainFrame::OnObjectSelFilter() 
{
	if( GetObjectSelFilterDlg()->IsWindowVisible() )
		GetObjectSelFilterDlg()->ShowWindow( SW_HIDE );
	else
		GetObjectSelFilterDlg()->ShowWindow( SW_SHOW );
}

void CMainFrame::OnUpdateObjectSelFilter(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_WorldEditMode == OBJECT_EDITMODE );
	pCmdUI->SetCheck( GetObjectSelFilterDlg()->IsWindowVisible() );
}

void CMainFrame::OnViewObjectSearch() 
{
	if( GetObjectSearchDlg()->IsWindowVisible() )
		GetObjectSearchDlg()->ShowWindow( SW_HIDE );
	else
		GetObjectSearchDlg()->ShowWindow( SW_SHOW );
}

void CMainFrame::OnUpdateViewObjectSearch(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetNumRegionDocs() > 0 );
	pCmdUI->SetCheck( GetObjectSearchDlg()->IsWindowVisible() );
}


void CMainFrame::OnDestroy() 
{
	GetColorSelectDlg( )->DestroyWindow( );
	CMRCMDIFrameWndSizeDock::OnDestroy();
}



BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	return CMRCMDIFrameWndSizeDock::OnCreateClient(lpcs, pContext);
}




// ---------------------------------------------------- //
// Main handlers
// ---------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDirectorySize
//
//	PURPOSE:	Recursively find the total size of a directory
//
// ----------------------------------------------------------------------- //

DWORD GetDirectorySize(char* szPath)
{
	DWORD dwSize = 0;
	WIN32_FIND_DATA data;
	
	SetCurrentDirectory(szPath);

	HANDLE hFile = FindFirstFile("*.*",&data);
	FindNextFile(hFile,&data);
	
	while(FindNextFile(hFile,&data))
	{
		if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			char szNewPath[MAX_PATH];
			sprintf(szNewPath, "%s\\%s",szPath,data.cFileName);

			dwSize += GetDirectorySize(szNewPath);
		}
		else
		{
			dwSize += (data.nFileSizeHigh * MAXDWORD) + data.nFileSizeLow;
		}
	}

	return dwSize;
}

void CMainFrame::OnCreateRezFile()
{
	CProjectBar *pBar;
	CString baseDir, filterStr, outputFilename;
	char baseDirTitle[256];
	int status;

	pBar = GetProjectBar();

	if(pBar->VerifyProjectIsOpen())
	{
		// Get the name of the directory the resources are in.
		baseDir = dfm_GetBaseDir(GetFileMgr());
		CHelpers::ExtractNames(baseDir, NULL, NULL, baseDirTitle, NULL);

		//Calculate the size of the rez file
		char szOldPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH,szOldPath);
		DWORD dwSize = GetDirectorySize(baseDir.GetBuffer(0));
		dwSize = dwSize / (DWORD)1024L; //convert to KBs
		SetCurrentDirectory(szOldPath);


		//lets make sure that they definately want to create the rez file
	
		//build up the string to display
		CString sConfirmMsg;
						
		sConfirmMsg += "Creating the rez file may take a substantial amount of time.\n";
		sConfirmMsg += "Are you sure you want to do this?\n\n";
		
		//show how big the rez file may be
		CString sDiskSize;
		sDiskSize.Format("Estimated size of Rez file: %.2fMB", dwSize / 1024.0f);

		sConfirmMsg += sDiskSize;

		//show the message box and give them a chance to back out
		if(MessageBox(sConfirmMsg, "Rez file confirmation", MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
		{
			return;
		}


		filterStr.LoadString(IDS_REZFILEFILTER);
		
		strcat(baseDirTitle, ".rez");
		outputFilename = GetProject()->m_BaseProjectDir + ".rez";

		CFileDialog FileDlg(FALSE, "rez", (LPCTSTR)outputFilename, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
			filterStr, GetMainFrame());

		BOOL bSelectFile = TRUE;

		while(bSelectFile)
		{
			if(FileDlg.DoModal() == IDOK)
			{
				//SCHLEGZ 1/23/98: check to see if we have enough space for a rez file
				
				//Calculate the size of memory free on disk
				struct _diskfree_t diskfree;
				DWORD dwDiskSpace = 0;
				int nDrive = _getdrive(); // use current default drive
				if (_getdiskfree(nDrive, &diskfree) == 0)
				{
					dwDiskSpace = (DWORD)diskfree.avail_clusters * (DWORD)diskfree.sectors_per_cluster *
								  (DWORD)diskfree.bytes_per_sector / (DWORD)1024L;
				}

				//make sure there is space
				if(dwSize < dwDiskSpace)
				{
					BeginWaitCursor();

					//make sure that the resources are synced
					pBar->Save();

					//there is enough space, so lets save some strings and make a rez
					//file
					RezCompiler("c", FileDlg.GetPathName(), baseDir, TRUE);

					EndWaitCursor();

					bSelectFile = FALSE;
				}
				else
				{
					AfxMessageBox("There is not enough free disk space available on target drive.",MB_OK | MB_ICONSTOP);
					bSelectFile = TRUE;
				}
			}
			else
			{
				//user hit cancel, so we don't want to select another file
				bSelectFile = FALSE;
			}
		}
	}
}

void CMainFrame::OnUpdateProjectOpen( CCmdUI* pCmdUI )
{
	if( GetProjectBar() )
		pCmdUI->Enable( GetProjectBar()->IsProjectOpen() );
}


void CMainFrame::OnReloadPalettes()
{
}


void CMainFrame::OnClose()
{
	m_bClosing = TRUE;

	// Save Registry information...
	SaveRegistryInfo();	
	
	//save the last open project
	CString sLastProject = GetProjectBar()->GetProjFilename();
	GetApp()->GetOptions().GetMiscOptions()->SetStringValue("LastProject", sLastProject);

	if( GetProjectBar() )
		if( !GetProjectBar()->Close())
			return;	
	
	// Destroy the windows that the project bar owns
	GetProjectBar()->DestroyControlWindows();	
	GetProjectBar()->SaveRegistryInfo(_T("General"));

	// Save toolbar states to registry...
	SaveBarState(_T("General"));
	SaveSizeBarState(_T("General"));		

	// Destroy the project windows that we own
	int i;
	for (i=0; i < m_projectControls.GetSize(); i++)
	{
		m_projectControls[i]->m_pWnd->DestroyWindow();
	}

	if ( m_pProjectBar )		delete m_pProjectBar;	
	if ( m_pPropertiesDlg )		delete m_pPropertiesDlg;	
	if ( m_pNodeView )			delete m_pNodeView;	
	if ( m_pClassListDlg )		delete m_pClassListDlg;	
	if ( m_pTextureDlg )		delete m_pTextureDlg;
	if ( m_pModelDlg )			delete m_pModelDlg;	
	if ( m_pWorldsDlg )			delete m_pWorldsDlg;	
	if ( m_pSoundDlg )			delete m_pSoundDlg;	
	if ( m_pSpriteDlg )			delete m_pSpriteDlg;	
	if ( m_pPrefabDlg )			delete m_pPrefabDlg;

	DeleteAndClearArray(m_projectControls);

	CMRCMDIFrameWndSizeDock::OnClose( );	
}


// ---------------------------------------------------- //
// Edit mode handlers
// ---------------------------------------------------- //

void CMainFrame::OnGeometryEditMode() 
{
	CRegionFrame *pFrame;
	CRegionDoc *pDoc;

	// Clear all brush/object selections...
	pFrame = ( CRegionFrame * )MDIGetActive( );
	ASSERT( pFrame );
	pDoc = ( CRegionDoc * )pFrame->GetActiveDocument( );
	ASSERT( pDoc );
	pDoc->m_TaggedVerts.GenRemoveAll();
	
	m_WorldEditMode = GEOMETRY_EDITMODE;
	UpdateAllRegions( REGIONVIEWUPDATE_EDITMODE );

	// Update the menubar with the mode menus
	UpdateModeMenus();
}

void CMainFrame::OnUpdateGeometryEditMode(CCmdUI* pCmdUI) 
{
	CRegionView::SetMenuHotKeyText(UIE_GEOMETRY_EDIT_MODE, pCmdUI);

	pCmdUI->Enable( GetNumRegionDocs() > 0 );
	pCmdUI->SetRadio( GetWorldEditMode() == GEOMETRY_EDITMODE );
}

void CMainFrame::OnObjectEditMode() 
{
	m_WorldEditMode = OBJECT_EDITMODE;
	UpdateAllRegions( REGIONVIEWUPDATE_EDITMODE );

	// Update the menubar with the mode menus
	UpdateModeMenus();
}

void CMainFrame::OnUpdateObjectEditMode(CCmdUI* pCmdUI) 
{
	CRegionView::SetMenuHotKeyText(UIE_OBJECT_EDIT_MODE, pCmdUI);

	pCmdUI->Enable( GetNumRegionDocs() > 0 );
	pCmdUI->SetRadio( GetWorldEditMode() == OBJECT_EDITMODE );
}

void CMainFrame::OnBrushEditMode() 
{
	CRegionFrame *pFrame;
	CRegionDoc *pDoc;

	m_WorldEditMode = BRUSH_EDITMODE;
	UpdateAllRegions( REGIONVIEWUPDATE_EDITMODE );

	// Update the menubar with the mode menus
	UpdateModeMenus();
}

void CMainFrame::OnUpdateBrushEditMode(CCmdUI* pCmdUI) 
{
	CRegionView::SetMenuHotKeyText(UIE_BRUSH_EDIT_MODE, pCmdUI);

	pCmdUI->Enable( GetNumRegionDocs() > 0 );
	pCmdUI->SetRadio( GetWorldEditMode() == BRUSH_EDITMODE );
}

void CMainFrame::OnTexturePass1()
{
	SetCurrTexture(0);
}

void CMainFrame::OnTexturePass2()
{
	SetCurrTexture(1);
}


/************************************************************************/
// Updates the menu bar with the mode menus.  This basically adds
// the correct menu and removes the incorrect menu.
void CMainFrame::UpdateModeMenus()
{
	// Get the menu
	CMenu *pMenu=GetMenu();

	// Destroy the current menu
	m_modeMenu.DestroyMenu();

	// Get the menu to add	
	m_modeMenu.LoadMenu(IDR_MAINFRAME_MODE_MENUS);

	// Get the menu string that is current at the mode menu's position
	CString sCurrentMenuString;
	pMenu->GetMenuString(MODE_MENU_INSERT_POS, sCurrentMenuString, MF_BYPOSITION);

	// Check to see if a mode menu already exists on the menubar.  If so, remove it!
	int i;
	for (i=0; i < m_modeMenu.GetMenuItemCount(); i++)
	{
		// Get the menu string
		CString sMenuString;
		m_modeMenu.GetMenuString(i, sMenuString, MF_BYPOSITION);

		if (sCurrentMenuString == sMenuString)
		{
			// Remove the menu
			pMenu->RemoveMenu(MODE_MENU_INSERT_POS, MF_BYPOSITION);
			break;
		}
	}	

	// The index to the mode menu which will be added
	int nModeMenuIndex=0;

	// Figure out the index to the mode menu to add
	switch (m_WorldEditMode)
	{
	case BRUSH_EDITMODE:
		{
			nModeMenuIndex=MODE_MENU_BRUSH_INDEX;
			break;
		}
	default:
		{
			// There isn't a mode menu for this editing mode
			m_modeMenu.DestroyMenu();

			// Redraw the menu bar
			DrawMenuBar();
			return;
		}
	}	

	// Get the popup menu
	CMenu *pPopupMenu=m_modeMenu.GetSubMenu(nModeMenuIndex);	

	// Get the menu string
	CString sMenuString;
	m_modeMenu.GetMenuString(nModeMenuIndex, sMenuString, MF_BYPOSITION);

	// Add the menu and detach it from the other menu
	pMenu->InsertMenu(MODE_MENU_INSERT_POS, MF_BYPOSITION | MF_POPUP, (UINT)pPopupMenu->Detach(), sMenuString);

	// Redraw the menu bar
	DrawMenuBar();
}

/************************************************************************/
// Centers the views on the current selection
void CMainFrame::OnCenterOnSelection() 
{
	//run through all currently open documents
	for(uint32 i=0; i < m_pProjectBar->m_RegionDocs; i++ )
	{
		CRegionDoc *pRegionDoc = m_pProjectBar->m_RegionDocs[i];

		LTVector vMin = pRegionDoc->GetRegion( )->m_vMarker;
		LTVector vMax = pRegionDoc->GetRegion( )->m_vMarker;

		if( pRegionDoc->GetRegion()->m_Selections > 0 )
		{
			vMin = pRegionDoc->m_SelectionMin;
			vMax = pRegionDoc->m_SelectionMax;
		}

		//make sure that each dimension has at least some specified size
		LTVector vCenter = (vMin + vMax) / 2;

		//make them dimensions instead of points
		vMin -= vCenter;
		vMax -= vCenter;

		static const float kfMinAxisDistance = 1.0f;
		VEC_MIN(vMin, vMin, LTVector(kfMinAxisDistance, kfMinAxisDistance, kfMinAxisDistance));
		VEC_MAX(vMax, vMax, LTVector(kfMinAxisDistance, kfMinAxisDistance, kfMinAxisDistance));

		//give a little bit of a buffer space around the selection
		static const float kfBufferSpaceScale = 1.2f;
		vMin = vMin * kfBufferSpaceScale;
		vMax = vMax * kfBufferSpaceScale;

		//alright, now we need to reposition each and every view		
		for(POSITION pos = pRegionDoc->GetFirstViewPosition(); pos; )
		{
			//make sure that this is one of our views
			CView *pView = pRegionDoc->GetNextView(pos);
		
			if( !pView->IsKindOf(RUNTIME_CLASS(CRegionView)) )
			{
				continue;
			}

			CRegionView *pRegionView = (CRegionView*)pView;

			if( pRegionView->IsPerspectiveViewType() )
			{
				//for the perspective view, we keep the same forward vector, and then need to control
				//the distance from a point. We do this for each and every point and then move the position back

				//determine the smallest view angle
				CPerspectiveViewDef* pPerspViewDef = (CPerspectiveViewDef*)pRegionView->m_pViewDef;
				float fSmallestAngle = LTMIN(pPerspViewDef->GetHorzFOV(), pPerspViewDef->GetVertFOV()) / 2.0f;

				float fTanAngle = (float)fabs(tan(fSmallestAngle));

				//alright, now we need to find the largest 'move back' distance
				LTVector vBBoxPts[8];
				vBBoxPts[0].Init(vMin.x, vMin.y, vMin.z);
				vBBoxPts[1].Init(vMin.x, vMin.y, vMax.z);
				vBBoxPts[2].Init(vMin.x, vMax.y, vMin.z);
				vBBoxPts[3].Init(vMin.x, vMax.y, vMax.z);
				vBBoxPts[4].Init(vMax.x, vMin.y, vMin.z);
				vBBoxPts[5].Init(vMax.x, vMin.y, vMax.z);
				vBBoxPts[6].Init(vMax.x, vMax.y, vMin.z);
				vBBoxPts[7].Init(vMax.x, vMax.y, vMax.z);

				float fSmallestDist = 100000.0f;
				
				const LTVector& vForward = pRegionView->Nav().Forward();

				for(uint32 nCurrPt = 0; nCurrPt < 8; nCurrPt++)
				{
					float fPtDotForward = vForward.Dot(vBBoxPts[nCurrPt]);

					//find the distance along the forward vector that we would need to be
					LTVector vParallel = vForward * fPtDotForward;
					LTVector vPerp = vBBoxPts[nCurrPt] - vParallel;
					float fBoxPtDist = vPerp.Mag();

					if(fPtDotForward < 0.0f)
						fBoxPtDist = -fBoxPtDist * 2.0f;

					float fCurrDist = fBoxPtDist / fTanAngle;

					if(fCurrDist < fSmallestDist)
					{
						fSmallestDist = fCurrDist;
					}
				}

				//now all we need to do is position the camera.
				LTVector vPos = vCenter + fSmallestDist * vForward;

				pRegionView->Nav().Pos() = vPos;
			}
			else
			{
				//we are in an ortho view, what we need to do is center it on the center,
				//and find the scale we need
				LTVector vCamPos = vCenter;

				// Remove the displacement from centerPoint along normal and add in current
				// displacement...
				vCamPos += pRegionView->Nav().Forward( ) * 
					( pRegionView->Nav().Forward( ).Dot( pRegionView->Nav().Pos( ) - vCamPos ));

				//now transform that point into our space
				LTVector vTransMax = pRegionView->m_pView->m_Transform * (vMax + vCenter);
				LTVector vTransCenter = pRegionView->m_pView->m_Transform * vCenter;

				//make sure we can access the info
				if(pRegionView->m_pViewDef && pRegionView->m_pViewDef->m_pInfo)
				{
					//now make it so that the extent will map to the edge of the view
					float fMagX = pRegionView->m_pViewDef->m_pInfo->m_fHalfWidth / fabs(vTransMax.x - vTransCenter.x);
					float fMagY = pRegionView->m_pViewDef->m_pInfo->m_fHalfHeight / fabs(vTransMax.y - vTransCenter.y);

					//setup that magnify scale to whichever shows more
					pRegionView->m_pViewDef->m_Magnify = LTMIN(fMagX, fMagY);
				}

				//setup the final position
				pRegionView->Nav().Pos() = vCamPos;
			}
		}

		pRegionDoc->RedrawAllViews();
	}

//	UpdateAllRegions( REGIONVIEWUPDATE_REDRAW );
}

void CMainFrame::OnUpdateCenterOnSelection(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetNumRegionDocs() > 0 );
}

/************************************************************************/
// Centers the views at the specific coordinate.
// Pass in TRUE if you wish to center perspective views as well.		
void CMainFrame::CenterViewsAtVector(CVector vCenter, BOOL bCenterPerspective)
{
	// Get the frame
	CRegionFrame *pFrame = (CRegionFrame *)MDIGetActive();	
	if (!pFrame)
	{
		return;
	}

	// Get the active document
	CRegionDoc *pRegionDoc = (CRegionDoc *)pFrame->GetActiveDocument();
	if (!pRegionDoc)
	{
		return;
	}	
	
	// Go through each view in the document
	POSITION pos;
	for( pos = pRegionDoc->GetFirstViewPosition(); pos; )
	{
		// Get the view
		CView *pView = pRegionDoc->GetNextView(pos);
	
		// Check to see if the view is a CRegionView
		if( pView->IsKindOf(RUNTIME_CLASS(CRegionView)) )
		{
			// Get the region view
			CRegionView *pRegionView = (CRegionView*)pView;

			// Check to see if the view is a perspective view
			if( pRegionView->IsPerspectiveViewType() )
			{
				if (bCenterPerspective)
				{
					// Move the camera to the marker
					pRegionView->Nav().Pos() = vCenter;
				}
			}
			else
			{					
				// Remove the displacement from centerPoint along normal and add in current
				// displacement...
				CVector vCamPos=vCenter;
				vCamPos += pRegionView->Nav().Forward( ) * 
					( pRegionView->Nav().Forward( ).Dot( pRegionView->Nav().Pos( ) - vCamPos ));
				pRegionView->Nav().Pos() = vCamPos;
			}
		}
	}

	// Redraw all of the views in this document
	pRegionDoc->RedrawAllViews();	
}

/************************************************************************/
// Centers the views on the marker
void CMainFrame::OnCenterOnMarker() 
{				
	// Get the frame
	CRegionFrame *pFrame = (CRegionFrame *)MDIGetActive();	
	if (!pFrame)
	{
		return;
	}

	// Get the active document
	CRegionDoc *pRegionDoc = (CRegionDoc *)pFrame->GetActiveDocument();
	if (!pRegionDoc)
	{
		return;
	}

	// Center the views
	CenterViewsAtVector(pRegionDoc->GetRegion( )->m_vMarker, TRUE);	
}

/************************************************************************/
void CMainFrame::OnUpdateCenterOnMarker(CCmdUI* pCmdUI) 
{
	// Get the frame
	CRegionFrame *pFrame = (CRegionFrame *)MDIGetActive();	
	if (!pFrame)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	// Get the active document
	CRegionDoc *pRegionDoc = (CRegionDoc *)pFrame->GetActiveDocument();
	if (!pRegionDoc)
	{
		pCmdUI->Enable(FALSE);
		return;
	}
}

//SCHLEGZ 1/22/98:  Added
void CMainFrame::OnUpdateMDINext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( GetNumRegionDocs() > 1 );
}

//Given a world name, this will use the options and launch a process to run the 
//specified world name
bool CMainFrame::RunWorld(const char* pszWorldName)
{
	// The run options
	COptionsRun *pOptions=GetApp()->GetOptions().GetRunOptions();
	if (!pOptions)
	{
		return false;
	}

	// Make sure a project is open.
	if(!GetProjectBar()->VerifyProjectIsOpen())
	{
		return false;
	}		
	
	//get the name of the .dat file (essentially the name of the file, minus 
	//the extension)

	//can't just remove a certain number of characters since there could be .ed, .lta
	//etc. Should just move back and truncate after the first period -JohnO
	CString sDatFilename = pszWorldName;
	
	for(int32 nCurrChar = sDatFilename.GetLength() - 1; nCurrChar >= 0; nCurrChar--)
	{
		//don't continue any further if there is a slash
		if(sDatFilename[(int)nCurrChar] == '\\')
		{
			break;
		}
		//otherwise, if it is a period, this is the first one, and where we want to
		//truncate
		if(sDatFilename[(int)nCurrChar] == '.')
		{
			sDatFilename = sDatFilename.Left(nCurrChar);
			break;
		}
	}

	// Verify that the .dat file exists
	if (_access(sDatFilename + ".dat", 00) != 0)
	{
		MessageBox("Error: Cannot find the .dat file for your world.  Please run the processor.",
				   "Cannot find file", MB_OK | MB_ICONERROR);
		return false;
	}

	// Get the project base directory
	CString sBaseProjectDir=GetProject()->m_BaseProjectDir;
	
	// Make sure that the file is within the base project
	if (sBaseProjectDir.GetLength() > sBaseProjectDir.GetLength() ||
		sBaseProjectDir != sDatFilename.Left(sBaseProjectDir.GetLength()))
	{
		MessageBox("Error: Your .dat file must be in your project path.",
				   "Invalid file path", MB_OK | MB_ICONERROR);
		return false;
	}

	// Get the relative path to the file but subtracting the base project directory from it
	CString sRelativePath=sDatFilename.Right(sDatFilename.GetLength()-sBaseProjectDir.GetLength()-1);
		
	// Create the program arguments string by replacing the "%WorldName%" string with the world name
	CString sArguments=pOptions->GetProgramArguments();

	CString sArgumentsUpper = sArguments;
	sArgumentsUpper.MakeUpper();

	int nWorldNamePos = sArgumentsUpper.Find("%WORLDNAME%");
	if(nWorldNamePos != -1)
	{
		//we have the world name, so we need to replace it in the original string
		sArguments =	sArguments.Left(nWorldNamePos) + 
						sRelativePath + 
						sArguments.Mid(nWorldNamePos + strlen("%WORLDNAME%"));
	}

	// Create the command line
	CString sCommandLine=pOptions->GetExecutable()+" "+sArguments;

	// Get the current working directory
	char szCurrentDir[MAX_PATH];
	_getcwd((char *)szCurrentDir, MAX_PATH);

	// Change the current working directory
	if (_chdir(pOptions->GetWorkingDirectory()) != 0)
	{
		MessageBox("Error: Cannot change working directory to: \""+pOptions->GetWorkingDirectory()+"\" "+
				   "Please verify your settings in the DEdit options dialog.",
				   "Cannot change directory", MB_OK | MB_ICONERROR);
		return false;
	}

	// Minimize so it doesn't mess with our mojo
	AfxGetMainWnd()->ShowWindow(SW_MINIMIZE);

	// The create process structures
	PROCESS_INFORMATION processInfo;
	STARTUPINFO			startInfo;

	memset(&startInfo, 0, sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);		
	BOOL bRet = CreateProcess(NULL, sCommandLine.GetBuffer(0), NULL, NULL, FALSE, 0, NULL, NULL, &startInfo, &processInfo);

	// Check to see if the process launched succesfully
	if (!bRet)
	{
		MessageBox("Unable to start process. Please verify your settings in the DEdit options dialog.", "Error", MB_OK | MB_ICONERROR);
	}

	// Change the working directory back
	_chdir(szCurrentDir);

	//succes 
	return true;
}

void CMainFrame::OnRunWorld()
{
	// Get the frame
	CRegionFrame *pFrame = (CRegionFrame *)MDIGetActive();	
	if (!pFrame)
	{
		return;
	}

	// Get the active document
	CRegionDoc *pDoc = (CRegionDoc *)pFrame->GetActiveDocument();
	if (!pDoc)	
	{
		return;
	}	

	// The world filename
	CString sFilename = pDoc->m_FileName;

	//Now run the world
	RunWorld(sFilename);
}


void CMainFrame::OnUpdateRunWorld(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetNumRegionDocs() > 0);
}


void CMainFrame::OnUpdateViewDebugWindow(CCmdUI *pCmdUI)
{
	//pCmdUI->Enable(GetDebugDlg()->IsWindowVisible());
	pCmdUI->SetCheck(GetDebugDlg()->IsWindowVisible());
}


void CMainFrame::OnViewDebugWindow()
{
	if(GetDebugDlg()->IsWindowVisible())
		GetDebugDlg()->ShowWindow(SW_HIDE);
	else
		GetDebugDlg()->ShowWindow(SW_SHOW);
}


void CMainFrame::OnUpdateViewTexturePalette(CCmdUI *pCmdUI)
{
	if( GetProjectBar( )->IsProjectOpen() )
	{
		pCmdUI->SetCheck(IsWindow(GetTextureDlg()->m_AllTextureDlg.m_hWnd));
	}
}


void CMainFrame::OnViewTexturePalette()
{
	if( GetProjectBar( )->VerifyProjectIsOpen() )
	{
		if( IsWindow( GetTextureDlg()->m_AllTextureDlg.m_hWnd ))
		{
			GetTextureDlg()->m_AllTextureDlg.DestroyWindow();
		}
		else
		{
			GetTextureDlg()->m_AllTextureDlg.Create(IDD_ALLTEXTUREDLG, this );
			GetTextureDlg()->m_AllTextureDlg.ShowWindow( SW_SHOW );
		}
	}
}


void CMainFrame::OnUpdateSingleNodeSelection( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( GetNumRegionDocs() > 0 );
	pCmdUI->SetCheck( m_NodeSelectionMode == SINGLE_SELECTION );
}

void CMainFrame::OnSingleNodeSelection()
{
	DWORD			i;

	
	m_NodeSelectionMode = SINGLE_SELECTION;

	for( i=0; i < m_pProjectBar->m_RegionDocs; i++ )
	{
		// if we have more than one item selected, we need to clear the selections
		if (m_pProjectBar->m_RegionDocs[i]->GetRegion()->GetNumSelections() > 1)
		{
			m_pProjectBar->m_RegionDocs[i]->GetRegion()->ClearSelections();
			m_pProjectBar->m_RegionDocs[i]->NotifySelectionChange();
			m_pProjectBar->m_RegionDocs[i]->RedrawAllViews();
		}
	}
}

void CMainFrame::OnUpdateMultiNodeSelection( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( (GetNumRegionDocs() > 0) );
	pCmdUI->SetCheck( m_NodeSelectionMode == MULTI_SELECTION );
}

void CMainFrame::OnMultiNodeSelection()
{
	DWORD			i;

	m_NodeSelectionMode = MULTI_SELECTION;
	
	// No longer clear selections
	/*
	for( i=0; i < m_pProjectBar->m_RegionDocs; i++ )
	{
		m_pProjectBar->m_RegionDocs[i]->GetRegion()->ClearSelections();
		m_pProjectBar->m_RegionDocs[i]->NotifySelectionChange();
		m_pProjectBar->m_RegionDocs[i]->RedrawAllViews();
	}
	*/
}



void CMainFrame::ActivateFrame(int nCmdShow) 
{
	CMRCMDIFrameWndSizeDock::ActivateFrame(nCmdShow);
}

void CMainFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMRCMDIFrameWndSizeDock::OnSetFocus(pOldWnd);
	
}


//-----------------------------------------------------------------------------
//
//  CMainFrame::SetTooltips()
//
//  Purpose:	Sets tooltips flags in toolbars.
//
//-----------------------------------------------------------------------------
void CMainFrame::SetToolTips() 
{	
	// Toggle tooltips...
	if (m_bToolTips)
	{
		m_wndMainBar.SetBarStyle(m_wndMainBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
		m_NodeViewToolbar.SetBarStyle(m_NodeViewToolbar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
		m_WorldEditToolbar.SetBarStyle(m_WorldEditToolbar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	}
	else
	{
		m_wndMainBar.SetBarStyle(m_wndMainBar.GetBarStyle() & ~(CBRS_TOOLTIPS | CBRS_FLYBY));
		m_NodeViewToolbar.SetBarStyle(m_NodeViewToolbar.GetBarStyle() & ~( CBRS_TOOLTIPS | CBRS_FLYBY));
		m_WorldEditToolbar.SetBarStyle(m_WorldEditToolbar.GetBarStyle() & ~( CBRS_TOOLTIPS | CBRS_FLYBY));
	}
}

//-----------------------------------------------------------------------------
//
//  CMainFrame::OnViewTooltips()
//
//  Purpose:	Handles toggle of tool tips.
//
//-----------------------------------------------------------------------------
void CMainFrame::OnViewTooltips() 
{
	m_bToolTips = !m_bToolTips;
	SetToolTips( );
}

//-----------------------------------------------------------------------------
//
//  CMainFrame::OnUpdateViewTooltips()
//
//  Purpose:	Handles updating menu for tooltips check.
//
//-----------------------------------------------------------------------------
void CMainFrame::OnUpdateViewTooltips( CCmdUI* pCmdUI ) 
{
	pCmdUI->SetCheck( m_bToolTips );
}


//-----------------------------------------------------------------------------
//
//  CMainFrame::CountUndo
//
//  Purpose:	Decrements an undo counter until zero, then resets value.
//				Returns TRUE when hits zero.
//
//-----------------------------------------------------------------------------
BOOL CMainFrame::CountUndo( UINT &iUndo, UINT &iResetVal )
{
	if( iUndo )
		iUndo--;

	if( iUndo == 0 && iResetVal )
	{
		iUndo = iResetVal;
		return TRUE;
	}

	return FALSE;
}

BOOL CMainFrame::CountUndoGeometry( )
{
	return CountUndo( m_iCurUndoGeometry, m_iUndoGeometry );
}

BOOL CMainFrame::CountUndoTextures( )
{
	return CountUndo( m_iCurUndoTextures, m_iUndoTextures );
}

BOOL CMainFrame::CountUndoProperties( )
{
	return CountUndo( m_iCurUndoProperties, m_iUndoProperties );
}

BOOL CMainFrame::CountUndoGeneral( )
{
	return CountUndo( m_iCurUndoGeneral, m_iUndoGeneral );
}

//-----------------------------------------------------------------------------
//
//  CMainFrame::SetUndoPreferencesX
//
//  Purpose:	Sets undo preferences.
//
//-----------------------------------------------------------------------------
void CMainFrame::SetUndoPreferencesDefault( UINT &iGeometry, UINT &iTextures, UINT &iProperties, UINT &iGeneral )
{
	iGeometry = 1;
	iTextures = 1;
	iProperties = 1;
	iGeneral = 1;
}

void CMainFrame::SetUndoPreferencesCoarse( UINT &iGeometry, UINT &iTextures, UINT &iProperties, UINT &iGeneral )
{
	iGeometry = 5;
	iTextures = 10;
	iProperties = 10;
	iGeneral = 10;
}

void CMainFrame::OnSelectTab(UINT id)
{
	int iTab;
	CProjectBar *pBar;


	pBar = GetProjectBar();

	// This handles the accelerators that select project tabs (Ctrl+1 - Ctrl+9).
	iTab = pBar->FindTabBySelectID(id);
	if(iTab != -1)
	{
		pBar->SetTab(pBar->m_TabInfo[iTab]->m_ControlType);
	}
}


void CMainFrame::OnToggleEditMode() 
{
	// Make sure that it is okay to change the mode by calling
	// the message handler directly
	if (GetNumRegionDocs() <= 0)
	{
		return;
	}

	switch (m_WorldEditMode)
	{
	case BRUSH_EDITMODE:
		{
			OnGeometryEditMode();
			break;
		}
	case GEOMETRY_EDITMODE:
		{
			OnObjectEditMode();
			break;
		}
	case OBJECT_EDITMODE:
		{
			OnBrushEditMode();
			break;
		}
	default:
		{
			ASSERT(FALSE);
		}
	}
}

/************************************************************************/
// This toggles the display of the floating control bar windows
void CMainFrame::OnViewToggleFloatingToolbars() 
{
//	POSITION pos = m_listControlBars.GetHeadPosition();
//	while (pos != NULL)
//	{
//		CControlBar* pControlBar = (CControlBar*)m_listControlBars.GetNext(pos);
//		ASSERT_VALID(pControlBar);		

	int i;
	for (i=0; i < m_projectControls.GetSize(); i++)
	{
		CControlBar *pControlBar=m_projectControls[i]->m_pWnd;

		// Check to see if it is floating
		if (pControlBar->IsFloating())
		{
			// Check to see if it is visible
			if(pControlBar->IsWindowVisible())
			{
				// Hide the control bar
				ShowControlBar( pControlBar, FALSE, FALSE );
			}
			else
			{
				// Show the control bar
				ShowControlBar( pControlBar, TRUE, FALSE );
			}
		}
	}

	// Check the project bar
	if (m_pProjectBar->IsFloating())
	{
		if (m_pProjectBar->IsWindowVisible())
		{
			ShowControlBar(m_pProjectBar, FALSE, FALSE);
		}
		else
		{
			ShowControlBar(m_pProjectBar, TRUE, FALSE);
		}
	}
}


//this message is sent whenever a model manager completes loading up a model,
//this indicates that the views need to be redrawn, and the memory management
//on the model manager needs to be performed
LRESULT CMainFrame::OnModelLoaded(WPARAM, LPARAM lParam)
{
	//update the memory for the model manager
	if(GetApp()->GetOptions().GetModelsOptions()->IsLimitMemoryUse())
	{
		GetApp()->GetModelMgr().SetMaxMemoryUsage(GetApp()->GetOptions().GetModelsOptions()->GetMaxMemoryUse());
	}

	//now we need to update the textures for the model that was just loaded
	CModelMesh *pMesh = (CModelMesh*)lParam;

	for(uint32 nCurrShape = 0; nCurrShape < pMesh->GetShapeList()->GetNumShapes(); nCurrShape++)
	{
		CMeshShape* pShape = pMesh->GetShapeList()->GetShape(nCurrShape);

		//now update the texture
		if(pShape->GetTextureFilename())
		{
			const char* pszFilename = pShape->GetTextureFilename();

			//see if we need to skip over any slashes
			if((pszFilename[0] == '\\') || (pszFilename[0] == '/'))
				pszFilename++;

			//try and load in the image
			if(dfm_GetFileIdentifier(GetFileMgr(), pszFilename, &pShape->m_pTextureFile) != DFM_OK)
			{
				//we have failed
				pShape->m_pTextureFile = NULL;		
			}
		}
	}

	//now tell all the views to update
	POSITION Position;
	for( Position = GetApp()->m_pWorldTemplate->GetFirstDocPosition(); Position != NULL; )
	{
		CRegionDoc* pDoc = (CRegionDoc*)GetApp()->m_pWorldTemplate->GetNextDoc( Position );
		if( pDoc != NULL )
		{
			pDoc->RedrawAllViews();
		}
	}	

	return 0;
}

void CMainFrame::OnRenameResources()
{
	GetRenameResourceDlg()->ShowWindow(SW_SHOW);
}

void CMainFrame::OnUpdateRenameResources(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetProjectBar() && GetProjectBar()->IsProjectOpen());
}

void CMainFrame::OnUpdateWorldObjects()
{
	UpdateAllLevelObjects();
}

void CMainFrame::OnUpdateUpdateWorldObjects( CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetProjectBar() && GetProjectBar()->IsProjectOpen());
}

//-----------------------------------------------------------------------------
//
//  CMainFrame::OnIgnoreMsgsInPrefabs()
//
//  Purpose:	Handles toggle of ignore prefab properties.
//
//-----------------------------------------------------------------------------

void CMainFrame::OnIgnoreMsgsInPrefabs() 
{
	m_bIgnoreMsgsInPrefabs = !m_bIgnoreMsgsInPrefabs;
}

//-----------------------------------------------------------------------------
//
//  CMainFrame::OnUpdateIgnoreMsgsInPrefabs()
//
//  Purpose:	Handles updating menu for ignore prefab properties check.
//
//-----------------------------------------------------------------------------

void CMainFrame::OnUpdateIgnoreMsgsInPrefabs( CCmdUI* pCmdUI ) 
{
	pCmdUI->SetCheck( m_bIgnoreMsgsInPrefabs );
}