#include "bdefs.h"
#include "dedit.h"
#include "projectbar.h"
#include "resource.h"
#include "mainfrm.h"
#include "stringdlg.h"
#include "spriteeditdlg.h"
#include "regionview.h"
#include "regionframe.h"
#include "texture.h"
#include "sysstreamsim.h"
#include "resnewdir.h"
#include "model.h"
#include "mainfrm.h"
#include "optionswindows.h"
#include "prefabdlg.h"
#include "ltamgr.h"
#include "optionsmisc.h"
#include "renameresourcedlg.h"
#include "optionsrun.h"

#include <direct.h>		//for _mkdir

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




/////////////////////////////////////////////////////////////////////////////
// CProjectBar

BEGIN_MESSAGE_MAP(CProjectBar, CMRCSizeDialogBar)
	//{{AFX_MSG_MAP(CProjectBar)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_PROJECTVIEW_TABS, OnTabsSelChange)
	ON_COMMAND(ID_POPUP_IMPORTTEXTURES, OnImportTextures)
	ON_COMMAND(ID_POPUP_IMPORTPCX, OnImportPcxFiles)
	ON_COMMAND(ID_POPUP_IMPORTMIPPCX, OnImportMipPcxFiles)
	ON_COMMAND(ID_IMPORT_TGA_FILES, OnImportTgaFiles)
	ON_COMMAND(ID_IMPORT_BUMPMAP, OnImportBumpMap)
	ON_COMMAND(ID_IMPORT_NORMALMAP, OnImportNormalMap)
	ON_COMMAND(ID_IMPORT_CUBEMAP, OnImportCubeMap)
	ON_COMMAND(ID_POPUP_IMPORTALPHAMASK, OnImportAlphaMask)
	ON_COMMAND(ID_IMPORT_8BIT_ALPHA, OnImport8BitAlphaMask)
	ON_COMMAND(ID_POPUP_CREATEALPHAFROMCOLOR, OnCreateAlphaFromColor)
	ON_COMMAND(ID_POPUP_CREATESOLIDALPHA, OnCreateSolidAlpha)
	ON_COMMAND(ID_POPUP_EXPORTPCX, OnExportPcxFile)
	ON_COMMAND(ID_POPUP_TEXTUREPROP, OnTextureProperties)
	ON_COMMAND(ID_POPUP_VIEWALLTEXTURES, OnViewAllTextures)
	ON_COMMAND(ID_POPUP_BATCHRELOAD, OnBatchReload)
	ON_COMMAND(ID_POPUP_SCALETEXTURECOORDS, OnScaleTextureCoords)
	ON_COMMAND(ID_POPUP_IMPORTSOUNDS, OnImportSounds)
	ON_COMMAND(ID_POPUP_IMPORTMODELS, OnImportModels)
	ON_COMMAND(ID_POPUP_CREATENEWSPRITE, OnPopupCreateNewSprite)
	ON_COMMAND(ID_POPUP_UPDATE, OnPopupUpdate)
	ON_COMMAND(ID_WORLD_ADDDIR, OnAddDir)
	ON_COMMAND(ID_PHYSICS_ADDDIRECTORY, OnAddDir)
	ON_COMMAND(ID_PREFAB_OPEN, OnPrefabOpen)
	ON_COMMAND(ID_REPLACE_SELECTED_PREFABS, OnReplaceSelectedPrefabs)
	ON_COMMAND(ID_RENAME_PREFAB, OnRenamePrefab)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_POPUP_ADDTOPALETTE, OnAddToPalette)
	ON_COMMAND(ID_POPUP_EXPORTDTX8_24, OnExport8BitDTXFile)
	ON_COMMAND(ID_POPUP_EXPORTDTX_ALL_AS_BPP32P, OnExportAllAsBPP_32P)
	ON_COMMAND(ID_CONVERT_TO_32P, OnConvertTo32P)
	ON_COMMAND(ID_RENAME_TEXTURE, OnRenameTexture)
	ON_COMMAND(ID_POPUP_FINDTEXINWORLD, OnFindTextureInWorld)
	ON_COMMAND(ID_POPUP_MAKETEXTUREWRITABLE, OnMakeTextureWritable)
	ON_COMMAND(ID_POPUP_SHOWTHUMBNAILS, OnShowThumbnails)
	ON_UPDATE_COMMAND_UI(ID_POPUP_SHOWTHUMBNAILS, OnUpdateShowThumbnails)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()




CProjectBar::CProjectBar()
{
	m_pCurTextureSel		= NULL;
	m_ProjectFileName[0]	= 0;
	m_TabInfo.SetSize(0);
	m_pTabImagesBitmap		= new CBitmap;
	m_bShowThumbnails		= true;
	m_bShowIcons			= true;
}


CProjectBar::~CProjectBar()
{
	DeleteAndClearArray(m_TabInfo);

	if(m_pTabImagesBitmap)
		delete m_pTabImagesBitmap;
}

BOOL CProjectBar::Init(CMainFrame *pFrame)
{
	CRect			rect;
	DWORD			i;
	CString			str;

	m_bShowThumbnails = true;

	// Set the project bar for the tab control
	m_Tabs.SetProjectBar(this);

	// Get the window options
	COptionsWindows *pOptions=GetApp()->GetOptions().GetWindowsOptions();

	// Setup our tab window list.
	DeleteAndClearArray(m_TabInfo);
	for (i=0; i < pOptions->GetNumBarControls(); i++)
	{
		pFrame->AddProjectControlInfo(m_TabInfo, pOptions->GetBarControl(i));
	}

	static CString strWndClass;
	if (strWndClass.IsEmpty())
	{
		strWndClass = AfxRegisterWndClass(CS_DBLCLKS);
	}

	if(!CMRCSizeDialogBar::Create(pFrame,IDD_PROJECTBAR_DLG,
	   /*WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | */CBRS_LEFT | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_HIDE_INPLACE | CBRS_SIZE_DYNAMIC,
									   ID_VIEW_PROJECT_WINDOW))
		return FALSE;

	EnableDocking( CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT );

	str.LoadString( IDS_PROJECTBAR_TITLE );
	SetWindowText( str );

	m_Style |= SZBARF_HIDE_ON_CLOSE; // We want this bar to hide instead of dock

	// Setup all the tabs and stuff.
	GetClientRect( &rect );

	m_Tabs.Create(WS_CHILD|WS_VISIBLE|TCS_TOOLTIPS|TCS_MULTILINE, rect,
		this, IDC_PROJECTVIEW_TABS );

	// Load tab images
	m_TabImages.Create(20, 20, ILC_COLOR32 | ILC_MASK, 11, 1);
	m_pTabImagesBitmap->LoadBitmap(IDB_PROJECT_ICONS);
	m_TabImages.Add(m_pTabImagesBitmap, RGB(0,255,0));

	m_Tabs.SetImageList( &m_TabImages );
	m_Tabs.SetPadding( CSize(4,4) );
	m_Tabs.SetItemSize( CSize(22,22) );

	// Use the status bar's font
	m_Tabs.SetFont(pFrame->GetStatusBar()->GetFont());

	m_ToolTips.Create( this, TTS_ALWAYSTIP );
	//m_Tabs.SetTooltips( &m_ToolTips );
	//m_Tabs.EnableToolTips( FALSE );


	TC_ITEM item;
	memset( &item, 0, sizeof(item) );
	item.mask = TCIF_TEXT | TCIF_IMAGE;
	CString csTemp;

	// Load Icon info
	m_bShowIcons = GetApp()->GetOptions().GetMiscOptions()->GetShowIcons();

	// Setup the tabs.
	for(i=0; i < m_TabInfo; i++)
	{
		csTemp.LoadString(m_TabInfo[i]->m_TitleStringID);
		item.pszText = csTemp.GetBuffer(0);
		item.cchTextMax = csTemp.GetLength();
		if (m_bShowIcons)
			item.iImage = m_TabInfo[i]->m_ControlType;  // Specific icon for tab
		else
			item.iImage = CMainFrame::CB_LAST_CONTROL_INDEX;  // Neutral icon
		m_Tabs.InsertItem( i, &item );

		// Create the dialog.
		CMRCSizeDialogBar *pDlgBar=(CMRCSizeDialogBar *)m_TabInfo[i]->m_pWnd;
		pDlgBar->Create(&m_Tabs, m_TabInfo[i]->m_DialogID, CBRS_NOALIGN, m_TabInfo[i]->m_SelectID);
		pDlgBar->ShowWindow(SW_HIDE);
	}

	// Select the first tab
	m_Tabs.SetCurSel(0);

	// Create the debug window
	m_DebugDlg.Create( IDD_DEBUGDLG, pFrame );

	// Show the first window
	if (m_TabInfo.GetSize() > 0)
	{
		m_pVisibleDlg = m_TabInfo[0]->m_pWnd;
		m_pVisibleDlg->ShowWindow( SW_SHOW );
	}
	else
	{
		m_pVisibleDlg=NULL;
	}

	ResizeElements();
	UpdateAll();

	Invalidate();
	UpdateWindow();

	return TRUE;
}

