// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATORPLAYER_H__
#define __ANIMATORPLAYER_H__

#include "Animator.h"

struct AniPlayerLower
{
	AniPlayerLower() {}
    AniPlayerLower(LTBOOL b, Ani e) { bLoops = b; eAni = e; }

    LTBOOL IsLoops() { return bLoops; }

    virtual LTBOOL IsStrafe() { return LTFALSE; }
    virtual LTBOOL IsCrouch() { return LTFALSE; }
    virtual LTBOOL IsSwim() { return LTFALSE; }

	Ani		eAni;
    LTBOOL   bLoops;
};

struct AniPlayerLowerStrafe : public AniPlayerLower
{
	AniPlayerLowerStrafe() {}
    AniPlayerLowerStrafe(LTBOOL b, Ani e) : AniPlayerLower(b, e) {}
    virtual LTBOOL IsStrafe() { return LTTRUE; }
};

struct AniPlayerLowerCrouch : public AniPlayerLower
{
	AniPlayerLowerCrouch() {}
    AniPlayerLowerCrouch(LTBOOL b, Ani e) : AniPlayerLower(b, e) {}
    virtual LTBOOL IsCrouch() { return LTTRUE; }
};

struct AniPlayerLowerSwim : public AniPlayerLower
{
	AniPlayerLowerSwim() {}
    AniPlayerLowerSwim(LTBOOL b, Ani e) : AniPlayerLower(b, e) {}
    virtual LTBOOL IsSwim() { return LTTRUE; }
};

struct AniPlayerLowerCrouchStrafe : public AniPlayerLower
{
	AniPlayerLowerCrouchStrafe() {}
    AniPlayerLowerCrouchStrafe(LTBOOL b, Ani e) : AniPlayerLower(b, e) {}
    virtual LTBOOL IsCrouch() { return LTTRUE; }
    virtual LTBOOL IsStrafe() { return LTTRUE; }
};

struct AniPlayerUpper
{
	AniPlayerUpper()
	{

	}

    AniPlayerUpper(LTBOOL b, Ani e1, Ani e2, Ani e3, Ani e4, Ani e5)
	{
		bLoops				= b;
		eAni				= e1;
		eAniStrafe			= e2;
		eAniCrouch			= e3;
		eAniSwim			= e4;
		eAniStrafeCrouch	= e5;
	}

    LTBOOL IsLoops() { return bLoops; }

	Ani	eAni;
	Ani	eAniStrafe;
	Ani	eAniCrouch;
	Ani	eAniSwim;
	Ani	eAniStrafeCrouch;
    LTBOOL bLoops;
};

struct AniPlayerMain
{
	AniPlayerMain()
	{
	}

    AniPlayerMain(LTBOOL b, Ani e1)
	{
		eAni = e1;
		bLoops = b;
	}

    LTBOOL IsLoops() { return bLoops; }

	Ani eAni;
    LTBOOL bLoops;
};

class CAnimatorPlayer : public CAnimator
{
	public :

		// Constants

		enum Constants
		{
			kNumWeapons = 5,
			kNumPostures = 7,
			kNumMovements = 6,
			kNumDirections = 10,
			kNumMains = 6,
		};

		// Main enums

		enum Main
		{
			eInvalid = 0,
			eClimbing = 1,
			eClimbingUp = 2,
			eClimbingDown = 3,
			eMotorcycle = 4,
			eSnowmobile = 5,
		};

		// Upper enums

		enum Weapon
		{
			// Shared

			eRifle		= 0,
			ePistol		= 1,
			eMelee		= 2,
			eThrow		= 3,
			eSunglasses = 4,
		};

		enum Posture
		{
			eUnalert	= 0,
			eAlert		= 1,
			eAim		= 2,
			eFire		= 3,
			eReload		= 4,
			eSelect		= 5,
			eDeselect	= 6,
		};

		// Lower enums

		enum Movement
		{
			eWalking		= 0,
			eRunning		= 1,
			eCrouching		= 2,
			eSwimming		= 3,
			eJumping		= 4,
			eRiding			= 5,
		};

		enum Direction
		{
			eNone			= 0,
			eForward		= 1,
			eBackward		= 2,
			eStrafeLeft		= 3,
			eStrafeRight	= 4,
			eJump			= 5,
			eTuck			= 6,
			eLand			= 7,
		};

	public :

		// Ctors/Dtors/etc

		CAnimatorPlayer();
		~CAnimatorPlayer();

		void Init(ILTCSBase* pInterface, HOBJECT hObject);  // Call only once

		void Reset(HOBJECT hObject); // Call whenever you change models

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Updates

		void Update();
		void UpdateDims();

		// Simple accessors

        LTBOOL IsAnimatingMain(Main eMain) const;
        LTBOOL IsAnimatingMainDone(Main eMain) const;
		inline void SetMain(Main eMain) { m_eMain = eMain; }
		inline Main GetMain() { return m_eMain; }
		inline Main GetLastMain() { return m_eLastMain; }

        LTBOOL IsAnimatingWeapon(Weapon eWeapon) const;
        LTBOOL IsAnimatingWeaponDone(Weapon eWeapon) const;
		inline void SetWeapon(Weapon eWeapon) { m_eWeapon = eWeapon; }
		inline Weapon GetWeapon() { return m_eWeapon; }
		inline Weapon GetLastWeapon() { return m_eLastWeapon; }

        LTBOOL IsAnimatingPosture(Posture ePosture) const;
        LTBOOL IsAnimatingPostureDone(Posture ePosture) const;
		inline void SetPosture(Posture ePosture) { m_ePosture = ePosture; }
		inline Posture GetPosture() { return m_ePosture; }
		inline Posture GetLastPosture() { return m_eLastPosture; }

