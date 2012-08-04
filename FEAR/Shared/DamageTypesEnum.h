// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypesEnum.h
//
// PURPOSE : Enums and string constants for damage types.
//
// (c) 2001-2005 Monolith Productions, Inc.  All Rights Reserved
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
#else
	#error	To use this include file, first define INCLUDE_AS_ENUM or INCLUDE_AS_STRING, to include the DamegeTypes as enums or string constants.
#endif


ADD_DT(UNSPECIFIED,Unspecified,NULL)					// DT_UNSPECIFIED - Unknown type (self-damage)
ADD_DT(BULLET,Bullet,NULL)								// DT_BULLET - (bullets)
ADD_DT(BURN,Burn,NULL)									// DT_BURN - (fire, corrosives)
ADD_DT(ELECTRICITY,Electricity,NULL)					// DT_ELECTRICITY - (electical, sparks, lightning bolts, etc.)
ADD_DT(CRUSH,Crush,NULL)								// DT_CRUSH - (falling, crushing, collision)
ADD_DT(EXPLODE,Explode,NULL)							// DT_EXPLODE - (explosions)
ADD_DT(ENDLESS_FALL,EndlessFall,ENDLESS FALL)			// DT_ENDLESS_FALL - (Falling and can never get up)
ADD_DT(STUN,Stun,NULL)									// DT_STUN 
ADD_DT(MELEE,Melee,NULL)								// DT_MELEE
ADD_DT(WORLDONLY, WorldOnly,WORLD ONLY)					// DT_WORLDONLY (can only damage worldmodels)
ADD_DT(RIFLE_BUTT, RifleButt, NULL )					// DT_RIFLE_BUTT
ADD_DT(TASER, Taser,NULL)								// DT_TASER (Level 1 taser weapon)
ADD_DT(TASER_UPGRADE, Taser_Upgrade,NULL)				// DT_TASER_UPGRADE (Level 2 taser weapon)
ADD_DT(HELMET_PIERCING, HelmetPiercing, NULL )			// DT_HELMET_PIERCING
ADD_DT(ENERGY, Energy, NULL )							// DT_ENERGY
ADD_DT(SLOW_MO, SlowMo, NULL )							// DT_SLOW_MO
ADD_DT(SUPERNATURAL, Supernatural, NULL )				// DT_SUPERNATURAL

