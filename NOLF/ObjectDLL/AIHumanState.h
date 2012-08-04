// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STATE_H__
#define __AI_HUMAN_STATE_H__

#include "AIState.h"
#include "AIMovement.h"
#include "AIHumanStrategy.h"
#include "AINudge.h"

class CAI;
class CAIHuman;

class CAIHumanState : public CAIState
{
	typedef CAIState super;
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIHumanState);

	public :

		typedef enum AIHumanStateType
		{
			eStateIdle,
			eStateAware,
			eStateLookAt,
			eStateDrowsy,
			eStateUnconscious,
			eStateStunned,
			eStateAssassinate,
			eStateDraw,
			eStateAttack,
			eStateAttackFromCover,
			eStateAttackFromVantage,
			eStateAttackFromView,
			eStateAttackOnSight,
			eStateAttackProp,
			eStateCover,
			eStatePanic,
			eStateDistress,
			eStatePatrol,
			eStateGoto,
			eStateFlee,
			eStateSearch,
			eStateChase,
			eStateTail,
			eStateFollowFootprint,
			eStateInvestigate,
			eStateCheckBody,
			eStatePickupObject,
			eStateUseObject,
			eStateTalk,
			eStateGetBackup,
			eStateScript,
			eStateCharge,
			eStateAnimate,
//			eStateCome,
			eStateFollow,
			eStateParaDive,
			eStateParaShoot,
			eStateParaDie,
			eStateParaEscape,
			eStateHeliAttack,
			eStateScotBox,
			eStateScotWin,
			eStateIngeSing,
			kNumStateTypes,
		};

	public :

		// Ctors/Dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEREAD hWrite);

		// Update

		virtual void PreUpdate();
		virtual void PostUpdate();
		virtual void UpdateSenses();
		virtual void UpdateAnimation();

		// Handlers

		virtual void HandleNameValuePair(char *szName, char *szValue);
		virtual void HandleBrokenLink(HOBJECT hObject);
		virtual void HandleModelString(ArgList* pArgList);

		// Simple accessors

		virtual AIHumanStateType GetType() = 0;
		virtual CNudge::Priority GetNudgePriority() { return CNudge::ePriorityLow; }
		virtual const char* GetName() { return "AIHumanState"; }

		// Misc

		void SearchOr(const char* szOr);
		LTBOOL CanChangeToState(AIHumanStateType eStateType) { return m_abCanChangeToState[eStateType]; }

	protected :

		// Simple accessors

		CAIHuman* GetAI() { return m_pAIHuman; }
		CAnimationContext* GetAnimationContext();

	protected :

		enum Pose
		{
			ePoseDefault,	// Query the animation context for our pose
			ePoseStand,
			ePoseSit,
		};

	protected :

		CAIHuman*	m_pAIHuman;
		LTBOOL		m_bInterrupt;
		Pose		m_ePose;

		CAIHumanStrategyFollowPath*		m_pStrategyFollowPath;
		CAIHumanStrategyDodge*			m_pStrategyDodge;
		CAIHumanStrategyCover*			m_pStrategyCover;
		CAIHumanStrategyShoot*			m_pStrategyShoot;
		CAIHumanStrategyGrenade*		m_pStrategyGrenade;
		CAIHumanStrategyOneShotAni*		m_pStrategyOneShotAni;
		CAIHumanStrategyFlashlight*		m_pStrategyFlashlight;

		LTBOOL							m_abCanChangeToState[kNumStateTypes];
};

class CAIHumanStateIdle : DEFINE_FACTORY_CLASS(CAIHumanStateIdle), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateIdle);

	public :

		// Update

		void UpdateAnimation();

		// Return query

        LTBOOL CanReturn() { return LTTRUE; }
		HSTRING CreateReturnString();

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateIdle; }
		const char* GetName() { return "Idle"; }
};

class CAIHumanStateAware : DEFINE_FACTORY_CLASS(CAIHumanStateAware), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateAware);

	public :

		// Ctors/dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();

		// Return query

        LTBOOL CanReturn() { return LTTRUE; }
		HSTRING CreateReturnString();

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateAware; }
		const char* GetName() { return "Aware"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }

	protected :

		CAnimationProp	m_aniAlert;
};

class CAIHumanStateLookAt : DEFINE_FACTORY_CLASS(CAIHumanStateLookAt), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateLookAt);

	public :

		// Ctors/dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateLookAt; }
		const char* GetName() { return "LookAt"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :

		CAnimationProp	m_aniLook;
};

