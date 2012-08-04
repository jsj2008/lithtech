// RegionView.cpp : implementation file
//

#include "bdefs.h"
#include "objectimporter.h"
#include "dedit.h"
#include "regiondoc.h"
#include "regionview.h"
#include "regionframe.h"
#include "usefuldc.h"
#include "editpoly.h"
#include "stringdlg.h"
#include "edithelpers.h"
#include "addobjectdlg.h"
#include "propertiesdlg.h"
#include "worldinfodlg.h"
#include "resource.h"
#include "classdlg.h"
#include "mainfrm.h"
#include "maptexturecoordsdlg.h"
#include "projectbar.h"
#include "geomroutines.h"
#include "rotateselection.h"
#include "texture.h"
#include "edit_actions.h"
#include "de_world.h"
#include "resource.h"
#include "node_ops.h"
#include "colorpicker.h"
#include "navigatordlg.h"
#include "navigatorstoredlg.h"
#include "advancedselect.h"
#include "scaleselectdlg.h"
#include "advancedselectdlg.h"
#include "objectbrowserdlg.h"
#include "optionsobjectbrowser.h"
#include "vectoredit.h"
#include "generateuniquenamesdlg.h"
#include "namechangereportdlg.h"
#include "cobjinterface.h"
#include "optionsgenerateuniquenames.h"
#include "stdrvtrackers.h"
#include "globalhotkeydb.h"
#include "bindkeydlg.h"
#include "ImportObjFileFormat.h"
#include "eventnames.h"
#include "ltamgr.h"
#include "meshshape.h"
#include "meshshapelist.h"
#include "camerafovdlg.h"
#include "polylightmap.h"
#include "optionsmisc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern BOOL g_bCheckNodes;

// MIKE 12/29/97 - Removed DoGroupNodes and merged it with OnSelectionGrouping because
//                 the undo stuff needed to be together.



//---------------------------------------------------------------------------
//
//  Statusbar strings
//
//  Purpose:	Global strings used by status bar.
//
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CRegionView

IMPLEMENT_DYNCREATE(CRegionView, CView)

CRegionView::CRegionView()
{
	m_TagDrawRect.SetRect( -1, -1, -1, -1 );
	
	// Setup all the defaults for the editing view.
	m_bInFocus				= FALSE;

	m_EditState				= EDIT_NOSTATE;
	m_dwGridSpacing			= 64;
	m_dwMajorGridSpacing	= 256;

	m_CurMouseOverHandle	= -1;

	m_CursorArrow	= AfxGetApp()->LoadStandardCursor( IDC_ARROW );
	m_CursorNS		= AfxGetApp()->LoadStandardCursor( IDC_SIZENS );
	m_CursorWE		= AfxGetApp()->LoadStandardCursor( IDC_SIZEWE );
	m_CursorNWSE	= AfxGetApp()->LoadStandardCursor( IDC_SIZENWSE );
	m_CursorNESW	= AfxGetApp()->LoadStandardCursor( IDC_SIZENESW );
	m_CursorSizeAll = AfxGetApp()->LoadStandardCursor( IDC_SIZEALL );

	m_pRegion		= NULL;
	SetViewRenderRegionView(this);
	
	m_rSelectDist				= 0.0f;
	m_nCurrentTextureVertIndex	= 0;

	m_bMouseOver				= FALSE;

	InitTracker();
}

CRegionView::~CRegionView()
{
// {MD 4/14/98} Apparently we don't need to free these since they're standard cursors.
//	::DeleteObject( m_CursorArrow );
//	::DeleteObject( m_CursorNS );
//	::DeleteObject( m_CursorWE );
}


BEGIN_MESSAGE_MAP(CRegionView, CView)
	ON_WM_CONTEXTMENU()
	//{{AFX_MSG_MAP(CRegionView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_COMMAND(ID_SHRINK_GRID_SPACING, OnShrinkGridSpacing)
	ON_UPDATE_COMMAND_UI(ID_SHRINK_GRID_SPACING, OnUpdateShrinkGridSpacing)
	ON_COMMAND(ID_EXPAND_GRID_SPACING, OnExpandGridSpacing)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_GRID_SPACING, OnUpdateExpandGridSpacing)
	ON_UPDATE_COMMAND_UI(ID_APPLY_COLOR, OnUpdateApplyColor)
	ON_COMMAND(ID_APPLY_COLOR, OnApplyColor)
	ON_COMMAND(ID_APPLY_TEXTURE, OnApplyTexture)
	ON_UPDATE_COMMAND_UI(ID_APPLY_TEXTURE, OnUpdateApplyTexture)
	ON_COMMAND(ID_REMOVE_TEXTURE, OnRemoveTexture)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_TEXTURE, OnUpdateRemoveTexture)
	ON_COMMAND(ID_HOLLOW_BRUSH, OnHollowBrush)
	ON_UPDATE_COMMAND_UI(ID_HOLLOW_BRUSH, OnUpdateSingleBrushOperation)
	ON_COMMAND(ID_IMPORT_TERRAIN_MAP, OnImportTerrainMap )
	ON_COMMAND(ID_ADDPRIM_BOX, OnCreatePrimitiveBox )
	ON_UPDATE_COMMAND_UI(ID_ADDPRIM_BOX, OnUpdateCreatePrimitiveBox)
	ON_COMMAND(ID_ADDPRIM_CYLINDER, OnCreatePrimitiveCylinder )
	ON_UPDATE_COMMAND_UI(ID_ADDPRIM_CYLINDER, OnUpdateCreatePrimitiveCylinder)
	ON_COMMAND(ID_ADDPRIM_PYRAMID, OnCreatePrimitivePyramid )
	ON_UPDATE_COMMAND_UI(ID_ADDPRIM_PYRAMID, OnUpdateCreatePrimitivePyramid)
	ON_COMMAND(ID_ADDPRIM_SPHERE, OnCreatePrimitiveSphere)
	ON_UPDATE_COMMAND_UI(ID_ADDPRIM_SPHERE, OnUpdateCreatePrimitiveSphere)
	ON_COMMAND(ID_ADDPRIM_PLANE, OnCreatePrimitivePlane)
	ON_UPDATE_COMMAND_UI(ID_ADDPRIM_PLANE, OnUpdateCreatePrimitivePlane)
	ON_COMMAND(ID_ADDPRIM_DOME, OnCreatePrimitiveDome)
	ON_UPDATE_COMMAND_UI(ID_ADDPRIM_DOME, OnUpdateCreatePrimitiveDome)
	ON_COMMAND(ID_POPUP_FUNCTIONS_REMOVEEXTRAEDGES, OnRemoveExtraEdges)
	ON_COMMAND(ID_POPUP_FUNCTIONS_MAPTEXTURECOORDS, OnMapTextureCoords)
	ON_UPDATE_COMMAND_UI(ID_POPUP_FUNCTIONS_MAPTEXTURECOORDS, OnUpdateMapTextureCoords)
	ON_COMMAND(ID_POPUP_FUNCTIONS_ROTATESELECTION, OnRotateSelection)
	ON_UPDATE_COMMAND_UI(ID_POPUP_FUNCTIONS_ROTATESELECTION, OnUpdateRotateSelection)
	ON_COMMAND(ID_POPUP_FUNCTIONS_RESETALLTEXTURECOORDINATES, OnResetAllTextureCoords)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_POPUP_FUNCTIONS_JOINTAGGEDVERTICES, OnJoinTaggedVertices)
	ON_COMMAND(ID_OBJECT_HEIGHT, OnObjectHeight)
	ON_COMMAND(ID_POPUP_ADDOBJECT, OnAddObject)
	ON_UPDATE_COMMAND_UI(ID_POPUP_ADDOBJECT, OnUpdateAddObject)
	ON_COMMAND(ID_POPUP_BINDTOOBJECT, OnBindToObject)
	ON_UPDATE_COMMAND_UI(ID_POPUP_BINDTOOBJECT, OnUpdateBindToObject)
	ON_COMMAND(ID_POPUP_OPTIONS_SHOWWIREFRAME, OnShowWireframe)
	ON_COMMAND(ID_POPUP_OBJECTPROPERTIES, OnObjectProperties)
	ON_COMMAND(ID_POPUP_WORLDINFO, OnWorldInfo)
	ON_COMMAND(ID_POPUP_SHADEMODE_WIREFRAMEPOLIES, OnShadeWireframe)
	ON_UPDATE_COMMAND_UI(ID_POPUP_SHADEMODE_WIREFRAMEPOLIES, OnUpdateShadeWireframe)
	ON_COMMAND(ID_POPUP_SHADEMODE_TEXTUREDPOLIES, OnShadeTextured)
	ON_UPDATE_COMMAND_UI(ID_POPUP_SHADEMODE_TEXTUREDPOLIES, OnUpdateShadeTextured)
	ON_COMMAND(ID_POPUP_SHADEMODE_FLATPOLIES, OnShadeFlat)
	ON_UPDATE_COMMAND_UI(ID_POPUP_SHADEMODE_FLATPOLIES, OnUpdateShadeFlat)
	ON_COMMAND(ID_POPUP_SHADEMODE_LIGHTMAPSONLY, OnShadeLightmapsOnly)
	ON_UPDATE_COMMAND_UI(ID_POPUP_SHADEMODE_LIGHTMAPSONLY, OnUpdateShadeLightmapsOnly)
	ON_COMMAND(ID_POPUP_OPTIONS_SHOWOBJECTS, OnShowObjects)
	ON_COMMAND(ID_POPUP_OPTIONS_SHOWNORMALS, OnShowNormals)
	ON_COMMAND(ID_POPUP_OPTIONS_BACKFACINGPOLIES, OnBackfacingPolies)
	ON_COMMAND(ID_POPUP_VIEWMODE_BACKVIEW, OnBackView)
	ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWMODE_BACKVIEW, OnUpdateBackView)
	ON_COMMAND(ID_POPUP_VIEWMODE_BOTTOMVIEW, OnBottomView)
	ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWMODE_BOTTOMVIEW, OnUpdateBottomView)
	ON_COMMAND(ID_POPUP_VIEWMODE_FRONTVIEW, OnFrontView)
	ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWMODE_FRONTVIEW, OnUpdateFrontView)
	ON_COMMAND(ID_POPUP_VIEWMODE_LEFTVIEW, OnLeftView)
	ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWMODE_LEFTVIEW, OnUpdateLeftView)
	ON_COMMAND(ID_POPUP_VIEWMODE_PERSPECTIVEVIEW, OnPerspectiveView)
	ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWMODE_PERSPECTIVEVIEW, OnUpdatePerspectiveView)
	ON_COMMAND(ID_POPUP_VIEWMODE_RIGHTVIEW, OnRightView)
	ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWMODE_RIGHTVIEW, OnUpdateRightView)
	ON_COMMAND(ID_POPUP_VIEWMODE_TOPVIEW, OnTopView)
	ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWMODE_TOPVIEW, OnUpdateTopView)
	ON_COMMAND(ID_POPUP_PROCESSWORLD, OnProcessWorld)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_POPUP_FUNCTIONS_CLEANUPGEOMETRY, OnCleanupGeometry)
	ON_COMMAND(ID_POPUP_OPTIONS_SHOWEDITGRID, OnShowEditGrid)
 	ON_COMMAND(ID_POPUP_OPTIONS_SHOWMARKER, OnShowMarker)
	ON_COMMAND(ID_POPUP_VIEWMODE_4VIEWCONFIGURATION, On4ViewConfiguration)
	ON_UPDATE_COMMAND_UI(ID_POPUP_VIEWMODE_4VIEWCONFIGURATION, OnUpdate4ViewConfiguration)
	ON_UPDATE_COMMAND_UI( ID_INDICATOR_VIEWTYPE, OnUpdateStatus )
	ON_COMMAND(ID_USEDEVICE1, OnUseDevice1)
	ON_COMMAND(ID_USEDEVICE2, OnUseDevice2)
	ON_COMMAND(ID_USEDEVICE3, OnUseDevice3)
	ON_COMMAND(ID_USEDEVICE4, OnUseDevice4)
	ON_COMMAND(ID_RENDERMODE1, OnRenderMode1)
	ON_COMMAND(ID_RENDERMODE2, OnRenderMode2)
	ON_COMMAND(ID_RENDERMODE3, OnRenderMode3)
	ON_COMMAND(ID_RENDERMODE4, OnRenderMode4)
	ON_COMMAND(ID_POPUP_REPLACEALLTEXTURES,OnReplaceTextures)
	ON_UPDATE_COMMAND_UI(ID_POPUP_REPLACEALLTEXTURES, OnUpdateReplaceTextures)
	ON_COMMAND(ID_REPLACE_TEXTURES_IN_SEL,OnReplaceTexturesInSel)
	ON_UPDATE_COMMAND_UI(ID_REPLACE_TEXTURES_IN_SEL, OnUpdateReplaceTexturesInSel)
	ON_COMMAND(ID_POPUP_SELECTTEXTURE,OnSelectTexture)
	ON_UPDATE_COMMAND_UI(ID_POPUP_SELECTTEXTURE, OnUpdateSelectTexture)
	ON_COMMAND(ID_SELECT_PREFAB, OnSelectPrefab)
	ON_UPDATE_COMMAND_UI(ID_SELECT_PREFAB, OnUpdateSelectPrefab)
	ON_COMMAND(ID_SELECTBRUSHCOLOR,OnSelectBrushColor)
	ON_UPDATE_COMMAND_UI(ID_SELECTBRUSHCOLOR, OnUpdateSelectBrushColor)
	ON_COMMAND(ID_RESET_DETAIL_LEVELS, OnResetDetailLevels)
	ON_COMMAND(ID_WORLD_SPECIAL_UPDATE_PROPS, OnUpdateAllObjectProperties)
	ON_COMMAND(ID_NAVIGATOR_STORE, OnNavigatorStore)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATOR_STORE, OnUpdateNavigatorStore)
	ON_COMMAND(ID_NAVIGATOR_ORGANIZE, OnNavigatorOrganize)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATOR_ORGANIZE, OnUpdateNavigatorOrganize)
	ON_COMMAND(ID_NAVIGATOR_GOTONEXT, OnNavigatorGotoNext)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATOR_GOTONEXT, OnUpdateNavigatorGotoNext)
	ON_COMMAND(ID_NAVIGATOR_GOTOPREVIOUS, OnNavigatorGotoPrevious)
	ON_UPDATE_COMMAND_UI(ID_NAVIGATOR_GOTOPREVIOUS, OnUpdateNavigatorGotoPrevious)
	ON_COMMAND(ID_SELECTION_SELECTALL, OnSelectionSelectAll)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_SELECTALL, OnUpdateSelectionSelectAll)
	ON_COMMAND(ID_SELECTION_SELECTNONE, OnSelectionSelectNone)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_SELECTNONE, OnUpdateSelectionSelectNone)
	ON_COMMAND(ID_SELECTION_SELECTINVERSE, OnSelectionSelectInverse)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_SELECTINVERSE, OnUpdateSelectionSelectInverse)
	ON_COMMAND(ID_SELECTION_ADVANCED, OnSelectionAdvanced)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_ADVANCED, OnUpdateSelectionAdvanced)
	ON_COMMAND(ID_SELECTION_HIDESELECTED, OnSelectionHideSelected)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_HIDESELECTED, OnUpdateSelectionHideSelected)
	ON_COMMAND(ID_SELECTION_UNHIDESELECTED, OnSelectionUnhideSelected)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_UNHIDESELECTED, OnUpdateSelectionUnhideSelected)
	ON_COMMAND(ID_SELECTION_GROUP, OnSelectionGroup)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_GROUP, OnUpdateSelectionGroup)
	ON_COMMAND(ID_WORLD_IMPORT_OBJECTS, OnWorldImportObjects)
	ON_COMMAND(ID_BRUSH_SPLITBRUSH, OnBrushSplitBrush)
	ON_UPDATE_COMMAND_UI(ID_BRUSH_SPLITBRUSH, OnUpdateBrushSplitBrush)
	ON_COMMAND(ID_SET_CAMERA_FOV, OnSetCameraFOV)
	ON_COMMAND(ID_BRUSH_FLIP, OnBrushFlip)
	//ON_UPDATE_COMMAND_UI(ID_BRUSH_CARVE, OnUpdateBrushFlip)
	ON_COMMAND(ID_BRUSH_CARVE, OnBrushCarve)
	ON_UPDATE_COMMAND_UI(ID_BRUSH_CARVE, OnUpdateBrushCarve)
	ON_COMMAND(ID_SELECTION_MIRROR_X, OnSelectionMirrorX)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_MIRROR_X, OnUpdateSelectionMirrorX)
	ON_COMMAND(ID_SELECTION_MIRROR_Y, OnSelectionMirrorY)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_MIRROR_Y, OnUpdateSelectionMirrorY)
	ON_COMMAND(ID_SELECTION_MIRROR_Z, OnSelectionMirrorZ)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_MIRROR_Z, OnUpdateSelectionMirrorZ)
	ON_COMMAND(ID_SELECTION_CONTAINER, OnSelectionContainer)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_CONTAINER, OnUpdateSelectionContainer)
	ON_COMMAND(ID_CENTER_SELECTION_ON_MARKER, OnCenterSelectionOnMarker)
	ON_UPDATE_COMMAND_UI(ID_CENTER_SELECTION_ON_MARKER, OnUpdateCenterSelectionOnMarker)
	ON_COMMAND(ID_CENTER_MARKER_ON_SELECTION, OnCenterMarkerOnSelection)
	ON_UPDATE_COMMAND_UI(ID_CENTER_MARKER_ON_SELECTION, OnUpdateCenterMarkerOnSelection)
	ON_COMMAND(ID_PLACE_MARKER_AT_CAMERA, OnPlaceMarkerAtCamera)
	ON_UPDATE_COMMAND_UI(ID_PLACE_MARKER_AT_CAMERA, OnUpdatePlaceMarkerAtCamera)
	ON_COMMAND(ID_SELECTION_GENERATE_UNIQUE_NAMES, OnSelectionGenerateUniqueNames)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_GENERATE_UNIQUE_NAMES, OnUpdateSelectionGenerateUniqueNames)
	ON_COMMAND(ID_WORLD_OBJECT_BROWSER, OnWorldObjectBrowser)
	ON_COMMAND(ID_PLACE_MARKER_AT_VECTOR, OnPlaceMarkerAtVector)
	ON_COMMAND(ID_WORLD_DEBUG_FIND_NAMING_CONFLICTS, OnWorldDebugFindNamingConflicts)
	ON_COMMAND(ID_WORLD_SCALE_GEOMETRY, OnWorldScaleGeometry)
	ON_COMMAND(ID_MAP_TEXTURE_TO_VIEW, OnMapTextureToView)
	ON_UPDATE_COMMAND_UI(ID_MAP_TEXTURE_TO_VIEW, OnUpdateMapTextureToView)
	ON_COMMAND(ID_RESET_TEXTURE_COORDS, OnResetTextureCoords)
	ON_UPDATE_COMMAND_UI(ID_RESET_TEXTURE_COORDS, OnUpdateResetTextureCoords)
	ON_COMMAND(ID_SELECTION_HIDEINVERSE, OnSelectionHideInverse)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_HIDEINVERSE, OnUpdateSelectionHideInverse)
	ON_COMMAND(ID_SELECTION_UNHIDEINVERSE, OnSelectionUnhideInverse)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_UNHIDEINVERSE, OnUpdateSelectionUnhideInverse)
	ON_COMMAND(ID_FILE_IMPORT_WORLD, OnFileImportWorld)
	ON_COMMAND(ID_SELECTION_SAVEPREFAB, OnSelectionSavePrefab)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_SAVEPREFAB, OnUpdateSelectionSavePrefab)
	ON_COMMAND(ID_SELECTION_SCALE, OnSelectionScale)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_SCALE, OnUpdateSelectionScale)
	ON_UPDATE_COMMAND_UI(ID_IMPORT_TERRAIN_MAP, OnUpdateSingleBrushOperation )	
	ON_UPDATE_COMMAND_UI( ID_INDICATOR_CAMERA, OnUpdateStatus )
	ON_UPDATE_COMMAND_UI( ID_INDICATOR_GRID, OnUpdateStatus )
	ON_COMMAND(ID_WORLD_SPECIAL_ALIGNPOLYTEXTURES, OnAlignPolyTextures)
	ON_COMMAND(ID_VIEW_MAXIMIZEVIEW1, OnViewMaximizeView1)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAXIMIZEVIEW1, OnUpdateViewMaximizeView1)
	ON_COMMAND(ID_VIEW_MAXIMIZEVIEW2, OnViewMaximizeView2)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAXIMIZEVIEW2, OnUpdateViewMaximizeView2)
	ON_COMMAND(ID_VIEW_MAXIMIZEVIEW3, OnViewMaximizeView3)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAXIMIZEVIEW3, OnUpdateViewMaximizeView3)
	ON_COMMAND(ID_VIEW_MAXIMIZEVIEW4, OnViewMaximizeView4)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAXIMIZEVIEW4, OnUpdateViewMaximizeView4)
	ON_COMMAND(ID_VIEW_MAXIMIZEACTIVEVIEW, OnViewMaximizeActiveView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAXIMIZEACTIVEVIEW, OnUpdateViewMaximizeActiveView)
	ON_COMMAND(ID_SELECTION_FREEZE_SELECTED, OnSelectionFreezeSelected)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_FREEZE_SELECTED, OnUpdateSelectionFreezeSelected)
	ON_COMMAND(ID_SELECTION_UNFREEZE_ALL, OnSelectionUnfreezeAll)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_UNFREEZE_ALL, OnUpdateSelectionUnfreezeAll)
	ON_COMMAND(ID_SNAP_VERTICES, OnSnapVertices)
	ON_UPDATE_COMMAND_UI(ID_SNAP_VERTICES, OnUpdateSnapVertices)
	
	ON_COMMAND(ID_VIEW_DISPLAY_ALL_MODELS, OnDisplayAllModels)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DISPLAY_ALL_MODELS, OnUpdateDisplayAllModels)
	ON_COMMAND(ID_VIEW_HIDE_ALL_MODELS, OnHideAllModels)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIDE_ALL_MODELS, OnUpdateHideAllModels)
	ON_COMMAND(ID_VIEW_DISPLAY_SELECTED_MODELS, OnDisplaySelectedModels)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DISPLAY_SELECTED_MODELS, OnUpdateDisplaySelectedModels)
	ON_COMMAND(ID_VIEW_HIDE_SELECTED_MODELS, OnHideSelectedModels)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIDE_SELECTED_MODELS, OnUpdateHideSelectedModels)
	ON_COMMAND(ID_VIEW_DISPLAY_MODELS_OF_CLASS, OnDisplayModelsOfClass)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DISPLAY_MODELS_OF_CLASS, OnUpdateDisplayModelsOfClass)
	ON_COMMAND(ID_VIEW_HIDE_MODELS_OF_CLASS, OnHideModelsOfClass)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIDE_MODELS_OF_CLASS, OnUpdateHideModelsOfClass)
	ON_COMMAND(ID_VIEW_DISPLAY_MODEL_POLYCOUNT, OnDisplayModelPolycount)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DISPLAY_MODEL_POLYCOUNT, OnUpdateDisplayModelPolycount)
	ON_COMMAND(ID_FIT_TEXTURE_TO_POLY, OnFitTextureToPoly)
	ON_UPDATE_COMMAND_UI(ID_FIT_TEXTURE_TO_POLY, OnUpdateFitTextureToPoly)
	ON_COMMAND(ID_TOGGLE_CLASS_ICONS, OnToggleClassIcons)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_CLASS_ICONS, OnUpdateToggleClassIcons)
	ON_COMMAND(ID_HIDE_FROZEN_NODES, OnHideFrozenNodes)
	ON_UPDATE_COMMAND_UI(ID_HIDE_FROZEN_NODES, OnUpdateHideFrozenNodes)
	ON_COMMAND(ID_CAMERA_TO_OBJECT, OnCameraToObject)
	ON_UPDATE_COMMAND_UI(ID_CAMERA_TO_OBJECT, OnUpdateCameraToObject)
	ON_COMMAND(ID_DISCONNECT_SELECTED_PREFABS, OnDisconnectSelectedPrefabs)
	ON_UPDATE_COMMAND_UI(ID_DISCONNECT_SELECTED_PREFABS, OnUpdateDisconnectSelectedPrefabs)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()





inline void CRegionView::CheckAMenuItem( CMenu *pMenu, UINT nItem, int testVal1, int testVal2 )
{
	pMenu->CheckMenuItem( nItem, (testVal1==testVal2) ? (MF_CHECKED|MF_BYCOMMAND) : (MF_UNCHECKED|MF_BYCOMMAND) );
}



inline void CRegionView::EnableAMenuItem( CMenu *pMenu, UINT nItem, int testVal1, int testVal2 )
{
	pMenu->EnableMenuItem( nItem, (testVal1 == testVal2) ? (MF_ENABLED | MF_BYCOMMAND) : (MF_GRAYED | MF_BYCOMMAND));
}

//determines if any polygons are selected for editing. This occurs when
//in geometry mode and the cursor is over a face, or if it is brush mode and
//a brush is selected
BOOL CRegionView::AreAnyPoliesSelected()
{
	if(GetEditMode() == GEOMETRY_EDITMODE)
	{
		return (TaggedPolies().GetSize() > 0) ? TRUE : FALSE;
	}
	else
	{
		return (GetRegion()->m_Selections.GetSize() > 0) ? TRUE : FALSE;
	}
}

BOOL CRegionView::OnIdle( LONG lCount )
{
	BOOL bTrackerBusy;

	BEGIN_MEMORYEXCEPTION()
	{
		CPoint		point = GetCurMousePos();
		BOOL		bDraw = FALSE;
		CRect		rect;


		if( !m_bInFocus || !m_pRegion )
			return TRUE;

		bTrackerBusy = m_cTrackerMgr.ProcessEvent(CUIEvent(UIEVENT_NONE));
		// Update the immediate selection information  (Note : This should be moved into a tracker...)
		if ( (!bTrackerBusy) && (point != m_LastMousePos))
		{
			if( UpdateImmediateSelection() )
			{
				GetRegionDoc()->UpdateAllViews( this );
				GetClientRect( &rect );
				bDraw = TRUE;
			}
		}

		if( bDraw )
			DrawRect( &rect );

		if(!bTrackerBusy)
			UpdateCursor();
		
		m_LastMousePos = point;

		UpdateStatusBar( );
	}
	END_MEMORYEXCEPTION()

	return bTrackerBusy;
}


void CRegionView::SetObjectEditMode()
{
	((CMainFrame *)AfxGetMainWnd())->OnObjectEditMode();
}

void CRegionView::SetGeometryEditMode()
{
	((CMainFrame *)AfxGetMainWnd())->OnGeometryEditMode();
}

void CRegionView::SetBrushEditMode()
{
	((CMainFrame *)AfxGetMainWnd())->OnBrushEditMode();
}

void CRegionView::NextTextureLayer()
{
	SetCurrTexture((GetCurrTexture() + 1) % CEditPoly::NUM_TEXTURES);
}


/////////////////////////////////////////////////////////////////////////////
// CRegionView drawing

void CRegionView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();

	BEGIN_MEMORYEXCEPTION()
	{
		DrawRect( NULL );
	}
	END_MEMORYEXCEPTION()
}


void CRegionView::DrawRect(CRect *pRect)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	//don't render if we don't have any area to draw in (this saves a lot of time when
	//views are minimized)
	if((rcClient.Width() < 2) || (rcClient.Height() < 2))
	{
		return;
	}

	CViewRender::DrawRect(pRect);

	if (m_bInFocus)
	{
		CRect cCorner;
		CDC *pDC = GetDC();
		if (pDC)
		{
			cCorner = rcClient;
			cCorner.DeflateRect(1,1,1,1);
			pDC->DrawEdge(cCorner, BDR_SUNKENOUTER, BF_TOPLEFT | BF_BOTTOMRIGHT);
			pDC->DrawEdge(rcClient, BDR_RAISEDINNER, BF_TOPLEFT | BF_BOTTOMRIGHT);
			ReleaseDC(pDC);
		}
	}
}


void CRegionView::GetRenderRect(LPRECT pRect, BOOL bClient)
{
	if (bClient)
		GetClientRect(pRect);
	else
		GetWindowRect(pRect);
	if (GetInFocus())
	{
		pRect->top += 2;
		pRect->left += 2;
		pRect->bottom -= 2;
		pRect->right -= 2;
	}
}



/////////////////////////////////////////////////////////////////////////////
// CRegionView diagnostics

#ifdef _DEBUG
void CRegionView::AssertValid() const
{
	CView::AssertValid();
}

void CRegionView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG






/////////////////////////////////////////////////////////////////////////////
// CRegionView message handlers

