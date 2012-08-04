// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_STEERING_H__
#define __AI_STEERING_H__

#include "ServerUtilities.h"
#include "TemplateList.h"
#include "AIUtils.h"

// ----------------------------------------------------------------------- //
//
// CLASS   : CSteerable
//
// PURPOSE : Steerable abstract class
//
// ----------------------------------------------------------------------- //

class CSteerable
{
	public : // Public methods

		CSteerable();

		// Simple accessors

        LTFLOAT Steerable_GetMass() { return Steerable_m_fMass; }
        LTFLOAT Steerable_GetMaxForce() { return Steerable_m_fMaxForce; }
        LTFLOAT Steerable_GetMaxSpeed() { return Steerable_m_fMaxSpeed; }
        const LTVector& Steerable_GetUpVector() { return Steerable_m_vUp; }
        const LTVector& Steerable_GetForwardVector() { return Steerable_m_vForward; }
        const LTVector& Steerable_GetRightVector() { return Steerable_m_vRight; }
        const LTVector& Steerable_GetPosition() { return Steerable_m_vPosition; }
        const LTVector& Steerable_GetVelocity() { return Steerable_m_vVelocity; }
        const LTVector& Steerable_GetLastForce() { return Steerable_m_vLastForce; }

        void Steerable_SetMass(LTFLOAT fMass) { Steerable_m_fMass = fMass; }
        void Steerable_SetMaxForce(LTFLOAT fMaxForce) { Steerable_m_fMaxForce = fMaxForce; }
        void Steerable_SetMaxSpeed(LTFLOAT fMaxSpeed) { Steerable_m_fMaxSpeed = fMaxSpeed; }
        void Steerable_SetUpVector(const LTVector& vUp) { Steerable_m_vUp = vUp; }
        void Steerable_SetForwardVector(const LTVector& vForward) { Steerable_m_vForward = vForward; }
        void Steerable_SetRightVector(const LTVector& vRight) { Steerable_m_vRight = vRight; }
        void Steerable_SetPosition(const LTVector& vPosition) { Steerable_m_vPosition = vPosition; }
        void Steerable_SetVelocity(const LTVector& vVelocity) { Steerable_m_vVelocity = vVelocity; }

	protected : // Protected methods

		friend class CSteeringMgr;

		// Derived CSteerables must implement the Pre and Post update. the Update itself is optional

        virtual LTBOOL Steerable_PreUpdate() = 0;
        virtual LTBOOL Steerable_Update(const LTVector& vSteeringDirection);
        virtual LTBOOL Steerable_PostUpdate() = 0;

        virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
		{
			SAVE_FLOAT(Steerable_m_fMass);
			SAVE_FLOAT(Steerable_m_fMaxForce);
			SAVE_FLOAT(Steerable_m_fMaxSpeed);
			SAVE_VECTOR(Steerable_m_vPosition);
			SAVE_VECTOR(Steerable_m_vVelocity);
			SAVE_VECTOR(Steerable_m_vUp);
			SAVE_VECTOR(Steerable_m_vForward);
			SAVE_VECTOR(Steerable_m_vRight);
			SAVE_VECTOR(Steerable_m_vLastForce);
		}

        virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
		{
			LOAD_FLOAT(Steerable_m_fMass);
			LOAD_FLOAT(Steerable_m_fMaxForce);
			LOAD_FLOAT(Steerable_m_fMaxSpeed);
			LOAD_VECTOR(Steerable_m_vPosition);
			LOAD_VECTOR(Steerable_m_vVelocity);
			LOAD_VECTOR(Steerable_m_vUp);
			LOAD_VECTOR(Steerable_m_vForward);
			LOAD_VECTOR(Steerable_m_vRight);
			LOAD_VECTOR(Steerable_m_vLastForce);
		}

	private : // Private member variables

        LTFLOAT Steerable_m_fMass;
        LTFLOAT Steerable_m_fMaxForce;
        LTFLOAT Steerable_m_fMaxSpeed;

        LTVector Steerable_m_vPosition;
        LTVector Steerable_m_vVelocity;

        LTVector Steerable_m_vUp;
        LTVector Steerable_m_vForward;
        LTVector Steerable_m_vRight;

