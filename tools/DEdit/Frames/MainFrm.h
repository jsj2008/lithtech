//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __MAINFRM_H__
#define __MAINFRM_H__

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

class CProjectBar;


#include "colorselectdlg.h"
#include "editprojectmgr.h"
#include "editclipboard.h"

#include "mrcext.h"	//schlegz for new project bar
#include "flattoolbar.h"
#include "dedockframewnd.h"

class CPropertiesDlg;
class CNodeView;
class ClassListDlg;
class CTextureDlg;
class CPhysicsDlg;
class CWorldsDlg;
class CModelDlg;
class CSoundDlg;
class CSpriteDlg;
class CPrefabDlg;

class CProjectControlBarInfo;
class CMainFrame : public CMRCMDIFrameWndSizeDock	//SCHLEGZ changed to new frame for new project bar support
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

	enum ProjectControl
	{
		CB_WORLDSVIEW = 0,
		CB_TEXTUREVIEW,
		CB_NODESVIEW,
		CB_PROPERTIESVIEW,
		CB_MODELVIEW,
		CB_SOUNDVIEW,
		CB_SPRITEVIEW,
		CB_CLASSVIEW,
		CB_PREFABVIEW,
		CB_LAST_CONTROL_INDEX
	};

// Attributes
public:

	CStatusBar*		GetStatusBar()			{ return &m_wndStatusBar; }
	CFlatToolbar*	GetToolBar()			{ return &m_wndMainBar; }

	CFlatToolbar*	GetNodeViewToolbar()	{ return &m_NodeViewToolbar; }
	CFlatToolbar*	GetWorldEditToolbar()	{ return &m_WorldEditToolbar; }
	CProjectBar*	GetProjectBar()			{ return m_pProjectBar; }

	CNodeView*		GetNodeView()			{ return m_pNodeView; }
	CPropertiesDlg*	GetPropertiesDlg()		{ return m_pPropertiesDlg; }		
	ClassListDlg*	GetClassListDlg()		{ return m_pClassListDlg; }
	CTextureDlg*	GetTextureDlg()			{ return m_pTextureDlg; }
	CModelDlg*		GetModelDlg()			{ return m_pModelDlg; }
	CWorldsDlg*		GetWorldsDlg()			{ return m_pWorldsDlg; }
	CSoundDlg*		GetSoundDlg()			{ return m_pSoundDlg; }
	CSpriteDlg*		GetSpriteDlg()			{ return m_pSpriteDlg; }
	CPrefabDlg*		GetPrefabDlg()			{ return m_pPrefabDlg; }

	int				GetWorldEditMode()		{ return m_WorldEditMode; }

	int				GetNodeSelectionMode()	{ return m_NodeSelectionMode; }
	void			SetToolTips( );
	BOOL			IsClosing( )			{ return m_bClosing; }

	CEditClipboard *GetClipboard( )			{ return &m_Clipboard; }

	BOOL			ShouldIgnoreMsgsInPrefabs() { return m_bIgnoreMsgsInPrefabs; }

	
	// Undo preferences

	BOOL			CountUndo( UINT &iUndo, UINT &iResetVal );
	BOOL			CountUndoGeometry( );
	BOOL			CountUndoTextures( );
	BOOL			CountUndoProperties( );
	BOOL			CountUndoGeneral( );
	
	void			SetUndoPreferencesDefault( UINT &iGeometry, UINT &iTextures, UINT &iProperties, UINT &iGeneral );
	void			SetUndoPreferencesCoarse( UINT &iGeometry, UINT &iTextures, UINT &iProperties, UINT &iGeneral );

	UINT			m_iUndoGeometry;
	UINT			m_iCurUndoGeometry;
	UINT			m_iUndoTextures;
	UINT			m_iCurUndoTextures;
	UINT			m_iUndoProperties;
	UINT			m_iCurUndoProperties;
	UINT			m_iUndoGeneral;
	UINT			m_iCurUndoGeneral;