void CRegionView::OnInitialUpdate() 
{
	CRect rcClient;
	int cx, cy;	

	CView::OnInitialUpdate();
	
	// Status message..
	GetClientRect(&rcClient);
	cx = rcClient.Width();
	cy = rcClient.Height();
	TRACE( "New World View: (%d, %d)\n", cx, cy );
	

	// Install the main timer.
	m_TimerID = SetTimer( 10, 1, NULL );

	// Get the region pointer and set it in various structures.
	m_pRegion = &GetRegionDoc()->m_Region;

	SetupInitialDrawMgr();		
}

void CRegionView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	BEGIN_MEMORYEXCEPTION()
	{
		if(cx && cy)
		{
			SetupNewSize( cx, cy );
		}	
	}
	END_MEMORYEXCEPTION()
}

BOOL CRegionView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CRegionView::OnDestroy() 
{
	CView::OnDestroy();
	
	KillTimer( m_TimerID );
}

//--------------------------------------------------------------------------------------
//
// CRegionView::OnLButtonDown
//
// Purpose:  Message handler for left button down.
//
//--------------------------------------------------------------------------------------
void CRegionView::OnLButtonDown( UINT nFlags, CPoint point ) 
{
	BEGIN_MEMORYEXCEPTION()
	{
		//SetCapture();

		m_cTrackerMgr.ProcessEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSELEFT, point));
	}
	END_MEMORYEXCEPTION()
	
	CView::OnLButtonDown(nFlags, point);
}

BOOL CRegionView::OnMouseWheel( UINT nFlags, short zDelta, CPoint point )
{
	// convert screen coordinates to window coordinates:
	CRect rect;
	GetWindowRect( &rect );
	CPoint cursorPt = point - CPoint( rect.left, rect.top );

	m_cTrackerMgr.ProcessEvent( CUIMouseEvent( UIEVENT_MOUSEWHEEL, UI_MOUSENONE, cursorPt, zDelta ) );
	return CView::OnMouseWheel( nFlags, zDelta, point );
}

void CRegionView::OnMButtonDown( UINT nFlags, CPoint point ) 
{
	BEGIN_MEMORYEXCEPTION()
	{
		m_cTrackerMgr.ProcessEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSEMIDDLE, point));
	}
	END_MEMORYEXCEPTION()
	
	CView::OnMButtonDown(nFlags, point);
}


void CRegionView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	BEGIN_MEMORYEXCEPTION()
	{
		BOOL		bDraw = FALSE;
		
		bDraw |= m_cTrackerMgr.ProcessEvent(CUIMouseEvent(UIEVENT_MOUSEUP, UI_MOUSEMIDDLE, point));

		if (!bDraw)
			bDraw |= UpdateImmediateSelection();

		if( bDraw )
			DrawRect();
	}
	END_MEMORYEXCEPTION()

	CView::OnMButtonUp(nFlags, point);
}


void CRegionView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRegionDoc *pDoc;
	CRegionView *pRegionView;
	POSITION pos;

	if (!m_cTrackerMgr.ProcessEvent(CUIMouseEvent(UIEVENT_MOUSEMOVE, UI_MOUSENONE, point)))
	{

		// Calculate the distance moved from last mouse point...
		LONG dist;
		dist = ( point.x - m_MousePoint.x ) * ( point.x - m_MousePoint.x ) + 
			( point.y - m_MousePoint.y ) * ( point.y - m_MousePoint.y );

		m_MousePoint = point;
		// Reset selection distance if mouse moved enough...
		if( dist > 3 )
			m_rSelectDist = 0.0f;	
	}

	if(!m_bMouseOver) 
	{
		m_bMouseOver = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize		= sizeof (tme);
		tme.dwFlags		= TME_HOVER | TME_LEAVE;
		tme.hwndTrack	= m_hWnd;
		tme.dwHoverTime = HOVER_DEFAULT;
		_TrackMouseEvent(&tme);
	}

	CView::OnMouseMove(nFlags, point);
}


void CRegionView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	BEGIN_MEMORYEXCEPTION()
	{
		BOOL		bDraw = FALSE;
		
		bDraw |= m_cTrackerMgr.ProcessEvent(CUIMouseEvent(UIEVENT_MOUSEUP, UI_MOUSELEFT, point));

		if (!bDraw)
			bDraw |= UpdateImmediateSelection();

		if( bDraw )
			DrawRect();
	}
	END_MEMORYEXCEPTION()

	CView::OnLButtonUp(nFlags, point);
}


void CRegionView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	BEGIN_MEMORYEXCEPTION()
	{
		m_bDoMenu = !m_cTrackerMgr.ProcessEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSERIGHT, point));
	}
	END_MEMORYEXCEPTION()
	
	// SetCapture();
	CView::OnRButtonDown(nFlags, point);
}


void CRegionView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	BEGIN_MEMORYEXCEPTION()
	{
		BOOL		bDraw = FALSE;

		if (!m_cTrackerMgr.ProcessEvent(CUIMouseEvent(UIEVENT_MOUSEUP, UI_MOUSERIGHT, point)))
			UpdateImmediateSelection();

		if( bDraw )
			DrawRect();

		// ReleaseCapture();	
		CView::OnRButtonUp(nFlags, point);
	}
	END_MEMORYEXCEPTION()
}


void CRegionView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if( m_bInFocus )
	{
		BEGIN_MEMORYEXCEPTION()
		{
			m_cTrackerMgr.ProcessEvent(CUIKeyEvent(UIEVENT_KEYDOWN, nChar, (nFlags & (1 << 14)) == 0));
		}
		END_MEMORYEXCEPTION()
	}
	
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CRegionView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BEGIN_MEMORYEXCEPTION()
	{	
		m_cTrackerMgr.ProcessEvent(CUIKeyEvent(UIEVENT_KEYUP, nChar));
	}
	END_MEMORYEXCEPTION()
	
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}



static void TriangulateBrushPolies(CEditBrush *pBrush)
{
	DWORD			i, j, k;
	CEditPoly		*pPoly, *pNewPoly;
	CReal			dot;

	CMoArray<CEditPoly*>	toAdd;
	CMoArray<CBasePoly*>	tris;
	
	
	// Go thru all polies.
	for(i=0; i < pBrush->m_Polies; i++)
	{
		pPoly = pBrush->m_Polies[i];

		for(j=0; j < pPoly->NumVerts(); j++)
		{
			dot = pPoly->m_Plane.m_Normal.Dot(pPoly->Pt(j)) - pPoly->m_Plane.m_Dist;
			
			if(dot > 0.1f || dot < -0.1f)
			{
				// Ok, needs to be triangulated.  Attempt to do so.
				if(pPoly->GetTriangles(tris))
				{
					for(k=0; k < tris; k++)
					{
						pNewPoly = new CEditPoly;
						
						pNewPoly->m_pBrush = pBrush;
						pNewPoly->CopyAttributes(pPoly);
						pNewPoly->m_Indices.CopyArray(tris[k]->m_Indices);

						g_GenerateNormal(pNewPoly);

						toAdd.Append(pNewPoly);
					}

					// Remove the poly from the brush.
					delete pPoly;
					pBrush->m_Polies.Remove(i);
					--i;
				}

				DeleteAndClearArray(tris);
				break;
			}
		}
	}

	for(i=0; i < toAdd; i++)
		pBrush->m_Polies.Append(toAdd[i]);

	pBrush->UpdatePlanes();
}


void CRegionView::OnAutoTriangulate()
{
	DWORD i;
	CEditRegion *pRegion = GetRegion();
	CWorldNode *pNode;
	PreActionList actionList;
	GPOS pos;


	// Build a list of mods.
	for(i=0; i < pRegion->m_Selections; i++)
	{
		pNode = pRegion->m_Selections[i];
		
		if(pNode->GetType() == Node_Brush)
		{
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pNode));
		}
	}
	
	// Setup the undo.
	GetRegionDoc()->Modify(&actionList, FALSE);

	// Carry out the action.	
	for(pos=actionList; pos; )
	{
		TriangulateBrushPolies(actionList.GetNext(pos)->m_pNode->AsBrush());
	}

	if(actionList.GetSize() > 0)
	{
		GetRegionDoc()->RedrawAllViews();
		GDeleteAndRemoveElements(actionList);
	}
}

void CRegionView::OnStartPreprocessor() 
{
	m_pRegion->UpdatePlanes();

	GetRegionDoc()->StartPreProcessor();
	SetFocus();
}


void CRegionView::OnKillFocus(CWnd* pNewWnd) 
{
	m_bInFocus = FALSE;

	// Get rid of the focus rectangle 
	// (Note : Can't DrawRect here because focus changes before the window is completely created)
	Invalidate();

	//clear out any pending trackers
	m_cTrackerMgr.FlushTrackers();

	// Send an empty message to the trackers so they can pick up on the focus change
	m_cTrackerMgr.ProcessEvent(CUIEvent(UIEVENT_NONE));

	CView::OnKillFocus(pNewWnd);
	
}


void CRegionView::OnSetFocus(CWnd* pOldWnd) 
{
	m_bInFocus = TRUE;	
	
	// Make sure the focus rectangle shows up
	// (Note : Can't DrawRect here because focus changes before the window is completely created)
	Invalidate();

	
	//clear out any pending trackers
	m_cTrackerMgr.FlushTrackers();


	// Send an empty message to the trackers so they can pick up on the focus change
	m_cTrackerMgr.ProcessEvent(CUIEvent(UIEVENT_NONE));

	CView::OnSetFocus(pOldWnd);

}


void CRegionView::OnEditUndo() 
{
	BEGIN_MEMORYEXCEPTION()
	{
		CRect		rect;
		
		BeginWaitCursor();

		// Turn off the drawing of the node view
		if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
		{
			GetNodeView()->SetRedraw(FALSE);
		}

		// Get the node ID of the selected node so that it can be reselected after the Undo
		DWORD dwID=0;		
		CWorldNode *pSelectedNode=GetNodeView()->GetSelectedNode();
		if (pSelectedNode)
		{
			dwID=pSelectedNode->GetUniqueID();
		}

		GetPropertiesDlg()->Term();
		GetNodeView( )->Term( );
		ClearSelections( TRUE, TRUE, TRUE );
		m_EditState = EDIT_NOSTATE;
		
		GetRegionDoc()->m_UndoMgr.Undo();
		GetRegionDoc()->Modify();

		DrawRect();
		SetFocus();
		GetNodeView()->Init(GetRegionDoc());
		GetDocument()->UpdateAllViews( this );

		// Determine if the selected node is still in the world and reselect the node if possible
		if (pSelectedNode)
		{
			pSelectedNode=GetRegion()->FindNodeByID(dwID);
			if (pSelectedNode)
			{
				GetNodeView()->HighlightNode(pSelectedNode);
			}
		}

		// Turn on the drawing of the node view
		if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
		{
			GetNodeView()->SetRedraw(TRUE);
			GetNodeView()->Invalidate();	
		}

		EndWaitCursor();
	}
	END_MEMORYEXCEPTION()
}

void CRegionView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetRegionDoc()->m_UndoMgr.m_nUndos > 0 );	
	SetMenuHotKeyText(UIE_EDIT_UNDO, pCmdUI);
}


void CRegionView::OnEditRedo() 
{
	BEGIN_MEMORYEXCEPTION()
	{
		CRect		rect;
	
		// Turn off the drawing of the node view
		if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
		{
			GetNodeView()->SetRedraw(FALSE);
		}

		// Get the node ID of the selected node so that it can be reselected after the Undo
		DWORD dwID=0;		
		CWorldNode *pSelectedNode=GetNodeView()->GetSelectedNode();
		if (pSelectedNode)
		{
			dwID=pSelectedNode->GetUniqueID();
		}

		GetPropertiesDlg()->Term();
		GetNodeView( )->Term( );
		ClearSelections( TRUE, TRUE, TRUE );
		m_EditState = EDIT_NOSTATE;
												   
		GetRegionDoc()->m_UndoMgr.Redo();
		GetRegionDoc()->Modify();

		DrawRect();
		SetFocus();
		GetNodeView()->Init(GetRegionDoc());
		GetDocument()->UpdateAllViews( this );

		// Determine if the selected node is still in the world and reselect the node if possible
		if (pSelectedNode)
		{
			pSelectedNode=GetRegion()->FindNodeByID(dwID);
			if (pSelectedNode)
			{
				GetNodeView()->HighlightNode(pSelectedNode);
			}
		}

		// Turn on the drawing of the node view
		if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
		{
			GetNodeView()->SetRedraw(TRUE);
			GetNodeView()->Invalidate();		
		}
	}
	END_MEMORYEXCEPTION()
}

void CRegionView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetRegionDoc()->m_UndoMgr.m_nRedos > 0 );
	SetMenuHotKeyText(UIE_EDIT_REDO, pCmdUI);
}



void CRegionView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if( lHint == REGIONVIEWUPDATE_REDRAW )
	{
		DrawRect();
	}
	else if( lHint == REGIONVIEWUPDATE_EDITMODE )
	{
		if (GetInFocus())
		{
			//we need to first clear out any of the old selected vertices (this way
			//if we delete a brush that contains them, we won't crash)
			TaggedVerts().GenRemoveAll();
			TaggedBrushes().Term();
			TaggedPolies().SetSize(1);


			//we now also need to update any immediate selections to ensure they are
			//valid
			UpdateImmediateSelection();
		}

		DrawRect();
	}
	else if( lHint &  REGIONVIEWUPDATE_VIEWID)
	{
		//remove the mask to get the value of the view we want to
		//update
		uint32 nVal = lHint & ~REGIONVIEWUPDATE_VIEWID;

		//only update ourselves if this is a matching view
		if(nVal == m_nViewID)
			DrawRect();
	}
	else
	{
		if( (pSender != this) && (pSender != NULL) )
			DrawRect();	
	}

}

void CRegionView::OnUpdateAddObject(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_ADD_OBJECT, pCmdUI);
}

void CRegionView::OnAddObject() 
{
	BEGIN_MEMORYEXCEPTION()
	{	
		// See what type of object, if any, is selected in the code view.
		CClassDlg dlg;
		
		CString sTitle;
		sTitle.LoadString(IDS_CHOOSE_OBJECT_TYPE);

		dlg.SetProject(GetProject());
		dlg.SetTitle(sTitle);

		if( dlg.DoModal() == IDOK )
		{
			// Get the document
			CRegionDoc *pDoc=GetRegionDoc();
			if (!pDoc)
			{
				return;
			}

			// Get the region
			CEditRegion *pRegion=pDoc->GetRegion();

			// Add the object
			CWorldNode *pObject=pDoc->AddObject(dlg.GetSelectedClass(), GetRegionDoc()->GetRegion()->GetMarker());

			// Bind the node to the active parent
			pDoc->BindNode(pObject, pRegion->GetActiveParentNode());

			// Select the object
			pDoc->SelectNode(pObject);
		}	
	}
	END_MEMORYEXCEPTION()
}

/************************************************************************/
// Import objects from a .txt file
void CRegionView::OnWorldImportObjects() 
{
	CString sFileName;
	sFileName = GetProject()->m_BaseProjectDir + "\\*.txt";
	// Let the user chose the file to import from
	CFileDialog fileDlg(TRUE, "txt", (LPCTSTR)sFileName, OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, "Text (*.txt)|*.txt|All Files (*.*)|*.*||");

	// Display the dialog
	if (fileDlg.DoModal() == IDOK)
	{
		//clear our selections so that the newly added objects will be selected
		m_pRegion->ClearSelections();

		//open up each and every file
		POSITION Pos = fileDlg.GetStartPosition();

		while(Pos)
		{
			CString sFilename = fileDlg.GetNextPathName(Pos);

			// Create the object importer class
			CObjectImporter importer;
			
			// Import the objects from the file
			importer.LoadObjectFile(sFilename);
			importer.ImportObjects();
		}
	}	
}

/************************************************************************/
// Imports a world into this world
void CRegionView::OnFileImportWorld() 
{
	CString sFileName;
	sFileName = GetProject()->m_BaseProjectDir + "\\*.lta;*.ltc";
	// Let the user choose the file to import from
	CFileDialog fileDlg(TRUE, "lta", (LPCTSTR)sFileName, NULL, "LTA File (*.lta;*.ltc)|*.lta;*.ltc|Obj File (*.obj)|*.obj||");

	// Display the dialog
	if (fileDlg.DoModal() != IDOK)
	{
		return;
	}

	// Import the file, leaving its original location in tact without
	// pasting it at the green crosshair.
	CString fileName = fileDlg.GetFileName();
	fileName.MakeUpper();
 	if( strstr( fileName, ".OBJ" ) )
	{
		ImportOBJFile( fileName, FALSE /* Don't place at crosshair */ );
	}
	else if( strstr( fileName, ".LTA" ) || strstr( fileName, ".LTC" ))
	{
		ImportLTAFile( fileName, FALSE /* Don't place at crosshair */ );
	}
	else
	{
		MessageBox("Error: Unknown extension on selected file");
	}
}

/************************************************************************/
// This imports a file into the region and optionally positions it at
// the green crosshair location.
//
// Returns: TRUE if the file import was a success.
BOOL CRegionView::ImportLTAFile(CString sFilename, BOOL bPositionAtCrosshair)
{
		// The string holder for the region
	CStringHolder stringHolder;

	// The region class
	CEditRegion region;

	// Set the string holder for the region
	region.m_pStringHolder = GetRegion()->m_pStringHolder;

	// Load the region
	BeginWaitCursor();
	// RegionLoadStatus status=region.LoadFile(file, NULL, GetProject());
	uint32 nFileVersion;
	bool bBinary = false;
	RegionLoadStatus status=region.LoadFile(sFilename, GetProject(), nFileVersion, bBinary);
	EndWaitCursor();

	// Close the file
	

	// Handle the errors
	switch (status)
	{
	case REGIONLOAD_INVALIDFILE:
		{
			MessageBox("Error: Invalid file", "Error", MB_OK);
			return FALSE;
			break;
		}
	case REGIONLOAD_OUTOFMEMORY:
		{
			MessageBox("Error: Out of memory", "Error", MB_OK);
			return FALSE;
			break;
		}
	case REGIONLOAD_INVALIDFILEVERSION:
		{
			MessageBox("Error: Invalid file version", "Error", MB_OK);
			return FALSE;
			break;
		}
	case REGIONLOAD_OK:
		{
			// No errors
			break;
		}
	default:
		{
			MessageBox("Unknown error", "Error", MB_OK);
			return FALSE;
		}
	}

	// Update all the texture IDs and polygon pieces.
	region.UpdateTextureIDs(GetProject());

	//resolve any children
	region.GetPrefabMgr()->SetRootPath(region.GetPrefabMgr()->GetRootPath());
	region.GetPrefabMgr()->BindAllRefs(region.GetRootNode());

	bool bModified;
	region.PostLoadUpdateVersion(nFileVersion, bModified);


	// This stores the nodes that are going to be pasted
	CMoArray<CWorldNode*> nodeArray;

	// Set the cache size to the number of brushes plus the number of objects
	nodeArray.SetCacheSize(region.m_Brushes.GetSize()+region.m_Objects.GetSize());

	// Build the flattened array so that it can be pasted via the clipboard
	region.FlattenTreeToArray(&region.m_RootNode, nodeArray);

	// Remove the root node
	if (nodeArray.GetSize() > 0)
	{
		nodeArray.Remove(0);
	}

	// Paste the nodes
	if (nodeArray.GetSize() > 0)
	{
		// Don't check nodes in debug because it takes a very long time
		BOOL bCheckNodes=g_bCheckNodes;
		g_bCheckNodes=FALSE;

		PreActionList actionList;
		((CMainFrame *)AfxGetMainWnd( ))->GetClipboard( )->PasteNodes( GetRegion(), nodeArray, actionList, bPositionAtCrosshair);

		// Setup the undo for the new nodes it added.
		GetRegionDoc()->Modify(&actionList, TRUE);
		GetRegionDoc()->NotifySelectionChange();

		GetRegionDoc()->SynchronizePropertyLists( GetRegion(), false );

		// Reset this parameter
		g_bCheckNodes=bCheckNodes;
	}

	//we don't want the imported world's info string overwriting the global
	//info string
	//GetRegion()->m_pInfoString = region.m_pInfoString;

	return TRUE;
}

/************************************************************************/
// This imports a file into the region and optionally positions it at
// the green crosshair location.
//
// Returns: TRUE if the file import was a success.

// -------------------------------------------------------------
bool CRegionView::CreateBrushesFromScene(CEditRegion *pRegion, CObjInterface *pScene )
{
	ClassDef		*pClassDef;
	ClassDef		*pTerrainClassDef;
	CEditBrush		*pTemplateBrush;
	CBaseEditObj	*pTerrainObj;
	CEditBrush		*pBrush;
	CEditPoly		*pPoly;
	CEditVert		vert;
	CBoolProp		*pNoSnap;

	pClassDef			= GetProject()->FindClassDef("Brush");	
	pTerrainClassDef	= GetProject()->FindClassDef("Terrain"); 

	pTemplateBrush = no_CreateNewBrush(pRegion);
	SetupWorldNodeFromClass(pClassDef, NULL, pTemplateBrush, pRegion);

	// Create a Terrain object to hold the brushes.
	pTerrainObj = new CBaseEditObj;
	no_InitializeNewNode(pRegion, pTerrainObj, pRegion->GetRootNode());
	SetupWorldNodeFromClass(pTerrainClassDef, NULL, pTerrainObj, pRegion);

	OutputDebugString("\ncreating brushes");

	if( pScene->m_faceSetList.GetSize()>0 )
	{
		for(int i=0; i < pScene->m_faceSetList.GetSize(); i++)
		{
			bool foundUVs = false;
			float tvert[8];
			float vvert[12];

			pBrush = no_CreateNewBrush(pRegion);
			no_AttachNode(pRegion, pBrush, pTerrainObj);
			pPoly = new CEditPoly(pBrush);
			
			assert(pPoly);

			pBrush->m_Polies.Append(pPoly);
			
			// Copy the values from the template brush.
			pBrush->m_PropList.CopyValues(&pTemplateBrush->m_PropList);
		
			pBrush->SetClassName(pTemplateBrush->GetClassName());			

			int faceSize = pScene->m_faceSetList[i].GetSize();
		
			if( pScene->m_flag == CObjInterface::RAW )
			{
				if( 3 == faceSize  || 4 == faceSize || 12 < faceSize )			// v v v [v]
				{
					for( int vertNum = 0; vertNum < faceSize; vertNum++ )
					{
						int idx = pScene->m_faceSetList[i][faceSize-vertNum-1];
						vert.x = pScene->m_vertexList[idx-1][0]; 
						vert.y = pScene->m_vertexList[idx-1][1]; 
						vert.z = pScene->m_vertexList[idx-1][2]; 
						pBrush->m_Points.Append(vert);
						pPoly->m_Indices.Append(vertNum);
					}
				}
				else if( 6 == faceSize  || 8 == faceSize )		// v/t v/t v/t [v/t]
				{
					foundUVs = true;	// face has texture info, don't reset later
					int uu, vv;
					uu = vv = 0;

					for(int vertNum = 0, indexCount = 0; 
						vertNum < faceSize; 
						vertNum+=2, indexCount++ )
					{
						int idx = pScene->m_faceSetList[i][faceSize-vertNum-2];
						vert.x = pScene->m_vertexList[idx-1][0]; 
						vert.y = pScene->m_vertexList[idx-1][1]; 
						vert.z = pScene->m_vertexList[idx-1][2];
						idx = pScene->m_faceSetList[i][faceSize-vertNum-1];
						tvert[uu++] = pScene->m_texCoordList[idx-1][0];
						tvert[uu++] = pScene->m_texCoordList[idx-1][1];
						vvert[vv++] = vert.x;
						vvert[vv++] = vert.y;
						vvert[vv++] = vert.z;
						pBrush->m_Points.Append(vert);
						pPoly->m_Indices.Append(indexCount);
					}
				}
				else 
				{
					for(int vertNum = 0, indexCount = 0;		// v/t/n v/t/n v/t/n [v/t/n]
						vertNum < faceSize; 
						vertNum+=3, indexCount++ )
					{
						int idx = pScene->m_faceSetList[i][faceSize-vertNum-3];
						vert.x = pScene->m_vertexList[idx-1][0]; 
						vert.y = pScene->m_vertexList[idx-1][1]; 
						vert.z = pScene->m_vertexList[idx-1][2]; 
						pBrush->m_Points.Append(vert);
						pPoly->m_Indices.Append(indexCount);
					}
				}
			} 
			else
			{
				if( 3 == faceSize  || 4 == faceSize || 12 < faceSize )			// v v v [v]
				{
					for( int vertNum = 0; vertNum < faceSize; vertNum++ )
					{
						int idx = pScene->m_faceSetList[i][vertNum];
						vert.x = pScene->m_vertexList[idx-1][0]; 
						vert.y = pScene->m_vertexList[idx-1][1]; 
						vert.z = pScene->m_vertexList[idx-1][2]; 
						pBrush->m_Points.Append(vert);
						pPoly->m_Indices.Append(vertNum);
					}
				}
				else if( 6 == faceSize  || 8 == faceSize )		// v/t v/t v/t [v/t]
				{
					for(int vertNum = 0, indexCount = 0; 
						vertNum < faceSize; 
						vertNum+=2, indexCount++ )
					{
						int idx = pScene->m_faceSetList[i][vertNum];
						vert.x = pScene->m_vertexList[idx-1][0]; 
						vert.y = pScene->m_vertexList[idx-1][1]; 
						vert.z = pScene->m_vertexList[idx-1][2]; 
						pBrush->m_Points.Append(vert);
						pPoly->m_Indices.Append(indexCount);
					}
				}
				else 
				{
					for(int vertNum = 0, indexCount = 0;		// v/t/n v/t/n v/t/n [v/t/n]
						vertNum < faceSize; 
						vertNum+=3, indexCount++ )
					{
						int idx = pScene->m_faceSetList[i][vertNum];
						vert.x = pScene->m_vertexList[idx-1][0]; 
						vert.y = pScene->m_vertexList[idx-1][1]; 
						vert.z = pScene->m_vertexList[idx-1][2]; 
						pBrush->m_Points.Append(vert);
						pPoly->m_Indices.Append(indexCount);
					}
				}

			}

			// Set NoSnap to TRUE.
			if((pNoSnap = (CBoolProp*)pBrush->m_PropList.GetProp("NoSnap")) && 
				pNoSnap->m_Type == LT_PT_BOOL)
			{
				pNoSnap->m_Value = TRUE;
			}

			// Set the lighting to gouraud
			CStringProp *pStringProp = (CStringProp*)pBrush->m_PropList.GetProp("Lighting");
			if(pStringProp && pStringProp->m_Type == PT_STRING)
			{
				pStringProp->SetString("Gouraud");
			}

			if((pNoSnap = (CBoolProp*)pBrush->m_PropList.GetProp("Subdivide")) && 
				pNoSnap->m_Type == LT_PT_BOOL)
			{
				pNoSnap->m_Value = FALSE;
			}

			pPoly->UpdatePlane();
			
			// setup the texture space, using UVs if they are available
			if( foundUVs )
			{
				if( !pPoly->SetUVTextureSpace(0, tvert, 256, 256 ) )
					return false ;
			}
			else
			{
				for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
					pPoly->SetupBaseTextureSpace(nCurrTex);
			}
		}
	}
	no_DestroyNode(pRegion, pTemplateBrush, FALSE);

	return true;
}


/************************************************************************/
BOOL CRegionView::ImportOBJFile(CString sFilename, BOOL bPositionAtCrosshair)
{
	// open the obj file
	// load the ed file into the scene
	// save the scene as an ed file and name it sFilename+".ed"
	// ImportWorldFile(CString sFilename+".ed", BOOL bPositionAtCrosshair)
	BOOL retVal = FALSE;
	
	CObjInterface objInterface;
	
	FILE* pObjFile = fopen(LPCTSTR(sFilename), "rb");
	if( pObjFile )
	{
		CImportObjFileFormat impObjDlg;
		int isOK = impObjDlg.DoModal();

		OutputDebugString("\ncalling objInterface.ImportObj()");

		if( isOK == IDOK )
		{
			CEditRegion		region;

			// the loader chokes on unlabeled null nodes, so label the root
			region.GetRootNode()->SetNodeLabel( "Container" );

			BeginWaitCursor( ); // Put the wait cursor on the screen

			if( impObjDlg.m_from3DSMax )
				objInterface.ImportObj(pObjFile,impObjDlg.m_reverse_normals, CObjInterface::F3DSMAX );
			else if( impObjDlg.m_fromMaya )
				objInterface.ImportObj(pObjFile,impObjDlg.m_reverse_normals, CObjInterface::FMAYA );
			else if( impObjDlg.m_fromSoftImage )
				objInterface.ImportObj(pObjFile,impObjDlg.m_reverse_normals, CObjInterface::FSOFT );

			OutputDebugString("\ncalling CreateBrushesFromScene()");
			if (CreateBrushesFromScene(&region, &objInterface ))
			{
				CString ltaFilename = sFilename+".lta";
				CLTAFile OutFile(ltaFilename, false, CLTAUtil::IsFileCompressed(ltaFilename));

				if(OutFile.IsValid())
				{
					// printf("\n( p \"saving into ed file: %s\" ); ", filename);
					OutputDebugString("\ncalling region.SaveFile(file);");
					region.SaveLTA(&OutFile);
					OutFile.Close();
					OutputDebugString("\ncalling ImportWorldFile");
					ImportLTAFile(ltaFilename, bPositionAtCrosshair);
					unlink(LPCTSTR(ltaFilename));
					retVal = TRUE;
				}
			}
			else 
			{
				MessageBox("Corrupt .OBJ file!", "Error", MB_OK | MB_TASKMODAL);
			}

			EndWaitCursor( ); // End the wait cursor
		}
		fclose(pObjFile);
	}
	return retVal;
}

