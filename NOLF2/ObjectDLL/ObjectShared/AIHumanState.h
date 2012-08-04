// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STATE_H__
#define __AI_HUMAN_STATE_H__

#include "AIState.h"
#include "AIHumanStrategy.h"
#include "AITypes.h"

// Forward declarations.
class CAI;
class CAIHuman;

class AIRegion;

class AIVolume;
class AIVolumeJunction;

class AINode;
class AINodeBackup;
class AINodeVantage;
class AINodeCover;
class AINodeSearch;
class AINodePickupObject;
class AINodeUseObject;
class AINodeTail;
class AINodePatrol;
class AINodeAssassinate;
class CAIHumanStrategyToggleLights;
enum  EnumAISoundType;
enum  EnumAIStimulusID;
enum  EnumTrackedNodeGroup;

class CAIHumanState : public CAIState
{
	typedef CAIState super;

	public :

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(State);

		CAIHumanState( );
		virtual ~CAIHumanState( );

		// Ctors/Dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Update

		virtual void PreUpdate();
		virtual void PostUpdate();
		virtual void UpdateAnimation();

		// Handlers

		virtual void HandleModelString(ArgList* pArgList);

		// Misc

		virtual LTBOOL DelayChangeState();

		void SetPose(EnumAnimProp ePose) { m_ePose = ePose; }
		EnumAnimProp GetPose() const { return m_ePose; }

	protected :

		// Simple accessors

		CAIHuman* GetAI() { return m_pAIHuman; }
		CAIHuman* GetAI() const { return m_pAIHuman; }
		CAnimationContext* GetAnimationContext();

	protected :

		CAIHuman*		m_pAIHuman;
		LTBOOL			m_bInterrupt;
		EnumAnimProp	m_ePose;

		CAIHumanStrategyFollowPath*		m_pStrategyFollowPath;
		CAIHumanStrategyDodge*			m_pStrategyDodge;
		CAIHumanStrategyCover*			m_pStrategyCover;
		CAIHumanStrategyShoot*			m_pStrategyShoot;
		CAIHumanStrategyGrenade*		m_pStrategyGrenade;
		CAIHumanStrategyOneShotAni*		m_pStrategyOneShotAni;
		CAIHumanStrategyFlashlight*		m_pStrategyFlashlight;
		CAIHumanStrategyTaunt*			m_pStrategyTaunt;
		CAIHumanStrategyToggleLights*	m_pStrategyToggleLights;
};

class CAIHumanStateIdle : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateIdle, kState_HumanIdle);

		CAIHumanStateIdle( );

		LTBOOL Init(CAIHuman* pAIHuman);

		// Update

		void UpdateAnimation();

		// Return query

        LTBOOL CanReturn() { return LTTRUE; }

		// Simple acccessors

};

class CAIHumanStateAware : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAware, kState_HumanAware);

		CAIHumanStateAware( );

		// Ctors/dtors/etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Return query

        LTBOOL CanReturn() { return LTTRUE; }

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }
		
		void SetPlayOnce(LTBOOL bPlayOnce) { m_bPlayOnce = bPlayOnce; m_bFirstUpdate = LTTRUE; }
		void SetAISound(EnumAISoundType eAISound) { m_eAISound = eAISound; }

	protected :

		LTBOOL			m_bPlayOnce;
		EnumAISoundType m_eAISound;
};

class CAIHumanStateLookAt : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateLookAt, kState_HumanLookAt);

		CAIHumanStateLookAt( );

		// Ctors/dtors/etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }

		void SetPos(const LTVector& vPos);
		void SetAISound(EnumAISoundType eAISound) { m_eAISound = eAISound; }
		void SetPause(LTBOOL b) { m_bPause = b; }

	protected :

		CAnimationProp		m_aniLook;
		EnumAISoundType		m_eAISound;
		LTBOOL				m_bPause;
};

class CAIHumanStateDraw : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDraw, kState_HumanDraw);

		 CAIHumanStateDraw( );
		~CAIHumanStateDraw( );

		// Ctors/Dtors/Etc

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		LTBOOL Init(CAIHuman* pAIHuman);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// Simple acccessors

		void SetFaceTarget(LTBOOL bFace) { m_bFaceTarget = bFace; }
