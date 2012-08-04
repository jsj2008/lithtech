// ----------------------------------------------------------------------- //
//
// MODULE  : SkillDefs.h
//
// PURPOSE : Definitions of skill related functions, this is basically
//				a quick and dirty way of replacing NOLF2's skill system.
//				A more robust and complete solution should be implemented
//				once the requirments for a new skill system are developed.
//
// CREATED : 03/22/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SKILLDEFS_H__
#define __SKILLDEFS_H__

// NOTE: there were #ifdef _CLIENTBUILDs around the client vs. server components here.
// the SEM solution would only compile the _CLIENTBUILD section in, which made kNumSkills
// enum to too low of a value, which caused GetSkillValue() to fail when called from the server.
enum ePlayerSkills
{
	//used on client
	eAimAccuracy,
	eAimCorrection,
	eAimZoomSway,
	eStaminaMoveDamage,
	eWeaponReloadSpeed,
	//used on server
	eArmorMax,
	eArmorPickup,
	eHealthMax,
	eHealthPickup,
	eStaminaDamage,
	eStaminaResistance,
	eStealthRadius,
	eWeaponDamage,
	kNumSkills
};

float GetSkillValue(ePlayerSkills skl);

#endif  // __SKILLDEFS_H__
