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
	#define ADD_ANIM_PROP(label) kAP_##label,
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

// Posture.

ADD_ANIM_PROP(POS_Crouch)			// kAP_POS_Crouch
ADD_ANIM_PROP(POS_Grounded)			// kAP_POS_Grounded
ADD_ANIM_PROP(POS_Mounted)			// kAP_POS_Mounted
ADD_ANIM_PROP(POS_Prone)			// kAP_POS_Prone
ADD_ANIM_PROP(POS_Sit)				// kAP_POS_Sit
ADD_ANIM_PROP(POS_Stand)			// kAP_POS_Stand
ADD_ANIM_PROP(POS_StandAimed)		// kAP_POS_StandAimed

// WeaponPosition.

ADD_ANIM_PROP(WPOS_Down)			// kAP_WPOS_Down
ADD_ANIM_PROP(WPOS_Lower)			// kAP_WPOS_Lower
ADD_ANIM_PROP(WPOS_Up)				// kAP_WPOS_Up

// WeaponHand.

ADD_ANIM_PROP(WHAND_Right)			// kAP_WHAND_Right
ADD_ANIM_PROP(WHAND_Left)			// kAP_WHAND_Left

// Weapon.

ADD_ANIM_PROP(WEAP_AssaultRifle)	// kAP_WEAP_AssaultRifle
ADD_ANIM_PROP(WEAP_Cannon)			// kAP_WEAP_Cannon
ADD_ANIM_PROP(WEAP_DualPistols)		// kAP_WEAP_DualPistols
ADD_ANIM_PROP(WEAP_Hands)			// kAP_WEAP_Hands
ADD_ANIM_PROP(WEAP_Melee)			// kAP_WEAP_Melee
ADD_ANIM_PROP(WEAP_MissileLauncher)	// kAP_WEAP_MissileLauncher
ADD_ANIM_PROP(WEAP_NailGun)			// kAP_WEAP_NailGun
ADD_ANIM_PROP(WEAP_Pistol)			// kAP_WEAP_Pistol
ADD_ANIM_PROP(WEAP_PlasmaGun)		// kAP_WEAP_PlasmaGun
ADD_ANIM_PROP(WEAP_Rifle)			// kAP_WEAP_Rifle
ADD_ANIM_PROP(WEAP_SemiAutoRifle)	// kAP_WEAP_SemiAutoRifle
ADD_ANIM_PROP(WEAP_Shotgun)			// kAP_WEAP_Shotgun
ADD_ANIM_PROP(WEAP_SubMachinegun)	// kAP_WEAP_SubMachinegun
ADD_ANIM_PROP(WEAP_Turret)			// kAP_WEAP_Turret
ADD_ANIM_PROP(WEAP_Remote)			// kAP_WEAP_Remote

// "DARK" Weapons.