//		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		LTBOOL				m_bDrew;
		LTBOOL				m_bFired;
		LTBOOL				m_bFaceTarget;
		EnumAIWeaponType	m_eWeaponType;
		EnumAnimProp		m_eWeaponProp;
};

class CAIHumanStateHolster : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateHolster, kState_HumanHolster);

		CAIHumanStateHolster( );

		// Ctors/Dtors/Etc

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		LTBOOL Init(CAIHuman* pAIHuman);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :

		LTBOOL			m_bHolstered;
		EnumAnimProp	m_eWeapon;
};

class CAIHumanStateAttack : public CAIHumanState
{
	typedef CAIHumanState super;

	enum EnumAttackFlags
	{
		kAttk_None				= 0x00,
		kAttk_ForceDodge		= 0x01,
		kAttk_ForceCrouch		= 0x02,
		kAttk_Crouching			= 0x04,
	};

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttack, kState_HumanAttack);

		CAIHumanStateAttack( );
		~CAIHumanStateAttack( );

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleDamage(const DamageStruct& damage);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

		void SetChaseDelay(LTFLOAT fDelay);
		void SetPosture(EnumAnimProp ePosture);
		const LTVector& GetScatterPosition() const { return m_pStrategyDodge->GetScatterPosition(); }

		LTBOOL RequestCrouch(HOBJECT hRequestingAI);
		LTBOOL RequestDodge(HOBJECT hRequestingAI);

		LTBOOL IsDodging() { return m_pStrategyDodge->IsDodging(); }

	protected :

		// Chase

		LTBOOL CanChase(LTBOOL bOutOfRange);

		// Face

		void FaceTarget(LTBOOL bFace);

	protected :

		LTFLOAT			m_fChaseTimer;
		LTFLOAT			m_fChaseDelay;
		CAnimationProp	m_aniPosture;
		uint32			m_dwAttackFlags;
		LTFLOAT			m_fCrouchTimer;
};

// For constructor.  warning C4355: 'this' : used in base member initializer list
#pragma warning( push )
#pragma warning( disable : 4355 )

class CAIHumanStateAttackProp : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackProp, kState_HumanAttackProp);

		CAIHumanStateAttackProp( );
		~CAIHumanStateAttackProp( );

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

		void SetProp(HOBJECT hProp);

	protected :

		LTObjRef		m_hProp;
};

#pragma warning( pop )

class CAIHumanStateAssassinate : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAssassinate, kState_HumanAssassinate);

		CAIHumanStateAssassinate( );
		~CAIHumanStateAssassinate( );

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

		void SetNode(AINodeAssassinate* pNode);

	protected :

		LTBOOL		m_bIgnoreVisibility;
};

class CAIHumanStateAttackFromCover : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackFromCover, kState_HumanAttackFromCover);

		CAIHumanStateAttackFromCover( );
		~CAIHumanStateAttackFromCover();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		EnumAIStrategyType GetRandomCoverStrategy(AINodeCover* pNode);
		LTBOOL SetCoverStrategy(EnumAIStrategyType eStrategy);

		// Handlers

		bool HandleCommand(const CParsedMsg &cMsg);
		void HandleDamage(const DamageStruct& damage);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

		void SetRetries(int32 nRetries) { m_nRetries = nRetries; }
		void SetCoverNode(HOBJECT hCoverNode);

	protected :

		// Update

		void UpdateFindCover();
		void UpdateGotoCover();
		void UpdateUseCover();

	protected :

        AINodeCover*	m_pCoverNode;
		int32			m_nRetries;
		LTBOOL			m_bBoostedHitpoints;
		LTBOOL			m_bCovered;
};

class CAIHumanStateAttackFromVantage : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackFromVantage, kState_HumanAttackFromVantage);

		CAIHumanStateAttackFromVantage( );
		~CAIHumanStateAttackFromVantage();

		// Ctors/Dtors/Etc

		LTBOOL	Init(CAIHuman* pAIHuman);
		void	CleanupState();

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		bool HandleCommand(const CParsedMsg &cMsg);
		void HandleDamage(const DamageStruct& damage);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }
		void SetVantageNode(HOBJECT hVantageNode);
		HOBJECT GetVantageNode();
		void IgnoreVisibility(LTBOOL bIgnoreVisibility) { m_bIgnoreVisibility = bIgnoreVisibility; }

	protected :

		void UpdateMoving();
		void UpdateAttacking();

	protected :

        AINodeVantage*	m_pVantageNode;
		LTFLOAT			m_fAttackTimer;
		LTBOOL			m_bIgnoreVisibility;
		LTBOOL			m_bFired;
};

