// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_ANIMAL_STRATEGY_H__
#define __AI_ANIMAL_STRATEGY_H__

#include "AIMovement.h"
#include "AnimatorAIAnimal.h"

class CAIAnimal;
class AI_Dog;
class AI_Shark;
class CAINode;
class CAIVolume;
class CAIPath;
class CAIPathWaypoint;

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIAnimalStrategy
//
// PURPOSE : AI Strategy abstract class
//
// ----------------------------------------------------------------------- //

class CAIAnimalStrategy
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIAnimalStrategy);

	public : // Public methods

		// Ctors/Dtors/etc

        virtual LTBOOL Init(CAIAnimal* pAIAnimal) { m_pAIAnimal = pAIAnimal; return LTTRUE; }

	protected : // Protected methods

		// Simple accessors

		CAIAnimal*	GetAI() { return m_pAIAnimal; }
		CAnimatorAIAnimal* GetAnimator();

	private : // Private member variables

		CAIAnimal*			m_pAIAnimal;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIDogStrategy
//
// PURPOSE : AI Strategy abstract class
//
// ----------------------------------------------------------------------- //

class CAIDogStrategy : public CAIAnimalStrategy
{
	typedef CAIAnimalStrategy super;

	DEFINE_ABSTRACT_FACTORY_METHODS(CAIDogStrategy);

	public : // Public member variables

		typedef enum AIDogStrategyType
		{
			eStrategyNone,
			eStrategyFollowPath,
			eStrategyOneShotAni,
		};

	public : // Public methods

		// Ctors/Dtors/etc

        virtual LTBOOL Init(AI_Dog* pAIDog);

		virtual void Load(HMESSAGEREAD hRead) {}
		virtual void Save(HMESSAGEWRITE hWrite) {}

		// Updates

        virtual LTBOOL Update() { return LTTRUE; }

		// Simple accessors

		virtual AIDogStrategyType GetType() { return eStrategyNone; }

	protected : // Protected methods

		// Simple accessors

		AI_Dog*	GetAI() { return m_pAIDog; }

	private : // Private member variables

		AI_Dog*			m_pAIDog;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIDogStrategyFollowPath
//
// PURPOSE : AI Follow path ability - to walk a path of AINodes
//
// ----------------------------------------------------------------------- //

class CAIDogStrategyFollowPath : DEFINE_FACTORY_CLASS(CAIDogStrategyFollowPath), public CAIDogStrategy
{
	typedef CAIDogStrategy super;

	DEFINE_FACTORY_METHODS(CAIDogStrategyFollowPath);

	public : // Public methods

		// Ctors/Dtors/etc

        LTBOOL Init(AI_Dog* pAIDog);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        LTBOOL Set(const LTVector& vDestination);
        LTBOOL Set(CAINode* pNodeDestination);
        LTBOOL Set(CAIVolume* pVolumeDestination);

        LTBOOL Update();

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);

		// Movement types

		void SetMovement(CAnimatorAIAnimal::Main eMovement);

		// Simple accessors

		AIDogStrategyType GetType() { return CAIDogStrategy::eStrategyFollowPath; }

        LTBOOL IsUnset() { return m_eState == eStateUnset; }
        LTBOOL IsSet() { return m_eState == eStateSet; }
        LTBOOL IsDone() { return m_eState == eStateDone; }

	private :

        LTBOOL UpdateMoveTo(CAIPathWaypoint* pWaypoint);
        LTBOOL UpdateOpenDoors(CAIPathWaypoint* pWaypoint);

	private : // Private enumerations

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

	private : // Protected member variables

		CAIPath*					m_pPath;
		State						m_eState;
		CAIMovementDog				m_AIMovement;
		CAnimatorAIAnimal::Main		m_eMovement;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIDogStrategyOneShotAni
//
// PURPOSE : AI CheckingPulse - ability to ... to play a one shot ani
//
// ----------------------------------------------------------------------- //

class CAIDogStrategyOneShotAni : DEFINE_FACTORY_CLASS(CAIDogStrategyOneShotAni), public CAIDogStrategy
{
	typedef CAIDogStrategy super;

	DEFINE_FACTORY_METHODS(CAIDogStrategyOneShotAni);

	public : // Public methods

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        LTBOOL Set(CAnimatorAIAnimal::Main eMain);

        LTBOOL Update();

		// Simple accessors

		AIDogStrategyType GetType() { return eStrategyOneShotAni; }
        LTBOOL IsAnimating() { return m_bAnimating; }

	protected : // Protected member variables

		CAnimatorAIAnimal::Main	m_eMain;
        LTBOOL                   m_bAnimating;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIPoodleStrategy
//
// PURPOSE : AI Strategy abstract class
//
// ----------------------------------------------------------------------- //

class CAIPoodleStrategy : public CAIAnimalStrategy
{
	typedef CAIAnimalStrategy super;

	DEFINE_ABSTRACT_FACTORY_METHODS(CAIPoodleStrategy);

	public : // Public member variables

		typedef enum AIPoodleStrategyType
		{
			eStrategyNone,
			eStrategyFollowPath,
		};

	public : // Public methods

		// Ctors/Dtors/etc

        virtual LTBOOL Init(AI_Poodle* pAIPoodle);

		virtual void Load(HMESSAGEREAD hRead) {}
		virtual void Save(HMESSAGEWRITE hWrite) {}

		// Updates

        virtual LTBOOL Update() { return LTTRUE; }

		// Simple accessors

		virtual AIPoodleStrategyType GetType() { return eStrategyNone; }

	protected : // Protected methods