class CAIHumanStateDrowsy : DEFINE_FACTORY_CLASS(CAIHumanStateDrowsy), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateDrowsy);

	public :

		// Ctors/dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses();

		// Handlers

		LTBOOL HandleCommand(char** pTokens, int nArgs);
		void HandleDamage(const DamageStruct& damage);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateDrowsy; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "Drowsy"; }

	protected :

		// Helpers

		void Awake();
		void Asleep();

	protected :

		enum State
		{
			eStateAsleep,
			eStateAwake,
		};

	protected :

		State	m_eState;
};

class CAIHumanStateUnconscious : DEFINE_FACTORY_CLASS(CAIHumanStateUnconscious), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateUnconscious);

	public :

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		LTBOOL Init(CAIHuman* pAIHuman);

		// Update 

		void Update();
		void UpdateAnimation();
		void UpdateSenses() {}

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);
		void HandleDamage(const DamageStruct& damage);

		// Simple accessors

		AIHumanStateType GetType() { return CAIHumanState::eStateUnconscious; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "Unconscious"; }
		LTBOOL RejectChangeState() { return m_eState != eStateConscious; }
		HMODELANIM GetDeathAni(LTBOOL bFront);

	protected :

		enum State
		{
			eStateUnconscious,
			eStateRegainingConsciousness,
			eStateConscious,
		};

		State		m_eState;
		LTFLOAT		m_fRegainConsciousnessTime;
		LTBOOL		m_bAware;
		LTFLOAT		m_fUnconsciousTime;
};

class CAIHumanStateStunned : DEFINE_FACTORY_CLASS(CAIHumanStateStunned), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateStunned);

	public :

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		LTBOOL Init(CAIHuman* pAIHuman);

		// Update 

		void Update();
		void UpdateAnimation();
		void UpdateSenses() {}

		// Simple accessors

		AIHumanStateType GetType() { return CAIHumanState::eStateStunned; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "Stunned"; }
		LTBOOL RejectChangeState() { return m_fElapsedTime <= 15.0f; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		LTFLOAT		m_fYellTimer;
};

class CAIHumanStateDraw : DEFINE_FACTORY_CLASS(CAIHumanStateDraw), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateDraw);

	public :

		// Ctors/Dtors/Etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() {}

		// Handlers

		void HandleModelString(ArgList* pArgList);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateDraw; }
		const char* GetName() { return "Draw"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		LTBOOL		m_bDrew;
};

class CAIHumanStateAttack : DEFINE_FACTORY_CLASS(CAIHumanStateAttack), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateAttack);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleDamage(const DamageStruct& damage);
		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateAttack; }
		CNudge::Priority GetNudgePriority();
		const char* GetName() { return "Attack"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		// Chase

		LTBOOL CanChase(LTBOOL bOutOfRange);

	protected :

		LTFLOAT			m_fChaseTimer;
		LTBOOL			m_bChase;
		LTFLOAT			m_fChaseDelay;
		CAnimationProp	m_aniPosture;
};

class CAIHumanStateAttackProp : DEFINE_FACTORY_CLASS(CAIHumanStateAttackProp), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateAttackProp);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);
		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateAttackProp; }
		const char* GetName() { return "AttackProp"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		HOBJECT		m_hProp;
};

class CAIHumanStateAssassinate : DEFINE_FACTORY_CLASS(CAIHumanStateAssassinate), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateAssassinate);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateAssassinate; }
		const char* GetName() { return "Assassinate"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		LTBOOL		m_bIgnoreVisibility;
};

class CAIHumanStateAttackFromCover : DEFINE_FACTORY_CLASS(CAIHumanStateAttackFromCover), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateAttackFromCover);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		CAIHumanStrategy::AIHumanStrategyType GetRandomCoverStrategy(CAINode* pNode);
		LTBOOL SetCoverStrategy(CAIHumanStrategy::AIHumanStrategyType eStrategy);

		// Handlers

		LTBOOL HandleCommand(char** pTokens, int nArgs);
		void HandleNameValuePair(char *szName, char *szValue);
		void HandleDamage(const DamageStruct& damage);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateAttackFromCover; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "AttackFromCover"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		// Update

		void UpdateFindCover();
		void UpdateGotoCover();
		void UpdateUseCover();

	protected :

		enum State
		{
			eStateFindCover,
			eStateGotoCover,
			eStateUseCover,
		};

	protected :

		State	m_eState;
        uint32  m_dwCoverNode;
		int32	m_nRetries;
};

