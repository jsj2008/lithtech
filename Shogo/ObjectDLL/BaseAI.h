// ----------------------------------------------------------------------- //
//
// MODULE  : BaseAI.h
//
// PURPOSE : Generic base AI object
//
// CREATED : 9/29/97
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_AI_H__
#define __BASE_AI_H__

#include "BaseCharacter.h"
#include "AIScriptList.h"
#include "AIPathList.h"
#include "ModelFuncs.h"


// Action flags (what is the AI currently doing)...

#define AI_AFLG_RUN				(1<<0)
#define AI_AFLG_JUMP			(1<<1)
#define AI_AFLG_DUCK			(1<<2)
#define AI_AFLG_WALK			(1<<3)
#define AI_AFLG_TURN_LEFT		(1<<4)
#define AI_AFLG_TURN_RIGHT		(1<<5)
#define AI_AFLG_POSE			(1<<6)
#define AI_AFLG_STRAFE_RIGHT	(1<<7)
#define AI_AFLG_STRAFE_LEFT		(1<<8)
#define AI_AFLG_STAND			(1<<9)
#define AI_AFLG_FIRE			(1<<10)
#define AI_AFLG_RUN_REVERSE		(1<<11)
#define AI_AFLG_WALK_REVERSE	(1<<12)
#define AI_AFLG_AIMING			(1<<13)
#define AI_AFLG_EVASIVE			(1<<14)


// Available Sound flags...

#define AI_SNDFLG_SETIDLE		(1<<0)
#define AI_SNDFLG_SETDEFENSIVE	(1<<1)
#define AI_SNDFLG_SETAGGRESSIVE	(1<<2)
#define AI_SNDFLG_SETRETREATING	(1<<3)
#define AI_SNDFLG_SETPANICKED	(1<<4)
#define AI_SNDFLG_SETPSYCHO		(1<<5)
#define AI_SNDFLG_SPOT			(1<<6)
#define AI_SNDFLG_DEATH			(1<<7)
#define AI_SNDFLG_IDLE			(1<<8)
#define AI_SNDFLG_DEFENSIVE		(1<<9)
#define AI_SNDFLG_AGGRESSIVE	(1<<10)
#define AI_SNDFLG_RETREATING	(1<<11)
#define AI_SNDFLG_GUARDING		(1<<12)
#define AI_SNDFLG_PANICKED		(1<<13)
#define AI_SNDFLG_PSYCHO		(1<<14)
#define AI_SNDFLG_FOLLOWLOST	(1<<15)
#define AI_SNDFLG_LOSTTARGET	(1<<16)
#define AI_SNDFLG_BUMPED		(1<<17)

#define AI_SNDFLG_ALLSOUNDS		((1<<18)-1)

// Available State flags...

#define AI_SFLG_IDLE			(1<<0)
#define AI_SFLG_DEFENSIVE		(1<<1)
#define AI_SFLG_AGGRESSIVE		(1<<2)
#define AI_SFLG_RETREATING		(1<<3)
#define AI_SFLG_GUARDING		(1<<4)
#define AI_SFLG_PANICKED		(1<<5)
#define AI_SFLG_PSYCHO			(1<<6) 

#define AI_SFLG_ALLSTATES		((1<<7)-1)


#define AI_SCRFLG_LOOP					(1<<0)
#define AI_SCRFLG_INT					(1<<1)
#define AI_SCRFLG_PUSHABLE				(1<<2)
#define AI_SCRFLG_OPPORTFIRE			(1<<3)
#define AI_SCRFLG_NORAGDOLL				(1<<4)
#define AI_SCRFLG_ATTACKTILLDEAD		(1<<5)


class CAIPathList;

class BaseAI : public CBaseCharacter
{
	public :

		BaseAI();
		~BaseAI();

		virtual void	HandleWeaponChange();

		DBOOL	CheckAlignment(CharacterAlignment ca, HOBJECT hObj);

