// ----------------------------------------------------------------------- //
//
// MODULE  : AISoundTypeEnums.h
//
// PURPOSE : Enums and string constants for AISounds.
//
// CREATED : 7/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_AISOUND_TYPE
	#undef ADD_AISOUND_TYPE
#endif
 
#if AISOUND_TYPE_AS_ENUM
	#define ADD_AISOUND_TYPE(label) kAIS_##label,
#elif AISOUND_TYPE_AS_STRING
	#define ADD_AISOUND_TYPE(label) #label,
#else
	#error ! To use this include file, first define either AISOUND_TYPE_AS_ENUM or AISOUND_TYPE_AS_STRING, to include the AISound types as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_AISOUND_TYPE(x, val) 
// where x is the name of the enum without the "kAIS" prefix.
// --------------------------------------------------------------------------

ADD_AISOUND_TYPE(None)						// kAIS_None
ADD_AISOUND_TYPE(Advancing)					// kAIS_Advancing
ADD_AISOUND_TYPE(Affirmative)				// kAIS_Affirmative
ADD_AISOUND_TYPE(Attack)					// kAIS_Attack
ADD_AISOUND_TYPE(BackupUrgent)				// kAIS_BackupUrgent
ADD_AISOUND_TYPE(Burning)					// kAIS_Burning
ADD_AISOUND_TYPE(Chase)						// kAIS_Chase
ADD_AISOUND_TYPE(CheckIn)					// kAIS_CheckIn
ADD_AISOUND_TYPE(CombatOpAcetyleneTank)		// kAIS_CombatOpAcetyleneTank
ADD_AISOUND_TYPE(CombatOpBarrel)			// kAIS_CombatOpBarrel
ADD_AISOUND_TYPE(CombatOpCanister)			// kAIS_CombatOpCanister
ADD_AISOUND_TYPE(CombatOpExtinguisher)		// kAIS_CombatOpExtinguisher
ADD_AISOUND_TYPE(CombatOpPowerBox)			// kAIS_CombatOpPowerBox
ADD_AISOUND_TYPE(CombatOpSupports)			// kAIS_CombatOpSupports
ADD_AISOUND_TYPE(CombatOpTank)				// kAIS_CombatOpTank
ADD_AISOUND_TYPE(CombatOpValve)				// kAIS_CombatOpValve
ADD_AISOUND_TYPE(Copy)						// kAIS_Copy
ADD_AISOUND_TYPE(Crush)						// kAIS_Crush
ADD_AISOUND_TYPE(DamageStun)				// kAIS_DamageStun
ADD_AISOUND_TYPE(Danger)					// kAIS_Danger
ADD_AISOUND_TYPE(DangerFire)				// kAIS_DangerFire
ADD_AISOUND_TYPE(Death)						// kAIS_Death
ADD_AISOUND_TYPE(DeathQuiet)				// kAIS_DeathQuiet
ADD_AISOUND_TYPE(DeathUnderWater)			// kAIS_DeathUnderWater
ADD_AISOUND_TYPE(DisturbanceHeardAlarming)	// kAIS_DisturbanceHeardAlarming
ADD_AISOUND_TYPE(DisturbanceSeenAlarming)	// kAIS_DisturbanceSeenAlarming
ADD_AISOUND_TYPE(DisturbanceSeenAlarmingFar)// kAIS_DisturbanceSeenAlarmingFar
ADD_AISOUND_TYPE(DisturbanceSeenFlashlight)	// kAIS_DisturbanceSeenFlashlight
ADD_AISOUND_TYPE(DoYouSeeHim)				// kAIS_DoYouSeeHim
ADD_AISOUND_TYPE(Explode)					// kAIS_Explode
ADD_AISOUND_TYPE(Fall)						// kAIS_Fall
ADD_AISOUND_TYPE(FallStairs)				// kAIS_FallStairs
ADD_AISOUND_TYPE(FanOut)					// kAIS_FanOut
ADD_AISOUND_TYPE(GetBehindHim)				// kAIS_GetBehindHim
ADD_AISOUND_TYPE(Give)						// kAIS_Give
ADD_AISOUND_TYPE(Grenade)					// kAIS_Grenade
ADD_AISOUND_TYPE(GrenadeDud)				// kAIS_GrenadeDud
ADD_AISOUND_TYPE(GrenadeThreat)				// kAIS_GrenadeThreat
ADD_AISOUND_TYPE(GrenadeThreatAlone)		// kAIS_GrenadeThreatAlone
ADD_AISOUND_TYPE(HearLostTarget)			// kAIS_HearLostTarget
ADD_AISOUND_TYPE(HeavyArmorDown)			// kAIS_HeavyArmorDown
ADD_AISOUND_TYPE(Hit)						// kAIS_Hit
ADD_AISOUND_TYPE(HitSeen)					// kAIS_HitSeen
ADD_AISOUND_TYPE(HitSeenMelee)				// kAIS_HitSeenMelee
ADD_AISOUND_TYPE(HoldPosition)				// kAIS_HoldPosition
ADD_AISOUND_TYPE(LeadFollowerCaughtUp)		// kAIS_LeadFollowerCaughtUp
ADD_AISOUND_TYPE(LeadFollowerFellBehind)	// kAIS_LeadFollowerFellBehind
ADD_AISOUND_TYPE(LocationBoxes)				// kAIS_LocationBoxes
ADD_AISOUND_TYPE(LocationCatwalk)			// kAIS_LocationCatwalk
ADD_AISOUND_TYPE(LocationCeiling)			// kAIS_LocationCeiling
ADD_AISOUND_TYPE(LocationChair)				// kAIS_LocationChair
ADD_AISOUND_TYPE(LocationColumn)			// kAIS_LocationColumn
ADD_AISOUND_TYPE(LocationCouch)				// kAIS_LocationCouch
ADD_AISOUND_TYPE(LocationCrate)				// kAIS_LocationCrate
ADD_AISOUND_TYPE(LocationCubicle)			// kAIS_LocationCubicle
ADD_AISOUND_TYPE(LocationDesk)				// kAIS_LocationDesk
ADD_AISOUND_TYPE(LocationDoor)				// kAIS_LocationDoor
ADD_AISOUND_TYPE(LocationKiosk)				// kAIS_LocationKiosk
ADD_AISOUND_TYPE(LocationMachine)			// kAIS_LocationMachine
ADD_AISOUND_TYPE(LocationPipe)				// kAIS_LocationPipe
ADD_AISOUND_TYPE(LocationPlanter)			// kAIS_LocationPlanter
ADD_AISOUND_TYPE(LocationShelf)				// kAIS_LocationShelf
ADD_AISOUND_TYPE(LocationTable)				// kAIS_LocationTable
ADD_AISOUND_TYPE(LocationVendingMachine)	// kAIS_LocationVendingMachine
ADD_AISOUND_TYPE(LocationVent)				// kAIS_LocationVent
ADD_AISOUND_TYPE(LocationWall)				// kAIS_LocationWall
ADD_AISOUND_TYPE(ManDown)					// kAIS_ManDown
ADD_AISOUND_TYPE(ManDownAll)				// kAIS_ManDownAll
ADD_AISOUND_TYPE(ManDownThree)				// kAIS_ManDownThree
ADD_AISOUND_TYPE(ManDownTwo)				// kAIS_ManDownTwo
ADD_AISOUND_TYPE(Miss)						// kAIS_Miss
ADD_AISOUND_TYPE(Negative)					// kAIS_Negative
ADD_AISOUND_TYPE(NegativeStrong)			// kAIS_NegativeStrong
ADD_AISOUND_TYPE(NoShot)					// kAIS_NoShot
ADD_AISOUND_TYPE(NowhereToGo)				// kAIS_NowhereToGo
ADD_AISOUND_TYPE(OrderAdvance)				// kAIS_OrderAdvance
ADD_AISOUND_TYPE(OrderCover)				// kAIS_OrderCover
ADD_AISOUND_TYPE(OrderCoverAim)				// kAIS_OrderCoverAim
ADD_AISOUND_TYPE(OrderCoverFOV)				// kAIS_OrderCoverFOV
ADD_AISOUND_TYPE(OrderCoverRadius)			// kAIS_OrderCoverRadius
ADD_AISOUND_TYPE(OrderInvestigate)			// kAIS_OrderInvestigate
ADD_AISOUND_TYPE(OrderMove)					// kAIS_OrderMove
ADD_AISOUND_TYPE(OrderSearch)				// kAIS_OrderSearch
ADD_AISOUND_TYPE(Pain)						// kAIS_Pain
ADD_AISOUND_TYPE(PainUnderWater)			// kAIS_PainUnderWater
ADD_AISOUND_TYPE(PinnedDown)				// kAIS_PinnedDown
ADD_AISOUND_TYPE(PoweredArmorDown)			// kAIS_PoweredArmorDown
ADD_AISOUND_TYPE(RandomChatter)				// kAIS_RandomChatter
ADD_AISOUND_TYPE(Regroup)					// kAIS_Regroup
ADD_AISOUND_TYPE(RequestAmmo)				// kAIS_RequestAmmo
ADD_AISOUND_TYPE(RequestAdvance)			// kAIS_RequestAdvance
ADD_AISOUND_TYPE(RequestCover)				// kAIS_RequestCover
ADD_AISOUND_TYPE(RequestCoverAim)			// kAIS_RequestCoverAim
ADD_AISOUND_TYPE(RequestCoverFOV)			// kAIS_RequestCoverFOV
ADD_AISOUND_TYPE(RequestCoverRadius)		// kAIS_RequestCoverRadius
ADD_AISOUND_TYPE(SearchCheckDownstairs)		// kAIS_RequestCheckDownstairs
ADD_AISOUND_TYPE(SearchCheckInside)			// kAIS_RequestCheckInside
ADD_AISOUND_TYPE(SearchCheckLab)			// kAIS_RequestCheckLab
ADD_AISOUND_TYPE(SearchCheckOffice)			// kAIS_RequestCheckOffice
ADD_AISOUND_TYPE(SearchCheckOutside)		// kAIS_RequestCheckOutside
ADD_AISOUND_TYPE(SearchCheckStairwell)		// kAIS_RequestCheckStairwell
ADD_AISOUND_TYPE(SearchCheckUpstairs)		// kAIS_RequestCheckUpstairs
ADD_AISOUND_TYPE(SearchClear)				// kAIS_SearchClear
ADD_AISOUND_TYPE(SearchFailed)				// kAIS_SearchFailed
ADD_AISOUND_TYPE(SearchGuard)				// kAIS_SearchGuard
ADD_AISOUND_TYPE(ShutUp)					// kAIS_ShutUp
ADD_AISOUND_TYPE(SlowMo)					// kAIS_SlowMo
ADD_AISOUND_TYPE(SniperDetected)			// kAIS_SniperDetected
ADD_AISOUND_TYPE(SuppressionFire)			// kAIS_SuppressionFire
ADD_AISOUND_TYPE(StatusCheck)				// kAIS_StatusCheck
ADD_AISOUND_TYPE(StatusLostContact)			// kAIS_StatusLostContact
ADD_AISOUND_TYPE(StatusOK)					// kAIS_StatusOK
ADD_AISOUND_TYPE(Thanks)					// kAIS_Thanks
ADD_AISOUND_TYPE(TurretDetected)			// kAIS_TurretDetected
ADD_AISOUND_TYPE(TurretStress)				// kAIS_TurretStress
ADD_AISOUND_TYPE(UnderFire)					// kAIS_UnderFire
ADD_AISOUND_TYPE(WhatThe)					// kAIS_WhatThe
ADD_AISOUND_TYPE(WhereIsHe)					// kAIS_WhereIsHe
ADD_AISOUND_TYPE(WhereShouldIGo)			// kAIS_WhereShouldIGo