/************************************************************************/
void CRegionView::OnBindToObject() 
{
	BEGIN_MEMORYEXCEPTION()
	{	
		// See what type of object, if any, is selected in the code view.
		CBaseEditObj	*pObj;
		CClassDlg		dlg;
		
		// Get the document and region
		CRegionDoc *pDoc=GetRegionDoc();
		CEditRegion *pRegion=pDoc->GetRegion();

		CString sTitle;
		sTitle.LoadString(IDS_CHOOSE_OBJECT_TYPE);

		// Enable individual binding if more than one node is selected
		if (pRegion->GetNumSelections() > 1)
		{
			dlg.SetEnableBindIndividually(TRUE);
		}

		dlg.SetProject(GetProject());
		dlg.SetTitle(sTitle);
		
		if( dlg.DoModal() != IDOK )
		{
			return;
		}

		// Check if no class was selected
		if (dlg.GetSelectedClass().IsEmpty())  return;

		PreActionList actionList;

		// Determine if we should bind individual objects to each brush
		if (dlg.GetBindIndividually() && pRegion->GetNumSelections() > 1)
		{
			// An array of nodes to select
			CArray<CWorldNode *, CWorldNode *>	nodesToSelect;

			int i;
			for (i=0; i < pRegion->GetNumSelections(); i++)
			{
				// The current selection
				CWorldNode *pSelectedNode=pRegion->GetSelection(i);

				// Skip null nodes
				if (pSelectedNode->GetType() == Node_Null)
				{
					continue;
				}

				// Add the object
				CWorldNode *pObject=pDoc->AddObject(dlg.GetSelectedClass(), pSelectedNode->GetUpperLeftCornerPos());

				// Bind the object to its parent
				pDoc->BindNode(pObject, pSelectedNode->FindParentContainer(), &actionList);

				// Bind the selected node to the object
				pDoc->BindNode(pSelectedNode, pObject, &actionList);

				// Add the node to the selection array
				nodesToSelect.Add(pObject);
			}

			// Select the nodes
			for (i=0; i < nodesToSelect.GetSize(); i++)
			{
				pDoc->SelectNode(nodesToSelect[i], FALSE);
			}
		}
		else
		{
			// Add the object
			CWorldNode *pObject=pDoc->AddObject(dlg.GetSelectedClass(), GetRegionDoc()->GetRegion()->GetMarker());

			// Set the initial parent node
			CWorldNode *pParentNode=pRegion->GetRootNode();

			// Get the first node that is selected.  This is used to determine the container node
			// to place the binded nodes under.  If the first node is a container node, then use that
			// one.
			if (pRegion->GetNumSelections() > 0)
			{
				// Get the first selected node
				CWorldNode *pSelection=pRegion->GetSelection(0);

				// If it is a NULL node then use this one
				if (pSelection->GetType() == Node_Null)
				{
					pParentNode=pSelection;
				}
				else
				{
					// Search for the parent container node
					CWorldNode *pParentContainer=pSelection->FindParentContainer();
					if (pParentContainer)
					{
						pParentNode=pParentContainer;
					}
				}
			}

			// Bind the object to its parent
			pDoc->BindNode(pObject, pParentNode, &actionList);

			// Bind the selected nodes to the object
			pDoc->BindNodeToSelected(pObject, &actionList);

			// Select the object
			pDoc->SelectNode(pObject, FALSE);
		}	
		// For some reason, doing this modify makes it happen twice..
		//pDoc->Modify(&actionList, FALSE);
	}
	END_MEMORYEXCEPTION()
}

void CRegionView::OnUpdateBindToObject(CCmdUI* pCmdUI)
{
	if(GetRegion()->m_Selections.GetSize() > 0)
	{
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
	SetMenuHotKeyText(UIE_BIND_TO_OBJECT, pCmdUI);
}



void CRegionView::OnUpdateObjectProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetRegion()->GetFirstSelectedObject() != NULL );
}


void CRegionView::OnWorldInfo() 
{
	CWorldInfoDlg dlg;

	if(m_pRegion)
	{
		dlg.m_pRegion = m_pRegion;
		dlg.m_InputWorldName = GetDocument()->GetTitle();

		if(dlg.DoModal() == IDOK)
		{
			m_pRegion->m_pInfoString = m_pRegion->m_pStringHolder->AddString(dlg.m_WorldInfoString);
			GetRegionDoc()->Modify();
		}
	}
}

void CRegionView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	//don't bring up the context menu if this view does not have focus currently
	if(GetFocus() != pWnd)
		return;

	char *deviceNames[50];
	int nDeviceNames, i;
	UINT itemID;

	if( m_bDoMenu )
	{
		m_rayMousePoint = ViewDef()->MakeRayFromScreenPoint( GetCurMousePos( ));

		CMenu menu;
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_REGION_VIEW));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);

		CWnd* pWndPopupOwner = this;
		while (pWndPopupOwner->GetStyle() & WS_CHILD)
			pWndPopupOwner = pWndPopupOwner->GetParent();

		
		// Fill in the Direct3D options.

		//First get the D3D popup menu
		CMenu *pDirect3dOptions = pPopup->GetSubMenu(9);
		ASSERT( pDirect3dOptions );		// someone changed the menu layout, fix the index

		pDirect3dOptions = pDirect3dOptions->GetSubMenu(7);
		ASSERT( pDirect3dOptions );		// someone changed the menu layout, fix the index

		if(pDirect3dOptions)
		{
			CheckAMenuItem(pDirect3dOptions, ID_USEDEVICE1, m_DeviceNum, 0);
			CheckAMenuItem(pDirect3dOptions, ID_USEDEVICE2, m_DeviceNum, 1);
			CheckAMenuItem(pDirect3dOptions, ID_USEDEVICE3, m_DeviceNum, 2);
			CheckAMenuItem(pDirect3dOptions, ID_USEDEVICE4, m_DeviceNum, 3);
			
			CheckAMenuItem(pDirect3dOptions, ID_RENDERMODE1, m_RenderMode, 0);
			CheckAMenuItem(pDirect3dOptions, ID_RENDERMODE2, m_RenderMode, 1);
			CheckAMenuItem(pDirect3dOptions, ID_RENDERMODE3, m_RenderMode, 2);
			CheckAMenuItem(pDirect3dOptions, ID_RENDERMODE4, m_RenderMode, 3);

			// Set device names.
			nDeviceNames = dm_GetDirectDrawDeviceNames(deviceNames, 4);

			for(i=0; i < nDeviceNames; i++)
			{
				itemID = pDirect3dOptions->GetMenuItemID(i);
				pDirect3dOptions->ModifyMenu(i, MF_BYPOSITION | MF_STRING, itemID, deviceNames[i]);
				free(deviceNames[i]);
			}

			// Disable ones they can't choose.
			for( ; i < 4; i++)
				pDirect3dOptions->RemoveMenu(nDeviceNames, MF_BYPOSITION);
		}		


		// Enable stuff.
		CheckAMenuItem( pPopup, ID_POPUP_SHADEMODE_WIREFRAMEPOLIES, GetShadeMode(), SM_WIREFRAME );
		CheckAMenuItem( pPopup, ID_POPUP_SHADEMODE_FLATPOLIES, GetShadeMode(), SM_FLAT );
		CheckAMenuItem( pPopup, ID_POPUP_SHADEMODE_TEXTUREDPOLIES, GetShadeMode(), SM_TEXTURED );
		CheckAMenuItem( pPopup, ID_POPUP_SHADEMODE_LIGHTMAPSONLY, GetShadeMode(), SM_LIGHTMAPSONLY);

		CheckAMenuItem( pPopup, ID_POPUP_VIEWMODE_PERSPECTIVEVIEW, GetViewMode(), VM_PERSPECTIVE );
		CheckAMenuItem( pPopup, ID_POPUP_VIEWMODE_TOPVIEW, GetViewMode(), VM_TOP );
		CheckAMenuItem( pPopup, ID_POPUP_VIEWMODE_BOTTOMVIEW, GetViewMode(), VM_BOTTOM );
		CheckAMenuItem( pPopup, ID_POPUP_VIEWMODE_FRONTVIEW, GetViewMode(), VM_FRONT );
		CheckAMenuItem( pPopup, ID_POPUP_VIEWMODE_BACKVIEW, GetViewMode(), VM_BACK );
		CheckAMenuItem( pPopup, ID_POPUP_VIEWMODE_LEFTVIEW, GetViewMode(), VM_LEFT );
		CheckAMenuItem( pPopup, ID_POPUP_VIEWMODE_RIGHTVIEW, GetViewMode(), VM_RIGHT );

		CheckAMenuItem( pPopup, ID_POPUP_OPTIONS_BACKFACINGPOLIES, IsSelectBackfaces(), TRUE );
		CheckAMenuItem( pPopup, ID_POPUP_OPTIONS_SHOWWIREFRAME, IsShowWireframe(), TRUE );
		CheckAMenuItem( pPopup, ID_POPUP_OPTIONS_SHOWNORMALS, IsShowNormals(), TRUE );
		CheckAMenuItem( pPopup, ID_POPUP_OPTIONS_SHOWOBJECTS, IsShowObjects(), TRUE );
		CheckAMenuItem( pPopup, ID_POPUP_OPTIONS_SHOWEDITGRID, IsShowGrid(), TRUE );
 		CheckAMenuItem( pPopup, ID_POPUP_OPTIONS_SHOWMARKER, IsShowMarker(), TRUE );

		
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
			pWndPopupOwner);
	}
}


BOOL CRegionView::PreTranslateMessage(MSG* pMsg)
{
	// CG: This block was added by the Pop-up Menu component
	{
		// Shift+F10: show pop-up menu.
		if ((((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) && // If we hit a key and
			(pMsg->wParam == VK_F10) && (GetKeyState(VK_SHIFT) & ~1)) != 0) ||	// it's Shift+F10 OR
			(pMsg->message == WM_CONTEXTMENU))									// Natural keyboard key
		{
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);

			CPoint point = rect.TopLeft();
			point.Offset(5, 5);
			OnContextMenu(NULL, point);

			return TRUE;
		}
	}

	return CView::PreTranslateMessage(pMsg);
}




//////////////////////////////////////////////////////
// MENU HANDLERS

void CRegionView::OnShadeWireframe() 
{
	SetShadeMode( SM_WIREFRAME );
	DrawRect();
}

void CRegionView::OnUpdateShadeWireframe(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	
	if(GetViewMode() != VM_PERSPECTIVE)
		pCmdUI->SetCheck();
	SetMenuHotKeyText(UIE_SHADE_WIREFRAME, pCmdUI);
}

void CRegionView::OnShadeTextured() 
{
	SetShadeMode( SM_TEXTURED );
	DrawRect();
}

void CRegionView::OnUpdateShadeTextured(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetViewMode() == VM_PERSPECTIVE) ? TRUE : FALSE);

	if(GetViewMode() != VM_PERSPECTIVE)
		pCmdUI->SetCheck(FALSE);
	SetMenuHotKeyText(UIE_SHADE_TEXTURES, pCmdUI);
}

void CRegionView::OnShadeFlat() 
{
	SetShadeMode( SM_FLAT );
	DrawRect();
}

void CRegionView::OnUpdateShadeFlat(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetViewMode() == VM_PERSPECTIVE) ? TRUE : FALSE);

	if(GetViewMode() != VM_PERSPECTIVE)
		pCmdUI->SetCheck(FALSE);
	SetMenuHotKeyText(UIE_SHADE_FLAT, pCmdUI);
}


void CRegionView::OnShadeLightmapsOnly() 
{
	SetShadeMode( SM_LIGHTMAPSONLY );
	DrawRect();
}


void CRegionView::OnUpdateShadeLightmapsOnly(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetViewMode() == VM_PERSPECTIVE) ? TRUE : FALSE);

	if(GetViewMode() != VM_PERSPECTIVE)
		pCmdUI->SetCheck(FALSE);
	SetMenuHotKeyText(UIE_SHADE_LIGHTMAPS, pCmdUI);
}

void CRegionView::OnBackView() 
{
	SetViewMode( VM_BACK );
	DrawRect();
}

void CRegionView::OnUpdateBackView(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_BACK_VIEW, pCmdUI);
}


void CRegionView::OnBottomView() 
{
	SetViewMode( VM_BOTTOM );
	DrawRect();
}

void CRegionView::OnUpdateBottomView(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_BOTTOM_VIEW, pCmdUI);
}


void CRegionView::OnFrontView() 
{
	SetViewMode( VM_FRONT );
	DrawRect();
}

void CRegionView::OnUpdateFrontView(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_FRONT_VIEW, pCmdUI);
}


void CRegionView::OnLeftView() 
{
	SetViewMode( VM_LEFT );
	DrawRect();
}

void CRegionView::OnUpdateLeftView(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_LEFT_VIEW, pCmdUI);
}


void CRegionView::OnPerspectiveView() 
{
	SetViewMode( VM_PERSPECTIVE );
	DrawRect();
}

void CRegionView::OnUpdatePerspectiveView(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_PERSPECTIVE_VIEW, pCmdUI);
}


void CRegionView::OnRightView() 
{
	SetViewMode( VM_RIGHT );
	DrawRect();
}

void CRegionView::OnUpdateRightView(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_RIGHT_VIEW, pCmdUI);
}


void CRegionView::OnTopView() 
{
	SetViewMode( VM_TOP );
	DrawRect();
}

void CRegionView::OnUpdateTopView(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_TOP_VIEW, pCmdUI);
}


void CRegionView::OnShowObjects()
{
	SetShowObjects(!IsShowObjects());
	DrawRect();
}


void CRegionView::OnBackfacingPolies() 
{
	SetSelectBackfaces( !IsSelectBackfaces() );
	
	UpdateImmediateSelection();
	DrawRect();
}


void CRegionView::OnShowWireframe() 
{
	SetShowWireframe(!IsShowWireframe());
	DrawRect();
}


void CRegionView::OnShowNormals() 
{
	SetShowNormals( !IsShowNormals() );
	DrawRect();
}


void CRegionView::OnRemoveExtraEdges() 
{
	BEGIN_MEMORYEXCEPTION()
	{
		CReal normalThresh = 0.05f;
		CReal distThresh = 0.0001f;
		DWORD i;
		LPOS pos;

		// MIKE 12/29/97 - Switched from CountUndoTextures to just TRUE.
		GetRegionDoc()->Modify();

		for(pos=m_pRegion->m_Brushes; pos; )
		{
			CEditBrush* pBrush = m_pRegion->m_Brushes.GetNext(pos);
			pBrush->RemoveExtraEdges( normalThresh, distThresh );
			m_pRegion->UpdateBrushGeometry(pBrush);
		}

		DrawRect();
		SetFocus();
	}
	END_MEMORYEXCEPTION()
}


void CRegionView::OnResetAllTextureCoords() 
{
}


// Returns a list of polies from either the immediate poly or from the selected brushes.
void CRegionView::GetSelectedPolies(CMoArray<CEditPoly*> &polies)
{
	CEditRegion *pRegion;
	CEditBrush *pBrush;
	DWORD i, j;


	pRegion = GetRegion();

	polies.SetCacheSize(40);

	if(GetEditMode() == GEOMETRY_EDITMODE)
	{	
		if(TaggedPolies() == 1)
		{
			// Just add the immediate poly.
			if(IPoly().IsValid())
				polies.Append(IPoly()());
		}
		else
		{
			// Add all the tagged polies (exclude the immediate poly).
			for(i=1; i < TaggedPolies(); i++)
			{
				CPolyRef &ref = TaggedPolies()[i];

				if(polies.FindElement(ref()) == BAD_INDEX)
				{
					polies.Append(ref());
				}
			}
		}
	}
	else
	{
		for(i=0; i < pRegion->m_Selections; i++)
		{
			if(pRegion->m_Selections[i]->GetType() == Node_Brush)
			{
				pBrush = (CEditBrush*)pRegion->m_Selections[i];

				for(j=0; j < pBrush->m_Polies; j++)
				{
					polies.Append(pBrush->m_Polies[j]);
				}
			}
		}
	}
}


void CRegionView::OnMapTextureCoords() 
{
	CMapTextureCoordsDlg dlg(this);
	CMoArray<CEditPoly*> polies;
	CEditPoly *pPoly;
	CTexture *pTexture;
	DVector normP, normQ;


	polies.SetCacheSize(200);
	GetSelectedPolies(polies);
	if(polies.GetSize() > 0)
	{
		pPoly = NULL;

		//find the first textured polygon
		for(uint32 nCurrPoly = 0; nCurrPoly < polies.GetSize(); nCurrPoly++)
		{
			if(polies[nCurrPoly]->GetTexture(GetCurrTexture()).m_pTextureFile)
			{
				pPoly = polies[nCurrPoly];
				break;
			}
		}

		if(pPoly)
		{
			CTexturedPlane& PolyTex = pPoly->GetTexture(GetCurrTexture());

			pTexture = dib_GetDibTexture(PolyTex.m_pTextureFile);
			if(pTexture && pTexture->m_pDib)
			{
				// Setup parameters for the dialog.
				dlg.m_TextureWidth = pTexture->m_pDib->GetWidth() >> pTexture->m_UIMipmapOffset;
				dlg.m_TextureHeight = pTexture->m_pDib->GetHeight() >> pTexture->m_UIMipmapOffset;
				
				//get the normal versions of the vectors
				normP = PolyTex.GetP();
				normP.Norm();
				normQ = PolyTex.GetQ();
				normQ.Norm();

				//find out the scale for this polygon
				dlg.m_UScale = (1.0f / PolyTex.GetP().Mag()) * (float)dlg.m_TextureWidth;
				dlg.m_VScale = (1.0f / PolyTex.GetQ().Mag()) * (float)dlg.m_TextureHeight;

				//find the U offset ranging from [0...TexWidth)
				dlg.m_UOffset = normP.Dot(PolyTex.GetO() - pPoly->Pt(0)) * PolyTex.GetP().Mag();
				dlg.m_UOffset = (float)fmod(dlg.m_UOffset, (float)dlg.m_TextureWidth);
				if(dlg.m_UOffset < 0.0f)
					dlg.m_UOffset = (float)dlg.m_TextureWidth + dlg.m_UOffset;
				if (dlg.m_UOffset < 0.001)
					dlg.m_UOffset = 0.0f;

				//find the V offset ranging from [0...TexHeight)
				dlg.m_VOffset = normQ.Dot(PolyTex.GetO() - pPoly->Pt(0)) * PolyTex.GetQ().Mag();
				dlg.m_VOffset = (float)fmod(dlg.m_VOffset, (float)dlg.m_TextureHeight);
				if(dlg.m_VOffset < 0.0f)
					dlg.m_VOffset = (float)dlg.m_TextureHeight + dlg.m_VOffset;
				if (dlg.m_VOffset < 0.001)
					dlg.m_VOffset = 0.0f;

				// User enters rotation in degrees, but saved in radians...
				dlg.m_Rotation = dlg.m_RefRotation = 0.0f;

				//we need to load up options from the registry
				dlg.m_bAutoApply = ::GetApp()->GetOptions().GetBoolValue("MapTextureAutoApply", TRUE);
				dlg.m_nWindowX	 = ::GetApp()->GetOptions().GetDWordValue("MapTextureWindowX", 100);
				dlg.m_nWindowY	 = ::GetApp()->GetOptions().GetDWordValue("MapTextureWindowY", 100);

				if( dlg.DoModal() == IDOK )
				{
				}

				//save the settngs
				::GetApp()->GetOptions().SetBoolValue("MapTextureAutoApply", dlg.m_bAutoApply);
				::GetApp()->GetOptions().SetDWordValue("MapTextureWindowX", dlg.m_nWindowX);
				::GetApp()->GetOptions().SetDWordValue("MapTextureWindowY", dlg.m_nWindowY);
			}
		}
	}
}

void CRegionView::OnUpdateMapTextureCoords(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(AreAnyPoliesSelected());
	SetMenuHotKeyText(UIE_MAP_TEXTURE_COORDS, pCmdUI);
}


void CRegionView::OnRotateSelection() 
{
	CRotateSelection dlg;
	CReal rRotation;
	CEditRegion *pRegion;
	CMatrix matRot;
	CVector vRotationVector, vRotationCenter;
	DWORD i;
	PreActionList actionList;
	CLinkedList<CEditBrush*> brushList;

	pRegion = GetRegion( );
	ASSERT( pRegion );

	if( !pRegion->m_Selections.GetSize( ))
		return;

	// Get input...
	if( dlg.DoModal( ) == IDOK )
	{
		BeginWaitCursor( );

		rRotation = dlg.m_rRotation;

		// Make sure input is -2PI <= x <= 2PI...
		if( rRotation >= 0 )
			rRotation = (float)fmod( rRotation * MATH_PI / 180.0f, 2.0f * MATH_PI );
		else
			rRotation = -(float)fmod( -rRotation * MATH_PI / 180.0f, 2.0f * MATH_PI );

		// Get the mouse point at rmb click...
		vRotationVector = m_pViewDef->m_Nav.Forward( );
		vRotationVector.Norm( );
		vRotationCenter = GetRegion()->m_vMarker;

		//now setup the rotation matrix. Note the use of the negative angle, this is
		//to cause the rotation to be clockwise which the artists requested.
		gr_SetupRotationAroundVector(&matRot, vRotationVector, -rRotation);

		// Setup undos...
		for(i=0; i < pRegion->m_Selections; i++)
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pRegion->m_Selections[i]));
		
		GetRegionDoc()->Modify(&actionList, TRUE);

		// Loop over all selected nodes...
		for( i = 0; i < pRegion->m_Selections.GetSize( ); i++ )
		{
			pRegion->m_Selections[i]->Rotate( matRot, vRotationCenter );
			
			if(pRegion->m_Selections[i]->GetType() == Node_Brush)
			{
				brushList.AddTail(pRegion->m_Selections[i]->AsBrush());
			}
		}

		// Update the geometry stuff...
		pRegion->UpdatePlanes(&brushList);
		pRegion->CleanupGeometry(&brushList);
		GetRegionDoc()->UpdateSelectionBox();

		EndWaitCursor( );
	}
}

void CRegionView::OnUpdateRotateSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetRegion()->m_Selections.GetSize() > 0) ? TRUE : FALSE);
	SetMenuHotKeyText(UIE_ROTATE_SELECTION, pCmdUI);
}


void CRegionView::OnJoinTaggedVertices() 
{
	DWORD		nVert;
	CEditBrush	*pBrush;
	GenList<CVertRef> &vertList = TaggedVerts();

	if( vertList.GenGetSize() == 0 )
		return;

	// Make sure all the tagged vertices are on the same brush
	GenListPos pFinger = vertList.GenBegin();

	nVert = vertList.GenGetAt(pFinger).m_iVert;
	pBrush = vertList.GenGetNext(pFinger).m_pBrush;
	while (vertList.GenIsValid(pFinger))
	{
		if (vertList.GenGetNext(pFinger).m_pBrush != pBrush)
			return;
	}

	// Go back and change the vertices to the first in the list
	pFinger = vertList.GenBegin();
	vertList.GenGetNext(pFinger);
	while (vertList.GenIsValid(pFinger))
	{
		DWORD nJoinVert = vertList.GenGetNext(pFinger).m_iVert;
		pBrush->ChangeVertexReferences( nJoinVert, nVert );
	}

	// Remove the now un-used vertices in the brush
	pBrush->RemoveUnusedVerts();
	
	// Clear out the vertex list
	vertList.GenRemoveAll();

	GetDocument()->UpdateAllViews( this );
}


void CRegionView::OnObjectHeight()
{
	CMoArray<CBaseEditObj*>	objects;
	CBaseEditObj			*pObj;
	CVectorProp				*pProp;

	DWORD					i;
	CEditRegion				*pRegion;
	CWorldNode				*pNode;
	CStringDlg				dlg;
	float					fHeight;
	
	CEditRay				ray;
	CPolyRef				ref;
	CReal					dist;
	PreActionList actionList;



	// First build a list of selected objects.
	pRegion = GetRegion();
	for(i=0; i < pRegion->m_Selections; i++)
	{
		pNode = pRegion->m_Selections[i];

		if(pNode->GetType() == Node_Object)
		{
			objects.Append(pNode->AsObject());
		}
	}

	// If there's more than one, prompt for the height above the ground.
	if(objects > 0)
	{
		dlg.m_bAllowNumbers = TRUE;
		
		if(dlg.DoModal(IDS_OBJECT_HEIGHT, IDS_ENTER_OBJECT_HEIGHT) == IDOK)
		{
			fHeight = (float)atof((LPCTSTR)dlg.m_EnteredText);
			
			// Setup an undo.
			for(i=0; i < objects; i++)
			{
				actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, objects[i]));
			}

			GetRegionDoc()->Modify(&actionList, TRUE);

			for(i=0; i < objects; i++)
			{
				pObj = objects[i];

				if(pProp = (CVectorProp*)pObj->m_PropList.GetProp("m_Pos"))
				{
					ray.m_Pos = pProp->m_Vector;
					ray.m_Dir.Init(0.0f, -1.0f, 0.0f);

					ref = CastRayAtPolies(ray, &dist);
					if(ref.IsValid())
					{
						pProp->m_Vector.y = (ray.m_Pos.y + ray.m_Dir.y*dist) + fHeight;
					}
				}
			}

			GetRegionDoc()->RedrawAllViews();
		}
	}
}


void CRegionView::OnProcessWorld() 
{
	GetRegionDoc()->StartPreProcessor();	
}

void CRegionView::OnFileSave() 
{
	GetRegionDoc()->OnSaveDocument( NULL );
}


void CRegionView::OnUpdateFileSaveAs( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( FALSE );
}

void CRegionView::OnCleanupGeometry() 
{
	// MIKE 12/29/97 - removed this call.. full undos puff and CleanupGeometry is a hack anyway.
	// GetRegionDoc()->Modify((( CMainFrame * )AfxGetMainWnd( ))->CountUndoGeometry( ));
	
	m_pRegion->CleanupGeometry();
}


void CRegionView::OnShowEditGrid() 
{
	SetShowGrid(!IsShowGrid());
	DrawRect( );
}

void CRegionView::OnShowMarker() 
{
	SetShowMarker(!IsShowMarker());
	DrawRect( );
}
 
void CRegionView::On4ViewConfiguration() 
{
	CRegionFrame		*pFrame = (CRegionFrame*)GetParent()->GetParent();
	CSplitterWnd		*pSplitter = pFrame->GetSplitter();
	CRegionView			*pRegionView;
	CRect				rect;

	
	pFrame->GetClientRect( &rect );

	pSplitter->SetRowInfo( 0, rect.Height()/2, 10 );
	pSplitter->SetColumnInfo( 0, rect.Width()/2, 10 );
	pSplitter->RecalcLayout();

	// Perspective view.
	pRegionView = (CRegionView*)pFrame->GetSplitter()->GetPane(0,0);
	pRegionView->SetViewMode( VM_PERSPECTIVE );

	// Top view.
	pRegionView = (CRegionView*)pFrame->GetSplitter()->GetPane(0,1);
	pRegionView->SetViewMode( VM_TOP );

	// Front view.
	pRegionView = (CRegionView*)pFrame->GetSplitter()->GetPane(1,1);
	pRegionView->SetViewMode( VM_FRONT );

	// Left view.
	pRegionView = (CRegionView*)pFrame->GetSplitter()->GetPane(1,0);
	pRegionView->SetViewMode( VM_LEFT );


	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdate4ViewConfiguration(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_RESET_VIEWPORTS, pCmdUI);
}

void CRegionView::OnUpdateSingleBrushOperation( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( (GetRegion()->NumBrushSelections() == 1) );
}

void CRegionView::OnUpdateShrinkGridSpacing( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( GetGridSpacing() > 1 );
	SetMenuHotKeyText(UIE_SHRINK_GRID_SPACING, pCmdUI);
}

void CRegionView::OnUpdateExpandGridSpacing( CCmdUI *pCmdUI )
{
	// NOTE: If the grid needs to be spaced larger, it needs to be saved using
	// a DWORD in the .lyt file.
	pCmdUI->Enable( GetGridSpacing() < (1<<15) );
	SetMenuHotKeyText(UIE_EXPAND_GRID_SPACING, pCmdUI);
}