		enum AIState 
		{
			IDLE=0, DEFENSIVE, AGGRESSIVE, RETREATING, GUARDING, PANICKED,
			PSYCHO, DYING, SCRIPT 
		};

		enum AICondition 
		{ 
			HEALTHY=0, COCKY, WOUNDED, DEAD 
		};

		enum AIDirection 
		{ 
			LEFT=0, RIGHT, NONE 
		};

		enum AIBravado
		{
			BRAVADO1=0, BRAVADO2, BRAVADO3, BRAVADO4, BRAVADO5
		};

		enum AIExperience
		{
			EXPERIENCE1=0, EXPERIENCE2, EXPERIENCE3, EXPERIENCE4, EXPERIENCE5
		};

		enum AIMarksmanship
		{
			MARKSMANSHIP1=0, MARKSMANSHIP2, MARKSMANSHIP3, MARKSMANSHIP4, 
			MARKSMANSHIP5, MARKSMANSHIP6
		};

		enum AIEvasive
		{
			EVASIVE1=0, EVASIVE2, EVASIVE3, EVASIVE4, EVASIVE5, NON_EVASIVE
		};

		enum AIRange
		{ 
			SHORT=0, MID, LONG 
		};

		enum AIScriptMovement
		{
			SM_WALK=0, SM_RUN, SM_FLY
		};


		DBOOL IsFiring();


	protected : // Member functions

		friend class CCharacterMgr;

		enum AIScriptState
		{
			DONE, SET_MOVEMENT, FOLLOW_PATH, PLAY_SOUND, SET_STATE, TARGET, WAIT, 
			PLAY_ANIMATION, MOVE_TO 
		};

		// Engine functions...

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		DBOOL   ReadProp(ObjectCreateStruct *pInfo);
		void	HandleTouch(HOBJECT hObj);
		void	HandleModelString(ArgList* pArgList);
		void	InitialUpdate();
		void	InitializeWeapons();
		void	SetAnimationIndexes();

		virtual void PostPropRead(ObjectCreateStruct *pStruct);
		virtual DBOOL ProcessCommand(char** pTokens, int nArgs, char* pNextCommand);


		// Utility functions...

		DVector	GetEyeLevel();
		DVector	GetKneeLevel();
		DBOOL	CheckForCollision(DVector vDir, DFLOAT fDist);
		DBOOL	CheckForCollisionForward(DFLOAT fDist);
		DBOOL	CheckForCollisionBackward(DFLOAT fDist);
		DBOOL	CheckForCollisionRight(DFLOAT fDist);
		DBOOL	CheckForCollisionLeft(DFLOAT fDist);
		DFLOAT  DistanceToObject(HOBJECT hObj);
		DBOOL	IsObjectVisible(HOBJECT hObj);
		DBOOL	IsObjectVisibleToAI(HOBJECT hObj);
		DBOOL	IsPosVisible(DVector* pvTheirPos);
		DBOOL	IsPosVisibleToAI(DVector* pvTheirPos);
		HOBJECT	CanAIHearWeaponFire(CBaseCharacter* pChar);
		DBOOL	CanAIHearObject(HOBJECT hObj);
		void	SetTarget(char* pTargetName);
		void	SetNewTarget(HOBJECT hNewTarget);
		void	SetNewDamager(HOBJECT hNewDamager);
		void	SetNewLeader(HOBJECT hNewLeader);
		void	PlayAnimation(char* pAniName);
		void	ApproachTarget();
		void	SearchForPlayer(HOBJECT hPlayerObj);

		void	HandleBumped();

		void	SpotPlayer(HOBJECT hObj);
		void	HandleLostPlayer();

		void	FaceObject(HOBJECT hObj);
		void	FacePos(DVector vTargetPos);
		void	TurnRorL();
		void	TurnLeft();
		void	TurnRight();
		void	TurnAround() { TurnLeft(); TurnLeft(); }
		void	WalkForward();
		void	WalkBackward();
		void	RunForward();
		void	RunBackward();
		void	StrafeLeft();
		void	StrafeRight();
		DBOOL	CanMoveDir(DVector & vDir, DFLOAT fCollisionDist = -1.0f);
		DBOOL	AvoidObstacle();
		void	TargetObject(HOBJECT hObj);