        LTBOOL IsAnimatingMovement(Movement eMovement) const;
        LTBOOL IsAnimatingMovementDone(Movement eMovement) const;
		inline void SetMovement(Movement eMovement) { m_eMovement = eMovement; }
		inline Movement GetMovement() { return m_eMovement; }
		inline Movement GetLastMovement() { return m_eLastMovement; }

        LTBOOL IsAnimatingDirection(Direction eDirection) const;
        LTBOOL IsAnimatingDirectionDone(Direction eDirection) const;
		inline void SetDirection(Direction eDirection) { m_eDirection = eDirection; }
		inline Direction GetDirection() { return m_eDirection; }
		inline Direction GetLastDirection() { return m_eLastDirection; }

	protected :

		// Tracker methods

		void ResetAniTracker(AniTracker eAniTracker);

		// Dims

        LTBOOL SetDims(HMODELANIM hAni);

	protected :

		Main		m_eMain;					// Our main
		Weapon		m_eWeapon;					// Our weapon
		Posture		m_ePosture;					// Our posture
		Movement	m_eMovement;				// Our movement
		Direction	m_eDirection;				// Our direction

		Main		m_eLastMain;				// Last frame's main
		Weapon		m_eLastWeapon;				// Last frame's weapon
		Posture		m_eLastPosture;				// Last frame's posture
		Movement	m_eLastMovement;			// Last frame's movement
		Direction	m_eLastDirection;			// Last frame's direction

		AniTracker	m_eAniTrackerUpper;			// Our ani trackers
		AniTracker	m_eAniTrackerLower;

		// Look up

		AniPlayerUpper* m_aapAniPlayerUppers[kNumWeapons][kNumPostures];
		AniPlayerLower* m_aapAniPlayerLowers[kNumMovements][kNumDirections];
		AniPlayerMain* m_apAniPlayerMains[kNumMains];

		// Enormous list of anis.

		AniPlayerLower				m_AniLowerBase;

		AniPlayerLower				m_AniStand;

		AniPlayerLower				m_AniWalkForward;
		AniPlayerLower				m_AniWalkBackward;
		AniPlayerLowerStrafe		m_AniWalkStrafeLeft;
		AniPlayerLowerStrafe		m_AniWalkStrafeRight;

		AniPlayerLower				m_AniRunForward;
		AniPlayerLower				m_AniRunBackward;
		AniPlayerLowerStrafe		m_AniRunStrafeLeft;
		AniPlayerLowerStrafe		m_AniRunStrafeRight;

		AniPlayerLowerCrouch		m_AniCrouch;
		AniPlayerLowerCrouch		m_AniCrouchForward;
		AniPlayerLowerCrouch		m_AniCrouchBackward;
		AniPlayerLowerCrouchStrafe	m_AniCrouchStrafeLeft;
		AniPlayerLowerCrouchStrafe	m_AniCrouchStrafeRight;

		AniPlayerLowerSwim			m_AniSwim;
		AniPlayerLowerSwim			m_AniSwimForward;

		AniPlayerLower				m_AniJumpJump;
		AniPlayerLower				m_AniJumpTuck;
		AniPlayerLower				m_AniJumpLand;

		AniPlayerUpper				m_AniUpperBase;

		AniPlayerUpper				m_AniRifleUnalert;
		AniPlayerUpper				m_AniRifleAlert;
		AniPlayerUpper				m_AniRifleAim;
		AniPlayerUpper				m_AniRifleFire;
		AniPlayerUpper				m_AniRifleReload;
		AniPlayerUpper				m_AniRifleSelect;
		AniPlayerUpper				m_AniRifleDeselect;

		AniPlayerUpper				m_AniPistolUnalert;
		AniPlayerUpper				m_AniPistolAlert;
		AniPlayerUpper				m_AniPistolAim;
		AniPlayerUpper				m_AniPistolFire;
		AniPlayerUpper				m_AniPistolReload;
		AniPlayerUpper				m_AniPistolSelect;
		AniPlayerUpper				m_AniPistolDeselect;

		AniPlayerUpper				m_AniMeleeUnalert;
		AniPlayerUpper				m_AniMeleeAlert;
		AniPlayerUpper				m_AniMeleeAim;
		AniPlayerUpper				m_AniMeleeFire;
		AniPlayerUpper				m_AniMeleeReload;
		AniPlayerUpper				m_AniMeleeSelect;
		AniPlayerUpper				m_AniMeleeDeselect;

		AniPlayerUpper				m_AniThrowUnalert;
		AniPlayerUpper				m_AniThrowAlert;
		AniPlayerUpper				m_AniThrowAim;
		AniPlayerUpper				m_AniThrowFire;
		AniPlayerUpper				m_AniThrowReload;
		AniPlayerUpper				m_AniThrowSelect;
		AniPlayerUpper				m_AniThrowDeselect;

		AniPlayerUpper				m_AniSunglassesUnalert;
		AniPlayerUpper				m_AniSunglassesAlert;
		AniPlayerUpper				m_AniSunglassesAim;
		AniPlayerUpper				m_AniSunglassesFire;
		AniPlayerUpper				m_AniSunglassesReload;
		AniPlayerUpper				m_AniSunglassesSelect;
		AniPlayerUpper				m_AniSunglassesDeselect;

		AniPlayerMain				m_AniMainBase;

		AniPlayerMain				m_AniClimb;
		AniPlayerMain				m_AniClimbUp;
		AniPlayerMain				m_AniClimbDown;
		AniPlayerMain				m_AniMotorcycle;
		AniPlayerMain				m_AniSnowmobile;
};

#endif // __ANIMATORPlayer_H__