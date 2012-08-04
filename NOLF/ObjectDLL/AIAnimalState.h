// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_ANIMAL_STATE_H__
#define __AI_ANIMAL_STATE_H__

#include "AIState.h"
#include "AnimatorAIAnimal.h"

// Forward declarations

class CAI;

class CAIAnimal;

class AI_Dog;
class CAIDogStrategyFollowPath;
class CAIDogStrategyOneShotAni;

class AI_Poodle;
class CAIPoodleStrategyFollowPath;

class AI_Shark;
class CAISharkStrategyFollowPath;
class CAISharkStrategyOneShotAni;

// Classes

class CAIAnimalState : public CAIState
{
	typedef CAIState super;

	DEFINE_ABSTRACT_FACTORY_METHODS(CAIAnimalState);

	public :

		// Ctors/Dtors/etc

        LTBOOL Init(CAIAnimal* pAIAnimal);

	protected : // Protected methods

		// Simple accessors

		CAIAnimal* GetAI() { return m_pAIAnimal; }

	protected : // Private member variables

		// These do not need to be saved

		CAIAnimal*			m_pAIAnimal;
};

class CAIDogState : public CAIAnimalState
{
	typedef CAIAnimalState super;

	DEFINE_ABSTRACT_FACTORY_METHODS(CAIAIDogState);

	public : // Public member variables

		typedef enum AIDogStateType
		{
			eStateIdle,
			eStateBark,
			eStateExcited,
			eStateHeat,
		};

	public :

		// Ctors/Dtors/etc

        virtual LTBOOL Init(AI_Dog* pAIDog);

		// Updates

		virtual void UpdateSenses();

		// Handlers

		virtual void HandleBrokenLink(HOBJECT hObject);

		// Simple acccessors

		virtual AIDogStateType GetType() = 0;

	protected : // Protected methods

		// Simple accessors

		AI_Dog* GetAI() { return m_pAIDog; }

	protected : // Private member variables

		// These do not need to be saved

		AI_Dog*						m_pAIDog;
		CAIDogStrategyFollowPath*	m_pStrategyFollowPath;
		CAIDogStrategyOneShotAni*	m_pStrategyOneShotAni;
};

class CAIDogStateIdle : DEFINE_FACTORY_CLASS(CAIDogStateIdle), public CAIDogState
{
	typedef CAIDogState super;

	DEFINE_FACTORY_METHODS(CAIDogStateIdle);

	public :

		// Ctor/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEREAD hWrite);

		// Updates

		void Update();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIDogStateType GetType() { return CAIDogState::eStateIdle; }

	protected :

		CAnimatorAIAnimal::Main	m_ePose;
};

class CAIDogStateBark : DEFINE_FACTORY_CLASS(CAIDogStateBark), public CAIDogState
{
	typedef CAIDogState super;

	DEFINE_FACTORY_METHODS(CAIDogStateBark);

	public :

		// Ctor/Dtors/etc

        LTBOOL Init(AI_Dog* pAIDog);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEREAD hWrite);

		// Updates

		void Update();
		void UpdateSenses() { }

		// Simple acccessors

		AIDogStateType GetType() { return CAIDogState::eStateBark; }
		HMODELANIM GetDeathAni(LTBOOL bFront);

	protected :

		enum State
		{
			eMove,
			eJump,
			eBark,
			eUnjump,
		};

	protected :

		State	m_eState;
        uint32  m_dwNode;
		LTFLOAT	m_fPause;
};

class CAIDogStateExcited : DEFINE_FACTORY_CLASS(CAIDogStateExcited), public CAIDogState
{
	typedef CAIDogState super;

	DEFINE_FACTORY_METHODS(CAIDogStateExcited);

	public :

		// Updates

		void Update();
		void UpdateSenses() { }

		// Simple acccessors

		AIDogStateType GetType() { return CAIDogState::eStateExcited; }

	protected :
};

