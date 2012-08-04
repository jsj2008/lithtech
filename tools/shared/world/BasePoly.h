//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : BasePoly.h
//
//	PURPOSE	  : Defines the CBasePoly class.
//
//	CREATED	  : October 15 1996
//
//
//------------------------------------------------------------------

#ifndef __BASEPOLY_H__
	#define __BASEPOLY_H__


	// Includes....
	#include "editvert.h"
	#include "editray.h"
	#include "editplane.h"
	#include "editbrush.h"


	// Defines....
	class CEditRegion;
	class CEditBrush;
	class CLTANode;
	class CLTAFile;



	class CBasePoly
	{
		public:

								CBasePoly();
								CBasePoly( CEditBrush *pBrush );
								CBasePoly( CEditBrush *pBrush, CMoDWordArray &indices );
								CBasePoly( CEditBrush *pBrush, uint32 i0, uint32 i1, uint32 i2, bool bUpdatePlane=FALSE );
			virtual				~CBasePoly()		{}

			void				Term();


		// Saving.
		public:

			bool				LoadBasePolyLTA( CLTANode* pNode );
			void				SaveBasePolyLTA( CLTAFile* pFile, uint32 level );

			bool				LoadBasePolyTBW( CAbstractIO& InFile );
			void				SaveBasePolyTBW( CAbstractIO& OutFile );

		// Functionality.
		public:

			bool				IsValid();

			// Called when a poly is split.
			virtual void		CopyAttributes( CBasePoly *pOther, CStringHolder *pStringHolder=NULL )	{}
			
			// Decrements all points >= index.
			void				DecrementPoints( uint32 index );

			// Tells if the point is in the polygon's plane or in the poly itself.
			bool				PointInPlane(const LTVector &point );
			bool				PointInPoly(const LTVector &point );

			void				RemoveCollinearVertices();
			bool				IsConvex();

			void				GetCenterPoint( CVector &centerPt );

			void				Flip();
			void				UpdatePlane();

			bool				GetTriangles( CMoArray<CBasePoly*> &polies );


		// Useful little helper geometry functions.
		public:

			bool				Intersect(const LTVector &a, const LTVector &b, const LTVector &c, const LTVector &d );
			bool				Diagonalie( uint32 a, uint32 b );

			bool				LeftOn( uint32 a, uint32 b, uint32 c );
			bool				Left( uint32 a, uint32 b, uint32 c );
			bool				InsideDiagonal( uint32 a, uint32 b );
	
			bool				ValidTriangle( uint32 a, uint32 b, uint32 c );

		
		// Accessors.
		public:

			CVector&			Normal()				{ return m_Plane.m_Normal; }
			CReal&				Dist()					{ return m_Plane.m_Dist; }

			uint32&				Index( uint32 i )		{ return m_Indices[i]; }
			uint32&				NextIndex( uint32 i )	{ return m_Indices.Next(i); }
			uint32&				LastIndex()				{ return m_Indices.Last(); }
			uint32				NumVerts() const		{ return m_Indices.GetSize(); }
			
			CEditVert&			Pt( uint32 i )			{ return m_pBrush->m_Points[m_Indices[i]]; }

			CEditVert&			NextPt( uint32 i )		{ return m_pBrush->m_Points[m_Indices.Next(i)]; }
			CEditVert&			PrevPt( uint32 i )		{ return m_pBrush->m_Points[m_Indices.Prev(i)]; }
			CEditVert&			LastPt()				{ return m_pBrush->m_Points[m_Indices.Last()]; }
			
		
		public:
			
			CEditPlane			m_Plane;

			CMoDWordArray		m_Indices;
			CEditBrush			*m_pBrush;

	};


#endif  // __BASEPOLY_H__


