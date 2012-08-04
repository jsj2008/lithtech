// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AnimationMgrHuman.h"

using namespace AnimationsHuman;

// ----------------------------------------------------------------------- //
//	
//	DATA:		AnimationsHuman
//
//	PURPOSE:	All the human animation props
//
// ----------------------------------------------------------------------- //

// Global animation mgr pointer

CAnimationMgrHuman* g_pAnimationMgrHuman = LTNULL;

// Animation props

namespace AnimationsHuman
{
	CAnimationProp aniCrouch;
	CAnimationProp aniStand;
	CAnimationProp aniSwim;
	CAnimationProp aniSit;
	CAnimationProp aniRide;
	CAnimationProp aniProne;

	CAnimationProp aniAim;
	CAnimationProp aniFire;
	CAnimationProp aniReload;
	CAnimationProp aniThrow;
	CAnimationProp aniDraw;

	CAnimationProp aniLower;
	CAnimationProp aniDown;
	CAnimationProp aniUp;

	CAnimationProp aniRun;
	CAnimationProp aniWalk;
	CAnimationProp aniPatrol;
	CAnimationProp aniCrawl;
	CAnimationProp aniPinned;

	CAnimationProp aniWeapon1;
	CAnimationProp aniWeapon2;
	CAnimationProp aniWeapon3;

	CAnimationProp aniSilent;
	CAnimationProp aniHappy;
	CAnimationProp aniAngry;
	CAnimationProp aniSad;
	CAnimationProp aniTense;
	CAnimationProp aniAgree;
	CAnimationProp aniDisagree;

	CAnimationProp aniCornerLeft;
	CAnimationProp aniCornerRight;
	CAnimationProp aniRollLeft;
	CAnimationProp aniRollRight;
	CAnimationProp aniShuffleLeft;
	CAnimationProp aniShuffleRight;
	CAnimationProp aniBlind;
	CAnimationProp aniPopup;

	CAnimationProp aniInvestigate;
	CAnimationProp aniDistress;
	CAnimationProp aniPanic;
	CAnimationProp aniDrowsy;
	CAnimationProp aniCovered;
	CAnimationProp aniUncovered;
	CAnimationProp aniCovering;
	CAnimationProp aniUncovering;

	CAnimationProp aniCheckPulse;
	CAnimationProp aniClipboard;
	CAnimationProp aniDust;
	CAnimationProp aniSweep;
	CAnimationProp aniWipe;
	CAnimationProp aniTicket;
	CAnimationProp aniOpenDoor;
	CAnimationProp aniKnockOnDoor;
	CAnimationProp aniPickup;
	CAnimationProp aniPushButton;
	CAnimationProp aniTail;
	CAnimationProp aniAsleep;
	CAnimationProp aniAwake;
	CAnimationProp aniUnconscious;
	CAnimationProp aniStunned;
	CAnimationProp aniFlashlight;
	CAnimationProp aniLookUnder;
	CAnimationProp aniLookOver;
	CAnimationProp aniLookLeft;
	CAnimationProp aniLookRight;
	CAnimationProp aniAlert1;
	CAnimationProp aniAlert2;
	CAnimationProp aniAlert3;

	CAnimationProp aniBox;			// Scot
	CAnimationProp aniPunch1;
	CAnimationProp aniPunch2;
	CAnimationProp aniPunch3;
	CAnimationProp aniPunch4;
	CAnimationProp aniSlam;
	CAnimationProp aniEnraged;
	CAnimationProp aniTaunting;
	CAnimationProp aniVictory;
	CAnimationProp aniDefeat;

