// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkAbstract.h
//
// PURPOSE : AI NavMesh Link abstract class definition
//
// CREATED : 07/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_ABSTRACT_H_
#define _AI_NAVMESH_LINK_ABSTRACT_H_

#include "GameBase.h"
#include "AIActionAbstract.h"
#include "AINodeTypes.h"
#include "AIEnumNavMeshLinkTypes.h"
#include "AIEnumNavMeshTypes.h"
#include "AI.h"

LINKTO_MODULE( AINavMeshLinkAbstract );

//-----------------------------------------------------------------

// Forward declarations.

class	CAI;
class	CAIPathNavMesh;
class	CAINavMeshGen;
class	CAINavMeshGenPoly;
class	CAINavMeshPoly;
class	CAINavMeshEdge;
class	CAIAStarMapNavMesh;
class	CAIStateUseSmartObject;
struct  AIDB_SmartObjectRecord;
struct	SPATH_NODE;

#define TRAVERSAL_IN_PROGRESS	true
#define LINK_CHECK_TIMEOUT		true

//-----------------------------------------------------------------

class AINavMeshLinkAbstract : public GameBase
{
public:
	typedef GameBase super;

	AINavMeshLinkAbstract();

	// Engine

	virtual uint32	EngineMessageFn(uint32 messageID, void *pvData, float fData);
	virtual void	ReadProp(const GenericPropList *pProps);
	virtual void	InitialUpdate();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Name

	const char* GetName() const { return m_strName.c_str(); }

	// NavMesh construction.

	void			SetNMLinkBounds( uint32 cBounds, LTVector* pvBounds );
	void			SetupNMLinkEdges();

	// Pathfinding.

	virtual bool			GetNumNMLinkNeighbors( int* pcNeighbors );
	virtual bool			GetNMLinkNeighborAtEdge( CAIAStarMapNavMesh* pMap, int iNeighbor, CAINavMeshPoly* pNMPolyParent, CAINavMeshPoly*& pNMPolyNeighbor );
	virtual bool			GetNMLinkOffsetEntryPos( ENUM_NMPolyID ePolyDest, const LTVector& vSourcePos, LTVector* pvOffset );
	virtual bool			IsLinkValidDest() { return true; }
	virtual bool			IsLinkPassable( CAI* /*pAI*/, ENUM_NMPolyID /*ePolyTo*/ );
	virtual float			GetNMLinkPathingWeight(CAI* /*pAI*/);
	virtual bool			SetNMLinkOffsetEntryPathLink( SPATH_NODE* pPathNode, CAI* pAI );
	virtual bool			PullStrings( const LTVector& vPtPrev, const LTVector& vPtNext, CAI* pAI, LTVector* pvNewPos );
	virtual bool			IsPullStringsModifying( CAI* pAI );
	bool					IsInLinkOrOffsetEntry( CAI* pAI );
	virtual bool			AllowStraightPaths() { return false; }
	virtual bool			AllowDynamicMovement( CAI* pAI );

	// SmartObject.

	AIDB_SmartObjectRecord*	GetSmartObject();

	// Goals / Actions.

	virtual bool	IsLinkRelevant( CAI* /*pAI*/ ) { return false; }
	virtual bool	IsLinkValid( CAI* /*pAI*/, EnumAIActionType /*eActionType*/, bool bTraversalInProgress );
	virtual void	ActivateTraversal( CAI* /*pAI*/, CAIStateUseSmartObject* /*pStateUseSmartObject*/ );
	virtual void	DeactivateTraversal( CAI* /*pAI*/ ) {}
	virtual bool	IsTraversalInProgress( CAI* pAI ) { return false; }
	virtual bool	IsTraversalComplete( CAI* /*pAI*/ ) { return false; }
	virtual void	ApplyTraversalEffect( CAI* /*pAI*/ ) {}

	// Enter / Exit.

	virtual void	HandleNavMeshLinkEnter( CAI* /*pAI*/ ) {}
	virtual void	HandleNavMeshLinkExit( CAI* /*pAI*/ ) {}

	// Movement modifying.

	virtual bool	GetAllowDirectionalMovement() { return false; }
	virtual bool	HandleAdvancePath( CAI* /*pAI*/ ) { return false; }
	virtual void	ApplyMovementAnimation( CAI* /*pAI*/ ) {}
	virtual void	ModifyMovement( CAI* pAI, CAIMovement::State eStatePrev, LTVector* pvNewPos, CAIMovement::State* peStateCur ) {}

	// Path modifying