class CAIHumanStateAttackFromView : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackFromView, kState_HumanAttackFromView);

		CAIHumanStateAttackFromView( );
		~CAIHumanStateAttackFromView();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

		void SetNode(AINode* pViewNode);

	protected :

		void UpdateMoving();
		void UpdateAttacking();

	protected :

        AINode*	m_pViewNode;
		LTFLOAT	m_fChaseTimer;
};

class CAIHumanStateCover : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateCover, kState_HumanCover);

		CAIHumanStateCover( );
		~CAIHumanStateCover();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		EnumAIStrategyType GetRandomCoverStrategy(AINodeCover* pCoverNode);
		LTBOOL SetCoverStrategy(EnumAIStrategyType eStrategy);

		// Handlers

		void HandleDamage(const DamageStruct& damage);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }
		void SetCoverNode(HOBJECT hCoverNode);

	protected :

        AINodeCover*	m_pCoverNode;
};

class CAIHumanStatePanic : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStatePanic, kState_HumanPanic);

		CAIHumanStatePanic( );
		~CAIHumanStatePanic();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Misc

		LTBOOL CanActivate() { return m_bCanActivate; }
		void SetCanActivate(LTBOOL bCanActivate) { m_bCanActivate = bCanActivate; }

		// Simple acccessors

		void SetPanicNode(AINodePanic* pPanicNode);

	protected :

		void GetRandomPanic(AINodePanic* pNode, CAnimationProp* pProp);

	protected :

		LTBOOL			m_bAtPanicDestination;
        AINodePanic*	m_pPanicNode;
		LTBOOL			m_bCanActivate;
		CAnimationProp	m_aniPanic;
};

class CAIHumanStateDistress : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDistress, kState_HumanDistress);

		CAIHumanStateDistress( );

		// Ctors/Dtors/Etc

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		LTBOOL Init(CAIHuman* pAIHuman);

		// Update

		void Update();
		void UpdateAnimation();

		// Misc

		LTBOOL CanActivate() { return m_bCanActivate; }
		void SetCanActivate(LTBOOL bCanActivate) { m_bCanActivate = bCanActivate; }

		// Simple acccessors

	protected :

		int		m_nDistressLevel;
		LTFLOAT	m_fDistress;
		LTBOOL	m_bCanActivate;
};

class CAIHumanStatePatrol : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStatePatrol, kState_HumanPatrol);

		CAIHumanStatePatrol( );
		~CAIHumanStatePatrol( );

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Return query

		LTBOOL CanReturn() { return !!m_pNode; }

		// Handlers

		void HandleVolumeEnter(AIVolume* pVolume);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

		void SetNode(AINodePatrol* pNode);
		AINodePatrol* GetNode() { return m_pNode; }
		AINodePatrol* GetLastNode() { return m_pLastNode; }
		void SetAwareness(EnumAnimProp eAwareness);

	protected :

		LTBOOL UpdateTaskWait();
		LTBOOL UpdateTaskAnimate();

	protected :

		LTFLOAT					m_fWaitTimer;
		LTFLOAT					m_fTalkTimer;

		EnumAnimProp			m_eAwareness;

		LTBOOL					m_bForward;
        AINodePatrol*			m_pNode;
        AINodePatrol*			m_pLastNode;
};