void CProjectBar::Term()
{
	//we also need to make sure that the rename resource dialog is no longer visible
	//so that user's can mix up resources going between projects
	GetRenameResourceDlg()->FreeAllItems();
	GetRenameResourceDlg()->ShowWindow(SW_HIDE);

	m_pCurTextureSel = NULL;
	ClearAll();
	CloseAllRegionDocs( );
	GetProject()->Term();
}

/************************************************************************/
// This destroys the control windows that the project bar owns
void CProjectBar::DestroyControlWindows()
{
	int i;
	for (i=0; i < m_TabInfo.GetSize(); i++)
	{
		m_TabInfo[i]->m_pWnd->DestroyWindow();
	}
}

//this will take the specified resource identifier, and write it out as the
//file specified
bool CProjectBar::CreateFileFromResource(LPCTSTR pResourceID, LPCTSTR pResType, const char* pszFilename)
{
	//Note: I really feel like these resources should be freed, but all the MSDN
	//documentation says that it is uneeded, and the functions to free the
	//resources are under the obsolete section

	//the resource handle we will be loading
	HRSRC hRes = ::FindResource(GetApp()->m_hInstance, pResourceID, pResType);

	//get the size of the resource
	DWORD	nSize = ::SizeofResource(GetApp()->m_hInstance, hRes);

	//check for failure
	if(nSize == 0)
	{
		AfxMessageBox("There was an error getting the size of a resource");
		return false;
	}

	//get the memory associated with the resource
	HGLOBAL hGlob = ::LoadResource(GetApp()->m_hInstance, hRes);

	LPVOID pData = ::LockResource(hGlob);

	//check for failure
	if(pData == NULL)
	{
		AfxMessageBox("There was an error getting the data of a resource");
		return false;
	}

	//now we can write out the data to disk
	FILE* pFile = fopen(pszFilename, "wb");

	//make sure that we opened the file
	if(pFile == NULL)
	{
		CString sError;
		sError.Format("Unable to open up the file %s for writing", pszFilename);
		AfxMessageBox(sError);
		return false;
	}

	//alright, we have the file, so write it out
	fwrite(pData, nSize, 1, pFile);

	//close up and bail
	fclose(pFile);

	return true;
}


//utility struct for AutoExtractClassIcons
struct SClassIconInfo
{
	const char* m_pszFilename;
	DWORD		m_nResource;
};


//this will find the class icons directory that is specified, and if it does not
//exist, it will create it, and add all the appropriate icons into it
void CProjectBar::AutoExtractClassIcons()
{
	//first we need to find the directory that we will be checking
	CString sFile(GetApp()->GetOptions().GetDisplayOptions()->GetClassIconsDir());

	//make sure that we don't have a leading slash
	sFile.TrimLeft("\\/");

	//make sure we have a trailing slash
	sFile.TrimRight("\\/");
	if(!sFile.IsEmpty())
	{
		sFile += "\\";
	}

	//the current directory we are trying to add
	CString sCurrDir(::dfm_GetBaseDir(GetFileMgr()));
	sCurrDir += '\\';

	//now we need to make sure that all the directories exist
	CString sDirList(sFile);
	while(1)
	{
		int32 nDirPos = sDirList.FindOneOf("\\/");

		//see if there are any more directories
		if(nDirPos == -1)
		{
			break;
		}

		//more directories
		CString sDir = sDirList.Left(nDirPos + 1);
		sDirList = sDirList.Mid(nDirPos + 1);

		//tack on the directory to the end, and create it
		sCurrDir += sDir;
		_mkdir(sCurrDir);
	}

	//now we need to check each and every icon to see if it exists
	SClassIconInfo Icons[] = {	{ "DemoSkyWorldModel.dtx", IDR_CICON_DEMOSKYWORLDMODEL },
								{ "DirLight.dtx", IDR_CICON_DIRLIGHT },
								{ "InsideDef.dtx", IDR_CICON_INSIDEDEF },
								{ "Light.dtx", IDR_CICON_LIGHT },
								{ "ObjectLight.dtx", IDR_CICON_OBJECTLIGHT },
								{ "SkyPointer.dtx", IDR_CICON_SKYPOINTER },
								{ "Terrain.dtx", IDR_CICON_TERRAIN } };

	uint32 nNumIcons = sizeof(Icons) / sizeof(Icons[0]);

	//now check each icon
	for(uint32 nCurrIcon = 0; nCurrIcon < nNumIcons; nCurrIcon++)
	{
		//build the filename
		CString sIconFile = sCurrDir + Icons[nCurrIcon].m_pszFilename;

		//see if we can open it
		FILE* pFile = fopen(sIconFile, "rb");
		if(pFile)
		{
			//already exists
			fclose(pFile);
			continue;
		}

		//doesn't exist, lets make it
		CreateFileFromResource(MAKEINTRESOURCE(Icons[nCurrIcon].m_nResource), "DTX", sIconFile);
	}

}

BOOL CProjectBar::Open( const char *pFilename )
{
	CMoFileIO file;
	char pathBuffer[MAX_PATH], fileBuffer[MAX_PATH], paletteFilename[MAX_PATH];
	DStream *pStream;

	CHelpers::ExtractPathAndFileName( pFilename, pathBuffer, fileBuffer );

	BeginWaitCursor( );

	Term( );

	// Really load the project.
	if( file.Open(pFilename, "rb") )
	{
		m_Project.LoadProjectFile( file );
		file.Close();
	}
	else
	{
		AppMessageBox( IDS_ERR_OPENFILE, MB_OK );
		return FALSE;
	}

	// Init the directory names in the project.
	m_Project.OnOpenProject(pathBuffer);

	UpdateAll();

	CString sentryFile = pathBuffer;

	//add the class icons if we need to
	if(GetApp()->GetOptions().GetMiscOptions()->IsAutoExtractIcons())
	{
		AutoExtractClassIcons();
	}

	// Load the layout file.
	if(GetApp()->GetOptions().GetMiscOptions()->IsLoadLYTFile())
		LoadLayoutFile( pFilename );

	strcpy( m_ProjectFileName, pFilename );

	// Try to load the project-based run options
	GetApp()->GetOptions().GetRunOptions()->LoadFromOptionsFile();

	// Finish up
	UpdateAll();

	EndWaitCursor( );

	return TRUE;
}



