// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef _AI_NODE_H_
#define _AI_NODE_H_

#include "ltengineobjects.h"
#include "IObjectPlugin.h"
#include "AnimationProp.h"
#include "GameBase.h"
#include "DebugLineSystem.h"

// Forward declarations.
enum EnumAISoundType;
enum EnumAINodeType;
enum EnumAIStateType;

class AIVolume;
class CAIHuman;

LINKTO_MODULE( AINode );

//
// ENUM: Types of AI nodes.
//
enum EnumAINodeType
{
	kNode_InvalidType = -1,

	#define AINODE_TYPE_AS_ENUM 1
	#include "AINodeTypeEnums.h"
	#undef AINODE_TYPE_AS_ENUM

	kNode_Count,
};

//
// STRINGS: const strings for AI node types.
//
static const char* s_aszAINodeTypes[] =
{
	#define AINODE_TYPE_AS_STRING 1
	#include "AINodeTypeEnums.h"
	#undef AINODE_TYPE_AS_STRING
};


enum EnumNodeStatus
{
	kStatus_Invalid,
	kStatus_Ok,
	kStatus_TooFar,
	kStatus_ThreatOutsideFOV,
	kStatus_ThreatInsideRadius,
	kStatus_ThreatBlockingPath,
	kStatus_SearchedRecently,
	kStatus_RequiresPartner,
};

class AINode : public GameBase
{
	public :

		static AINode* HandleToObject(HOBJECT hNode)
		{
			return (AINode*)g_pLTServer->HandleToObject(hNode);
		}

	public :

		// Ctor/Dtor

		AINode();
		virtual ~AINode();

		// Init

		virtual void Init();

		// Verify

		virtual void Verify();

		// Engine

		virtual uint32 EngineMessageFn(uint32 messageID, void *pvData, LTFLOAT fData);
		virtual void ReadProp(ObjectCreateStruct *pocs);
		virtual bool OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Name

		HSTRING GetName() const { return m_hstrName; }

		// Face

		LTBOOL ShouldFaceNodeForward() const { return m_bFaceNodeForward; }

		// Owner

		void	SetNodeOwner(HOBJECT hOwner);
		HOBJECT GetNodeOwner() { return m_hNodeOwner; }

		// Containing volume

		AIVolume *GetNodeContainingVolume() const { return m_pContainingVolume; }

		// Position/Orientation

		const LTVector& GetPos() const { return m_vPos; }
		const LTVector& GetUp() { return m_vUp; }
		const LTVector& GetForward() { return m_vForward; }
		const LTVector& GetRight() { return m_vRight; }

		// Radius

		virtual LTFLOAT	GetRadius() const { return m_fRadius; }
		virtual LTFLOAT	GetRadiusSqr() const { return m_fRadiusSqr; }

		// Command

		virtual LTBOOL HasCmd() { return LTFALSE; }
		virtual HSTRING	GetCmd() { return LTNULL; }

		// Object

		virtual LTBOOL	HasObject() { return LTFALSE; }
		virtual HSTRING	GetObject() { return LTNULL; }

		// Status

		virtual EnumNodeStatus	GetStatus(const LTVector& vPos, HOBJECT hThreat) const { return kStatus_Ok; }

		// Alignment

		int GetRequiredRelationTemplateID() const { return m_nRequiredRelationTemplateID; }

		// Lock/Unlock

		void Lock(HOBJECT hAI);
		void Unlock(HOBJECT hAI);
		LTBOOL IsLocked() const;
		uint32 GetLockCount() const { return m_nNodeLockCount; }
		
		void ResetActivationTime();
		void ResetActivationTime(LTFLOAT fResetTime);
		LTBOOL IsTimedOut() const;
		LTFLOAT GetNodeLastActivationTime() { return m_fNodeLastActivationTime; }
		LTFLOAT GetNodeReactivationTime() { return m_fNodeReactivationTime; }

		// Enable/Disable

		void Disable()
		{
			m_bNodeEnabled = LTFALSE;
		}
		
		void Enable()
		{
			m_bNodeEnabled = LTTRUE;
		}
		
		LTBOOL IsDisabled() const
		{
			return !m_bNodeEnabled;
		}

		LTBOOL IsLockedDisabledOrTimedOut() const { return IsLocked() || IsDisabled() || IsTimedOut(); }

		// Type

		virtual EnumAINodeType GetType() { return kNode_Base; }
		virtual LTBOOL NodeTypeIsActive(EnumAINodeType eNodeType) { return eNodeType == GetType(); }

