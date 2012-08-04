// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATORAIANIMAL_H__
#define __ANIMATORAIANIMAL_H__

#include "Animator.h"

struct AniAIAnimalMain
{
	AniAIAnimalMain()
	{
	}

    AniAIAnimalMain(LTBOOL b, Ani e1)
	{
		eAni = e1;
		bLoops = b;
	}

    LTBOOL IsLoops() { return bLoops; }

	Ani eAni;
    LTBOOL bLoops;
};

class CAnimatorAIAnimal : public CAnimator
{
	public :

		// Constants

		enum Constants
		{
			kNumMains = 16,
		};

		// Main enums

		enum Main
		{
			eInvalid = 0,
			eIdle = 1,
			eWalking = 4,
			eRunning = 5,
			eBite = 6,
			eIdleStand = 7,
			eIdleSit = 8,
			eIdleLay = 9,
			eFenceJump = 10,
			eFenceBark = 11,
			eFenceUnjump = 12,
			eSniffPoodle = 13,
			eSwimming = 14,
			eMaul = 15,
		};

	public :

		// Ctors/Dtors/etc

		CAnimatorAIAnimal();
		~CAnimatorAIAnimal();

		void Init(ILTCSBase *pInterface, HOBJECT hObject);

		// Updates

		void Update();

		// Simple accessors

        LTBOOL IsAnimatingMain(Main eMain) const;
        LTBOOL IsAnimatingMainDone(Main eMain) const;
		inline void SetMain(Main eMain) { m_eMain = eMain; }
		inline Main GetMain() { return m_eMain; }
		inline Main GetLastMain() { return m_eLastMain; }

	protected :

		// Tracker methods

		void ResetAniTracker(AniTracker eAniTracker);

		// Dims

        LTBOOL SetDims(HMODELANIM hAni);

	protected :

		Main		m_eMain;					// Our main

		Main		m_eLastMain;				// Last frame's main

		// Ani

		AniAIAnimalMain* m_apAniAIAnimalMains[kNumMains];

		// Enormous list of anis.

		AniAIAnimalMain				m_AniBase;
		AniAIAnimalMain				m_AniIdle;
		AniAIAnimalMain				m_AniIdleStand;
		AniAIAnimalMain				m_AniIdleSit;
		AniAIAnimalMain				m_AniIdleLay;
		AniAIAnimalMain				m_AniFenceJump;
		AniAIAnimalMain				m_AniFenceBark;
		AniAIAnimalMain				m_AniFenceUnjump;
		AniAIAnimalMain				m_AniWalking;
		AniAIAnimalMain				m_AniRunning;
		AniAIAnimalMain				m_AniBite;
		AniAIAnimalMain				m_AniSniffPoodle;
		AniAIAnimalMain				m_AniSwimming;
		AniAIAnimalMain				m_AniMaul;
};

#endif // __ANIMATORAIAnimal_H__