BOOL CProjectBar::Save()
{
	CMoFileIO		file;

	if( strlen(m_ProjectFileName) > 0 )
	{
		// Save the project file.
		if( file.Open(m_ProjectFileName, "wb") )
		{
			GetProject()->SaveProjectFile( file );
			file.Close();
		}

		SaveLayoutFile( m_ProjectFileName );

		// Close texture palette and save if necessary
		if( IsWindow( GetTextureDlg()->m_AllTextureDlg.m_hWnd ))
			GetTextureDlg()->m_AllTextureDlg.DestroyWindow(false);

		// Save the project-based run options
		GetApp()->GetOptions().GetRunOptions()->SaveToOptionsFile();
	}

	return true;
}


BOOL CProjectBar::Close()
{
	if( !Save( ))
		return FALSE;

	if( !CloseAllRegionDocs( ))
		return FALSE;

	Term();
	memset(m_ProjectFileName, 0, sizeof(m_ProjectFileName));

	return TRUE;
}


BOOL CProjectBar::CloseAllRegionDocs( )
{
	// Close ALL region documents.
	while( m_RegionDocs > 0 )
	{
		CRegionDoc *pDoc = m_RegionDocs[0];

		if( !pDoc->SaveModified())
			return FALSE;
		GetApp()->m_pWorldTemplate->RemoveDocument( pDoc );
		pDoc->OnCloseDocument();
	}

	return TRUE;
}

BOOL CProjectBar::IsProjectOpen()
{
	return strlen(m_ProjectFileName) > 0;
}


BOOL CProjectBar::VerifyProjectIsOpen()
{
	if( IsProjectOpen() )
	{
		return TRUE;
	}
	else
	{
		AppMessageBox( IDS_NOPROJECTOPEN, MB_OK );
		return FALSE;
	}
}


void CProjectBar::ResizeElements()
{
	CRect rcWnd,rect, rcItem;
	DWORD i;


	if( ::IsWindow(m_Tabs.m_hWnd) && ::IsWindow(m_hWnd) )
	{
		// the rect of the dialog bar
		GetClientRect(&rcWnd);

		rcWnd.DeflateRect( 6, 6 );
		m_Tabs.MoveWindow( &rcWnd, FALSE );

		// move them
		m_Tabs.GetItemRect(m_Tabs.GetCurSel(),&rect);
		m_Tabs.GetClientRect(&rcWnd);
		rcWnd.top = rect.top;

		m_Tabs.GetItemRect( 0, &rcItem );
		rcWnd.top += rcItem.Height();
		rcWnd.DeflateRect( 3, 3 );

		for(i=0; i < m_TabInfo; i++)
		{
			if( ::IsWindow(m_TabInfo[i]->m_pWnd->m_hWnd) )
				m_TabInfo[i]->m_pWnd->MoveWindow( &rcWnd, TRUE );
		}
	}

}


void CProjectBar::UpdateAll()
{
	if(IsProjectOpen())
	{
		UpdateTextureDlg();
		UpdatePropertiesDlg();
		UpdateTextureIDs();
		UpdateWorldsDlg( );
		UpdateModelDlg();
		UpdateSoundDlg();
		UpdateSpriteDlg();
		UpdatePrefabDlg();
		GetClassListDlg()->Update();
	}
}

void CProjectBar::ClearAll()
{
	GetTextureDlg()->ClearAll();
	UpdatePropertiesDlg();
	UpdateTextureIDs();
	GetWorldsDlg()->ClearAll();
	GetModelDlg()->ClearAll();
	GetSoundDlg()->ClearAll();
	GetSpriteDlg()->ClearAll();
	GetPrefabDlg()->ClearAll();
	GetClassListDlg()->ClearAll();
}

void CProjectBar::UpdateTextureDlg()
{
	GetTextureDlg()->UpdateDirectories();
}

void CProjectBar::UpdatePropertiesDlg( )
{
	GetPropertiesDlg()->Redraw();
}


void CProjectBar::UpdateWorldsDlg()
{
	GetWorldsDlg()->UpdateDirectories( );
}

void CProjectBar::UpdateNodeView()
{
	GetNodeView()->Update( );
}

void CProjectBar::UpdateModelDlg()
{
	GetModelDlg()->UpdateDirectories( );
}

void CProjectBar::UpdateSoundDlg()
{
	GetSoundDlg()->UpdateDirectories( );
}

void CProjectBar::UpdateSpriteDlg()
{
	GetSpriteDlg()->UpdateDirectories( );
}

void CProjectBar::UpdatePrefabDlg()
{
	GetPrefabDlg()->UpdateDirectories( );
}

void CProjectBar::OnSize(UINT nType, int cx, int cy)
{
	CMRCSizeDialogBar::OnSize(nType, cx, cy);

	ResizeElements();
}


void CProjectBar::OnTabsSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	CMainFrame::ProjectControl controlType = m_TabInfo[m_Tabs.GetCurSel()]->m_ControlType;

	SetTab( controlType );

	*pResult = 0;
	ResizeElements();
}

void CProjectBar::SetTab(CMainFrame::ProjectControl controlType)
{
	if (m_pVisibleDlg)
	{
		m_pVisibleDlg->ShowWindow( SW_HIDE );
	}

	// Set the visible dialog
	BOOL bFound=FALSE;
	int nTabIndex=0;

	int i;
	for (i=0; i < m_TabInfo.GetSize(); i++)
	{
		if (controlType == m_TabInfo[i]->m_ControlType)
		{
			m_pVisibleDlg = m_TabInfo[i]->m_pWnd;
			nTabIndex=i;
			bFound=TRUE;
			break;
		}
	}
	ASSERT(bFound);

	if (m_pVisibleDlg)
	{
		m_pVisibleDlg->ShowWindow( SW_SHOW );
	}

	m_Tabs.SetCurSel(nTabIndex);
}

/************************************************************************/
// This undocks a tab and docks it to the mainframe
void CProjectBar::UndockTab(int nIndex, CPoint position, bool bHide)
{
	// The info for thie index
	CProjectControlBarInfo *pInfo=m_TabInfo[nIndex];

	// Get the control bar
	CMRCSizeDialogBar *pControlBar=pInfo->m_pWnd;

	// Terminate the tab
	SpecialCaseTabTerm(pControlBar, pInfo->m_ControlType);

	// Destroy the control bar window
	pControlBar->DestroyWindow();

	// Get the main frame
	CMainFrame *pFrame=GetMainFrame();

	// Create the control bar
	if (pFrame->CreateProjectControlBar(pInfo->m_ControlType) != pControlBar)
	{
		ASSERT(FALSE);
	}

	if (bHide)  // Make floating tab invisible
		pFrame->ShowControlBar(pControlBar, FALSE, FALSE);

	// Dock the window to the mainframe
	pFrame->FloatControlBar(pControlBar, position);

	// Make sure that the window gets properly sized
	pControlBar->RepositionControls();

	// Do special case initialization for this tab
	SpecialCaseTabInit(pControlBar, pInfo->m_ControlType);

	// Delete the tab info for this window
	delete m_TabInfo[nIndex];
	m_TabInfo.Remove(nIndex);

	// Remove the tab for this window
	m_Tabs.DeleteItem(nIndex);

	// There is not a visible dialog anymore (until a new tab is selected)
	m_pVisibleDlg=NULL;

	// Set the new tab
	int nTabs=m_TabInfo.GetSize();
	if (nTabs > 0)
	{
		if (nTabs <= nIndex)
		{
			// Go to the last tab
			SetTab(m_TabInfo[nTabs-1]->m_ControlType);
		}
		else
		{
			// Go the next tab in line
			SetTab(m_TabInfo[nIndex]->m_ControlType);
		}
	}

	// Redraw
	Invalidate();
	UpdateWindow();
}

