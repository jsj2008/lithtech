#include "bdefs.h"
#include "objectimporter.h"
#include "dedit.h"
#include "regiondoc.h"
#include "regionview.h"
#include "eventnames.h"
#include "stdrvtrackers.h"

void CRegionView::InitTracker()
{
	//the current tracker that is being added
	CRegionViewTracker *pRVTracker;
	
	//trackers that need to be preserved for overriding inside of other trackers
	//(primarily the drawing tracker)
	CRegionViewTracker *pZoomTracker, *pMouseZoomTracker, *pMoveTracker, *pDragTracker;
	CRegionViewTracker *pShrinkGridTracker, *pExpandGridTracker;

	// Set up the focus tracker
	pRVTracker = new CRVTrackerFocus(UIE_TRACKER_FOCUS, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the depth-specific selection callback
	pRVTracker = new CRVCallback(UIE_SELECT_DEPTH, this, CRegionView::OnDepthSelect);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, 'Y'));
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the apply-texture callbacks
	pRVTracker = new CRVCallback(UIE_APPLY_TEXTURE_CLICK, this, CRegionView::OnApplyTextureClick);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, 'Y'));
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerMenuItem( UIE_APPLY_TEXTURE_TO_SEL, this, OnApplyTexture, OnUpdateApplyTexture );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_REMOVE_TEXTURE, this, OnRemoveTexture, OnUpdateRemoveTexture );
	m_cTrackerMgr.AddTracker( pRVTracker );

	// Set up the speed test callback
	/*
	pRVTracker = new CRVCallback(UIE_TEST_RENDER_SPEED, this, CRegionView::OnTestRenderSpeed);
	((CRVCallback *)pRVTracker)->SetFireOnStart(FALSE);
	m_cTrackerMgr.AddTracker(pRVTracker);
	*/

	// Set up the split callback
	pRVTracker = new CRVCallbackSimple(UIE_SPLIT_BRUSH, this, CRegionView::OnBrushSplitBrush);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the unselect callback
	pRVTracker = new CRVCallback(UIE_SELECT_NONE, this, CRegionView::OnSelectNone);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the grid direction callbacks
	pRVTracker = new CRVCallback(UIE_SET_GRID_FORWARD, this, CRegionView::OnSetGridOrientation);
	((CRVCallback *)pRVTracker)->SetData(GRID_FORWARD);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_SET_GRID_RIGHT, this, CRegionView::OnSetGridOrientation);
	((CRVCallback *)pRVTracker)->SetData(GRID_RIGHT);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_SET_GRID_UP, this, CRegionView::OnSetGridOrientation);
	((CRVCallback *)pRVTracker)->SetData(GRID_UP);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the init texture space callback
	pRVTracker = new CRVCallback(UIE_INIT_TEXTURE_SPACE, this, CRegionView::OnInitTextureSpace);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the delete callback
	pRVTracker = new CRVCallback(UIE_DELETE, this, CRegionView::OnDelete);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the nudge callbacks
	pRVTracker = new CRVCallback(UIE_NUDGE_UP, this, CRegionView::OnNudge);
	((CRVCallback *)pRVTracker)->SetData(VK_UP);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	((CRVCallback *)pRVTracker)->SetToggle(1, CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_NUDGE_DOWN, this, CRegionView::OnNudge);
	((CRVCallback *)pRVTracker)->SetData(VK_DOWN);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	((CRVCallback *)pRVTracker)->SetToggle(1, CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_NUDGE_LEFT, this, CRegionView::OnNudge);
	((CRVCallback *)pRVTracker)->SetData(VK_LEFT);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	((CRVCallback *)pRVTracker)->SetToggle(1, CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_NUDGE_RIGHT, this, CRegionView::OnNudge);
	((CRVCallback *)pRVTracker)->SetData(VK_RIGHT);
	((CRVCallback *)pRVTracker)->SetToggle(0, CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	((CRVCallback *)pRVTracker)->SetToggle(1, CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
	m_cTrackerMgr.AddTracker(pRVTracker);


	// Set up the delete edge callbacks
	pRVTracker = new CRVCallback(UIE_REMOVE_EDGES, this, CRegionView::OnDeleteEdges);
	((CRVCallback *)pRVTracker)->SetData(TRUE);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVCallback(UIE_DELETE_EDGES, this, CRegionView::OnDeleteEdges);
	((CRVCallback *)pRVTracker)->SetData(FALSE);
	m_cTrackerMgr.AddTracker(pRVTracker);


	// Set up the split edge callback
	pRVTracker = new CRVCallback(UIE_SPLIT_EDGE, this, CRegionView::OnSplitEdges);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the delete tagged callback
	pRVTracker = new CRVCallback(UIE_DELETE_TAGGED_POLIES, this, CRegionView::OnDeleteTaggedPolygons);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the grid on poly callback
	pRVTracker = new CRVCallback(UIE_GRID_TO_POLY, this, CRegionView::OnGridToPoly);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the poly vert tag callback
	pRVTracker = new CRVCallback(UIE_TAG_POLY_VERTS, this, CRegionView::OnTagPolyVerts);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the flip normal callback
	pRVTracker = new CRVCallback(UIE_FLIP_NORMAL, this, CRegionView::OnFlipNormal);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the orbit vertex callback
	pRVTracker = new CRVCallback(UIE_ORBIT_VERTEX, this, CRegionView::OnOrbitVertex);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the brush rotation tracker
	pRVTracker = new CRVTrackerNodeRotate(UIE_NODE_ROTATE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the texture movement tracker
	pRVTracker = new CRVTrackerTextureMove(UIE_TEXTURE_MOVE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the brush sizing tracker
	pRVTracker = new CRVTrackerBrushSize(UIE_BRUSH_SIZE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the object sizing tracker
	pRVTracker = new CRVTrackerObjectSize(UIE_BRUSH_SIZE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the curve editing tracker
	pRVTracker = new CRVTrackerCurveEdit(UIE_CURVE_EDIT, this);
	pRVTracker->SetPhantomEnd(FALSE);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the far clipping plane tracker
	pRVTracker = new CRVTrackerFarDist(UIE_FAR_DIST, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the grid sizing tracker
	pRVTracker = new CRVTrackerGridSize(UIE_GRID_SIZE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the marker placement tracker
	pRVTracker = new CRVTrackerMarkerMove(UIE_MARKER_MOVE, this);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the marker centering tracker
	pRVTracker = new CRVTrackerMenuItem(UIE_MARKER_CENTER, this, OnCenterMarkerOnSelection, OnUpdateCenterMarkerOnSelection);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the selection centering tracker
	pRVTracker = new CRVTrackerMenuItem(UIE_CENTER_SEL_ON_MARKER, this, OnCenterSelectionOnMarker, OnUpdateCenterSelectionOnMarker);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the vertex movement trackers
 
	pRVTracker = new CRVTrackerVertMove(UIE_MOVE_VERT, this, CRVTrackerVertMove::esm_Poly);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerVertMove(UIE_MOVE_SEL_VERT, this, CRVTrackerVertMove::esm_SelVert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerVertMove(UIE_MOVE_IMM_VERT, this, CRVTrackerVertMove::esm_ImmVert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the snapping vertex movement trackers
	// Note : I couldn't find any good key combinations to allow the poly snapping versions.. :(
	/*
	pRVTracker = new CRVTrackerVertSnap("VertSnap.Edge.Poly", this, CRVTrackerVertSnap::esm_Poly, CRVTrackerVertSnap::est_Edge);
	pRVTracker->AddStartEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'E'));
	pRVTracker->AddStartEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	pRVTracker->AddEndEvent(CUIKeyEvent(UIEVENT_KEYUP, 'E'));
	m_cTrackerMgr.AddTracker(pRVTracker);
	*/

	pRVTracker = new CRVTrackerVertSnap(UIE_VERT_SNAP_EDGE_SEL, this, CRVTrackerVertSnap::esm_SelVert, CRVTrackerVertSnap::est_Edge);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerVertSnap(UIE_VERT_SNAP_EDGE, this, CRVTrackerVertSnap::esm_ImmVert, CRVTrackerVertSnap::est_Edge);
	m_cTrackerMgr.AddTracker(pRVTracker);

	/*
	pRVTracker = new CRVTrackerVertSnap("VertSnap.Vert.Poly", this, CRVTrackerVertSnap::esm_Poly, CRVTrackerVertSnap::est_Vert);
	pRVTracker->AddStartEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'V'));
	pRVTracker->AddStartEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
	pRVTracker->AddEndEvent(CUIKeyEvent(UIEVENT_KEYUP, 'V'));
	m_cTrackerMgr.AddTracker(pRVTracker);
	*/

	pRVTracker = new CRVTrackerVertSnap(UIE_VERT_SNAP_VERT_SEL, this, CRVTrackerVertSnap::esm_SelVert, CRVTrackerVertSnap::est_Vert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerVertSnap(UIE_VERT_SNAP_VERT, this, CRVTrackerVertSnap::esm_ImmVert, CRVTrackerVertSnap::est_Vert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the poly scaling tracker
	pRVTracker = new CRVTrackerPolyScale(UIE_SCALE_POLY, this, CRVTrackerPolyScale::esm_Poly);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the poly scaling tracker
	pRVTracker = new CRVTrackerVertScale(UIE_SCALE_POLY_DIVIDE, this, CRVTrackerVertScale::esm_Poly);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the vertex scaling tracker
	pRVTracker = new CRVTrackerVertScale(UIE_SCALE_VERT_SEL, this, CRVTrackerVertScale::esm_SelVert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the vertex scaling tracker
	pRVTracker = new CRVTrackerVertScale(UIE_SCALE_VERT_IMM, this, CRVTrackerVertScale::esm_ImmVert);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the extrusion tracker
	pRVTracker = new CRVTrackerExtrudePoly(UIE_EXTRUDE_POLY, this, false);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the extrusion (new brush) tracker
	pRVTracker = new CRVTrackerExtrudePoly(UIE_EXTRUDE_POLY_BRUSH, this, true);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the node movement trackers
	pRVTracker = new CRVTrackerNodeMove(UIE_MOVE_NODE_HANDLE, this, CRVTrackerNodeMove::FLAG_HANDLE);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerNodeMove(UIE_SNAP_NODE, this, CRVTrackerNodeMove::FLAG_SNAP);
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerNodeMove(UIE_MOVE_NODE_PERP, this, CRVTrackerNodeMove::FLAG_PERP);
	m_cTrackerMgr.AddTracker(pRVTracker);

	//set up the zoom tracker
	pRVTracker = new CRVTrackerZoom(UIE_ZOOM, this);
	pRVTracker->SetPhantomEnd(FALSE);
	pMouseZoomTracker = pRVTracker;
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerNavDrag(UIE_NAV_DRAG, this);
	pRVTracker->SetPhantomEnd(FALSE);
	pDragTracker = pRVTracker;
	m_cTrackerMgr.AddTracker(pRVTracker);

	pRVTracker = new CRVTrackerNavArcRotate(UIE_NAV_ARC_ROTATE, this);
	pRVTracker->SetPhantomEnd(FALSE);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the navigation movement tracker
	pRVTracker = new CRVTrackerNavMove(UIE_NAV_MOVE, this);
	pRVTracker->SetPhantomEnd(FALSE);
	pMoveTracker = pRVTracker;
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the navigation rotation tracker
	pRVTracker = new CRVTrackerNavRotate(UIE_NAV_ROTATE, this);
	pRVTracker->SetPhantomEnd(FALSE);
	pZoomTracker = pRVTracker;
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the navigation orbit tracker
	pRVTracker = new CRVTrackerNavOrbit(UIE_NAV_ORBIT, this);
	pRVTracker->SetPhantomEnd(FALSE);
	m_cTrackerMgr.AddTracker(pRVTracker);

	//handle grid resizing
	pShrinkGridTracker = new CRVTrackerMenuItem( UIE_SHRINK_GRID_SPACING, this, OnShrinkGridSpacing, OnUpdateShrinkGridSpacing );
	m_cTrackerMgr.AddTracker( pShrinkGridTracker );
	pExpandGridTracker = new CRVTrackerMenuItem( UIE_EXPAND_GRID_SPACING, this, OnExpandGridSpacing, OnUpdateExpandGridSpacing );
	m_cTrackerMgr.AddTracker( pExpandGridTracker );


	// Set up the Draw Poly callback
	pRVTracker = new CRVTrackerDrawPoly(UIE_DRAW_POLY, this);
	pRVTracker->AddOverride(pZoomTracker);
	pRVTracker->AddOverride(pMouseZoomTracker);
	pRVTracker->AddOverride(pMoveTracker);
	pRVTracker->AddOverride(pDragTracker);
	pRVTracker->AddOverride(pShrinkGridTracker);
	pRVTracker->AddOverride(pExpandGridTracker);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Set up the tagging trackers
	pRVTracker = new CRVTrackerTag(UIE_TAG_ALL, this, CRVTrackerTag::FLAG_ALL);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Un-Join selected brushes
	pRVTracker = new CRVCallbackSimple(UIE_UNJOIN_BRUSHES, this, CRegionView::OnBrushUnJoin);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Join selected brushes
	pRVTracker = new CRVCallbackSimple(UIE_JOIN_BRUSHES, this, CRegionView::OnBrushJoin);
	m_cTrackerMgr.AddTracker(pRVTracker);

	// Triangulate selected brushes..
	pRVTracker = new CRVCallbackSimple(UIE_TRI_BRUSHES, this, CRegionView::OnAutoTriangulate);
	m_cTrackerMgr.AddTracker(pRVTracker);

	/*
	pRVTracker = new CRVTrackerTag(UIE_TAG_BRUSH, this, CRVTrackerTag::FLAG_BRUSH | CRVTrackerTag::FLAG_OBJECT);
	m_cTrackerMgr.AddTracker(pRVTracker);
	*/

	//maximize select viewports
	pRVTracker = new CRVTrackerMenuItem( UIE_MAXIMIZE_VIEW1, this, OnViewMaximizeView1, OnUpdateViewMaximizeView1 );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_MAXIMIZE_VIEW2, this, OnViewMaximizeView2, OnUpdateViewMaximizeView2 );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_MAXIMIZE_VIEW3, this, OnViewMaximizeView3, OnUpdateViewMaximizeView3 );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_MAXIMIZE_VIEW4, this, OnViewMaximizeView4, OnUpdateViewMaximizeView4 );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_MAXIMIZE_ACTIVE_VIEW, this, OnViewMaximizeActiveView, OnUpdateViewMaximizeActiveView, TRUE );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//reset the viewport configuration
	pRVTracker = new CRVTrackerMenuItem( UIE_RESET_VIEWPORTS, this, On4ViewConfiguration, OnUpdate4ViewConfiguration );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//freeze the selected nodes
	pRVTracker = new CRVTrackerMenuItem( UIE_FREEZE_SELECTED, this, OnSelectionFreezeSelected, OnUpdateSelectionFreezeSelected );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//unfreeze all the nodes
	pRVTracker = new CRVTrackerMenuItem( UIE_UNFREEZE_ALL, this, OnSelectionUnfreezeAll, OnUpdateSelectionUnfreezeAll );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//texture mirroring
	pRVTracker = new CRVTrackerMenuItem( UIE_MIRROR_TEXTURE_X, this, OnMirrorTextureX, OnUpdateMirrorTextureX );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_MIRROR_TEXTURE_Y, this, OnMirrorTextureY, OnUpdateMirrorTextureY );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//texture matching
	pRVTracker = new CRVTrackerMenuItem( UIE_MATCH_TEXTURE_COORDS, this, OnMatchTextureCoords, OnUpdateMatchTextureCoords);
	m_cTrackerMgr.AddTracker( pRVTracker );

	//mirroring operations
	pRVTracker = new CRVTrackerMenuItem( UIE_MIRROR_X, this, OnSelectionMirrorX, OnUpdateSelectionMirrorX );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_MIRROR_Y, this, OnSelectionMirrorY, OnUpdateSelectionMirrorY );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_MIRROR_Z, this, OnSelectionMirrorZ, OnUpdateSelectionMirrorZ );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_ALL, this, OnSelectionSelectAll, OnUpdateSelectionSelectAll );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_INVERSE, this, OnSelectionSelectInverse, OnUpdateSelectionSelectInverse );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_CONTAINER, this, OnSelectionContainer, OnUpdateSelectionContainer );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_ADVANCED_SELECT, this, OnSelectionAdvanced, OnUpdateSelectionAdvanced );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SCALE_SELECTION, this, OnSelectionScale, OnUpdateSelectionScale );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SAVE_AS_PREFAB, this, OnSelectionSavePrefab, OnUpdateSelectionSavePrefab );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_GENERATE_UNIQUE_NAMES, this, OnSelectionGenerateUniqueNames, OnUpdateSelectionGenerateUniqueNames );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_GROUP_SELECTION, this, OnSelectionGroup, OnUpdateSelectionGroup );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_FLIP_BRUSH, this, OnBrushFlip, OnUpdateBrushFlip );
	m_cTrackerMgr.AddTracker( pRVTracker );
	
	//handle redo/undo operations
	pRVTracker = new CRVTrackerMenuItem( UIE_EDIT_UNDO, this, OnEditUndo, OnUpdateEditUndo );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_EDIT_REDO, this, OnEditRedo, OnUpdateEditRedo );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//setup the trackers for the different views
	pRVTracker = new CRVTrackerMenuItem( UIE_TOP_VIEW, this, OnTopView, OnUpdateTopView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_BOTTOM_VIEW, this, OnBottomView, OnUpdateBottomView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_LEFT_VIEW, this, OnLeftView, OnUpdateLeftView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_RIGHT_VIEW, this, OnRightView, OnUpdateRightView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_FRONT_VIEW, this, OnFrontView, OnUpdateFrontView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_BACK_VIEW, this, OnBackView, OnUpdateBackView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_PERSPECTIVE_VIEW, this, OnPerspectiveView, OnUpdatePerspectiveView );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//navigator trackers
	pRVTracker = new CRVTrackerMenuItem( UIE_NAVIGATOR_STORE, this, OnNavigatorStore, OnUpdateNavigatorStore );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_NAVIGATOR_ORGANIZE, this, OnNavigatorOrganize, OnUpdateNavigatorOrganize );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_NAVIGATOR_NEXT, this, OnNavigatorGotoNext, OnUpdateNavigatorGotoNext );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_NAVIGATOR_PREVIOUS, this, OnNavigatorGotoPrevious, OnUpdateNavigatorGotoPrevious );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle adding objects
	pRVTracker = new CRVTrackerMenuItem( UIE_ADD_OBJECT, this, OnAddObject, OnUpdateAddObject );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle the different shade modes
	pRVTracker = new CRVTrackerMenuItem( UIE_SHADE_FLAT, this, OnShadeFlat, OnUpdateShadeFlat );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHADE_WIREFRAME, this, OnShadeWireframe, OnUpdateShadeWireframe );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHADE_TEXTURES, this, OnShadeTextured, OnUpdateShadeTextured );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHADE_LIGHTMAPS, this, OnShadeLightmapsOnly, OnUpdateShadeLightmapsOnly );
	m_cTrackerMgr.AddTracker( pRVTracker );


	//surface applicators
	pRVTracker = new CRVTrackerMenuItem( UIE_APPLY_COLOR, this, OnApplyColor, OnUpdateApplyColor );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_TEXTURE, this, OnSelectTexture, OnUpdateSelectTexture );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_BRUSH_COLOR, this, OnSelectBrushColor, OnUpdateSelectBrushColor );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SELECT_PREFAB, this, OnSelectPrefab, OnUpdateSelectPrefab );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle hiding
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_INVERSE, this, OnSelectionHideInverse, OnUpdateSelectionHideInverse );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_UNHIDE_INVERSE, this, OnSelectionUnhideInverse, OnUpdateSelectionUnhideInverse );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_SELECTED, this, OnSelectionHideSelected, OnUpdateSelectionHideSelected );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_UNHIDE_SELECTED, this, OnSelectionUnhideSelected, OnUpdateSelectionUnhideSelected );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle rotation of objects
	pRVTracker = new CRVTrackerMenuItem( UIE_ROTATE_SELECTION, this, OnRotateSelection, OnUpdateRotateSelection );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVCallback(UIE_ROTATE_SELECTION_LEFT, this, CRegionView::OnNudgeRotate);
	((CRVCallback *)pRVTracker)->SetData(VK_LEFT);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVCallback(UIE_ROTATE_SELECTION_RIGHT, this, CRegionView::OnNudgeRotate);
	((CRVCallback *)pRVTracker)->SetData(VK_RIGHT);
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle binding brushes to objects
	pRVTracker = new CRVTrackerMenuItem( UIE_BIND_TO_OBJECT, this, OnBindToObject, OnUpdateBindToObject );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//handle the model showing keys
	pRVTracker = new CRVTrackerMenuItem( UIE_SHOW_ALL_MODELS, this, OnDisplayAllModels, OnUpdateDisplayAllModels);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_ALL_MODELS, this, OnHideAllModels, OnUpdateHideAllModels);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHOW_SELECTED_MODELS, this, OnDisplaySelectedModels, OnUpdateDisplaySelectedModels);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_SELECTED_MODELS, this, OnHideSelectedModels, OnUpdateHideSelectedModels);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHOW_MODELS_OF_CLASS, this, OnDisplayModelsOfClass, OnUpdateDisplayModelsOfClass);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_MODELS_OF_CLASS, this, OnHideModelsOfClass, OnUpdateHideModelsOfClass);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_SHOW_MODEL_POLYCOUNT, this, OnDisplayModelPolycount, OnUpdateDisplayModelPolycount);
	m_cTrackerMgr.AddTracker( pRVTracker );

	//the trackers for creating primitives
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_BOX, this, OnCreatePrimitiveBox, OnUpdateCreatePrimitiveBox);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_DOME, this, OnCreatePrimitiveDome, OnUpdateCreatePrimitiveDome);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_SPHERE, this, OnCreatePrimitiveSphere, OnUpdateCreatePrimitiveSphere);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_PLANE, this, OnCreatePrimitivePlane, OnUpdateCreatePrimitivePlane);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_PYRAMID, this, OnCreatePrimitivePyramid, OnUpdateCreatePrimitivePyramid);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_CREATE_CYLINDER, this, OnCreatePrimitiveCylinder, OnUpdateCreatePrimitiveCylinder);
	m_cTrackerMgr.AddTracker( pRVTracker );



	//setup the trackers for the different modes
	pRVTracker = new CRVCallbackSimple(UIE_OBJECT_EDIT_MODE, this, CRegionView::SetObjectEditMode);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVCallbackSimple(UIE_GEOMETRY_EDIT_MODE, this, CRegionView::SetGeometryEditMode);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVCallbackSimple(UIE_BRUSH_EDIT_MODE, this, CRegionView::SetBrushEditMode);
	m_cTrackerMgr.AddTracker( pRVTracker );

	//texturing
	pRVTracker = new CRVCallbackSimple( UIE_NEXT_TEXTURE_LAYER, this, CRegionView::NextTextureLayer);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerTextureWrap( UIE_TEXTURE_WRAP, this);
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_MAP_TEXTURE_COORDS, this, OnMapTextureCoords, OnUpdateMapTextureCoords );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_MAP_TEXTURE_TO_VIEW, this, OnMapTextureToView, OnUpdateMapTextureToView );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_RESET_TEXTURE_COORDS, this, OnResetTextureCoords, OnUpdateResetTextureCoords );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_REPLACE_TEXTURES, this, OnReplaceTextures, OnUpdateReplaceTextures );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_REPLACE_TEXTURES_IN_SEL, this, OnReplaceTexturesInSel, OnUpdateReplaceTexturesInSel );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_FIT_TEXTURE_TO_POLY, this, OnFitTextureToPoly, OnUpdateFitTextureToPoly );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_TOGGLE_CLASS_ICONS, this, OnToggleClassIcons, OnUpdateToggleClassIcons );
	m_cTrackerMgr.AddTracker( pRVTracker );
	pRVTracker = new CRVTrackerMenuItem( UIE_HIDE_FROZEN_NODES, this, OnHideFrozenNodes, OnUpdateHideFrozenNodes );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_CAMERA_TO_OBJECT, this, OnCameraToObject, OnUpdateCameraToObject );
	m_cTrackerMgr.AddTracker( pRVTracker );

	pRVTracker = new CRVTrackerMenuItem( UIE_DISCONNECT_SELECTED_PREFABS, this, OnDisconnectSelectedPrefabs, OnUpdateDisconnectSelectedPrefabs );
	m_cTrackerMgr.AddTracker( pRVTracker );

	//class icons
	pRVTracker = new CRVTrackerMenuItem( UIE_TOGGLE_CLASS_ICONS, this, OnToggleClassIcons, OnUpdateToggleClassIcons);
	m_cTrackerMgr.AddTracker( pRVTracker );

}
