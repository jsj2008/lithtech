//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditBrush.h
//
//	PURPOSE	  : Defines the CEditBrush class.
//
//	CREATED	  : November 12 1996
//
//
//------------------------------------------------------------------

#ifndef __EDITBRUSH_H__
#define __EDITBRUSH_H__


	// Includes....
	#include "editvert.h"
	#include "boundingbox.h"
	#include "editplane.h"
	#include "worldnode.h"
	#include "boundingsphere.h"
	

	// Defines....
	class CEditPoly;
	class CEditRegion;
	class CLTANode;
	class CLTAFile;


	// Different flags for the brushes
	enum EBrushFlags
	{
		BRUSHFLAG_VISIBLE			= (NODEFLAG_USER << 0)
	};

	// EditBrush stuff.
	class CEditBrush : public CWorldNode
	{
		public:
	
									CEditBrush();
			virtual					~CEditBrush();
			
			virtual void			Term();
			virtual CWorldNode*		AllocateSameKind()	{return new CEditBrush;}
			virtual void			DoCopy(CWorldNode *pOther);			
			void					DoCopy(CWorldNode *pOther, CStringHolder *pStringHolder);

		// Loading/saving.
		public:

			BOOL					LoadLTA( CLTANode* pNode, CStringHolder *pStringHolder );
			void					SaveLTA( CLTAFile* pFile, uint32 level );

			bool					LoadTBW( CAbstractIO& InFile, CStringHolder *pStringHolder );
			void					SaveTBW( CAbstractIO& OutFile );


		// Geometry stuff.
		public:

			BOOL					IsOpen();
			void					CopyEditBrush( CEditBrush *pOther, CStringHolder *pStringHolder=NULL );
			
			uint32					FindPolyWithEdge( uint32 iStart, uint32 v1, uint32 v2 );
			uint32					FindPolyWithPoints( uint32 index1, uint32 index2 );
			BOOL					FindPoliesWithEdge( uint32 v1, uint32 v2, CEditPoly*&pPoly1, CEditPoly*&pPoly2, uint32 &nContaining );
			uint32					FindClosestVert( const LTVector &vec );

			//this will add a vertex to the list, unless it can find one that is within
			//fThreshold distance of the one trying to be added, in which case it will use that
			//point
			uint32					AddVertOrGetClosest(const LTVector& vPos, CReal fThreshold);
			
			void					RemovePolyVerts( CMoDWordArray &indices, CMoArray<CEditPoly*> &exclude, CMoArray<CEditPoly*> &changedPolies );
			void					RemoveVertex( uint32 iVert );
			void					RemoveUnusedVerts();
			void					RemoveExtraEdges( CReal normalThresh, CReal distThresh );
			void					RemoveDuplicatePoints( CReal distThresh );
			void					ChangeVertexReferences( uint32 from, uint32 to );

			void					FlipNormals();			// Flips all of the polygons
			void					UpdatePlanes();		

		public:
			
			CEditPoly*				JoinPolygons( CEditPoly *pPoly1, CEditPoly *pPoly2, uint32 v1, uint32 v2 );

			//called to update all bounding volume information
			void					UpdateBoundingInfo()		{ UpdateBSphere(); }

			//updates the bounding sphere of this brush
			void					UpdateBSphere();

			//calculates a bounding box for this brush
			CBoundingBox			CalcBoundingBox();
		
			// Returns the upper left corner of the brush as it would appear in DEdit
			CVector					GetUpperLeftCornerPos();

			// Moves a brush to a position.
			void					MoveBrush(const LTVector& vPos);
			void					OffsetBrush(const LTVector& vDelta);

			virtual	void			Rotate(LTMatrix &mMatrix, LTVector &vCenter);

		// Accessors.
		public:

			//handles determining if the brush is visible or not
			bool					IsVisible()				{ return IsFlagSet(BRUSHFLAG_VISIBLE) != 0; }
			void					SetVisible( BOOL bVis )	{ SetFlag(BRUSHFLAG_VISIBLE, bVis != 0); }

		// Brush data.
		public:

			//the region that holds this brush
			CEditRegion*			m_pRegion;
			
			CMoArray<CEditPoly*>	m_Polies;
			CEditVertArray			m_Points;

			// The brush's index into the world..
			// Only valid during saving.
			uint32					m_RegionBrushIndex;

			// The brush display color.
			uint8					r, g, b;

			CBoundingSphere			m_BoundingSphere;
		
		protected:
			//don't allow the copy constructor
			CEditBrush( CEditBrush& pCopyFrom )	{}

#ifdef DIRECTEDITOR_BUILD
		// Notification that a property of this object has changed
		virtual void			OnPropertyChanged(CBaseProp* pProperty, bool bNotifyGame, const char *pModifiers);

#endif // DIRECTEDITOR_BUILD

				
	};


	void SimpleSplitBrush( CEditRegion *pRegion, CEditPoly *pRoot, CEditBrush *pToSplit, CEditBrush *newBrushes[2] );


#endif  // __EDITBRUSH_H__


