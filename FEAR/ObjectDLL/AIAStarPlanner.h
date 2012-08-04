// ----------------------------------------------------------------------- //
//
// MODULE  : AIAStarPlanner.h
//
// PURPOSE : AStar Node, Goal, Storage, and Map classes for finding
//           action plans from the planner.
//
// CREATED : 1/30/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_ASTAR_PLANNER_H_
#define _AI_ASTAR_PLANNER_H_

#include "AIAStarStorageLinkedList.h"
#include "AIWorldState.h"
#include "AIActionAbstract.h"


//-----------------------------------------------------------------

// Forward declarations.

class CAIAStarMapPlanner;
class CAIGoalAbstract;
class CAIActionAbstract;

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarNodePlanner : public CAIAStarNodeAbstract
{
public:
	DECLARE_AI_FACTORY_CLASS( CAIAStarNodePlanner );

	 CAIAStarNodePlanner();
	~CAIAStarNodePlanner() {}

	// Debug.

	virtual void	DebugPrint() { TRACE( "ASTAR: %d\n", eAStarNodeID ); }
	virtual void	DebugPrintExpand();
	virtual void	DebugPrintNeighbor();

public:

	CAIWorldState		wsWorldStateCur;
	CAIWorldState		wsWorldStateGoal;
	CAIAStarMachine*	pAStarMachine;
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarGoalPlanner : public CAIAStarGoalAbstract
{
public:
	 CAIAStarGoalPlanner();
	~CAIAStarGoalPlanner();

	// CAIAStarGoalAbstract required functions.

	virtual void	SetDestNode( ENUM_AStarNodeID eAStarNode ) { m_eAStarNodeDest = eAStarNode; }
	virtual float	GetHeuristicDistance( CAIAStarNodeAbstract* pAStarNode );
	virtual float	GetActualCost( CAI* pAI, CAIAStarNodeAbstract* pAStarNodeA, CAIAStarNodeAbstract* pAStarNodeB );
	virtual bool	IsAStarFinished( CAIAStarNodeAbstract* pAStarNode );

	// Initialization.

	void			InitAStarGoalPlanner( CAI* pAI, CAIAStarMapPlanner* pAStarMapPlanner, CAIGoalAbstract* pGoal );

	bool			IsPlanValid( CAIAStarNodePlanner* pAStarNode );

protected:

	ENUM_AStarNodeID		m_eAStarNodeDest;
	CAIAStarMapPlanner*		m_pAStarMapPlanner;
	CAIGoalAbstract*		m_pAIGoal;
	CAI*					m_pAI;
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarStoragePlanner : public CAIAStarStorageLinkedList
{
public:

	void							InitAStarStoragePlanner( CAIAStarMachine* pAStarMachine );

	// CAIAStarStorageLinkedList overrides.

	virtual CAIAStarNodeAbstract*	CreateAStarNode( ENUM_AStarNodeID eAStarNode );
	virtual void					DestroyAStarNode( CAIAStarNodeAbstract* pAStarNode );

protected:

	CAIAStarMachine*	m_pAIAStarMachine;
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

typedef std::vector<EnumAIActionType, LTAllocator<EnumAIActionType, LT_MEM_TYPE_OBJECTSHELL> > AI_ACTION_TYPE_LIST;

class CAIAStarMapPlanner : public CAIAStarMapAbstract
{
public:
	 CAIAStarMapPlanner();
	~CAIAStarMapPlanner();

	// CAIAStarMapAbstract required functions.

	virtual int				 GetNumAStarNeighbors( CAI* pAI, CAIAStarNodeAbstract* pAStarNode );
	virtual ENUM_AStarNodeID GetAStarNeighbor( CAI* pAI, CAIAStarNodeAbstract* pAStarNode, int iNeighbor, CAIAStarStorageAbstract* pAStarStorage );

	virtual void			SetAStarFlags( ENUM_AStarNodeID eAStarNode, unsigned long dwFlags, unsigned long dwMask );
	virtual unsigned long	GetAStarFlags( ENUM_AStarNodeID eAStarNode );

	// Initialization.

	void	BuildEffectActionsTable();
	void	InitAStarMapPlanner( CAI* pAI );
	
	// ID conversion.

	EnumAIActionType		ConvertID_AStarNode2AIAction( ENUM_AStarNodeID eAStarNode ) { return ( EnumAIActionType )eAStarNode; }
	ENUM_AStarNodeID		ConvertID_AIAction2AStarNode( EnumAIActionType eAction ) { return ( ENUM_AStarNodeID )eAction; }

	// AIAction access.

	CAIActionAbstract*		GetAIAction( ENUM_AStarNodeID eAStarNode );

protected:

	CAI*					m_pAI;
	AI_ACTION_TYPE_LIST		m_lstEffectActions[kWSK_Count];
	EnumAIActionType		m_aNeighborActions[kAct_Count];
	int						m_cNeighborActions;
	bool					m_bEffectTableBuilt;
};

//-----------------------------------------------------------------

#endif // _AI_ASTAR_PLANNER_H_