class CAIHumanStateAttackFromVantage : DEFINE_FACTORY_CLASS(CAIHumanStateAttackFromVantage), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateAttackFromVantage);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		LTBOOL HandleCommand(char** pTokens, int nArgs);
		void HandleDamage(const DamageStruct& damage);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateAttackFromVantage; }
		const char* GetName() { return "AttackFromVantage"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		void UpdateMoving();
		void UpdateAttacking();

	protected :

		enum State
		{
			eStateUnset,
			eStateMoving,
			eStateAttacking,
		};

	protected :

		State	m_eState;
        uint32  m_dwVantageNode;
		LTFLOAT	m_fAttackTimer;
};

class CAIHumanStateAttackFromView : DEFINE_FACTORY_CLASS(CAIHumanStateAttackFromView), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateAttackFromView);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateAttackFromView; }
		const char* GetName() { return "AttackFromView"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		LTBOOL FindView();

		void UpdateMoving();
		void UpdateAttacking();

	protected :

		enum State
		{
			eStateUnset,
			eStateMoving,
			eStateAttacking,
		};

	protected :

		State	m_eState;
        uint32  m_dwViewNode;
		LTFLOAT	m_fChaseTimer;
};

class CAIHumanStateAttackOnSight : DEFINE_FACTORY_CLASS(CAIHumanStateAttackOnSight), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateAttackOnSight);

	public :

		// Ctors/Dtors/Etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleDamage(const DamageStruct& damage);
		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateAttackOnSight; }
		const char* GetName() { return "AttackOnSight"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		LTBOOL		m_bChaseDelay;
};

class CAIHumanStateCover : DEFINE_FACTORY_CLASS(CAIHumanStateCover), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateCover);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses();

		CAIHumanStrategy::AIHumanStrategyType GetRandomCoverStrategy(CAINode* pNode);
		LTBOOL SetCoverStrategy(CAIHumanStrategy::AIHumanStrategyType eStrategy);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateCover; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "Cover"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		enum State
		{
			eStateGoingToCover,
			eStateAtCover,
		};

	protected :

		State	m_eState;
        uint32  m_dwCoverNode;
};

class CAIHumanStatePanic : DEFINE_FACTORY_CLASS(CAIHumanStatePanic), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStatePanic);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Misc

		LTBOOL CanActivate() { return m_bCanActivate; }

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStatePanic; }
		const char* GetName() { return "Panic"; }

	protected :

		const CAnimationProp& GetRandomPanic(CAINode* pNode) const;

	protected :

		LTBOOL			m_bAtPanicDestination;
        uint32			m_dwPanicNode;
		LTBOOL			m_bCanActivate;
		CAnimationProp	m_aniPanic;
};

class CAIHumanStateDistress : DEFINE_FACTORY_CLASS(CAIHumanStateDistress), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateDistress);

	public :

		// Ctors/Dtors/Etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Misc

		LTBOOL CanActivate() { return m_bCanActivate; }

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateDistress; }
		const char* GetName() { return "Distress"; }

	protected :

		int		m_nDistressLevel;
		LTFLOAT	m_fDistress;
		LTBOOL	m_bCanActivate;
};

class CAIHumanStatePatrol : DEFINE_FACTORY_CLASS(CAIHumanStatePatrol), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStatePatrol);



	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Return query

		LTBOOL CanReturn() { return m_cNodes > 0; }
		HSTRING CreateReturnString();

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStatePatrol; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "Patrol"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :

		LTBOOL UpdateTaskWait();
		LTBOOL UpdateTaskAnimate();

		CAINode* SafeGetPatrolNode(int32 iPatrolNode);

	protected :

		enum Task
		{
			eTaskWait,
			eTaskAnimate,
		};

		enum Constants
		{
			kMaxPatrolNodes = 12,
		};

	protected :

		LTFLOAT					m_fWaitTimer;
		LTFLOAT					m_fTalkTimer;

		LTBOOL					m_bForward;
        uint32					m_adwNodes[kMaxPatrolNodes];
		int						m_cNodes;
		int						m_iNextNode;
		int						m_iDirNode;
		LTBOOL					m_bFace;
		LTBOOL					m_bLoop;
		LTBOOL					m_bCircle;

		Task					m_eTask;
		CAnimationProp			m_aniTask;
};

