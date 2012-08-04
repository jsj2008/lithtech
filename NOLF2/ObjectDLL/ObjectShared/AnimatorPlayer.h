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

    AniPlayerMain(LTBOOL b, Ani eIdle, Ani eTwitch1, Ani eTwitch2, Ani eIn, Ani eOut )
	{
		eAniIdle = eIdle;
		eAniTwitch1 = eTwitch1;
		eAniTwitch2 = eTwitch2;
		eAniIn	= eIn;
		eAniOut	= eOut;
		bLoops = b;
	}

    LTBOOL IsLoops() { return bLoops; }
	
	Ani eAniIdle;
	Ani eAniTwitch1;
	Ani eAniTwitch2;
	Ani eAniIn;
	Ani eAniOut;
    LTBOOL bLoops;
};

class CAnimatorPlayer : public CAnimator
{
	public :

		// Main enums

		enum Main
		{
			eInvalid		= 0,
			eClimbing		,
			eClimbingUp		,
			eClimbingDown	,		
			eSnowmobile		,
			eDT_Slippery	,
			eDT_Sleeping	,
			eDT_Stun		,
			eDT_Laughing	,
			eDT_BearTrap	,
			eDT_Glue		,
			eDT_Hurt		,
			eDT_HurtWalk	,
	
			kNumMains	
		};

		// Upper enums

		enum Weapon
		{
			// Shared

			eRifle		= 0,
			ePistol		,
			eMelee		,
			eThrow		,
			eRifle2		,
			eRifle3		,
			eHolster	,
			ePlace		,
			eGadget		,
			eStars		,

			kNumWeapons
		};

		enum Posture
		{
			eUnalert	= 0,
			eAlert		,
			eAim		,
			eFire		,
			eFire2		,
			eFire3		,
			eReload		,
			eSelect		,
			eDeselect	,
			eCarry		,
			
			kNumPostures
		};

		// Lower enums

		enum Movement
		{
			eWalking		= 0,
			eRunning		,
			eCrouching		,
			eSwimming		,
			eJumping		,
			eRiding			,

			kNumMovements
		};

		enum Direction
		{
			eNone			= 0,
			eForward		,
			eBackward		,
			eStrafeLeft		,
			eStrafeRight	,
			eJump			,
			eTuck			,
			eLand			,

			kNumDirections	
		};

		// Upper and Lower enums

		enum Lean
		{
			eCenter			= 0,
			eLeft			,
			eRight			,

			kNumLeans
		};

	public :

		// Ctors/Dtors/etc

		CAnimatorPlayer();
		~CAnimatorPlayer();

		void Init(HOBJECT hObject);  // Call only once

		void Reset(HOBJECT hObject); // Call whenever you change models
		
		void SetMainToBase();	// Forces the main tracker to the base animation.

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		// Updates

		void Update();
		void UpdateForceDucking();

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

		LTBOOL IsAnimatingLean(Lean eLean) const;
		LTBOOL IsAnimatingLeanDone(Lean eLean) const;
		inline void SetLean(Lean eLean) { m_eLean = eLean; }
		inline Lean GetLean() { return m_eLean; }
		inline Lean GetLastLean() { return m_eLastLean; }

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
		Lean		m_eLean;					// Our lean

		Main		m_eLastMain;				// Last frame's main
		Weapon		m_eLastWeapon;				// Last frame's weapon
		Posture		m_eLastPosture;				// Last frame's posture
		Movement	m_eLastMovement;			// Last frame's movement
		Direction	m_eLastDirection;			// Last frame's direction
		Lean		m_eLastLean;				// Last frame's lean

		AniTracker	m_eAniTrackerUpper;			// Our ani trackers
		AniTracker	m_eAniTrackerLower;

		// Look up

		AniPlayerUpper* m_aapAniPlayerUppers[kNumWeapons][kNumPostures][kNumLeans];
		AniPlayerLower* m_aapAniPlayerLowers[kNumMovements][kNumDirections][kNumLeans];
		AniPlayerMain* m_apAniPlayerMains[kNumMains];

		// Enormous list of anis.

		AniPlayerLower				m_AniLowerBase;

		AniPlayerLower				m_AniStand;
		AniPlayerLower				m_AniStandLeanL;
		AniPlayerLower				m_AniStandLeanR;