ADD_ANIM_PROP(WEAP_45)							// kAP_WEAP_45
ADD_ANIM_PROP(WEAP_Revolver)					// kAP_WEAP_Revolver
//ADD_ANIM_PROP(WEAP_Rifle)						// kAP_WEAP_Rifle
//ADD_ANIM_PROP(WEAP_Shotgun)						// kAP_WEAP_Shotgun
ADD_ANIM_PROP(WEAP_DoubleBarrelShotgun)			// kAP_WEAP_DoubleBarrelShotgun
ADD_ANIM_PROP(WEAP_SubGun)						// kAP_WEAP_SubGun
ADD_ANIM_PROP(WEAP_45_Melee)					// kAP_WEAP_45_Melee
ADD_ANIM_PROP(WEAP_Revolver_Melee)				// kAP_WEAP_Revolver_Melee
ADD_ANIM_PROP(WEAP_Rifle_Melee)					// kAP_WEAP_Rifle
ADD_ANIM_PROP(WEAP_Shotgun_Melee)				// kAP_WEAP_Shotgun_Melee
ADD_ANIM_PROP(WEAP_DoubleBarrelShotgun_Melee)	// kAP_WEAP_DoubleBarrelShotgun_Melee
ADD_ANIM_PROP(WEAP_SubGun_Melee)				// kAP_WEAP_SubGun_Melee
ADD_ANIM_PROP(WEAP_1HandedDebris)				// kAP_WEAP_1HandedDebris
ADD_ANIM_PROP(WEAP_2HandedDebris)				// kAP_WEAP_2HandedDebris
ADD_ANIM_PROP(WEAP_Crowbar)						// kAP_WEAP_Crowbar
ADD_ANIM_PROP(WEAP_FireAxe)						// kAP_WEAP_FireAxe
ADD_ANIM_PROP(WEAP_Shovel)						// kAP_WEAP_Shovel
ADD_ANIM_PROP(WEAP_SledgeHammer)				// kAP_WEAP_SledgeHammer
ADD_ANIM_PROP(WEAP_FireExtinguisher)			// kAP_WEAP_FireExtinguisher
ADD_ANIM_PROP(WEAP_DoubleShotgun)			// kAP_WEAP_DoubleShotgun
ADD_ANIM_PROP(WEAP_PumpShotgun)			// kAP_WEAP_PumpShotgun
ADD_ANIM_PROP(WEAP_G00)				// kAP_WEAP_G00
ADD_ANIM_PROP(WEAP_G01)				// kAP_WEAP_G01
ADD_ANIM_PROP(WEAP_G02)				// kAP_WEAP_G02
ADD_ANIM_PROP(WEAP_G03)				// kAP_WEAP_G03
ADD_ANIM_PROP(WEAP_G04)				// kAP_WEAP_G04
ADD_ANIM_PROP(WEAP_G05)				// kAP_WEAP_G05
ADD_ANIM_PROP(WEAP_G06)				// kAP_WEAP_G06
ADD_ANIM_PROP(WEAP_G07)				// kAP_WEAP_G07
ADD_ANIM_PROP(WEAP_G08)				// kAP_WEAP_G08
ADD_ANIM_PROP(WEAP_G09)				// kAP_WEAP_G09
ADD_ANIM_PROP(WEAP_Taser)			// kAP_WEAP_Taser
ADD_ANIM_PROP(WEAP_Taser_Upgrade)	// kAP_WEAP_Taser_Upgrade
ADD_ANIM_PROP(WEAP_Sampler)			// kAP_WEAP_Sampler
ADD_ANIM_PROP(WEAP_Camera)			// kAP_WEAP_Camera
ADD_ANIM_PROP(WEAP_Scanner)			// kAP_WEAP_Scanner
ADD_ANIM_PROP(WEAP_CollectionBase)	// kAP_WEAP_CollectionBase
ADD_ANIM_PROP(WEAP_Laser)			// kAP_WEAP_Laser
ADD_ANIM_PROP(WEAP_Blacklight)		// kAP_WEAP_Blacklight
ADD_ANIM_PROP(WEAP_Spectrometer)	// kAP_WEAP_Spectrometer

// "DARK" Forensics.

ADD_ANIM_PROP(WEAP_Laser_Scanner)				// kAP_WEAP_Laser_Scanner
//ADD_ANIM_PROP(WEAP_Laser)						// kAP_WEAP_Laser
//ADD_ANIM_PROP(WEAP_Spectrometer)				// kAP_WEAP_Spectrometer
//ADD_ANIM_PROP(WEAP_Camera)						// kAP_WEAP_Camera
//ADD_ANIM_PROP(WEAP_Scanner)						// kAP_WEAP_Scanner
//ADD_ANIM_PROP(WEAP_Sampler)						// kAP_WEAP_Sampler

// Movement.

ADD_ANIM_PROP(MOV_Fall)				// kAP_MOV_Fall
ADD_ANIM_PROP(MOV_Idle)				// kAP_MOV_Idle
ADD_ANIM_PROP(MOV_Jump)				// kAP_MOV_Jump
ADD_ANIM_PROP(MOV_Land)				// kAP_MOV_Land
ADD_ANIM_PROP(MOV_Run)				// kAP_MOV_Run
ADD_ANIM_PROP(MOV_Stumble)			// kAP_MOV_Stumble
ADD_ANIM_PROP(MOV_Swim)				// kAP_MOV_Swim
ADD_ANIM_PROP(MOV_Walk)				// kAP_MOV_Walk

// Movement Direction.

