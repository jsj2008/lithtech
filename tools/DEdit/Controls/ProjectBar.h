#ifndef __PROJECTBAR_H__
#define __PROJECTBAR_H__

// Includes....
#include "resizebar.h"
#include "texturedlg.h"
#include "editprojectmgr.h"
#include "regiondoc.h"
#include "propertiesdlg.h"
#include "worldsdlg.h"
#include "nodeview.h"
#include "modeldlg.h"
#include "sounddlg.h"
#include "spritedlg.h"
#include "prefabdlg.h"
#include "spriteeditdlg.h"
#include "debugdlg.h"
#include "classlistdlg.h"
#include "projecttabctrl.h"

#include "mainfrm.h"
#include "mrcext.h"

// Flags for importing.
#define PCX_TO_TEXTURE		(1<<0)
#define TGA_TO_TEXTURE		(1<<1)

typedef BOOL (*CheckExistFn)( CEditProjectMgr *pMgr, const char *pName );



class CMainFrame;
class CProjectControlBarInfo;
class CProjectBar : public CMRCSizeDialogBar
{
// Construction
public:

CProjectBar();

// Dialog Data
	//{{AFX_DATA(CProjectBar)
	bool	m_bShowThumbnails;  // Whether or not we want to show thumbnails in the texture view
	//}}AFX_DATA



// Custom CDEditView stuff.
public:

	// Called at the very beginning and and of the app.
	BOOL			Init( CMainFrame *pFrame );
	void			Term();
	
	// This destroys the control windows that the project bar owns
	void			DestroyControlWindows();

	// Called to switch level and stuff.
	BOOL			Open( const char *pFilename );
	BOOL			Save();
	BOOL			Close();

	BOOL			IsProjectOpen();
	BOOL			VerifyProjectIsOpen();

	
	CEditProjectMgr*	GetProject()	{ return &m_Project; }


	
	void			ResizeElements();

	void			UpdateAll();
	void			ClearAll();

	void			UpdateTextureDlg();
	void			UpdatePropertiesDlg();
	void			UpdateTextureIDs();
	void			UpdateWorldsDlg( );
	void			UpdateNodeView( );
	void			UpdateModelDlg( );
	void			UpdateSoundDlg( );
	void			UpdateSpriteDlg( );			
	void			UpdatePrefabDlg( );

	void			GetLayoutFilename( const char *pGetFrom, char *pOutput );
	char *			GetProjFilename( ) { return m_ProjectFileName; }
	void			LoadLayoutFile( const char *pPathName );
	void			SaveLayoutFile( const char *pPathName );

	void			RestoreRegistryInfo( LPCTSTR lpszProfileName );
	void			SaveRegistryInfo( LPCTSTR lpszProfileName );

	void			SetTab( CMainFrame::ProjectControl controlType );
	int				FindTabBySelectID(DWORD id);

	// This gets called when the "hide" button is pressed
	virtual void OnButtonHide();

	// This is called to open a region doc.
	// pName must be a FULL filename.
	CRegionDoc*		OpenRegionDoc(const char* fullName, bool bForceSyncObjects = false, bool bAllowPropSync = true);

	BOOL			CloseAllRegionDocs( );

	// Region docs call this when they close.
	void			RemoveRegionDoc( CRegionDoc *pDoc );


	// Called to bring up and remove sprite dialogs.
	void			EditSpriteFile(DFileIdent *pIdent);


	// Returns the name of the currently selected texture.
	void			GetCurrentTextureName( char *szTexutureFilename, BOOL bUseSprites=FALSE );

	// Called by the texture list to set the current texture selection.
	void			SetCurTextureSel(DFileIdent *pIdent);

	// Finds and selects the currently selected texture in the tree/list
	void			FindCurTextureSel();

	// Loads every world file and adjusts the texture mapping coordinates based on new texture sizes
	bool			BatchTextureScale( const CMoArray<BatchTextureScaleInfo*>& textureInfo, const char* inputDir );
	
	//given the name this will find and select the prefab in the prefab pane
	bool			SelectPrefab(const char* pszPrefabName);

