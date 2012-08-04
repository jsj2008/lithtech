// ----------------------------------------------------------------------- //
//
// MODULE  : AIMgr.h
//
// PURPOSE : AIMgr class definition
//
// CREATED : 2/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AI_MGR_H__
#define __AI_MGR_H__


// Forward declarations.

class	CAICoordinator;
class	CAIPlanner;
class	CAIActionMgr;
class	CAIStimulusMgr;
class	CAISoundMgr;
class	CAINodeMgr;
class	CAINavMesh;
class	CAIQuadTree;
class	CAIPathMgrNavMesh;
class	CAIWorkingMemoryCentral;


// ----------------------------------------------------------------------- //

class CAIMgr
{
	public:

		CAIMgr();
		~CAIMgr();

		// Initialization / Termination.

		void	InitAI();
		void	TermAI();

		// Pre/PostStateWorld.
		void	PreStartWorld( bool bSwitchingWorlds );
		void	PostStartWorld();

		// Save / Load.

		void	Save( ILTMessage_Write *pMsg );
		void	Load( ILTMessage_Read *pMsg );

		// Garbage collection.

		void	CollectGarbage();

		// Update.

		void	Update();
		void	UpdateAISensors();

		// Debugging

		void	OnDebugCmd( HOBJECT hSender, const CParsedMsg &crParsedMsg );

	protected:
		void LoadNavMesh();

		void	SetupNavMesh();

	protected:

		CAICoordinator*			m_pAICoordinator;
		CAIPlanner*				m_pAIPlanner;
		CAIActionMgr*			m_pAIActionMgr;
		CAITargetSelectMgr*		m_pAITargetSelectMgr;
		CAIStimulusMgr*			m_pAIStimulusMgr;
		CAISoundMgr*			m_pAISoundMgr;
		CAINodeMgr*				m_pAINodeMgr;
		CAINavMesh*				m_pAINavMesh;
		CAIQuadTree*			m_pAIQuadTree;
		CAIPathMgrNavMesh*		m_pAIPathMgrNavMesh;
		CAIWorkingMemoryCentral* m_pAICentralMemory;

		LTObjRef				m_hNextAI;
};


// ----------------------------------------------------------------------- //

#endif