        LTVector Steerable_m_vLastForce;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CSteering
//
// PURPOSE : Steering abstract class
//
// ----------------------------------------------------------------------- //

class CSteering
{
	public : // Public member variables

		typedef enum SteeringType
		{
			eSteeringSeek,
			eSteeringArrival,
		};

	public : // Public methods

		// Ctors/Dtors/etc

		CSteering();
		virtual ~CSteering() { Term(); }

		// Methods

        virtual LTBOOL Init(CSteerable* pSteerable);
		virtual void Term();

        virtual LTVector Update() = 0;

		// Simple accessors

		virtual SteeringType GetType() = 0;

		CSteerable* GetSteerable() { return m_pSteerable; }

        LTFLOAT GetPriority() { return m_fPriority; }
        void SetPriority(LTFLOAT fPriority) { m_fPriority = fPriority; }

        LTBOOL IsEnabled() { return m_bEnabled; }
        void Enable() { m_bEnabled = LTTRUE; }
        void Disable() { m_bEnabled = LTFALSE; }

		// Save/Load

        virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

	private : // Private member variables

        LTFLOAT      m_fPriority;    // Our priority in SteeringMgr
        LTBOOL       m_bEnabled;     // Are we enabled?
		CSteerable* m_pSteerable;	// The steerable we will be controlling
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CSteeringSeek
//
// PURPOSE : "Seek" steering behavior
//
// ----------------------------------------------------------------------- //

class CSteeringSeek : public CSteering
{
	public : // Public methods

		// Ctors/Dtors/etc

		CSteeringSeek();
		virtual ~CSteeringSeek() { Term(); }

		virtual void Term();

		// Methods

        virtual void Set(const LTVector& vTarget) { m_vTarget = vTarget; }
        virtual LTVector Update();

		// Simple accessors

		virtual SteeringType GetType() { return eSteeringSeek; }

		// Save/Load

        virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
		{
			CSteering::Save(hWrite, dwSaveFlags);

			SAVE_VECTOR(m_vTarget);
		}

        virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
		{
			CSteering::Load(hRead, dwLoadFlags);

			LOAD_VECTOR(m_vTarget);
		}

	protected : // Private member variables

        LTVector     m_vTarget;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CSteeringArrival
//
// PURPOSE : "Arrival" steering behavior. It's the same as seek except it
//			 slows down as it gets closer to the target.
//
// ----------------------------------------------------------------------- //

class CSteeringArrival : public CSteeringSeek
{
	public : // Public methods

		// Methods

        virtual void Set(const LTVector& vTarget, LTFLOAT fArrivalDistance)
		{
			m_vTarget = vTarget;
			m_fArrivalDistance = fArrivalDistance;
		}

        virtual LTVector Update();

		// Simple accessors

		virtual SteeringType GetType() { return eSteeringArrival; }

		// Save/Load

        virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
		{
			CSteeringSeek::Save(hWrite, dwSaveFlags);

			SAVE_FLOAT(m_fArrivalDistance);
		}

        virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
		{
			CSteeringSeek::Load(hRead, dwLoadFlags);

			LOAD_FLOAT(m_fArrivalDistance);
		}

	protected : // Protected methods

        LTFLOAT      m_fArrivalDistance;
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CSteeringMgr
//
// PURPOSE : Mgr to handle all steering behaviors
//
// ----------------------------------------------------------------------- //

class CSteeringMgr
{
	public : // Public methods

		// Ctors/Dtors/etc

		CSteeringMgr();
		~CSteeringMgr() { Term(); }

		// Methods

        LTBOOL Init(CSteerable* pSteerable);
		void Term();

        LTBOOL Update();

		void EnableAllSteering();										// No reason to ever call this really
		void DisableAllSteering();										// Probably want to do this each time you change states

		void EnableSteering(CSteering::SteeringType eSteering);			// Enable a specific steering
		void DisableSteering(CSteering::SteeringType eSteering);		// Disable a specific steering

		CSteering* GetSteering(CSteering::SteeringType eSteering);		// Get a steering behavior

		// Save/Load

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

	private : // Private member variables

		CSteerable*						m_pSteerable;					// The steerable we're controlling
		CSteeringSeek					m_SteeringSeek;					// The seek steering behavior
		CSteeringArrival				m_SteeringArrival;				// The seek arrival behavior
};

#endif