void CRegionView::OnShrinkGridSpacing()
{
	if( GetGridSpacing() <= 1 )
		return;
	
	SetGridSpacing( GetGridSpacing() >> 1 );
	DrawRect();
	UpdateStatusBar( );
}

void CRegionView::OnExpandGridSpacing()
{
	SetGridSpacing( GetGridSpacing() << 1 );
	DrawRect();
	UpdateStatusBar( );
}


void CRegionView::OnUpdateApplyColor( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( (GetRegion()->m_Selections > 0) && (GetEditMode() == BRUSH_EDITMODE) );
	SetMenuHotKeyText(UIE_APPLY_COLOR, pCmdUI);
}


void CRegionView::OnApplyColor()
{
	COLORREF color;
	DWORD i;
	CEditBrush *pBrush;
	PreActionList actionList;
	CWorldNode *pNode;

	
	// Build an undo list.
	for(i=0; i < GetRegion()->m_Selections; i++)
	{
		pNode = GetRegion()->m_Selections[i];
		if(pNode->GetType() == Node_Brush)
		{
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pNode));
		}
	}

	GetRegionDoc()->Modify(&actionList, TRUE);

	// Do the mods.
	color = GetColorSelectDlg()->GetCurColor();
	for( i=0; i < GetRegion()->m_Selections; i++ )
	{
		if(GetRegion()->m_Selections[i]->GetType() == Node_Brush)
		{
			pBrush = GetRegion()->m_Selections[i]->AsBrush();
			
			pBrush->r = GetRValue( color );
			pBrush->g = GetGValue( color );
			pBrush->b = GetBValue( color );
		}
	}

	GetRegionDoc()->RedrawAllViews();
}


void CRegionView::OnUpdateApplyTexture( CCmdUI *pCmdUI )
{
	SetMenuHotKeyText(UIE_APPLY_TEXTURE_TO_SEL, pCmdUI);
}

void CRegionView::OnApplyTexture()
{
	DWORD i, k;
	char *pCurTexName;
	CEditBrush *pBrush;
	CWorldNode *pNode;
	
	PreActionList actionList;
	CPreAction *pAction;
	GPOS pos;
	
	if( (GetEditMode() == BRUSH_EDITMODE) ||
		(GetEditMode() == OBJECT_EDITMODE))
	{
		// Put the current texture on the selected brushes.
		if( GetCurrentTextureName(pCurTexName, TRUE) )
		{
			// Build the list of the brushes we're going to modify..
			for(i=0; i < GetRegion()->m_Selections; i++)
			{
				pNode = GetRegion()->m_Selections[i];
				if(pNode->GetType() == Node_Brush)
				{
					actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pNode));
				}
			}
			
			// Store the changes..
			GetRegionDoc()->Modify(&actionList, FALSE);
			
			// Make the changes.
			for(pos=actionList; pos; )
			{
				CWorldNode* pNode = actionList.GetNext(pos)->m_pNode;

				if(pNode->GetType() == Node_Brush)
				{
					pBrush = (CEditBrush*)pNode;
			
					for( k=0; k < pBrush->m_Polies; k++ )
					{
						pBrush->m_Polies[k]->GetTexture(GetCurrTexture()).m_pTextureName = pCurTexName;
						pBrush->m_Polies[k]->GetTexture(GetCurrTexture()).UpdateTextureID();
					}
				}
			}
		}
	}
	else if(GetEditMode() == GEOMETRY_EDITMODE)
	{
		// Put the selected texture onto the current poly.
		if( IPoly().IsValid() && GetCurrentTextureName(pCurTexName, TRUE) )
		{
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, IPoly().m_pBrush));
			GetRegionDoc()->Modify(&actionList, TRUE);
			
			IPoly()()->GetTexture(GetCurrTexture()).m_pTextureName = pCurTexName;
			IPoly()()->GetTexture(GetCurrTexture()).UpdateTextureID();
		}
	}

	GDeleteAndRemoveElements(actionList);
	GetRegionDoc()->RedrawAllViews();
}


void CRegionView::OnUpdateRemoveTexture( CCmdUI *pCmdUI )
{
	SetMenuHotKeyText(UIE_REMOVE_TEXTURE, pCmdUI);
}

void CRegionView::OnRemoveTexture()
{
	DWORD i, k;
	char *pCurTexName;
	CWorldNode *pNode;
	
	PreActionList actionList;
	CPreAction *pAction;
	GPOS pos;
	
	if( (GetEditMode() == BRUSH_EDITMODE) ||
		(GetEditMode() == OBJECT_EDITMODE))
	{
		// Remove the current texture from the selected brushes.

		// Build the list of the brushes we're going to modify..
		for(i=0; i < GetRegion()->m_Selections; i++)
		{
			pNode = GetRegion()->m_Selections[i];
			if(pNode->GetType() == Node_Brush)
			{
				//we have a brush, determine if the active texture layer has a texture assigned to it
				CEditBrush* pBrush = pNode->AsBrush();
				for(uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies.GetSize(); nCurrPoly++)
				{
					//see if this has a texture
					CTexturedPlane* pTex = &pBrush->m_Polies[nCurrPoly]->GetTexture(GetCurrTexture());

					if(pTex->m_pTextureName && (stricmp(pTex->m_pTextureName, "Default") != 0))
					{
						//this is a brush we want to add
						actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pNode));
						break;
					}
				}
			}
		}
		
		// Store the changes..
		GetRegionDoc()->Modify(&actionList, FALSE);
		
		// Make the changes.
		for(pos=actionList; pos; )
		{
			CWorldNode* pNode = actionList.GetNext(pos)->m_pNode;

			if(pNode->GetType() == Node_Brush)
			{
				CEditBrush* pBrush = (CEditBrush*)pNode;
		
				for( k=0; k < pBrush->m_Polies; k++ )
				{
					pBrush->m_Polies[k]->GetTexture(GetCurrTexture()).m_pTextureName = "Default";
					pBrush->m_Polies[k]->GetTexture(GetCurrTexture()).UpdateTextureID();
				}
			}
		}
	}
	else if(GetEditMode() == GEOMETRY_EDITMODE)
	{
		// Put the selected texture onto the current poly.
		if( IPoly().IsValid() )
		{
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, IPoly().m_pBrush));
			GetRegionDoc()->Modify(&actionList, TRUE);
			
			IPoly()()->GetTexture(GetCurrTexture()).m_pTextureName = "Default";
			IPoly()()->GetTexture(GetCurrTexture()).UpdateTextureID();
		}
	}

	GDeleteAndRemoveElements(actionList);
	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnObjectProperties() 
{
	GetRegionDoc()->SetupPropertiesDlg(TRUE);
}


void CRegionView::OnUpdateStatus( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( TRUE );
}

//------------------------------------------------------------------------------------
//
//  CRegionView::UpdateStatusBar()
//
//  Purpose:	Updates the text in the status bar that pertain to this view.
//
//------------------------------------------------------------------------------------
void CRegionView::UpdateStatusBar()
{
	CMainFrame *pFrame;
	CRect rRect;
	CPoint ptMousePos;
			
	if( !m_bInFocus )
		return;

	GetCursorPos( &ptMousePos );
	GetOwner()->GetWindowRect( &rRect );

	pFrame = ( CMainFrame * )AfxGetMainWnd();
	ASSERT( pFrame );
	if( pFrame )
	{
		CString sViewMode, sGrid, sCamera, sCursor, sTemp;
		int nViewMode;
		CVector vCamOrient, vCamPos, vCursorPos;
		CEditRay rayMousePoint;

		sGrid.LoadString( IDS_GRID );
		sTemp.Format( "%d", GetGridSpacing( ));
		sGrid += sTemp;
//		pStatusBar->SetPaneText( pStatusBar->CommandToIndex( ID_INDICATOR_GRID ), sGrid );
		pFrame->UpdateStatusText(ID_INDICATOR_GRID, sGrid.GetBuffer(0));
		
		// Only update the status bar when this view has focus...
		if( rRect.PtInRect( ptMousePos ))
		{
			rayMousePoint = ViewDef()->MakeRayFromScreenPoint( GetCurMousePos( ));
			vCamOrient = Nav( ).Forward( );
			vCamPos = Nav( ).Pos( );
			nViewMode = GetViewMode( );

			switch( nViewMode )
			{
				case VM_TOP:
					sViewMode.LoadString( IDS_VIEW_TOP );
					sCamera.Format( "@(%1.0f,n/a,%1.0f)", vCamPos.x, vCamPos.z );
					vCursorPos.Init( rayMousePoint.m_Pos.x, 0.0, rayMousePoint.m_Pos.z );
					sCursor.Format( "@(%1.0f,n/a,%1.0f)", vCursorPos.x, vCursorPos.z );
					break;
				case VM_BOTTOM:
					sViewMode.LoadString( IDS_VIEW_BOTTOM );
					sCamera.Format( "@(%1.0f,n/a,%1.0f)", vCamPos.x, vCamPos.z );
					vCursorPos.Init( rayMousePoint.m_Pos.x, 0.0, rayMousePoint.m_Pos.z );
					sCursor.Format( "@(%1.0f,n/a,%1.0f)", vCursorPos.x, vCursorPos.z );
					break;
				case VM_LEFT:
					sViewMode.LoadString( IDS_VIEW_LEFT );
					sCamera.Format( "@(n/a,%1.0f,%1.0f)", vCamPos.y, vCamPos.z );
					vCursorPos.Init( 0.0, rayMousePoint.m_Pos.y, rayMousePoint.m_Pos.z );
					sCursor.Format( "@(n/a,%1.0f,%1.0f)", vCursorPos.y, vCursorPos.z );
					break;
				case VM_RIGHT:
					sViewMode.LoadString( IDS_VIEW_RIGHT );
					sCamera.Format( "@(n/a,%1.0f,%1.0f)", vCamPos.y, vCamPos.z );
					vCursorPos.Init( 0.0, rayMousePoint.m_Pos.y, rayMousePoint.m_Pos.z );
					sCursor.Format( "@(n/a,%1.0f,%1.0f)", vCursorPos.y, vCursorPos.z );
					break;
				case VM_FRONT:
					sViewMode.LoadString( IDS_VIEW_FRONT );
					sCamera.Format( "@(%1.0f,%1.0f,n/a)", vCamPos.x, vCamPos.y );
					vCursorPos.Init( rayMousePoint.m_Pos.x, rayMousePoint.m_Pos.y, 0.0 );
					sCursor.Format( "@(%1.0f,%1.0f,n/a)", vCursorPos.x, vCursorPos.y );
					break;
				case VM_BACK:
					sViewMode.LoadString( IDS_VIEW_BACK );
					sCamera.Format( "@(%1.0f,%1.0f,n/a)", vCamPos.x, vCamPos.y );
					vCursorPos.Init( rayMousePoint.m_Pos.x, rayMousePoint.m_Pos.y, 0.0 );
					sCursor.Format( "@(%1.0f,%1.0f,n/a)", vCursorPos.x, vCursorPos.y );
					break;
				case VM_PERSPECTIVE:
				{
					CVector vDir;

					sViewMode.LoadString( IDS_VIEW_PERSP );
					sCamera.Format( "(%#3.1f,%#3.1f,%#3.1f)@(%1.0f,%1.0f,%1.0f)", 
						vCamOrient.x, vCamOrient.y, vCamOrient.z, 
						vCamPos.x, vCamPos.y, vCamPos.z );

					// Get the position and orientation the user selected with mouse...
					vCursorPos = rayMousePoint.m_Pos;
					vDir = rayMousePoint.m_Dir;
					vDir.Norm( );

					// Calculate a position along ray cast by mouse...
					vCursorPos = vCursorPos + vDir * 100;

					sCursor.Format( "@(%1.0f,%1.0f,%1.0f)", 
						vCursorPos.x, vCursorPos.y, vCursorPos.z );
					break;
				}
			}

			pFrame->UpdateStatusText(ID_INDICATOR_VIEWTYPE, sViewMode.GetBuffer(0));
			pFrame->UpdateStatusText(ID_INDICATOR_CAMERA, sCamera.GetBuffer(0));
			pFrame->UpdateStatusText(ID_INDICATOR_CURSOR, sCursor.GetBuffer(0));
		}
	}
}

void CRegionView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	if( bActivate )
	{
		CRegionDoc *pDoc;
		pDoc = GetRegionDoc( );
		if( pDoc != NULL && !pDoc->m_WorldName.IsEmpty( ))
		{
			GetNodeView()->Init(pDoc);
		}
	}
	
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);	
}

void CRegionView::OnActivateFrame(UINT nState, CFrameWnd* pFrameWnd)
{
	if (!nState)
		m_cTrackerMgr.Cancel();

	CView::OnActivateFrame(nState, pFrameWnd);
}

void CRegionView::OnUseDevice1()
{
	RestartRender(FALSE, 0, -1, TRUE);
}


void CRegionView::OnUseDevice2()
{
	RestartRender(FALSE, 1, -1, TRUE);
}


void CRegionView::OnUseDevice3()
{
	RestartRender(FALSE, 2, -1, TRUE);
}


void CRegionView::OnUseDevice4()
{
	RestartRender(FALSE, 3, -1, TRUE);
}


void CRegionView::OnRenderMode1()
{
	RestartRender(FALSE, -1, 0, TRUE);
}


void CRegionView::OnRenderMode2()
{
	RestartRender(FALSE, -1, 1, TRUE);
}


void CRegionView::OnRenderMode3()
{
	RestartRender(FALSE, -1, 2, TRUE);
}


void CRegionView::OnRenderMode4()
{
	RestartRender(FALSE, -1, 3, TRUE);
}


//
//  This is used for showing brush names when the mouse pointer is held over them
//


LRESULT CRegionView::OnMouseHover(WPARAM wParam, LPARAM lParam) 
{

	char output[512];
	CBrushRef currentBrush = GetMouseOverBrush();

	// This seems to bomb out under odd circumstances, so I added some nested ifs for debugging
	if(currentBrush.IsValid())
	{
		if(currentBrush.m_pBrush != NULL)
		{
			if(currentBrush.m_pBrush->GetName() != NULL)
			{
				sprintf(output, "Brush Name: %s", currentBrush.m_pBrush->GetName());
			}
			else ASSERT(false); // Let David C. know if this asserts
		}
		else ASSERT(false); // Let David C. know if this asserts
	} 
	else 
	{
		sprintf(output, "   ");
	}

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->UpdateStatusText(ID_INDICATOR_BRUSHNAME, output);

	m_bMouseOver = FALSE;

	return 0;
}


LRESULT CRegionView::OnMouseLeave(WPARAM wParam, LPARAM lParam) 
{

	m_bMouseOver = FALSE;

	return 0;
}

void CRegionView::OnSelectPrefab()
{
	//get the first selected prefab
	uint32 nNumSel = GetRegion()->GetNumSelections();
	for(uint32 nCurrSel = 0; nCurrSel < nNumSel; nCurrSel++)
	{
		if(GetRegion()->GetSelection(nCurrSel)->GetType() == Node_PrefabRef)
		{
			GetProjectBar()->SelectPrefab(((CPrefabRef*)GetRegion()->GetSelection(nCurrSel))->GetPrefabFilename());
			break;
		}
	}
}

void CRegionView::OnUpdateSelectPrefab(CCmdUI* pCmdUI)
{
	//see if we have any prefabs selected
	uint32 nNumSel = GetRegion()->GetNumSelections();
	BOOL bEnable = FALSE;

	for(uint32 nCurrSel = 0; nCurrSel < nNumSel; nCurrSel++)
	{
		if(GetRegion()->GetSelection(nCurrSel)->GetType() == Node_PrefabRef)
		{
			bEnable = TRUE;
			break;
		}
	}

	pCmdUI->Enable(bEnable);	
	SetMenuHotKeyText(UIE_SELECT_PREFAB, pCmdUI);
}


void CRegionView::ReplaceTextures(bool bSelectedOnly)
{
	LPOS lPos;
	DWORD i;
	CEditBrush *pBrush;
	CEditPoly *pPoly;
	int nOccurences;
	char szTextName[MAX_PATH], *pSrcTextureName;
	PreActionList actionList;
	CEditRegion *pRegion;
	CMoArray<CEditPoly *> cPolyList;

	if( !IPoly().IsValid())
		return;

	GetProjectBar()->GetCurrentTextureName(szTextName);
	pSrcTextureName = IPoly()()->GetTexture(GetCurrTexture()).m_pTextureName;
	pRegion = GetRegion();
	
	nOccurences = 0;	
	for(lPos=pRegion->m_Brushes; lPos; )
	{
		pBrush = pRegion->m_Brushes.GetNext(lPos);
	
		//see if we want to skip this one because it isn't selected
		if(bSelectedOnly && !pBrush->IsFlagSet(NODEFLAG_SELECTED))
			continue;
	
		for(i=0; i < pBrush->m_Polies; i++)
		{
			pPoly = pBrush->m_Polies[i];

			const char* pszTextureName = pPoly->GetTexture(GetCurrTexture()).m_pTextureName;

			if(pszTextureName)
			{
				if(CHelpers::UpperStrcmp(pszTextureName, pSrcTextureName))
				{
					++nOccurences;
					// Add an undo.
					AddToActionListIfNew(&actionList, new CPreAction(ACTION_MODIFYNODE, pBrush), TRUE);
					// Add it to the modify list
					cPolyList.Add(pPoly);
				}
			}
		}
	}

	// Save the undo list.
	GetRegionDoc()->Modify(&actionList, TRUE);

	// Actually modify the polies
	for(i = 0; i < cPolyList.GetSize(); i++)
	{
		pPoly = cPolyList[i];
		const char* pszTextureName = pPoly->GetTexture(GetCurrTexture()).m_pTextureName;

		if(pszTextureName)
		{
			if(CHelpers::UpperStrcmp(pszTextureName, pSrcTextureName))
			{
				pPoly->GetTexture(GetCurrTexture()).m_pTextureName = pRegion->m_pStringHolder->AddString(szTextName);
				pPoly->GetTexture(GetCurrTexture()).UpdateTextureID();
			}
		}
	}

	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnReplaceTextures()
{
	ReplaceTextures(false);
}

void CRegionView::OnUpdateReplaceTextures(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IPoly().IsValid());
	SetMenuHotKeyText(UIE_REPLACE_TEXTURES, pCmdUI);
}

void CRegionView::OnReplaceTexturesInSel()
{
	ReplaceTextures(true);
}

void CRegionView::OnUpdateReplaceTexturesInSel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IPoly().IsValid());
	SetMenuHotKeyText(UIE_REPLACE_TEXTURES_IN_SEL, pCmdUI);
}

void CRegionView::OnSelectTexture()
{
	//(YF 11-29-00)
	CEditPoly* pPoly = NULL;	

	//see if we are in geometry mode
	if( GetEditMode() == GEOMETRY_EDITMODE )		
	{	
		//we are in geometry mode. See if we are pointing at a face currently
		if( !IPoly( ).IsValid( ))

		{	
			// Not pointing at a face so select texture from first selected poly
			CMoArray<CEditPoly*> polies;
			polies.SetCacheSize(50);
			GetSelectedPolies( polies );
			if( polies > 0 )
			{
				pPoly = polies[ 0 ];
			}
		}
		else
		{
			//we are pointing at a face, so lets get the texture from that
			pPoly = IPoly()();
		}
	}
	else
	{
		//we are in brush editing mode, and should therefore 
		CEditRegion *pRegion = GetRegion();
		for( uint32 nCurrNode = 0; nCurrNode < pRegion->GetNumSelections(); ++nCurrNode )
		{
			CWorldNode* pNode = pRegion->GetSelection( nCurrNode );
			if( pNode->GetType() == Node_Brush )
			{
				CEditBrush* pBrush = pNode->AsBrush();
				ASSERT( pBrush );

				for( uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies; ++nCurrPoly )
				{
					//get this current polygon
					pPoly = pBrush->m_Polies[ nCurrPoly ];

					//see if this polygon has a valid texture
					if( pPoly && pPoly->GetTexture(GetCurrTexture()).m_pTextureName && 
						(strlen( pPoly->GetTexture(GetCurrTexture()).m_pTextureName ) > 0) && 
						stricmp( pPoly->GetTexture(GetCurrTexture()).m_pTextureName, "Default" ) != 0 )
					{
						// polygon with a texture found, so break out of the 2 for loops.
						nCurrNode = pRegion->GetNumSelections();	// this breaks out of the i loop.
						break;										// this breaks out of the j loop.
					}
				}
			}
		}
	}

	//if we have found a polygon, grab its texture
	if( pPoly )
	{	
		//actually select the texture
		GetProjectBar()->SetCurTextureSel( pPoly->GetTexture(GetCurrTexture()).m_pTextureFile );	
		GetProjectBar()->FindCurTextureSel();

		//set up the main frame status bar
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->UpdateStatusText( ID_INDICATOR_TEXTURE, pPoly->GetTexture(GetCurrTexture()).m_pTextureName );
		GetTextureDlg()->RenderLargeImage();
	}	

	return;
}


void CRegionView::OnUpdateSelectTexture(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(AreAnyPoliesSelected() ? TRUE : FALSE);
	SetMenuHotKeyText(UIE_SELECT_TEXTURE, pCmdUI);
}


void CRegionView::OnSelectBrushColor()
{
	CEditPoly* pPoly = NULL;	

	//see if we are in geometry mode
	if( GetEditMode() == GEOMETRY_EDITMODE )		
	{	
		//we are in geometry mode. See if we are pointing at a face currently
		if( !IPoly( ).IsValid( ))

		{	
			// Not pointing at a face so select color from first selected poly
			CMoArray<CEditPoly*> polies;
			polies.SetCacheSize(50);
			GetSelectedPolies( polies );
			if( polies > 0 )
			{
				pPoly = polies[ 0 ];
			}
		}
		else
		{
			//we are pointing at a face, so lets get the color from that
			pPoly = IPoly()();
		}
	}
	else
	{
		//we are in brush editing mode, and should therefore 
		CEditRegion *pRegion = GetRegion();
		for( uint32 nCurrNode = 0; nCurrNode < pRegion->GetNumSelections(); ++nCurrNode )
		{
			CWorldNode* pNode = pRegion->GetSelection( nCurrNode );
			if( pNode->GetType() == Node_Brush )
			{
				CEditBrush* pBrush = pNode->AsBrush();
				ASSERT( pBrush );

				//get this current polygon
				if(pBrush->m_Polies.GetSize() > 0)
				{
					pPoly = pBrush->m_Polies[ 0 ];
				}
			}
		}
	}

	//if we have found a polygon, grab its color
	if( pPoly )
	{	
		//select this color in the picker
		CEditBrush* pBrush = pPoly->m_pBrush;
		GetColorSelectDlg()->SetCurColor(RGB(pBrush->r, pBrush->g, pBrush->b), TRUE);
	}	

	return;
}


void CRegionView::OnUpdateSelectBrushColor(CCmdUI* pCmdUI)
{

	pCmdUI->Enable(AreAnyPoliesSelected() ? TRUE : FALSE);
	SetMenuHotKeyText(UIE_SELECT_BRUSH_COLOR, pCmdUI);
}

void CRegionView::OnResetDetailLevels()
{
	CEditRegion *pRegion;
	CEditBrush *pBrush;
	LPOS pos;
	CBoolProp *pProp;

	pRegion = GetRegion();
	for(pos=pRegion->m_Brushes; pos; )
	{
		pBrush = pRegion->m_Brushes.GetNext(pos);

		pProp = (CBoolProp*)pBrush->m_PropList.GetProp("Detail");
		if(pProp)
		{
			pProp->m_Value = false;
		}
	}
}

void CRegionView::OnUpdateAllObjectProperties()
{
	CRegionDoc		*pRegionDoc = LTNULL;

	pRegionDoc = GetRegionDoc();
	if( !pRegionDoc ) 
		return;

	pRegionDoc->UpdateAllObjectProperties();
}

/******************************************************************/
// Store the current camera position in the list of navigator positions
void CRegionView::OnNavigatorStore() 
{
	// Create the default position string based off of this view
	CString sString;
	sString.Format("POS(%.1f, %.1f, %.1f)", Nav().m_Position.x, Nav().m_Position.y, Nav().m_Position.z);

	// Ask the user for the name that they want for this position
	CNavigatorStoreDlg storeDlg;
	storeDlg.m_sName=sString;

	if (storeDlg.DoModal() == IDOK)
	{		
		// Create the new navigation position
		CNavigatorPosItem *pNavigatorItem=new CNavigatorPosItem();

		// Set the name
		pNavigatorItem->SetDescription(storeDlg.m_sName.GetBuffer(0));

		// Add each view Nav position			
		CDocument* pDoc = GetDocument();
		POSITION pos = pDoc->GetFirstViewPosition();

		// Loop through each view
		while (pos != NULL)
		{
			CRegionView* pView =(CRegionView *)pDoc->GetNextView(pos);
			if(pView != NULL)
			{
				// Add the navigator
				CNavigator *pNav=new CNavigator;
				*pNav=pView->Nav();
				pNavigatorItem->AddNavigator(pNav);
			}	
		}

		// Get the edit region
		CEditRegion *pRegion = GetRegion();

		// Store the position in the array
		if (pRegion)
		{
			CNavigatorPosArray *pArray=pRegion->GetNavigatorPosArray();
			if (pArray)
			{
				pArray->Add(pNavigatorItem);
			}
		}

		// Set the modified flag		
		if (pDoc)
		{
			pDoc->SetModifiedFlag(TRUE);
		}
	}
}

void CRegionView::OnUpdateNavigatorStore(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_NAVIGATOR_STORE, pCmdUI);
}

/******************************************************************/
// Open the navigator organizer dialog
void CRegionView::OnNavigatorOrganize() 
{
	// Get the edit region
	CEditRegion *pRegion = GetRegion();
	if (!pRegion)
	{
		return;
	}

	// Create the navigator dialog
	CNavigatorDlg navigatorDlg;
	navigatorDlg.SetRegionView(this);

	navigatorDlg.SetNavigatorPosArray(pRegion->GetNavigatorPosArray());
	navigatorDlg.DoModal();	

	// Set the modified flag
	CDocument *pDoc=GetDocument();
	if (pDoc)
	{
		pDoc->SetModifiedFlag(TRUE);
	}
}

void CRegionView::OnUpdateNavigatorOrganize(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_NAVIGATOR_ORGANIZE, pCmdUI);
}

/******************************************************************/
// Go to the next navigator position
void CRegionView::OnNavigatorGotoNext() 
{
	// Get the edit region
	CEditRegion *pRegion = GetRegion();
	if (!pRegion)
	{
		return;
	}

	// Get the navigator array
	CNavigatorPosArray *pNavigatorArray=pRegion->GetNavigatorPosArray();
	if (!pNavigatorArray || pNavigatorArray->GetSize() <= 0)
	{
		return;
	}

	// Get the last viewed position and increment it
	int nIndex=pRegion->m_nLastNavigatorPos;
	nIndex++;	

	// Wrap the index if necessary
	if (nIndex > pNavigatorArray->GetSize()-1)
	{
		nIndex=0;
	}

	// View the position
	SwitchNavigator(nIndex);
}

void CRegionView::OnUpdateNavigatorGotoNext(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_NAVIGATOR_NEXT, pCmdUI);
}


/******************************************************************/
// Go to the previous navigator position
void CRegionView::OnNavigatorGotoPrevious() 
{
	// Get the edit region
	CEditRegion *pRegion = GetRegion();
	if (!pRegion)
	{
		return;
	}

	// Get the navigator array
	CNavigatorPosArray *pNavigatorArray=pRegion->GetNavigatorPosArray();
	if (!pNavigatorArray || pNavigatorArray->GetSize() <= 0)
	{
		return;
	}

	// Get the last viewed position and decrement it
	int nIndex=pRegion->m_nLastNavigatorPos;
	nIndex--;	

	// Wrap the index if necessary
	if (nIndex < 0)
	{
		nIndex=pNavigatorArray->GetSize()-1;
	}

	// View the position
	SwitchNavigator(nIndex);	
}

void CRegionView::OnUpdateNavigatorGotoPrevious(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_NAVIGATOR_PREVIOUS, pCmdUI);
}


// Switch the navigator to one of the stored navigators for this level
void CRegionView::SwitchNavigator(int nNavIndex)
{	
	// Get the edit region
	CEditRegion *pRegion = GetRegion();	
	if (!pRegion)
	{
		return;
	}

	// Get the navigator array
	CNavigatorPosArray *pNavigatorArray=pRegion->GetNavigatorPosArray();
	if (!pNavigatorArray)
	{
		return;
	}

	// Get the document
	CDocument* pDoc = GetDocument();
	POSITION pos = pDoc->GetFirstViewPosition();

	// Loop through each view
	int nCurrentIndex=0;
	while (pos != NULL)
	{
		CRegionView* pView =(CRegionView *)pDoc->GetNextView(pos);
		if(pView != NULL)
		{
			// Set the navigator
			CNavigator *pNavigator=pNavigatorArray->GetAt(nNavIndex)->GetNavigator(nCurrentIndex);
			pView->Nav()=*pNavigator;
			nCurrentIndex++;
		}	
	}

	// Set the "last viewed position" variable
	pRegion->m_nLastNavigatorPos=nNavIndex;

	GetRegionDoc()->RedrawAllViews();
}