class CAIHumanStateGoto : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateGoto, kState_HumanGoto);

		CAIHumanStateGoto( );
		~CAIHumanStateGoto( );

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Update

		virtual void Update();
		void UpdateAnimation();

		// Return query

        LTBOOL CanReturn() { return LTTRUE; }

		// Handlers

		void HandleVolumeEnter(AIVolume* pVolume);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

		void SetDest(const LTVector& vDest) { m_vDest = vDest; }
		void SetDestNode(HOBJECT hDestNode);
		void SetMovement(EnumAnimProp eMovement) { m_eMovement = eMovement;	m_pStrategyFollowPath->SetMovement(eMovement); }
		void SetAwareness(EnumAnimProp eAwareness) { m_eAwareness = eAwareness; }
		void SetMood(EnumAnimProp eMood) { m_eMood = eMood; }
		EnumAnimProp GetMood() const { return m_eMood; }
		void SetWeaponPosition(EnumAnimProp eWeaponPosition) { m_eWeaponPosition = eWeaponPosition; }
		void SetLoopingSound(EnumAISoundType eLoopingSound) { m_eLoopingAISound = eLoopingSound; }
		void SetCloseEnoughDist(LTFLOAT fDist) { m_fCloseEnoughDistSqr = fDist;	m_fCloseEnoughDistSqr *= m_fCloseEnoughDistSqr; }
		void SetCloseEnoughDistSqr(LTFLOAT fDistSqr) { m_fCloseEnoughDistSqr = fDistSqr; }
		void TurnOffLights(LTBOOL bTurnOffLights) { m_bTurnOffLights = bTurnOffLights; }

	protected :
	
		AINode* SafeGetGotoNode(int32 iGotoNode);

	protected :

		enum Constants
		{
			kMaxGotoNodes = 12,
		};

	protected :

		LTVector		m_vDest;
		LTObjRef		m_hDestNode;
        AINode*			m_apNodes[kMaxGotoNodes];
		int				m_cNodes;
		int				m_iNextNode;
		LTBOOL			m_bLoop;
		EnumAnimProp	m_eMovement;
		EnumAnimProp	m_eAwareness;
		EnumAnimProp	m_eMood;
		EnumAnimProp	m_eWeaponPosition;
		EnumAISoundType	m_eLoopingAISound;
		LTFLOAT			m_fCloseEnoughDistSqr;
		LTBOOL			m_bTurnOffLights;
};

class CAIHumanStateFlee : public CAIHumanStateGoto
{
	typedef CAIHumanStateGoto super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateFlee, kState_HumanFlee);

		CAIHumanStateFlee( );

		// Ctors/Dtors/etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple accessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

		void SetDanger(HOBJECT hDanger);

	protected :

		LTObjRef	m_hDanger;
};

class CAIHumanStateSearch : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateSearch, kState_HumanSearch);

		CAIHumanStateSearch( );
		~CAIHumanStateSearch( );

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleVolumeEnter(AIVolume* pVolume);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }

		void SetPause(LTBOOL b) { m_bPause = b; }
		void SetEngage(LTBOOL b) { m_bEngage = b; }
		void SetFace(LTBOOL b) { m_bFace = b; }
		void SetDestRegion(HOBJECT hRegion) { m_hDestRegion = hRegion; }
		void SetLimitSearchCount(LTBOOL bLimit) { m_bLimitSearchCount = bLimit; }
		void		IgnoreJunctions(LTBOOL b) { m_bIgnoreJunctions = b; }
		AIVolume*	GetLastVolume() const { return m_pLastVolume; }
		AIVolume*	GetJunctionVolume() const { return m_pJunctionVolume; }

	protected :

		LTBOOL FindNode();
		void GetRandomSearch(AINodeSearch* pNode, CAnimationProps* paniSearch) const;

	protected :

		AINodeSearch*	m_pSearchNode;
		AIRegion*		m_pSearchRegion;
		LTObjRef		m_hDestRegion;

		LTBOOL			m_bFace;
		LTBOOL			m_bEngage;
		LTBOOL			m_bPause;
		LTBOOL			m_bDone;
		LTBOOL			m_bAdded;
		LTBOOL			m_bSearching;
		LTBOOL			m_bIgnoreJunctions;

		LTFLOAT			m_fNextPause;

		LTBOOL			m_bCheckedLantern;

		AIVolume*		m_pLastVolume;
		AIVolume*		m_pJunctionVolume;

		LTBOOL			m_bLimitSearchCount;
		uint32			m_cSearchedNodes;

		CAnimationProps	m_aniSearch;
};