class CAIDogStateHeat : DEFINE_FACTORY_CLASS(CAIDogStateHeat), public CAIDogState
{
	typedef CAIDogState super;

	DEFINE_FACTORY_METHODS(CAIDogStateHeat);

	public :

		// Ctors/Dtors/etc

        LTBOOL Init(AI_Dog* pAIDog);

		// Updates

		void Update();
		void UpdateSenses() { }

		// Simple acccessors

		AIDogStateType GetType() { return CAIDogState::eStateHeat; }

	protected :
};

class CAIPoodleState : public CAIAnimalState
{
	typedef CAIAnimalState super;

	DEFINE_ABSTRACT_FACTORY_METHODS(CAIAIPoodleState);

	public : // Public member variables

		typedef enum AIPoodleStateType
		{
			eStateStartup,
			eStateShutdown,
			eStateSeduce,
			eStateDischarge,
		};

	public :

		// Ctors/Dtors/etc

        virtual LTBOOL Init(AI_Poodle* pAIPoodle);

		// Updates

		virtual void UpdateSenses();

		// Handlers

		virtual void HandleBrokenLink(HOBJECT hObject);

		// Simple acccessors

		virtual AIPoodleStateType GetType() = 0;

	protected : // Protected methods

		// Simple accessors

		AI_Poodle* GetAI() { return m_pAIPoodle; }

	protected : // Private member variables

		// These do not need to be saved

		AI_Poodle*						m_pAIPoodle;
		CAIPoodleStrategyFollowPath*	m_pStrategyFollowPath;
};

class CAIPoodleStateSeduce : DEFINE_FACTORY_CLASS(CAIPoodleStateSeduce), public CAIPoodleState
{
	typedef CAIPoodleState super;

	DEFINE_FACTORY_METHODS(CAIPoodleStateSeduce);

	public :

		// Ctors/Dtors/Etc

        LTBOOL Init(AI_Poodle* pAIPoodle);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		void Update();
		void UpdateSenses() { }

		// Simple acccessors

		AIPoodleStateType GetType() { return CAIPoodleState::eStateSeduce; }

	protected :

		DWORD	m_dwNode;
};

class CAIPoodleStateDischarge : DEFINE_FACTORY_CLASS(CAIPoodleStateDischarge), public CAIPoodleState
{
	typedef CAIPoodleState super;

	DEFINE_FACTORY_METHODS(CAIPoodleStateDischarge);

	public :

		// Ctors/Dtors/Etc

        LTBOOL Init(AI_Poodle* pAIPoodle);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		void Update();
		void UpdateSenses() { }

		// Simple acccessors

		AIPoodleStateType GetType() { return CAIPoodleState::eStateDischarge; }

	protected :

        LTBOOL       m_bDischarged;
};

class CAIPoodleStateShutdown : DEFINE_FACTORY_CLASS(CAIPoodleStateShutdown), public CAIPoodleState
{
	typedef CAIPoodleState super;

	DEFINE_FACTORY_METHODS(CAIPoodleStateShutdown);

	public :

		// Updates

		virtual void Update();

		// Simple acccessors

		virtual AIPoodleStateType GetType() { return CAIPoodleState::eStateShutdown; }

	protected :
};

class CAISharkState : public CAIAnimalState
{
	typedef CAIAnimalState super;

	DEFINE_ABSTRACT_FACTORY_METHODS(CAIAISharkState);

	public : // Public member variables

		typedef enum AISharkStateType
		{
			eStateIdle,
			eStateChase,
			eStateGoto,
			eStateBite,
			eStateMaul,
			eStateWait,
		};

	public :

		// Ctors/Dtors/etc

        virtual LTBOOL Init(AI_Shark* pAIShark);

		// Updates

		void UpdateSenses() { }

		// Handlers

		virtual void HandleBrokenLink(HOBJECT hObject);

		// Simple acccessors

		virtual AISharkStateType GetType() = 0;

