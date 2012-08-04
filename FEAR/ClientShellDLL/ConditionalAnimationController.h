//-----------------------------------------------------------------------------
//
// MODULE  : ConditionalAnimationController.h
//
// PURPOSE : Used to select and apply player animations based on current
//				game conditions in response to game-driven events (stimuli).
//
// CREATED : 1/27/05
//
// (c) 2005 Monolith Productions, Inc.	All Rights Reserved
//
//-----------------------------------------------------------------------------

#ifndef __CONDITIONAL_ANIMATION_CONTROLLER_H__
#define __CONDITIONAL_ANIMATION_CONTROLLER_H__

//-----------------------------------------------------------------------------

#include "idatabasemgr.h"
#include "AnimationContext.h"

//-----------------------------------------------------------------------------
// Database data wrappers.

struct ConditionalQueueData
{
	HRECORD m_QueueLink;
	double m_fWindowStartTime;
	CParsedMsg::CToken m_WindowStartTiming;
	double m_fWindowEndTime;
	CParsedMsg::CToken m_WindowEndTiming;
	float m_fExitPointTime;
	CParsedMsg::CToken m_ExitPointTiming;
};
typedef std::vector<ConditionalQueueData> QueueLinks;

#define DEFAULT_CONTEXT (-1)

struct ConditionalSubAction
{
	EnumAnimProp m_ePart;
	uint32 m_kContext;
	float m_fDuration;
	float m_fRate;
	CParsedMsg::CToken m_Difficulty;
	QueueLinks m_Links;
	float m_fLength;
};
typedef std::vector<ConditionalSubAction> SubActions;

struct IndividualCondition
{
	bool m_bAny;
};

struct DirectionCondition : IndividualCondition
{
	DirectionCondition()
	{}
	DirectionCondition(const char* Direction)
	{
		m_Direction = Direction;
		static CParsedMsg::CToken s_cTok_AnyDirection("AnyDirection");
		m_bAny = (m_Direction == s_cTok_AnyDirection);
	}
	bool operator==(const CParsedMsg::CToken& Direction) const
	{
		return m_bAny || (m_Direction == Direction);
	}
	CParsedMsg::CToken m_Direction;
};

struct BodyStateCondition : IndividualCondition
{
	BodyStateCondition()
	{}
	BodyStateCondition(const char* BodyState)
	{
		m_BodyState = BodyState;
		static CParsedMsg::CToken s_cTok_AnyBodyState("AnyBodyState");
		m_bAny = (m_BodyState == s_cTok_AnyBodyState);
	}
	bool operator==(const CParsedMsg::CToken& BodyState) const
	{
		return m_bAny || (m_BodyState == BodyState);
	}
	CParsedMsg::CToken m_BodyState;
};

struct OwnerCondition : IndividualCondition
{
	OwnerCondition()
	{}
	OwnerCondition(const char* Owner)
	{
		m_Owner = Owner;
		static CParsedMsg::CToken s_cTok_AnyOwner("AnyOwner");
		m_bAny = (m_Owner == s_cTok_AnyOwner);
	}
	bool operator==(const CParsedMsg::CToken& Owner) const
	{
		return m_bAny || (m_Owner == Owner);
	}
	CParsedMsg::CToken m_Owner;
};

struct LookContextCondition : IndividualCondition
{
	LookContextCondition()
	{}
	LookContextCondition(const char* LookContext)
	{
		m_LookContext = LookContext;
		static CParsedMsg::CToken s_cTok_AnyLookContext("AnyLookContext");
		m_bAny = (m_LookContext == s_cTok_AnyLookContext);
	}
	bool operator==(const CParsedMsg::CToken& LookContext) const
	{
		return m_bAny || (m_LookContext == LookContext);
	}
	CParsedMsg::CToken m_LookContext;
};

struct RangeCondition : IndividualCondition
{
	RangeCondition()
	{}
	RangeCondition(const LTVector2& Range)
	{
		m_Range = Range;
		m_bAny = (m_Range.y == 0.0f);
	}
	bool Contains(float fValue)
	{
		return m_bAny || (m_Range.x <= fValue && fValue <= m_Range.y);
	}
	LTVector2 m_Range;
};