class CAIHumanStateUseObject : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateUseObject, kState_HumanUseObject);

		CAIHumanStateUseObject( );
		~CAIHumanStateUseObject();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleVolumeEnter(AIVolume* pVolume);
		void HandleNameValuePair(char *szName, char *szValue);
		void HandleModelString(ArgList* pArgList);
		void HandleDamage(const DamageStruct& damage);

		// Methods.

		void DoPickupObject();
		void Pause(LTBOOL bPause);
		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

		virtual LTBOOL SetNode(AINodeUseObject* pUseNode);
		void StateHandlesNodeLocking(LTBOOL bStateHandlesNodeLocking) { m_bStateHandlesNodeLocking = bStateHandlesNodeLocking; }
		void SetAwareness(EnumAnimProp eAwareness) { m_eAwareness = eAwareness; }
		void SetActivity(EnumAnimProp eActivity) { m_eActivity = eActivity; }
		void SetWeaponPosition(EnumAnimProp eWeaponPosition) { m_eWeaponPosition = eWeaponPosition; }
		void SetSmartObjectCommand(HSTRING hstrCmd) { m_hstrSmartObjectCmd = hstrCmd; }
		void SetRequireBareHands(LTBOOL b) { m_bRequireBareHands = b; }
		void SetVulnerable(LTBOOL b) { m_bVulnerable = b; }
		void SetLoopingSound(EnumAISoundType eLoopingAISound) { m_eLoopingAISound = eLoopingAISound; }
		void SetAlertFirst(LTBOOL b) { m_bAlertFirst = b; }
		void TurnOffLights(LTBOOL b) { m_bTurnOffLights = b; }
		void TurnOnLights(LTBOOL b) { m_bTurnOnLights = b; }
		EnumAnimProp GetActivity() const { return m_eActivity; }
		EnumAnimProp GetMood() const { return m_eMood; }
		LTFLOAT GetAnimTimeRemaining() { return m_fAnimTime - m_fAnimTimer; }
		void AddAnimTime(LTFLOAT fTime) { m_fAnimTimer -= fTime; }
		void StartAnimation();

	protected :

		LTObjRef			m_hObject;
        AINodeUseObject*	m_pUseNode;
		LTBOOL				m_bLeaveNodeLocked;
		LTBOOL				m_bStateHandlesNodeLocking;
		HSTRING				m_hstrSmartObjectCmd;
		LTBOOL				m_bPickedUp;
		EnumAnimProp		m_eAction;
		EnumAnimProp		m_eActivity;
		EnumAnimProp		m_eAwareness;
		EnumAnimProp		m_eWeaponPosition;
		EnumAnimProp		m_eWeaponAction;
		EnumAnimProp		m_eMood;
		LTFLOAT				m_fAnimTime;
		LTFLOAT				m_fAnimTimer;
		LTFLOAT				m_fNextFidgetTime;
		LTFLOAT				m_fMinFidgetTime;
		LTFLOAT				m_fMaxFidgetTime;
		EnumAISoundType		m_eLoopingAISound;
		LTBOOL				m_bRequireBareHands;
		LTBOOL				m_bTurnOffLights;
		LTBOOL				m_bTurnOnLights;
		LTBOOL				m_bVulnerable;
		LTBOOL				m_bPlayedFirstSound;
		LTBOOL				m_bAlertFirst;
		EnumTrackedNodeGroup m_eTrackingGroup;
};

class CAIHumanStateTail : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateTail, kState_HumanTail);

		CAIHumanStateTail();
		~CAIHumanStateTail();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :

		enum Constants
		{
			kMaxTailNodes = 16,
		};

	protected :

		AINodeTail*		m_pTailNode;
		uint32			m_cTailNodes;
		AINodeTail*		m_apTailNodes[kMaxTailNodes];
};

class CAIHumanStateFollowFootprint : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateFollowFootprint, kState_HumanFollowFootprint);

		CAIHumanStateFollowFootprint();
		~CAIHumanStateFollowFootprint();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }

		void ResetFootprint(LTFloat fTimestamp, const LTVector& vPos);
		void SetSearch(LTBOOL bSearch) { m_bSearch = bSearch; }

	protected :

		LTFLOAT		m_fNewTimestamp;
		LTFLOAT		m_fLastTimestamp;
		LTVector	m_vFootprintPos;

		LTFLOAT		m_fLatestTimestamp;
		LTBOOL		m_bSearch;
};

class CAIHumanStateInvestigate : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateInvestigate, kState_HumanInvestigate);

		CAIHumanStateInvestigate();
		~CAIHumanStateInvestigate();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }

		void Reset(HOBJECT hStimulusSource, EnumAISenseType eSenseType, EnumAISoundType eAISound, LTVector& vStimulusPos, LTVector& vStimulusDir);
		void SetSearch(LTBOOL bSearch) { m_bSearch = bSearch; }
		void SetCloseEnoughDistSqr(LTFLOAT fDistSqr) { m_fCloseEnoughDistSqr = fDistSqr; }
		void SetPause(LTBOOL b) { m_bPause = b; }

	protected :

		LTObjRef			m_hEnemy;
		EnumAISenseType		m_eSenseType;
		LTVector			m_vPosition;
		LTVector			m_vDirection;
		LTFLOAT				m_fCloseEnoughDistSqr;
		LTBOOL				m_bSearch;
		EnumAISoundType		m_eAISound;
		LTBOOL				m_bPause;
		LTBOOL				m_bCheckedLantern;
};