	// Get the dims of the model...
	BOOL			GetModelDims( char *szModelFile, CVector *pDims );

	// This undocks a tab and docks it to the mainframe
	void			UndockTab(int nIndex, CPoint position, bool bHide=false);

	// This takes an undocked control bar and inserts it into the tab dialog
	void			DockTab(CWnd *pWnd, CMainFrame::ProjectControl controlType);

	// This hides the currently selected tab
	void			OnHideCurrent();

	// This unhides all tabs
	void			OnUnhideAll();

	// This shows/hides the tab icons
	void			OnShowIcons();

	// Search for whether a given tab is present in the Project Bar
	bool			IsTabPresent(CMainFrame::ProjectControl tabType);

	//this will take the specified resource identifier, and write it out as the 
	//file specified. For more information, see documentation on FindResource
	static bool		CreateFileFromResource(LPCTSTR pResourceID, LPCTSTR pResourceType, const char* pszFilename);

	//this will find the class icons directory that is specified, and if it does not
	//exist, it will create it, and add all the appropriate icons into it
	void			AutoExtractClassIcons();

	// Currently selected texture in the views
	struct DFileIdent_t	*m_pCurTextureSel;

	// Any time a region doc is created, it's added to here.
	CMoArray<CRegionDoc*>					m_RegionDocs;
	CRegionDoc*								m_pNewRegionDoc;

	// Info about the tab windows.
	CMoArray<CProjectControlBarInfo *>		m_TabInfo;


	// The document's project.
	CEditProjectMgr		m_Project;

	// The full filename of the Project File...
	char				m_ProjectFileName[MAX_PATH];

	// Tab dialog stuff.
	CWnd				*m_pVisibleDlg;

	CDebugDlg			m_DebugDlg;
	CProjectTabCtrl		m_Tabs;
	CImageList			m_TabImages;
	CBitmap*			m_pTabImagesBitmap;
	CToolTipCtrl		m_ToolTips;
							
	CSpriteEditDlg		m_SpriteEditDlg;

protected:
	// Do special case initialization/termination (for docking and undocking)
	void				SpecialCaseTabInit(CMRCSizeDialogBar *pControlBar, CMainFrame::ProjectControl);
	void				SpecialCaseTabTerm(CMRCSizeDialogBar *pControlBar, CMainFrame::ProjectControl);

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProjectBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CProjectBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTabsSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnImportTextures();
	afx_msg void OnImportPcxFiles();
	afx_msg void OnImportMipPcxFiles();
	afx_msg void OnImportTgaFiles();
	afx_msg void OnImportCubeMap();
	afx_msg void OnImportBumpMap();
	afx_msg void OnImportNormalMap();
	afx_msg void OnImportAlphaMask();
	afx_msg void OnImport8BitAlphaMask();
	afx_msg void OnCreateAlphaFromColor();
	afx_msg void OnCreateSolidAlpha();
	afx_msg void OnExportPcxFile();
	afx_msg void OnTextureProperties();
	afx_msg void OnViewAllTextures();
	afx_msg void OnBatchReload();
	afx_msg void OnScaleTextureCoords();
	afx_msg void OnImportSounds();
	afx_msg void OnImportModels();
	afx_msg void OnPopupCreateNewSprite();
	afx_msg void OnPopupUpdate();
	afx_msg void OnAddDir();
	afx_msg void OnPrefabOpen();
	afx_msg void OnReplaceSelectedPrefabs();
	afx_msg void OnRenamePrefab();
	afx_msg void OnAddToPalette();
	afx_msg void OnExport8BitDTXFile();
	afx_msg void OnExportAllAsBPP_32P();
	afx_msg void OnConvertTo32P();
	afx_msg void OnRenameTexture();
	afx_msg void OnFindTextureInWorld();
	afx_msg void OnMakeTextureWritable();
	afx_msg void OnShowThumbnails();
	afx_msg void OnUpdateShowThumbnails(CCmdUI *pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif  // __PROJECTBAR_H__