		// Debug

		int DrawSelf();
		int HideSelf();
		virtual	DebugLine::Color GetDebugColor() { if (m_bNodeEnabled) return Color::Red; else return Color::DkRed; }

	protected :
	
		HSTRING		m_hstrName;

		LTVector	m_vPos;

		LTVector	m_vInitialPitchYawRoll;
		LTVector	m_vUp;
		LTVector	m_vForward;
		LTVector	m_vRight;

		LTFLOAT		m_fRadius;
		LTFLOAT		m_fRadiusSqr;

		LTBOOL		m_bFaceNodeForward;

		LTFLOAT		m_fNodeReactivationTime;
		LTFLOAT		m_fNodeNextActivationTime;
		LTFLOAT		m_fNodeLastActivationTime;

		uint32		m_nNodeLockCount;
		LTBOOL		m_bNodeEnabled;

		LTObjRef	m_hNodeOwner;

		bool		m_bContainingVolumeValid;
		AIVolume *	m_pContainingVolume;

		int			m_nRequiredRelationTemplateID;
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

class AINodeDeath : public AINode
{
public:
	virtual EnumAINodeType GetType() { return kNode_Death; }
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

	virtual void ReadProp(ObjectCreateStruct *pocs);

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Command

	virtual LTBOOL	HasCmd() { return !!m_hstrGotoCmd; }
	virtual HSTRING	GetCmd() { return m_hstrGotoCmd; }

	// Type

	virtual EnumAINodeType GetType() { return kNode_Goto; }

protected:

	HSTRING	m_hstrGotoCmd;
};

//----------------------------------------------------------------------------
//              
//	CLASS:		AINodeChangeWeapons
//              
//	PURPOSE:	Base class which addes the ability to specify a weapon change
//				at a particular node.  Doing this through inheritance is a bit
//				nasty, but it seems to be the best way to handle it at this 
//				point without adding some new structural to the Node system.
//              
//----------------------------------------------------------------------------
class AINodeChangeWeapons :  public AINode
{
protected:
	struct ChangeWeaponSet
	{
		std::string m_szChangeToWeapon;
		std::string m_szChangeToWeaponRequirement;
	};

public:
	AINodeChangeWeapons();
	virtual ~AINodeChangeWeapons();

	// Interface:

	virtual void ReadProp(ObjectCreateStruct* pocs);
	virtual void Save(ILTMessage_Write *pMsg);
	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Verify();

	// Added functionality:

	const ChangeWeaponSet* const GetWeaponTransition(CAIHuman* pAIHuman) const;
	bool ShouldChangeWeapons(CAIHuman* pAIHuman) const;
	const char* const GetWeaponString(CAIHuman* pAIHuman) const;

	static int GetWeaponCount() { return cm_WeaponCount; }

protected:
	static const int cm_WeaponCount;

private:

	// Copy Constructor and Asignment Operator private to prevent 
	// automatic generation and inappropriate, unintentional use
	AINodeChangeWeapons(const AINodeChangeWeapons& rhs) {}
	AINodeChangeWeapons& operator=(const AINodeChangeWeapons& rhs ) {}

	std::vector<ChangeWeaponSet> m_WeaponSets;
};

//---------------------------------------------------------------------------

class AINodeCover : public AINodeChangeWeapons
{
	typedef AINodeChangeWeapons super;

	public :

		enum Flags
		{
			kFlagDuck				= 0x00000001,
			kFlagBlind				= 0x00000002,
			kFlag1WayCorner			= 0x00000004,
			kFlag2WayCorner			= 0x00000008,
			kFlagAny				= 0x0000000F,
			kNumFlags				= 4,
		};

	public :

		// Ctor/Dtor

		AINodeCover();
		virtual ~AINodeCover();

		// Verify

		virtual void Verify();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Status

		virtual EnumNodeStatus	GetStatus(const LTVector& vPos, HOBJECT hThreat) const;

		// Object

		virtual LTBOOL	HasObject() { return !!m_hstrObject; }
		virtual HSTRING	GetObject() { return m_hstrObject; }
		void			ClearObject();

		// Methods

		LTFLOAT	GetThreatRadiusSqr() const { return m_fThreatRadiusSqr; }
		LTBOOL	IsIgnoreDir() const { return m_bIgnoreDir; }
		uint32	GetFlags() { return m_dwFlags; }
		LTFLOAT GetHitpointsBoost() const { return m_fHitpointsBoost; }
		LTFLOAT GetTimeout() const { return m_fTimeout; }
		
