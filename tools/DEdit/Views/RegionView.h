// RegionView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRegionView view


#ifndef __REGIONVIEW_H__
#define __REGIONVIEW_H__


	#include "editgrid.h"
	#include "editpoly.h"
	#include "editregion.h"
	#include "editray.h"
	#include "navigator.h"
	#include "editbrush.h"
	#include "refs.h"
	#include "viewrender.h"
	#include "trackers\trackermgr.h"


	// In ViewFunctions.cpp
	void SetupBrushProperties(CEditBrush *pBrush, CEditRegion *pRegion);


	// Useful definitions.
	#define BEGIN_MEMORYEXCEPTION()		try {
	#define END_MEMORYEXCEPTION()		} catch( CMemoryException * x ) { GiveMemoryWarning(); x->Delete();}

	// Edit modes.
	#define BRUSH_EDITMODE						0
	#define GEOMETRY_EDITMODE					1
	#define OBJECT_EDITMODE						2


	// Shade modes.
	#define SM_WIREFRAME						0
	#define SM_FLAT								1
	#define SM_TEXTURED							2
	#define SM_LIGHTMAPSONLY					3

	// Updating hints..
	#define REGIONVIEWUPDATE_EDITMODE			101
	#define REGIONVIEWUPDATE_REDRAW				102
	#define REGIONVIEWUPDATE_VIEWID				0x8000

	// Macro for creating a view ID update
	#define REGIONVIEWUPDATE_VIEW(a)			(REGIONVIEWUPDATE_VIEWID | (a))
	
	// The number of move/size handles
	#define NUM_BOX_HANDLES						18

	enum EditState
	{
		EDIT_NOSTATE,
		EDIT_DRAWINGPOLY
	};

	// Grid directions
	#define GRID_FORWARD						0
	#define GRID_RIGHT							1
	#define GRID_UP								2


	// Defines....
	class CRegionDoc;
	class CEditPoly;
	class CObjInterface;



	class CRegionView : public CView, public CViewRender
	{
	public:
		DECLARE_DYNCREATE(CRegionView)
		CRegionView();           // protected constructor used by dynamic creation
		virtual ~CRegionView();


		// Called by the Workspace doc to save some state info.
		void						LoadProjFile( CAbstractIO &file );
		void						SaveProjFile( CAbstractIO &file );


		void UpdateStatusBar();
		virtual BOOL PreTranslateMessage(MSG* pMsg);


	// Accessors
	public:

		// Set/Get the grid spacing
		DWORD						GetGridSpacing()				{ return m_dwGridSpacing; }
		void						SetGridSpacing( DWORD size )	{ m_dwGridSpacing = DMAX(size, 1); DrawRect(); }

		DWORD						GetMajorGridSpacing()				{ return m_dwMajorGridSpacing; }
		void						SetMajorGridSpacing(DWORD dwSize)	{ m_dwMajorGridSpacing=dwSize; DrawRect(); }

		float						GetGridSize()					{ return EditGrid().m_DrawSize; }
		void						SetGridSize( float fSize )		{ EditGrid().m_DrawSize= fSize; DrawRect(); }
		
		CRegionDoc*					GetRegionDoc()	{ return (CRegionDoc*)GetDocument(); }
		CEditRegion*				GetRegion()		{ return m_pRegion; }

		void						DeleteSelectedNodes();
	
	// View-ish stuff.
	public:
	
		void						CheckAMenuItem( CMenu *pMenu, UINT nItem, int testVal1, int testVal2 );
		void						EnableAMenuItem( CMenu *pMenu, UINT nItem, int testVal1, int testVal2 );

		BOOL						OnIdle( LONG lCount );
		CPoint						GetCurMousePos();
		

		uint32						GetShadeMode() const;
		void						SetShadeMode( int mode );

		BOOL						IsSelectBackfaces() const;
		void						SetSelectBackfaces( BOOL bBackface );

		BOOL						IsShowNormals() const;
		void						SetShowNormals( BOOL bShow );

		BOOL						IsShowWireframe() const;
		void						SetShowWireframe( BOOL bShow );

		BOOL						IsShowGrid() const;
		void						SetShowGrid(BOOL bShow);

		BOOL						IsShowObjects() const;
		void						SetShowObjects(BOOL bShow);

		BOOL						IsShowMarker() const;
		void						SetShowMarker(BOOL bShow);

		int							GetEditMode();

		//used to notify when the hot keys have been changed
		void						OnHotKeysChanged();

		// Switch the navigator to one of the stored navigators for this level
		void						SwitchNavigator(int nNavIndex);

		// DrawRect override for focus drawing
		void						DrawRect(CRect *pRect=NULL);

		// Get the rendering rectangle
		void						GetRenderRect(LPRECT pRect, BOOL bClient = TRUE);

		BOOL						GetInFocus() const { return m_bInFocus; };
		void						SetInFocus(BOOL bInFocus) { m_bInFocus = bInFocus; };

		//given a list of polygons, it will back up all the nodes associated with them
		//and set up the undo
		void						MakeUndoForPolygons(CMoArray<CEditPoly*>* pPolyList);
	

	// Trackers and tracker functions.
	public:

		void						InitTracker();

		// Tracker command callbacks
		BOOL						OnDepthSelect(CUITracker *pTracker);
		BOOL						OnApplyTextureClick(CUITracker *pTracker);
		BOOL						OnSetGridOrientation(CUITracker *pTracker);
		BOOL						OnInitTextureSpace(CUITracker *pTracker);
		BOOL						OnSelectNone(CUITracker *pTracker);
		BOOL						OnDelete(CUITracker *pTracker);
		BOOL						OnNudge(CUITracker *pTracker);
		BOOL						OnNudgeRotate(CUITracker *pTracker);
		BOOL						OnSplitEdges(CUITracker *pTracker);
		BOOL						OnDeleteTaggedPolygons(CUITracker *pTracker);
		BOOL						OnDeleteEdges(CUITracker *pTracker);
		BOOL						OnGridToPoly(CUITracker *pTracker);
		BOOL						OnTagPolyVerts(CUITracker *pTracker);
		BOOL						OnFlipNormal(CUITracker *pTracker);
		BOOL						OnOrbitVertex(CUITracker *pTracker);

		BOOL						OnSelectLightNode(CUITracker *pTracker);
		BOOL						OnEditLightNode(CUITracker *pTracker);

	// Tagging stuff.
	public:
		
		// Used to help with immediate selection.
		BOOL						GetClosestPoint( CPoint &point, CVector &vClosest, BOOL bSelectedOnly, CReal *pDist = NULL );

		CVertRef					GetClosestVert( CEditBrush *pBrush, CPoint point, CReal *pDist=NULL );
		CVertRef					GetClosestVert( CPoint &point, BOOL bSelectedOnly, CReal *pDist=NULL, bool bIgnoreFrozen = true );
		CVertRef					GetClosestVert( CVertRefArray &vertArray, CPoint point, CReal *pDist=NULL, bool bIgnoreFrozen = true );
		
		CEdgeRef					GetClosestEdge( CPoint &point, BOOL bSelectedOnly = FALSE );
		CEdgeRef					GetClosestEdge( CEditBrush *pBrush, CPoint point, CReal *pDist=NULL );

		CPolyRef					CastRayAtPolies( CEditRay &ray, CReal *pDist=NULL, CReal rMinDist = 0.0f, BOOL bSelectedOnly = FALSE );
		CPolyRef					CastRayAtPolies( CEditBrush *pBrush, CEditRay &ray, CReal *pDist=NULL );

		CWorldNode*					GetClosestObject(const CPoint& point, BOOL bSelectedOnly, CReal *pDist=NULL, BOOL bFilter = FALSE );



		// Makes a tag list out of either the immediate selection or the current tag list.
		// Returns TRUE if there's anything in the list.
		BOOL						GetSelectedPoly( CVertRefArray &list );
		BOOL						GetSelectedVerts( CVertRefArray &list );
		BOOL						GetImmediateVert( CVertRefArray &list );
		CBrushRef					GetMouseOverBrush( CReal *pDist = NULL, CReal rMinDist = 0.0f );
		BOOL						GetSelectedBrushes( CVertRefArray &list );
		BOOL						MakeTaggedEdgeList( CEdgeRefArray &list );

		// Tags all points/edges/polies in the tag rect.
		void						TagPointsInRect( CRect &rect );

		// Tags all points/edges/polies in the tag rect.
		void						GetObjectsInRect( CRect &rect, CMoArray< CWorldNode * > &objects, BOOL bFilter = FALSE );

		// Tags/untags the given object.
		void						ToggleVertexTag( CVertRef ref );
		void						ToggleEdgeTag( CEdgeRef edge );

		GenList<CVertRef>&			TaggedVerts()	{ return GetRegionDoc()->m_TaggedVerts; }
		CBrushRefArray&				TaggedBrushes()	{ return GetRegionDoc()->m_TaggedBrushes; }
		CPolyRefArray&				TaggedPolies() {return GetRegionDoc()->m_TaggedPolies;}
		
		CVertRef&					IVert()		{ return GetRegionDoc()->m_ImmediateVertexTag; }
		CEdgeRef&					IEdge()		{ return GetRegionDoc()->m_ImmediateEdgeTag; }
		CBrushRef&					IBrush()	{ return GetRegionDoc()->m_ImmediateBrushTag; }
		CPolyRef&					IPoly()		{ return GetRegionDoc()->m_TaggedPolies[0]; }
		
		CEditBrush&					DrawingBrush()	{ return GetRegionDoc()->m_DrawingBrush; }

	// UI helpers.
	public:

		//determines if any polygons are selected for editing. This occurs when
		//in geometry mode and the cursor is over a face, or if it is brush mode and
		//a brush is selected
		BOOL						AreAnyPoliesSelected();

		// Gets the current texture name from the controls.
		BOOL						GetCurrentTextureName( char* &pTextureName, BOOL bUseSprites=FALSE );

		// Gives a low memory warning.
		void						GiveMemoryWarning();

		// These will update any polies that contain any of these vertices.
		void						UpdatePlanesOfPolies( CVertRefArray &verts );
		
		// Finds the intersection with the right EditPlane.
		BOOL						GetVertexFromPoint( CPoint point, CVector &vertex, BOOL bSnapToGrid = TRUE );

		// Updates the current immediate selection.
		// Returns TRUE if it's changed.
		BOOL						UpdateImmediateSelection();

		// Updates the cursor to be a different one if necessary.
		void						UpdateCursor();

		// Gets all relevant information for moving selection handles.
		BOOL						GetHandleInfo( int theHandle, HCURSOR *pCursor, CVector &scaleNormal, CVector &scaleOrigin );

		// Gets the current selection box handle that the mouse is over.
		// -1 if none.
		int32						GetCurrentMouseOverHandle();

		//Returns the active region.  If none is active, just return the first region 
		//in the active document.
		CEditRegion*				GetActiveRegion();

		//  maximizes a view.  0 = upper left, 1 = upper right, 2 = lower left, 3 = lower right
		void						MaximizeView( uint32 nViewIndex );	

		// Tells if the given key is down.  Pass it in uppercase.
		BOOL						IsKeyDown( int key );
		
		// Updates all current trackers.
		void						UpdateTrackers( CPoint &point );
		
		// Splits any edges in the selection list.
		void						SplitEdges();			

		// Deletes any tagged vertices or polygons
		void						DeleteTaggedVertices();
		void						DeleteTaggedEdges( BOOL bRemoveOriginalPolyPoints );
		void						DeleteTaggedPolygons();

		// Does a carve operation (boolean subtraction).
		void						DoCarveOperation();

		// Does a flip operation (reverses polygon winding order).
		void						DoFlipOperation();

		// Offsets (moves them in the world) the selected nodes by a vector
		void						OffsetSelectedNodes(CVector vOffset);

		// Offsets the texture map for the selected polygon in the specified
		// direction (use 1 for positive, -1 for negative, or zero for no
		// movement).  The grid size is used to offset the texture.  If SHIFT
		// is pressed, then the texture is moved one unit at a time.
		void						OffsetTextureForPoly(int nPDirection, int nQDirection, BOOL bGrid = FALSE);

		// Scale the texture map for the selected polygon in the specified
		// direction (use 1 to expand, -1 to shrink, or zero for neither)
		// The grid size is used to scale the texture.
		void						ScaleTextureForPoly(int nPDirection, int nQDirection);

		// Mirrors the selected nodes along one of the axis specified in vAxis.
		// vAxis must either be (1, 0, 0) (0,1,0) or (0,0,1)
		void						MirrorSelectedNodes(CVector vAxis);

		// Returns TRUE if we are in a situation in which we can split a brush.
		// pnLastSelectedBrushIndex is filled in with the last selected brush index.
		BOOL						CanSplitBrush(int *pnLastSelectedBrush=NULL);

		// Generates unique names for the selected objects
		//
		// bUpdateRefProps		- Updates properties for objects that reference
		//						  other objects that have had their names change
		// bUpdateSelPropsOnly	- Only objects that are selected will have their
		//						  properties updated
		// bDisplayReport		- A dialog reporting the name changes is displayed
		// bGenerateUndoInfo	- Undo information is generated and added to the document
		void						GenerateUniqueNamesForSelected(BOOL bUpdateRefProps, BOOL bUpdateSelPropsOnly, BOOL bDisplayReport=FALSE, BOOL bGenerateUndoInfo=TRUE);

		// Extrudes the given polygon.
		bool						ExtrudePoly( CPolyRef poly, CVertRefArray &newVerts, bool bRemoveOriginal, bool bFlipAll, bool bNewBrush = false);

		// Clears the given selection categories.
		void						ClearSelections( BOOL bClearVerts=TRUE, BOOL bClearEdges=TRUE, BOOL bClearPolies=TRUE, BOOL bClearBrushes=TRUE );
		
		// Finishes each drawing operation.
		void						FinishDrawingPoly();
		void						SetupDrawingBrush( CEditBrush* pNewBrush );

		// Splits the polygon along the 'edge' defined by indices.
		void						SplitPolyWithEdge( CPolyRef poly, CEditVertArray &verts );

		// Import terrain map
		void						DoImportTerrainMap( CEditBrush *pBrush, CString &pathName );

		// Create primitives
		void						DoCreatePrimitiveBox( CVector &vCenter, CReal fSide );
		void						DoCreatePrimitiveCylinder( CVector &vCenter, int nNumSides, CReal fHeight, CReal fRadius );
		void						DoCreatePrimitivePyramid( CVector &vCenter, int nNumSides, CReal fHeight, CReal fRadius );
		void						DoCreatePrimitiveSphere(CVector &vCenter, int nSubdivisionsX, int nSubdivisionsY, CReal fRadius, BOOL bDome=FALSE);
		void						DoCreatePrimitivePlane(const LTVector& vBasis, const LTVector& vRight, const LTVector& vUp, float fWidth, float fHeight, bool bSquare);

		void						GetSelectedPolies(CMoArray<CEditPoly*> &polies);

		// This imports a file into the region and optionally positions it at
		// the green crosshair location.
		//
		// Returns: TRUE if the file import was a success.
		BOOL						ImportLTAFile(CString sFilename, BOOL bPositionAtCrosshair);
		BOOL						ImportOBJFile(CString sFilename, BOOL bPositionAtCrosshair);
		bool						CreateBrushesFromScene(CEditRegion *pRegion, CObjInterface *pScene );

		//given a user input event name and a menu handler, it will set up the text for
		//the menu item
		static void					SetMenuHotKeyText(const char* pszUIE, CCmdUI* pCmdID);

		// Does a recursive search through the node tree for nodes with a node flag, adding them to the actionList.  Used for undo.
		void						UndoHelper(PreActionList *actionList, CWorldNode *node, const uint32 flag=0, const bool wantFlag=true) const;

	private:
		// Helper function for GetObjectsInRect
		void RecurseAndGetObjectsInRect(CRect &rect, CMoArray< CWorldNode * > &objects, CWorldNode *pRoot, BOOL bFilter);
		// Helper function for GetClosestObject
		CWorldNode *RecurseAndGetClosestObject(const CPoint &point, BOOL bSelectedOnly, CWorldNode *pRoot, SDWORD &nDistMin, float &fViewMin, BOOL bFilter);

	// Operations
	public:

		//these are callback functions for the trackers
		void						SetObjectEditMode();
		void						SetGeometryEditMode();
		void						SetBrushEditMode();
		void						NextTextureLayer();

		//handles replacing the texture specified by the immediate poly with the currently
		//active texture
		void						ReplaceTextures(bool bSelectionOnly);

	// Overrides
	
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CRegionView)
	public:
		virtual void OnInitialUpdate();
		virtual void OnDraw(CDC* pDC);      // overridden to draw this view
		virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
		virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
		virtual void OnActivateFrame( UINT nState, CFrameWnd* pFrameWnd );
		//}}AFX_VIRTUAL

	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif

		// Generated message map functions
	public:
		//{{AFX_MSG(CRegionView)
		afx_msg void OnShrinkGridSpacing();
		afx_msg void OnUpdateShrinkGridSpacing( CCmdUI *pCmdUI );
		afx_msg void OnExpandGridSpacing();
		afx_msg void OnUpdateExpandGridSpacing( CCmdUI *pCmdUI );
		afx_msg void OnApplyColor();
		afx_msg void OnUpdateApplyColor( CCmdUI *pCmdUI );
		afx_msg void OnApplyTexture();
		afx_msg void OnUpdateApplyTexture( CCmdUI *pCmdUI );
		afx_msg void OnRemoveTexture();
		afx_msg void OnUpdateRemoveTexture( CCmdUI *pCmdUI );
		afx_msg void OnImportTerrainMap();
		afx_msg void OnCreatePrimitiveBox();
		afx_msg void OnUpdateCreatePrimitiveBox( CCmdUI *pCmdUI );
		afx_msg void OnCreatePrimitiveCylinder();
		afx_msg void OnUpdateCreatePrimitiveCylinder( CCmdUI *pCmdUI );
		afx_msg void OnCreatePrimitivePyramid();
		afx_msg void OnUpdateCreatePrimitivePyramid( CCmdUI *pCmdUI );
		afx_msg void OnCreatePrimitiveSphere();
		afx_msg void OnUpdateCreatePrimitiveSphere( CCmdUI *pCmdUI );
		afx_msg void OnCreatePrimitivePlane();
		afx_msg void OnUpdateCreatePrimitivePlane( CCmdUI *pCmdUI );
		afx_msg void OnCreatePrimitiveDome();
		afx_msg void OnUpdateCreatePrimitiveDome( CCmdUI *pCmdUI );
		afx_msg void OnUpdateSingleBrushOperation( CCmdUI *pCmdUI );
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnDestroy();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
		afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnAlignPolyTextures();
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void OnContextMenu(CWnd*, CPoint point);
 		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnAutoTriangulate();
		afx_msg void OnStartPreprocessor();
		afx_msg void OnKillFocus(CWnd* pNewWnd);
		afx_msg void OnSetFocus(CWnd* pOldWnd);
		afx_msg void OnRemoveExtraEdges();
		afx_msg void OnMapTextureCoords();
		afx_msg void OnUpdateMapTextureCoords(CCmdUI* pCmdUI);
		afx_msg void OnRotateSelection();
		afx_msg void OnUpdateRotateSelection(CCmdUI* pCmdUI);
		afx_msg void OnResetAllTextureCoords();
		afx_msg void OnEditUndo();
		afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
		afx_msg void OnEditRedo();
		afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
		afx_msg void OnJoinTaggedVertices();
		afx_msg void OnObjectHeight();
		afx_msg void OnBindToObject();
		afx_msg void OnUpdateBindToObject(CCmdUI* pCmdUI);
		afx_msg void OnSnapVertices();
		afx_msg void OnUpdateSnapVertices(CCmdUI* pCmdUI);
		afx_msg void OnAddObject();
		afx_msg void OnUpdateAddObject(CCmdUI* pCmdUI);
		afx_msg void OnShowWireframe();
		afx_msg void OnUpdateShowWireframe(CCmdUI* pCmdUI);
		afx_msg void OnShowObjects();
		afx_msg void OnUpdateShowObjects(CCmdUI* pCmdUI);
		afx_msg void OnUpdateGeometryEditing(CCmdUI* pCmdUI);
		afx_msg void OnUpdateObjectEditing(CCmdUI* pCmdUI);
		afx_msg void OnObjectProperties();
		afx_msg void OnUpdateObjectProperties(CCmdUI* pCmdUI);
		afx_msg void OnWorldInfo();
		afx_msg void OnShadeWireframe();
		afx_msg void OnUpdateShadeWireframe(CCmdUI* pCmdUI);
		afx_msg void OnShadeTextured();
		afx_msg void OnUpdateShadeTextured(CCmdUI* pCmdUI);
		afx_msg void OnShadeFlat();
		afx_msg void OnUpdateShadeFlat(CCmdUI* pCmdUI);
		afx_msg void OnShadeLightmapsOnly();
		afx_msg void OnUpdateShadeLightmapsOnly(CCmdUI* pCmdUI);
		afx_msg void OnShowNormals();
		afx_msg void OnBackfacingPolies();
		afx_msg void OnBackView();
		afx_msg void OnUpdateBackView(CCmdUI* pCmdUI);
		afx_msg void OnBottomView();
		afx_msg void OnUpdateBottomView(CCmdUI* pCmdUI);
		afx_msg void OnFrontView();
		afx_msg void OnUpdateFrontView(CCmdUI* pCmdUI);
		afx_msg void OnLeftView();
		afx_msg void OnUpdateLeftView(CCmdUI* pCmdUI);
		afx_msg void OnPerspectiveView();
		afx_msg void OnUpdatePerspectiveView(CCmdUI* pCmdUI);
		afx_msg void OnRightView();
		afx_msg void OnUpdateRightView(CCmdUI* pCmdUI);
		afx_msg void OnTopView();
		afx_msg void OnUpdateTopView(CCmdUI* pCmdUI);
		afx_msg void OnProcessWorld();
		afx_msg void OnFileSave();
		afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
		afx_msg void OnCleanupGeometry();
		afx_msg void OnShowEditGrid();
		afx_msg void OnShowMarker();
		afx_msg void On4ViewConfiguration();
		afx_msg void OnUpdate4ViewConfiguration(CCmdUI* pCmdUI);
		afx_msg void OnDisplayAllModels();
		afx_msg void OnUpdateDisplayAllModels(CCmdUI* pCmdUI);
		afx_msg void OnHideAllModels();
		afx_msg void OnUpdateHideAllModels(CCmdUI* pCmdUI);
		afx_msg void OnDisplaySelectedModels();
		afx_msg void OnUpdateDisplaySelectedModels(CCmdUI* pCmdUI);
		afx_msg void OnHideSelectedModels();
		afx_msg void OnUpdateHideSelectedModels(CCmdUI* pCmdUI);
		afx_msg void OnDisplayModelsOfClass();
		afx_msg void OnUpdateDisplayModelsOfClass(CCmdUI* pCmdUI);
		afx_msg void OnHideModelsOfClass();
		afx_msg void OnUpdateHideModelsOfClass(CCmdUI* pCmdUI);
		afx_msg void OnDisplayModelPolycount();
		afx_msg void OnUpdateDisplayModelPolycount(CCmdUI* pCmdUI);
		afx_msg void OnUpdateStatus( CCmdUI* pCmdUI );
		afx_msg void OnUseDevice1();
		afx_msg void OnUseDevice2();
		afx_msg void OnUseDevice3();
		afx_msg void OnUseDevice4();
		afx_msg void OnRenderMode1();
		afx_msg void OnRenderMode2();
		afx_msg void OnRenderMode3();
		afx_msg void OnRenderMode4();
		afx_msg void OnReplaceTextures();
		afx_msg void OnUpdateReplaceTextures(CCmdUI* pCmdUI);
		afx_msg void OnReplaceTexturesInSel();
		afx_msg void OnUpdateReplaceTexturesInSel(CCmdUI* pCmdUI);
		afx_msg void OnSelectTexture();
		afx_msg void OnUpdateSelectTexture(CCmdUI* pCmdUI);
		afx_msg void OnSelectPrefab();
		afx_msg void OnUpdateSelectPrefab(CCmdUI* pCmdUI);
		afx_msg void OnSelectBrushColor();
		afx_msg void OnUpdateSelectBrushColor(CCmdUI* pCmdUI);
		afx_msg void OnResetDetailLevels();
		afx_msg void OnUpdateAllObjectProperties();
		afx_msg void OnNavigatorStore();
		afx_msg void OnUpdateNavigatorStore(CCmdUI* pCmdUI);
		afx_msg void OnNavigatorOrganize();
		afx_msg void OnUpdateNavigatorOrganize(CCmdUI* pCmdUI);
		afx_msg void OnNavigatorGotoNext();
		afx_msg void OnUpdateNavigatorGotoNext(CCmdUI* pCmdUI);
		afx_msg void OnNavigatorGotoPrevious();
		afx_msg void OnUpdateNavigatorGotoPrevious(CCmdUI* pCmdUI);
		afx_msg void OnSelectionSelectAll();
		afx_msg void OnUpdateSelectionSelectAll(CCmdUI* pCmdUI);
		afx_msg void OnSelectionSelectNone();
		afx_msg void OnUpdateSelectionSelectNone(CCmdUI* pCmdUI);
		afx_msg void OnSelectionSelectInverse();
		afx_msg void OnUpdateSelectionSelectInverse(CCmdUI* pCmdUI);
		afx_msg void OnSelectionAdvanced();
		afx_msg void OnUpdateSelectionAdvanced(CCmdUI* pCmdUI);
		afx_msg void OnSelectionHideSelected();
		afx_msg void OnUpdateSelectionHideSelected(CCmdUI* pCmdUI);
		afx_msg void OnSelectionUnhideSelected();
		afx_msg void OnUpdateSelectionUnhideSelected(CCmdUI* pCmdUI);
		afx_msg void OnSelectionGroup();
		afx_msg void OnUpdateSelectionGroup(CCmdUI* pCmdUI);
		afx_msg void OnWorldImportObjects();
		afx_msg void OnBrushSplitBrush();
		afx_msg void OnUpdateBrushSplitBrush(CCmdUI* pCmdUI);
		afx_msg void OnBrushCarve();
		afx_msg void OnUpdateBrushCarve(CCmdUI* pCmdUI);
		afx_msg void OnBrushJoin();
		afx_msg void OnBrushUnJoin();
		afx_msg void OnSetCameraFOV();
		afx_msg void OnBrushFlip();
		afx_msg void OnUpdateBrushFlip(CCmdUI* pCmdUI);
		afx_msg void OnSelectionMirrorX();
		afx_msg void OnUpdateSelectionMirrorX(CCmdUI* pCmdUI);
		afx_msg void OnSelectionMirrorY();
		afx_msg void OnUpdateSelectionMirrorY(CCmdUI* pCmdUI);
		afx_msg void OnSelectionMirrorZ();
		afx_msg void OnUpdateSelectionMirrorZ(CCmdUI* pCmdUI);
		afx_msg void OnSelectionContainer();
		afx_msg void OnUpdateSelectionContainer(CCmdUI* pCmdUI);
		afx_msg void OnCenterSelectionOnMarker();
		afx_msg void OnUpdateCenterSelectionOnMarker(CCmdUI* pCmdUI);
		afx_msg void OnCenterMarkerOnSelection();
		afx_msg void OnUpdateCenterMarkerOnSelection(CCmdUI* pCmdUI);
		afx_msg void OnPlaceMarkerAtCamera();
		afx_msg void OnUpdatePlaceMarkerAtCamera(CCmdUI* pCmdUI);
		afx_msg void OnSelectionGenerateUniqueNames();
		afx_msg void OnUpdateSelectionGenerateUniqueNames(CCmdUI* pCmdUI);
		afx_msg void OnWorldObjectBrowser();
		afx_msg void OnPlaceMarkerAtVector();
		afx_msg void OnWorldDebugFindNamingConflicts();
		afx_msg void OnWorldScaleGeometry();
		afx_msg void OnMapTextureToView();
		afx_msg void OnUpdateMapTextureToView(CCmdUI* pCmdUI);
		afx_msg void OnResetTextureCoords();
		afx_msg void OnUpdateResetTextureCoords(CCmdUI* pCmdUI);
		afx_msg void OnSelectionHideInverse();
		afx_msg void OnUpdateSelectionHideInverse(CCmdUI* pCmdUI);
		afx_msg void OnSelectionUnhideInverse();
		afx_msg void OnUpdateSelectionUnhideInverse(CCmdUI* pCmdUI);
		afx_msg void OnFileImportWorld();
		afx_msg void OnSelectionSavePrefab();
		afx_msg void OnSelectionScale();
		afx_msg void OnUpdateSelectionSavePrefab(CCmdUI* pCmdUI);
		afx_msg void OnUpdateSelectionScale(CCmdUI* pCmdUI);
		afx_msg void OnViewMaximizeView1();
		afx_msg void OnUpdateViewMaximizeView1(CCmdUI* pCmdUI);
		afx_msg void OnViewMaximizeView2();
		afx_msg void OnUpdateViewMaximizeView2(CCmdUI* pCmdUI);
		afx_msg void OnViewMaximizeView3();
		afx_msg void OnUpdateViewMaximizeView3(CCmdUI* pCmdUI);
		afx_msg void OnViewMaximizeView4();
		afx_msg void OnUpdateViewMaximizeView4(CCmdUI* pCmdUI);
		afx_msg void OnViewMaximizeActiveView();
		afx_msg void OnUpdateViewMaximizeActiveView(CCmdUI* pCmdUI);
		afx_msg void OnSelectionFreezeSelected();
		afx_msg void OnUpdateSelectionFreezeSelected(CCmdUI* pCmdUI);
		afx_msg void OnSelectionUnfreezeAll();
		afx_msg void OnUpdateSelectionUnfreezeAll(CCmdUI* pCmdUI);
		afx_msg void OnFitTextureToPoly();
		afx_msg void OnUpdateFitTextureToPoly(CCmdUI* pCmdUI);
		afx_msg void OnToggleClassIcons();
		afx_msg void OnUpdateToggleClassIcons(CCmdUI* pCmdUI);
		afx_msg void OnHideFrozenNodes();
		afx_msg void OnUpdateHideFrozenNodes(CCmdUI* pCmdUI);
		afx_msg void OnCameraToObject();
		afx_msg void OnUpdateCameraToObject(CCmdUI* pCmdUI);
		afx_msg void OnDisconnectSelectedPrefabs();
		afx_msg void OnUpdateDisconnectSelectedPrefabs(CCmdUI* pCmdUI);
		afx_msg void OnMirrorTextureX();
		afx_msg void OnUpdateMirrorTextureX(CCmdUI* pCmdUI);
		afx_msg void OnMirrorTextureY();
		afx_msg void OnUpdateMirrorTextureY(CCmdUI* pCmdUI);
		afx_msg void OnMatchTextureCoords();
		afx_msg void OnUpdateMatchTextureCoords(CCmdUI* pCmdUI);
		afx_msg void OnHollowBrush();
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()

		
		//-------------Member Variables-----------------//
	public:

		//the ID of the viewport (0-3). Used for accessing viewport specific properties
		uint32						m_nViewID;

		CEditRay					m_rayMousePoint;

		// THE region you're editing.
		CEditRegion					*m_pRegion;

		// When the right button goes down, if no editing actions are taken,
		// it sets this to TRUE so the popup menu comes up.
		BOOL						m_bDoMenu;

		DWORD						m_dwGridSpacing;			
		DWORD						m_dwMajorGridSpacing;

		// Timing stuff.
		UINT						m_TimerID;

		// Used to track the mouse.
		CPoint						m_LastMousePos;			


		CPolyRef					m_PolyToSplit;

		// Is the window in focus?
		BOOL						m_bInFocus;				
		CUITrackerMgr				m_cTrackerMgr;

		// used for mouse movement tracking
		BOOL						m_bMouseOver; 

		CRect						m_TagDrawRect;

		// Handles on the selection box.
		CVector						m_BoxHandles[NUM_BOX_HANDLES];
		CPoint						m_HandlePos[NUM_BOX_HANDLES];
		BOOL						m_HandlesInFrustum[NUM_BOX_HANDLES];
		
		// More info on the box.
		CVector						m_BoxMiddle;
		CVector						m_BoxHalf;

		int							m_CurMouseOverHandle;

		// Current state.
		EditState					m_EditState;			

		// Different cursors for the app.
		HCURSOR						m_CursorArrow;
		HCURSOR						m_CursorNS;
		HCURSOR						m_CursorWE;
		HCURSOR						m_CursorNWSE;
		HCURSOR						m_CursorNESW; 
		HCURSOR						m_CursorSizeAll;
		
		// Is a size handle selected?  (If not, it won't try to size the brush selections.)
		BOOL						m_bSizeHandleSelected;

		// Used for texture polygons with the "K" key
		int							m_nCurrentTextureVertIndex;

		// Distance to selected object.  Used to cycle selections.
		CReal						m_rSelectDist;
		
		CPoint						m_MousePoint;
	};

	/////////////////////////////////////////////////////////////////////////////


#endif  // __REGIONVIEW_H__