ADD_ANIM_PROP(MDIR_BackLeft)		// kAP_MDIR_BackLeft
ADD_ANIM_PROP(MDIR_BackRight)		// kAP_MDIR_BackRight
ADD_ANIM_PROP(MDIR_Backward)		// kAP_MDIR_Backward
ADD_ANIM_PROP(MDIR_Down)			// kAP_MDIR_Down
ADD_ANIM_PROP(MDIR_ForeLeft)		// kAP_MDIR_ForeLeft
ADD_ANIM_PROP(MDIR_ForeRight)		// kAP_MDIR_ForeRight
ADD_ANIM_PROP(MDIR_Forward)			// kAP_MDIR_Forward
ADD_ANIM_PROP(MDIR_High)			// kAP_MDIR_High
ADD_ANIM_PROP(MDIR_Left)			// kAP_MDIR_Left
ADD_ANIM_PROP(MDIR_Low)				// kAP_MDIR_Low
ADD_ANIM_PROP(MDIR_Right)			// kAP_MDIR_Right 
ADD_ANIM_PROP(MDIR_ShuffleLeft)		// kAP_MDIR_ShuffleLeft
ADD_ANIM_PROP(MDIR_ShuffleRight)	// kAP_MDIR_ShuffleRight
ADD_ANIM_PROP(MDIR_Up)				// kAP_MDIR_Up

// Damage.

ADD_ANIM_PROP(DMG_Bleeding)			// kAP_DMG_Bleeding
ADD_ANIM_PROP(DMG_Bullet)			// kAP_DMG_Bullet
ADD_ANIM_PROP(DMG_Burning)			// kAP_DMG_Burning
ADD_ANIM_PROP(DMG_Choking)			// kAP_DMG_Choking
ADD_ANIM_PROP(DMG_Electrocuting)	// kAP_DMG_Electrocuting
ADD_ANIM_PROP(DMG_Explode)			// kAP_DMG_Explode
ADD_ANIM_PROP(DMG_Glued)			// kAP_DMG_Glued
ADD_ANIM_PROP(DMG_Laughing)			// kAP_DMG_Laughing
ADD_ANIM_PROP(DMG_Melee)			// kAP_DMG_Melee
ADD_ANIM_PROP(DMG_Poisoning)		// kAP_DMG_Poisoning
ADD_ANIM_PROP(DMG_Sleeping)			// kAP_DMG_Sleeping
ADD_ANIM_PROP(DMG_Slipping)			// kAP_DMG_Slipping
ADD_ANIM_PROP(DMG_Stunned)			// kAP_DMG_Stunned
ADD_ANIM_PROP(DMG_Taser)			// kAP_DMG_Taser
ADD_ANIM_PROP(DMG_Taser_Upgrade)	// kAP_DMG_Taser_Upgrade
ADD_ANIM_PROP(DMG_Trapped)			// kAP_DMG_Trapped

// Damage Direction.

ADD_ANIM_PROP(DDIR_Back)			// kAP_DDIR_Back
ADD_ANIM_PROP(DDIR_Front)			// kAP_DDIR_Front
ADD_ANIM_PROP(DDIR_Left)			// kAP_DDIR_Left
ADD_ANIM_PROP(DDIR_Right)			// kAP_DDIR_Right

// Body Part.

ADD_ANIM_PROP(BODY_Head)			// kAP_BODY_Head
ADD_ANIM_PROP(BODY_LeftArm)			// kAP_BODY_LeftArm
ADD_ANIM_PROP(BODY_LeftLeg)			// kAP_BODY_LeftLeg
ADD_ANIM_PROP(BODY_RightArm)		// kAP_BODY_RightArm
ADD_ANIM_PROP(BODY_RightLeg)		// kAP_BODY_RightLeg
ADD_ANIM_PROP(BODY_Torso)			// kAP_BODY_Torso

// Activity.

