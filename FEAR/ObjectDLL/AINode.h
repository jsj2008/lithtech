// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef _AI_NODE_H_
#define _AI_NODE_H_

#include "GameBase.h"
#include "DebugLineSystem.h"
#include "AINodeCluster.h"
#include "AIUtils.h"
#include "AIEnumNavMeshTypes.h"
#include "AIEnumAIThreatPosition.h"
#include "AISounds.h"
#include "AIEnumStateTypes.h"
#include "AnimationProp.h"
#include "AIRegion.h"
#include "NamedObjectList.h"
#include "AINodeValidators.h"
#include "AINodeTypes.h"

// Forward declarations.
class	CAI;
class	CAnimationProps;
struct  AIDB_SmartObjectRecord;

LINKTO_MODULE( AINode );

#define NODE_DIMS	16.f



// --------------------------------------------------------------------------

class AINode : public GameBase
{
	public :
		static AINode* HandleToObject(HOBJECT hNode);

	public :

		// Ctor/Dtor

		AINode();
		virtual ~AINode();

		// Init

		virtual void InitNode();
		virtual void AllNodesInitialized() {}

		// Verify

		virtual void Verify();

		// Engine

		virtual uint32 EngineMessageFn(uint32 messageID, void *pvData, float fData);
		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Name

		const char* GetNodeName() const { return m_strName.c_str(); }

		// Owner

		void	SetNodeOwner(HOBJECT hOwner);
		HOBJECT GetNodeOwner();

		// Containing NavMesh poly

		ENUM_NMPolyID GetNodeContainingNMPoly() const { return m_eContainingNMPoly; }
		virtual bool AllowOutsideNavMesh() { return false; }

		// Position/Orientation

		virtual bool GetDestinationPosition(CAI* pAI, const LTVector& vThreatPosition, LTVector& vOutPosition) const { vOutPosition = GetPos(); return true; }

		const LTVector& GetPos() const { return m_vPos; }
		const LTRotation& GetRot() { return m_rRot; }

		// Facing.

		bool			GetFaceNode() const { return m_bFaceNode; }
		const LTVector&	GetNodeFaceDir() const { return	m_vFaceDir; }

		// Radius

		virtual float	GetRadius() const { return m_fRadius; }
		virtual float	GetRadiusSqr() const { return m_fRadius * m_fRadius; }

		// Region

		ENUM_AIRegionID	GetAIRegion() const { return m_eAIRegion; }

		// Boundary Radius.

		virtual float			GetBoundaryRadiusSqr() const { return 0.f; }
		virtual ENUM_AIRegionID	GetBoundaryAIRegion() const { return kAIRegion_Invalid; }

		// Object

		virtual bool	HasObject() { return false; }
		virtual HOBJECT	GetObject() { return NULL; }

		// Status

		uint32					FilterStatusFlags(CAI* pAI, uint32 dwStatusFlags) const;
		virtual bool			IsNodeValid( CAI* /*pAI*/, const LTVector& /*vPosAI*/, HOBJECT /*hThreat*/, EnumAIThreatPosition eThreatPos, uint32 /*dwStatusFlags*/ ) { return true; }
		virtual bool			IsAIInRadiusOrRegion( CAI* /*pAI*/, const LTVector& vPos, float fSearchMult );
		bool					IsNodeInRadiusOrRegion( AINode* pNode );
		bool					IsCharacterInRadiusOrRegion( HOBJECT hChar );
		bool					IsPosInRadiusOrRegion( const LTVector& vPos, ENUM_NMPolyID eNMPoly, float fSearchMult );
		void					GetThreatPosition( CAI* pAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, LTVector* pvPos, ENUM_NMPolyID* peNMPoly );

		// Character type restrictions.

		uint32 GetCharTypeMask() const { return m_dwCharTypeMask; }

		// Grenades.

		virtual bool AllowThrowGrenades() const { return true; }
		virtual bool RequiresStraightPathToThrowGrenades() const { return true; }

		// Lock/Unlock

		virtual void LockNode(HOBJECT hAI);
		virtual void UnlockNode(HOBJECT hAI);
		bool IsNodeLocked();
		HOBJECT GetLockingAI();

		void ResetActivationTime();
		void ResetActivationTime(double fResetTime);
		bool IsNodeTimedOut() const;
		double GetNodeLastActivationTime() { return m_fNodeLastActivationTime; }
		double GetNodeReactivationTime() { return m_fNodeReactivationTime; }
		double GetNodeDepartureTime() const { return m_fNodeDepartureTime; }

		// Enable/Disable