		// Type

		EnumAINodeType GetType() { return kNode_Cover; }

	protected :

		uint32		m_dwFlags;
		HSTRING		m_hstrObject;
		LTFLOAT		m_fThreatRadiusSqr;
		LTBOOL		m_bIgnoreDir;
		LTFLOAT		m_fFovDp;
		LTFLOAT		m_fTimeout;

		LTFLOAT		m_fHitpointsBoost;
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

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		
		// Type

		EnumAINodeType GetType() { return kNode_View; }

	protected :
};

//---------------------------------------------------------------------------

class AINodeSearch : public AINode
{
	typedef AINode super;

	public :

		enum Flags
		{
			kFlagShineFlashlight	= 0x00000001,
			kFlagLookUnder			= 0x00000002,
			kFlagLookOver			= 0x00000004,
			kFlagLookUp				= 0x00000008,
			kFlagLookLeft			= 0x00000010,
			kFlagLookRight			= 0x00000020,
			kFlagKnockOnDoor		= 0x00000040,
			kFlagAlert				= 0x00000080,
			kFlagAny				= 0x000000FF,
			kNumFlags				= 8,
		};

		enum EnumSearchType
		{
			kSearch_Default,
			kSearch_OneWay,
			kSearch_Corner,
		};

	public :

		// Ctors/Dtors/etc

		AINodeSearch();
		virtual ~AINodeSearch();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Status

		virtual EnumNodeStatus	GetStatus(const LTVector& vPos, HOBJECT hThreat) const;

		// Methods

		void Search();
		void SearchReset() { m_bSearched = LTFALSE; }

		uint32 GetFlags() const { return m_dwFlags; }
		EnumSearchType GetSearchType() { return m_eSearchType; }

		// Type

		EnumAINodeType GetType() { return kNode_Search; }

	protected :

		uint32			m_dwFlags;
		LTBOOL			m_bSearched;
		EnumSearchType	m_eSearchType;
};

class AINodeSearchPlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

//---------------------------------------------------------------------------

class AINodePanic : public AINode
{
	typedef AINode super;

	public :

		enum Flags
		{
			kFlagStand				= 0x00000001,
			kFlagCrouch				= 0x00000002,
			kFlagAny				= 0x00000003,
			kNumFlags				= 2,
		};

	public :

		// Ctors/Dtors/etc

		AINodePanic();
		virtual ~AINodePanic();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Object

		virtual LTBOOL	HasObject() { return !!m_hstrObject; }
		virtual HSTRING	GetObject() { return m_hstrObject; }
		void			ClearObject();

		// Methods

		uint32 GetFlags() { return m_dwFlags; }
		
		// Type

		EnumAINodeType GetType() { return kNode_Panic; }

	protected :

		uint32		m_dwFlags;
		HSTRING		m_hstrObject;
};

//---------------------------------------------------------------------------

class AINodeVantage : public AINodeChangeWeapons
{
	typedef AINodeChangeWeapons super;

	public :

		// Ctors/Dtors/etc

		AINodeVantage();
		virtual ~AINodeVantage();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Object

		virtual LTBOOL	HasObject() { return !!m_hstrObject; }
		virtual HSTRING	GetObject() { return m_hstrObject; }

		// Status

		virtual EnumNodeStatus	GetStatus(const LTVector& vPos, HOBJECT hThreat) const;

		// Methods

		LTFLOAT GetThreatRadiusSqr() const { return m_fThreatRadiusSqr; }
		LTBOOL IsIgnoreDir() const { return m_bIgnoreDir; }

		// Type

		EnumAINodeType GetType() { return kNode_Vantage; }

	protected :

		HSTRING		m_hstrObject;
		LTFLOAT		m_fThreatRadiusSqr;
		LTBOOL		m_bIgnoreDir;
		LTFLOAT		m_fFovDp;
};

//---------------------------------------------------------------------------

class AINodeVantageRoof : public AINodeVantage
{
	public :
		// Type
		EnumAINodeType GetType() { return kNode_VantageRoof; }
};

//---------------------------------------------------------------------------

class AINodeAssassinate : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeAssassinate();
		virtual ~AINodeAssassinate();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Type

		EnumAINodeType GetType() { return kNode_Assassinate; }

		// Methods.

		EnumAnimProp	GetMovement() const { return m_eMovement; }
		LTBOOL			IgnoreVisibility() const { return m_bIgnoreVisibility; }