		AniPlayerLower				m_AniWalkForward;
		AniPlayerLower				m_AniWalkBackward;
		AniPlayerLowerStrafe		m_AniWalkStrafeLeft;
		AniPlayerLowerStrafe		m_AniWalkStrafeRight;

		AniPlayerLower				m_AniRunForward;
		AniPlayerLower				m_AniRunBackward;
		AniPlayerLowerStrafe		m_AniRunStrafeLeft;
		AniPlayerLowerStrafe		m_AniRunStrafeRight;

		AniPlayerLowerCrouch		m_AniCrouch;
		AniPlayerLowerCrouch		m_AniCrouchLeanL;
		AniPlayerLowerCrouch		m_AniCrouchLeanR;
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

		AniPlayerUpper				m_AniCarryBody;

		// eRifle
		AniPlayerUpper				m_AniRifleUnalert;
		AniPlayerUpper				m_AniRifleAlert;
		AniPlayerUpper				m_AniRifleAim;
		AniPlayerUpper				m_AniRifleFire;
		AniPlayerUpper				m_AniRifleReload;
		AniPlayerUpper				m_AniRifleSelect;
		AniPlayerUpper				m_AniRifleDeselect;
		AniPlayerUpper				m_AniRifleAimLeanR;
		AniPlayerUpper				m_AniRifleAimLeanL;
		AniPlayerUpper				m_AniRifleFireLeanR;
		AniPlayerUpper				m_AniRifleFireLeanL;

		// ePistol
		AniPlayerUpper				m_AniPistolUnalert;
		AniPlayerUpper				m_AniPistolAlert;
		AniPlayerUpper				m_AniPistolAim;
		AniPlayerUpper				m_AniPistolFire;
		AniPlayerUpper				m_AniPistolReload;
		AniPlayerUpper				m_AniPistolSelect;
		AniPlayerUpper				m_AniPistolDeselect;
		AniPlayerUpper				m_AniPistolAimLeanR;
		AniPlayerUpper				m_AniPistolAimLeanL;
		AniPlayerUpper				m_AniPistolFireLeanR;
		AniPlayerUpper				m_AniPistolFireLeanL;

		// eMelee
		AniPlayerUpper				m_AniMeleeUnalert;
		AniPlayerUpper				m_AniMeleeAlert;
		AniPlayerUpper				m_AniMeleeAim;
		AniPlayerUpper				m_AniMeleeFire;
		AniPlayerUpper				m_AniMeleeFire2;
		AniPlayerUpper				m_AniMeleeFire3;
		AniPlayerUpper				m_AniMeleeReload;
		AniPlayerUpper				m_AniMeleeSelect;
		AniPlayerUpper				m_AniMeleeDeselect;
		AniPlayerUpper				m_AniMeleeAimLeanR;
		AniPlayerUpper				m_AniMeleeAimLeanL;
		AniPlayerUpper				m_AniMeleeFireLeanR;
		AniPlayerUpper				m_AniMeleeFireLeanL;
		
		// eThrow
		AniPlayerUpper				m_AniThrowUnalert;
		AniPlayerUpper				m_AniThrowAlert;
		AniPlayerUpper				m_AniThrowAim;
		AniPlayerUpper				m_AniThrowFire;
		AniPlayerUpper				m_AniThrowReload;
		AniPlayerUpper				m_AniThrowSelect;
		AniPlayerUpper				m_AniThrowDeselect;
		AniPlayerUpper				m_AniThrowAimLeanR;
		AniPlayerUpper				m_AniThrowAimLeanL;
		AniPlayerUpper				m_AniThrowFireLeanR;
		AniPlayerUpper				m_AniThrowFireLeanL;

		// eRifle2
		AniPlayerUpper				m_AniRifle2Unalert;
		AniPlayerUpper				m_AniRifle2Alert;
		AniPlayerUpper				m_AniRifle2Aim;
		AniPlayerUpper				m_AniRifle2Fire;
		AniPlayerUpper				m_AniRifle2Reload;
		AniPlayerUpper				m_AniRifle2Select;
		AniPlayerUpper				m_AniRifle2Deselect;
		AniPlayerUpper				m_AniRifle2AimLeanR;
		AniPlayerUpper				m_AniRifle2AimLeanL;
		AniPlayerUpper				m_AniRifle2FireLeanR;
		AniPlayerUpper				m_AniRifle2FireLeanL;

