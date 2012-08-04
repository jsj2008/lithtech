// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATORAIVEHICLE_H__
#define __ANIMATORAIVEHICLE_H__

#include "Animator.h"

struct AniAIVehicleMain
{
	AniAIVehicleMain()
	{
	}

    AniAIVehicleMain(LTBOOL b, Ani e1)
	{
		eAni = e1;
		bLoops = b;
	}

    LTBOOL IsLoops() { return bLoops; }

	Ani eAni;
    LTBOOL bLoops;
};

struct AniAIVehicleFire
{
	AniAIVehicleFire()
	{
	}

	AniAIVehicleFire(Ani e1)
	{
		eAni = e1;
	}

	Ani eAni;
};

class CAnimatorAIVehicle : public CAnimator
{
	public :

		// Constants

		enum Constants
		{
			kNumMains = 8,
			kNumFires = 6,
		};

		// Main enums

		enum Main
		{
			eInvalid = 0,
			eIdle = 1,
			eOpenRightDoor = 4,
			eOpenedRightDoor = 5,
			eCloseRightDoor = 6,
			eClosedRightDoor = 7,
		};

		enum Fire
		{
			eFireAim = 0,
			eFireFull = 1,
			eFire1 = 2,
			eFire2 = 3,
			eFire3 = 4,
			eFire4 = 5,
		};

	public :

		// Ctors/Dtors/etc

		CAnimatorAIVehicle();
		~CAnimatorAIVehicle();

		void Init(ILTCSBase *pInterface, HOBJECT hObject);

		// Updates

		void Update();

		// Simple accessors

        LTBOOL IsAnimatingMain(Main eMain) const;
        LTBOOL IsAnimatingMainDone(Main eMain) const;
		inline void SetMain(Main eMain) { m_eMain = eMain; }
		inline Main GetMain() { return m_eMain; }
		inline Main GetLastMain() { return m_eLastMain; }

		inline void SetFire(Fire eFire) { m_eFire = eFire; }

	protected :

		// Tracker methods

		void ResetAniTracker(AniTracker eAniTracker);

		// Dims

        LTBOOL SetDims(HMODELANIM hAni);

	protected :

		// Animation trackers

		AniTracker	m_eAniTrackerFire;			// Our firing animation tracker

		// Animations

		Main		m_eMain;					// Our main
		Fire		m_eFire;					// Our Fire

		Main		m_eLastMain;				// Last frame's main
		Fire		m_eLastFire;				// Last frame's fire

		// Ani

		AniAIVehicleMain* m_apAniAIVehicleMains[kNumMains];
		AniAIVehicleFire* m_apAniAIVehicleFires[kNumFires];

		// Enormous list of anis.

		AniAIVehicleMain				m_AniBase;
		AniAIVehicleMain				m_AniIdle;
		AniAIVehicleMain				m_AniOpenRightDoor;
		AniAIVehicleMain				m_AniOpenedRightDoor;
		AniAIVehicleMain				m_AniCloseRightDoor;
		AniAIVehicleMain				m_AniClosedRightDoor;

		AniAIVehicleFire				m_AniFireAim;
		AniAIVehicleFire				m_AniFireFull;
		AniAIVehicleFire				m_AniFire1;
		AniAIVehicleFire				m_AniFire2;
		AniAIVehicleFire				m_AniFire3;
		AniAIVehicleFire				m_AniFire4;
};

#endif // __ANIMATORAIVehicle_H__