ADD_ANIM_PROP(ATVT_Ascending)		// kAP_ATVT_Ascending
ADD_ANIM_PROP(ATVT_Admiring)		// kAP_ATVT_Admiring
ADD_ANIM_PROP(ATVT_Advancing)		// kAP_ATVT_Advancing
ADD_ANIM_PROP(ATVT_Ambushing)		// kAP_ATVT_Ambushing
ADD_ANIM_PROP(ATVT_BangDoor)		// kAP_ATVT_BangDoor
ADD_ANIM_PROP(ATVT_Berserking)		// kAP_ATVT_Berserking
ADD_ANIM_PROP(ATVT_Blitz)			// kAP_ATVT_Blitz
ADD_ANIM_PROP(ATVT_Blind)			// kAP_ATVT_Blind
ADD_ANIM_PROP(ATVT_CarDriver)		// kAP_ATVT_CarDriver
ADD_ANIM_PROP(ATVT_Chanting)		// kAP_ATVT_Chanting
ADD_ANIM_PROP(ATVT_Climb)			// kAP_ATVT_Climb
ADD_ANIM_PROP(ATVT_Covered)			// kAP_ATVT_Covered
ADD_ANIM_PROP(ATVT_Covering)		// kAP_ATVT_Covering
ADD_ANIM_PROP(ATVT_Crawl)			// kAP_ATVT_Crawl
ADD_ANIM_PROP(ATVT_Descending)		// kAP_ATVT_Descending
ADD_ANIM_PROP(ATVT_Distress)		// kAP_ATVT_Distress
ADD_ANIM_PROP(ATVT_Ducking)			// kAP_ATVT_Ducking
ADD_ANIM_PROP(ATVT_FastRope)		// kAP_ATVT_FastRope
ADD_ANIM_PROP(ATVT_Following)		// kAP_ATVT_Following
ADD_ANIM_PROP(ATVT_Hiding)			// kAP_ATVT_Hiding
ADD_ANIM_PROP(ATVT_Hurrying)		// kAP_ATVT_Hurrying
ADD_ANIM_PROP(ATVT_Investigate)		// kAP_ATVT_Investigate
ADD_ANIM_PROP(ATVT_KnockDown)		// kAP_ATVT_KnockDown
ADD_ANIM_PROP(ATVT_LeaningBack)		// kAP_ATVT_LeaningBack
ADD_ANIM_PROP(ATVT_LeaningBar)		// kAP_ATVT_LeaningBar
ADD_ANIM_PROP(ATVT_LeaningLeft)		// kAP_ATVT_LeaningLeft
ADD_ANIM_PROP(ATVT_LeaningRail)		// kAP_ATVT_LeaningRail
ADD_ANIM_PROP(ATVT_LeaningRight)	// kAP_ATVT_LeaningRight
ADD_ANIM_PROP(ATVT_Leap)			// kAP_ATVT_Leap
ADD_ANIM_PROP(ATVT_LeapNarrow)		// kAP_ATVT_LeapNarrow
ADD_ANIM_PROP(ATVT_Limping)			// kAP_ATVT_Limping
ADD_ANIM_PROP(ATVT_Loping)			// kAP_ATVT_Loping
ADD_ANIM_PROP(ATVT_Lunging)			// kAP_ATVT_Lunging
ADD_ANIM_PROP(ATVT_Menacing)		// kAP_ATVT_Menacing
ADD_ANIM_PROP(ATVT_MeatMobile)		// kAP_ATVT_MeatMobile
ADD_ANIM_PROP(ATVT_Motorcycle)		// kAP_ATVT_Motorcycle
ADD_ANIM_PROP(ATVT_Operating)		// kAP_ATVT_Operating
ADD_ANIM_PROP(ATVT_OverDesk)		// kAP_ATVT_OverDesk
ADD_ANIM_PROP(ATVT_OverRailing)		// kAP_ATVT_OverRailing
ADD_ANIM_PROP(ATVT_Panic)			// kAP_ATVT_Panic
ADD_ANIM_PROP(ATVT_Patrolling)		// kAP_ATVT_Patrolling
ADD_ANIM_PROP(ATVT_Safety)			// kAP_ATVT_Safety
ADD_ANIM_PROP(ATVT_Searching)		// kAP_ATVT_Searching
ADD_ANIM_PROP(ATVT_Smoking)			// kAP_ATVT_Smoking
ADD_ANIM_PROP(ATVT_StepAside)		// kAP_ATVT_StepAside
ADD_ANIM_PROP(ATVT_StepStairs)		// kAP_ATVT_StepStairs
ADD_ANIM_PROP(ATVT_SurpriseAttackRight)	// kAP_ATVT_SurpriseAttackRight
ADD_ANIM_PROP(ATVT_SurpriseAttackLeft)	// kAP_ATVT_SurpriseAttackLeft
ADD_ANIM_PROP(ATVT_ThruVent)		// kAP_ATVT_ThruVent
ADD_ANIM_PROP(ATVT_TrainLeft)		// kAP_ATVT_TrainLeft
ADD_ANIM_PROP(ATVT_TrainRight)		// kAP_ATVT_TrainRight
ADD_ANIM_PROP(ATVT_TrainTop)		// kAP_ATVT_TrainTop
ADD_ANIM_PROP(ATVT_Uncovered)		// kAP_ATVT_Uncovered
ADD_ANIM_PROP(ATVT_Uncovering)		// kAP_ATVT_Uncovering