/************************************************************************/
// Handles the "Select All" command
void CRegionView::OnSelectionSelectAll() 
{
	// Get the edit region
	CEditRegion *pRegion=GetRegion();
	
	// Show the wait cursor
	BeginWaitCursor();

	// Select all of the nodes
	if (pRegion)
	{	
		pRegion->RecurseAndSelect(pRegion->GetRootNode(), FALSE);

		// Update the selection array
		pRegion->UpdateSelectionArray();

		// Make sure that the root node isn't selected
		pRegion->UnselectNode(pRegion->GetRootNode());
	}	

	// Redraw
	GetRegionDoc()->NotifySelectionChange();
	GetRegionDoc()->RedrawAllViews();

	// Stop showing the wait cursor
	EndWaitCursor();
}

void CRegionView::OnUpdateSelectionSelectAll(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_SELECT_ALL, pCmdUI);
}

/************************************************************************/
// Handles the "Select None" command
void CRegionView::OnSelectionSelectNone() 
{
	// Get the edit region
	CEditRegion *pRegion=GetRegion();
	
	// Select all of the nodes
	if (pRegion)
	{
		pRegion->RecurseAndUnselect(pRegion->GetRootNode(), FALSE);

		// Update the selection array
		pRegion->UpdateSelectionArray();
	}

	// Redraw
	GetRegionDoc()->NotifySelectionChange();
	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdateSelectionSelectNone(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_SELECT_NONE, pCmdUI);
}

/************************************************************************/
// Handles the "Select Inverse" command
void CRegionView::OnSelectionSelectInverse() 
{
	// Get the edit region
	CEditRegion *pRegion=GetRegion();
	
	// Select all of the nodes
	if (pRegion)
	{
		pRegion->RecurseAndInverseSelect(pRegion->GetRootNode());

		// Make sure that the root node isn't selected
		pRegion->UnselectNode(pRegion->GetRootNode());

		// Update the selection array
		pRegion->UpdateSelectionArray();
	}

	// Redraw
	GetRegionDoc()->RedrawAllViews();
}

/************************************************************************/
// The command UI update for the "Select Inverse" command
void CRegionView::OnUpdateSelectionSelectInverse(CCmdUI* pCmdUI) 
{
	// Get the edit region
	CEditRegion *pRegion=GetRegion();
	
	if (pRegion)
	{
		// Get the number of selections
		if (pRegion->GetNumSelections() > 0)
		{
			// Enable the control
			pCmdUI->Enable(TRUE);
		}
		else
		{
			// Disable the control
			pCmdUI->Enable(FALSE);
		}
	}
	
	SetMenuHotKeyText(UIE_SELECT_INVERSE, pCmdUI);
}

/************************************************************************/
// Handles the "Advanced Select" command
void CRegionView::OnSelectionAdvanced() 
{				
	// Show the advanced select dialog
	AdvancedSelectDlg dlg;
	dlg.LoadRegistrySettings();

	if(dlg.DoModal() == IDOK)
	{
		dlg.SaveRegistrySettings();

		// Set the select flag based on the radio button
		BOOL bSelect;
		if (dlg.m_nSelect == 0)
		{
			bSelect=TRUE;
		}
		else
		{
			bSelect=FALSE;
		}

		// Get the document
		CRegionDoc *pDoc=(CRegionDoc *)GetDocument();
		if(pDoc)
		{
			// Setup the advanced select class
			CAdvancedSelect advancedSelect;
			advancedSelect.Init(pDoc);

			// Add the criteria
			if (dlg.m_bNodesOfClass)
			{
				advancedSelect.AddClassCriteria(dlg.m_ClassName);
			}
			if (dlg.m_bObjectsWithName)
			{
				advancedSelect.AddNameCriteria(dlg.m_ObjectName, !dlg.m_bMatchWholeWord);
			}
			if (dlg.m_bNodesWithProperty)
			{
				advancedSelect.AddPropertyCriteria(dlg.AllocPropertyFromData(), dlg.m_bMatchValue);
			}

			// Show the wait cursor
			BeginWaitCursor();

			// Turn off the drawing of the node view
			if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
			{
				GetNodeView()->SetRedraw(FALSE);
			}

			// Do the selection operation
			if (bSelect)
			{
				advancedSelect.Select(TRUE, dlg.m_bShowResults);
			}
			else
			{
				advancedSelect.Unselect(TRUE, dlg.m_bShowResults);
			}			

			// Turn on the drawing of the node view
			if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
			{
				GetNodeView()->SetRedraw(TRUE);
			}

			GetNodeView()->Update(); // Refresh node view

			advancedSelect.ClearCriteria(); // Clean-up

			// Stop showing the wait cursor
			EndWaitCursor();
		}
	}	
}

void CRegionView::OnUpdateSelectionAdvanced(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_ADVANCED_SELECT, pCmdUI);
}



/************************************************************************/
// Handles the "Select Container" command
void CRegionView::OnSelectionContainer() 
{
	// Get the document
	CRegionDoc *pDoc=GetRegionDoc();
	if (!pDoc)		
	{
		return;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();
	if (!pRegion)
	{
		return;
	}

	// Make sure that there are selections
	if (pRegion->GetNumSelections() <= 0)
	{
		return;
	}

	// Store the currently selected nodes	
	CMoArray<CWorldNode *>	m_selectedNodeArray;
	m_selectedNodeArray.SetCacheSize(50);

	int i;
	for (i=0; i < pRegion->GetNumSelections(); i++)
	{
		CWorldNode *pSelection=pRegion->GetSelection(i);

		// Skip over container nodes
//		if (pSelection->GetType() != Node_Null)
		{
			m_selectedNodeArray.Add(pSelection);
		}
	}

	// Make an array of the container nodes for the selected nodes
	CMoArray<CWorldNode *>	m_containerArray;
	m_containerArray.SetCacheSize(50);

	// Select the container node for each selected node
	for (i=0; i < m_selectedNodeArray.GetSize(); i++)
	{
		// Get the container
		CWorldNode *pContainer=m_selectedNodeArray[i]->GetParent();		
		
		// Make sure that the container hasn't already been added to the array
		BOOL bFound=FALSE;

		int n;
		for (n=0; n < m_containerArray.GetSize(); n++)
		{
			if (m_containerArray[n] == pContainer)
			{
				bFound=TRUE;
				break;
			}
		}

		// Add the container if it wasn't found in the array
		if (!bFound)
		{
			m_containerArray.Add(pContainer);
		}
	}

	// Select all of the container nodes
	for (i=0; i < m_containerArray.GetSize(); i++)
	{
		// Select the nodes
		pRegion->RecurseAndSelect(m_containerArray[i]);		
	}		

	// Make sure that the root node didn't get selected
	pRegion->UnselectNode(pRegion->GetRootNode());

	// Notify that the selection has changed
	pDoc->NotifySelectionChange();
}

/************************************************************************/
// Command update handler for the "Select Container" command
void CRegionView::OnUpdateSelectionContainer(CCmdUI* pCmdUI) 
{
	// Get the document
	CRegionDoc *pDoc=GetRegionDoc();
	if (!pDoc)		
	{
		return;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();
	if (!pRegion)
	{
		return;
	}

	if (pRegion->GetNumSelections() <= 0)
	{
		pCmdUI->Enable(FALSE);
	}	

	SetMenuHotKeyText(UIE_SELECT_CONTAINER, pCmdUI);
}

/************************************************************************/
// Handles the "Hide Selected" command
void CRegionView::OnSelectionHideSelected() 
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}

	// Put the wait cursor on the screen
	BeginWaitCursor( );

	int i;

	// Setup the undo information 
	if(GetApp()->GetOptions().GetMiscOptions()->IsUndoFreezeHide())
	{
		PreActionList actionList;	
		for (i=0; i < pRegion->GetNumSelections(); i++)
		{
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pRegion->GetSelection(i)));
		}
		GetRegionDoc()->Modify(&actionList, TRUE);
	}
	else
		GetRegionDoc()->Modify();

	for (i=0; i < pRegion->GetNumSelections(); i++)
	{
		// Hide the node
		pRegion->GetSelection(i)->HideNode();
	}

	// Update the node images
	GetNodeView()->UpdateNodeImage(pRegion->GetRootNode());

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();

	// End the wait cursor
	EndWaitCursor( );
}

/************************************************************************/
// The command UI update for the "Hide Selected" command
void CRegionView::OnUpdateSelectionHideSelected(CCmdUI* pCmdUI) 
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}

	if (pRegion->GetNumSelections() <= 0)
	{
		pCmdUI->Enable(FALSE);
	}
	SetMenuHotKeyText(UIE_HIDE_SELECTED, pCmdUI);
}

/************************************************************************/
// Handles the "Unhide Selected" command
void CRegionView::OnSelectionUnhideSelected() 
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}	

	// Put the wait cursor on the screen
	BeginWaitCursor( );
	
	int i;

	// Setup the undo information 
	if(GetApp()->GetOptions().GetMiscOptions()->IsUndoFreezeHide())
		{
		PreActionList actionList;	
		for (i=0; i < pRegion->GetNumSelections(); i++)
		{
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pRegion->GetSelection(i)));
		}
		GetRegionDoc()->Modify(&actionList, TRUE);
	}
	else
		GetRegionDoc()->Modify();

	for (i=0; i < pRegion->GetNumSelections(); i++)
	{
		// Show the node
		pRegion->GetSelection(i)->ShowNode();
	}

	// Update the node images
	GetNodeView()->UpdateNodeImage(pRegion->GetRootNode());

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();

	// End the wait cursor
	EndWaitCursor( );
}

/************************************************************************/
// The command UI update for the "Unhide Selected" command
void CRegionView::OnUpdateSelectionUnhideSelected(CCmdUI* pCmdUI) 
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}
	
	if (pRegion->GetNumSelections() <= 0)
	{
		pCmdUI->Enable(FALSE);
	}
	SetMenuHotKeyText(UIE_UNHIDE_SELECTED, pCmdUI);
}

/************************************************************************/
// Handles the "Hide Inverse" command
void CRegionView::OnSelectionHideInverse() 
{
	// Get the edit region
	CEditRegion *pRegion=GetRegion();
	
	// Put the wait cursor on the screen
	BeginWaitCursor( );

	// Hide the inverse nodes
	if (pRegion)
	{
		// Setup the undo information 
		if(GetApp()->GetOptions().GetMiscOptions()->IsUndoFreezeHide())
		{
			PreActionList actionList;	
			UndoHelper(&actionList, pRegion->GetRootNode(), NODEFLAG_SELECTED, false);
			GetRegionDoc()->Modify(&actionList, TRUE);
		}
		else
			GetRegionDoc()->Modify();

		pRegion->RecurseAndInverseHide(pRegion->GetRootNode());		

		// Make sure that the root is not hidden
		pRegion->GetRootNode()->ShowNode();
	}

	// Update the node images
	GetNodeView()->UpdateNodeImage(pRegion->GetRootNode());

	// Redraw
	GetRegionDoc()->RedrawAllViews();	

	// End the wait cursor
	EndWaitCursor( );
}

/************************************************************************/
// The command UI update for the "Hide Inverse" command
void CRegionView::OnUpdateSelectionHideInverse(CCmdUI* pCmdUI) 
{
	SetMenuHotKeyText(UIE_HIDE_INVERSE, pCmdUI);
}

/************************************************************************/
// Handles the "Unhide Inverse" command
void CRegionView::OnSelectionUnhideInverse() 
{
	// Get the edit region
	CEditRegion *pRegion=GetRegion();

	// Put the wait cursor on the screen
	BeginWaitCursor( );

	// Hide the inverse nodes
	if (pRegion)
	{
		// Setup the undo information 
		if(GetApp()->GetOptions().GetMiscOptions()->IsUndoFreezeHide())
		{
			PreActionList actionList;	
			UndoHelper(&actionList, pRegion->GetRootNode(), NODEFLAG_SELECTED, false);
			GetRegionDoc()->Modify(&actionList, TRUE);
		}
		else
			GetRegionDoc()->Modify();


		pRegion->RecurseAndInverseUnhide(pRegion->GetRootNode());		
	}

	// Update the node images
	GetNodeView()->UpdateNodeImage(pRegion->GetRootNode());

	// Redraw
	GetRegionDoc()->RedrawAllViews();	

	// End the wait cursor
	EndWaitCursor( );
}

/************************************************************************/
// The command UI update for the "Unhide Inverse" command
void CRegionView::OnUpdateSelectionUnhideInverse(CCmdUI* pCmdUI) 
{	
	SetMenuHotKeyText(UIE_UNHIDE_INVERSE, pCmdUI);
}

/************************************************************************/
// The group function was called
void CRegionView::OnSelectionGroup() 
{	
	// Get the document
	CRegionDoc *pDoc=GetRegionDoc();
	if (!pDoc)
	{
		return;
	}

	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}	

	// Put the wait cursor on the screen
	BeginWaitCursor( );

	// Add the null node to be parent...
	CWorldNode *pContainer = GetRegion()->AddNullNode(GetRegion()->GetActiveParentNode());
	
	// Add to the undo list.
	PreActionList actionList;
	actionList.AddTail(new CPreAction(ACTION_ADDEDNODE, pContainer));
				
	// Move the nodes
	pDoc->MoveSelectedNodes(pContainer);

	// Select the new container node
	GetRegion( )->RecurseAndSelect(pContainer);

	// Stop displaying the wait cursor
	EndWaitCursor( );	
}

/************************************************************************/
// Command UI updater for the group function
void CRegionView::OnUpdateSelectionGroup(CCmdUI* pCmdUI) 
{
	// Get the document
	CRegionDoc *pDoc=GetRegionDoc();
	if (!pDoc)		
	{
		return;
	}

	// Get the region
	CEditRegion *pRegion=pDoc->GetRegion();
	if (!pRegion)
	{
		return;
	}
	
	if (pRegion->GetNumSelections() <= 0)
	{
		pCmdUI->Enable(FALSE);
	}		

	SetMenuHotKeyText(UIE_GROUP_SELECTION, pCmdUI);
}

/************************************************************************/
// The split brush option was selected from the menu
void CRegionView::OnBrushSplitBrush() 
{		
	if (GetEditMode() != BRUSH_EDITMODE)
		return;

	CEditRegion *pRegion=GetRegion();

	// Make sure that we have one brush selected
	if((pRegion->GetNumSelections() < 1) || (pRegion->GetSelection(0)->GetType() != Node_Brush))
		return;

	LTVector forward, other;
	CEditPlane edgePlane;
	CEditBrush *pToSplit;
	CEditBrush *sides[2];
	DWORD indSel, indPoly, indPoint, indSide,indBrush;
	PreActionList actionList;
	CMoArray<CEditBrush*> toDelete;
	CEditVert newPt;
	PolySide side;
	CEditPoly *pPoly;
	CReal dot;

	// Check to see if we can split and get the last selected brush index
	int nLastSelectedBrush=0;
	if (!CanSplitBrush(&nLastSelectedBrush))
	{
		return;
	}	
	
	// Get the drawing brush
	CEditBrush *pBrush=NULL;
	
	// Check to see if the splitting plane should be from the drawing brush
	BOOL bSplitFromView=FALSE;
	if (DrawingBrush().m_Points.GetSize() > 0)
	{
		bSplitFromView=TRUE;
		pBrush=&DrawingBrush();

		// Make a plane from its points and our view.
		forward = ViewDef()->m_Nav.Forward();
		other = pBrush->m_Points[1] - pBrush->m_Points[0];
		edgePlane.m_Normal = forward.Cross(other);
		edgePlane.m_Normal.Norm();
		edgePlane.m_Dist = edgePlane.m_Normal.Dot(pBrush->m_Points[0]);
	}
	else
	{
		// Get the last selected brush
		pBrush=pRegion->GetSelection(nLastSelectedBrush)->AsBrush();

		// Use the last selected brush
		edgePlane.m_Normal = pBrush->m_Polies[0]->Normal();
		edgePlane.m_Normal.Norm();
		edgePlane.m_Dist = pBrush->m_Polies[0]->Dist();
	}

	for(indSel=0; indSel < pRegion->m_Selections; indSel++)
	{
		if(pRegion->m_Selections[indSel]->GetType() == Node_Brush)
		{
			pToSplit = pRegion->m_Selections[indSel]->AsBrush();
			
			// Don't split the splitting brush
			if (pToSplit == pBrush)
			{
				continue;
			}

			if(pToSplit->m_Polies)
			{
				// whether any of the brush's polies are on the front or the back of the splitting plane
				bool onBack = false, onFront = false; 

				// Check to see which side of the splitting plane the brush is on
				for (indPoly=0; indPoly < pToSplit->m_Polies.GetSize(); indPoly++)
				{
					pPoly = pToSplit->m_Polies[indPoly];

					for(indPoint=0; indPoint < pPoly->m_Indices.GetSize(); indPoint++ )
					{
						dot = edgePlane.m_Normal.Dot(pPoly->Pt(indPoint)) - edgePlane.m_Dist;
						if( dot > POINT_SIDE_EPSILON )
						{
							onFront = true;
						}
						else if( dot < -POINT_SIDE_EPSILON )
						{
							onBack  = true;
						}
					}
				}

				// Only continue if a useful split
				if (onBack && onFront)
				{

					CEditPoly temp(pToSplit->m_Polies[0]);
					temp.m_Plane = edgePlane;
					temp.m_Indices.Term();
					
					temp.m_Indices.Append(pToSplit->AddVertOrGetClosest(pBrush->m_Points[0], 0.01f));
					temp.m_Indices.Append(pToSplit->AddVertOrGetClosest(pBrush->m_Points[1], 0.01f));

					if (bSplitFromView)
					{
						// Create the new point from the view vector
						newPt = pBrush->m_Points[0] + forward;
					}
					else
					{
						// Just grab the 3rd point in the polygon
						newPt = pBrush->m_Points[2];
					}

					temp.m_Indices.Append(pToSplit->AddVertOrGetClosest(newPt, 0.01f));
					
					SimpleSplitBrush(pRegion, &temp, pToSplit, sides);
					// Update the verts in the original brush for proper undo handling
					pToSplit->RemoveUnusedVerts();
					if(sides[0]->m_Polies >= 1 && sides[1]->m_Polies >= 1)
					{
						// Setup an undo.
						actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pToSplit));
						actionList.AddTail(new CPreAction(ACTION_ADDEDNODE, sides[0]));
						actionList.AddTail(new CPreAction(ACTION_ADDEDNODE, sides[1]));

						for(indSide=0; indSide < 2; indSide++)
						{
							sides[indSide]->RemoveUnusedVerts();
							sides[indSide]->UpdateBoundingInfo();
						}

						no_AttachNode(pRegion, sides[0], pToSplit->GetParent());
						no_AttachNode(pRegion, sides[1], pToSplit->GetParent());
						
						// (delete later so we can make the undo)
						toDelete.Append(pToSplit);
					}
					else
					{
						no_DestroyNode(pRegion, sides[0], FALSE);
						no_DestroyNode(pRegion, sides[1], FALSE);
					}
				}
			}
		}
	}

	GetRegionDoc()->Modify(&actionList, TRUE);

	// Delete the brushes that were split.
	for(indBrush=0; indBrush < toDelete; indBrush++)
	{
		GetNodeView( )->DeleteNode(toDelete[indBrush]);
		no_DestroyNode(pRegion, toDelete[indBrush], FALSE);
	}

	pBrush->Term();
	m_EditState = EDIT_NOSTATE;

	GetRegionDoc()->RedrawAllViews();


}

//used to notify when the hot keys have been changed
void CRegionView::OnHotKeysChanged()
{
	m_cTrackerMgr.FlushTrackers();
}

/************************************************************************/
// The command update handler for the split brush command
void CRegionView::OnUpdateBrushSplitBrush(CCmdUI* pCmdUI) 
{	
	// Start by enabling the menu item
	pCmdUI->Enable(CanSplitBrush());

	SetMenuHotKeyText(UIE_SPLIT_BRUSH, pCmdUI);
}


/************************************************************************/
// Returns TRUE if we are in a situation in which we can split a brush.
// pnLastSelectedBrush is filled in with the last selected brush index.
BOOL CRegionView::CanSplitBrush(int *pnLastSelectedBrush)
{
	// Get the region
	CEditRegion *pRegion=GetRegion();

	// Make sure that there is at least something selected
	if (pRegion->GetNumSelections() == 0)
	{
		return FALSE;
	}
	
	// See how many brushes are selected and store the index of the last brush
	int nBrushesSelected=0;
	int nLastBrushIndex=0;
	
	int i;
	for(i=0; i < pRegion->GetNumSelections(); i++)
	{
		if(pRegion->GetSelection(i)->GetType() == Node_Brush)
		{
			nBrushesSelected++;
			nLastBrushIndex=i;			
		}
	}

	// Store the last selected brush
	if (pnLastSelectedBrush)
	{
		*pnLastSelectedBrush=nLastBrushIndex;
	}

	// We must have at least one brush selected
	if (nBrushesSelected == 0)
	{		
		return FALSE;
	}

	// Check to see if the user is using the drawing brush
	if (DrawingBrush().m_Points > 0)
	{
		// There must be only 2 points set
		if (DrawingBrush().m_Points != 3)
		{			
			return FALSE;
		}

		// Don't allow splitting in the perspective view
		if (IsPerspectiveViewType())
		{			
			return FALSE;
		}	
	}
	else
	{
		// There must be more than one brush selected
		if (nBrushesSelected < 2)
		{			
			return FALSE;
		}

		// Check to see if the last brush is a plane (one poly)
		CEditBrush *pLastBrush=pRegion->GetSelection(nLastBrushIndex)->AsBrush();

		if (pLastBrush->m_Polies.GetSize() != 1)
		{			
			return FALSE;
		}

		// Make sure that the brush has at least 3 points
		if (pLastBrush->m_Points.GetSize() < 3)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/************************************************************************/
// Handler for the join command
void CRegionView::OnBrushJoin() 
{
	if (GetEditMode() != BRUSH_EDITMODE)
		return;

	CEditRegion *pRegion=GetRegion();

	// Make sure that we have at least 2 brushes selected
	if(pRegion->GetNumSelections() < 2)
		return;

	PreActionList actionList;

	// Build a list of mods.
	for(uint32 nUndoLoop =0; nUndoLoop < pRegion->m_Selections.GetSize(); ++nUndoLoop)
	{
		CWorldNode *pNode = pRegion->m_Selections[nUndoLoop ];
		
		if(pNode->GetType() == Node_Brush)
		{
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pNode));
		}
	}
	
	// Setup the undo.
	GetRegionDoc()->Modify(&actionList, FALSE);

	// Start joinin'!
	CEditBrush *pNewBrush = LTNULL;

	// Add the other brushes to the first brush
	for (uint32 nJoinLoop = 0; nJoinLoop < pRegion->m_Selections.GetSize(); ++nJoinLoop)
	{
		CWorldNode *pJoinNode = pRegion->m_Selections[nJoinLoop];
		if (pJoinNode->GetType() != Node_Brush)
		{
			// If it's not a brush, un-select and try again..
			pRegion->UnselectNode(pJoinNode);
			--nJoinLoop;
			continue;
		}
		
		CEditBrush *pJoinBrush = pJoinNode->AsBrush();

		// Use the first brush we come to..
		if (!pNewBrush)
		{
			pNewBrush = pJoinBrush;
			pRegion->UnselectNode(pJoinNode);
			--nJoinLoop;
			continue;
		}

		uint32 nVertOffset = pNewBrush->m_Points.GetSize();

		// Adjust the vertex references (Note : Reverse order to prevent overwrites..)
		for (uint32 nAdjVertLoop = pJoinBrush->m_Points.GetSize(); nAdjVertLoop > 0; --nAdjVertLoop)
		{
			pJoinBrush->ChangeVertexReferences(nAdjVertLoop - 1, nAdjVertLoop + nVertOffset - 1);
		}

		// Suck 'em over..
		pNewBrush->m_Points.AppendArray(pJoinBrush->m_Points);
		pNewBrush->m_Polies.AppendArray(pJoinBrush->m_Polies);

		// Point the polys in the right place
		for (uint32 nClearLoop = 0; nClearLoop < pJoinBrush->m_Polies.GetSize(); ++nClearLoop)
			pJoinBrush->m_Polies[nClearLoop]->m_pBrush = pNewBrush;

		// Clear out the old arrays
		pJoinBrush->m_Points.RemoveAll();
		pJoinBrush->m_Polies.RemoveAll();
	}

	// Skip out if there weren't actually any brushes...
	if (pNewBrush == 0)
		return;

	pNewBrush->UpdateBoundingInfo();

	// Delete the selected brushes
	DeleteSelectedNodes();

	// Geometry clean-up..
	pRegion->CleanupGeometry();

	// Select the new brush
	GetRegion()->SelectNode( pNewBrush->AsNode() );
	GetRegionDoc()->NotifySelectionChange();

	// Re-draw
	GetRegionDoc()->RedrawAllViews();
}

/************************************************************************/
// Handler for the un-join command
void CRegionView::OnBrushUnJoin() 
{
	if (GetEditMode() != BRUSH_EDITMODE)
		return;

	// Get the brush list from the selection list
	CMoArray<CEditBrush *> inBrushes;

	inBrushes.SetCacheSize(GetRegion()->m_Selections.GetSize());
	for (uint32 nFillInLoop = 0; nFillInLoop < GetRegion()->m_Selections.GetSize(); ++nFillInLoop)
	{
		CWorldNode *pNode = GetRegion()->m_Selections[nFillInLoop];
		if (pNode->GetType() == Node_Brush)
		{
			inBrushes.Append(pNode->AsBrush());
		}
	}

	// Empty?  Guess they don't want anything un-joined...
	if (!inBrushes.GetSize())
		return;

	// This might take a while.. Put the wait cursor on the screen
	BeginWaitCursor( );

	// Create an undo list
	PreActionList actionList;

	// Start un-joining
	for(uint32 nInBrushLoop = 0; nInBrushLoop < inBrushes.GetSize(); ++nInBrushLoop)
	{
		// Get our current brush..
		CEditBrush *pInBrush = inBrushes[nInBrushLoop];

		// Remember that it's going to be modified
		actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, inBrushes[nInBrushLoop]));

		// Go through the polygons..
		for (uint32 nInPolyLoop = 0; nInPolyLoop < pInBrush->m_Polies.GetSize(); ++nInPolyLoop)
		{
			CEditPoly *pInPoly = pInBrush->m_Polies[nInPolyLoop];

			// Create a new brush for this polygon
			CEditBrush *pOutBrush = no_CreateNewBrush(GetRegion(), pInBrush->GetParent());
			pOutBrush->m_PropList.CopyValues(&pInBrush->m_PropList);
			pOutBrush->SetName(pInBrush->GetName());

			// Create a new poly for the brush
			CEditPoly *pOutPoly = new CEditPoly(pInPoly);
			pOutPoly->m_pBrush = pOutBrush;

			// Copy the points of the current polygon
			for (uint32 nInPointLoop = 0; nInPointLoop < pInPoly->m_Indices.GetSize(); ++nInPointLoop)
			{
				pOutBrush->m_Points.Append(pInPoly->Pt(nInPointLoop));
				pOutPoly->m_Indices[nInPointLoop] = nInPointLoop;
			}

			// Add the poly to the brush
			pOutBrush->m_Polies.Append(pOutPoly);

			// Update the brush
			pOutBrush->UpdateBoundingInfo();

			// Add it to the undo list
			actionList.AddTail(new CPreAction(ACTION_ADDEDNODE, pOutBrush));

			// Select it
			GetRegion()->SelectNode(pOutBrush);
		}

		// Update the node view's list of the new brushes
		GetNodeView()->RecurseUpdateLabels(pInBrush->GetParent());
	}

	// Remember the undo
	GetRegionDoc()->Modify(&actionList, TRUE);

	// Delete the old brushes
	for (uint32 nDeleteLoop = 0; nDeleteLoop < inBrushes.GetSize(); ++nDeleteLoop)
	{
		GetNodeView( )->DeleteNode(inBrushes[nDeleteLoop]);
		no_DestroyNode(GetRegion(), inBrushes[nDeleteLoop], FALSE);
	}

	// Geometry clean-up..
	GetRegion()->CleanupGeometry();

	// Tell everyone we've been playing with the selections
	GetRegionDoc()->NotifySelectionChange();

	// Re-draw
	GetRegionDoc()->RedrawAllViews();

	// Ok, you're done waiting..
	EndWaitCursor( );
}

/************************************************************************/
// Handler for the flip command
void CRegionView::OnBrushFlip() 
{
	if (GetEditMode() != BRUSH_EDITMODE)
		return;

	CEditRegion *pRegion=GetRegion();

	// Make sure that we have one brush selected
	if((pRegion->GetNumSelections() < 1) || (pRegion->GetSelection(0)->GetType() != Node_Brush))
		return;

	DoFlipOperation();	
}

