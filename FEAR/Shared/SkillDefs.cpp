// ----------------------------------------------------------------------- //
//
// MODULE  : SkillDefs.cpp
//
// PURPOSE : Implementation of skill related functions, this is basically
//				a quick and dirty way of replacing NOLF2's skill system.
//				A more robust and complete solution should be implemented
//				once the requirments for a new skill system are developed.
//
// CREATED : 03/22/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "SkillDefs.h"
#include "VarTrack.h"

static char szConVars[kNumSkills][64] = 
{
	"SkillAimAccuracy",
	"SkillAimCorrection",
	"SkillAimZoomSway",
	"SkillStaminaMoveDamage",
	"SkillWeaponReloadSpeed",
	"SkillArmorMax",
	"SkillArmorPickup",
	"SkillHealthMax",
	"SkillHealthPickup",
	"SkillStaminaDamage",
	"SkillStaminaResistance",
	"SkillStealthRadius",
	"SkillWeaponDamage",
};

static VarTrack g_vtSkillVars[kNumSkills]; 

float GetSkillValue(ePlayerSkills skl)
{
	if (!g_vtSkillVars[0].IsInitted()) 
	{
		for (int i = 0;i < kNumSkills; i++)
		{
			g_vtSkillVars[i].Init(g_pLTBase,szConVars[i],NULL,1.0f);
		}
	}

	return g_vtSkillVars[skl].GetFloat(1.0f);
}


