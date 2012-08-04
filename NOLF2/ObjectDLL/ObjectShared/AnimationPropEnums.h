// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationPropEnums.h
//
// PURPOSE : Enums and string constants for animation properties.
//
// CREATED : 6/13/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_ANIM_PROP
	#undef ADD_ANIM_PROP
#endif
 
#if ANIM_PROP_AS_ENUM
	#define ADD_ANIM_PROP(label) kAP_##label##,
#elif ANIM_PROP_AS_STRING
	#define ADD_ANIM_PROP(label) #label,
#else
	#error ! To use this include file, first define either ANIM_PROP_AS_ENUM or ANIM_PROP_AS_STRING, to include the sense types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_ANIM_PROP(x) 
// where x is the name of the enum without the "kAP_" prefix.
// --------------------------------------------------------------------------

ADD_ANIM_PROP(None)				// kAP_None
ADD_ANIM_PROP(Any)				// kAP_Any
ADD_ANIM_PROP(Crouch)			// kAP_Crouch
ADD_ANIM_PROP(Stand)			// kAP_Stand
ADD_ANIM_PROP(Sit)				// kAP_Sit
ADD_ANIM_PROP(Ride)				// kAP_Ride
ADD_ANIM_PROP(Prone)			// kAP_Prone
ADD_ANIM_PROP(Aim)				// kAP_Aim
ADD_ANIM_PROP(Fire)				// kAP_Fire
ADD_ANIM_PROP(FireSecondary)	// kAP_FireSecondary
ADD_ANIM_PROP(Reload)			// kAP_Reload
ADD_ANIM_PROP(Draw)				// kAP_Draw
ADD_ANIM_PROP(Holster)			// kAP_Holster
ADD_ANIM_PROP(Throw)			// kAP_Throw
ADD_ANIM_PROP(Box)				// kAP_Box
ADD_ANIM_PROP(Punch1)			// kAP_Punch1
ADD_ANIM_PROP(Punch2)			// kAP_Punch2
ADD_ANIM_PROP(Punch3)			// kAP_Punch3
ADD_ANIM_PROP(Punch4)			// kAP_Punch4
ADD_ANIM_PROP(Slam)				// kAP_Slam
ADD_ANIM_PROP(Sword1)			// kAP_Sword1
ADD_ANIM_PROP(Sword2)			// kAP_Sword2
ADD_ANIM_PROP(Lower)			// kAP_Lower
ADD_ANIM_PROP(Down)				// kAP_Down
ADD_ANIM_PROP(Up)				// kAP_Up
ADD_ANIM_PROP(Swim)				// kAP_Swim
ADD_ANIM_PROP(Hover)			// kAP_Hover
ADD_ANIM_PROP(Run)				// kAP_Run
ADD_ANIM_PROP(Sprint)		// kAP_Sprint
ADD_ANIM_PROP(Walk)				// kAP_Walk
ADD_ANIM_PROP(Crawl)			// kAP_Crawl
ADD_ANIM_PROP(Pinned)			// kAP_Pinned
ADD_ANIM_PROP(Weapon1)			// kAP_Weapon1
ADD_ANIM_PROP(Weapon2)			// kAP_Weapon2
ADD_ANIM_PROP(Weapon3)			// kAP_Weapon3
ADD_ANIM_PROP(Silent)			// kAP_Silent
ADD_ANIM_PROP(Happy)			// kAP_Happy
ADD_ANIM_PROP(Angry)			// kAP_Angry
ADD_ANIM_PROP(Sad)				// kAP_Sad
ADD_ANIM_PROP(Tense)			// kAP_Tense
ADD_ANIM_PROP(Agree)			// kAP_Agree
ADD_ANIM_PROP(Disagree)			// kAP_Disagree
ADD_ANIM_PROP(CornerLeft)		// kAP_CornerLeft	
ADD_ANIM_PROP(CornerRight)		// kAP_CornerRight
ADD_ANIM_PROP(RollLeft)			// kAP_RollLeft
ADD_ANIM_PROP(RollRight)		// kAP_RollRight
ADD_ANIM_PROP(ShuffleLeft)		// kAP_ShuffleLeft
ADD_ANIM_PROP(ShuffleRight)		// kAP_ShuffleRight
ADD_ANIM_PROP(Blind)			// kAP_Blind
ADD_ANIM_PROP(Popup)			// kAP_Popup
ADD_ANIM_PROP(Block)			// kAP_Block
ADD_ANIM_PROP(Patrol)			// kAP_Patrol
ADD_ANIM_PROP(Distress)			// kAP_Distress
ADD_ANIM_PROP(Investigate)		// kAP_Investigate
ADD_ANIM_PROP(InvestigateDark)	// kAP_InvestigateDark
ADD_ANIM_PROP(Panic)			// kAP_Panic
ADD_ANIM_PROP(Drowsy)			// kAP_Drowsy
ADD_ANIM_PROP(Covered)			// kAP_Covered
ADD_ANIM_PROP(Uncovered)		// kAP_Uncovered
ADD_ANIM_PROP(Covering)			// kAP_Covering
ADD_ANIM_PROP(Uncovering)		// kAP_Uncovering
ADD_ANIM_PROP(CheckPulse)		// kAP_CheckPulse
ADD_ANIM_PROP(KickBody)			// kAP_KickBody
ADD_ANIM_PROP(DisposeBody)		// kAP_DisposeBody
ADD_ANIM_PROP(Clipboard)		// kAP_Clipboard
ADD_ANIM_PROP(Ticket)			// kAP_Ticket
ADD_ANIM_PROP(Dust)				// kAP_Dust
ADD_ANIM_PROP(Sweep)			// kAP_Sweep
ADD_ANIM_PROP(Wipe)				// kAP_Wipe
ADD_ANIM_PROP(OpenDoor)			// kAP_OpenDoor
ADD_ANIM_PROP(KickDoor)			// kAP_KickDoor
ADD_ANIM_PROP(KnockOnDoor)		// kAP_KnockOnDoor
ADD_ANIM_PROP(Pickup)			// kAP_Pickup
ADD_ANIM_PROP(PushButton)		// kAP_PushButton
ADD_ANIM_PROP(Tail)				// kAP_Tail
ADD_ANIM_PROP(Asleep)			// kAP_Asleep
ADD_ANIM_PROP(Awake)			// kAP_Awake
ADD_ANIM_PROP(Unconscious)		// kAP_Unconscious
ADD_ANIM_PROP(Stunned)			// kAP_Stunned
ADD_ANIM_PROP(Enraged)			// kAP_Enraged
ADD_ANIM_PROP(Victory)			// kAP_Victory
ADD_ANIM_PROP(Defeat)			// kAP_Defeat
ADD_ANIM_PROP(Taunting)			// kAP_Taunting
ADD_ANIM_PROP(Alert)			// kAP_Alert
ADD_ANIM_PROP(LookLeft)			// kAP_LookLeft
ADD_ANIM_PROP(LookRight)		// kAP_LookRight
ADD_ANIM_PROP(LookUp)			// kAP_LookUp
ADD_ANIM_PROP(LookUnder)		// kAP_LookUnder
ADD_ANIM_PROP(LookOver)			// kAP_LookOver
ADD_ANIM_PROP(Flashlight)		// kAP_Flashlight
ADD_ANIM_PROP(Sing)				// kAP_Sing
ADD_ANIM_PROP(ClimbUp)			// kAP_ClimbUp
ADD_ANIM_PROP(ClimbDown)		// kAP_ClimbDown
ADD_ANIM_PROP(ClimbUpFast)		// kAP_ClimbUpFast
ADD_ANIM_PROP(ClimbDownFast)	// kAP_ClimbDownFast
ADD_ANIM_PROP(Taunt)			// kAP_Taunt
ADD_ANIM_PROP(LungeStart)		// kAP_LungeStart
ADD_ANIM_PROP(LungeFly)			// kAP_LungeFly
ADD_ANIM_PROP(LungeLand)		// kAP_LungeLand
ADD_ANIM_PROP(RetreatStart)		// kAP_RetreatStart
ADD_ANIM_PROP(RetreatFly)		// kAP_RetreatFly
ADD_ANIM_PROP(RetreatLand)		// kAP_RetreatLand
ADD_ANIM_PROP(Wait)				// kAP_Wait
ADD_ANIM_PROP(Idle)				// kAP_Idle
ADD_ANIM_PROP(LookAround)		// kAP_LookAround
ADD_ANIM_PROP(Alarm)			// kAP_Alarm
ADD_ANIM_PROP(AlarmBox)			// kAP_AlarmBox
ADD_ANIM_PROP(JumpStart)		// kAP_JumpStart
ADD_ANIM_PROP(JumpFly)			// kAP_JumpFly
ADD_ANIM_PROP(JumpLand)			// kAP_JumpLand
ADD_ANIM_PROP(JumpUpStart)		// kAP_JumpUpStart
ADD_ANIM_PROP(JumpUpFly)		// kAP_JumpUpFly
ADD_ANIM_PROP(JumpUpLand)		// kAP_JumpUpLand
ADD_ANIM_PROP(JumpDownStart)	// kAP_JumpDownStart
ADD_ANIM_PROP(JumpDownFly)		// kAP_JumpDownFly
ADD_ANIM_PROP(JumpDownLand)		// kAP_JumpDownLand
ADD_ANIM_PROP(JumpDownLandBad)	// kAP_JumpDownLandBad
ADD_ANIM_PROP(Disappear)		// kAP_Disappear
ADD_ANIM_PROP(Reappear)			// kAP_Reappear
ADD_ANIM_PROP(StartDeflect)		// kAP_StartDeflect
ADD_ANIM_PROP(HoldDeflect)		// kAP_HoldDeflect
ADD_ANIM_PROP(EndDeflect)		// kAP_EndDeflect
ADD_ANIM_PROP(StartCatch)		// kAP_StartCatch
ADD_ANIM_PROP(HoldCatch)		// kAP_HoldCatch
ADD_ANIM_PROP(EndCatch)			// kAP_EndCatch
ADD_ANIM_PROP(Challenge)		// kAP_Challenge
ADD_ANIM_PROP(Mark)				// kAP_Mark
ADD_ANIM_PROP(Fidget)			// kAP_Fidget
ADD_ANIM_PROP(Smoking)			// kAP_Smoking
ADD_ANIM_PROP(DeskWork)			// kAP_DeskWork
ADD_ANIM_PROP(FilingHigh)		// kAP_FilingHigh
ADD_ANIM_PROP(FilingLow)		// kAP_FilingLow
ADD_ANIM_PROP(Peeing)			// kAP_Peeing
ADD_ANIM_PROP(PeeingOutside)	// kAP_PeeingOutside
ADD_ANIM_PROP(ThrowDown)		// kAP_ThrowDown
ADD_ANIM_PROP(CloseDrawer)		// kAP_CloseDrawer
ADD_ANIM_PROP(DeskWrite)		// kAP_DeskWrite
ADD_ANIM_PROP(Drinking)			// kAP_Drinking
ADD_ANIM_PROP(Hammering)		// kAP_Hammering
ADD_ANIM_PROP(Wrenching)		// kAP_Wrenching
ADD_ANIM_PROP(LeaningBack)		// kAP_LeaningBack
ADD_ANIM_PROP(LeaningBar)		// kAP_LeaningBar
ADD_ANIM_PROP(LeaningLeft)		// kAP_LeaningLeft
ADD_ANIM_PROP(LeaningRight)		// kAP_LeaningRight
ADD_ANIM_PROP(LeaningRail)		// kAP_LeaningRail
ADD_ANIM_PROP(Typing)			// kAP_Typing
ADD_ANIM_PROP(Researching)		// kAP_Researching
ADD_ANIM_PROP(Microscoping)		// kAP_Microscoping
ADD_ANIM_PROP(Warming)			// kAP_Warming
ADD_ANIM_PROP(AwarenessTest1)	// kAP_AwarenessTest1
ADD_ANIM_PROP(AwarenessTest2)	// kAP_AwarenessTest2
ADD_ANIM_PROP(AwarenessTest3)	// kAP_AwarenessTest3
ADD_ANIM_PROP(SwitchOn)			// kAP_SwitchOn
ADD_ANIM_PROP(SwitchOff)		// kAP_SwitchOff
ADD_ANIM_PROP(Poster)			// kAP_Poster
ADD_ANIM_PROP(Examining)		// kAP_Examining
ADD_ANIM_PROP(EnterTeleport)	// kAP_EnterTeleport
ADD_ANIM_PROP(ExitTeleport)		// kAP_ExitTeleport
ADD_ANIM_PROP(Resurrecting)		// kAP_Resurrecting
ADD_ANIM_PROP(SmashLeft)		// kAP_SmashLeft
ADD_ANIM_PROP(SmashRight)		// kAP_SmashRight
ADD_ANIM_PROP(PowerDown)		// kAP_PowerDown
ADD_ANIM_PROP(PowerUp)			// kAP_PowerUp
ADD_ANIM_PROP(Discover)			// kAP_Discover
ADD_ANIM_PROP(Attracted)		// kAP_Attracted
ADD_ANIM_PROP(Steaming)			// kAP_Steaming
ADD_ANIM_PROP(Death)			// kAP_Death
ADD_ANIM_PROP(GetOnLadderLeft)	// kAP_GetOnLadderLeft
ADD_ANIM_PROP(GetOnLadderRight)	// kAP_GetOnLadderRight
ADD_ANIM_PROP(GetOffLadderLeft)	// kAP_GetOffLadderLeft
ADD_ANIM_PROP(GetOffLadderRight)// kAP_GetOffLadderRight
ADD_ANIM_PROP(DamageSleeping)	// kAP_DamageSleeping
ADD_ANIM_PROP(DamageStunned)	// kAP_DamageStunned
ADD_ANIM_PROP(DamageTrapped)	// kAP_DamageTrapped
ADD_ANIM_PROP(DamageGlued)		// kAP_DamageGlued
ADD_ANIM_PROP(DamageLaughing)	// kAP_DamageLaughing
ADD_ANIM_PROP(DamageSlipping)	// kAP_DamageSlipping
ADD_ANIM_PROP(DamageBurning)	// kAP_DamageBurning
ADD_ANIM_PROP(DamageBleeding)	// kAP_DamageBleeding
ADD_ANIM_PROP(DamageChoking)	// kAP_DamageChoking
ADD_ANIM_PROP(DamageElectrocuting)	// kAP_DamageElectrocuting
ADD_ANIM_PROP(DamagePoisoning)	// kAP_DamagePoisoning
ADD_ANIM_PROP(Interrupt)		// kAP_Interrupt
ADD_ANIM_PROP(OverRailing)		// kAP_OverRailing
ADD_ANIM_PROP(Operating)		// kAP_Operating
ADD_ANIM_PROP(Phoning)			// kAP_Phoning
ADD_ANIM_PROP(Banging)			// kAP_Banging
ADD_ANIM_PROP(Vending)			// kAP_Vending
ADD_ANIM_PROP(WashingHands)		// kAP_WashingHands
ADD_ANIM_PROP(HidingLeft)		// kAP_HidingLeft
ADD_ANIM_PROP(HidingRight)		// kAP_HidingRight
ADD_ANIM_PROP(Dancing)			// kAP_Dancing
ADD_ANIM_PROP(Entertaining)		// kAP_Entertaining
ADD_ANIM_PROP(FlipTable)		// kAP_FlipTable
ADD_ANIM_PROP(FlipDesk)			// kAP_FlipDesk
ADD_ANIM_PROP(KnockOver)		// kAP_KnockOver
ADD_ANIM_PROP(Phone)			// kAP_Phone
ADD_ANIM_PROP(Menacing)			// kAP_Menacing
ADD_ANIM_PROP(Admiring)			// kAP_Admiring
ADD_ANIM_PROP(BackUp)			// kAP_BackUp
ADD_ANIM_PROP(FlankRight)		// kAP_FlankRight
ADD_ANIM_PROP(FlankLeft)		// kAP_FlankLeft
ADD_ANIM_PROP(CatchBreath)		// kAP_CatchBreath
ADD_ANIM_PROP(Browsing)			// kAP_Browsing
ADD_ANIM_PROP(Struggling)		// kAP_Struggling
ADD_ANIM_PROP(Operating2)		// kAP_Operating2
ADD_ANIM_PROP(DrinkFountain)	// kAP_DrinkFountain

ADD_ANIM_PROP(FireStreamStart)			//  kAP_FireStreamStart
ADD_ANIM_PROP(FireSecondaryStreamStart)	//  kAP_FireSecondaryStreamStart
ADD_ANIM_PROP(FireStream)				//  kAP_FireStream
ADD_ANIM_PROP(FireSecondaryStream)		//  kAP_FireSecondaryStream
ADD_ANIM_PROP(FireStreamEnd)			//  kAP_FireStreamEnd
ADD_ANIM_PROP(FireSecondaryStreamEnd)	//  kAP_FireSecondaryStreamEnd