	CAnimationProp aniSing;			// Inge
	CAnimationProp aniSword1;
	CAnimationProp aniSword2;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationMgrHuman::Init
//
//	PURPOSE:	Initializes all the animation props
//
// ----------------------------------------------------------------------- //

LTBOOL CAnimationMgrHuman::Init()
{
	if ( !CAnimationMgr::Init("Attributes\\AnimationsHuman.txt") )
	{
		return LTFALSE;
	}

	aniCrouch		= FindAnimationProp("Crouch");
	aniStand		= FindAnimationProp("Stand");
	aniSwim			= FindAnimationProp("Swim");
	aniSit			= FindAnimationProp("Sit");
	aniRide			= FindAnimationProp("Ride");
	aniProne		= FindAnimationProp("Prone");

	aniAim			= FindAnimationProp("Aim");
	aniFire			= FindAnimationProp("Fire");
	aniReload		= FindAnimationProp("Reload");
	aniThrow		= FindAnimationProp("Throw");
	aniDraw			= FindAnimationProp("Draw");

	aniLower		= FindAnimationProp("Lower");
	aniDown			= FindAnimationProp("Down");
	aniUp			= FindAnimationProp("Up");

	aniRun			= FindAnimationProp("Run");
	aniWalk			= FindAnimationProp("Walk");
	aniPatrol		= FindAnimationProp("Patrol");
	aniCrawl		= FindAnimationProp("Crawl");
	aniPinned		= FindAnimationProp("Pinned");

	aniWeapon1		= FindAnimationProp("Weapon1");
	aniWeapon2		= FindAnimationProp("Weapon2");
	aniWeapon3		= FindAnimationProp("Weapon3");

	aniSilent		= FindAnimationProp("Silent");
	aniHappy		= FindAnimationProp("Happy");
	aniAngry		= FindAnimationProp("Angry");
	aniSad			= FindAnimationProp("Sad");
	aniTense		= FindAnimationProp("Tense");
	aniAgree		= FindAnimationProp("Agree");
	aniDisagree		= FindAnimationProp("Disagree");

	aniCornerLeft	= FindAnimationProp("CornerLeft");
	aniCornerRight	= FindAnimationProp("CornerRight");
	aniRollLeft		= FindAnimationProp("RollLeft");
	aniRollRight	= FindAnimationProp("RollRight");
	aniShuffleLeft	= FindAnimationProp("ShuffleLeft");
	aniShuffleRight	= FindAnimationProp("ShuffleRight");
	aniBlind		= FindAnimationProp("Blind");
	aniPopup		= FindAnimationProp("Popup");
	aniCovered		= FindAnimationProp("Covered");
	aniUncovered	= FindAnimationProp("Uncovered");
	aniCovering		= FindAnimationProp("Covering");
	aniUncovering	= FindAnimationProp("Uncovering");

	aniInvestigate	= FindAnimationProp("Investigate");
	aniDistress		= FindAnimationProp("Distress");
	aniPanic		= FindAnimationProp("Panic");
	aniDrowsy		= FindAnimationProp("Drowsy");

	aniCheckPulse	= FindAnimationProp("CheckPulse");
	aniClipboard	= FindAnimationProp("Clipboard");
	aniSweep		= FindAnimationProp("Sweep");
	aniWipe			= FindAnimationProp("Wipe");
	aniDust			= FindAnimationProp("Dust");
	aniOpenDoor		= FindAnimationProp("OpenDoor");
	aniKnockOnDoor	= FindAnimationProp("KnockOnDoor");
	aniPickup		= FindAnimationProp("Pickup");
	aniPushButton	= FindAnimationProp("PushButton");
	aniTail			= FindAnimationProp("Tail");
	aniAwake		= FindAnimationProp("Awake");
	aniAsleep		= FindAnimationProp("Asleep");
	aniUnconscious	= FindAnimationProp("Unconscious");
	aniStunned		= FindAnimationProp("Stunned");
	aniFlashlight	= FindAnimationProp("Flashlight");
	aniLookUnder	= FindAnimationProp("LookUnder");
	aniLookOver		= FindAnimationProp("LookOver");
	aniLookLeft		= FindAnimationProp("LookLeft");
	aniLookRight	= FindAnimationProp("LookRight");
	aniAlert1		= FindAnimationProp("Alert1");
	aniAlert2		= FindAnimationProp("Alert2");
	aniAlert3		= FindAnimationProp("Alert3");

	aniBox			= FindAnimationProp("Box");
	aniPunch1		= FindAnimationProp("Punch1");
	aniPunch2		= FindAnimationProp("Punch2");
	aniPunch3		= FindAnimationProp("Punch3");
	aniPunch4		= FindAnimationProp("Punch4");
	aniSlam			= FindAnimationProp("Slam");
	aniEnraged		= FindAnimationProp("Enraged");
	aniTaunting		= FindAnimationProp("Taunting");
	aniVictory		= FindAnimationProp("Victory");
	aniDefeat		= FindAnimationProp("Defeat");

	aniSing			= FindAnimationProp("Sing");
	aniSword1		= FindAnimationProp("Sword1");
	aniSword2		= FindAnimationProp("Sword2");

	return LTTRUE;
}
