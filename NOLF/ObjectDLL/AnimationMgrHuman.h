// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATION_MGR_HUMAN_H__
#define __ANIMATION_MGR_HUMAN_H__

#include "AnimationMgr.h"

// Animation

extern class CAnimationMgrHuman* g_pAnimationMgrHuman;

class CAnimationMgrHuman : public CAnimationMgr
{
	public :

		CAnimationMgrHuman()
		{
			g_pAnimationMgrHuman = this;
		}

		~CAnimationMgrHuman()
		{
			g_pAnimationMgrHuman = LTNULL;
		}

		LTBOOL Init();
};

namespace AnimationsHuman
{
	extern CAnimationProp aniCrouch;
	extern CAnimationProp aniStand;
	extern CAnimationProp aniSwim;
	extern CAnimationProp aniSit;
	extern CAnimationProp aniRide;
	extern CAnimationProp aniProne;

	extern CAnimationProp aniAim;
	extern CAnimationProp aniFire;
	extern CAnimationProp aniReload;
	extern CAnimationProp aniThrow;
	extern CAnimationProp aniDraw;

	extern CAnimationProp aniLower;
	extern CAnimationProp aniDown;
	extern CAnimationProp aniUp;

	extern CAnimationProp aniRun;
	extern CAnimationProp aniWalk;
	extern CAnimationProp aniPatrol;
	extern CAnimationProp aniCrawl;
	extern CAnimationProp aniPinned;

	extern CAnimationProp aniWeapon1;
	extern CAnimationProp aniWeapon2;
	extern CAnimationProp aniWeapon3;

	extern CAnimationProp aniSilent;
	extern CAnimationProp aniHappy;
	extern CAnimationProp aniAngry;
	extern CAnimationProp aniSad;
	extern CAnimationProp aniTense;
	extern CAnimationProp aniAgree;
	extern CAnimationProp aniDisagree;

	extern CAnimationProp aniCornerLeft;
	extern CAnimationProp aniCornerRight;
	extern CAnimationProp aniRollLeft;
	extern CAnimationProp aniRollRight;
	extern CAnimationProp aniShuffleLeft;
	extern CAnimationProp aniShuffleRight;
	extern CAnimationProp aniBlind;
	extern CAnimationProp aniPopup;

	extern CAnimationProp aniInvestigate;
	extern CAnimationProp aniDistress;
	extern CAnimationProp aniPanic;
	extern CAnimationProp aniDrowsy;
	extern CAnimationProp aniCovered;
	extern CAnimationProp aniUncovered;
	extern CAnimationProp aniCovering;
	extern CAnimationProp aniUncovering;

	extern CAnimationProp aniCheckPulse;
	extern CAnimationProp aniClipboard;
	extern CAnimationProp aniSweep;
	extern CAnimationProp aniWipe;
	extern CAnimationProp aniDust;
	extern CAnimationProp aniTicket;
	extern CAnimationProp aniOpenDoor;
	extern CAnimationProp aniKnockOnDoor;
	extern CAnimationProp aniPickup;
	extern CAnimationProp aniPushButton;
	extern CAnimationProp aniTail;
	extern CAnimationProp aniAsleep;
	extern CAnimationProp aniAwake;
	extern CAnimationProp aniUnconscious;
	extern CAnimationProp aniStunned;
	extern CAnimationProp aniFlashlight;
	extern CAnimationProp aniLookUnder;
	extern CAnimationProp aniLookOver;
	extern CAnimationProp aniLookLeft;
	extern CAnimationProp aniLookRight;
	extern CAnimationProp aniAlert1;
	extern CAnimationProp aniAlert2;
	extern CAnimationProp aniAlert3;

	extern CAnimationProp aniEnraged;	// Scot
	extern CAnimationProp aniTaunting;
	extern CAnimationProp aniVictory;
	extern CAnimationProp aniDefeat;
	extern CAnimationProp aniBox;
	extern CAnimationProp aniDash;
	extern CAnimationProp aniPunch1;
	extern CAnimationProp aniPunch2;
	extern CAnimationProp aniPunch3;
	extern CAnimationProp aniPunch4;
	extern CAnimationProp aniSlam;

	extern CAnimationProp aniSing;		// Inge
	extern CAnimationProp aniSword1;
	extern CAnimationProp aniSword2;
};

#endif // #ifndef __ANIMATION_MGR_HUMAN_H__