		virtual void TargetPos(DVector vTargetPos);
		virtual void UpdateOnGround();

		virtual DBOOL ChooseTarget();
		virtual DBOOL ChooseOpportunityTarget();
		virtual DBOOL ShootTarget();
		virtual	DBOOL InLineOfSight(HOBJECT hObj);
		virtual DBOOL TargetInRange(AIRange eType=LONG);
		virtual DVector GetFirePos(DVector* pvPos) { return HandHeldWeaponFirePos(); }
		virtual DVector GetTargetDir(DVector & vFirePos, DVector & vTargetPos);
		virtual DBOOL GetTargetPos(DVector & vTargetPos);

		virtual void HitByObject(HOBJECT hObj, DFLOAT fDamage);

		virtual void PlayDeathSound();

		void	ClearActionFlags()				{ m_dwAction = 0; }
		void	SetActionFlag(DDWORD dwFlg)		{ m_dwAction |= dwFlg; }
		void	ClearActionFlag(DDWORD dwFlg)	{ m_dwAction &= ~dwFlg; }
		DDWORD	GetActionFlags() const			{ return m_dwAction; }

		void	SetSoundFlag(DDWORD dwFlg)		{ m_dwAvailableSounds |= dwFlg; }
		void	ClearSoundFlag(DDWORD dwFlg)	{ m_dwAvailableSounds &= ~dwFlg; }

		void	SetStateFlag(DDWORD dwFlg)		{ m_dwAvailableStates |= dwFlg; }
		void	ClearStateFlag(DDWORD dwFlg)	{ m_dwAvailableStates &= ~dwFlg; }
		DBOOL	IsValidState(AIState eState);


		char*	GetStateString();
		char*	GetConditionString();
		void	PrintDebugInfo();

		virtual char*	GetDeathSound();
		virtual char*	GetDamageSound(DamageType eType);

		// State machine functions...

		virtual DBOOL Update();

		virtual void UpdateControlFlags();
		virtual void UpdateMovement();
		virtual void UpdateSounds();
		virtual void UpdateSenses();
		virtual void UpdateEvasiveAction();
		virtual void UpdateWeapon();
		virtual void UpdateTargetPos();

		virtual void SetState(char* pState);
		virtual void SetState(AIState eState);

		virtual void SetIdle();
		virtual void UpdateIdle();

		virtual void SetDefensive();
		virtual void UpdateDefensive();

		virtual void SetAggressive();
		virtual void UpdateAggressive();

		virtual void SetRetreating();
		virtual void UpdateRetreating();

		virtual void SetGuarding();
		virtual void UpdateGuarding();

		virtual void SetPanicked();
		virtual void UpdatePanicked();

		virtual void SetPsycho();
		virtual void UpdatePsycho();

		virtual void SetDying();
		virtual void UpdateDying();

		virtual void SetScript();
		virtual void UpdateScript();
		virtual void UpdateScriptCommand();

		virtual void RecomputeCondition();
		virtual void RecomputeState();
		virtual AIState FindClosestValidState(AIState eNewState, AIState ePrevRecurseState);

		// Script command states...

		virtual void SetFollowPathCmd();
		virtual void SetSetMovementCmd();
		virtual void SetSetAnimationStateCmd();
		virtual void SetPlaysoundCmd();
		virtual void SetSetStateCmd();
		virtual void SetTargetCmd();
		virtual void SetWaitCmd();
		virtual void SetPlayAnimationCmd();
		virtual void SetChangeWeaponCmd();
		virtual void SetMoveToObjectCmd();
		virtual void SetFollowObjectCmd();
		virtual void SetFollowTimeCmd();
		virtual void SetSpawnCmd();
		virtual void SetRemoveCmd();