/************************************************************************/
// The command update handler for the flip command
void CRegionView::OnUpdateBrushFlip(CCmdUI* pCmdUI) 
{	
	// Get the edit region
	CEditRegion *pRegion=GetRegion();

	// Make sure that we have one brush selected
	if(pRegion->GetNumSelections() == 1 && pRegion->GetSelection(0)->GetType() == Node_Brush)
	{
		// Enable the menu item
		pCmdUI->Enable(TRUE);		
	}
	else
	{
		// Disable the menu item
		pCmdUI->Enable(FALSE);
	}

	SetMenuHotKeyText(UIE_FLIP_BRUSH, pCmdUI);
}


/************************************************************************/
// Performs a mirror along the X-Axis
void CRegionView::OnSelectionMirrorX() 
{
	MirrorSelectedNodes(CVector(1,0,0));
}

/************************************************************************/
// Command update handler for the mirror x-axis command
void CRegionView::OnUpdateSelectionMirrorX(CCmdUI* pCmdUI) 
{
	// Get the edit region
	CEditRegion *pRegion=GetRegion();

	// Make sure that there are selection nodes
	if (pRegion->GetNumSelections() > 0)
	{
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}

	SetMenuHotKeyText(UIE_MIRROR_X, pCmdUI);
}

/************************************************************************/
// Performs a mirror along the Y-Axis
void CRegionView::OnSelectionMirrorY() 
{
	MirrorSelectedNodes(CVector(0,1,0));
}

/************************************************************************/
// Command update handler for the mirror y-axis command
void CRegionView::OnUpdateSelectionMirrorY(CCmdUI* pCmdUI) 
{
	// Get the edit region
	CEditRegion *pRegion=GetRegion();

	// Make sure that there are selection nodes
	if (pRegion->GetNumSelections() > 0)
	{
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
	SetMenuHotKeyText(UIE_MIRROR_Y, pCmdUI);
}

/************************************************************************/
// Performs a mirror along the Z-Axis
void CRegionView::OnSelectionMirrorZ() 
{
	MirrorSelectedNodes(CVector(0,0,1));	
}

/************************************************************************/
// Command update handler for the mirror z-axis command
void CRegionView::OnUpdateSelectionMirrorZ(CCmdUI* pCmdUI) 
{
	// Get the edit region
	CEditRegion *pRegion=GetRegion();

	// Make sure that there are selection nodes
	if (pRegion->GetNumSelections() > 0)
	{
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
	SetMenuHotKeyText(UIE_MIRROR_Z, pCmdUI);
}

/************************************************************************/
// Centers the seleciton about the currently placed marker

void CRegionView::OnCenterSelectionOnMarker()
{
	CRegionDoc *pDoc = GetRegionDoc();
	CEditRegion *pRegion = GetRegion();
	if (!pRegion || !pDoc || (pRegion->GetNumSelections() == 0))
	{
		return;
	}

	//make sure the selection box is up to date
	pDoc->UpdateSelectionBox();

	//figure out where we are moving the lower corner to
	LTVector vMoveTo = pRegion->m_vMarker - (pDoc->m_SelectionMax - pDoc->m_SelectionMin) * 0.5f;

	//find a delta vector for that move
	LTVector vMoveAmount = vMoveTo - pDoc->m_SelectionMin;

	//offset these nodes
	OffsetSelectedNodes(vMoveAmount);

	// Update the views
	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdateCenterSelectionOnMarker(CCmdUI* pCmdUI)
{
	// Get the region doc
	CRegionDoc *pDoc=GetRegionDoc();
	if (!pDoc)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	// Make sure that there are selected objects
	if (pRegion->GetNumSelections() < 1)
	{
		pCmdUI->Enable(FALSE);
	}

	SetMenuHotKeyText(UIE_CENTER_SEL_ON_MARKER, pCmdUI);
}

/************************************************************************/
// Centers the marker on the selected brushes or objects
void CRegionView::OnCenterMarkerOnSelection() 
{
	CRegionDoc *pDoc=GetRegionDoc();
	CEditRegion *pRegion=GetRegion();
	if (!pRegion || !pDoc || (pRegion->GetNumSelections() == 0))
	{
		return;
	}

	//make sure the selection box is up to date
	pDoc->UpdateSelectionBox();

	// Place the marker	
	pRegion->m_vMarker = pDoc->m_SelectionMin + ((pDoc->m_SelectionMax - pDoc->m_SelectionMin) / 2.0f);

	// Update the views
	GetRegionDoc()->RedrawAllViews();
}

/************************************************************************/
// Command update handler for the "center marker on selection" command
void CRegionView::OnUpdateCenterMarkerOnSelection(CCmdUI* pCmdUI) 
{
	// Get the region doc
	CRegionDoc *pDoc=GetRegionDoc();
	if (!pDoc)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	// Make sure that there are selected objects
	if (pRegion->GetNumSelections() < 1)
	{
		pCmdUI->Enable(FALSE);
	}

	SetMenuHotKeyText(UIE_MARKER_CENTER, pCmdUI);
}

/************************************************************************/
// Places the marker at the camera position in a perspective view
void CRegionView::OnPlaceMarkerAtCamera() 
{
	// This should only be called in a perspective view
	ASSERT(IsPerspectiveViewType());

	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}

	// Move the marker location
	pRegion->m_vMarker=Nav().Pos();

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();
}

/************************************************************************/
// Places the marker at a specific location
void CRegionView::OnPlaceMarkerAtVector() 
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}

	// The vector editing dialog
	CVectorEdit vectorEditDlg;
	
	// Set the dialog to the current marker position
	vectorEditDlg.SetVector(pRegion->m_vMarker);

	// Display the dialog
	if (vectorEditDlg.DoModal() == IDOK)
	{
		// Move the marker
		pRegion->m_vMarker=vectorEditDlg.GetVector();

		// Redraw the views
		GetRegionDoc()->RedrawAllViews();
	}
}

/************************************************************************/
// Command update handler for the "place marker at camera" command
void CRegionView::OnUpdatePlaceMarkerAtCamera(CCmdUI* pCmdUI) 
{
	// Enable this command if we are in a perspective view
	pCmdUI->Enable(IsPerspectiveViewType());
}

/************************************************************************/
// Generates unique names for the selected objects
//
// bUpdateRefProps		- Updates properties for objects that reference
//						  other objects that have had their names change
// bUpdateSelPropsOnly	- Only objects that are selected will have their
//						  properties updated
// bDisplayReport		- A dialog reporting the name changes is displayed
// bGenerateUndoInfo	- Undo information is generated and added to the document
void CRegionView::GenerateUniqueNamesForSelected(BOOL bUpdateRefProps, BOOL bUpdateSelPropsOnly, BOOL bDisplayReport, BOOL bGenerateUndoInfo)
{
	// Get the region doc
	CRegionDoc *pDoc=GetRegionDoc();
	ASSERT(pDoc);

	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}
	
	// Show the wait cursor
	BeginWaitCursor();

	// Generate undo information if needed
	if (bGenerateUndoInfo)
	{
		// The undo list
		PreActionList objectNameUndoInfo;				

		// Generate the undo information
		pRegion->GenerateUniqueNamesForSelected(NULL, NULL, &objectNameUndoInfo, TRUE /* Store undo information only */);

		// Store the undo information
		pDoc->Modify(&objectNameUndoInfo, TRUE);
	}

	// Generate unique names for the objects
	CStringArray originalNameArray;
	CStringArray updatedNameArray;
	pRegion->GenerateUniqueNamesForSelected(&originalNameArray, &updatedNameArray);

	// Stores the property names that are changed and their original/updated values
	CStringArray propertyNameArray;
	CStringArray propertyOriginalValueArray;
	CStringArray propertyUpdatedValueArray;

	// Update the referencing properties for the objects
	if (bUpdateRefProps)
	{
		// Store the undo information
		if (bGenerateUndoInfo)
		{
			PreActionList propertyUndoInfo;

			// Start out by storing the undo information
			int i;
			for (i=0; i < originalNameArray.GetSize(); i++)
			{
				pRegion->UpdateObjectsReferenceProps(bUpdateSelPropsOnly, originalNameArray[i], updatedNameArray[i],
													 NULL, NULL, NULL, &propertyUndoInfo, TRUE /* Store undo information only */);
			}

			// Store the undo information
			pDoc->Modify(&propertyUndoInfo, TRUE);
		}

		// Update the object properties
		int i;
		for (i=0; i < originalNameArray.GetSize(); i++)
		{
			pRegion->UpdateObjectsReferenceProps(bUpdateSelPropsOnly, originalNameArray[i], updatedNameArray[i],
												 &propertyNameArray, &propertyOriginalValueArray, &propertyUpdatedValueArray);
		}
	}		

	// Stop showing the wait cursor
	EndWaitCursor();

	// Display a report if necessary
	if (bDisplayReport && originalNameArray.GetSize() > 0)
	{		
		CNameChangeReportDlg nameChangeReportDlg;
		nameChangeReportDlg.SetObjectNameArrays(&originalNameArray, &updatedNameArray);
		nameChangeReportDlg.SetPropertyNameArrays(&propertyNameArray, &propertyOriginalValueArray, &propertyUpdatedValueArray);

		// Display the dialog
		nameChangeReportDlg.DoModal();
	}
}

/************************************************************************/
// Generates unique names for the selected nodes
void CRegionView::OnSelectionGenerateUniqueNames() 
{	
	// Construct the dialog
	COptionsGenerateUniqueNames *pOptions=GetApp()->GetOptions().GetGenerateUniqueNamesOptions();

	CGenerateUniqueNamesDlg uniqueNamesDlg;
	uniqueNamesDlg.m_bUpdateRefProps=pOptions->GetUpdateRefProps();
	uniqueNamesDlg.m_bUpdateSelPropsOnly=pOptions->GetUpdateSelPropsOnly();
	uniqueNamesDlg.m_bDisplayReportOfChanges=pOptions->GetDisplayReportOfChanges();

	// Display the "Generate Unique Names" dialog
	if (uniqueNamesDlg.DoModal() == IDOK)
	{
		// Save the options
		pOptions->SetUpdateRefProps(uniqueNamesDlg.m_bUpdateRefProps);
		pOptions->SetUpdateSelPropsOnly(uniqueNamesDlg.m_bUpdateSelPropsOnly);
		pOptions->SetDisplayReportOfChanges(uniqueNamesDlg.m_bDisplayReportOfChanges);

		// Generate the unique names
		GenerateUniqueNamesForSelected(uniqueNamesDlg.m_bUpdateRefProps, uniqueNamesDlg.m_bUpdateSelPropsOnly, uniqueNamesDlg.m_bDisplayReportOfChanges);
				
		// Update the properties dialog
		GetRegionDoc()->SetupPropertiesDlg(FALSE);

		// Redraw all of the views
		GetRegionDoc()->RedrawAllViews();				
	}
}

/************************************************************************/
// Command update handler for the "generate unique names" command
void CRegionView::OnUpdateSelectionGenerateUniqueNames(CCmdUI* pCmdUI) 
{	
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	// Make sure that there are selected objects
	if (pRegion->GetNumSelections() < 1)
	{
		pCmdUI->Enable(FALSE);
	}	

	SetMenuHotKeyText(UIE_GENERATE_UNIQUE_NAMES, pCmdUI);
}

/************************************************************************/
// Save the selected nodes as a prefab file
void CRegionView::OnSelectionSavePrefab() 
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{		
		return;
	}

	// Make sure that we have selected nodes
	if (pRegion->GetNumSelections() < 1)
	{
		return;
	}

	CString sFileName;
	sFileName = GetProject()->m_BaseProjectDir + "\\Prefab\\*.lta;*.ltc;*.tbw";
	// Let the user choose the file to save to
	CFileDialog fileDlg(FALSE, "lta", (LPCTSTR)sFileName, OFN_OVERWRITEPROMPT, "DEdit World (*.lta;*.ltc;*.tbw)|*.lta;*.ltc;*.tbw|All Files (*.*)|*.*||");

	// Display the dialog
	if (fileDlg.DoModal() != IDOK)
	{
		return;
	}

	// Create the region
	CEditRegion region;	

	// Don't check nodes in debug because it takes a very long time
	BOOL bCheckNodes=g_bCheckNodes;
	g_bCheckNodes=FALSE;

	// Paste the selected nodes into the new region
	PreActionList actionList;
	((CMainFrame *)AfxGetMainWnd( ))->GetClipboard( )->PasteNodes(&region, GetRegion()->m_Selections, actionList, FALSE);

	// Clear the action list
	GDeleteAndRemoveElements(actionList);

	// Move the region to be relative to the marker
	region.OffsetSelectedNodes(-(GetRegion()->m_vMarker));

	// Clear the selections in the new region
	region.ClearSelections();

	// Reset this parameter
	g_bCheckNodes=bCheckNodes;

	// Open the file
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath( LPCTSTR(fileDlg.GetPathName()), NULL, dir, fname, ext );

	bool bBinary = false;

	if(stricmp(ext, ".tbw") == 0)
		bBinary = true;

	if(bBinary)
	{
		CMoFileIO OutFile;

		if( OutFile.Open(fileDlg.GetPathName(), "wb") )
		{
			region.SaveTBW(OutFile);
			OutFile.Close();
		}
		else
		{
			MessageBox("Error: Unable to open file", "Error", MB_OK);
		}
	}
	else
	{
		CLTAFile OutFile(fileDlg.GetPathName(), false, CLTAUtil::IsFileCompressed(fileDlg.GetPathName()));

		if( OutFile.IsValid() )
		{
			region.SaveLTA(&OutFile);
			OutFile.Close();
		}
		else
		{
			MessageBox("Error: Unable to open file", "Error", MB_OK);
		}
	}

	// Update the project bar
	GetProjectBar()->UpdateAll();	
}

/************************************************************************/
// Command update handler for the "Save Prefab" option
void CRegionView::OnUpdateSelectionSavePrefab(CCmdUI* pCmdUI) 
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	// Make sure that there are selected objects
	if (pRegion->GetNumSelections() < 1)
	{
		pCmdUI->Enable(FALSE);
	}		

	SetMenuHotKeyText(UIE_SAVE_AS_PREFAB, pCmdUI);
}


/************************************************************************/
// Scale the selected brushes by given amounts along the axes
void CRegionView::OnSelectionScale() 
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{		
		return;
	}

	// Make sure that we have selected nodes
	if (pRegion->GetNumSelections() < 1)
	{
		return;
	}

// Show the scale selected dialog

	CVector SelectionSize;
	CScaleSelectDlg dlg;

	 // Get the document
	CRegionDoc *pDoc=GetRegionDoc();

	if (!pDoc)  return;

	SelectionSize = pDoc->m_SelectionMax - pDoc->m_SelectionMin;

	dlg.SetValues(&SelectionSize);

	if(dlg.DoModal() == IDOK)
	{
				
	// Show the wait cursor
	BeginWaitCursor();

	CVector		scaleAmt(1.0f, 1.0f, 1.0f);	
	uint32		i, j;

	CVector		startVal = pDoc->m_SelectionMax - pDoc->m_SelectionMin;
	CVector		originVal = (pDoc->m_SelectionMax + pDoc->m_SelectionMin)/CVector(2.0f, 2.0f, 2.0f);

	CEditBrush	 *pBrush;
	CBaseEditObj *pObject;
	CVector		*pVec;
	CWorldNode	*pNode;

	CVector O, P, Q; // texture coords for polies
	CVector unitP, unitQ, cur;
	int dot;

	CEditRegion *pRegion = GetRegion();


	pDoc->SetupUndoForSelections();

	// Get the amount to scale.
	if (startVal.x != 0.0f)
	{
		scaleAmt.x = SelectionSize.x / startVal.x;	
	}
	if (startVal.y != 0.0f)
	{
		scaleAmt.y = SelectionSize.y / startVal.y;	
	}
	if (startVal.z != 0.0f)
	{
		scaleAmt.z = SelectionSize.z / startVal.z;	
	}		

	// Don't allow scaling to a zero width
	if (!scaleAmt.x || !scaleAmt.y || !scaleAmt.z)
	{
		pDoc->UpdateSelectionBox();
		return;
	}

	// Determine if the polygon needs to be flipped
	BOOL bFlip=FALSE;
	if (scaleAmt.x < 0.0f)
	{	
		bFlip=!bFlip;
	}
	if (scaleAmt.y < 0.0f)
	{	
		bFlip=!bFlip;
	}
	if (scaleAmt.z < 0.0f)
	{	
		bFlip=!bFlip;
	}

	for( i=0; i < pRegion->m_Selections; i++ )
	{
		pNode = pRegion->m_Selections[i];

		if(pNode->GetType() == Node_Brush)
		{
			pBrush = pNode->AsBrush();

			if (bFlip)
			{
				pBrush->FlipNormals();
			}

			for( j=0; j < pBrush->m_Points; j++ )
			{
				pVec = &pBrush->m_Points[j];

				(*pVec) -= originVal;
				(*pVec).x *= scaleAmt.x;
				(*pVec).y *= scaleAmt.y;
				(*pVec).z *= scaleAmt.z;
				(*pVec) += originVal;
			}

			if(dlg.m_bKeepTextures) 
			{  // if they want to keep texture alignments, we must adjust
				for( j=0; j < pBrush->m_Polies; j++ )
				{
					for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
					{
						pBrush->m_Polies[j]->GetTexture(nCurrTex).GetTextureSpace(O, P, Q);

						// Adjust Texture OPQs to scale with the geometry

						O -= originVal;
						O.x *= scaleAmt.x;
						O.y *= scaleAmt.y;
						O.z *= scaleAmt.z;
						O += originVal;
						
						P.x /= scaleAmt.x;
						P.y /= scaleAmt.y;
						P.z /= scaleAmt.z;
						Q.x /= scaleAmt.x;
						Q.y /= scaleAmt.y;
						Q.z /= scaleAmt.z;

						pBrush->m_Polies[j]->SetTextureSpace(nCurrTex, O, P, Q);
					}
				}
			}

			GetRegion()->UpdateBrushGeometry(pBrush);

		}
		else if((pNode->GetType() == Node_Object) || (pNode->GetType() == Node_PrefabRef))
		{
			LTVector vVec = pNode->GetPos();

			vVec -= originVal;
			vVec.x *= scaleAmt.x;
			vVec.y *= scaleAmt.y;
			vVec.z *= scaleAmt.z;
			vVec += originVal;
			pNode->SetPos(vVec);

			if (pNode->GetType() == Node_Object)
			{
				pObject = pNode->AsObject();

				if (pObject->GetNumDims() != 0) // has dims, so update the object
				{
					LTVector* dimV = pObject->GetDim(0);

					dimV->x *= scaleAmt.x;
					dimV->y *= scaleAmt.y;
					dimV->z *= scaleAmt.z;

					// update the brush properties as well

					char* tempchar;

					if (pObject->GetPropertyList() != NULL)
					{
						for (int j=0; j < pObject->GetPropertyList()->GetSize(); j++) 
						{
							if (strcmp("Dims", pObject->GetPropertyList()->GetAt(j)->GetName()) == 0) 
							{
								((CVectorProp*)pObject->GetPropertyList()->GetAt(j))->SetVector(*dimV);
								break;
							}
						}
					}
				}
			}
		}
	}

	// Recalculate plane equations for all planes...
	pRegion->UpdatePlanes();
	pRegion->CleanupGeometry( );

	pDoc->UpdateSelectionBox();

	// Update properties...
	pDoc->SetupPropertiesDlg( FALSE );

	//MessageBox(NULL, "Object resize completed", "Debug success", MB_OK);

	return;

			
	EndWaitCursor();
	}	

	// Update the project bar
	GetProjectBar()->UpdateAll();	
}
/************************************************************************/
// Command update handler for the selection scaling option
void CRegionView::OnUpdateSelectionScale(CCmdUI* pCmdUI) 
{

	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		pCmdUI->Enable(FALSE);
		return;
	}

	// Make sure that there are selected objects
	if (pRegion->GetNumSelections() < 1)
	{
		pCmdUI->Enable(FALSE);
	}		

	SetMenuHotKeyText(UIE_SCALE_SELECTION, pCmdUI);
}

/************************************************************************/
// This command opens up the object browser
void CRegionView::OnWorldObjectBrowser() 
{	
	// Get the region doc
	CRegionDoc *pDoc=GetRegionDoc();
	ASSERT(pDoc);

	// Get the region
	CEditRegion *pRegion=GetRegion();
	ASSERT(pRegion);

	// Create the object browser dialog
	CObjectBrowserDlg objectBrowserDlg;

	// Search the selected items and see if there is an object selected.
	// If there is, select it in the object browser
	int i;
	for (i=0; i < pRegion->GetNumSelections(); i++)
	{
		// Get the node
		CWorldNode *pNode=pRegion->GetSelection(i);

		// Check to see if the node is an object
		if (pNode->GetType() == Node_Object)
		{
			// Select the node in the browser
			objectBrowserDlg.SetSelectedObject(pNode->GetName());
			break;
		}
	}

	// Set whether or not to group the objects by type
	COptionsObjectBrowser *pOptions=GetApp()->GetOptions().GetObjectBrowserOptions();
	if (pOptions)
	{
		objectBrowserDlg.m_bGroupByType=pOptions->IsGroupByType();
	}

	// Display the browser dialog
	if (objectBrowserDlg.DoModal() == IDOK)
	{
		// Save whether or not to group the objects by type		
		if (pOptions)
		{
			pOptions->SetGroupByType(objectBrowserDlg.m_bGroupByType);
		}
	}
}

/************************************************************************/
// This function selects the nodes that have naming conflicts with
// other nodes in the world.
void CRegionView::OnWorldDebugFindNamingConflicts() 
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}

	// Display the wait cursor
	BeginWaitCursor();

	// Turn off the drawing of the node view
	if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
	{
		GetNodeView()->SetRedraw(FALSE);
	}

	// Clear the selections
	pRegion->ClearSelections();

	// Go through each object and see if they are unique
	int i;
	for (i=0; i < pRegion->m_Objects.GetSize(); i++)
	{
		// Get the object
		CBaseEditObj *pObject=pRegion->m_Objects[i];

		// See if the object is unique
		if (pRegion->FindObjectsByName(pObject->GetName(), NULL, 2) > 1)
		{
			// Select the object
			pRegion->SelectNode(pObject);

			// Make sure that the node is visible in the node view
			GetNodeView()->HighlightNode(pObject);
		}
	}

	// Turn off the drawing of the node view
	if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
	{
		GetNodeView()->SetRedraw(TRUE);
		GetNodeView()->Invalidate();
	}

	GetRegionDoc()->NotifySelectionChange();
	GetRegionDoc()->RedrawAllViews();

	// Stop displaying the wait cursor
	EndWaitCursor();
}


/************************************************************************/
// This function scales the world by an input amount
void CRegionView::OnWorldScaleGeometry()
{
	// Get the scaling factor from the user
	CStringDlg dlg;
	dlg.m_bAllowNumbers = TRUE;
	dlg.m_bAllowOthers = TRUE;
		
	float fScaleFactor = 0.0f;

	while (dlg.DoModal(IDS_SCALE_WORLD, IDS_SCALE_ENTER_VALUE) == IDOK)
	{
		fScaleFactor = (float)atof((LPCTSTR)dlg.m_EnteredText);
		if (!fScaleFactor)
			MessageBox("Please enter a non-zero floating point value.", "Invalid Value");
		else
			break;			
	}
	if (!fScaleFactor || (fScaleFactor == 1.0f))
		return;

	// Display the wait cursor
	BeginWaitCursor();

	// Flag the world as modified
	GetRegionDoc()->Modify();

	// Scale the world
	GetRegion()->ScaleBy(fScaleFactor);

	// Display the wait cursor
	EndWaitCursor();

	// Refresh the selection cube
	GetRegionDoc()->UpdateSelectionBox();
	// Update the properties dialog
	GetRegionDoc()->SetupPropertiesDlg( FALSE );

	// Redraw all of the views
	GetRegionDoc()->RedrawAllViews();
}


//given a list of polygons, it will back up all the nodes associated with them
//and set up the undo
void CRegionView::MakeUndoForPolygons(CMoArray<CEditPoly*>* pPolyList)
{
	ASSERT(pPolyList);

	// Setup the undo information 
	PreActionList actionList;	

	for(uint32 i=0; i < pPolyList->GetSize(); i++)
	{
		bool bMakeUndo = true;

		//make sure another poly doesn't already have this brush
		for(uint32 j = 0; j < i; j++)
		{
			if((*pPolyList)[j]->m_pBrush == (*pPolyList)[i]->m_pBrush)
			{
				bMakeUndo = false;
				break;
			}
		}

		if(bMakeUndo)
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, (*pPolyList)[i]->m_pBrush));

	}

	//register the undo
	GetRegionDoc()->Modify(&actionList, TRUE);
}