class CAIHumanStateCheckBody : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateCheckBody, kState_HumanCheckBody);

		CAIHumanStateCheckBody();
		~CAIHumanStateCheckBody();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }

		void SetBody(HOBJECT hBody);

	protected :

		LTObjRef		m_hBody;
		EnumAnimProp	m_eCheckAnim;
		LTBOOL			m_bPathToBody;
};

class CAIHumanStateChase : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateChase, kState_HumanChase);

		CAIHumanStateChase();
		~CAIHumanStateChase();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleVolumeEnter(AIVolume* pVolume);
		void HandleVolumeExit(AIVolume* pVolume);

		// Junction Volume Handling

		void SetRandomPathFromJunction();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

		AIVolume*		GetLastVolume() const { return m_pLastVolume; }
		AIVolume*		GetJunctionVolume() const { return m_pJunctionVolume; }
		AIVolume*		GetJunctionCorrectVolume() const { return m_pJunctionCorrectVolume; }
		void			SetLastVolume(AIVolume* pLastVolume) { m_pLastVolume = pLastVolume; }
		void			SetJunctionVolume(AIVolume* pJunctionVolume) { m_pJunctionVolume = pJunctionVolume; }
		void			SetJunctionActionVolume(AIVolume* pJunctionActionVolume) { m_pJunctionActionVolume = pJunctionActionVolume; }
		void			SeekTarget(LTBOOL bSeek) { m_bSeekTarget = bSeek; }
		void			KeepDistance(LTBOOL bKeepDistance) { m_bKeepDistance = bKeepDistance; }
		LTBOOL			IsOutOfBreath() const { return m_bOutOfBreath; }

		virtual void	SetStateStatus(EnumAIStateStatus eStateStatus);

	protected :
		LTBOOL			CalculatePath();

		LTBOOL			m_bSeen;
		LTVector		m_vSeenPosition;
		LTBOOL			m_bCanGetLost;

		LTBOOL			m_bStatusChanged;

		LTBOOL			m_bUseStraightPath;
		LTBOOL			m_bSeekTarget;

		LTFLOAT			m_fSeeEnemyTime;
		LTFLOAT			m_fEnduranceTime;
		LTFLOAT			m_fVisionBlockedTime;

		LTBOOL			m_bKeepDistance;

		LTBOOL			m_bOutOfBreath;

		AIVolume*		m_pLastVolume;
		AIVolume*		m_pJunctionVolume;
		AIVolume*		m_pJunctionActionVolume;
		AIVolume*		m_pJunctionCorrectVolume;
};

class CAIHumanStateGetBackup : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateGetBackup, kState_HumanGetBackup);

		CAIHumanStateGetBackup( );
		~CAIHumanStateGetBackup();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

		void SetDest(AINodeBackup* pNode);
		void SetEnemySeenPos( LTVector& vPos );


	protected :

        AINodeBackup*		m_pBackupNode;
};

class CAIHumanStateTalk : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateTalk, kState_HumanTalk);

		CAIHumanStateTalk( );
		~CAIHumanStateTalk();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		bool HandleCommand(const CParsedMsg &cMsg);

		// Gestures

		void SetGesture(HSTRING hstrGesture);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

		void SetMood(EnumAnimProp eMood) { m_eMood = eMood; }
		void SetFace(HOBJECT hFace);
		void SetFaceTime(LTFLOAT fTime);
		void SetGuardNode(HOBJECT hGuardNode);

	protected :

		EnumAnimProp	m_eMood;
		LTObjRef		m_hFace;
		LTFLOAT			m_fFaceTime;
		LTFLOAT			m_fFaceTimer;
		LTVector		m_vInitialForward;
		LTObjRef		m_hGuardNode;
};