		virtual void DisableNode();	
		virtual void EnableNode();
		
		bool IsNodeDisabled() const
		{
			return !m_EnabledValidator.IsEnabled();
		}

		virtual bool IsLockedDisabledOrTimedOut( HOBJECT hQueryingAI ) { return IsNodeLocked() || IsNodeDisabled() || IsNodeTimedOut(); }

		// Only dynamic.

		bool		IsDynamicOnly() const { return m_bDynamicOnly; }

		// Arrival / Departure.

		virtual void HandleAIArrival( CAI* pAI );
		virtual void HandleAIDeparture( CAI* pAI );

		// Cluster.

		void				SetAINodeClusterID( EnumAINodeClusterID eCluster ) { m_eNodeClusterID = eCluster; }
		EnumAINodeClusterID	GetAINodeClusterID() const { return m_eNodeClusterID; }

		// Type

		virtual EnumAINodeType GetType() const { return kNode_Base; }

		// Debug

		virtual int DrawSelf();
		virtual int HideSelf();
		virtual void GetDebugName( char* pszBuffer, const uint32 nBufferSize );
		virtual	DebugLine::Color GetDebugColor() { if (m_EnabledValidator.IsEnabled()) return Color::Red; else return Color::DkRed; }
		virtual void UpdateDebugDrawStatus( HOBJECT hTarget );

	protected :
	
		std::string		m_strName;

		EnumAINodeClusterID	m_eNodeClusterID;

		LTVector		m_vPos;
		LTRotation		m_rRot;
		
		float			m_fRadius;

		ENUM_AIRegionID	m_eAIRegion;
		std::string		m_strAIRegion;

		double			m_fNodeReactivationTime;
		double			m_fNodeNextActivationTime;
		double			m_fNodeLastActivationTime;
		double			m_fNodeDepartureTime;

		LTObjRef		m_hLockingAI;
		AINodeValidatorEnabled m_EnabledValidator;

		LTObjRef		m_hNodeOwner;

		ENUM_NMPolyID	m_eContainingNMPoly;

		uint32			m_dwCharTypeMask;

		LTVector		m_vFaceDir;
		bool			m_bFaceNode;

		bool			m_bDynamicOnly;

		bool			m_bDebugNodeIsValid;

		// Message Handlers...

		DECLARE_MSG_HANDLER( AINode, HandleEnableMsg );
		DECLARE_MSG_HANDLER( AINode, HandleDisableMsg );
		DECLARE_MSG_HANDLER( AINode, HandleRemoveMsg );
};

class AINodePlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
	virtual LTRESULT PreHook_PropChanged( const char *szObjName, const char *szPropName, const int nPropType, const GenericProp &gpPropValue, ILTPreInterface *pInterface, const char *szModifiers );

private: // Members...

		CCommandMgrPlugin m_CmdMgrPlugin;
};

//---------------------------------------------------------------------------

class AINodeGoto : public AINode
{
	typedef AINode super;

public:

	// Ctors/Dtors/etc

	AINodeGoto();
	virtual ~AINodeGoto();

	// Engine 

	virtual void ReadProp(const GenericPropList *pProps);

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Arrival / Departure.

	virtual void HandleAIArrival( CAI* pAI );

	// Type

	virtual EnumAINodeType GetType() const { return kNode_Goto; }

protected:

	// Command

	bool	HasCmd() { return !m_strGotoCmd.empty(); }
	const char*	GetCmd() { return m_strGotoCmd.c_str(); }

	std::string	m_strGotoCmd;
};

//---------------------------------------------------------------------------

class AINodeView : public AINode
{
	typedef AINode super;

	public :

		// Ctor/Dtor

		AINodeView();
		virtual ~AINodeView();

		// Verify

		virtual void Verify();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
	
		// Type

		EnumAINodeType GetType() const { return kNode_View; }

		// Status

		virtual bool			IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );

	protected :

		AINodeValidatorValidForFollow	m_ValidForFollowValidator;
		AINodeValidatorPlayerOnNode		m_PlayerOnNodeValidator;
};

//---------------------------------------------------------------------------

class AINodeSmartObject : public AINode
{
	typedef AINode super;

	public :

		DEFINE_CAST(AINodeSmartObject);

		// Ctors/Dtors/etc

		AINodeSmartObject();
		virtual ~AINodeSmartObject();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Init

		virtual void InitNode();

		// Object

		virtual bool	HasObject() { return !!m_hAnimObject; }
		virtual HOBJECT GetObject() { return m_hAnimObject; }

		// Dependency.

		HOBJECT GetDependency();