/************************************************************************/
// This takes an undocked control bar and inserts it into the tab dialog
void CProjectBar::DockTab(CWnd *pWnd, CMainFrame::ProjectControl controlType)
{
	// Get the control bar
	CMRCSizeDialogBar *pControlBar=(CMRCSizeDialogBar *)pWnd;

	// Terminate the window
	SpecialCaseTabTerm(pControlBar, controlType);

	// Destroy the control bar window
	CWnd *pDestroyedWindow=GetMainFrame()->DestroyProjectControlBar(controlType);
	ASSERT(pDestroyedWindow);
	ASSERT(pDestroyedWindow == pControlBar);

	// Add the tab info
	CProjectControlBarInfo *pTabInfo=GetMainFrame()->AddProjectControlInfo(m_TabInfo, controlType);

	// Make sure that the correct window was added
	ASSERT(pTabInfo->m_pWnd->GetSafeHwnd() == pControlBar->GetSafeHwnd());

	// Create a new control bar window and attach it to the tab dialog
	pControlBar->Create(&m_Tabs, pTabInfo->m_DialogID, CBRS_NOALIGN, pTabInfo->m_SelectID);
	pControlBar->ShowWindow(SW_HIDE);

	// Do special case initialization for this tab
	SpecialCaseTabInit(pControlBar, controlType);

	// Add the tab to the tab control
	int nTabIndex=m_TabInfo.GetSize()-1;

	TC_ITEM item;
	memset( &item, 0, sizeof(item) );
	item.mask = TCIF_TEXT | TCIF_IMAGE;

	CString sTemp;
	sTemp.LoadString(pTabInfo->m_TitleStringID);
	item.pszText = sTemp.GetBuffer(0);
	item.cchTextMax = sTemp.GetLength();

	// Set the item's image index
	if (m_bShowIcons)
		item.iImage = m_TabInfo[nTabIndex]->m_ControlType;  // Specific icon for tab
	else
		item.iImage = CMainFrame::CB_LAST_CONTROL_INDEX;  // Neutral icon

	m_Tabs.InsertItem(nTabIndex, &item );

	// Resize the items
	ResizeElements();

	// Select tab
	SetTab(m_TabInfo[nTabIndex]->m_ControlType);

	// Redraw
	Invalidate();

	// Save the settings in the registry
	GetApp()->GetOptions().GetWindowsOptions()->SetBarControlArray(m_TabInfo);
}

/************************************************************************/
// Do special case initialization (for docking and undocking)
void CProjectBar::SpecialCaseTabInit(CMRCSizeDialogBar *pControlBar, CMainFrame::ProjectControl controlType)
{
	// Get the region doc
	CRegionDoc *pDoc=GetActiveRegionDoc();

	// Switch on the tab type
	switch (controlType)
	{
	case CMainFrame::CB_WORLDSVIEW:		// The world view
		{
			UpdateWorldsDlg();
			break;
		}
	case CMainFrame::CB_TEXTUREVIEW:	// The texture view
		{
			UpdateTextureDlg();
			break;
		}
	case CMainFrame::CB_NODESVIEW:
		{
			// The node view does not require any special case initialization
			break;
		}
	case CMainFrame::CB_PROPERTIESVIEW:	// The properties view
		{
			if (pDoc)
			{
				pDoc->SetupPropertiesDlg(FALSE);
			}
			break;
		}
	case CMainFrame::CB_MODELVIEW:		// The model view
		{
			UpdateModelDlg();
			break;
		}
	case CMainFrame::CB_SOUNDVIEW:		// The sound view
		{
			UpdateSoundDlg();
			break;
		}
	case CMainFrame::CB_SPRITEVIEW:		// The sprite view
		{
			UpdateSpriteDlg();
			break;
		}
	case CMainFrame::CB_PREFABVIEW:
		{
			UpdatePrefabDlg();
			break;
		}
	case CMainFrame::CB_CLASSVIEW:		// The class view
		{
			GetClassListDlg()->Update();
			break;
		}
	default:
		{
			ASSERT(FALSE);
		}
	}
}

/************************************************************************/
// Do special case termination (for docking and undocking)
void CProjectBar::SpecialCaseTabTerm(CMRCSizeDialogBar *pControlBar, CMainFrame::ProjectControl controlType)
{
	// Switch on the tab type
	switch (controlType)
	{
	case CMainFrame::CB_PROPERTIESVIEW:
		{
			GetPropertiesDlg()->Term();
			break;
		}
	}
}

int CProjectBar::FindTabBySelectID(DWORD id)
{
	DWORD i;

	for(i=0; i < m_TabInfo; i++)
		if(m_TabInfo[i]->m_SelectID == id)
			return (int)i;

	return -1;
}

void CProjectBar::OnImportSounds()
{
	if( VerifyProjectIsOpen() )
	{
		GetSoundDlg()->DoImportOperation();
	}
}


void CProjectBar::OnImportPcxFiles()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoImportOperation(PCX_TO_TEXTURE);
	}
}


void CProjectBar::OnImportTgaFiles()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoImportOperation(TGA_TO_TEXTURE);
	}
}


void CProjectBar::OnImportCubeMap()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoImportCubeMapOperation();
	}
}

void CProjectBar::OnImportBumpMap()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoImportBumpMapOperation();
	}
}

void CProjectBar::OnImportNormalMap()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoImportNormalMapOperation();
	}
}


void CProjectBar::OnImportMipPcxFiles()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoImportMipPcxOperation();
	}
}


void CProjectBar::OnCreateSolidAlpha()
{
	if(VerifyProjectIsOpen())
	{
		GetTextureDlg()->DoCreateAlphaMask(CAM_Solid);
	}
}


void CProjectBar::OnCreateAlphaFromColor()
{
	if(VerifyProjectIsOpen())
	{
		GetTextureDlg()->DoCreateAlphaMask(CAM_FromColor);
	}
}


void CProjectBar::OnImportAlphaMask()
{
	if(VerifyProjectIsOpen())
	{
		GetTextureDlg()->DoCreateAlphaMask(CAM_From4BitPCX);
	}
}


void CProjectBar::OnImport8BitAlphaMask()
{
	if(VerifyProjectIsOpen())
	{
		GetTextureDlg()->DoCreateAlphaMask(CAM_From8BitPCX);
	}
}


void CProjectBar::OnExportPcxFile()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoExportPcxOperation();
	}
}


void CProjectBar::OnExport8BitDTXFile()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoExport8BitDTXOperation();
	}
}

void CProjectBar::OnExportAllAsBPP_32P()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoExportAllAsBPP_32POperation();
	}
}

void CProjectBar::OnConvertTo32P()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoConvertTo32P();
	}
}

void CProjectBar::OnRenameTexture()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoRenameTexture();
	}
}

void CProjectBar::OnTextureProperties()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoTextureProperties();
	}
}

void CProjectBar::OnViewAllTextures()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoViewAllTextures();
	}
}


void CProjectBar::OnImportTextures()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoImportOperation(0);
	}
}

void CProjectBar::OnShowThumbnails()
{
	m_bShowThumbnails = !m_bShowThumbnails;

	GetApp()->GetOptions().GetMiscOptions()->SetShowThumbnails(m_bShowThumbnails);

	//CMenu* pMenu = GetMenu();
	//pMenu->CheckMenuItem(ID_POPUP_SHOWTHUMBNAILS, MF_CHECKED );

	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoShowThumbnails(m_bShowThumbnails);
		GetPrefabDlg()->DoShowThumbnails(m_bShowThumbnails);
		GetSpriteDlg()->DoShowThumbnails(m_bShowThumbnails);
	}
}

void CProjectBar::OnUpdateShowThumbnails(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bShowThumbnails);
}

void CProjectBar::OnBatchReload()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoBatchReload();
	}
}


void CProjectBar::OnScaleTextureCoords()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoScaleTextureCoords();
	}
}