class CAIHumanStateGoto : DEFINE_FACTORY_CLASS(CAIHumanStateGoto), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateGoto);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Update

		virtual void Update();
		void UpdateAnimation();

		// Handlers

		virtual void HandleNameValuePair(char *szName, char *szValue);

		// Return query

        LTBOOL CanReturn() { return LTTRUE; }
		HSTRING CreateReturnString();

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateGoto; }
		const char* GetName() { return "Goto"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :
	
		CAINode* SafeGetGotoNode(int32 iGotoNode);

	protected :

		enum Constants
		{
			kMaxGotoNodes = 12,
		};

	protected :

		LTBOOL		m_bFace;
		LTVector	m_vDest;
        uint32		m_adwNodes[kMaxGotoNodes];
		int			m_cNodes;
		int			m_iNextNode;
		LTBOOL		m_bLoop;
};

class CAIHumanStateFlee : DEFINE_FACTORY_CLASS(CAIHumanStateFlee), public CAIHumanStateGoto
{
	typedef CAIHumanStateGoto super;
	DEFINE_FACTORY_METHODS(CAIHumanStateFlee);

	public :

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateSenses() {}
		void UpdateAnimation();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);
		void HandleBrokenLink(HOBJECT hObject);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateFlee; }
		const char* GetName() { return "Flee"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		HOBJECT		m_hDanger;
};

class CAIHumanStateSearch : DEFINE_FACTORY_CLASS(CAIHumanStateSearch), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateSearch);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateSearch; }
		const char* GetName() { return "Search"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }

	protected :

		LTBOOL FindNode();
		const CAnimationProp& GetRandomSearch(CAINode* pNode) const;

	protected :

        uint32			m_iSearchNode;
		uint32			m_iSearchRegion;

		LTBOOL			m_bFace;
		LTBOOL			m_bEngage;
		LTBOOL			m_bPause;
		LTBOOL			m_bDone;
		LTBOOL			m_bAdded;
		LTBOOL			m_bSearching;

		CAnimationProp	m_aniSearch;
};

class CAIHumanStatePickupObject : DEFINE_FACTORY_CLASS(CAIHumanStatePickupObject), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStatePickupObject);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);
		void HandleBrokenLink(HOBJECT hObject);
		void HandleModelString(ArgList* pArgList);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStatePickupObject; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "PickupObject"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :

		void DoPickupWeapon();

	protected :

		typedef enum ObjectType
		{
			eNone,
			eWeapon,
		};

		LTBOOL		m_bPickedUp;
		ObjectType	m_eObjectType;
		HOBJECT		m_hObject;
        uint32      m_dwPickupNode;
};

class CAIHumanStateUseObject : DEFINE_FACTORY_CLASS(CAIHumanStateUseObject), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateUseObject);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);
		void HandleBrokenLink(HOBJECT hObject);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateUseObject; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "UseObject"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :

		typedef enum ObjectType
		{
			eNone,
			eAlarm,
			eSwitch,
		};

		ObjectType						m_eObjectType;
		HOBJECT							m_hObject;
        uint32                          m_dwUseNode;
};

class CAIHumanStateTail : DEFINE_FACTORY_CLASS(CAIHumanStateTail), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateTail);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);
		void HandleModelString(ArgList* pArgList);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateTail; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "Tail"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :

		enum State
		{
			eStatePosing,
			eStateMoving,
		};

		enum Constants
		{
			kMaxTailNodes = 16,
		};

	protected :

		State	m_eState;
        uint32  m_dwTailNode;
		int		m_cTailNodes;
        uint32  m_adwTailNodes[kMaxTailNodes];
};

class CAIHumanStateFollowFootprint : DEFINE_FACTORY_CLASS(CAIHumanStateFollowFootprint), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateFollowFootprint);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateFollowFootprint; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "FollowFootprint"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }

	protected :

		LTFLOAT		m_fLatestTimestamp;
		LTBOOL		m_bSearch;
};

class CAIHumanStateInvestigate : DEFINE_FACTORY_CLASS(CAIHumanStateInvestigate), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateInvestigate);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);
		void HandleBrokenLink(HOBJECT hObject);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateInvestigate; }
//		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "Investigate"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }

	protected :

		HOBJECT		m_hEnemy;
		SenseType	m_stSenseType;
		LTVector	m_vPosition;
		LTBOOL		m_bSearch;
};