		// Type

		EnumAINodeType GetType() const { return kNode_SmartObject; }

		// Status

		virtual bool			IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );

		// Anim Props.

		virtual void			GetAnimProps( CAnimationProps* pProps );

		// Debug

		virtual void GetDebugName( char* pszBuffer, const uint32 nBufferSize );

		// Methods.

		AIDB_SmartObjectRecord* GetSmartObject();
		void			PreActivate();
		void			PostActivate();

		void			ApplyChildModels( HOBJECT  );
		

	protected :

		std::string			m_strObject;
		LTObjRef			m_hAnimObject;
		std::string			m_strDependency;
		LTObjRef			m_hDependency;
		uint32				m_nSmartObjectID;
		std::string			m_strPreActivateCommand;
		std::string			m_strPostActivateCommand;
};

class AINodeSmartObjectPlugin : public AINodePlugin
{
	typedef std::vector<EnumAINodeType> NodeTypeList;

public:
	AINodeSmartObjectPlugin();

	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
	virtual LTRESULT PreHook_Dims(const char* szRezPath, const char* szPropName, const char* szPropValue, char* szModelFilenameBuf, 
						int	  nModelFilenameBufLen, LTVector & vDims, const char* pszObjName, ILTPreInterface *pInterface);

	void AddValidNodeType(EnumAINodeType eType) { m_ValidNodeTypes.push_back(eType); }

private:
	NodeTypeList m_ValidNodeTypes;
};

//---------------------------------------------------------------------------

class AINodeSearch : public AINodeSmartObject
{
	typedef AINodeSmartObject super;

	public :

		// Ctors/Dtors/etc

		AINodeSearch();
		virtual ~AINodeSearch();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Init

		virtual void AllNodesInitialized();

		// Status

		virtual bool IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );

		// Type

		EnumAINodeType GetType() const { return kNode_Search; }

		// Location.

		EnumAISoundType	GetLocationAISoundType() const { return m_eLocationAISoundType; }

	protected :

		CNamedObjectList	m_PointOfInterestNodes;
		EnumAISoundType		m_eLocationAISoundType;
};


class AINodeSearchPlugin : public AINodeSmartObjectPlugin
{
	typedef AINodeSmartObjectPlugin super;

public:
	AINodeSearchPlugin()
	{
		AddValidNodeType(kNode_Search);
	}

	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

//---------------------------------------------------------------------------

class AINodeAmbush : public AINodeSmartObject
{
	typedef AINodeSmartObject super;

public :

	// Ctor/Dtor

	AINodeAmbush();
	virtual ~AINodeAmbush();

	// Engine 

	virtual void ReadProp(const GenericPropList *pProps);

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Init

	virtual void InitNode();

	// Status

	virtual bool IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );

	// Arrival / Departure.

	virtual void HandleAIArrival( CAI* pAI );

	// Methods

	virtual float	GetBoundaryRadiusSqr() const { return m_BoundaryRadiusValidator.GetBoundaryRadiusSqr(); }
	virtual ENUM_AIRegionID	GetBoundaryAIRegion() const { return m_BoundaryRadiusValidator.GetBoundaryAIRegion(); }
	float			GetThreatRadiusSqr() const { return m_ThreatRadiusValidator.GetThreatRadiusSqr(); }
	bool			IsIgnoreDir() const { return m_bIgnoreDir; }

	// Type

	EnumAINodeType GetType() const { return kNode_Ambush; }

	// Debug

	virtual	DebugLine::Color GetDebugColor();

protected :

	AINodeValidatorDamaged			m_DamagedValidator;
	AINodeValidatorAvoid			m_AvoidValidator;
	AINodeValidatorValidForFollow	m_ValidForFollowValidator;
	AINodeValidatorPlayerOnNode		m_PlayerOnNodeValidator;
	AINodeValidatorThreatRadius		m_ThreatRadiusValidator;
	AINodeValidatorBoundaryRadius	m_BoundaryRadiusValidator;

	bool		m_bIgnoreDir;
	float		m_fFovDp;
	float		m_fMinExpiration;
	float		m_fMaxExpiration;
	double		m_fExpirationTime;

	bool	HasCmd() { return !m_strAmbushCmd.empty(); }
	const char*	GetCmd() { return m_strAmbushCmd.c_str(); }

	std::string	m_strAmbushCmd;
};

class AINodeAmbushPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeAmbushPlugin()
	{
		AddValidNodeType(kNode_Ambush);
	}
};

//---------------------------------------------------------------------------

class AINodeCover : public AINodeSmartObject
{
	typedef AINodeSmartObject super;