	protected :

		EnumAnimProp		m_eMovement;
		LTBOOL				m_bIgnoreVisibility;
};

class AINodeAssassinatePlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

//---------------------------------------------------------------------------

class AINodeUseObject : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeUseObject();
		virtual ~AINodeUseObject();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Init

		virtual void Init();

		// Object

		virtual LTBOOL	HasObject() { return !!m_hstrObject; }
		virtual HSTRING	GetObject() { return m_hstrObject; }
		HOBJECT	GetHObject();

		// Properties

		LTBOOL IsOneWay() const { return m_bOneWay; }

		// Type

		EnumAINodeType GetType() { return kNode_UseObject; }
		virtual LTBOOL NodeTypeIsActive(EnumAINodeType eNodeType);

		// Status

		virtual EnumNodeStatus	GetStatus(const LTVector& vPos, HOBJECT hThreat) const;

		// Methods.

		HSTRING			GetSmartObjectCommand(EnumAINodeType eNodeType);
		EnumAIStateType	GetSmartObjectState() { return m_eSmartObjectState; }
		void			SetSmartObjectState(EnumAIStateType eStateType) { m_eSmartObjectState = eStateType; }
		EnumAnimProp	GetMovement() const { return m_eMovement; }
		void			SetMovement(EnumAnimProp eMovement) { m_eMovement = eMovement; }
		EnumAISoundType	GetFirstSound() const { return m_eFirstSound; }
		void			SetFirstSound(EnumAISoundType eSound) { m_eFirstSound = eSound; }
		EnumAISoundType	GetFidgetSound() const { return m_eFidgetSound; }
		void			PreActivate();
		void			PostActivate();

		void			ApplyChildModels( HOBJECT  );
		

	protected :

		HSTRING				m_hstrObject;
		LTObjRef			m_hUseObject;
		EnumAnimProp		m_eMovement;
		uint32				m_nSmartObjectID;
		EnumAISoundType		m_eFirstSound;
		EnumAISoundType		m_eFidgetSound;
		HSTRING				m_hstrPreActivateCommand;
		HSTRING				m_hstrPostActivateCommand;
		EnumAIStateType		m_eSmartObjectState;
		LTBOOL				m_bOneWay;

		
};

class AINodeUseObjectPlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

//---------------------------------------------------------------------------

class AINodeBackup : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeBackup();
		virtual ~AINodeBackup();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Command

		virtual LTBOOL	HasCmd() { return !!m_hstrBackupCmd; }
		virtual HSTRING	GetCmd() { return m_hstrBackupCmd; }

		// Type

		EnumAINodeType GetType() { return kNode_Backup; }

		// Status

		virtual EnumNodeStatus	GetStatus(const LTVector& vPos, HOBJECT hThreat) const;

		// Methods.

		EnumAnimProp	GetMovement() const { return m_eMovement; }
		void			SetEnemySeenPos(LTVector& vPos) { m_vEnemySeenPos = vPos; }		
		const LTVector&	GetEnemySeenPos() const { return m_vEnemySeenPos; }		
		LTFLOAT			GetStimulusRadius() { return m_fStimulusRadius; }
		LTFLOAT			GetStimulusRadiusSqr() { return m_fStimulusRadiusSqr; }
		uint32			GetCryWolfCount() const { return m_cCryWolf; }
		uint32			GetArrivalCount() const { return m_cArrivalCount; }
		void			IncrementArrivalCount() { ++m_cArrivalCount; }

	protected :

		HSTRING			m_hstrBackupCmd;
		EnumAnimProp	m_eMovement;
		LTVector		m_vEnemySeenPos;
		LTFLOAT			m_fStimulusRadius;
		LTFLOAT			m_fStimulusRadiusSqr;
		uint32			m_cCryWolf;
		uint32			m_cArrivalCount;
};

class AINodeBackupPlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

//---------------------------------------------------------------------------

class AINodeTraining : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeTraining();
		virtual ~AINodeTraining();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Command

		virtual LTBOOL	HasCmd() { return !!m_hstrCmd; }
		virtual HSTRING	GetCmd() { return m_hstrCmd; }

		// Type

		EnumAINodeType GetType() { return kNode_Training; }

	protected :

		HSTRING	m_hstrCmd;
};

//---------------------------------------------------------------------------

class AINodeTrainingFailure : public AINodeTraining
{
	typedef AINodeTraining super;

	public :