// Operations
public:
	void			UpdateStatusText(int nIndex, const char* szText);

	void			InitProjectBar();

	int				GetNumRegionDocs();
	void			UpdateAllRegions( LPARAM hint );
	
	// Updates the menu bar with the mode menus.  This basically adds
	// the correct menu and removes the incorrect menu.
	void			UpdateModeMenus();

	// Returns a pointer to the mode menu
	CMenu			*GetModeMenu()			{ return &m_modeMenu; }

	// Centers the views at the specific coordinate.
	// Pass in TRUE if you wish to center perspective views as well.		
	void			CenterViewsAtVector(CVector vCenter, BOOL bCenterPerspective=TRUE);
	
	// Call this to add a project control info structure based on the
	// control type.
	// Returns: pointer to the allocated control info structure.
	CProjectControlBarInfo	*AddProjectControlInfo(CMoArray<CProjectControlBarInfo *> &controlArray,
												   CMainFrame::ProjectControl controlType);

	// This creates a project bar window (from the control type) and returns
	// a pointer to the created window.  Note that it doesn't allocate a new
	// CMRCSizeDialogBar, it is just returning a pointer to the window that
	// is assocated with the control type.
	CMRCSizeDialogBar		*CreateProjectControlBar(CMainFrame::ProjectControl controlType);

	// This destroys a project bar window and returns a pointer to the
	// destroyed window.
	CMRCSizeDialogBar		*DestroyProjectControlBar(CMainFrame::ProjectControl controlType);

	//Given a world name, this will use the options and launch a process to run the 
	//specified world name
	bool					RunWorld(const char* pszWorldName);

private:

	void			DockControlBarRightOf(CToolBar* Bar,CToolBar* LeftOf);
	void			RestoreRegistryInfo( );
	void			SaveRegistryInfo( );


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int nCmdShow = -1);
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members

	int					m_WorldEditMode;
	int					m_NodeSelectionMode;

	BOOL				m_bShown;

	CStatusBar			m_wndStatusBar;

	CFlatToolbar		m_wndMainBar;
	CFlatToolbar		m_NodeViewToolbar;
	CFlatToolbar		m_WorldEditToolbar;

	// The project bar
	CProjectBar			*m_pProjectBar;
	
	CNodeView			*m_pNodeView;		// The node view
	CPropertiesDlg		*m_pPropertiesDlg;	// The properties dialog		
	ClassListDlg		*m_pClassListDlg;	// The class list
	CTextureDlg			*m_pTextureDlg;		// The texture dialog
	CModelDlg			*m_pModelDlg;		// The model dialog
	CWorldsDlg			*m_pWorldsDlg;		// The world dialog
	CSoundDlg			*m_pSoundDlg;		// The sound dialog
	CSpriteDlg			*m_pSpriteDlg;		// The sprite dialog
	CPrefabDlg			*m_pPrefabDlg;		// The prefab dialog

	// These are the project control bars that the MainFrame owns.
	// The rest of the control bars should be owned by the project bar.
	CMoArray<CProjectControlBarInfo *>	m_projectControls;

	BOOL				m_bToolTips;
	BOOL				m_bClosing;

	CEditClipboard		m_Clipboard;

	CMenu				m_modeMenu;			// The mode menu which is added dynamically
	
	BOOL				m_bIgnoreMsgsInPrefabs;