class CAIHumanStateCheckBody : DEFINE_FACTORY_CLASS(CAIHumanStateCheckBody), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateCheckBody);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);
		void HandleBrokenLink(HOBJECT hObject);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateCheckBody; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "CheckBody"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodInvestigate; }

	protected :

		HOBJECT		m_hBody;
		LTBOOL		m_bSearch;
};

class CAIHumanStateChase : DEFINE_FACTORY_CLASS(CAIHumanStateChase), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateChase);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateChase; }
		const char* GetName() { return "Chase"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		LTFLOAT m_fStopTime;
		LTFLOAT	m_fVisibleTimer;

		LTBOOL	m_bPlayedChaseSound;
		LTBOOL	m_bPlayedFoundSound;
};

class CAIHumanStateGetBackup : DEFINE_FACTORY_CLASS(CAIHumanStateGetBackup), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateGetBackup);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Misc

		LTBOOL DelayChangeState();

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateGetBackup; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "GetBackup"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

        uint32      m_dwNode;
		LTBOOL		m_bSendingTrigger;		// This does not need to get saved.
};

class CAIHumanStateTalk : DEFINE_FACTORY_CLASS(CAIHumanStateTalk), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateTalk);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);
		LTBOOL HandleCommand(char** pTokens, int nArgs);
		void HandleBrokenLink(HOBJECT hObject);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateTalk; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "Talk"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :

		enum Mood
		{
			eMoodHappy,
			eMoodAngry,
			eMoodSad,
			eMoodTense,
			eMoodAgree,
			eMoodDisagree,
		};

	protected :

		Mood		m_eMood;
		HOBJECT		m_hFace;
		LTFLOAT		m_fFaceTime;
		LTFLOAT		m_fFaceTimer;
		LTVector	m_vInitialForward;
};

class CAIHumanStateCharge : DEFINE_FACTORY_CLASS(CAIHumanStateCharge), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateCharge);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateCharge; }
		const char* GetName() { return "Charge"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		LTBOOL		m_bAttacking;
		LTFLOAT		m_fAttackDistanceSqr;
		LTBOOL		m_bYelled;
		LTFLOAT		m_fYellDistanceSqr;
		LTBOOL		m_bStopped;
		LTFLOAT		m_fStopDistanceSqr;
};

class CAIHumanStateAnimate : DEFINE_FACTORY_CLASS(CAIHumanStateAnimate), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateAnimate);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Return query

        LTBOOL CanReturn() { return LTTRUE; }
		HSTRING CreateReturnString();

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateAnimate; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "Animate"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :

		HSTRING		m_hstrAnim;
		LTBOOL		m_bLoop;
};
/*
class CAIHumanStateCome : DEFINE_FACTORY_CLASS(CAIHumanStateCome), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateCome);

	public :

		// Ctors/Dtors/Etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateCome; }
		const char* GetName() { return "Come"; }
};
*/
class CAIHumanStateFollow : DEFINE_FACTORY_CLASS(CAIHumanStateFollow), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateFollow);

	public :

		// Ctors/Dtors/Etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateFollow; }
		const char* GetName() { return "Follow"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodRoutine; }

	protected :

		enum State
		{
			eStateFollowing,
			eStateHolding,
		};

	private :

		State		m_eState;

		LTFLOAT		m_fRangeSqr;

		LTFLOAT		m_fRangeTime;
		LTFLOAT		m_fRangeTimer;

		LTFLOAT		m_fTimer;
};

class CAIHumanStateParaDive : DEFINE_FACTORY_CLASS(CAIHumanStateParaDive), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateParaDive);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		// Handlers

		void HandleTouch(HOBJECT hObject);

		// Update

		void Update();
		void UpdateSenses() { }

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateParaDive; }
		const char* GetName() { return "ParaDive"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }
};

class CAIHumanStateParaShoot : DEFINE_FACTORY_CLASS(CAIHumanStateParaShoot), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateParaShoot);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateSenses() { }

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateParaShoot; }
		const char* GetName() { return "ParaShoot"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }
};

class CAIHumanStateParaDie : DEFINE_FACTORY_CLASS(CAIHumanStateParaDie), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateParaDie);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateSenses() { }

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateParaDie; }
		const char* GetName() { return "ParaDie"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		enum State
		{
			eStateOpenChute,
			eStateDie,
			eStateDied,
		};

	protected :

		State		m_eState;
};