void CProjectBar::OnImportModels()
{
	if( VerifyProjectIsOpen() )
	{
		GetModelDlg()->DoImportOperation();
	}
}

void CProjectBar::OnPopupCreateNewSprite()
{
	CString				fileName, extension, str, fullName;
	CStringDlg			dlg;
	FILE				*fp;
	DFileIdent *pIdent;
	CMoFileIO file;
	int status;


	if( VerifyProjectIsOpen() )
	{
		dlg.m_bAllowNumbers = TRUE;
		dlg.m_bAllowLetters = TRUE;
		dlg.m_bAllowFile	= TRUE;
		dlg.m_MaxStringLen = 70;

		if( dlg.DoModal(IDS_NEWSPRITE, IDS_ENTERSPRITENAME) == IDOK )
		{
			extension.LoadString( IDS_SPRITE_EXTENSION );
			fileName = dfm_BuildName(GetSpriteDlg()->m_csCurrentDir, dlg.m_EnteredText + extension);
			fullName = dfm_GetFullFilename( GetFileMgr( ), fileName );

			BOOL bCreate = TRUE;

			// If the file already exist, ask if they want to overwrite...
			if( dfm_OpenFileRelative( GetFileMgr(), fileName, file ))
			{
				file.Close( );

				str.FormatMessage( IDS_SPRITE_ALREADY_EXISTS, dlg.m_EnteredText );
				status = AfxMessageBox( str, MB_YESNOCANCEL | MB_ICONQUESTION );

				if( status == IDNO )
				{
					bCreate = FALSE;
				}
				else if( status == IDCANCEL )
				{
					return;
				}
			}

			if( bCreate )
			{
				fp = fopen( fullName, "wb" );
				if( !fp )
				{
					str.FormatMessage( IDS_ERRORCREATING_SPRITE, dlg.m_EnteredText );
					AppMessageBox( str, MB_OK );
					return;
				}
				fclose( fp );
			}


			// Create the dialog and hook it to the document's stuff.
			dfm_GetFileIdentifier(GetFileMgr(), fileName, &pIdent);
			if(pIdent)
			{
				EditSpriteFile(pIdent);
			}

			UpdateAll();
		}
	}
}


void CProjectBar::OnPopupUpdate()
{
	UpdateAll();
}


void CProjectBar::GetLayoutFilename( const char *pGetFrom, char *pOutput )
{
	char		pathName[256], fileName[256];
	char		baseFilename[256], extension[256];
	CString		layoutFileName;
	CString		layoutExtension;

	CHelpers::ExtractPathAndFileName( pGetFrom, pathName, fileName );
	CHelpers::ExtractFileNameAndExtension( fileName, baseFilename, extension );

	layoutExtension.LoadString( IDS_LAYOUT_EXTENSION );
	layoutFileName = pathName;
	layoutFileName = dfm_BuildName( layoutFileName, baseFilename + layoutExtension );

	strcpy( pOutput, layoutFileName );
}

//------------------------------------------------------------------------------
//
//  CProjectBar::LoadLayoutFile()
//
//  Purpose:	Loads layout file containing regions left open when project was
//				closed.
//
//------------------------------------------------------------------------------
void CProjectBar::LoadLayoutFile( const char *pPathName )
{
	char				layoutFilename[MAX_PATH], docName[MAX_PATH];
	CString				docFilename, ext;
	CMoFileIO			file;

	BYTE				statusByte;
	CRect				rect;

	DWORD				i, nRegionDocs;
	CRegionView			*pRegionView;
	CRegionFrame		*pRegionFrame;
	CSplitterWnd		*pSplitter;
	POSITION			pos;

	CRegionDoc			*pRegionDoc;
	CObject				*pObj;

	CMultiDocTemplate	*pRegionTemp = GetApp()->m_pWorldTemplate;


	// Create layout name based on project name...
	GetLayoutFilename( pPathName, layoutFilename );

	//figure out what the path is for this project
	char directoryName[MAX_PATH];
	char fileName[MAX_PATH];
	CHelpers::ExtractPathAndFileName( pPathName, directoryName, fileName );


	if( file.Open(layoutFilename, "rb") )
	{
		// Read in all the Region docs and views.
		file >> nRegionDocs;
		nRegionDocs = MIN(nRegionDocs, 20);

		for( i=0; i < nRegionDocs; i++ )
		{
   			// Read document name...
   			file.ReadString(docName, MAX_PATH);

			//if the file is relative, we need to prepend the directory name onto it
			if(!CHelpers::IsFileAbsolute(docName))
			{
				strncpy(docName, (const char*)dfm_BuildName(directoryName, docName), MAX_PATH);
			}

			//now figure out the filename and extension
 			char filename[MAX_PATH];
 			char filenameExt[MAX_PATH];
			CHelpers::ExtractFileNameAndExtension( docName, filename, filenameExt );

 			if( (stricmp(filenameExt, "lta") == 0) || (stricmp(filenameExt, "ltc") == 0) || (stricmp(filenameExt, "tbw") == 0) )
   			{
				// Open the region document...
				pRegionDoc = OpenRegionDoc( docName );
				if( pRegionDoc )
				{
					// Read all views...
					pos = pRegionDoc->GetFirstViewPosition();
					while(1)
					{
						// Read status byte...
						// This byte is '1' if there is another view for this document, or '0'
						// if the last view was already read...
						file >> statusByte;

						// Handle condition when finished reading views...
						if( statusByte == 0 )
						{
							// Get the frame for the document...
							pRegionFrame = ( CRegionFrame * )pRegionView->GetParent()->GetParent();
							ASSERT( pRegionFrame );

							// Read in the window placement...
							WINDOWPLACEMENT wp;
							DWORD temp;
							wp.length = sizeof( WINDOWPLACEMENT );
							file >> temp;  wp.flags = temp;
							file >> temp;  wp.showCmd = temp;
							file >> temp;	wp.ptMinPosition.x = temp;
							file >> temp;	wp.ptMinPosition.y = temp;
							file >> temp;	wp.ptMaxPosition.x = temp;
							file >> temp;	wp.ptMaxPosition.y = temp;
							file >> temp;	wp.rcNormalPosition.left = temp;
							file >> temp;	wp.rcNormalPosition.top = temp;
							file >> temp;	wp.rcNormalPosition.right = temp;
							file >> temp;	wp.rcNormalPosition.bottom = temp;

							pRegionFrame->SetWindowPlacement( &wp );

							// Split the window panes evenly...
							pRegionFrame->GetClientRect( &rect );
							pSplitter = pRegionFrame->GetSplitter( );
							ASSERT( pSplitter );
							pSplitter->SetRowInfo( 0, rect.Height()/2, 10 );
							pSplitter->SetColumnInfo( 0, rect.Width()/2, 10 );
							pSplitter->RecalcLayout();
							pRegionFrame->Invalidate( FALSE );
							break;
						}
						// Handle condition when another view needs to be read...
						else if( statusByte == 1 )
						{
							// Get pointer to view...
							if( pos )
							{
								pObj = (CRegionView*)pRegionDoc->GetNextView( pos );
							}
							else
							{
								pObj = pRegionDoc->CreateNewRegionView();
							}

							// Load info about the view...
							if( pObj->IsKindOf( RUNTIME_CLASS( CRegionView )))
							{
								pRegionView = (CRegionView*)pObj;

								pRegionView->LoadProjFile( file );
							}
						}
						else
							return;
					}
				}
			}
		}
		file.Close();
	}
}