void CRegionView::OnMapTextureToView()
{
	DVector O, P, Q;
	DWORD i;
	CMoArray<CEditPoly*> polies;

	O.Init();
	P = ViewDef()->m_Nav.Right();
	Q = -ViewDef()->m_Nav.Up();

	polies.SetCacheSize(256);
	GetSelectedPolies(polies);

	MakeUndoForPolygons(&polies);


	//now that the undo is done, actually do the operation
	for(i = 0; i < polies.GetSize(); i++)
	{
		polies[i]->SetTextureSpace(GetCurrTexture(), O, P, Q);
	}

	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdateMapTextureToView(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(AreAnyPoliesSelected());
	SetMenuHotKeyText(UIE_MAP_TEXTURE_TO_VIEW, pCmdUI);
}



void CRegionView::OnResetTextureCoords()
{
	// Setup the undo information 
	PreActionList actionList;	

	if((GetEditMode() == BRUSH_EDITMODE) || (GetEditMode() == GEOMETRY_EDITMODE))
	{
		DWORD i;
		CMoArray<CEditPoly*> polies;

		polies.SetCacheSize(256);
		GetSelectedPolies(polies);

		CEditPoly *pPoly;

		MakeUndoForPolygons(&polies);

		// If using brush mode
		if (GetEditMode() == BRUSH_EDITMODE)
		{
			// Use cubic mapping
			for(i=0; i < polies; i++)
			{
				polies[i]->SetupBaseTextureSpace(GetCurrTexture());
				polies[i]->GetTexture(GetCurrTexture()).SetO(LTVector(0.0f, 0.0f, 0.0f));
			}
		}
		else if (GetEditMode() == GEOMETRY_EDITMODE)
		{
			// If only one polygon is selected..
			if (polies.GetSize() < 2)
			{
				// Use the polygon orientations as the texture space
				pPoly = polies[0];
				CTexturedPlane& PolyTex = pPoly->GetTexture(GetCurrTexture());

				LTVector vP = pPoly->Pt(0) - pPoly->Pt(1);
				vP.Norm();

				LTVector vQ = vP.Cross(pPoly->Normal());
				vQ.Norm();

				PolyTex.SetO(pPoly->Pt(0));
				PolyTex.SetP(vP);
				PolyTex.SetQ(vQ);
			}
			else
			{
				// Otherwise use the texture space from the first polygon
				CVector cVectorO, cVectorP, cVectorQ;
				pPoly = polies[0];
				CTexturedPlane& PolyTex = pPoly->GetTexture(GetCurrTexture());

				cVectorO = PolyTex.GetO();
				cVectorP = PolyTex.GetP();
				cVectorQ = PolyTex.GetQ();
				for (i = 1; i < polies.GetSize(); i++)
				{
					pPoly = polies[i];
					pPoly->GetTexture(GetCurrTexture()).SetO(cVectorO);
					pPoly->GetTexture(GetCurrTexture()).SetP(cVectorP);
					pPoly->GetTexture(GetCurrTexture()).SetQ(cVectorQ);
				}
			}
		}
	}

	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdateResetTextureCoords(CCmdUI* pCmdUI)
{
	BOOL bEnable = AreAnyPoliesSelected();

	pCmdUI->Enable(bEnable);
	SetMenuHotKeyText(UIE_RESET_TEXTURE_COORDS, pCmdUI);
}

BOOL CRegionView::OnDepthSelect(CUITracker *pTracker)
{
	CRVCallback *pRVCTracker = (CRVCallback *)pTracker;

	BOOL bResult = TRUE;

	if( GetEditMode() == GEOMETRY_EDITMODE )
	{
		// Toggle the poly tag.
		if(IPoly().IsValid())
		{
			BOOL bFound = FALSE;
			for(DWORD i=1; i < TaggedPolies(); i++)
			{
				if(TaggedPolies()[i] == IPoly())
				{
					TaggedPolies().Remove(i);
					bFound = TRUE;
					break;
				}
			}

			if(!bFound)
			{						
				TaggedPolies().Add(IPoly());
			}

			GetRegionDoc()->RedrawAllViews();
		}
	}
	else if(GetEditMode() == BRUSH_EDITMODE)
	{	
		CReal dist;

		// Subtree or single toggle the current brush.
		CBrushRef brushRef = GetMouseOverBrush( &dist, m_rSelectDist );
		if( !brushRef.IsValid() && m_rSelectDist > 0.0f )
		{
			// Start at beginning and try again...
			m_rSelectDist = 0.0f;
			brushRef = GetMouseOverBrush( &dist, m_rSelectDist );
		}
		  
		if( brushRef.IsValid( ))
		{
			m_rSelectDist = dist;
			GetNodeView( )->m_NodeViewTree.SelectTree( brushRef(), pRVCTracker->GetToggleState(0));
//					GetRegionDoc()->NotifySelectionChange();
		}
	}			
	else if( GetEditMode() == OBJECT_EDITMODE )
	{
		// Setup the closest object to be selected and in the property dialog.
		CWorldNode* pObj = GetClosestObject( GetCurMousePos(), FALSE );
		if( pObj )
		{
			GetNodeView( )->m_NodeViewTree.SelectTree( pObj, pRVCTracker->GetToggleState(0));
		}

		GetRegionDoc()->NotifySelectionChange();
	}
	else
		bResult = FALSE;

	return bResult;
}

BOOL CRegionView::OnApplyTextureClick(CUITracker *pTracker)
{
	CRVCallback *pRVCTracker = (CRVCallback *)pTracker;

	BOOL bResult = TRUE;

	if( GetEditMode() == BRUSH_EDITMODE )
	{
		// Apply the currently selected texture to the brush that the user is clicking with the mouse
		CReal dist;
		
		// Get the brush that the user is clicking on
		CBrushRef brushRef = GetMouseOverBrush( &dist, m_rSelectDist );
		if( !brushRef.IsValid() && m_rSelectDist > 0.0f )
		{
			// Start at beginning and try again...
			m_rSelectDist = 0.0f;
			brushRef = GetMouseOverBrush( &dist, m_rSelectDist );
		}

		if( brushRef.IsValid( ))
		{
			// Select the brush
			m_rSelectDist = dist;
			GetNodeView( )->m_NodeViewTree.SelectTree( brushRef(), pRVCTracker->GetToggleState(0));

			// Apply the texture
			OnApplyTexture();
		}
	}
	else
		bResult = FALSE;

	return bResult;
}

BOOL CRegionView::OnSetGridOrientation(CUITracker *pTracker)
{
	if (!IsPerspectiveViewType())
		return FALSE;

	CRVCallback *pRVCTracker = (CRVCallback *)pTracker;

	switch (pRVCTracker->GetData())
	{
		case GRID_FORWARD : 
		{
			EditGrid().Pos().Init( 0.0f, 0.0f, 0.0f );
			EditGrid().Forward().Init( 0.0f, 1.0f, 0.0f );
			EditGrid().Up().Init( 0.0f, 0.0f, 1.0f );
			EditGrid().MakeRight();
			break;
		}

		case GRID_RIGHT :
		{
			EditGrid().Pos().Init( 0.0f, 0.0f, 0.0f );
			EditGrid().Forward().Init( 1.0f, 0.0f, 0.0f );
			EditGrid().Up().Init( 0.0f, 0.0f, 1.0f );
			EditGrid().MakeRight();
			break;
		}
	
		case GRID_UP :
		{
			EditGrid().Pos().Init( 0.0f, 0.0f, 0.0f );
			EditGrid().Forward().Init( 0.0f, 0.0f, 1.0f );
			EditGrid().Up().Init( 0.0f, 1.0f, 0.0f );
			EditGrid().MakeRight();
			break;
		}
	}

	//refresh the views
	GetRegionDoc()->RedrawAllViews();

	return TRUE;
}

BOOL CRegionView::OnInitTextureSpace(CUITracker *pTracker)
{
	BOOL bResult = TRUE;

	if ((GetEditMode() == BRUSH_EDITMODE) && (GetRegion()->m_Selections.GetSize()))
	{
		DWORD i, k;

		CWorldNode *pNode;
		CEditBrush *pBrush;

		//first setup an undo for all the brushes
		PreActionList actionList;

		for(i=0; i < GetRegion()->m_Selections; i++)
		{
			pNode = GetRegion()->m_Selections[i];
			if(pNode->GetType() == Node_Brush)
			{
				actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pNode));
			}
		}

		GetRegionDoc()->Modify(&actionList, TRUE);

		// InitTextureSpace() on the selected brushes' polies.
		for( i=0; i < GetRegion()->m_Selections; i++ )
		{
			pNode = GetRegion()->m_Selections[i];

			if(pNode->GetType() == Node_Brush)
			{
				pBrush = pNode->AsBrush();
				
				for( k=0; k < pBrush->m_Polies; k++ )
					pBrush->m_Polies[k]->SetupBaseTextureSpace(GetCurrTexture());
			}
		}
		GetRegionDoc()->RedrawPerspectiveViews();
	}					
	else if ((GetEditMode() == GEOMETRY_EDITMODE) && (IPoly().IsValid()))
	{
		// InitTextureSpace() on the immediate polygon.
		GetRegionDoc()->Modify(new CPreAction(ACTION_MODIFYNODE, IPoly().m_pBrush), TRUE);
		
		CEditPoly *pPoly = IPoly()();

		// Make sure that the current vertex index is within range
		if (m_nCurrentTextureVertIndex >= pPoly->m_Indices.GetSize())
		{
			m_nCurrentTextureVertIndex=0;
		}

		DWORD index=m_nCurrentTextureVertIndex;
		m_nCurrentTextureVertIndex++;					

		if(index != BAD_INDEX )
		{
			CTexturedPlane& PolyTex = pPoly->GetTexture(GetCurrTexture());

			CVector O = PolyTex.GetO();
			CVector P = PolyTex.GetP();
			CVector Q = PolyTex.GetQ();

			//determine if we need to flip the vectors in order to preserve
			//any mirroring on the texture. This is done by finding the expected
			//vector, and seeing if it matches up to the specified vector direction
			//wise. If it doesn't, we know we need to mirror it when we are done
			float fPDot = (Q.Cross(pPoly->Normal())).Dot(P);
			float fQDot = (P.Cross(pPoly->Normal())).Dot(Q);

			bool bMirrorP = (fPDot > 0.0f);
			bool bMirrorQ = bMirrorP ? (fQDot < 0.0f) : (fQDot > 0.0f);
			
			float pScale = (bMirrorP) ? -P.Mag() : P.Mag();
			float qScale = (bMirrorQ) ? -Q.Mag() : Q.Mag();

			O = pPoly->Pt(index);
			P = pPoly->NextPt(index) - O;
			Q = pPoly->Normal().Cross(P);

			P.Norm();
			P *= pScale;

			Q.Norm();
			Q *= qScale;

			pPoly->SetTextureSpace(GetCurrTexture(), O, P, Q);
		}
		
		GetRegionDoc()->RedrawPerspectiveViews();
	}					
	else
		bResult = FALSE;

	return bResult;
}

BOOL CRegionView::OnSelectNone(CUITracker *pTracker)
{
	BOOL bResult = TRUE;

	switch (GetEditMode())
	{
		case BRUSH_EDITMODE :
		case OBJECT_EDITMODE :
		{
			GetRegion()->ClearSelections();
			GetRegionDoc()->NotifySelectionChange();
			GetRegionDoc()->RedrawAllViews();
			break;
		}
		case GEOMETRY_EDITMODE :
		{
			// Untag all vertices.
			TaggedVerts().GenRemoveAll();
			ClearSelections();
			GetRegionDoc()->RedrawAllViews();
			break;
		}
		default :
			bResult = FALSE;
			break;
	}

	if (bResult) 
	{
		// Sets the dirty bit for the file.  Unnecessary if the undo code is included
		GetRegionDoc()->SetModifiedFlag( TRUE );
		GetRegionDoc()->SetTitle(true);
	}

	return bResult;
}

BOOL CRegionView::OnDelete(CUITracker *pTracker)
{
	BOOL bResult = TRUE;

	switch (GetEditMode())
	{
		case BRUSH_EDITMODE :
		case OBJECT_EDITMODE :
			DeleteSelectedNodes();
			break;
		case GEOMETRY_EDITMODE :
			DeleteTaggedVertices();
			break;
		default :
			bResult = FALSE;
	}

	return bResult;
}

BOOL CRegionView::OnNudge(CUITracker *pTracker)
{
	CRVCallback *pRVCTracker = (CRVCallback *)pTracker;

	BOOL bResult = TRUE;

	switch (GetEditMode())
	{
		case BRUSH_EDITMODE :
		case OBJECT_EDITMODE :
		{
			bResult = GetRegion()->m_Selections.GetSize() != 0;
			float fDistance = (pRVCTracker->GetToggleState(1)) ? 1.0f : (float)m_dwGridSpacing;
			switch (pRVCTracker->GetData())
			{
				case VK_LEFT:
					OffsetSelectedNodes(ViewDef()->m_Nav.m_Right*(float)fDistance*(-1));				
					break;
				case VK_RIGHT:
					OffsetSelectedNodes(ViewDef()->m_Nav.m_Right*(float)fDistance);
					break;
				case VK_UP:
					OffsetSelectedNodes(ViewDef()->m_Nav.m_Up*(float)fDistance);
					break;
				case VK_DOWN:
					OffsetSelectedNodes(ViewDef()->m_Nav.m_Up*(float)fDistance*(-1));
					break;
				default :
					bResult = FALSE;
					break;
			}
			// Update properties dialog...
			m_pView->GetRegionDoc()->SetupPropertiesDlg( FALSE );
			break;
		}
		case GEOMETRY_EDITMODE :
		{
			if (pRVCTracker->GetToggleState(0))
			{
				// Scale the texture
				switch (pRVCTracker->GetData())
				{
					case VK_LEFT:
						ScaleTextureForPoly(1 /* P direction */, 0 /* Q direction */);
						break;
					case VK_RIGHT:
						ScaleTextureForPoly(-1 /* P direction */, 0 /* Q direction */ );
						break;
					case VK_UP:
						ScaleTextureForPoly(0 /* P direction */, -1 /* Q direction */ );
						break;
					case VK_DOWN:
						ScaleTextureForPoly(0 /* P direction */,  1 /* Q direction */ );
						break;
					default :
						bResult = FALSE;
						break;
				}
			}
			else
			{
				BOOL bGrid = pRVCTracker->GetToggleState(1);
				// Move the texture
				switch (pRVCTracker->GetData())
				{
					case VK_LEFT:
						OffsetTextureForPoly(-1 /* P direction */, 0 /* Q direction */, bGrid );
						break;
					case VK_RIGHT:
						OffsetTextureForPoly(1 /* P direction */, 0 /* Q direction */, bGrid );
						break;
					case VK_UP:
						OffsetTextureForPoly(0 /* P direction */, -1 /* Q direction */, bGrid );
						break;
					case VK_DOWN:
						OffsetTextureForPoly(0 /* P direction */, 1 /* Q direction */, bGrid );
						break;
					default :
						bResult = FALSE;
						break;
				}
			}
			break;
		}
		default :
			bResult = FALSE;
			break;
	}

	return bResult;
}

BOOL CRegionView::OnNudgeRotate(CUITracker *pTracker)
{
	CRVCallback *pRVCTracker = (CRVCallback *)pTracker;

	CRotateSelection dlg;
	CReal rRotation;
	CEditRegion *pRegion;
	CMatrix matRot;
	CVector vRotationVector, vRotationCenter;
	DWORD i;
	PreActionList actionList;
	CLinkedList<CEditBrush*> brushList;

	pRegion = GetRegion( );
	ASSERT( pRegion );

	if( !pRegion->m_Selections.GetSize( ))
		return FALSE;

	// Rotate...
	BeginWaitCursor( );

	if (pRVCTracker->GetData() == VK_LEFT)
		rRotation = 45 * MATH_PI / 180.0f;
	else
		rRotation = -45 * MATH_PI / 180.0f;

	// Set up the rotation matrix
	vRotationVector = m_pViewDef->m_Nav.Forward( );
	vRotationVector.Norm( );
	vRotationCenter = GetRegion()->m_vMarker;
	gr_SetupRotationAroundVector(&matRot, vRotationVector, rRotation);

	// Setup undos...
	for(i=0; i < pRegion->m_Selections; i++)
		actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pRegion->m_Selections[i]));
		
	GetRegionDoc()->Modify(&actionList, TRUE);

	// Loop over all selected nodes...
	for( i = 0; i < pRegion->m_Selections.GetSize( ); i++ )
	{
		pRegion->m_Selections[i]->Rotate( matRot, vRotationCenter );
		
		if(pRegion->m_Selections[i]->GetType() == Node_Brush)
			brushList.AddTail(pRegion->m_Selections[i]->AsBrush());
	}

	// Update the geometry stuff...
	pRegion->UpdatePlanes(&brushList);
	pRegion->CleanupGeometry(&brushList);
	GetRegionDoc()->UpdateSelectionBox();

	EndWaitCursor( );

	return TRUE;
}

BOOL CRegionView::OnSplitEdges(CUITracker *pTracker)
{
	if (GetEditMode() != GEOMETRY_EDITMODE)
		return FALSE;

	SplitEdges();

	return TRUE;
}

BOOL CRegionView::OnDeleteTaggedPolygons(CUITracker *pTracker)
{
	if (GetEditMode() != GEOMETRY_EDITMODE)
		return FALSE;

	DeleteTaggedPolygons();

	return TRUE;
}

BOOL CRegionView::OnDeleteEdges(CUITracker *pTracker)
{
	if (GetEditMode() != GEOMETRY_EDITMODE)
		return FALSE;

	DeleteTaggedEdges((BOOL)(((CRVCallback *)pTracker)->GetData()));

	return TRUE;
}

BOOL CRegionView::OnGridToPoly(CUITracker *pTracker)
{
	if (GetEditMode() != GEOMETRY_EDITMODE)
		return FALSE;

	if (!IPoly().IsValid())
		return FALSE;

	// Align the grid to the current polygon

	CEditPoly *pPoly = IPoly()();
					
	EditGrid().Right() = pPoly->Pt(1) - pPoly->Pt(0);
	EditGrid().Right().Norm();
	EditGrid().Forward() = pPoly->Normal();
	EditGrid().MakeUp();

	if( EditGrid().Forward().Dot(Nav().Pos() - EditGrid().Pos()) < 0.0f )
		EditGrid().Forward() = -EditGrid().Forward();

	EditGrid().Pos() = pPoly->Pt(0);
	
	GetRegionDoc()->RedrawAllViews();

	return TRUE;
}

BOOL CRegionView::OnTagPolyVerts(CUITracker *pTracker)
{
	GenListPos foundPos;

	if (GetEditMode() != GEOMETRY_EDITMODE)
		return FALSE;

	if (!IPoly().IsValid())
		return FALSE;

	CEditPoly *pPoly = IPoly()();
	DWORD i;

	CVertRef ref;

	// Tag the vertices of the immediate polygon
	for( i=0; i < pPoly->m_Indices; i++ )
	{
		ref.Init( IPoly().m_pBrush, pPoly->m_Indices[i] );
		
		if( !TaggedVerts().GenFindElement(ref, foundPos) )
			TaggedVerts().GenAppend( ref );
	}

	GetRegionDoc()->RedrawAllViews();

	return TRUE;
}

BOOL CRegionView::OnFlipNormal(CUITracker *pTracker)
{
	// Flip the brushes instead...
	if (GetEditMode() == BRUSH_EDITMODE)
	{
		OnBrushFlip();
		return TRUE;
	}

	if (GetEditMode() != GEOMETRY_EDITMODE)
		return FALSE;

	if (!IPoly().IsValid())
		return FALSE;

	// Flip the current polygon's normal.
	GetRegionDoc()->Modify(new CPreAction(ACTION_MODIFYNODE, IPoly().m_pBrush), TRUE);

	IPoly()()->Flip();
	
	UpdateImmediateSelection();
	GetRegionDoc()->RedrawAllViews();

	return TRUE;
}

BOOL CRegionView::OnOrbitVertex(CUITracker *pTracker)
{
	if (GetEditMode() != GEOMETRY_EDITMODE)
		return FALSE;

	if( (!IVert().IsValid()) || (!IsPerspectiveViewType()) )
		return FALSE;

	// Center the orbit on the current vertex.
	Nav().LookAt( IVert()() );
	//Nav().UpdateViewerDistance();

	return TRUE;
}

//given a user input event name and a menu handler, it will set up the text for
//the menu item
void CRegionView::SetMenuHotKeyText(const char* pszUIE, CCmdUI* pCmdUI)
{
	if((pCmdUI == NULL) || (pCmdUI->m_pMenu == NULL))
		return;

	//try and get the hotkey
	const CHotKey* pHotKey = CGlobalHotKeyDB::m_DB.GetHotKey( pszUIE );
	
	if( pHotKey == NULL)
	{
		return;
	}

	//get the existing text
	CString sText;
	pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nID, sText, MF_BYCOMMAND);

	//strip out everything past a tab if there is one
	int nTabPos = sText.Find('\t');

	if(nTabPos != -1)
	{
		sText = sText.Left(nTabPos);
	}
	
	//now we want to tack on the tab and the hot key name
	sText += "\t";

	sText += CBindKeyDlg::HotKeyToString( *pHotKey );

	//now we actually set the text of the menu item
	pCmdUI->SetText(sText);
}

// Returns the active region.  If none is active, just return the first region in 
// the active document.
CEditRegion* CRegionView::GetActiveRegion()
{
	CEditRegion *pRegion = GetRegion();
	if( !pRegion )
	{
		CDocTemplate* pTemp = GetApp()->m_pWorldTemplate;
		POSITION position = pTemp->GetFirstDocPosition();
		CDocument* pDoc = pTemp->GetNextDoc( position );
		POSITION viewPos = pDoc->GetFirstViewPosition();
		CRegionView* pView = (CRegionView*)pDoc->GetNextView( viewPos );
		pRegion = pView->GetRegion();
	}
	return pRegion;
}

//***************************************************************************//

//determines if a viewport is maximized
static bool IsViewMaximized(uint32 nRow, uint32 nCol, int nColWidths[], int nRowHeights[])
{
	if((nColWidths[(nCol + 1) % 2] == 0) && (nRowHeights[(nRow + 1) % 2] == 0))
		return true;

	return false;
}

//given a view index, this will calculate the row and column indices
static void GetViewRowAndCol(uint32 nView, uint32& nRow, uint32& nCol)
{
	switch( nView )
	{
		case 0:
			nRow = 0;
			nCol = 0;
			break;
		case 1:
			nRow = 1;
			nCol = 0;
			break;
		case 2:
			nRow = 0;
			nCol = 1;
			break;
		case 3:
			nRow = 1;
			nCol = 1;
			break;
		default:
			ASSERT(false);
			nRow = 0;
			nCol = 0;
			break;
	}
}

void CRegionView::MaximizeView( uint32 nViewIndex )
{
	CRegionFrame		*pFrame = (CRegionFrame*)GetParent()->GetParent();
	CSplitterWnd		*pSplitter = pFrame->GetSplitter();
	CRegionView			*pRegionView;
	CRegionDoc			*pRegionDoc = (CRegionDoc*)GetDocument();
	CRect				rect;

	//get the area of the client rectangle
	pFrame->GetClientRect( &rect );

	//get the dimensions of each one of the panes
	int nColWidths[2], nRowHeights[2], nGarbage;
	pSplitter->GetRowInfo( 0, nRowHeights[0], nGarbage );
	pSplitter->GetRowInfo( 1, nRowHeights[1], nGarbage );
	pSplitter->GetColumnInfo( 0, nColWidths[0], nGarbage );
	pSplitter->GetColumnInfo( 1, nColWidths[1], nGarbage );

	//get the row and column of the view we are modifying
	uint32 nViewRow, nViewCol;
	GetViewRowAndCol(nViewIndex, nViewRow, nViewCol);

	//we need to determine if we are maximizing the window, or restoring it to the
	//normal 4 pane view. If the other windows are minimized, then we are restoring it,
	//otherwise, we are maximizing
	if( IsViewMaximized(nViewRow, nViewCol, nColWidths, nRowHeights))
	{
		//we are restoring
		pSplitter->SetRowInfo(0, (int)((float)rect.Height() * pRegionDoc->m_fSavedSplitCenterY), 10);
		pSplitter->SetColumnInfo(0, (int)((float)rect.Width() * pRegionDoc->m_fSavedSplitCenterX), 10);
		pSplitter->RecalcLayout();

		return;
	}

	//see if any of the views are maximized
	BOOL bAnyMaximized = FALSE;

	for(uint32 nCurrView = 0; nCurrView < 4; nCurrView++)
	{
		uint32 nCurrRow, nCurrCol;
		GetViewRowAndCol(nCurrView, nCurrRow, nCurrCol);
		if(IsViewMaximized(nCurrRow, nCurrCol, nColWidths, nRowHeights))
		{
			bAnyMaximized = TRUE;
			break;
		}
	}

	//we are going to maximize, so save the ratio of the viewports so they can
	//be restored later, but only store it if nothing is already maximized
	if(!bAnyMaximized)
	{
		pRegionDoc->m_fSavedSplitCenterX = nColWidths[0] / (float)rect.Width();
		pRegionDoc->m_fSavedSplitCenterY = nRowHeights[0] / (float)rect.Height();
	}
	
	pSplitter->SetRowInfo( ( nViewIndex % 2 ) ? 0 : 1, 0, 10 );
	pSplitter->SetRowInfo( ( nViewIndex % 2 ) ? 1 : 0, rect.Height(), 10 );
	pSplitter->SetColumnInfo( ( nViewIndex > 1 ) ? 0 : 1, 0, 10 );
	pSplitter->SetColumnInfo( ( nViewIndex > 1 ) ? 1 : 0, rect.Width(), 10 );
	pSplitter->RecalcLayout();

	//we are maximizing the view. Before this is done however, we need to update the key
	//listing of the tracker we are switching it to to prevent it from sending
	//the same event twice
	pRegionView = (CRegionView*)pFrame->GetSplitter()->GetPane(nViewRow, nViewCol);
	if( pRegionView )
	{
		CUITracker* pTracker = pRegionView->m_cTrackerMgr.FindTracker("Focus");
		if(pTracker)
		{
			((CRVTrackerFocus*)pTracker)->RefreshKeyList();
		}
	}	

	// Call SetUpNewSize() on the viewports being minimized first, then
	// call SetUpNewSize() on the viewport being maximized first:
	for( int i = 0; i < 5; ++i )
	{
		CRegionView* pRegionView = NULL;

		if( i == nViewIndex )	// skip viewport being maximized (do it last; i.e. when i == 4)
			continue;

		uint32 nPaneToSize = i;

		if( nPaneToSize == 4 )
			nPaneToSize = nViewIndex;	// call SetupNewSize() on the view being maximized last (resize the other views first so that it will have the max. video ram available when it resizes the view being maximized)

		//find out which pane we are currently manipulating
		uint32 nPaneRow, nPaneCol;
		GetViewRowAndCol(nPaneToSize, nPaneRow, nPaneCol);
		
		pRegionView = (CRegionView*)pFrame->GetSplitter()->GetPane(nPaneRow, nPaneCol);

		//now actually resize the pane
		if( pRegionView )
		{
			CRect rect;
			pRegionView->GetClientRect( &rect );
			pRegionView->SetupNewSize( rect.Width(), rect.Height() );
		}
	}

	GetRegionDoc()->RedrawAllViews();
}

//***************************************************************************//
void CRegionView::OnViewMaximizeView1() 
{
	MaximizeView( 0 );
}

void CRegionView::OnUpdateViewMaximizeView1(CCmdUI* pCmdUI) 
{
	CEditRegion *pRegion = GetActiveRegion();
	if( !pRegion )
		return;

	SetMenuHotKeyText(UIE_MAXIMIZE_VIEW1, pCmdUI);
}

//***************************************************************************//
void CRegionView::OnViewMaximizeView2() 
{
	MaximizeView( 2 );
}

void CRegionView::OnUpdateViewMaximizeView2(CCmdUI* pCmdUI) 
{
	CEditRegion *pRegion = GetActiveRegion();
	if( !pRegion )
		return;

	SetMenuHotKeyText(UIE_MAXIMIZE_VIEW2, pCmdUI);
}

//***************************************************************************//
void CRegionView::OnViewMaximizeView3() 
{
	MaximizeView( 1 );
}

void CRegionView::OnUpdateViewMaximizeView3(CCmdUI* pCmdUI) 
{
	CEditRegion *pRegion = GetActiveRegion();
	if( !pRegion )
		return;

	SetMenuHotKeyText(UIE_MAXIMIZE_VIEW3, pCmdUI);
}

//***************************************************************************//
void CRegionView::OnViewMaximizeView4() 
{
	MaximizeView( 3 );
}

void CRegionView::OnUpdateViewMaximizeView4(CCmdUI* pCmdUI) 
{
	CEditRegion *pRegion = GetActiveRegion();
	if( !pRegion )
		return;

	SetMenuHotKeyText(UIE_MAXIMIZE_VIEW4, pCmdUI);
}

//***************************************************************************//
void CRegionView::OnViewMaximizeActiveView() 
{
	CRegionFrame* pFrame = (CRegionFrame*)GetParent()->GetParent();
	CRegionView* pRegionView = (CRegionView*)pFrame->GetSplitter()->GetPane(0,0);
	if( ::GetFocus() == pRegionView->m_hWnd )
	{
		MaximizeView( 0 );
		return;
	}
	pRegionView = (CRegionView*)pFrame->GetSplitter()->GetPane(1,0);
	if( ::GetFocus() == pRegionView->m_hWnd )
	{
		MaximizeView( 1 );
		return;
	}
	pRegionView = (CRegionView*)pFrame->GetSplitter()->GetPane(0,1);
	if( ::GetFocus() == pRegionView->m_hWnd )
	{
		MaximizeView( 2 );
		return;
	}
	pRegionView = (CRegionView*)pFrame->GetSplitter()->GetPane(1,1);
	if( ::GetFocus() == pRegionView->m_hWnd )
	{
		MaximizeView( 3 );
		return;
	}
}

void CRegionView::OnUpdateViewMaximizeActiveView(CCmdUI* pCmdUI) 
{
	CEditRegion *pRegion = GetActiveRegion();
	if( !pRegion )
		return;

	SetMenuHotKeyText(UIE_MAXIMIZE_ACTIVE_VIEW, pCmdUI);
}

//***************************************************************************//
void CRegionView::OnSelectionFreezeSelected()
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}

	// Put the wait cursor on the screen
	BeginWaitCursor( );

	// Get the selected nodes
	CMoArray<CWorldNode*> selectedNodes;
	selectedNodes.CopyArray(pRegion->m_Selections);
		
	CWorldNode::FindParentNodes(selectedNodes);

	int i;

	// Setup the undo information 
	if(GetApp()->GetOptions().GetMiscOptions()->IsUndoFreezeHide())
	{
		PreActionList actionList;	
		for (i=0; i < pRegion->GetNumSelections(); i++)
		{
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pRegion->GetSelection(i)));
		}
		GetRegionDoc()->Modify(&actionList, TRUE);
	}
	else
		GetRegionDoc()->Modify();


	for (i=0; i < pRegion->GetNumSelections(); i++)
	{
		// Hide the node
		pRegion->FreezeNode(pRegion->GetSelection(i), true, true, false);
	}

	//clean up all the old selections
	pRegion->ClearSelections();

	// Update the node images
	GetNodeView()->UpdateNodeImage(pRegion->GetRootNode());

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();

	// End the wait cursor
	EndWaitCursor( );
}

void CRegionView::OnUpdateSelectionFreezeSelected(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetRegion()->GetNumSelections() > 0) ? TRUE : FALSE);
	SetMenuHotKeyText(UIE_FREEZE_SELECTED, pCmdUI);
}

//***************************************************************************//

void CRegionView::OnSelectionUnfreezeAll()
{
	// Setup the undo information 
	if(GetApp()->GetOptions().GetMiscOptions()->IsUndoFreezeHide())
	{
		PreActionList actionList;	
		UndoHelper(&actionList, GetRegion()->GetRootNode(), NODEFLAG_FROZEN, true);
		GetRegionDoc()->Modify(&actionList, TRUE);
	}
	else
		GetRegionDoc()->Modify();

	//unfreeze the entire tree
	GetRegion()->FreezeNode(GetRegion()->GetRootNode(), false, false, true);

	// Update the node images
	GetNodeView()->UpdateNodeImage(GetRegion()->GetRootNode());

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdateSelectionUnfreezeAll(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_UNFREEZE_ALL, pCmdUI);
}

/*************************************************************************/
// Model Viewing command handlers
void CRegionView::OnDisplayAllModels()
{
	//unfreeze the entire tree
	GetRegion()->GetRootNode()->ShowModel(TRUE, TRUE);

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdateDisplayAllModels(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_SHOW_ALL_MODELS, pCmdUI);
}