		// Ctors/Dtors/etc

		AINodeTrainingFailure();
		virtual ~AINodeTrainingFailure();

		// Type

		EnumAINodeType GetType() { return kNode_TrainingFailure; }
};

//---------------------------------------------------------------------------

class AINodeTrainingSuccess : public AINodeTraining
{
	typedef AINodeTraining super;

	public :

		// Ctors/Dtors/etc

		AINodeTrainingSuccess();
		virtual ~AINodeTrainingSuccess();

		// Type

		EnumAINodeType GetType() { return kNode_TrainingSuccess; }
};

//---------------------------------------------------------------------------

class AINodeTail : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeTail();
		virtual ~AINodeTail();

		// Type

		EnumAINodeType GetType() { return kNode_Tail; }
};

//---------------------------------------------------------------------------

class AINodePatrol : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodePatrol();
		virtual ~AINodePatrol();

		// Init

		virtual void Init();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Next

		AINodePatrol* GetNext() { return m_pNext; }
		AINodePatrol* GetPrev() { return m_pPrev; }

		// Command

		virtual LTBOOL HasCmd() { return !!m_hstrPatrolCommand; }
		virtual HSTRING GetCmd() { return m_hstrPatrolCommand; }

		// PatrolAction

		EnumAnimProp GetAction() { return m_ePatrolAction; }
		
		// Type

		EnumAINodeType GetType() { return kNode_Patrol; }

	protected :

		friend class AINodePatrol;

		void SetPrev(AINodePatrol* pPrev);

	protected :

		union {
		AINodePatrol*	m_pNext;
		HSTRING			m_hstrNext;	};
		AINodePatrol*	m_pPrev;

		HSTRING			m_hstrPatrolCommand;

		EnumAnimProp	m_ePatrolAction;
};

class AINodePatrolPlugin : public CEditStringPlugin
{
public:
	AINodePatrolPlugin();
	virtual LTRESULT PreHook_PropChanged( const char *szObjName, const char *szPropName, const int nPropType, const GenericProp &gpPropValue, ILTPreInterface *pInterface, const char *szModifiers );

private: // Members...

	AINodePlugin	m_AINodePlugin;
};

//----------------------------------------------------------------------------
//              
//	CLASS:		AINodeObstruct
//              
//	PURPOSE:	Obstruction nodes are to be placed in positions where, if an 
//				AI moved to this location, they will .. Obstruct something.
//				Typically, this may mean that the AI is obstruction either 
//				another AI, or player progress.  This is primarily for defensive
//				movements.
//              
//----------------------------------------------------------------------------
class AINodeObstruct : public AINode
{
	typedef AINode super;

	public :
		// Public members

		enum Flags
		{
			kFlagVantage			= 0x01,
			kFlagAny				= 0x01,
			kNumFlags				= 1,
		};

	public :

		// Ctors/Dtors/etc

		AINodeObstruct();
		virtual ~AINodeObstruct();

		// Engine 

		virtual void ReadProp(ObjectCreateStruct *pocs);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Status
		virtual EnumNodeStatus	GetStatus(const LTVector& vPos, HOBJECT hThreat) const;
		EnumAnimProp		GetMovement() const { return m_eMovement; }

		// Methods

		uint32	GetFlags() { return m_dwFlags; }
		LTFLOAT	GetThreatRadiusSqr() const { return m_fThreatRadiusSqr; }
		HSTRING	GetThreatRadiusReaction() const { return m_hstrThreatRadiusReaction; }
		LTBOOL	IsIgnoreDir() const { return m_bIgnoreDir; }
		HSTRING	GetDamageCmd() const { return m_hstrDamageCmd; }

		// Type

		EnumAINodeType GetType() { return kNode_Obstruct; }

	protected:
		// Protected members

		uint32		m_dwFlags;
		LTFLOAT		m_fThreatRadiusSqr;
		LTBOOL		m_bIgnoreDir;
		LTFLOAT		m_fFovDp;
		HSTRING		m_hstrThreatRadiusReaction;
		HSTRING		m_hstrDamageCmd;
		EnumAnimProp m_eMovement;

	private:
		bool		 IsThreatInRadius(const LTVector& vThreatPos) const;
		bool		 IsThreatInFOV(const LTVector& vThreatPos) const;

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		AINodeObstruct(const AINodeObstruct& rhs) {}
		AINodeObstruct& operator=(const AINodeObstruct& rhs ) {}
};

#endif // _AI_NODE_H_