//------------------------------------------------------------------------------
//
//  CProjectBar::SaveLayoutFile()
//
//  Purpose:	Saves layout file containing regions left open when project was
//				closed.
//
//------------------------------------------------------------------------------
void CProjectBar::SaveLayoutFile( const char *pPathName )
{
	char			layoutFilename[MAX_PATH];
	char			directoryName[MAX_PATH];
	char			fileName[MAX_PATH];
	CMoFileIO		file;
	DWORD			i;
	CRegionDoc		*pDoc;

	CRegionView		*pRegionView;
	POSITION		pos;
	CRect			rect;


	// Create layout file name based on project file name...
	GetLayoutFilename( pPathName, layoutFilename );

	//get the path and filename
	CHelpers::ExtractPathAndFileName( pPathName, directoryName, fileName );


	if( file.Open(layoutFilename, "wb") )
	{
		// Write out the CRegionDoc and view descriptions.
		file << m_RegionDocs.GetSize();
		for( i=0; i < m_RegionDocs; i++ )
		{
			pDoc = m_RegionDocs[i];


			//make the files inside be relative to the path
			CString sFilename = pDoc->m_FileName;

			//if it starts up with the same directory, save it
			uint32 nDirLen = strlen(directoryName);
			if(sFilename.Left(nDirLen).CompareNoCase(directoryName) == 0)
			{
				sFilename = sFilename.Mid(nDirLen);
				if(sFilename.GetLength() && ((sFilename[0] == '\\') || (sFilename[0] == '/')))
					sFilename = sFilename.Mid(1);
			}

			// Write the filename of the doc...
			file.WriteString( (char*)((const char*)sFilename) );

			// Write out info on views...
			pos = pDoc->GetFirstViewPosition();
			while( pos )
			{
				pRegionView = (CRegionView*)pDoc->GetNextView(pos);

				if( pRegionView->IsKindOf(RUNTIME_CLASS(CRegionView)) )
				{
					// Write status byte which says view info follows...
					file << (BYTE)1;

					pRegionView->SaveProjFile( file );
				}
			}

			// Write status byte which says finished writing view info...
			file << (BYTE)0;

			WINDOWPLACEMENT wp;
			DWORD temp;
			CRegionFrame *pRegionFrame;

			// Write out frame position info...
			wp.length = sizeof( WINDOWPLACEMENT );
			ASSERT( pRegionView->GetParent( ));
			pRegionFrame = ( CRegionFrame * )pRegionView->GetParent()->GetParent( );
			ASSERT( pRegionFrame );
			pRegionFrame->GetWindowPlacement( &wp );

			wp.flags = 0;
			if( GetMainFrame( )->IsZoomed( ))
				wp.flags |= WPF_RESTORETOMAXIMIZED;
			temp = wp.flags;  file << temp;
			temp = wp.showCmd;  file << temp;
			file << wp.ptMinPosition.x;
			file << wp.ptMinPosition.y;
			file << wp.ptMaxPosition.x;
			file << wp.ptMaxPosition.y;
			file << wp.rcNormalPosition.left;
			file << wp.rcNormalPosition.top;
			file << wp.rcNormalPosition.right;
			file << wp.rcNormalPosition.bottom;
		}


		file.Close();
	}
}



void CProjectBar::UpdateTextureIDs()
{
	for( int i = 0; i < m_RegionDocs; i++ )
	{
		m_RegionDocs[i]->m_Region.UpdateTextureIDs( &m_Project );
	}
}


CRegionDoc* CProjectBar::OpenRegionDoc(const char* fullName, bool bForceSyncObjects, bool bAllowPropSync)
{
	CRegionDoc			*pDoc;
	CMultiDocTemplate	*pDocTemp = GetApp()->m_pWorldTemplate;
	POSITION			docPos;


	// This whole block replaces this line .. I had to prevent MFC from adding it to the MRU list.
	// pDoc = (CRegionDoc*)pDocTemp->OpenDocumentFile( pName, TRUE );
	{
		BOOL				bAutoDelete;
		CFrameWnd			*pFrame;


		for( docPos = pDocTemp->GetFirstDocPosition(); docPos != NULL; )
		{
			pDoc = ( CRegionDoc * )pDocTemp->GetNextDoc( docPos );
			if( !pDoc->m_FileName.CompareNoCase(fullName))
			{
				POSITION pos;
				CView *pView;

				pos = pDoc->GetFirstViewPosition( );
				if( pos == NULL )
					return NULL;

				pView = pDoc->GetNextView( pos );
				pView->GetParentFrame( )->BringWindowToTop( );
				return NULL;
			}
		}

		pDoc = (CRegionDoc*)pDocTemp->CreateNewDocument();

		ASSERT_VALID(pDoc);

		bAutoDelete = pDoc->m_bAutoDelete;
		pDoc->m_bAutoDelete = FALSE;   // don't destroy if something goes wrong

		pFrame = pDocTemp->CreateNewFrame(pDoc, NULL);
		ASSERT_VALID(pFrame);

		pDoc->m_bAutoDelete = bAutoDelete;

		// open an existing document
		CWaitCursor wait;
		if (!pDoc->InitDocument(fullName, bForceSyncObjects, bAllowPropSync))
		{
			// user has be alerted to what failed in OnOpenDocument
			TRACE0("CDocument::OnOpenDocument returned FALSE.\n");
			pFrame->DestroyWindow();
			return NULL;
		}

		pDoc->SetPathName(fullName, FALSE);
		pDocTemp->InitialUpdateFrame( pFrame, pDoc, TRUE );
	}

	pDoc->SetTitle(pDoc->IsModified());

	m_RegionDocs.Append( pDoc );

	// Set view info...
	POSITION pos;
	CRegionView *pView;
	CRegionFrame *pRegionFrame;
	pos = pDoc->GetFirstViewPosition();
	ASSERT( pos );

	pView = ( CRegionView * )pDoc->GetNextView( pos );
	ASSERT( pView );
	pRegionFrame = ( CRegionFrame * )pView->GetParent()->GetParent();
	ASSERT( pRegionFrame );

	CRect rect;
	WINDOWPLACEMENT wPlace;
	AfxGetMainWnd( )->GetClientRect( &rect );
	AfxGetMainWnd( )->ClientToScreen( &rect );

	wPlace.length = sizeof( WINDOWPLACEMENT );
	pRegionFrame->GetWindowPlacement( &wPlace );
	wPlace.showCmd = SW_SHOWMAXIMIZED;
	pRegionFrame->SetWindowPlacement( &wPlace );

	pView->On4ViewConfiguration( );

	GetNodeView()->Init(pDoc);
	pDoc->NotifySelectionChange();
	return pDoc;
}


void CProjectBar::RemoveRegionDoc( CRegionDoc *pDoc )
{
	DWORD		index = m_RegionDocs.FindElement(pDoc);

	if( index != BAD_INDEX )
		m_RegionDocs.Remove( index );
}


void CProjectBar::EditSpriteFile(DFileIdent *pIdent)
{
	if( IsWindow( m_SpriteEditDlg.m_hWnd ))
		return;

	m_SpriteEditDlg.m_pFile = pIdent;
	m_SpriteEditDlg.Create( this );
	m_SpriteEditDlg.ShowWindow( SW_SHOW );
}


void CProjectBar::GetCurrentTextureName( char *szTextureFilename, BOOL bUseSprites )
{
	int nIndex;
	DFileIdent *pFile;

	if(bUseSprites)
	{
		if(GetSpriteDlg()->IsWindowVisible())
		{
			nIndex = GetSpriteDlg()->m_SpriteList.GetNextItem(-1,LVNI_SELECTED);
			if(nIndex != -1)
			{
				pFile = (DFileIdent*)GetSpriteDlg()->m_SpriteList.GetItemData(nIndex);
				if(pFile)
					strcpy(szTextureFilename, pFile->m_Filename);
				else
					szTextureFilename[0] = 0;

				return;
			}
		}
	}

	if(m_pCurTextureSel)
	{
		strcpy(szTextureFilename, m_pCurTextureSel->m_Filename);
	}
	else
	{
		szTextureFilename[0] = '\0';
	}
}