	public :

		// Ctor/Dtor

		AINodeCover();
		virtual ~AINodeCover();

		// Verify

		virtual void Verify();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Init

		virtual void InitNode();

		// Status

		virtual bool IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );
		virtual bool IsAIInRadiusOrRegion( CAI* /*pAI*/, const LTVector& vPos, float fSearchMult );

		// Grenades.

		virtual bool AllowThrowGrenades() const { return m_bThrowGrenades; }
		virtual bool RequiresStraightPathToThrowGrenades() const { return false; }

		// Arrival / Departure.

		virtual void HandleAIArrival( CAI* pAI );
		virtual void HandleAIDeparture( CAI* pAI );

		// Anim Props.

		virtual void GetAnimProps( CAnimationProps* pProps );

		// Methods

		virtual float	GetBoundaryRadiusSqr() const { return m_BoundaryRadiusValidator.GetBoundaryRadiusSqr(); }
		virtual ENUM_AIRegionID	GetBoundaryAIRegion() const { return m_BoundaryRadiusValidator.GetBoundaryAIRegion(); }
		float			GetThreatRadiusSqr() const { return m_ThreatRadiusValidator.GetThreatRadiusSqr(); }
		bool			IsIgnoreDir() const { return m_bIgnoreDir; }

		// Type

		EnumAINodeType GetType() const { return kNode_Cover; }

		// Debug

		virtual	DebugLine::Color GetDebugColor();

	protected :

		AINodeValidatorDamaged			m_DamagedValidator;
		AINodeValidatorAvoid			m_AvoidValidator;
		AINodeValidatorValidForFollow	m_ValidForFollowValidator;
		AINodeValidatorPlayerOnNode		m_PlayerOnNodeValidator;
		AINodeValidatorThreatRadius		m_ThreatRadiusValidator;
		AINodeValidatorBoundaryRadius	m_BoundaryRadiusValidator;

		bool		m_bThrowGrenades;

		bool		m_bIgnoreDir;
		float		m_fFovDp;
};

class AINodeCoverPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeCoverPlugin()
	{
		AddValidNodeType(kNode_Cover);
	}
};

//---------------------------------------------------------------------------

class AINodePatrol : public AINodeSmartObject
{
	typedef AINodeSmartObject super;

	public :

		// Ctors/Dtors/etc

		AINodePatrol();
		virtual ~AINodePatrol();

		// Init

		virtual void InitNode();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Arrival / Departure.

		virtual void HandleAIArrival( CAI* pAI );

		// Next

		AINodePatrol* GetNext() { return m_pNext; }
		AINodePatrol* GetPrev() { return m_pPrev; }

		// Path locking.

		void ClaimPatrolPath( CAI* pAI, bool bClaim );

		// Command

		virtual bool HasCmd() { return !m_strPatrolCommand.empty(); }
		virtual const char* GetCmd() { return m_strPatrolCommand.c_str(); }
		
		// Type

		EnumAINodeType GetType() const { return kNode_Patrol; }

		// Debug drawing

		virtual int DrawSelf();
		virtual int HideSelf();

	protected :

		void SetPrev(AINodePatrol* pPrev);

	protected :

		AINodePatrol*	m_pNext;
		std::string		m_strNext; 

		AINodePatrol*	m_pPrev;

		std::string		m_strPatrolCommand;
};

class AINodePatrolPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodePatrolPlugin()
	{
		AddValidNodeType( kNode_Patrol );
		AddValidNodeType( kNode_WorkItem );
		AddValidNodeType( kNode_MenacePlace );
	}
};

//---------------------------------------------------------------------------

class AINodeVehicle : public AINodeSmartObject
{
	typedef AINodeSmartObject super;

public :

	DEFINE_CAST(AINodeVehicle);

	// Ctors/Dtors/etc

	AINodeVehicle();
	virtual ~AINodeVehicle();

	// Init

	virtual void InitNode();

	// Engine 

	virtual void ReadProp(const GenericPropList *pProps);

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// KeyframeToRigidBody.

	HOBJECT		GetVehicleKeyframeToRigidBody() const { return m_hVehicleKeyframeToRigidBody; }

	// Type

	EnumAINodeType GetType() const { return kNode_Vehicle; }

protected :

	LTObjRef		m_hVehicleKeyframeToRigidBody;
	std::string		m_strVehicleKeyframeToRigidBody;
};

class AINodeVehiclePlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeVehiclePlugin()
	{
		AddValidNodeType( kNode_Vehicle );
	}
};

#endif // _AI_NODE_H_
