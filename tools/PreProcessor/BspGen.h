//------------------------------------------------------------------
//
//	FILE	  : BspGen.h
//
//	PURPOSE	  : Defines the CBspGen class, which creates a BSP
//				tree out of a simple list of polygons (CModel).
//
//	CREATED	  : 2nd May 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __BSPGEN_H__
	#define __BSPGEN_H__


	// Defines....
	#define FILTER_ADDREVERSE	(1<<0)	// Add an extra poly for the reverse side (hullmakers do this).
	#define FILTER_ALWAYSADD	(1<<1)	// Always add the poly regardless of which side of the BSP it's on.


	typedef enum
	{

		BSPGEN_OK,
		BSPGEN_ERROR,
		BSPGEN_OUTOFMEMORY

	} BspGenStatus;

	class CBspGen;
	class CPrePoly;
	class CPreWorld;
	
	// Includes....
	#include "bdefs.h"
	#include "node.h"


	class CPolyList;


	class CBspGenOptions
	{
	public:

						CBspGenOptions()
						{
							m_pWorld				= NULL;
							m_pCustomPolyList		= NULL;
							m_bPostProcess			= true;
							m_nThreadsToUse			= 0;
						}

		CPreWorld		*m_pWorld;
		
		// If you set this, it won't generate the tree from the world's poly list,
		// it'll just use the polies in this list.
		CGLinkedList<CPrePoly*>	*m_pCustomPolyList;

		bool			m_bPostProcess;
		uint32			m_nThreadsToUse;
	};



	class CBspGen
	{
		public:

							CBspGen();
							~CBspGen();
						
			
		// Member functions
		public:
			
			// Makes the BSP tree for you.
			bool			GenerateBspTree(CBspGenOptions *pOptions);
	
		// Private member functions
		public:

			// Initial data structuring routines.
			bool				MinimizeNodePolySize(NODEREF iNode, CReal maxX, CReal maxY);


		// Actual BSP generation routines.
		public:

			// NEVER call this directly .. the threads call it...
			void				RecurseOnList(CPolyList &polies, NODEREF *pRootNode, uint32 nLevel);

			void				ThreadedRecurseOnList(CPolyList &polies, NODEREF *pRootNode, uint32 nLevel);
			CPrePoly*			GetBestSplit(CPolyList &polies, uint32 nLevel);

			void				SplitPolyArray(CPolyList &polies, CPrePoly *pSplitOn, CPolyList *sideArrays[2]);
			void				SplitPoly(CPrePoly *pSplitOn, CPrePoly *pToSplit, CPolyList *sideArrays[2]);

			void				DoPreProcessing();
			bool				DoPostProcessing();

		
		// Public member variables
		public:
	
			CBspGenOptions		m_Options;
			
			// The model that all the data is stuffed into.
			CPreWorld			*m_pWorld;

			// Stats from the generation.
			uint32				m_nPolySplits;
			uint32				m_nSplitPolyArrayCalls;


		public:

			// Controls thread creation.
			uint32				m_nThreadsBeingUsed;
			
			// Toggles....
			bool				m_bRemoveIntersectingPolies;
			
			// This controls access to critical stuff between threads.
			void				*m_AppendCS;
			
			
			// This is where points are collected for 'portal' polies as stuff is chopped up.
			CMoDWordArray		m_PortalPolyPts;
	
	};


	// Helper functions.

	// Filters the inside portions of the poly into the BSP (or all portions if you
	// specify FILTER_ALWAYSADD).
	// If pInsideFragments is non-null, it won't actually add the poly to the BSP, it will
	// just fill pInsideFragments with the fragments that are in valid space.

	// Pass in a combination of the FILTER_ flags.
	void FilterPolyIntoBSP(
		CPreWorld			*pWorld, 
		NODEREF				pRoot, 
		CPrePoly			*pPoly, 
		uint32				flags,
		CMoArray<CPrePoly*> *pInsideFragments=NULL);

	CNode* AddNode(CNodeList &nodeList, const CPrePlane *pPlane, CPrePoly *pPoly);

	// Returns TRUE if there is a node with the given poly.
	CNode* NodeWithPoly(CPreWorld *pWorld, CPrePoly *pPoly);

	// Adds a node, surface, and poly to the backside of the specified node 
	// (and keeps the BSP structure intact).
	CNode* AddBacksideNode(CPreWorld *pWorld, CNode *pNode);

	// Used for HullMakers. Creates a poly that faces the opposite way and adds it to the world.
	CPrePoly* AddReversedPoly(
		CPreWorld *pWorld,
		const CPrePoly *pPoly);

	
#endif