struct FOVCondition : IndividualCondition
{
	FOVCondition()
	{}
	FOVCondition(float Degrees)
	{
		m_fLimit = LTCos(DEG2RAD(Degrees) * 0.5f);	// half-angle from center
		m_bAny = (m_fLimit == 1.0f);
	}
	bool Contains(float fDot)
	{
		return m_bAny || (fDot >= m_fLimit);
	}
	float m_fLimit;
};

struct ConditionalCondition
{
	DirectionCondition m_Direction;
	HRECORD m_rQueueLink;
	FOVCondition m_EnemyCamFOV;
	FOVCondition m_EnemyPosFOV;
	RangeCondition m_EnemyDistance;
	BodyStateCondition m_EnemyState;
	FOVCondition m_ProjectileFOV;
	RangeCondition m_ProjectileDistance;
	OwnerCondition m_ProjectileOwner;
	LookContextCondition m_LookContext;
	float m_fWeight;
	HRECORD m_rGroup;
};
typedef std::vector<ConditionalCondition> Conditions;

struct ConditionalAction
{
	CParsedMsg::CToken m_Stimulus;
	EnumAnimProp m_eAction;
	Conditions m_Conditions;
	SubActions m_SubActions;
	std::string m_sEndStimulus;
};
typedef std::vector<ConditionalAction> Actions;

//-----------------------------------------------------------------------------
// An action selected via the associated condition.

struct ActionCondition
{
	ConditionalAction* m_pAction;
	ConditionalCondition* m_pCondition;

	ActionCondition(ConditionalAction* pAction, ConditionalCondition* pCondition)
	: m_pAction(pAction)
	, m_pCondition(pCondition)
	{}
};

typedef std::vector<ActionCondition> ActionConditions;

//-----------------------------------------------------------------------------

class ConditionalAnimationController
{
public:

	ConditionalAnimationController(){}
	ConditionalAnimationController(HRECORD hController)
		{ Init(hController); }
	~ConditionalAnimationController(){}

	void Update();
	void Reset() { ResetCurrentAction(); }

	bool HandleStimulus(const char* pszStimulus);
	bool HandlingStimulus(const char* pszStimulus) const;
	bool HandlingStimulusGroup(const char* pszStimulus) const;
	bool ActiveStimulus() const;

private:

	void Init(HRECORD hController);

	void StartNewAction(ConditionalAction* pNewAction);
	bool PlayNextSubAction(double fResidue=0.0);
	void ActionFinished();
	void ResetCurrentAction();

	double GetCurrentTime() const;
	CAnimationContext* GetAnimContext() const;
	void UpdatePlayerAnimProps() const;
	void SetAnimRate(float fRate) const;
	void SetAnimLength(float fSeconds) const;
	void SetAnimStartingTime(double fSeconds) const;
	void SetAnimLooping(bool bLoop) const;
	void ClearCachedAni() const;

	const char* GetMovementStr() const;
	const char* GetBodyStateStr(BodyState eBodyState) const;
	const char* GetOwnerStateStr(HOBJECT hShooter) const;
	const char* GetDifficultyStr() const;
	static uint32 GetAnimContext(const char* pszContext);
	double ConvertTiming(double fTime, const char* pszTiming) const;

	HRECORD m_hController;
	Actions m_Actions;

	//NOTE: all state data moved into PlayerBody.
};

struct ConditionalAnimState
{
	ConditionalAnimState()
		: m_pCurrentAction(NULL)
	{}

	ConditionalAction* m_pCurrentAction;
	ConditionalSubAction* m_pCurrentSubAction;
	ConditionalAction* m_pPendingAction;

	uint32 m_nCurrentSubActionPart;	// sub actions are played in enum order.

	EnumAnimProp m_eAction;
	EnumAnimProp m_eSubAction;

	uint32 m_kCurrentContext;

	float m_PendingActionStartTime;
	double m_fSubActionStartTime;
	double m_fSubActionEndTime;
};

#endif //__CONDITIONAL_ANIMATION_CONTROLLER_H__