		virtual void UpdateFollowPathCmd();
		virtual void UpdateWaitCmd();
		virtual void UpdatePlayAnimationCmd();
		virtual void UpdateMoveToObjectCmd();
		virtual void UpdateFollowObjectCmd();

		virtual DBOOL UpdateScriptMovement(DVector* pvTargetPos);

		virtual void BuildScript(char* pScriptBody);

		void	NewUpdateMovement();
		void	HandleGameRestore();


		// Difficulty related functions...

		virtual void	AdjustMarksmanshipPerturb();
		virtual void	AdjustEvasiveDelay();
		virtual	void	AdjustFireDelay();
		virtual	void	AdjustDamageAggregate();


	protected : // Member Variables

		HOBJECT		m_hTarget;				// Who am I currently targeting
		HOBJECT		m_hLastDamager;			// Who last shot me

		AIState			m_eState;			// State of the AI
		AICondition		m_eCondition;		// Condition of the AI
		AIDirection		m_eLastTurnDir;		// What direction did we turn last?
		AIBravado		m_eBravado;			// Our bravado
		AIExperience	m_eExperience;		// Our smarts
		AIMarksmanship	m_eMarksmanship;	// Our aiming skills
		AIEvasive		m_eEvasive;			// Our dodging skills

		DDWORD		m_dwAction;				// Current actions
		DDWORD		m_dwLastAction;			// Actions last frame
		DDWORD		m_dwAvailableStates;	// What states can we go to
		DFLOAT		m_fVisibleRange;		// How far can AI see
		DFLOAT		m_fHearingRange;		// How far can AI hear
	
		DFLOAT		m_fRecomputeTime;		// The earliest we can recompute our state
		DFLOAT		m_fNextBumpedTime;		// Next time we can get bumped

		DVector		m_vRight;				// Object's right vector
		DVector		m_vUp;					// Object's up vector
		DVector		m_vForward;				// Object's forward
		DVector		m_vLastPos;				// Object's last position
		DBOOL		m_bStuckOnSomething;	// Are we stuck on a wall/ledge?

		DFLOAT		m_fNextSoundTime;		// When can I play a sound
		DFLOAT		m_fFollowStartTime;		// When did I start following this object
		DFLOAT		m_fFollowTime;			// How long should I follow this object

		DFLOAT		m_fFireStartTime;		// When should I start firing
		DFLOAT		m_fFireStopTime;		// When should I stop firing

		DBOOL		m_bRecompute;			// Should I reevaluate my state

		DBOOL		m_bSpottedPlayer;		// Have we spotted the player yet?
		DBOOL		m_bLostPlayer;			// Did we lose the player
		DBYTE		m_nWeaponId;			// ID of weapon

		HSTRING	m_hstrSpotTriggerTarget;	// Object to send spot trigger msg to
		HSTRING	m_hstrSpotTriggerMessage;	// Message to send to spot trigger obj
		DDWORD	m_nSpotTriggerNumSends;		// Number of times to send the message

		HSTRING	m_hstrLostTargetTriggerTarget;	// Object to send lost target trigger msg to
		HSTRING	m_hstrLostTargetTriggerMessage;	// Message to send to the lost target trigger obj
		DDWORD	m_nLostTargetTriggerNumSends;	// Number of times to send the message

		HSTRING	m_hstrBumpedTriggerTarget;	// Object to send bumped trigger msg to
		HSTRING	m_hstrBumpedTriggerMessage;	// Message to send to the bumped trigger obj
		DDWORD	m_nBumpedTriggerNumSends;	// Number of times to send the message

		// Sound stuff

		HSTRING	m_hstrSetIdleSound;			// Sound to play when going idle
		HSTRING	m_hstrSetDefensiveSound;	// Sound to play when going defensive
		HSTRING	m_hstrSetAggressiveSound;	// Sound to play when going aggressive
		HSTRING	m_hstrSetRetreatingSound;	// Sound to play when going retreating
		HSTRING	m_hstrSetPanickedSound;		// Sound to play when going panicked
		HSTRING	m_hstrSetPsychoSound;		// Sound to play when going psycho

