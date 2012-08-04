// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_VEHICLE_STATE_H__
#define __AI_VEHICLE_STATE_H__

#include "AIState.h"
#include "AnimatorAIVehicle.h"

// Forward declarations

class CAI;

class CAIVehicle;

class AI_Helicopter;
class CAIHelicopterStrategyFollowPath;
class CAIHelicopterStrategyShoot;

// Classes

class CAIVehicleState : public CAIState
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIVehicleState);

	public :

		// Ctors/Dtors/etc

        LTBOOL Init(CAIVehicle* pAIVehicle);

	protected : // Protected methods

		// Simple accessors

		CAIVehicle* GetAI() { return m_pAIVehicle; }

	protected : // Private member variables

		// These do not need to be saved

		CAIVehicle*			m_pAIVehicle;
};

class CAIHelicopterState : public CAIVehicleState
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIAIHelicopterState);

	public : // Public member variables

		typedef enum AIHelicopterStateType
		{
			eStateIdle,
			eStateGoto,
			eStateAttack,
			eStateChase,
		};

	public :

		// Ctors/Dtors/etc

        virtual LTBOOL Init(AI_Helicopter* pAIHelicopter);

		// Updates

		void UpdateSenses() { }

		// Handlers

		virtual void HandleBrokenLink(HOBJECT hObject);

		// Simple acccessors

		virtual AIHelicopterStateType GetType() = 0;

	protected : // Protected methods

		// Simple accessors

		AI_Helicopter* GetAI() { return m_pAIHelicopter; }

	protected : // Private member variables

		// These do not need to be saved

		AI_Helicopter*						m_pAIHelicopter;
		CAIHelicopterStrategyFollowPath*	m_pStrategyFollowPath;
		CAIHelicopterStrategyShoot*			m_pStrategyShoot;
};

class CAIHelicopterStateIdle : DEFINE_FACTORY_CLASS(CAIHelicopterStateIdle), public CAIHelicopterState
{
	DEFINE_FACTORY_METHODS(CAIHelicopterStateIdle);

	public :
	
		// Simple acccessors

		AIHelicopterStateType GetType() { return CAIHelicopterState::eStateIdle; }

	protected :

};

class CAIHelicopterStateGoto : DEFINE_FACTORY_CLASS(CAIHelicopterStateGoto), public CAIHelicopterState
{
	DEFINE_FACTORY_METHODS(CAIHelicopterStateGoto);

	public : // Public methods

		// Ctors/Dtors/Etc

        LTBOOL Init(AI_Helicopter* pAIHelicopter);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		void Update();

		// Handlers

		void HandleNameValuePair(char *szName, char *szValue);

		// Simple acccessors

		AIHelicopterStateType GetType() { return CAIHelicopterState::eStateGoto; }

	protected  :

		enum Constants
		{
			kMaxGotoNodes = 12,
		};

	protected : // Protected member variables

        LTVector	m_vDest;
        uint32		m_adwNodes[kMaxGotoNodes];
		int			m_cNodes;
		int			m_iNextNode;
        LTBOOL		m_bLoop;
};

class CAIHelicopterStateAttack : DEFINE_FACTORY_CLASS(CAIHelicopterStateAttack), public CAIHelicopterState
{
	DEFINE_FACTORY_METHODS(CAIHelicopterStateAttack);

	public :

		// Ctor/Dtors/etc

        LTBOOL Init(AI_Helicopter* pAIHelicopter);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEREAD hWrite);

		// Updates

		void Update();

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);
        LTBOOL HandleCommand(char** pTokens, int nArgs);
		void HandleNameValuePair(char *szName, char *szValue);
		void HandleModelString(ArgList* pArgList);

		// Simple acccessors

		AIHelicopterStateType GetType() { return CAIHelicopterState::eStateAttack; }

	protected :

        LTBOOL   m_abActiveWeapons[AI_MAX_WEAPONS];
		HOBJECT	m_hTarget;
};

#endif