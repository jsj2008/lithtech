// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypesEnum.h
//
// PURPOSE : Enums and string constants for damage types.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

//
// The following macros allow the enum entries to be included as the 
// body of an enum, the body of a const char* string list, or as a constructor
// for a DTINFO struct.
//

#undef ADD_DT

#if defined(INCLUDE_AS_ENUM)
	#define ADD_DT(id,tag,altname) DT_##id,
#elif defined(INCLUDE_AS_STRING)
	#define ADD_DT(id,bute,altname) #bute,
#elif defined(INCLUDE_AS_STRUCT)
	#define ADD_DT(id,bute,altname) DTINFO(DT_##id,#bute,#altname),
#else
	#error	To use this include file, first define INCLUDE_AS_ENUM, INCLUDE_AS_STRING or INCLUDE_AS_STRUCT, to include the goals as enums, bute string constants, or struct constructors.
#endif

// NOTE: The order of these are IMPORTANT!!
//		 The numbers to the right of the damage type should match up identiclly to the 
//		 numbers representing the damage types found in the Ammo section of weapons.txt.
//		 Additionally, there is a habbit of sending any of the first 8 damage types' flags
//		 as a byte, so if they are moved outside that range there will be instances in
//		 the code that will send blanks rather than relevant flags. 


ADD_DT(UNSPECIFIED,Unspecified,NULL)					// 0 DT_UNSPECIFIED - Unknown type (self-damage)
ADD_DT(BLEEDING,Bleeding,NULL)							// 1 DT_BLEEDING - (loss of blood)
ADD_DT(BULLET,Bullet,NULL)								// 2 DT_BULLET - (bullets)
ADD_DT(BURN,Burn,NULL)									// 3 DT_BURN - (fire, corrosives)
ADD_DT(CHOKE,Choke,NULL)								// 4 DT_CHOKE - (water, hostile atmosphere)
ADD_DT(CRUSH,Crush,NULL)								// 5 DT_CRUSH - (falling, crushing, collision)
ADD_DT(ELECTROCUTE,Electrocute,NULL)					// 6 DT_ELECTROCUTE - (electricity)
ADD_DT(EXPLODE,Explode,NULL)							// 7 DT_EXPLODE - (explosions)
ADD_DT(FREEZE,Freeze,NULL)								// 8 DT_FREEZE - (freezing air or fluid)
ADD_DT(POISON,Poison,NULL)								// 9 DT_POISON - (poison gas/dart)
ADD_DT(ENDLESS_FALL,EndlessFall,ENDLESS FALL)			// 10 DT_ENDLESS_FALL - (Falling and can never get up)
ADD_DT(SLEEPING,Sleeping,NULL)							// 11 DT_SLEEPING
ADD_DT(STUN,Stun,NULL)									// 12 DT_STUN 
ADD_DT(MELEE,Melee,NULL)								// 13 DT_MELEE
ADD_DT(CAMERA_DISABLER,CameraDisabler,CAMERA DISABLER)	// 14 DT_CAMERA_DISABLER
ADD_DT(GLUE,Glue,NULL)									// 15 DT_GLUE - (Glue Bomb)
ADD_DT(BEAR_TRAP, BearTrap,BEAR TRAP)					// 16 DT_BEAR_TRAP
ADD_DT(LAUGHING,Laughing,NULL)							// 17 DT_LAUGHING - (Uncontrolable Laughing)
ADD_DT(ASSS,AntiSuperSoldierSerum,ANTI-SUPER SOLDIER SERUM)		// 18 DT_ASSS - (Anti-super soldier serum)

// Special gadget damage types...

ADD_DT(GADGET_CODE_DECIPHERER,CodeDecipherer,GADGET CODE DECIPHERER)		// 19 DT_GADGET_CODE_DECIPHERER
ADD_DT(GADGET_POODLE,Poodle,GADGET POODLE)				// 20 DT_GADGET_POODLE
ADD_DT(GADGET_LOCK_PICK,LockPick,GADGET LOCK PICK)		// 21 DT_GADGET_LOCK_PICK
ADD_DT(GADGET_WELDER,Welder,GADGET WELDER)				// 22 DT_GADGET_WELDER
ADD_DT(GADGET_LIGHTER,Lighter,GADGET LIGHTER)			// 23 DT_GADGET_LIGHTER
ADD_DT(GADGET_CAMERA,Camera,GADGET CAMERA)				// 24 DT_GADGET_CAMERA
ADD_DT(WORLDONLY, WorldOnly,WORLD ONLY)					// 25 DT_WORLDONLY (can only damage worldmodels)
ADD_DT(GADGET_INFRA_RED,InfraRed,GADGET INFRA RED)		// 26 DT_GADGET_INFRA_RED
ADD_DT(GADGET_DECAYPOWDER,DecayPowder,GADGET DECAY POWDER)		// 27 DT_GADGET_DECAYPOWDER
ADD_DT(GADGET_TIME_BOMB, TimeBomb,GADGET TIME BOMB)			// 28 DT_GADGET_TIME_BOMB
ADD_DT(GADGET_INK_REAGENT, InkReagent,GADGET INK REAGENT )	// 29 DT_GADGET_INK_REAGENT
ADD_DT(GADGET_EAVESDROPBUG, EavesDropBug,GADGET EAVESDROP BUG )	// 30 DT_GADGET_EAVESDROPBUG

ADD_DT(SLIPPERY, Slippery,NULL)							// 31 DT_SLIPPERY - (Banana, oil slick)
ADD_DT(SWORD, Sword,NULL)								// 32 DT_SWORD - (Katana, Tulwar)
ADD_DT(GADGET_TRACKER, Tracker,GADGET_TRACKER)			// 33 DT_GADGET_TRACKER							
ADD_DT(DOOMSDAYBURN, DoomsDayBurn,NULL)					// 34 DT_DOOMSDAYBURN
