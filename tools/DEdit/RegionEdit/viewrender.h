//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

// This module holds all the rendering-related structures
// for a CRegionView.  (These are all renderer-independent, but
// they define all the viewing parameters).

#ifndef __VIEWRENDER_H__
#define __VIEWRENDER_H__


	#include "drawmgr.h"
	#include "viewdef.h"


	class CRegionView;
	

	// Defines....
	#define VM_TOP				0
	#define VM_BOTTOM			1
	#define VM_LEFT				2
	#define VM_RIGHT			3
	#define VM_FRONT			4
	#define VM_BACK				5
	#define VM_PERSPECTIVE		6

	#define NUM_VIEWDEFS		7


	class CViewRender
	{
		public:

							CViewRender();
							~CViewRender();

			// Call right away..
			void			SetViewRenderRegionView(CRegionView *pView);

			// Draw a frame.
			void			DrawRect(CRect *pRect=NULL);

			// Restart the renderer with new options.
			void			RestartRender(BOOL bForce, int deviceNum, int renderMode, BOOL bRedraw);

			// Call on the view's first update.
			void			SetupInitialDrawMgr();
			
			// Call when the window is resized.
			void			SetupNewSize(int x, int y);


		private:

			// Init everything before drawing a frame.
			void			InitFrame();

		
		public:

			CMatrix			m_Transform;
			CMatrix			m_Rotation;

			CPlane			m_ClipPlanes[6];
			DWORD			m_nClipPlanesToUse;  // 4 in parallel mode, 6 in perspective.

		public:
					
			// The drawmgr itself.
			DrawMgr			*m_pDrawMgr;

			// d3d init options.
			int				m_DeviceNum;
			int				m_RenderMode;

			CViewDef		*m_pViewDef;
			
			CRegionView		*m_pView;


		// Accessors.
		public:

			CViewDefInfo*	ViewDefInfo()			{ return &m_ViewDefInfo; }
			CViewDef*		ViewDef()				{ return m_pViewDef; }
			CNavigator&		Nav()					{ return m_pViewDef->m_Nav; }
			CEditGrid&		EditGrid()				{ return m_pViewDef->m_Grid; }

			CReal&			NearZ()					{ return m_pViewDef->m_NearZ; }
			CReal&			FarZ()					{ return m_pViewDef->m_FarZ; }
																   
			BOOL			IsPerspectiveViewType()	{ return m_pViewDef->ViewType() == PERSPECTIVE_VIEWTYPE; }
			BOOL			IsParallelViewType()	{ return m_pViewDef->ViewType() == PARALLEL_VIEWTYPE; }
			
			void			InitViewDefs();
			void			SetViewMode( uint32 mode );
			uint32			GetViewMode();
			

		// Geometry routines.
		public:

			bool InsideFrustum(const LTVector &vec ) const
			{
				for(uint32 i=0; i < m_nClipPlanesToUse; i++ )
					if( !InsidePlane(m_ClipPlanes[i], vec) )
						return false;
				return true;
			}

			bool SphereInsideFrustum(const LTVector &vCenter, float fRadius) const
			{
				for(uint32 i=0; i < m_nClipPlanesToUse; i++ )
					if( !SphereInsidePlane(m_ClipPlanes[i], vCenter, fRadius) )
						return false;
				return true;
			}


			BOOL				ClipLineToFrustum(TLVertex *pVerts, uint32 nClipMask = 0xFFFFFFFF);
			BOOL				TransformAndProjectInFrustum( CVector &vec, CPoint &point );
																	
			BOOL				TransformAndProject( CVector &vec, CPoint &point );
			void				TransformPt( CVector &pt );
			void				TransformPt( CVector &in, CVector &out );

			bool				InsidePlane(const LTPlane &plane, const LTVector &vec ) const;
			bool				SphereInsidePlane( const LTPlane& plane, const LTVector &vCenter, float fRadius) const;
			void				IntersectPlane( CPlane &plane, CVector &dest, CVector &start, CVector &end );

		//lighting routines
		public:

			//meant to be called at the start of each frame to update light vectors
			void				InitLighting(const LTVector& vWorldSpaceLight, CReal fAmbient);

			//called to light a point that is in world space. Returns the intensity
			CReal				WorldSpaceLightInt(const LTVector& vNormal);

			//called to light a point that is in camera space. Returns the intensity
			CReal				CameraSpaceLightInt(const LTVector& vNormal);

			//given a number between -1 and 1, it will map it into the appropriate lighting
			//color
			CReal				AdjustDotLight(CReal fDot);

		//member variables
		public:

			// See the top of this file for the #define'd view mode codes.
			CParallelViewDef	m_ParallelViews[6];
			CPerspectiveViewDef	m_PerspectiveView;

			CViewDefInfo	m_ViewDefInfo;
			CViewDef		*m_ViewDefs[NUM_VIEWDEFS];

			//light vectors
			LTVector		m_vWorldSpaceLight;
			LTVector		m_vCameraSpaceLight;
			CReal			m_fAmbientScale;
			CReal			m_fAmbientLight;

		private:

			uint32			m_nViewMode;

	};
	
	inline CReal CViewRender::WorldSpaceLightInt(const LTVector& vNormal)
	{
		return AdjustDotLight(m_vWorldSpaceLight.Dot(vNormal));
	}

	inline CReal CViewRender::CameraSpaceLightInt(const LTVector& vNormal)
	{
		return AdjustDotLight(m_vCameraSpaceLight.Dot(vNormal));
	}

	//given a number between -1 and 1, it will map it into the appropriate lighting
	//color
	inline CReal CViewRender::AdjustDotLight(CReal fDot)
	{
		return fDot * m_fAmbientScale + m_fAmbientLight;
	}

	inline void CViewRender::TransformPt( CVector &pt )
	{
		m_Transform.Apply( pt );
	}


	inline void CViewRender::TransformPt( CVector &in, CVector &out )
	{
		m_Transform.Apply( in, out );
	}


	inline bool CViewRender::InsidePlane(const LTPlane &plane, const LTVector &vec ) const
	{
		return (plane.m_Normal.Dot(vec) - plane.m_Dist) > 0.0f;
	}

	inline bool CViewRender::SphereInsidePlane(const LTPlane& plane, const LTVector& vCenter, float fRadius) const
	{
		return (plane.m_Normal.Dot(vCenter) - plane.m_Dist) > -fRadius;
	}


	inline void CViewRender::IntersectPlane( CPlane &plane, CVector &dest, CVector &start, CVector &end )
	{
		CReal		dot1 = plane.m_Normal.Dot(start) - plane.m_Dist;
		CReal		dot2 = plane.m_Normal.Dot(end) - plane.m_Dist;
		CReal		t = -dot1 / (dot2 - dot1);

		dest = start + ((end - start) * t);
	}


#endif // __VIEWRENDER_H__






