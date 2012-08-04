// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_VEHICLE_STRATEGY_H__
#define __AI_VEHICLE_STRATEGY_H__

#include "AIMovement.h"
#include "AnimatorAIVehicle.h"
#include "AI.h"

class CAIVehicle;
class AI_Helicopter;
class CAINode;
class CAIVolume;
class CAIPath;
class CAIPathWaypoint;
class CWeapon;

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIVehicleStrategy
//
// PURPOSE : AI Strategy abstract class
//
// ----------------------------------------------------------------------- //

class CAIVehicleStrategy
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIVehicleStrategy);

	public : // Public methods

		// Ctors/Dtors/etc

        virtual LTBOOL Init(CAIVehicle* pAIVehicle) { m_pAIVehicle = pAIVehicle; return LTTRUE; }

	protected : // Protected methods

		// Simple accessors

		CAIVehicle*	GetAI() { return m_pAIVehicle; }
		CAnimatorAIVehicle* GetAnimator();

	private : // Private member variables

		CAIVehicle*			m_pAIVehicle;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHelicopterStrategy
//
// PURPOSE : AI Strategy abstract class
//
// ----------------------------------------------------------------------- //

class CAIHelicopterStrategy : public CAIVehicleStrategy
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIHelicopterStrategy);

	public : // Public member variables

		typedef enum AIHelicopterStrategyType
		{
			eStrategyNone,
			eStrategyFollowPath,
			eStrategyShoot,
		};

	public : // Public methods

		// Ctors/Dtors/etc

		virtual LTBOOL Init(AI_Helicopter* pAIHelicopter);

		virtual void Load(HMESSAGEREAD hRead) {}
		virtual void Save(HMESSAGEWRITE hWrite) {}

		// Updates

        virtual LTBOOL Update() { return LTTRUE; }

		// Simple accessors

		virtual AIHelicopterStrategyType GetType() { return eStrategyNone; }

	protected : // Protected methods

		// Simple accessors

		AI_Helicopter*	GetAI() { return m_pAIHelicopter; }

	private : // Private member variables

		AI_Helicopter*	m_pAIHelicopter;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHelicopterStrategyFollowPath
//
// PURPOSE : AI Follow path ability - to walk a path of AINodes
//
// ----------------------------------------------------------------------- //

class CAIHelicopterStrategyFollowPath : DEFINE_FACTORY_CLASS(CAIHelicopterStrategyFollowPath), public CAIHelicopterStrategy
{
	DEFINE_FACTORY_METHODS(CAIHelicopterStrategyFollowPath);

	public : // Public methods

		// Ctors/Dtors/etc

		LTBOOL Init(AI_Helicopter* pAIHelicopter);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		LTBOOL Set(const LTVector& vDestination);
		LTBOOL Set(CAINode* pNodeDestination);
		LTBOOL Set(CAIVolume* pVolumeDestination);

		LTBOOL Update();

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);
		void HandleTouch(HOBJECT hObject);

		// Simple accessors

		AIHelicopterStrategyType GetType() { return CAIHelicopterStrategy::eStrategyFollowPath; }

		LTBOOL IsUnset() { return m_eState == eStateUnset; }
		LTBOOL IsSet() { return m_eState == eStateSet; }
		LTBOOL IsDone() { return m_eState == eStateDone; }

	private :

		LTBOOL UpdateMoveTo(CAIPathWaypoint* pWaypoint);

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
		CAIMovementHelicopter		m_AIMovement;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIHelicopterStrategyShoot
//
// PURPOSE : AI shoot ability - to fire at something
//
// ----------------------------------------------------------------------- //

class CAIHelicopterStrategyShoot : DEFINE_FACTORY_CLASS(CAIHelicopterStrategyShoot), public CAIHelicopterStrategy
{
	DEFINE_FACTORY_METHODS(CAIHelicopterStrategyShoot);

	protected : // Inner classes

		struct BURSTSTRUCT
		{
			LTBOOL		m_bFired;
			LTBOOL		m_bActive;
			int			m_nLastAmmoInClip;
			LTFLOAT		m_fBurstInterval;
			int			m_nBurstShots;

			BURSTSTRUCT()
			{
				m_bFired = LTFALSE;
				m_bActive = LTTRUE;
				m_nLastAmmoInClip = 0;
				m_fBurstInterval = 0;
				m_nBurstShots = 0;
			}

			void Save(HMESSAGEWRITE hWrite)
			{
				SAVE_BOOL(m_bFired);
				SAVE_BOOL(m_bActive);
				SAVE_INT(m_nLastAmmoInClip);
				SAVE_FLOAT(m_fBurstInterval);
				SAVE_INT(m_nBurstShots);
			}

			void Load(HMESSAGEREAD hRead)
			{
				LOAD_BOOL(m_bFired);
				LOAD_BOOL(m_bActive);
				LOAD_INT(m_nLastAmmoInClip);
				LOAD_FLOAT(m_fBurstInterval);
				LOAD_INT(m_nBurstShots);
			}
		};

	public : // Public methods

		// Ctors/Dtors/etc

		LTBOOL Init(AI_Helicopter* pAIHelicopter);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

		LTBOOL Update();

		// Target

		void SetTarget(HOBJECT hTarget);

		// Firing

		void FireWeapon(int iWeapon) { m_aBursts[iWeapon].m_bFired = LTTRUE; }
		void SetFire(CAnimatorAIVehicle::Fire eFire) { m_eFire = eFire; }

		// Weapons

		void ActivateWeapon(int iWeapon) { m_aBursts[iWeapon].m_bActive = LTTRUE; }
		void DeactivateWeapon(int iWeapon) { m_aBursts[iWeapon].m_bActive = LTFALSE; }

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);

		// Simple accessors

		AIHelicopterStrategyType GetType() { return CAIHelicopterStrategy::eStrategyShoot; }

	protected : // Protected methods

		// Updates

		void UpdateFiring(CWeapon* pWeapon, BURSTSTRUCT* pStruct);
		void UpdateAiming(CWeapon* pWeapon, BURSTSTRUCT* pStruct);

		// Burst methods

		void CalculateBurst(CWeapon* pWeapon, BURSTSTRUCT* pStruct);

		// Aim/Fire

		void Aim(CWeapon* pWeapon, BURSTSTRUCT* pStruct);
		void Fire(CWeapon* pWeapon, BURSTSTRUCT* pStruct);

	protected : // Private member variables

		HOBJECT						m_hTarget;					// Our target
		BURSTSTRUCT					m_aBursts[AI_MAX_WEAPONS];	// Burst info tracker
		CAnimatorAIVehicle::Fire	m_eFire;					// Our firing animation
};

#endif