// Action.

ADD_ANIM_PROP(ACT_Aim)				// kAP_ACT_Aim
ADD_ANIM_PROP(ACT_Alarm)			// kAP_ACT_Alarm
ADD_ANIM_PROP(ACT_Alert)			// kAP_ACT_Alert
ADD_ANIM_PROP(ACT_Asleep)			// kAP_ACT_Asleep
ADD_ANIM_PROP(ACT_AttackBerserker)	// kAP_ACT_AttackBerserker
ADD_ANIM_PROP(ACT_AttackMelee)		// kAP_ACT_AttackMelee
ADD_ANIM_PROP(ACT_AttackMeleeFromDistance) // kAP_ACT_AttackMeleeFromDistance
ADD_ANIM_PROP(ACT_AttackMeleeFromRun) // kAP_ACT_AttackMeleeFromRun
ADD_ANIM_PROP(ACT_Awake)			// kAP_ACT_Awake
ADD_ANIM_PROP(ACT_Backpedal)		// kAP_ACT_Backpedal 
ADD_ANIM_PROP(ACT_Block)			// kAP_ACT_Block
ADD_ANIM_PROP(ACT_BlockOutAttack)	// kAP_ACT_BlockOutAttack
ADD_ANIM_PROP(ACT_BlockOutNormal)	// kAP_ACT_BlockOutNormal
ADD_ANIM_PROP(ACT_Carrying)			// kAP_ACT_Carrying
ADD_ANIM_PROP(ACT_CatchBreath)		// kAP_ACT_CatchBreath
ADD_ANIM_PROP(ACT_ChangingPrimaryFrom45)			// kAP_ACT_ChangingPrimaryFrom45
ADD_ANIM_PROP(ACT_ChangingPrimaryFromDoubleShotgun)	// kAP_ACT_ChangingPrimaryFromDoubleShotgun
ADD_ANIM_PROP(ACT_ChangingPrimaryFromRevolver)		// kAP_ACT_ChangingPrimaryFromRevolver
ADD_ANIM_PROP(ACT_ChangingPrimaryFromRifle)			// kAP_ACT_ChangingPrimaryFromRifle
ADD_ANIM_PROP(ACT_ChangingPrimaryFromPumpShotgun)	// kAP_ACT_ChangingPrimaryFromPumpShotgun
ADD_ANIM_PROP(ACT_ChangingPrimaryFromSubGun)		// kAP_ACT_ChangingPrimaryFromSubGun
ADD_ANIM_PROP(ACT_ChangingPrimaryMelee)	// kAP_ACT_ChangingPrimaryMelee
ADD_ANIM_PROP(ACT_ChangingPrimaryRanged)// kAP_ACT_ChangingPrimaryRanged
ADD_ANIM_PROP(ACT_CheckAmmo)		// kAP_ACT_CheckAmmo
ADD_ANIM_PROP(ACT_CheckPulse)		// kAP_ACT_CheckPulse
ADD_ANIM_PROP(ACT_ClearSurface)		// ACT_ClearSurface
ADD_ANIM_PROP(ACT_CrouchIdleKick)	// kAP_ACT_CrouchIdleKick
ADD_ANIM_PROP(ACT_Death)			// kAP_ACT_Death
ADD_ANIM_PROP(ACT_DefeatedRecoil)	// kAP_ACT_DefeatedRecoil
ADD_ANIM_PROP(ACT_Deselect)			// kAP_ACT_Deselect
ADD_ANIM_PROP(ACT_Discover)			// kAP_ACT_Discover
ADD_ANIM_PROP(ACT_Dismount)			// kAP_ACT_Dismount
ADD_ANIM_PROP(ACT_DismountAggressive)	// ACT_ACT_DismountAggressive
ADD_ANIM_PROP(ACT_DiveFly)			// kAP_ACT_DiveFly
ADD_ANIM_PROP(ACT_Dodge)			// kAP_ACT_Dodge
ADD_ANIM_PROP(ACT_Draw)				// kAP_ACT_Draw
ADD_ANIM_PROP(ACT_DropWeapon)		// kAP_ACT_DropWeapon
ADD_ANIM_PROP(ACT_DuckUnder)		// kAP_ACT_DuckUnder
ADD_ANIM_PROP(ACT_DumpsterJump)		// kAP_ACT_DumpsterJump
ADD_ANIM_PROP(ACT_Fidget)			// kAP_ACT_Fidget
ADD_ANIM_PROP(ACT_FinishingMove)	// kAP_ACT_FinishingMove
ADD_ANIM_PROP(ACT_Fire)				// kAP_ACT_Fire
ADD_ANIM_PROP(ACT_FireNode)			// kAP_ACT_FireNode
ADD_ANIM_PROP(ACT_FireSecondary)	// kAP_ACT_FireSecondary
ADD_ANIM_PROP(ACT_Flashlight)		// kAP_ACT_Flashlight
ADD_ANIM_PROP(ACT_FlipDesk)			// kAP_ACT_FlipDesk
ADD_ANIM_PROP(ACT_FlipTable)		// kAP_ACT_FlipTable
ADD_ANIM_PROP(ACT_GetOffLadder)		// kAP_ACT_GetOffLadder
ADD_ANIM_PROP(ACT_GetOffLadderTopLeft)	// kAP_ACT_GetOffLadderTopLeft
ADD_ANIM_PROP(ACT_GetOffLadderTopRight)	// kAP_ACT_GetOffLadderTopRight
ADD_ANIM_PROP(ACT_GetOnLadder)		// kAP_ACT_GetOnLadder
ADD_ANIM_PROP(ACT_GetOnLadderTop)	// kAP_ACT_GetOnLadderTop
ADD_ANIM_PROP(ACT_HandSignal)		// kAP_ACT_HandSignal
ADD_ANIM_PROP(ACT_Holster)			// kAP_ACT_Holster
ADD_ANIM_PROP(ACT_Idle)				// kAP_ACT_Idle
ADD_ANIM_PROP(ACT_Interrupt)		// kAP_ACT_Interrupt
ADD_ANIM_PROP(ACT_JumpBackFire)		// kAP_ACT_JumpBackFire
ADD_ANIM_PROP(ACT_JumpBox)			// kAP_ACT_JumpBox
ADD_ANIM_PROP(ACT_JumpDown)			// kAP_ACT_JumpDown
ADD_ANIM_PROP(ACT_JumpDown150)		// kAP_ACT_JumpDown150
ADD_ANIM_PROP(ACT_JumpDown380)		// kAP_ACT_JumpDown380
ADD_ANIM_PROP(ACT_JumpIdleKick)		// kAP_ACT_JumpIdleKick
ADD_ANIM_PROP(ACT_JumpOver)			// kAP_ACT_JumpOver
ADD_ANIM_PROP(ACT_JumpRunKick)		// kAP_ACT_JumpRunKick
ADD_ANIM_PROP(ACT_JumpStrafeLeftFire) // kAP_ACT_JumpStrafeLeftFire
ADD_ANIM_PROP(ACT_JumpStrafeRightFire) // kAP_ACT_JumpStrafeRightFire
ADD_ANIM_PROP(ACT_KickBody)			// kAP_ACT_KickBody
ADD_ANIM_PROP(ACT_KickDoor)			// kAP_ACT_KickDoor
ADD_ANIM_PROP(ACT_KnockOnDoor)		// kAP_ACT_KnockOnDoor
ADD_ANIM_PROP(ACT_KnockOver)		// kAP_ACT_KnockOver
ADD_ANIM_PROP(ACT_LadderHoldLeft)	// kAP_ACT_LadderHoldLeft
ADD_ANIM_PROP(ACT_LadderHoldRight)	// kAP_ACT_LadderHoldRight
ADD_ANIM_PROP(ACT_LadderUpLeft)		// kAP_ACT_LadderUpLeft
ADD_ANIM_PROP(ACT_LadderUpRight)	// kAP_ACT_LadderUpRight
ADD_ANIM_PROP(ACT_LongRecoil)		// kAP_ACT_LongRecoil
ADD_ANIM_PROP(ACT_LookAround)		// kAP_ACT_LookAround
ADD_ANIM_PROP(ACT_LookLeft)			// kAP_ACT_LookLeft
ADD_ANIM_PROP(ACT_LookOver)			// kAP_ACT_LookOver
ADD_ANIM_PROP(ACT_LookRight)		// kAP_ACT_LookRight
ADD_ANIM_PROP(ACT_LookUnder)		// kAP_ACT_LookUnder
ADD_ANIM_PROP(ACT_LookUp)			// kAP_ACT_LookUp
ADD_ANIM_PROP(ACT_LowRunKick)		// kAP_ACT_LowRunKick
ADD_ANIM_PROP(ACT_MeleeBlocked)		// kAP_ACT_MeleeBlocked
ADD_ANIM_PROP(ACT_Mount)			// kAP_ACT_Mount
ADD_ANIM_PROP(ACT_OpenDoor)			// kAP_ACT_OpenDoor
ADD_ANIM_PROP(ACT_Pickup)			// kAP_ACT_Pickup
ADD_ANIM_PROP(ACT_PickupFromGround)	// kAP_ACT_PickupFromGround
ADD_ANIM_PROP(ACT_PickupExaggerated)// kAP_ACT_PickupExaggerated
ADD_ANIM_PROP(ACT_PostFire)			// kAP_ACT_PostFire
ADD_ANIM_PROP(ACT_PreFire)			// kAP_ACT_PreFire
ADD_ANIM_PROP(ACT_PushButton)		// kAP_ACT_PushButton
ADD_ANIM_PROP(ACT_Reload)			// kAP_ACT_Reload
ADD_ANIM_PROP(ACT_Roll)				// kAP_ACT_Roll
ADD_ANIM_PROP(ACT_Select)			// kAP_ACT_Select
ADD_ANIM_PROP(ACT_ShortRecoil)		// kAP_ACT_ShortRecoil
ADD_ANIM_PROP(ACT_ShortRecoilRecovery)	// kAP_ACT_ShortRecoilRecovery
ADD_ANIM_PROP(ACT_Shove)			// kAP_ACT_Shove
ADD_ANIM_PROP(ACT_Shoved)			// kAP_ACT_Shoved
ADD_ANIM_PROP(ACT_Shuffle)			// kAP_ACT_Shuffle
ADD_ANIM_PROP(ACT_Slide)			// kAP_ACT_Slide
ADD_ANIM_PROP(ACT_SlideDownLadder)	// kAP_ACT_SlideDownLadder
ADD_ANIM_PROP(ACT_SlideKick)		// kAP_ACT_SlideKick
ADD_ANIM_PROP(ACT_StoryMode)		// kAP_ACT_StoryMode
ADD_ANIM_PROP(ACT_Stunned)			// kAP_ACT_Stunned
ADD_ANIM_PROP(ACT_SurpriseAttackNear)	// kAP_ACT_SurpriseAttackNear
ADD_ANIM_PROP(ACT_SurpriseAttackFar)	// kAP_ACT_SurpriseAttackFar
ADD_ANIM_PROP(ACT_Surprised)		// kAP_ACT_Surprised
ADD_ANIM_PROP(ACT_SwitchOff)		// kAP_ACT_SwitchOff
ADD_ANIM_PROP(ACT_SwitchOn)			// kAP_ACT_SwitchOn
ADD_ANIM_PROP(ACT_Talk)				// kAP_ACT_Talk
ADD_ANIM_PROP(ACT_Throw)			// kAP_ACT_Throw
ADD_ANIM_PROP(ACT_ThrowDown)		// kAP_ACT_ThrowDown
ADD_ANIM_PROP(ACT_ThrowHold)		// kAP_ACT_ThrowHold
ADD_ANIM_PROP(ACT_ThrowStart)		// kAP_ACT_ThrowStart
ADD_ANIM_PROP(ACT_Topple)			// kAP_ACT_Topple
ADD_ANIM_PROP(ACT_Unconscious)		// kAP_ACT_Unconscious
ADD_ANIM_PROP(ACT_WeaponBroke)			// kAP_ACT_WeaponBroke
ADD_ANIM_PROP(ACT_WeaponGrabbed)	// kAP_ACT_WeaponGrabbed
ADD_ANIM_PROP(ACT_WindowJump)		// kAP_ACT_WindowJump