//given the name this will find and select the prefab in the prefab pane
bool CProjectBar::SelectPrefab(const char* pszPrefabName)
{
	return GetPrefabDlg()->SelectPrefab(pszPrefabName);
}


void CProjectBar::SetCurTextureSel(DFileIdent *pIdent)
{
	m_pCurTextureSel = pIdent;
}

void CProjectBar::FindCurTextureSel()
{
	GetTextureDlg()->FindTexture(m_pCurTextureSel);
	// Update thumbnail images, since the list gets thrashed
	GetTextureDlg()->DoShowThumbnails(GetProjectBar()->m_bShowThumbnails); 
}

static const TCHAR szProjectBarSection[] = _T("%s-ProjectBar");
static const TCHAR szProjectBarDefaultCx[] = _T("Default.Cx");
static const TCHAR szProjectBarDefaultCy[] = _T("Default.Cy");

void CProjectBar::RestoreRegistryInfo( LPCTSTR lpszProfileName )
{
	CWinApp* pApp = AfxGetApp();

	TCHAR szSection[256];
	wsprintf( szSection, szProjectBarSection, lpszProfileName);
//	m_sizeDefault.cx = pApp->GetProfileInt( szSection, szProjectBarDefaultCx, m_sizeDefault.cx );
//	m_sizeDefault.cy = pApp->GetProfileInt( szSection, szProjectBarDefaultCy, m_sizeDefault.cy );
//	m_sizeDocked = m_sizeFloating = m_sizeDefault;

	m_bShowThumbnails = GetApp()->GetOptions().GetMiscOptions()->GetShowThumbnails();
}
 
void CProjectBar::SaveRegistryInfo( LPCTSTR lpszProfileName )
{
	CWinApp* pApp = AfxGetApp();

	TCHAR szSection[256];
	wsprintf( szSection, szProjectBarSection, lpszProfileName);
//	pApp->WriteProfileInt( szSection, szProjectBarDefaultCx, m_sizeDocked.cx );
//	pApp->WriteProfileInt( szSection, szProjectBarDefaultCy, m_sizeDocked.cy );

	// Save Icons preferences
	GetApp()->GetOptions().GetMiscOptions()->SetShowIcons(m_bShowIcons);

	// Save Thumbnails preferences
	GetApp()->GetOptions().GetMiscOptions()->SetShowThumbnails(m_bShowThumbnails);

	// Save the settings in the registry
	GetApp()->GetOptions().GetWindowsOptions()->SetBarControlArray(m_TabInfo);
}

void CProjectBar::OnReplaceSelectedPrefabs()
{
	GetPrefabDlg()->OnReplaceSelectedPrefabs();
}

void CProjectBar::OnRenamePrefab()
{
	GetPrefabDlg()->OnRenamePrefab();
}


void CProjectBar::OnPrefabOpen() 
{
	GetPrefabDlg()->OnPrefabOpen();	
}

// NOTE: This function uses ID_WORLD_ADDDIR but is actually where the directory-adding code goes.
void CProjectBar::OnAddDir()
{
	CString path;
	CResNewDir dlg;
	CBaseRezDlg *pVisibleDlg;

	if(m_pVisibleDlg)
	{
		if(m_pVisibleDlg->IsKindOf(RUNTIME_CLASS(CBaseRezDlg)))
		{
			pVisibleDlg = (CBaseRezDlg*)m_pVisibleDlg;

			if(dlg.DoModal() == IDOK)
			{
				path = dfm_BuildName(pVisibleDlg->m_csCurrentDir, dlg.m_szNewDir);
				
				CResourceMgr::CreateDir(path, pVisibleDlg->m_ResourceType);
				pVisibleDlg->UpdateDirectories();
				pVisibleDlg->Invalidate(FALSE);
			}
		}
	}
}


BOOL CProjectBar::GetModelDims( char *szModelFile, LTVector *pDims )
{
	Model *pModel;
	DVector vDims;	 
	char tempStr[200];

	ASSERT( pDims );
	ASSERT( szModelFile );
	if( !pDims || !szModelFile )
		return FALSE;

	// {MD 1/18/98: editor was treating sprite files like model files and hanging..}
	strncpy(tempStr, szModelFile, sizeof(tempStr)-1);
	strupr(tempStr);
	if(strstr(tempStr, ".LTA") || strstr(tempStr, ".LTC"))
	{
		CLTALoadOnlyAlloc Allocator(2048);
		CLTAReader reader;  CLTANode *root, *node;
		
		if(!reader.Open(szModelFile, CLTAUtil::IsFileCompressed(szModelFile)))  return false;

		root = CLTANodeReader::LoadNode(&reader, "dims", &Allocator);

		node = root->GetElement(1);

		vDims.x = atof(node->GetElement(0)->GetValue()); 	
		vDims.y = atof(node->GetElement(1)->GetValue());  	
		vDims.z = atof(node->GetElement(2)->GetValue());

		reader.Close();

		Allocator.FreeNode(root);
		Allocator.FreeAllMemory();
	}
	else
	{
		return false;
	}

	pDims->x = vDims.x;
	pDims->y = vDims.y;
	pDims->z = vDims.z;

	return TRUE;
}

void CProjectBar::OnAddToPalette() 
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoAddToPalette();
	}
}
 

// load each world and adjust the OPQs for every poly that references
// one of the changed textures in textureInfo
bool CProjectBar::BatchTextureScale( const CMoArray<BatchTextureScaleInfo*>& textureInfo, const char* inputDir )
{
	// make sure directory name ends in a '\'
	char dir[_MAX_PATH];
	strcpy( dir, inputDir );
	int dirLen = strlen( dir );
	if( dir[dirLen-1] != '\\' )
	{
		dir[dirLen] = '\\';
		dir[dirLen+1] = 0;
		dirLen++;
	}

	// loop over all .lta files
	WIN32_FIND_DATA findData;
	HANDLE findHandle;
	CString startDir;

	startDir = dir;
	startDir += "*.*";
	findHandle = FindFirstFile( LPCTSTR(startDir), &findData );

	if( findHandle != INVALID_HANDLE_VALUE )
	{
		do
		{
			if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				// ignore directories starting with .
				if( findData.cFileName[0] != '.' )
				{
					// found a directory, recurse into it
					CString newDir = dir;
					newDir += findData.cFileName;
					BatchTextureScale( textureInfo, newDir );
				}
			}
			else
			{
				// it's a file, check the file type
				char ext[_MAX_EXT];
				char file[_MAX_FNAME];

				_splitpath( findData.cFileName, NULL, NULL, file, ext );
				CString resourcePath( dir );
				resourcePath = resourcePath.Left( resourcePath.GetLength() - 1 );
				resourcePath = resourcePath.Right( resourcePath.GetLength() - GetProject()->m_BaseProjectDir.GetLength() - 1 );

				// make sure the extension is lower case
				for( unsigned i = 0; i < strlen( ext ); i++ )
				{
					if( isupper( ext[i] ) )
						ext[i] = tolower( ext[i] );
				}

				// found a world file, process it if it's in a world directory
				if( ( (stricmp( ext, ".lta" ) == 0) || (stricmp( ext, ".ltc") == 0) || (stricmp( ext, ".tbw") == 0) ) && 
					CResourceMgr::IsDirType( resourcePath, RESTYPE_WORLD ) )
				{
					CString inFile = dir;
					inFile += file;
					inFile += ext;
					
					// load the world if it's not already open
					CRegionDoc* regionDoc;
					bool alreadyOpen = false;
					for( int i = 0; i < m_RegionDocs; i++ )
					{
						if( !inFile.CompareNoCase( m_RegionDocs[i]->GetPathName() ) )
						{
							alreadyOpen = true;
							regionDoc = m_RegionDocs[i];
						}
					}
					if( !alreadyOpen )
						regionDoc = OpenRegionDoc( inFile );

					// process the world
					regionDoc->TextureScale( textureInfo );

					// save and close the world
					regionDoc->OnSaveDocument( inFile );
					if( !alreadyOpen )
						regionDoc->OnCloseDocument();
				}
			}
		}
		while( FindNextFile( findHandle, &findData ) );

		FindClose( findHandle );
	}

	return true;
}