	protected : // Protected methods

		// Simple accessors

		AI_Shark* GetAI() { return m_pAIShark; }

	protected : // Private member variables

		// These do not need to be saved

		AI_Shark*					m_pAIShark;
		CAISharkStrategyFollowPath*	m_pStrategyFollowPath;
		CAISharkStrategyOneShotAni*	m_pStrategyOneShotAni;
};

class CAISharkStateIdle : DEFINE_FACTORY_CLASS(CAISharkStateIdle), public CAISharkState
{
	typedef CAISharkState super;

	DEFINE_FACTORY_METHODS(CAISharkStateIdle);

	public :

		// Ctor/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEREAD hWrite);

		// Updates

		void Update();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AISharkStateType GetType() { return CAISharkState::eStateIdle; }

	protected :

		CAnimatorAIAnimal::Main	m_ePose;
};

class CAISharkStateChase : DEFINE_FACTORY_CLASS(CAISharkStateChase), public CAISharkState
{
	typedef CAISharkState super;

	DEFINE_FACTORY_METHODS(CAISharkStateChase);

	public :

		// Ctors/Dtors/Etc

        LTBOOL Init(AI_Shark* pAIShark);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		void Update();

		// Handlers

		void HandleTouch(HOBJECT hObject);

		// Simple acccessors

		AISharkStateType GetType() { return CAISharkState::eStateChase; }
};

class CAISharkStateGoto : DEFINE_FACTORY_CLASS(CAISharkStateGoto), public CAISharkState
{
	typedef CAISharkState super;

	DEFINE_FACTORY_METHODS(CAISharkStateGoto);

	public : // Public constants

		enum Constants
		{
			kMaxGotoNodes = 12,
		};

	public : // Public methods

		// Ctors/Dtors/Etc

        LTBOOL Init(AI_Shark* pAIShark);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		void Update();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AISharkStateType GetType() { return CAISharkState::eStateGoto; }

	protected : // Protected member variables

		LTVector	m_vDest;
		uint32		m_adwNodes[kMaxGotoNodes];
		int			m_cNodes;
		int			m_iNextNode;
		LTBOOL		m_bLoop;
};

class CAISharkStateBite : DEFINE_FACTORY_CLASS(CAISharkStateBite), public CAISharkState
{
	typedef CAISharkState super;

	DEFINE_FACTORY_METHODS(CAISharkStateBite);

	public :

		// Ctors/Dtors/Etc

        LTBOOL Init(AI_Shark* pAIShark);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		void Update();

		// Simple acccessors

		AISharkStateType GetType() { return CAISharkState::eStateBite; }

	protected :

        LTBOOL       m_bBiting;
};

class CAISharkStateMaul : DEFINE_FACTORY_CLASS(CAISharkStateMaul), public CAISharkState
{
	typedef CAISharkState super;

	DEFINE_FACTORY_METHODS(CAISharkStateMaul);

	public :

		// Ctors/Dtors/Etc

        LTBOOL Init(AI_Shark* pAIShark);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		void Update();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);
		void HandleBrokenLink(HOBJECT hObject);

		// Simple acccessors

		AISharkStateType GetType() { return CAISharkState::eStateMaul; }

	protected :

		HOBJECT		m_hTarget;
        LTBOOL      m_bBiting;
		LTBOOL		m_bDone;
		LTFLOAT		m_fMaulTime;
};

class CAISharkStateWait : DEFINE_FACTORY_CLASS(CAISharkStateWait), public CAISharkState
{
	typedef CAISharkState super;

	DEFINE_FACTORY_METHODS(CAISharkStateWait);

	public :

		// Ctors/Dtors/Etc

        LTBOOL Init(AI_Shark* pAIShark);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		void Update();

		// Simple acccessors

		AISharkStateType GetType() { return CAISharkState::eStateWait; }

	protected :

		uint32	m_dwNode;
};

#endif