// Generated message map functions
public:
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnSelectTab(UINT id);
	afx_msg void OnRunWorld();
	afx_msg void OnUpdateRunWorld(CCmdUI *pCmdUI);
	afx_msg void OnShowWorldClasses();
	afx_msg void OnUpdateViewDebugWindow(CCmdUI *pCmdUI);
	afx_msg void OnViewDebugWindow();
	afx_msg void OnUpdateViewTexturePalette(CCmdUI *pCmdUI);
	afx_msg void OnViewTexturePalette();
	afx_msg void OnUpdateSingleNodeSelection( CCmdUI *pCmdUI );
	afx_msg void OnUpdateMultiNodeSelection( CCmdUI *pCmdUI );
	afx_msg void OnSingleNodeSelection();
	afx_msg void OnMultiNodeSelection();
	afx_msg void OnCreateRezFile();
	afx_msg void OnConvertEd2LTA();
	afx_msg void OnReloadPalettes();
	afx_msg void OnClose();
	afx_msg void OnTexturePass1();
	afx_msg void OnTexturePass2();
	afx_msg void OnUpdateProjectOpen( CCmdUI *pCmdUI );
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewProjectWindow();
	afx_msg void OnUpdateViewProjectWindow(CCmdUI* pCmdUI);
	afx_msg void OnPrefabTracker();
	afx_msg void OnUpdatePrefabTracker(CCmdUI* pCmdUI);
	afx_msg void OnObjectTracker();
	afx_msg void OnUpdateObjectTracker(CCmdUI* pCmdUI);
	afx_msg void OnTextureTracker();
	afx_msg void OnUpdateTextureTracker(CCmdUI* pCmdUI);
	afx_msg void OnSearchReplaceInTextures();
	afx_msg void OnUpdateSearchReplaceInTextures(CCmdUI* pCmdUI);
	afx_msg void OnViewColorSelection();
	afx_msg void OnUpdateViewColorSelection(CCmdUI* pCmdUI);
	afx_msg void OnViewLevelTextures();
	afx_msg void OnUpdateViewLevelTextures(CCmdUI* pCmdUI);
	afx_msg void OnViewObjectSearch();
	afx_msg void OnUpdateViewObjectSearch(CCmdUI* pCmdUI);
	afx_msg void OnObjectSelFilter();
	afx_msg void OnUpdateObjectSelFilter(CCmdUI* pCmdUI);
	afx_msg void OnViewLevelError();
	afx_msg void OnUpdateViewLevelError(CCmdUI* pCmdUI);
	afx_msg void OnViewLevelItems();
	afx_msg void OnUpdateViewLevelItems(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnGeometryEditMode();
	afx_msg void OnUpdateGeometryEditMode(CCmdUI* pCmdUI);
	afx_msg void OnObjectEditMode();
	afx_msg void OnUpdateObjectEditMode(CCmdUI* pCmdUI);
	afx_msg void OnBrushEditMode();
	afx_msg void OnUpdateBrushEditMode(CCmdUI* pCmdUI);
	afx_msg void OnCenterOnSelection();
	afx_msg void OnUpdateCenterOnSelection( CCmdUI *pCmdUI );
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnViewTooltips();
	afx_msg void OnUpdateViewTooltips(CCmdUI* pCmdUI);
	afx_msg void OnOptions();
	afx_msg void OnPaletteEdit();
	afx_msg void OnUpdateMDINext( CCmdUI* pCmdUI);
	afx_msg void OnToggleEditMode();
	afx_msg void OnCenterOnMarker();
	afx_msg void OnUpdateCenterOnMarker(CCmdUI* pCmdUI);
	afx_msg void OnViewToggleFloatingToolbars();
	afx_msg LRESULT OnModelLoaded(WPARAM, LPARAM);
	afx_msg void OnRenameResources();
	afx_msg void OnUpdateRenameResources( CCmdUI* pCmdUI);
	afx_msg void OnUpdateWorldObjects();
	afx_msg void OnUpdateUpdateWorldObjects( CCmdUI* pCmdUI);
	afx_msg void OnIgnoreMsgsInPrefabs();
	afx_msg void OnUpdateIgnoreMsgsInPrefabs( CCmdUI* pCmdUI );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CProjectControlBarInfo
{
public:
	CProjectControlBarInfo() {}
	
	CProjectControlBarInfo(CMRCSizeDialogBar *pWnd, DWORD title, DWORD dlg, DWORD selectID, CMainFrame::ProjectControl controlType)
	{
		m_pWnd = pWnd;					
		m_TitleStringID = title;
		m_DialogID = dlg;
		m_SelectID = selectID;
		m_ControlType=controlType;
	}

	CMRCSizeDialogBar			*m_pWnd;
	DWORD						m_TitleStringID;
	DWORD						m_DialogID;	
	DWORD						m_SelectID;
	CMainFrame::ProjectControl	m_ControlType;
};

typedef CMoArray<CProjectControlBarInfo *> CProjectControlBarInfoArray;

/////////////////////////////////////////////////////////////////////////////


#endif  // __MAINFRM_H__



