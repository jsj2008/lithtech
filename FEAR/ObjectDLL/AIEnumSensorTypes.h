// ----------------------------------------------------------------------- //
//
// MODULE  : AIEnumSensorTypes.h
//
// PURPOSE : Enums and string constants for sensors.
//
// CREATED : 2/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, or the body of a const char* string list.
//

#ifdef ADD_SENSOR_TYPE
	#undef ADD_SENSOR_CLASS
	#undef ADD_SENSOR_TYPE
#endif
 
#if SENSOR_TYPE_AS_ENUM
	#define ADD_SENSOR_CLASS(label) kSensor_##label,
	#define ADD_SENSOR_TYPE(label) kSensor_##label,
#elif SENSOR_TYPE_AS_STRING
	#define ADD_SENSOR_CLASS(label) #label,
	#define ADD_SENSOR_TYPE(label) #label,
#elif SENSOR_TYPE_AS_SWITCH
	#define ADD_SENSOR_CLASS(label) case kSensor_##label: extern CAIClassAbstract* AIFactoryCreateCAISensor##label(); return (CAISensorAbstract*)AIFactoryCreateCAISensor##label();
	#define ADD_SENSOR_TYPE(label)
#else
	#error	To use this include file, first define either SENSOR_TYPE_AS_ENUM or SENSOR_TYPE_AS_STRING, to include the sensors as enums, or string constants.
#endif

// --------------------------------------------------------------------------
// USAGE: To add a new enum, just add a ADD_SENSOR_TYPE(x) where 
// x is the name of the enum without the "kSensor_" prefix.
// --------------------------------------------------------------------------

