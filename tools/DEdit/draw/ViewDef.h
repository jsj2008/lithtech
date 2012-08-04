//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : ViewDef.h
//
//	PURPOSE	  : Defines the CViewDef class used to abstract any
//              differences between parallel and perspective views.
//
//	CREATED	  : November 13 1996
//
//
//------------------------------------------------------------------

#ifndef __VIEWDEF_H__
	#define __VIEWDEF_H__


	// Includes....
	#include "navigator.h"
	#include "editray.h"
	#include "editgrid.h"


	#define PERSPECTIVE_VIEWTYPE		0
	#define PARALLEL_VIEWTYPE			1


	class CViewDefInfo
	{
		public:

			CViewDefInfo()
			{
				SetDims(0, 0);
			}

			void SetDims(uint32 nWidth, uint32 nHeight)
			{
				m_Width					= nWidth;
				m_Height				= nHeight;
				m_fWidth				= (CReal)nWidth;
				m_fHeight				= (CReal)nHeight;
				m_fHalfWidth			= m_fWidth * 0.5f;
				m_fHalfHeight			= m_fHeight * 0.5f;
				m_fProjectHalfWidth		= m_fWidth * 0.5f;
				m_fProjectHalfHeight	= m_fHeight * 0.5f;
			}


			DWORD	m_Width, m_Height;
			
			CReal	m_fWidth, m_fHeight;
			CReal	m_fHalfWidth, m_fHalfHeight;
			CReal	m_fProjectHalfWidth, m_fProjectHalfHeight;
	};


	class CViewDef
	{
		public:

			void				InitGrid(	int posX, int posY, int posZ,
											int forwardX, int forwardY, int forwardZ,
											int upX, int upY, int upZ,
											int drawSize
										);

			void				InitNav(	CReal posX, CReal posY, CReal posZ,
											int forwardX, int forwardY, int forwardZ,
											int upX, int upY, int upZ,
											CReal lookAtX, CReal lookAtY, CReal lookAtZ );

			//called whenever the size of the view definition is changed so that the
			//view can update any dependant variables
			virtual void		UpdateSize()		{}

			virtual CEditRay	MakeRayFromScreenPoint( CPoint point )=0;
			
			virtual void		ProjectPt( CVector &in, CPoint &out )=0;
			virtual void		ProjectPt( CVector &in, CVector &out )=0;

			virtual void		SetupFrustumPlanes( CPlane *pPlanes )=0;

			uint32				ViewType() const		{ return m_nViewType; }


		public:

			//this is a variable so we don't have to use polymorphism, so it can
			//be inlined
			uint32				m_nViewType;

			CNavigator			m_Nav;
			CEditGrid			m_Grid;
			CReal				m_NearZ, m_FarZ;

			CViewDefInfo		*m_pInfo;
			CReal				m_Magnify;

	};



	class CPerspectiveViewDef : public CViewDef
	{
		public:

			CPerspectiveViewDef();

			//used to set the projection for the camera, this will calculate the
			//vertical FOV from the aspect ratio
			void		SetupCamera(CReal fVertFOV, bool bUseAspect);

			//accessors for each FOV property
			CReal		GetHorzFOV() const			{ return m_fHorzFOV; }
			CReal		GetVertFOV() const			{ return m_fVertFOV; }
			bool		IsUseAspect() const			{ return m_bUseAspectRatio; }

			void		UpdateSize();

			CEditRay	MakeRayFromScreenPoint( CPoint point );
			
			void		ProjectPt( CVector &in, CPoint &out );
			void		ProjectPt( CVector &in, CVector &out );

			void		SetupFrustumPlanes( CPlane *pPlanes );

		private:

			//camera field of fiew vertically and horizontally (rad)
			CReal		m_fVertFOV;
			CReal		m_fHorzFOV;

			//whether or not to use the horizontal FOV or simply base it upon an
			//aspect ratio
			bool		m_bUseAspectRatio;

			//precalculated scalar for the vertical projection
			CReal		m_fProjScale;		
			
			//the precaclualted scalar for the horizontal projection. This may or may
			//not have the aspect ration premultiplied into it to allow for square
			//views
			CReal		m_fAspectProjScale;

	};


	class CParallelViewDef : public CViewDef
	{
		public:

			CParallelViewDef()
			{
				m_nViewType = PARALLEL_VIEWTYPE; 
			}

			CEditRay	MakeRayFromScreenPoint( CPoint point );
			
			void		ProjectPt( CVector &in, CPoint &out );
			void		ProjectPt( CVector &in, CVector &out );

			void		SetupFrustumPlanes( CPlane *pPlanes );

	};


#endif  // __VIEWDEF_H__