		// Simple accessors

		AI_Poodle*	GetAI() { return m_pAIPoodle; }

	private : // Private member variables

		AI_Poodle*			m_pAIPoodle;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIPoodleStrategyFollowPath
//
// PURPOSE : AI Follow path ability - to walk a path of AINodes
//
// ----------------------------------------------------------------------- //

class CAIPoodleStrategyFollowPath : DEFINE_FACTORY_CLASS(CAIPoodleStrategyFollowPath), public CAIPoodleStrategy
{
	typedef CAIPoodleStrategy super;

	DEFINE_FACTORY_METHODS(CAIPoodleStrategyFollowPath);

	public : // Public methods

		// Ctors/Dtors/etc

        LTBOOL Init(AI_Poodle* pAIPoodle);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        LTBOOL Set(const LTVector& vDestination);
        LTBOOL Set(CAINode* pNodeDestination);
        LTBOOL Set(CAIVolume* pVolumeDestination);

        LTBOOL Update();

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);

		// Movement types

		void SetMovement(CAnimatorAIAnimal::Main eMovement);

		// Simple accessors

		AIPoodleStrategyType GetType() { return CAIPoodleStrategy::eStrategyFollowPath; }

        LTBOOL IsUnset() { return m_eState == eStateUnset; }
        LTBOOL IsSet() { return m_eState == eStateSet; }
        LTBOOL IsDone() { return m_eState == eStateDone; }

	private :

        LTBOOL UpdateMoveTo(CAIPathWaypoint* pWaypoint);
        LTBOOL UpdateOpenDoors(CAIPathWaypoint* pWaypoint);

	private : // Private enumerations

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

	private : // Protected member variables

		CAIPath*					m_pPath;
		State						m_eState;
		CAIMovementPoodle			m_AIMovement;
		CAnimatorAIAnimal::Main		m_eMovement;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAISharkStrategy
//
// PURPOSE : AI Strategy abstract class
//
// ----------------------------------------------------------------------- //

class CAISharkStrategy : public CAIAnimalStrategy
{
	typedef CAIAnimalStrategy super;

	DEFINE_ABSTRACT_FACTORY_METHODS(CAISharkStrategy);

	public : // Public member variables

		typedef enum AISharkStrategyType
		{
			eStrategyNone,
			eStrategyFollowPath,
			eStrategyOneShotAni,
		};

	public : // Public methods

		// Ctors/Dtors/etc

        virtual LTBOOL Init(AI_Shark* pAIShark);

		virtual void Load(HMESSAGEREAD hRead) {}
		virtual void Save(HMESSAGEWRITE hWrite) {}

		// Updates

        virtual LTBOOL Update() { return LTTRUE; }

		// Simple accessors

		virtual AISharkStrategyType GetType() { return eStrategyNone; }

	protected : // Protected methods

		// Simple accessors

		AI_Shark*	GetAI() { return m_pAIShark; }

	private : // Private member variables

		AI_Shark*			m_pAIShark;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAISharkStrategyFollowPath
//
// PURPOSE : AI Follow path ability - to walk a path of AINodes
//
// ----------------------------------------------------------------------- //

class CAISharkStrategyFollowPath : DEFINE_FACTORY_CLASS(CAISharkStrategyFollowPath), public CAISharkStrategy
{
	typedef CAISharkStrategy super;

	DEFINE_FACTORY_METHODS(CAISharkStrategyFollowPath);

	public : // Public methods

		// Ctors/Dtors/etc

        LTBOOL Init(AI_Shark* pAIShark);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        LTBOOL Set(const LTVector& vDestination);
        LTBOOL Set(CAINode* pNodeDestination);
        LTBOOL Set(CAIVolume* pVolumeDestination);
		void Stop();

		// Updates

        LTBOOL Update();

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);

		// Movement types

		void SetMovement(CAnimatorAIAnimal::Main eMovement);

		// Simple accessors

		AISharkStrategyType GetType() { return CAISharkStrategy::eStrategyFollowPath; }

        LTBOOL IsUnset() { return m_eState == eStateUnset; }
        LTBOOL IsSet() { return m_eState == eStateSet; }
        LTBOOL IsDone() { return m_eState == eStateDone; }

	private :

        LTBOOL UpdateMoveTo(CAIPathWaypoint* pWaypoint);
        LTBOOL UpdateOpenDoors(CAIPathWaypoint* pWaypoint);

	private : // Private enumerations

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

	private : // Protected member variables

		CAIPath*					m_pPath;
		State						m_eState;
		CAIMovementShark			m_AIMovement;
		CAnimatorAIAnimal::Main		m_eMovement;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAISharkStrategyOneShotAni
//
// PURPOSE : AI CheckingPulse - ability to ... to play a one shot ani
//
// ----------------------------------------------------------------------- //

class CAISharkStrategyOneShotAni : DEFINE_FACTORY_CLASS(CAISharkStrategyOneShotAni), public CAISharkStrategy
{
	typedef CAISharkStrategy super;

	DEFINE_FACTORY_METHODS(CAISharkStrategyOneShotAni);

	public : // Public methods

		// Ctors/Dtors/etc

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        LTBOOL Set(CAnimatorAIAnimal::Main eMain);

        LTBOOL Update();

		// Simple accessors

		AISharkStrategyType GetType() { return eStrategyOneShotAni; }
        LTBOOL IsAnimating() { return m_bAnimating; }

	protected : // Protected member variables

		CAnimatorAIAnimal::Main	m_eMain;
        LTBOOL                   m_bAnimating;
};

#endif