class CAIHumanStateCharge : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateCharge, kState_HumanCharge);

		CAIHumanStateCharge( );
		~CAIHumanStateCharge();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

		void SetAttackDistSqr(LTFLOAT fDist) { m_fAttackDistanceSqr = fDist; }
		void SetYellDistSqr(LTFLOAT fDist) { m_fYellDistanceSqr = fDist; }
		void SetStopDistSqr(LTFLOAT fDist) { m_fStopDistanceSqr = fDist; }

	protected :

		LTBOOL		m_bAttacking;
		LTFLOAT		m_fAttackDistanceSqr;
		LTBOOL		m_bYelled;
		LTFLOAT		m_fYellDistanceSqr;
		LTBOOL		m_bStopped;
		LTFLOAT		m_fStopDistanceSqr;
};

class CAIHumanStateAnimate : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAnimate, kState_HumanAnimate);

		CAIHumanStateAnimate( );
		~CAIHumanStateAnimate();

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();

		// Return query

        LTBOOL CanReturn() { return LTTRUE; }

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

		void SetAnimation(HSTRING hstrAnim, LTBOOL bLoop);
		void SetAnimation(const CAnimationProps& animProps, LTBOOL bLoop);

	protected :

		HSTRING		m_hstrAnim;
		LTBOOL		m_bLoop;
		LTBOOL		m_bResetAnim;
};

class CAIHumanStateFollow : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateFollow, kState_HumanFollow);

		CAIHumanStateFollow( );
		~CAIHumanStateFollow();

		// Ctors/Dtors/Etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

		void SetRangeSqr(LTFLOAT fRangeSqr) { m_fRangeSqr = fRangeSqr; }
		void SetRangeTime(LTFLOAT fRangeTime) { m_fRangeTime = fRangeTime; }
		void SetMovement(EnumAnimProp eMovement);

	private :
		void HandleFollowing();
		void HandleHolding();

		void TransitionToFollowing();
		void TransitionToHolding();

		float Get2DDistance(const LTVector& vOtherPos) const;
		LTVector GetTargetPosition() const ;

		LTFLOAT		m_fRangeSqr;			// Distance from the target that is 'close enough'
		LTFLOAT		m_fRangeTime;			// How frequently the AI should face the target
		LTFLOAT		m_fRangeTimer;			// How long till the AI should next face the target
		LTFLOAT		m_fTimer;				// Time since the AI last updated the destination position
		float		m_fBaseAIHoverAcceleration; // Original acceleration on function entry
};
 
class CAIHumanStateLongJump : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateLongJump, kState_HumanLongJump);

		CAIHumanStateLongJump( );
		~CAIHumanStateLongJump( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// Attack counting.

		void CountAttackers(LTBOOL bCount);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }
		
		void	SetStopDistance(const float flDist);
		LTFLOAT GetStopDistance() const { return m_fLongJumpStopDist; }
		void	SetLongJumpDest(const LTVector& vLongJumpDest) { m_vLongJumpDest = vLongJumpDest; } 
		void	SetLongJumpSequence(EnumAnimProp eAnimProp1, EnumAnimProp eAnimProp2, EnumAnimProp eAnimProp3) { m_eAnimProp1 = eAnimProp1; m_eAnimProp2 = eAnimProp2; m_eAnimProp3 = eAnimProp3; } 
		void	SetLongJumpToTarget(LTBOOL bJumpToTarget) { m_bJumpToTarget = bJumpToTarget; }
		void	SetLongJumpSpeed(LTFLOAT fSpeed) { m_fJumpSpeed = fSpeed; }
		void	FaceDest(LTBOOL bFaceDest) { m_bFaceDest = bFaceDest; }

	protected:

		void	SetupLongJumpMovement();

	protected :

		LTBOOL			m_bFired;
		LTBOOL			m_bJumpToTarget;
		LTFLOAT			m_fLongJumpHeightMin;
		LTFLOAT			m_fLongJumpHeightMax;
		LTVector		m_vLongJumpDest;
		LTFLOAT			m_fLongJumpStopDist;
		EnumAnimProp	m_eAnimProp1;
		EnumAnimProp	m_eAnimProp2;
		EnumAnimProp	m_eAnimProp3;
		uint32			m_iAnimRandomSeed;
		LTBOOL			m_bFaceDest;
		LTFLOAT			m_fJumpSpeed;
		LTBOOL			m_bCountAttackers;
};

#endif
