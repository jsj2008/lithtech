//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

// This module defines the DrawBase class.  This derives from DrawMgr and defines
// most of the rendering functionality.  D3DRender and CEditRender derive from this.

#ifndef __DRAWBASE_H__
#define __DRAWBASE_H__



	#include "editgrid.h"
	#include "editpoly.h"
	#include "drawmgr.h"
	#include "viewdef.h"

	class CRegionView;
	class CMeshShapeList;
	class CBaseEditObj;
	class CPreMainWorld;
	class CDecalInfo;


	#define ORIGIN_LINE_SIZE 10.0f
	#define D3DRGB_255(r,g,b) (0xFF000000 | ((long)r << 16) | ((long)g << 8) | (long)b)
	#define D3DRGB_GETR(color)	(((color) >> 16) & 0xFF)
	#define D3DRGB_GETG(color)	(((color) >> 8) & 0xFF)
	#define D3DRGB_GETB(color)	(((color) >> 0) & 0xFF)

	//used when rendering nodes so that information can be passed down heirarchicaly
	struct CNodeStackInfo
	{
		//the transformation matrix, relative to the camera
		LTMatrix		m_Transform;

		//whether or not this should be selected
		BOOL			m_bSelected;

		//whether or not this should be frozen
		BOOL			m_bFrozen;

		//determine if models must be shown
		BOOL			m_bForceShowModels;
	};


	extern float g_A;


	class DrawBase : public DrawMgr
	{
	// The main interface used by the views.
	public:
		
						DrawBase(CRegionView *pView);
		virtual			~DrawBase() {}
					
		// This is overridden by the subclasses and they call it when they're ready to have 
		// drawing commands issued.
		virtual void	Draw();
		
		virtual void	Resize(int x, int y) {}
		virtual void	Term() {}

		// Subclasses call this first thing in Draw.
		BOOL			InitFrame();


	// The software/d3d abstraction functions.
	public:

		virtual void	DrawFlatPoly2(TLVertex *pPoints, DWORD nPoints, CVector *pNormal, COLORREF color) {}
		virtual void	DrawTexturedPoly(CEditPoly *pPoly) {}
		virtual void	DrawTintedTexturedPoly(CEditPoly* pPoly, DWORD nTintColor)	{}

		//provides full control over rendering a textured polygon
		virtual void		DrawTexturedPoly(	TLVertex* pVerts, uint32 nNumVerts,  
												BOOL bTint, DWORD nTintColor, DFileIdent_t *pTextureFile, 
												DFileIdent_t *pDetailFile, DWORD nFlatShade, uint32 nClipMask = 0xFFFFFFFF) {}


		// Draw the line (it already has been transformed and clipped).
		virtual void	DrawLine(TLVertex *pVerts) {}

		// Draws a projected vertex.  Z value should be intact.
		virtual void	DrawVert(CVector &pos, DWORD borderColor, DWORD fillColor, int nSize)=0;

		//draws a model for the specified object
		virtual void	DrawModel(CBaseEditObj* pObject, uint32 nMode, CMeshShapeList* pShapeList, LTMatrix& mObjTrans)	{}

		// Enable/disable z-buffer (reads AND writes).
		// Returns the previous z-enable state.
		virtual BOOL	ZEnable(BOOL bEnable) {return FALSE;}
			
	// Functions implemented by DrawMgr.
	public:

		// Clips/projects/draws the line.
		void			Draw3dLine(TLVertex *pVerts);
		void			Draw3dLine2(TLVertex &vert1, TLVertex &vert2);

		// Transforms and calls Draw3dLine.
		void			TransformAndDrawLine(const CVector &pPt1, const CVector &pPt2, DWORD lineColor);
		
		// Transforms and draws the vertex.
		void			TransformAndDrawVert(CVector &pos, DWORD borderColor, DWORD fillColor, int nSize);

		// Draw the poly edges.
		void			DrawPolyLines(CEditPoly *pPoly, int xOffset, int yOffset, DWORD lineColor);

		// Draw the grid.
		BOOL			DrawEditGrid(CEditGrid *pGrid);

		bool			ClipPolyToPlane(LTPlane *pPlane, TLVertex *pPoints, WORD &pointStart, WORD &pointEnd);
		bool			ClipPolyToPlane(LTPlane *pPlane, LMVertex *pPoints, WORD &pointStart, WORD &pointEnd);

		// Sets up the point to point map and returns a cookie that you can 
		// fill each entry with when you use it.
		uint16			InitPointToPointMap(uint32 nPoints);

		// Just translates to CVectors and calls DrawFlatPoly2.
		void			DrawFlatPoly(CBasePoly *pPoly, COLORREF color);

		//draws all decals in the world based upon the settings specified by the user
		void			DrawDecals();

		// Draws the handles
		void			DrawHandles();

		// Calls DrawVert for each vertex.
		void			DrawPolyVerts(CEditPoly *pPoly, DWORD lineColor, DWORD fillColor);

		// Draws the 'drawing' poly (the one they're laying out with spacebar).
		void			DrawDrawingPoly();

		void			InternalDrawBox(CVector *pPoints, BOOL bFilled, COLORREF color);
		void			DrawRotatedBox(CVector *pPos, CVector *pDims, CVector &right, CVector &up, CVector &forward, BOOL bFilled, COLORREF color);
		void			DrawBox(CVector *pPos, CVector *pDims, BOOL bFilled, COLORREF color);
		void			DrawCircle(CVector *pCenter, CReal fRadius, UINT nLineSegs, CVector *pForward, CVector *pUp, DWORD color);
		void			DrawFieldOfView(DVector *pPos, LTMatrix mOrientation,
										float fov, float length, DWORD color);

		void			DrawLineToObjects(CBaseEditObj *pSrcObject, char *pDestName, DWORD color);
		void			DrawArrowLine(DVector *pSrc, DVector *pDest, DWORD color);

		// Draws a node and all of its children
		void			DrawNode		(CWorldNode *pNode, CNodeStackInfo *pInfo);
		void			DrawNodeChildren(CWorldNode *pNode, CNodeStackInfo *pInfo);
		void			DrawObject		(CWorldNode *pNode, CNodeStackInfo *pInfo);
		void			DrawPrefab		(CWorldNode *pNode, CNodeStackInfo *pInfo);
		void			DrawBrush		(CWorldNode *pNode, CNodeStackInfo *pInfo);

		void			DrawBrushNormals(CEditBrush *pBrush, CNodeStackInfo* pInfo);

		void			DrawPaths();
		void			DrawSelections();
		void			DrawSelectedBrushBox();
		void			DrawOrigin();
		void			DrawMarker();

		//determines the color for the drawing polygon. This will pick a color based
		//upon the status of the drawing polygon, wheter it is valid, concave, or
		//self intersecting
		DWORD			GetDrawingPolyColor(CEditBrush* pBrush);

	protected:

		//recursive function that will draw a decal node on a world tree and all the contained
		//prefabs
		void DrawDecalOnNode(CWorldNode* pNode, CDecalInfo& Info, LTMatrix& TransMat);

		//draws a single decal object. Assumes that the passed in node is a decal
		void			DrawDecal(CWorldNode* pNode);

		COLORREF		SetPolyVerts(	TLVertex *pVerts, CVector &v1, CVector &v2, 
										CVector &v3, CVector &v4, 
										CVector &normal, CVector &baseColor);


		bool			SetupObject(CBaseEditObj* pObject, CNodeStackInfo *pInfo);
		bool			SetupBrush(CEditBrush* pObject, CNodeStackInfo *pInfo);

		// Returns a display color as a D3DCOLOR
		DWORD			GetDisplayColorD3D(uint32 nColor) const
		{
			return m_pDisplayOptions->GetD3DColor(nColor);
		}

		// Returns the vertex/handle size
		int				GetVertexSize();		
		int				GetHandleSize();
		
	public:

		// Fills in 8 points given box corners.
		static void		GetBoxPoints(const LTVector &min, const LTVector &max, LTVector *pPoints);

	// Vars.
	public:
		//cached display options pointer so we don't have to keep retreiving it
		COptionsDisplay	*m_pDisplayOptions;

		// The current view we're rendering.
		CRegionView		*m_pView;

		CViewDef		*m_pViewDef;

		// The point to point map to optimize drawing in wireframe so we don't
		// have to draw the same lines over and over

		//a connection from one point to another for the brush given by the cookie specified	
		class CLineConnection
		{
		public:
			uint16	m_nCookie;
			uint32	m_nConnectTo;
		};

		//all the connections from a point (within a certain threshold)
		class CPointConnections
		{
		public:
			enum {	MAX_VERT_CONNECTIONS	= 8	};
			CLineConnection		m_Connections[MAX_VERT_CONNECTIONS];
		};

		CMoArray<CPointConnections>	m_PointToPoint;
		uint16						m_nCookie;

	protected:

		//determines if a render flag is set
		bool			IsRenderFlag(uint32 nFlag) const	{return (m_nRenderFlags & nFlag) ? true : false;}

		//the rendering flags. This is used to control how rendering operates...
		uint32			m_nRenderFlags;

	};


	// These are used to setup the z and rhw from a normal z in a perspective view
	template <class T>
	inline void SetupVertZPerspective(T &vert, float z)
	{
		vert.rhw = 1.0f / z;
		vert.m_Vec.z = g_A - g_A * vert.rhw;
	}

	// Sets up a vertex z in a parallel viewport
	template <class T>
	inline void SetupVertZParallel(T &vert)
	{
		vert.rhw = 1.0f;
		vert.m_Vec.z = 0.0f;
	}

	template <class T>
	inline void SetupVertPerspective(T *pVert, float x, float y, float z, DWORD theColor)
	{
		pVert->m_Vec.x = x;
		pVert->m_Vec.y = y;
		pVert->color = theColor;
		SetupVertZPerspective(*pVert, z);
	}

	template <class T>
	inline void SetupVertParallel(T *pVert, float x, float y, DWORD theColor)
	{
		pVert->m_Vec.x = x;
		pVert->m_Vec.y = y;
		pVert->color = theColor;
		SetupVertZParallel(*pVert);
	}


	// Returns the vertex size
	inline int DrawBase::GetVertexSize()
	{
		return m_pDisplayOptions->GetVertexSize();
	}

	// Returns the handle size
	inline int DrawBase::GetHandleSize()
	{
		return m_pDisplayOptions->GetHandleSize();
	}


#endif