class CAIHumanStateParaEscape : DEFINE_FACTORY_CLASS(CAIHumanStateParaEscape), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateParaEscape);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateSenses() { }

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateParaEscape; }
		const char* GetName() { return "ParaEscape"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		enum State
		{
			eStateOpenChute,
			eStateEscape,
			eStateEscaped,
		};

	protected :

		State		m_eState;
};

class CAIHumanStateHeliAttack : DEFINE_FACTORY_CLASS(CAIHumanStateHeliAttack), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateHeliAttack);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleDamage(const DamageStruct& damage);
		void HandleNameValuePair(char *szName, char *szValue);
		LTBOOL HandleCommand(char** pTokens, int nArgs);
		void HandleBrokenLink(HOBJECT hObject);

		// Damage

		LTBOOL CanBeDamagedAsAttachment();

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateHeliAttack; }
		const char* GetName() { return "HeliAttack"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		HOBJECT m_hHelicopter;
};

class CAIHumanStateScotBox : DEFINE_FACTORY_CLASS(CAIHumanStateScotBox), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateScotBox);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);
		void HandleNameValuePair(char *szName, char *szValue);
		void HandleDamage(const DamageStruct& damage);
		void HandleModelString(ArgList* pArgList);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateScotBox; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "ScotBox"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		void UpdateDynamite();
		void UpdateVictory();
		void UpdateDefeat();
		void UpdateTaunting();
		void UpdateBoxing();
		void UpdatePunching();
		void UpdateClosing();
		void UpdateSlamming();

	protected :

		enum State
		{
			eStateVictory,
			eStateDefeat,
			eStateTaunting,
			eStateBoxing,
			eStatePunching,
			eStateClosing,
			eStateSlamming,
			eStateDynamite,
		};

	protected :

		State			m_eState;
		LTFLOAT			m_fClosingTimer;
		LTFLOAT			m_fBoxTimer;
		CAnimationProp	m_aniPunch;
		LTFLOAT			m_fAnger;
		HSTRING			m_hstrDefeat;
		HOBJECT			m_hDamageMeter;
};

class CAIHumanStateIngeSing : DEFINE_FACTORY_CLASS(CAIHumanStateIngeSing), public CAIHumanState
{
	typedef CAIHumanState super;
	DEFINE_FACTORY_METHODS(CAIHumanStateIngeSing);

	public :

		// Ctors/Dtors/Etc

		LTBOOL Init(CAIHuman* pAIHuman);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Update

		void Update();
		void UpdateAnimation();
		void UpdateSenses() { }

		// Handlers

		LTBOOL HandleCommand(char** pTokens, int nArgs);
		void HandleNameValuePair(char *szName, char *szValue);
		void HandleBrokenLink(HOBJECT hObject);
		void HandleDamage(const DamageStruct& damage);
		void HandleModelString(ArgList* pArgList);

		// Simple acccessors

		AIHumanStateType GetType() { return CAIHumanState::eStateIngeSing; }
		CNudge::Priority GetNudgePriority() { return CNudge::ePriorityHigh; }
		const char* GetName() { return "IngeSing"; }
		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }

	protected :

		// Update helpers

		void UpdateHenchmen();
		HOBJECT FindTeleporter();
		LTBOOL IsTeleporterValid(HOBJECT hTeleporter);

		void UpdatePower();
		void UpdateClosing();
		void UpdateSinging();
		void UpdateSwinging();

	protected :

		enum Constants
		{
			kMaxHenchmen = 32,
			kMaxTeleporters = 32,
			kMaxExplosions = 32,
		};

		enum State
		{
			eStatePower,
			eStateClosing,
			eStateSinging,
			eStateSwinging,
		};

	protected :

		State			m_eState;

		CAnimationProp	m_aniSword;

		LTBOOL			m_bSinging;
		LTFLOAT			m_fSingingTimer;

		LTFLOAT			m_fClosingTimer;

		LTBOOL			m_bAtPower;
		LTBOOL			m_bPowerOn;

		HOBJECT			m_ahTeleporters[kMaxTeleporters];
		uint32			m_cTeleporters;

		HOBJECT			m_ahExplosions[kMaxExplosions];
		uint32			m_cExplosions;

		HOBJECT			m_ahHenchmen[kMaxHenchmen];
		uint32			m_cHenchmen;
		uint32			m_cActiveHenchmen;
		uint32			m_iHenchmanNext;
};

#endif