// LookContext

ADD_ANIM_PROP(LOOK_High)			// kAP_LOOK_High
ADD_ANIM_PROP(LOOK_Low)				// kAP_LOOK_Low
ADD_ANIM_PROP(LOOK_Mid)				// kAP_LOOK_Mid

// Lean

ADD_ANIM_PROP(LEAN_Left)			// kAP_LEAN_Left
ADD_ANIM_PROP(LEAN_Right)			// kAP_LEAN_Right

// SonicType

ADD_ANIM_PROP(SONIC_Blast)			// kAP_SONIC_Blast
ADD_ANIM_PROP(SONIC_Blast2)			// kAP_SONIC_Blast2
ADD_ANIM_PROP(SONIC_Guide)			// kAP_SONIC_Guide
ADD_ANIM_PROP(SONIC_Guide2)			// kAP_SONIC_Guide2
ADD_ANIM_PROP(SONIC_Alter)			// kAP_SONIC_Alter
ADD_ANIM_PROP(SONIC_Alter2)			// kAP_SONIC_Alter2
ADD_ANIM_PROP(SONIC_Disabled)		// kAP_SONIC_Disabled

// SpecialFire subtypes

//!!ARL: Now this this is the only prop, we could probably remove this group.
ADD_ANIM_PROP(SF_SpecialAim)		// kAP_SF_SpecialAim