ADD_SENSOR_CLASS(AwarenessMod)			// kSensor_AwarenessMod
ADD_SENSOR_CLASS(Berserker)				// kSensor_Berserker
ADD_SENSOR_CLASS(Cloak)					// kSensor_Cloak
ADD_SENSOR_CLASS(CombatOpportunity)		// kSensor_CombatOpportunity
ADD_SENSOR_CLASS(CoolMove)				// kSensor_CoolMove
ADD_SENSOR_CLASS(CounterMelee)			// kSensor_CounterMelee
ADD_SENSOR_CLASS(Critter)				// kSensor_Critter
ADD_SENSOR_CLASS(Damage)				// kSensor_Damage
ADD_SENSOR_CLASS(DamageFX)				// kSensor_DamageFX
ADD_SENSOR_CLASS(DamageTurret)			// kSensor_DamageTurret
ADD_SENSOR_CLASS(EnemyDamaged)			// kSensor_EnemyDamaged
ADD_SENSOR_CLASS(EnemyDamagedSuicide)	// kSensor_EnemyDamagedSuicide
ADD_SENSOR_CLASS(Flashlight)			// kSensor_Flashlight
ADD_SENSOR_CLASS(FlyThru)				// kSensor_FlyThru
ADD_SENSOR_CLASS(Guard)					// kSensor_Guard
ADD_SENSOR_CLASS(HearDisturbance)		// kSensor_HearDisturbance
ADD_SENSOR_CLASS(InstantDeath)			// kSensor_InstantDeath
ADD_SENSOR_CLASS(Location)				// kSensor_Location
ADD_SENSOR_CLASS(LostTarget)			// kSensor_LostTarget
ADD_SENSOR_CLASS(Lunge)					// kSensor_Lunge
ADD_SENSOR_CLASS(Node)					// kSensor_Node
ADD_SENSOR_CLASS(NodeCombat)			// kSensor_NodeCombat
ADD_SENSOR_CLASS(NodeFollow)			// kSensor_NodeFollow
ADD_SENSOR_CLASS(NodeGuarded)			// kSensor_NodeGuarded
ADD_SENSOR_CLASS(NodeInterest)			// kSensor_NodeInterest
ADD_SENSOR_CLASS(NodeSafetyFirePosition)// kSensor_NodeSafetyFirePosition
ADD_SENSOR_CLASS(NodeSearch)			// kSensor_NodeSearch
ADD_SENSOR_CLASS(NodeStalk)				// kSensor_NodeStalk
ADD_SENSOR_CLASS(PassTarget)			// kSensor_PassTarget
ADD_SENSOR_CLASS(Patrol)				// kSensor_Patrol
ADD_SENSOR_CLASS(PersonalBubble)		// kSensor_PersonalBubble
ADD_SENSOR_CLASS(PlayerTooClose)		// kSensor_PlayerTooClose
ADD_SENSOR_CLASS(Pulse)					// kSensor_Pulse
ADD_SENSOR_CLASS(Scanner)				// kSensor_Scanner
ADD_SENSOR_CLASS(SeeDanger)				// kSensor_SeeDanger
ADD_SENSOR_CLASS(SeeDisturbance)		// kSensor_SeeDisturbance
ADD_SENSOR_CLASS(SeeEnemy)				// kSensor_SeeEnemy
ADD_SENSOR_CLASS(SeeEnemyNoFOV)			// kSensor_SeeEnemyNoFOV
ADD_SENSOR_CLASS(SeeEnemyNoShootThru)	// kSensor_SeeEnemyNoShootThru
ADD_SENSOR_CLASS(SeeFlashlightBeam)		// kSensor_SeeFlashlightBeam
ADD_SENSOR_CLASS(SeekEnemy)				// kSensor_SeekEnemy
ADD_SENSOR_CLASS(Shoved)				// kSensor_Shoved
ADD_SENSOR_CLASS(SlowMo)				// kSensor_SlowMo
ADD_SENSOR_CLASS(SquadCommunicationThreat)	// kSensor_SquadCommunicationThreat
ADD_SENSOR_CLASS(StatusCheck)			// kSensor_StatusCheck
ADD_SENSOR_CLASS(FlamePot)				// kSensor_FlamePot
ADD_SENSOR_CLASS(TargetIsAimingAtMe)	// kSensor_TargetIsAimingAtMe
ADD_SENSOR_CLASS(TargetIsAimingAtMeMelee)	// kSensor_TargetIsAimingAtMeMelee
ADD_SENSOR_CLASS(TargetIsAimingAtMeParanoid)	// kSensor_TargetIsAimingAtMeParanoid
ADD_SENSOR_CLASS(TargetIsAwareOfMyPosition)		// kSensor_TargetIsAwareOfMyPosition
ADD_SENSOR_CLASS(Traitor)				// kSensor_Traitor
ADD_SENSOR_CLASS(Touch)					// kSensor_Touch
ADD_SENSOR_CLASS(ValidPosition)			// kSensor_ValidPosition
ADD_SENSOR_CLASS(View)					// kSensor_View
ADD_SENSOR_CLASS(WeaponItem)			// kSensor_WeaponItem

ADD_SENSOR_TYPE(Ambush)					// kSensor_Ambush
ADD_SENSOR_TYPE(Alarm)					// kSensor_Alarm
ADD_SENSOR_TYPE(Armored)				// kSensor_Armored
ADD_SENSOR_TYPE(CombatOpportunityView)	// kSensor_CombatOpportunityView
ADD_SENSOR_TYPE(CombatOpportunityUse)	// kSensor_CombatOpportunityUse
ADD_SENSOR_TYPE(Cover)					// kSensor_Cover
ADD_SENSOR_TYPE(DarkChant)				// kSensor_DarkChant
ADD_SENSOR_TYPE(DarkWait)				// kSensor_DarkWait
ADD_SENSOR_TYPE(FallBack)				// kSensor_FallBack
ADD_SENSOR_TYPE(Hide)					// kSensor_Hide
ADD_SENSOR_TYPE(Menace)					// kSensor_Menace
ADD_SENSOR_TYPE(Safety)					// kSensor_Safety
ADD_SENSOR_TYPE(Surprise)				// kSensor_Surprise
ADD_SENSOR_TYPE(Work)					// kSensor_Work


