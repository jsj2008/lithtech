// ----------------------------------------------------------------------- //
//
// MODULE  : AIAStarMachine.h
//
// PURPOSE : A* machine declaration, and abstract classes for components.
//
// CREATED : 11/26/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_ASTAR_MACHINE_H_
#define _AI_ASTAR_MACHINE_H_

#include "AIClassFactory.h"


//-----------------------------------------------------------------

// Forward declarations.

class CAI;
class CAIAStarMachine;
class CAIAStarMapAbstract;
class CAIAStarStorageAbstract;

//-----------------------------------------------------------------

enum ENUM_AStarNodeID
{
	kASTARNODE_Invalid = -1,
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarNodeAbstract : public CAIClassAbstract
{
public:

	DECLARE_AI_FACTORY_CLASS_ABSTRACT();

	CAIAStarNodeAbstract();
	virtual ~CAIAStarNodeAbstract();

	virtual void	DebugPrint() = 0;
	virtual void	DebugPrintExpand() {}
	virtual void	DebugPrintNeighbor() {}

public:

	ENUM_AStarNodeID		eAStarNodeID;

	float					fGoal;
	float					fHeuristic;
	float					fFitness;

	CAIAStarNodeAbstract*	pListPrev;
	CAIAStarNodeAbstract*	pListNext;
	CAIAStarNodeAbstract*	pAStarParent;
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarGoalAbstract
{
public:
	virtual void	SetDestNode( ENUM_AStarNodeID eAStarNode ) = 0;

	virtual float	GetHeuristicDistance( CAIAStarNodeAbstract* pAStarNode ) = 0;
	virtual float	GetActualCost( CAI* pAI, CAIAStarNodeAbstract* pAStarNodeA, CAIAStarNodeAbstract* pAStarNodeB ) = 0;
	virtual bool	IsAStarFinished( CAIAStarNodeAbstract* pAStarNode ) = 0;
	virtual bool	IsAStarNodePassable( ENUM_AStarNodeID /*eAStarNode*/ ) { return true; }
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarStorageAbstract
{
public:
	virtual CAIAStarNodeAbstract*	CreateAStarNode( ENUM_AStarNodeID eAStarNode ) = 0;
	virtual void					DestroyAStarNode( CAIAStarNodeAbstract* pAStarNode ) = 0;

	virtual void	ResetAStarStorage() = 0;
	virtual void	AddToOpenList( CAIAStarNodeAbstract* pAStarNode, CAIAStarMapAbstract* pAStarMap ) = 0;
	virtual void	AddToClosedList( CAIAStarNodeAbstract* pAStarNode, CAIAStarMapAbstract* pAStarMap ) = 0;
	virtual void	RemoveFromOpenList( CAIAStarNodeAbstract* pAStarNode ) = 0;
	virtual void	RemoveFromClosedList( CAIAStarNodeAbstract* pAStarNode ) = 0;

	virtual CAIAStarNodeAbstract*	FindInOpenList( ENUM_AStarNodeID eAStarNode ) = 0;
	virtual CAIAStarNodeAbstract*	FindInClosedList( ENUM_AStarNodeID eAStarNode ) = 0;

	virtual CAIAStarNodeAbstract*	RemoveCheapestOpenNode() = 0;
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarMapAbstract
{
public:
	enum ENUM_ASTAR_FLAGS
	{
		kASTAR_Unchecked	= 0x00,
		kASTAR_Open			= 0x01,
		kASTAR_Closed		= 0x02,
		kASTAR_OpenOrClosed	= kASTAR_Open | kASTAR_Closed,
		kASTAR_NotPassable	= 0x04,
	};

public:

	virtual int					GetNumAStarNeighbors( CAI* pAI, CAIAStarNodeAbstract* pAStarNode ) = 0;
	virtual ENUM_AStarNodeID	GetAStarNeighbor( CAI* pAI, CAIAStarNodeAbstract* pAStarNode, int iNeighbor, CAIAStarStorageAbstract* pAStarStorage ) = 0;

	virtual void				SetAStarFlags( ENUM_AStarNodeID eAStarNode, unsigned long dwFlags, unsigned long dwMask ) = 0;
	virtual unsigned long		GetAStarFlags( ENUM_AStarNodeID eAStarNode ) = 0;

	virtual void				SetupPotentialNeighbor( CAIAStarNodeAbstract* /*pAStarNodeParent*/, CAIAStarNodeAbstract* /*pAStarNodeChild*/ ) {}
	virtual void				FinalizeNeighbor( CAIAStarNodeAbstract* /*pAStarNodeChild*/ ) {}
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarMachine
{
public:

	 CAIAStarMachine();
	~CAIAStarMachine();

	// Initialization of the machine.

	void	InitAStar( CAIAStarStorageAbstract* pAStarStorage, CAIAStarGoalAbstract* pAStarGoal, CAIAStarMapAbstract* pAStarMap );

	// Setup for A*.

	void	SetAStarSource( ENUM_AStarNodeID eAStarNodeSource );
	void	SetAStarDest( ENUM_AStarNodeID eAStarNodeDest );

	// Run the A* Algorithm.

	void	RunAStar( CAI* pAI );

	// Data access.

	CAIAStarNodeAbstract*		GetAStarNodeCur() { return m_pAStarNodeCur; } 
	CAIAStarStorageAbstract*	GetAStarStorage() { return m_pAStarStorage; }
	CAIAStarMapAbstract*		GetAStarMap() { return m_pAStarMap; }

protected:

	CAIAStarStorageAbstract*	m_pAStarStorage;
	CAIAStarGoalAbstract*		m_pAStarGoal;
	CAIAStarMapAbstract*		m_pAStarMap;

	CAIAStarNodeAbstract*		m_pAStarNodeCur;

	ENUM_AStarNodeID			m_eAStarNodeSource;
	ENUM_AStarNodeID			m_eAStarNodeDest;
};

//-----------------------------------------------------------------

#endif // _AI_ASTAR_MACHINE_H_