ADD_ANIM_PROP(CLIENT_Local)			// kAP_CLIENT_Local
ADD_ANIM_PROP(CLIENT_NonLocal)		// kAP_CLIENT_NonLocal

// ConditionalAnimation

ADD_ANIM_PROP(CA_TaserSelect)		// kAP_CA_TaserSelect
ADD_ANIM_PROP(CA_TaserAim)			// kAP_CA_TaserAim
ADD_ANIM_PROP(CA_TaserDeselect)		// kAP_CA_TaserDeselect
ADD_ANIM_PROP(CA_BerserkerDance)	// kAP_CA_BerserkerDance
ADD_ANIM_PROP(CA_BerserkerKick)		// kAP_CA_BerserkerKick
ADD_ANIM_PROP(CA_Swing)				// kAP_CA_Swing
ADD_ANIM_PROP(CA_Block)				// kAP_CA_Block
ADD_ANIM_PROP(CA_FinishingMove)		// kAP_CA_FinishingMove
ADD_ANIM_PROP(CA_FinishingKick)		// kAP_CA_FinishingKick
ADD_ANIM_PROP(CA_Laser)				// kAP_CA_Laser
ADD_ANIM_PROP(CA_Spectrometer)		// kAP_CA_Spectrometer
ADD_ANIM_PROP(CA_BlackLight)		// kAP_CA_BlackLight

// ConditionalSubAction

ADD_ANIM_PROP(CSA_Begin)			// kAP_CSA_Begin
//...
ADD_ANIM_PROP(CSA_Pre)				// kAP_CSA_Pre
ADD_ANIM_PROP(CSA_Main)				// kAP_CSA_Main
ADD_ANIM_PROP(CSA_Post)				// kAP_CSA_Post
//...
ADD_ANIM_PROP(CSA_End)				// kAP_CSA_End