void CProjectBar::OnMakeTextureWritable()
{
	if( VerifyProjectIsOpen() )
	{
		GetTextureDlg()->DoMakeTextureWritable();
	}
}

void CProjectBar::OnFindTextureInWorld() 
{
	// Get the active document
	CRegionDoc *pDoc = GetActiveRegionDoc();
	if (!pDoc)
		return;

	// Do we even have anything selected?
	if (!m_pCurTextureSel)
		return;

	// Select the desired brushes
	pDoc->SelectBrushesByTexture(m_pCurTextureSel->m_Filename);
}

void CProjectBar::OnHideCurrent()
{
	// Get the currently selected tab
	int nIndex = m_Tabs.GetCurSel(); 

	// Check for no selection
	if (nIndex == -1)  return;

	// Undock and hide tab
	UndockTab(nIndex, CPoint(200,200), true);
}

void CProjectBar::OnUnhideAll()
{
	CFrameWnd* pFrameWnd;

	// Properties dialog
	if (!IsTabPresent(CMainFrame::CB_PROPERTIESVIEW) && !GetPropertiesDlg()->IsWindowVisible())
	{		
		pFrameWnd = GetPropertiesDlg()->GetParentFrame();
		ASSERT(pFrameWnd);

		pFrameWnd->FloatControlBar(GetPropertiesDlg(), CPoint(0,0));
		DockTab(GetPropertiesDlg(), (CMainFrame::ProjectControl)CMainFrame::CB_PROPERTIESVIEW);
	}
	// Class dialog
	if (!IsTabPresent(CMainFrame::CB_CLASSVIEW) && !GetClassListDlg()->IsWindowVisible())
	{
		pFrameWnd = GetClassListDlg()->GetParentFrame();
		ASSERT(pFrameWnd);

		pFrameWnd->FloatControlBar(GetClassListDlg(), CPoint(0,0));
		DockTab(GetClassListDlg(), (CMainFrame::ProjectControl)CMainFrame::CB_CLASSVIEW);
	}
	// Texture dialog
	if (!IsTabPresent(CMainFrame::CB_TEXTUREVIEW) && !GetTextureDlg()->IsWindowVisible())
	{
		pFrameWnd = GetTextureDlg()->GetParentFrame();
		ASSERT(pFrameWnd);

		pFrameWnd->FloatControlBar(GetTextureDlg(), CPoint(0,0));
		DockTab(GetTextureDlg(), (CMainFrame::ProjectControl)CMainFrame::CB_TEXTUREVIEW);
	}
	// Model dialog
	if (!IsTabPresent(CMainFrame::CB_MODELVIEW) && !GetModelDlg()->IsWindowVisible())
	{
		pFrameWnd = GetModelDlg()->GetParentFrame();
		ASSERT(pFrameWnd);

		pFrameWnd->FloatControlBar(GetModelDlg(), CPoint(0,0));
		DockTab(GetModelDlg(), (CMainFrame::ProjectControl)CMainFrame::CB_MODELVIEW);
	}
	// Worlds dialog
	if (!IsTabPresent(CMainFrame::CB_WORLDSVIEW) && !GetWorldsDlg()->IsWindowVisible())
	{
		pFrameWnd = GetWorldsDlg()->GetParentFrame();
		ASSERT(pFrameWnd);

		pFrameWnd->FloatControlBar(GetWorldsDlg(), CPoint(0,0));
		DockTab(GetWorldsDlg(), (CMainFrame::ProjectControl)CMainFrame::CB_WORLDSVIEW);
	}
	// Sound dialog
	if (!IsTabPresent(CMainFrame::CB_SOUNDVIEW) && !GetSoundDlg()->IsWindowVisible())
	{
		pFrameWnd = GetSoundDlg()->GetParentFrame();
		ASSERT(pFrameWnd);

		pFrameWnd->FloatControlBar(GetSoundDlg(), CPoint(0,0));
		DockTab(GetSoundDlg(), (CMainFrame::ProjectControl)CMainFrame::CB_SOUNDVIEW);
	}
	// Sprite dialog
	if (!IsTabPresent(CMainFrame::CB_SPRITEVIEW) && !GetSpriteDlg()->IsWindowVisible())
	{
		pFrameWnd = GetSpriteDlg()->GetParentFrame();
		ASSERT(pFrameWnd);

		pFrameWnd->FloatControlBar(GetSpriteDlg(), CPoint(0,0));
		DockTab(GetSpriteDlg(), (CMainFrame::ProjectControl)CMainFrame::CB_SPRITEVIEW);
	}
	// Nodeview dialog
	if (!IsTabPresent(CMainFrame::CB_NODESVIEW) && !GetNodeView()->IsWindowVisible())
	{
		pFrameWnd = GetNodeView()->GetParentFrame();
		ASSERT(pFrameWnd);

		pFrameWnd->FloatControlBar(GetNodeView(), CPoint(0,0));
		DockTab(GetNodeView(), (CMainFrame::ProjectControl)CMainFrame::CB_NODESVIEW);
	}
	// Prefab dialog
	if (!IsTabPresent(CMainFrame::CB_PREFABVIEW) && !GetPrefabDlg()->IsWindowVisible())
	{
		pFrameWnd = GetPrefabDlg()->GetParentFrame();
		ASSERT(pFrameWnd);

		pFrameWnd->FloatControlBar(GetPrefabDlg(), CPoint(0,0));
		DockTab(GetPrefabDlg(), (CMainFrame::ProjectControl)CMainFrame::CB_PREFABVIEW);
	}
}


bool CProjectBar::IsTabPresent(CMainFrame::ProjectControl tabType)
{
	for (int32 i=0; i<m_TabInfo; i++)
	{
		if (tabType == m_TabInfo[i]->m_ControlType)  return true;
	}

	return false;
}

void CProjectBar::OnShowIcons()
{
	TCITEM item;

	m_bShowIcons = !m_bShowIcons;  // If we're showing, switch so we aren't and vice versa

	GetApp()->GetOptions().GetMiscOptions()->SetShowIcons(m_bShowIcons);

	// Setup the tabs.
	for(uint16 i=0; i < m_TabInfo; i++)
	{
		if (m_Tabs.GetItem(i, &item))
		{
			// Set the item's image index
			if (m_bShowIcons)
				item.iImage = m_TabInfo[i]->m_ControlType;  // Specific icon for tab
			else
				item.iImage = CMainFrame::CB_LAST_CONTROL_INDEX;  // Neutral icon

			item.mask |= TCIF_IMAGE;

			// Set the item	
			m_Tabs.SetItem(i, &item);
		}			
	}
}




/************************************************************************/
// This gets called when the user selects "Dock" from the context menu or clicks 'X'
void CProjectBar::OnButtonHide()
{
	CMRCSizeControlBar::OnButtonHide();

	/*if (m_pDockBar != NULL)
	{
		ASSERT(m_pDockContext != NULL);

		if ((m_dwDockStyle & CBRS_ALIGN_ANY) == 0)  // if we can't dock
		{
			OnToggleAllowDocking();	// re-enable docking...
		}

		m_pDockContext->ToggleDocking();

		ResizeElements();
		Invalidate();
	}*/
}