void CRegionView::OnHideAllModels()
{
	//unfreeze the entire tree
	GetRegion()->GetRootNode()->ShowModel(FALSE, TRUE);

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdateHideAllModels(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_HIDE_ALL_MODELS, pCmdUI);
}

void CRegionView::OnDisplaySelectedModels()
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}

	for (uint32 nCurrSel = 0; nCurrSel < pRegion->GetNumSelections(); nCurrSel++)
	{
		// Hide the node
		pRegion->GetSelection(nCurrSel)->ShowModel(TRUE, FALSE);
	}

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdateDisplaySelectedModels(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetRegion()->GetNumSelections() > 0) ? TRUE : FALSE);
	SetMenuHotKeyText(UIE_SHOW_SELECTED_MODELS, pCmdUI);
}

void CRegionView::OnHideSelectedModels()
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}

	for (uint32 nCurrSel = 0; nCurrSel < pRegion->GetNumSelections(); nCurrSel++)
	{
		// Hide the node
		pRegion->GetSelection(nCurrSel)->ShowModel(FALSE, FALSE);
	}

	// Redraw the views
	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdateHideSelectedModels(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetRegion()->GetNumSelections() > 0) ? TRUE : FALSE);
	SetMenuHotKeyText(UIE_HIDE_SELECTED_MODELS, pCmdUI);
}

//this will recursively search through the node tree, and set the showing
//model property for all nodes that are of the specified class
static void ShowModelsForClass(CWorldNode* pNode, const CString& sClass, BOOL bShowModel)
{
	ASSERT(pNode);

	//see if we have an object
	if(pNode->GetType() == Node_Object)
	{
		if(sClass.CompareNoCase(pNode->AsObject()->GetClassName()) == 0)
		{
			pNode->ShowModel(bShowModel, FALSE);
		}
	}

	//now run through all of its children
	GPOS pos = pNode->m_Children;
	while (pos)
	{
		ShowModelsForClass(pNode->m_Children.GetNext(pos), sClass, bShowModel);
	}

}

void CRegionView::OnDisplayModelsOfClass()
{
	//first have the user select the class that they want displayed
	CClassDlg dlg;
	dlg.SetProject(GetProject());
	dlg.SetTitle("Show Models for which Class");

	if(dlg.DoModal() != IDOK)
	{
		//they aborted the operation
		return;
	}

	//now we have to go through the list of objects, and find out what objects match
	//the class
	ShowModelsForClass(GetRegion()->GetRootNode(), dlg.GetSelectedClass(), TRUE);		
}

void CRegionView::OnUpdateDisplayModelsOfClass(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_SHOW_MODELS_OF_CLASS, pCmdUI);
}

void CRegionView::OnHideModelsOfClass()
{
	//first have the user select the class that they want displayed
	CClassDlg dlg;
	dlg.SetProject(GetProject());
	dlg.SetTitle("Hide Models for which Class");

	if(dlg.DoModal() != IDOK)
	{
		//they aborted the operation
		return;
	}

	//now we have to go through the list of objects, and find out what objects match
	//the class
	ShowModelsForClass(GetRegion()->GetRootNode(), dlg.GetSelectedClass(), FALSE);
	
	// Redraw the views
	GetRegionDoc()->RedrawAllViews();

}

void CRegionView::OnUpdateHideModelsOfClass(CCmdUI* pCmdUI)
{
	SetMenuHotKeyText(UIE_HIDE_MODELS_OF_CLASS, pCmdUI);
}

void CRegionView::OnDisplayModelPolycount()
{
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)  return;

	uint32 nPolycount = 0;

	for (uint32 nCurrSel = 0; nCurrSel < pRegion->GetNumSelections(); nCurrSel++)
	{
		if (pRegion->GetSelection(nCurrSel)->GetType() == Node_Object)
		{
			CBaseEditObj *pObj = pRegion->GetSelection(nCurrSel)->AsObject();

			//see if this object has a valid handle
			if(!pObj->GetModelHandle().IsValid())
				continue;

			CMeshShapeList *pList;
			
			if(!GetApp()->GetModelMgr().GetShapeList(pObj->GetModelHandle(), &pList))
				continue;

			for (uint32 i=0; i<pList->GetNumShapes(); i++)
			{
				nPolycount += pList->GetShape(i)->GetNumTriangles();
			}

			GetApp()->GetModelMgr().ReleaseShapeList(pObj->GetModelHandle());
		}
	}

	char text[128];
	sprintf(text, "%d polygons counted in all models selected.", nPolycount);
	AppMessageBox(text, MB_OK);
}

void CRegionView::OnUpdateDisplayModelPolycount(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetRegion()->GetNumSelections() > 0) ? TRUE : FALSE);
	SetMenuHotKeyText(UIE_SHOW_MODEL_POLYCOUNT, pCmdUI);
}


void CRegionView::UndoHelper(PreActionList *actionList, CWorldNode *node, const uint32 flag, const bool wantFlag) const
{
	// If this node has the flag set properly, add it to the list

	if (flag == 0) 
	{
		actionList->AddTail(new CPreAction(ACTION_MODIFYNODE, node));
	}
	else if (wantFlag && (node->IsFlagSet(flag)))
	{
		actionList->AddTail(new CPreAction(ACTION_MODIFYNODE, node));
	}
	else if (!wantFlag && !(node->IsFlagSet(flag)))
	{
		actionList->AddTail(new CPreAction(ACTION_MODIFYNODE, node));
	}

	// Recurse through all children

	GPOS ChildPos = node->m_Children;

	while(ChildPos)
	{
		UndoHelper(actionList, node->m_Children.GetNext(ChildPos), flag, wantFlag);
	}
}

//**************************************************************************//
void CRegionView::OnFitTextureToPoly()
{
	//only in geometry edit mode
	if(GetEditMode() != GEOMETRY_EDITMODE)
		return;

	//we need to build up a list of polygons that we will be fitting the face
	//to
	CMoArray<CEditPoly*> PolyList;

	PolyList.SetCacheSize(256);
	GetSelectedPolies(PolyList);

	//make sure we have polygons
	if(PolyList.GetSize() == 0)
		return;

	//now we need to get the PQ vectors from the first polygon
	LTVector vP = PolyList[0]->GetTexture(GetCurrTexture()).GetP();
	LTVector vQ = PolyList[0]->GetTexture(GetCurrTexture()).GetQ();

	//set the O to the first point
	LTVector vO = PolyList[0]->Pt(0);

	//find a normalized version of P and Q.
	LTVector vPNorm(vP);
	vPNorm.Norm();

	LTVector vQNorm(vQ);
	vQNorm.Norm();

	//run through all polygons and all their points and adjust the O
	uint32 nCurrPoly;
	for(nCurrPoly = 0; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		CEditPoly *pPoly = PolyList[nCurrPoly];

		for(uint32 nCurrPt = 0; nCurrPt < pPoly->NumVerts(); nCurrPt++)
		{
			//do some dot products to determine how O needs to move
			CReal fPDot = (pPoly->Pt(nCurrPt) - vO).Dot(vPNorm);
			CReal fQDot = (pPoly->Pt(nCurrPt) - vO).Dot(vQNorm);

			//if either is negative, we need to shift the origin
			if(fPDot < 0.0f)
			{
				vO += vPNorm * fPDot;
			}
			if(fQDot < 0.0f)
			{
				vO += vQNorm * fQDot;
			}
		}
	}

	//ok, we now have the final origin, now we need to stretch P and Q to form a box
	//over all the points
	CReal fPLen = 0.0f;
	CReal fQLen = 0.0f;


	MakeUndoForPolygons(&PolyList);

	//also we need to do the undo around here as well
	for(nCurrPoly = 0; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		CEditPoly* pPoly = PolyList[nCurrPoly];

		for(uint32 nCurrPt = 0; nCurrPt < pPoly->NumVerts(); nCurrPt++)
		{
			LTVector vToPt = pPoly->Pt(nCurrPt) - vO;

			//find the lengths with respect to P and Q
			CReal fCurrPLen = vToPt.Dot(vPNorm);
			CReal fCurrQLen = vToPt.Dot(vQNorm);

			fPLen = LTMAX(fPLen, fCurrPLen);
			fQLen = LTMAX(fQLen, fCurrQLen);
		}
	}

	//now we create the final P and Q
	vP = vPNorm / fPLen;
	vQ = vQNorm / fQLen;
					
	//now set all the polygons, and make the undo
	for(nCurrPoly = 0; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		CEditPoly *pPoly = PolyList[nCurrPoly];

		//get the dimensions of the textures
		uint32 nTexWidth  = pPoly->GetTexture(GetCurrTexture()).GetBaseTextureWidth();
		uint32 nTexHeight = pPoly->GetTexture(GetCurrTexture()).GetBaseTextureHeight();

		pPoly->SetTextureSpace(GetCurrTexture(), vO, vP * (float)nTexWidth, vQ * (float)nTexHeight);
	}

	Invalidate();
}

//handler for fitting texture to a face
void CRegionView::OnUpdateFitTextureToPoly(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((GetEditMode() == GEOMETRY_EDITMODE) && AreAnyPoliesSelected());
	SetMenuHotKeyText(UIE_FIT_TEXTURE_TO_POLY, pCmdUI);
 
}

//for toggling class icons in the views
void CRegionView::OnToggleClassIcons()
{
	//get the old value
	BOOL bOld = GetApp()->GetOptions().GetDisplayOptions()->IsShowClassIcons();

	//set the new
	GetApp()->GetOptions().GetDisplayOptions()->SetShowClassIcons(bOld ? FALSE : TRUE);

	//redraw the views
	Invalidate();
	GetRegionDoc()->UpdateAllViews( this );
}

void CRegionView::OnUpdateToggleClassIcons(CCmdUI* pCmdUI)
{
	//set the check appropriately
	pCmdUI->SetCheck(GetApp()->GetOptions().GetDisplayOptions()->IsShowClassIcons());

	//set the text
	SetMenuHotKeyText(UIE_TOGGLE_CLASS_ICONS, pCmdUI);
}

//for toggling class icons in the views
void CRegionView::OnHideFrozenNodes()
{
	//get the old value
	BOOL bOld = GetApp()->GetOptions().GetDisplayOptions()->IsHideFrozenNodes();

	//set the new
	GetApp()->GetOptions().GetDisplayOptions()->SetHideFrozenNodes(bOld ? FALSE : TRUE);

	//redraw the views
	Invalidate();
	GetRegionDoc()->UpdateAllViews( this );
}

void CRegionView::OnUpdateHideFrozenNodes(CCmdUI* pCmdUI)
{
	//set the check appropriately
	pCmdUI->SetCheck(GetApp()->GetOptions().GetDisplayOptions()->IsHideFrozenNodes());

	//set the text
	SetMenuHotKeyText(UIE_HIDE_FROZEN_NODES, pCmdUI);
}

void CRegionView::OnSetCameraFOV()
{
	CCameraFOVDlg Dlg;

	Dlg.DoModal();
}

void CRegionView::OnCameraToObject()
{
	//run through and find the first selected object
	for(uint32 nCurrSel = 0; nCurrSel < GetRegion()->GetNumSelections(); nCurrSel++)
	{
		if(GetRegion()->GetSelection(nCurrSel)->GetType() == NODE_OBJECT)
		{
			CBaseEditObj* pObj = GetRegion()->GetSelection(nCurrSel)->AsObject();

			//we have our object, so we need to steal the position and orientation,
			//and set that up for our camera
			LTVector vPos = pObj->GetPos();

			LTMatrix mObjTrans;
			gr_SetupMatrixEuler(pObj->GetOr(), mObjTrans.m);

			LTVector vUp, vRight, vForward;
			mObjTrans.GetBasisVectors(&vRight, &vUp, &vForward);

			Nav().Pos() = vPos;
			Nav().LookAt(vPos + vForward * 100.0f);

			// Redraw the views
			GetRegionDoc()->RedrawAllViews();
		}
	}
}

void CRegionView::OnUpdateCameraToObject(CCmdUI* pCmdUI)
{
	BOOL bValid = FALSE;

	if(GetViewMode() == VM_PERSPECTIVE)
	{

		//make sure we have an object selected
		for(uint32 nCurrSel = 0; nCurrSel < GetRegion()->GetNumSelections(); nCurrSel++)
		{
			if(GetRegion()->GetSelection(nCurrSel)->GetType() == NODE_OBJECT)
			{
				bValid = TRUE;
				break;
			}
		}
	}

	pCmdUI->Enable(bValid);
	//set the text
	SetMenuHotKeyText(UIE_CAMERA_TO_OBJECT, pCmdUI);
}

//callback for reporting errors while instantiating prefabs
static void ReportPrefabError(const char* pszError)
{
	GetDebugDlg()->ShowWindow(SW_SHOW);
	GetDebugDlg()->AddMessage(pszError);
}

void CRegionView::OnDisconnectSelectedPrefabs()
{
	//setup the undo list
	PreActionList undoList;

	//the list of detached trees
	CMoArray<CWorldNode*> RootList;

	//run through and find the selected prefabs
	int32 nCurrSel;
	for(nCurrSel = 0; nCurrSel < GetRegion()->GetNumSelections(); nCurrSel++)
	{
		if(GetRegion()->GetSelection(nCurrSel)->GetType() == Node_PrefabRef)
		{
			//get the prefab
			CPrefabRef* pPrefabRef = (CPrefabRef*)GetRegion()->GetSelection(nCurrSel);

			//instantiate it
			CWorldNode *pNewRoot = pPrefabRef->InstantiatePrefab(GetRegion(), ReportPrefabError, &undoList);
			RootList.Append(pNewRoot);

			//Setup this node for undo so it will be recreated when undone
			undoList.AddTail(new CPreAction(ACTION_MODIFYNODE, pPrefabRef));
		}
	}

	//now we setup the undo so that the prefabs will all be backed up in their current
	//state
	GetRegionDoc()->Modify(&undoList);

	//now that the nodes have been backed up for undo, we need to go through and destroy
	//the prefabs and handle the new selections (note that we run through backwards,
	//this is because removing the node removes it from the selection and that messes
	//up the count when going forward)
	for(nCurrSel = GetRegion()->GetNumSelections() - 1; nCurrSel >= 0; nCurrSel--)
	{
		if(GetRegion()->GetSelection(nCurrSel)->GetType() == Node_PrefabRef)
		{
			//get the prefab
			CPrefabRef* pPrefabRef = (CPrefabRef*)GetRegion()->GetSelection(nCurrSel);

			//destroy the node
			no_DestroyNode(GetRegion(), pPrefabRef, TRUE);
		}
	}

	//now select all of the children that came off of it since it was
	//originally selected
	for(uint32 nCurrRoot = 0; nCurrRoot < RootList.GetSize(); nCurrRoot++)
	{
		GetRegion()->RecurseAndSelect(RootList[nCurrRoot]);
	}

	//update the selection box and redraw the views
	GetRegionDoc()->UpdateSelectionBox();
	GetRegionDoc()->RedrawAllViews();
}

void CRegionView::OnUpdateDisconnectSelectedPrefabs(CCmdUI* pCmdUI)
{
	BOOL bValid = FALSE;

	//make sure we have an object selected
	for(uint32 nCurrSel = 0; nCurrSel < GetRegion()->GetNumSelections(); nCurrSel++)
	{
		if(GetRegion()->GetSelection(nCurrSel)->GetType() == Node_PrefabRef)
		{
			bValid = TRUE;
			break;
		}
	}
	
	pCmdUI->Enable(bValid);
	//set the text
	SetMenuHotKeyText(UIE_DISCONNECT_SELECTED_PREFABS, pCmdUI);
}

void CRegionView::OnMirrorTextureX()
{
	//we need to build up a list of polygons that we will be fitting the face
	//to
	CMoArray<CEditPoly*> PolyList;

	PolyList.SetCacheSize(256);
	GetSelectedPolies(PolyList);

	//make the undo
	if(PolyList.GetSize() > 0)
		MakeUndoForPolygons(&PolyList);

	//run through the polygons and invert the P vector
	for(uint32 nCurrPoly = 0; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		CEditPoly* pPoly = PolyList[nCurrPoly];
		CTexturedPlane& PolyTex = pPoly->GetTexture(GetCurrTexture());
		pPoly->SetTextureSpace(GetCurrTexture(), PolyTex.GetO(), -PolyTex.GetP(), PolyTex.GetQ());
	}

	GetRegionDoc()->RedrawPerspectiveViews();
}

void CRegionView::OnUpdateMirrorTextureX(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(AreAnyPoliesSelected());
	SetMenuHotKeyText(UIE_MIRROR_TEXTURE_X, pCmdUI);
}

void CRegionView::OnMirrorTextureY()
{
	//we need to build up a list of polygons that we will be fitting the face
	//to
	CMoArray<CEditPoly*> PolyList;

	PolyList.SetCacheSize(256);
	GetSelectedPolies(PolyList);

	//make the undo
	if(PolyList.GetSize() > 0)
		MakeUndoForPolygons(&PolyList);

	//run through the polygons and invert the Q vector
	for(uint32 nCurrPoly = 0; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		CEditPoly* pPoly = PolyList[nCurrPoly];
		CTexturedPlane& PolyTex = pPoly->GetTexture(GetCurrTexture());
		pPoly->SetTextureSpace(GetCurrTexture(), PolyTex.GetO(), PolyTex.GetP(), -PolyTex.GetQ());
	}

	GetRegionDoc()->RedrawPerspectiveViews();
}

void CRegionView::OnUpdateMirrorTextureY(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(AreAnyPoliesSelected());
	SetMenuHotKeyText(UIE_MIRROR_TEXTURE_Y, pCmdUI);
}

void CRegionView::OnMatchTextureCoords()
{
	//only allow this if we are in geometry mode and have more than one polygon
	//selected
	if((GetEditMode() == GEOMETRY_EDITMODE) && (TaggedPolies().GetSize() < 2))
	{
		return;
	}

	//we need to build up a list of polygons that we will be adjusting
	CMoArray<CEditPoly*> PolyList;

	PolyList.SetCacheSize(256);
	GetSelectedPolies(PolyList);

	//first setup an undo
	MakeUndoForPolygons(&PolyList);

	//we will be taking the settings from the first polygon
	CEditPoly* pSource = PolyList[0];

	//get the texture coordinates and size of this polygon
	LTVector vSrcO, vSrcP, vSrcQ;
	pSource->GetTexture(GetCurrTexture()).GetTextureSpace(vSrcO, vSrcP, vSrcQ);

	uint32 nSrcWid = pSource->GetTexture(GetCurrTexture()).GetBaseTextureWidth();
	uint32 nSrcHgt = pSource->GetTexture(GetCurrTexture()).GetBaseTextureHeight();

	//now for all the other polygons we want to set the texture space to match
	for(uint32 nCurrPoly = 1; nCurrPoly < PolyList.GetSize(); nCurrPoly++)
	{
		CEditPoly* pCurr = PolyList[nCurrPoly];

		//get the size of this texture
		uint32 nCurrWid = pCurr->GetTexture(GetCurrTexture()).GetBaseTextureWidth();
		uint32 nCurrHgt = pCurr->GetTexture(GetCurrTexture()).GetBaseTextureHeight();

		//find out what scale factor we are going to want to apply
		CReal fXScale = (CReal)nCurrWid / (CReal)nSrcWid;
		CReal fYScale = (CReal)nCurrHgt / (CReal)nSrcHgt;

		//setup the new texture space
		pCurr->SetTextureSpace(GetCurrTexture(), vSrcO, vSrcP * fXScale, vSrcQ * fYScale);
	}

	//redraw the textured views
	GetRegionDoc()->RedrawPerspectiveViews();
}

void CRegionView::OnUpdateMatchTextureCoords(CCmdUI* pCmdUI)
{
	//only allow this if we are in geometry mode and have more than one polygon
	//selected
	if((GetEditMode() == GEOMETRY_EDITMODE) && (TaggedPolies().GetSize() > 1))
	{
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}

	SetMenuHotKeyText(UIE_MATCH_TEXTURE_COORDS, pCmdUI);
}

void CRegionView::OnSnapVertices()
{
	//make sure that the user wants to do this
	CString sText;
	sText += "This tool will run through the current selection and truncate the position of all vertex coordinates. ";
	sText += "This will cause all points to lie on the grid, however with any non-triangulated geometry it can cause non-planar polygons. ";
	sText += "In addition this can cause seams to appear with vertices that are not co-incident. It is highly recommended that you back";
	sText += "up work before performing this, and thoroughly test the modified geometry before proceeding forward with the changes. ";
	sText += "Press cancel now if you do not want to perform this operation. ";

	if(MessageBox(sText, "Are you sure you want to do this?", MB_OKCANCEL) != IDOK)
	{
		return;
	}

	for(uint32 nCurrSel = 0; nCurrSel < GetRegion()->m_Selections.GetSize(); nCurrSel++)
	{
		if(GetRegion()->m_Selections[nCurrSel]->GetType() == Node_Brush)
		{
			CEditBrush* pBrush = (CEditBrush*)GetRegion()->m_Selections[nCurrSel];

			for(uint32 i = 0; i < pBrush->m_Points; i++)
			{
				pBrush->m_Points[i].x = floor(pBrush->m_Points[i].x);
				pBrush->m_Points[i].y = floor(pBrush->m_Points[i].y);
				pBrush->m_Points[i].z = floor(pBrush->m_Points[i].z);
			}
			
			pBrush->UpdateBoundingInfo();
			pBrush->UpdatePlanes();

			GetRegionDoc()->UpdateSelectionBox();
		}
	}
}

void CRegionView::OnUpdateSnapVertices(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	for(uint32 nCurrSel = 0; nCurrSel < GetRegion()->m_Selections.GetSize(); nCurrSel++)
	{
		if(GetRegion()->m_Selections[nCurrSel]->GetType() == Node_Brush)
		{
			bEnable = TRUE;
			break;
		}
	}

	pCmdUI->Enable(bEnable);
}

void CRegionView::OnHollowBrush()
{
	DWORD				i, j, k, l;

	CStringDlg			dlg;
	CWorldNode			*pParent;
	CReal				thickness;
	CPolyRef			polyRef;
	CVertRefArray		newVerts;

	CEditPoly			*pPoly;
	CEditBrush			*pBrush;
	CEditBrush			*pNewBrush;
	CEditPoly			*pNewPoly;
	CVector				brushPos;
	BOOL				bOk, bValidNumber;
	DMatrix mPlaneNormal;
	DVector vPlaneDist;
	DPlane tempPlane;


	// undo list
	PreActionList actionList;

	if( (GetRegion()->m_Selections == 1) && (GetRegion()->m_Selections[0]->GetType() == Node_Brush) )
	{
		dlg.m_bAllowNumbers = TRUE;

		bOk = TRUE;
		bValidNumber = FALSE;
		dlg.m_EnteredText.Format( "%d", 16 );
		while( bOk && !bValidNumber )
		{
			if( bOk = ( dlg.DoModal(IDS_HOLLOWBRUSHCAPTION, IDS_ENTERBRUSHTHICKNESS) == IDOK ))
			{
				thickness = (CReal)atoi( dlg.m_EnteredText );
				if( thickness != 0.0f )
				{
					bValidNumber = TRUE;
				}
				else
				{
					AppMessageBox( IDS_ERROR_BRUSH_THICKNESS, MB_OK );
				}
			}
		}

		if( bOk )
		{
			pParent = GetRegion()->AddNullNode(GetRegion()->m_Selections[0]->GetParent());
			pBrush = GetRegion()->m_Selections[0]->AsBrush();

			CEditVertArray cHullPoints;
			CMoArray<DPlane> cPlanes;

			// Calculate the points on the hull
			for ( i = 0; i < pBrush->m_Points.GetSize(); i++ )
			{
				CVector vHullPoint( pBrush->m_Points[i] );

				// Clear out the plane list
				cPlanes.RemoveAll();

				// Find three polygons which contain this index
				for ( j = 0; (j < pBrush->m_Polies.GetSize()) && (cPlanes.GetSize() < 3); j++ )
				{
					pPoly = pBrush->m_Polies[j];
					for ( k = 0; k < pPoly->m_Indices.GetSize(); k++ )
					{
						if ( pPoly->m_Indices[k] == i )
						{
							// Convert to a DPlane and extend the plane out.
							tempPlane.m_Normal = pPoly->m_Plane.m_Normal;
							tempPlane.m_Dist = pPoly->m_Plane.m_Dist + thickness;	// Move the plane out..
							cPlanes.Append(tempPlane);
							break;
						}
					}
				}

				// If enough planes were found, calculate the intersection
				if ( cPlanes.GetSize() >= 3 )
				{
					// Iterate through the plane combinations until a valid combination of 3 planes is found
					bOk = FALSE;
					for (j = 0; (j < cPlanes.GetSize() - 2) && (!bOk); j++)
					{
						for (k = j + 1; (k < cPlanes.GetSize() - 1) && (!bOk); k++)
						{
							for (l = k + 1; (l < cPlanes.GetSize()) && (!bOk); l++)
							{
								bOk = gr_IntersectPlanes(cPlanes[j], cPlanes[k], cPlanes[l], vHullPoint);
							}
						}
					}

					if (!bOk)
					{
						MessageBox("Please remove any neighboring coplanar polygons before performing this operation.");
						return;
					}
				}
				else if ( cPlanes.GetSize() != 0 )
				{
					// If this happens, the brush is invalid and shouldn't be processed.
					MessageBox("Invalid geometry has been detected on this brush.  Please clean up the geometry and try again.");
					return;
				}
				// If cPlanes.GetSize() == 0 then the vertex isn't on any of the polygons and
				//	can safely be ignored.

				// Append this point to the list
				cHullPoints.Append( vHullPoint );
			}

			for( i=0; i < pBrush->m_Polies; i++ )
			{
				pPoly = pBrush->m_Polies[i];

				// Setup a new brush and a new starting poly.
				pNewBrush = no_CreateNewBrush(GetRegion(), pParent);
				pNewPoly = new CEditPoly( pNewBrush );

				pNewBrush->m_Polies.Append( pNewPoly );
				for( j=0; j < pPoly->NumVerts(); j++ )
				{
					pNewPoly->m_Indices.Append( (WORD)j );
					pNewBrush->m_Points.Append( pPoly->Pt(j) );
				}

				pNewPoly->Normal() = pPoly->Normal();
				pNewPoly->Dist() = pPoly->Dist();

				// Copy texture attributes
				pNewPoly->CopyAttributes(pPoly);

				// Extrude.
				polyRef.Init( pNewBrush, 0 );
				if( ExtrudePoly( polyRef, newVerts, FALSE, FALSE /*flip all*/, TRUE /*perpendicular*/ ) )
				{
					// ExtrudePoly makes all the polies except the starting one
					// facing the correct direction.
					pNewPoly->Flip();

					for( j=0; j < newVerts; j++ )
					{
						newVerts[j]() = cHullPoints[pPoly->m_Indices[j]];
					}

					for( j=0; j < pNewBrush->m_Polies; j++ )
					{
						if ( thickness < 0.0f )						
						{
							pNewBrush->m_Polies[i]->Flip();
						}
						pNewBrush->m_Polies[j]->UpdatePlane();								
						
						// Setup the base texture space
						for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
						{
							pNewBrush->m_Polies[j]->SetupBaseTextureSpace(nCurrTex);
						}
					}
				}

				// Clean up this brush
				CReal normalThresh = 0.05f;
				CReal distThresh = 0.0001f;
				pNewBrush->RemoveExtraEdges( normalThresh, distThresh );
				m_pRegion->UpdateBrushGeometry(pNewBrush);

				// Add to the undo list
				actionList.AddTail(new CPreAction(ACTION_ADDEDNODE, pNewBrush));
			}

			// Add the final brushes to the undo list
			GetRegionDoc()->Modify(&actionList, TRUE);

			DeleteSelectedNodes( );

			GetNodeView( )->UpdateNodeImage( GetRegion( )->GetRootNode( ));
			GetRegionDoc()->NotifySelectionChange();
		}
	}
	else
	{
		AppMessageBox( IDS_INVALIDHOLLOW, MB_OK );
	}
}

/************************************************************************/
// Handler for the carve command
void CRegionView::OnBrushCarve() 
{
	if (GetEditMode() != BRUSH_EDITMODE)
		return;

	CEditRegion *pRegion=GetRegion();

	// Make sure that we have one brush selected
	if((pRegion->GetNumSelections() < 1) || (pRegion->GetSelection(0)->GetType() != Node_Brush))
		return;

	//TODO: re-implement carve
	//DoCarveOperation();	
}

/************************************************************************/
// The command update handler for the carve command
void CRegionView::OnUpdateBrushCarve(CCmdUI* pCmdUI) 
{	
	// Get the edit region
	CEditRegion *pRegion=GetRegion();

	// Make sure that we have one brush selected
	if(pRegion->GetNumSelections() == 1 && pRegion->GetSelection(0)->GetType() == Node_Brush)
	{
		// Enable the menu item
		pCmdUI->Enable(TRUE);		
	}
	else
	{
		// Disable the menu item
		pCmdUI->Enable(FALSE);
	}
}