		// eRifle3
		AniPlayerUpper				m_AniRifle3Unalert;
		AniPlayerUpper				m_AniRifle3Alert;
		AniPlayerUpper				m_AniRifle3Aim;
		AniPlayerUpper				m_AniRifle3Fire;
		AniPlayerUpper				m_AniRifle3Reload;
		AniPlayerUpper				m_AniRifle3Select;
		AniPlayerUpper				m_AniRifle3Deselect;
		AniPlayerUpper				m_AniRifle3AimLeanR;
		AniPlayerUpper				m_AniRifle3AimLeanL;
		AniPlayerUpper				m_AniRifle3FireLeanR;
		AniPlayerUpper				m_AniRifle3FireLeanL;

		// eHolster
		AniPlayerUpper				m_AniHolsterUnalert;
		AniPlayerUpper				m_AniHolsterAlert;
		AniPlayerUpper				m_AniHolsterAim;
		AniPlayerUpper				m_AniHolsterFire;
		AniPlayerUpper				m_AniHolsterReload;
		AniPlayerUpper				m_AniHolsterSelect;
		AniPlayerUpper				m_AniHolsterDeselect;
		AniPlayerUpper				m_AniHolsterAimLeanR;
		AniPlayerUpper				m_AniHolsterAimLeanL;
		AniPlayerUpper				m_AniHolsterFireLeanR;
		AniPlayerUpper				m_AniHolsterFireLeanL;

		// ePlace
		AniPlayerUpper				m_AniPlaceUnalert;
		AniPlayerUpper				m_AniPlaceAlert;
		AniPlayerUpper				m_AniPlaceAim;
		AniPlayerUpper				m_AniPlaceFire;
		AniPlayerUpper				m_AniPlaceReload;
		AniPlayerUpper				m_AniPlaceSelect;
		AniPlayerUpper				m_AniPlaceDeselect;
		AniPlayerUpper				m_AniPlaceAimLeanR;
		AniPlayerUpper				m_AniPlaceAimLeanL;
		AniPlayerUpper				m_AniPlaceFireLeanR;
		AniPlayerUpper				m_AniPlaceFireLeanL;

		// eGadget
		AniPlayerUpper				m_AniGadgetUnalert;
		AniPlayerUpper				m_AniGadgetAlert;
		AniPlayerUpper				m_AniGadgetAim;
		AniPlayerUpper				m_AniGadgetFire;
		AniPlayerUpper				m_AniGadgetReload;
		AniPlayerUpper				m_AniGadgetSelect;
		AniPlayerUpper				m_AniGadgetDeselect;
		AniPlayerUpper				m_AniGadgetAimLeanR;
		AniPlayerUpper				m_AniGadgetAimLeanL;
		AniPlayerUpper				m_AniGadgetFireLeanR;
		AniPlayerUpper				m_AniGadgetFireLeanL;

		// eStars
		AniPlayerUpper				m_AniStarsUnalert;
		AniPlayerUpper				m_AniStarsAlert;
		AniPlayerUpper				m_AniStarsAim;
		AniPlayerUpper				m_AniStarsFire;
		AniPlayerUpper				m_AniStarsReload;
		AniPlayerUpper				m_AniStarsSelect;
		AniPlayerUpper				m_AniStarsDeselect;
		AniPlayerUpper				m_AniStarsAimLeanR;
		AniPlayerUpper				m_AniStarsAimLeanL;
		AniPlayerUpper				m_AniStarsFireLeanR;
		AniPlayerUpper				m_AniStarsFireLeanL;

		AniPlayerMain				m_AniMainBase;

		AniPlayerMain				m_AniClimb;
		AniPlayerMain				m_AniClimbUp;
		AniPlayerMain				m_AniClimbDown;
		AniPlayerMain				m_AniSnowmobile;

		AniPlayerMain				m_AniDT_Slippery;
		AniPlayerMain				m_AniDT_Sleeping;
		AniPlayerMain				m_AniDT_Stun;
		AniPlayerMain				m_AniDT_Laughing;
		AniPlayerMain				m_AniDT_BearTrap;
		AniPlayerMain				m_AniDT_Glue;
		AniPlayerMain				m_AniDT_Hurt;
		AniPlayerMain				m_AniDT_HurtWalk;
};

#endif // __ANIMATORPlayer_H__