		HSTRING	m_hstrSpotSound;			// Sound to play when target spotted
		HSTRING	m_hstrLostTargetSound;		// Sound to play when target is lost
		HSTRING	m_hstrDeathSound;			// Sound to play if we die
		HSTRING	m_hstrIdleSound;			// Sound to play when idle
		HSTRING	m_hstrDefensiveSound;		// Sound to play when defensive
		HSTRING	m_hstrAggressiveSound;		// Sound to play when aggressive
		HSTRING	m_hstrRetreatingSound;		// Sound to play when retreating
		HSTRING	m_hstrGuardingSound;		// Sound to play when guarding
		HSTRING	m_hstrPanickedSound;		// Sound to play when panicked
		HSTRING	m_hstrPsychoSound;			// Sound to play when psycho
		HSTRING	m_hstrFollowLostSound;		// Sound to play when I lost my leader
		HSTRING	m_hstrBumpedSound;			// Sound to play when I'm bumped

		DDWORD	m_dwAvailableSounds;		// What sounds can we play

		DBOOL	m_bOkAdjustVel;				// Adjust velocity based on the model size

		// Script stuff

		DBOOL			m_bUpdateScriptCmd;	// Time to get next script command
		AISCRIPTCMD		m_curScriptCmd;		// The current script command
		CAIScriptList	m_scriptCmdList;	// List of all script commands
		DBOOL			m_bScriptInterruptable;  // Can script be interrupted?

		CAIPathList		 m_AIPathList;		// Current list of path points
		AIScriptMovement m_eScriptMovement;	// Current mode of script movement
		DFLOAT			 m_fScriptWaitEnd;	// Time when script wait should end
		DVector			 m_vMoveToPos;		// Position to move to for the MoveToObject command

		HOBJECT			 m_hLeader;			// Object I should follow
		DBOOL			 m_bLostLeader;		// Did we lose the leader?

		AIScriptState	m_eScriptState;		// State of currently running script
		int				m_nScriptCmdIndex;	// Index into m_scriptCmdList
		DDWORD			m_dwScriptFlags;	// Flags used in scripting state
		AIState			m_eOldState;		// Old state (for scripting)
		DBOOL			m_bLoopScriptedAni; // Loop scripted animations?

		DFLOAT			m_fPredTravelDist;	// Predicted distance we would travel
		DFLOAT			m_fLastDistTraveled;// How far did we travel last frame


		DFLOAT			m_fStartEvasiveTime;
		DFLOAT			m_fStopEvasiveTime;
		DDWORD			m_dwEvasiveAction;
		DDWORD			m_dwLastEvasiveAction;

		DFLOAT			m_fFireRestAdjust;
		DBYTE			m_nNumRecoilFireTrys;	// Number of times we tried to
												// fire while recoiling
		DBYTE			m_nMaxCheeseCount;		// Max number of times we'll not
												// fire when recoiling before we fire anyway

		DVector			m_vTargetPos;			// "Current" position of target.
		DVector			m_vTargetLastPos;		// Last seen position of target.

		DFLOAT			m_fNextTargetTime;		// When should we calculate m_vTargetPos
		DFLOAT			m_fTargetCalcDelta;		// Time between calculations of m_vTargetPos

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...


		DFLOAT		m_fCurTime;					// The current game time

		// Debug stuff

		int			m_nDebugRecomputeValue;		// Value computed
		DFLOAT		m_fLastTouchForce;			// Last touch notify force

		DDWORD		m_dwLastLoadFlags;			// Game restore flags

		DVector		m_vPos;						// Current pos this frame...
		DBOOL		m_bSearchingForPlayer;		// Are we searching for the player

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();
};


#endif // __BASE_AI_H__