	virtual bool	GetNewPathClearEnteringLink( CAI* /*pAI*/ ) { return true; }
	virtual ENUM_NMPolyID GetNewPathSourcePoly( CAI* /*pAI*/) { return kNMPoly_Invalid; }

	// Reservation and delay.

	void			ReserveNMLink( CAI* pAI, bool bReserve );
	void			ApplyNMLinkDeathDelay( CAI* pAI );

	// Active / Enabled.

	bool			IsNMLinkActiveToAI( CAI* pAI );
	bool			IsNMLinkEnabledToAI( CAI* pAI, bool bCheckTimeout );

	// Data access.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_InvalidType; }
	ENUM_NMLinkID					GetNMLinkID() const { return m_eNMLinkID; }
	void							SetNMPolyID( ENUM_NMPolyID ePolyID ) { m_eNMPolyID = ePolyID; }
	ENUM_NMPolyID					GetNMPolyID() const { return m_eNMPolyID; }
	void							SetNMLinkActive( bool bActive ) { m_bLinkActive = bActive; }
	void							SetNMLinkEnabled( bool bEnabled ) { m_bLinkEnabled = bEnabled; }
	bool							GetNMLinkEnabled() const { return m_bLinkEnabled; }
	virtual float					GetNMLinkOffsetEntryDistA() const { return m_fEntryOffsetDistA; }
	virtual float					GetNMLinkOffsetEntryDistB() const { return m_fEntryOffsetDistB; }

protected:

	// Setup.

	CAINavMeshEdge*					FindMatchingPolyEdge( const LTVector& v0, const LTVector& v1 );
	float							FindFloor( const LTVector& vPos, const LTVector& vDir, float fOffset );

	// Pathfinding.

	bool							GetNeighborOffsetEntryPos( int iNeighbor, const LTVector& vSourcePos, LTVector* pvOffset );

	// Message Handlers...

	DECLARE_MSG_HANDLER( AINavMeshLinkAbstract, HandleAllMsgs );
	DECLARE_MSG_HANDLER( AINavMeshLinkAbstract, HandleActiveMsg );
	DECLARE_MSG_HANDLER( AINavMeshLinkAbstract, HandleDisableMsg );
	DECLARE_MSG_HANDLER( AINavMeshLinkAbstract, HandleEnableMsg );
	DECLARE_MSG_HANDLER( AINavMeshLinkAbstract, HandleMinActiveAwarenessMsg );
	DECLARE_MSG_HANDLER( AINavMeshLinkAbstract, HandleMaxActiveAwarenessMsg );
	DECLARE_MSG_HANDLER( AINavMeshLinkAbstract, HandleMinEnabledAwarenessMsg );
	DECLARE_MSG_HANDLER( AINavMeshLinkAbstract, HandleMaxEnabledAwarenessMsg );
	DECLARE_MSG_HANDLER( AINavMeshLinkAbstract, HandleRemoveMsg );

protected:

	std::string				m_strName;

	ENUM_NMLinkID			m_eNMLinkID;
	ENUM_NMPolyID			m_eNMPolyID;
	uint32					m_nSmartObjectID;
	bool					m_bLinkActive;
	bool					m_bLinkEnabled;
	EnumAIAwarenessMod		m_eEnabledAwarenessMod;
	EnumAIAwareness			m_eMinEnabledAwareness;
	EnumAIAwareness			m_eMaxEnabledAwareness;
	EnumAIAwareness			m_eMinActiveAwareness;
	EnumAIAwareness			m_eMaxActiveAwareness;

	bool					m_bTraversalTimedOut;
	double					m_fNextTraversalTime;

	double					m_fNextPreferredTime;
	float					m_fPreferredDelay;

	int						m_cLinkBounds;
	LTVector*				m_pvLinkBounds;

	LTVector				m_vLinkEdgeA0;
	LTVector				m_vLinkEdgeA1;
	LTVector				m_vMidPtLinkEdgeA;

	LTVector				m_vLinkEdgeB0;
	LTVector				m_vLinkEdgeB1;
	LTVector				m_vMidPtLinkEdgeB;

	LTVector				m_vLinkDirXZ;
	float					m_fLinkDistXZ;

	float					m_fEntryOffsetDistA;
	float					m_fEntryOffsetDistB;

	float					m_fExitOffsetDistA;
	float					m_fExitOffsetDistB;

	float					m_fFloorBottom;
	float					m_fFloorTop;

	LTObjRef				m_hReservingAI;
};

//-----------------------------------------------------------------

class AINavMeshLinkAbstractPlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLink; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_ABSTRACT_H_
