// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIUtils.h"
#include "AIDog.h"
#include "AISense.h"
#include "AIState.h"
#include "DeathScene.h"
#include "AINodeMgr.h"


/*

  Current available reactions, some of which are context sensitive:

  ... = not implemented
  ~~~ = partially implemented
  +++ = implemented

... Chase
... Bark
... Excited
... Heat

*/

// TODO: Optimize all these fucking strcmps!!!!!!!!!!!!!

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AI_Dog::DoReaction()
//
//	PURPOSE:	Execute a reaction
//
// ----------------------------------------------------------------------- //

void AI_Dog::DoReaction(HSTRING hstrReaction, CAISense* pAISense, LTBOOL bIndividual)
{
	if ( !hstrReaction ) return;

	char szTrigger[1024];
    char* szReaction = g_pLTServer->GetStringData(hstrReaction);
	_ASSERT(szReaction);

	if ( !strcmp(szReaction, c_szNoReaction) )
	{
		return;
	}
	else if ( !strcmp(szReaction, "Bark") )
	{
        sprintf(szTrigger, "TARGET %s;BARK", g_pLTServer->GetObjectName(pAISense->GetStimulus()));
	}
	else if ( !strcmp(szReaction, "Excited") )
	{
		sprintf(szTrigger, "EXCITED");
	}
	else if ( !strcmp(szReaction, "Heat") )
	{
		sprintf(szTrigger, "HEAT");
	}
	else
	{
		// If it doesn't match one of these, it must be a command

        sprintf(szTrigger, g_pLTServer->GetStringData(hstrReaction));
	}

	DidReaction(pAISense, bIndividual);
	ChangeState(szTrigger);
	GetSenseMgr()->StopUpdating();
}

REACTIONSTRUCT g_aAIDogReactions[] =
{
	{ "ISE1st",

		c_szNoReaction,
		"Bark",
	},

	{ "ISE",

		c_szNoReaction,
		"Bark",
	},

	{ "ISEFalse1st",

		c_szNoReaction,
		"Bark",
	},

	{ "ISEFalse",

		c_szNoReaction,
		"Bark",
	},

	{ "ISEFlashlight1st",

		c_szNoReaction,
		"Bark",
	},

	{ "ISEFlashlight",

		c_szNoReaction,
		"Bark",
	},

	{ "ISEFlashlightFalse1st",

		c_szNoReaction,
		"Bark",
	},

	{ "ISEFlashlightFalse",

		c_szNoReaction,
		"Bark",
	},

	{ "IHEFootstep1st",

		c_szNoReaction,
		"Bark",
	},

	{ "IHEFootstep",

		c_szNoReaction,
		"Bark",
	},

	{ "IHEFootstepFalse1st",

		c_szNoReaction,
		"Bark",
	},

	{ "IHEFootstepFalse",

		c_szNoReaction,
		"Bark",
	},

	{ "IHEWeaponFire1st",

		c_szNoReaction,
		"Bark",
	},

	{ "IHAWeaponFire1st",

		c_szNoReaction,
		"Bark",
	},

	{ "IHEWeaponFire",

		c_szNoReaction,
		"Bark",
	},

	{ "IHAWeaponFire",

		c_szNoReaction,
		"Bark",
	},

	{ "IHEWeaponImpact1st",

		c_szNoReaction,
		"Bark",
	},

	{ "IHEWeaponImpact",

		c_szNoReaction,
		"Bark",
	},

	{ "ISADeath1st",

		c_szNoReaction,
		"Bark",
	},

	{ "ISADeath",

		c_szNoReaction,
		"Bark",
	},

	{ "ISEFootprint1st",

		c_szNoReaction,
		"Bark",
	},

	{ "ISEFootprint",

		c_szNoReaction,
		"Bark",
	},

	{ "IHEDisturbance1st",

		c_szNoReaction,
		"Bark",
	},

	{ "IHEDisturbance",

		c_szNoReaction,
		"Bark",
	},

	{ "IHAPain1st",

		c_szNoReaction,
		"Bark",
	},

	{ "IHAPain",

		c_szNoReaction,
		"Bark",
	},

	{ "IHADeath1st",

		c_szNoReaction,
		"Bark",
	},

	{ "IHADeath",

		c_szNoReaction,
		"Bark",
	},

	{ "GSE1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GSE",

		c_szNoReaction,
		"Bark",
	},

	{ "GSEFalse1st",

		c_szNoReaction,
		//"Halt",
		"Bark",
	},

	{ "GSEFalse",

		c_szNoReaction,
		"Bark",
	},

	{ "GSEFlashlight1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GSEFlashlight",

		c_szNoReaction,
		"Bark",
	},

	{ "GSEFlashlightFalse1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GSEFlashlightFalse",

		c_szNoReaction,
		"Bark",
	},

	{ "GHEFootstep1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GHEFootstep",

		c_szNoReaction,
		"Bark",
	},

	{ "GHEFootstepFalse1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GHEFootstepFalse",

		c_szNoReaction,
		"Bark",
	},

	{ "GHEWeaponFire1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GHEWeaponFire",

		c_szNoReaction,
		"Bark",
	},

	{ "GHAWeaponFire1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GHAWeaponFire",

		c_szNoReaction,
		"Bark",
	},

	{ "GHEWeaponImpact1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GHEWeaponImpact",

		c_szNoReaction,
		"Bark",
	},

	{ "GSADeath1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GSADeath",

		c_szNoReaction,
		"Bark",
	},

	{ "GSEFootprint1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GSEFootprint",

		c_szNoReaction,
		"Bark",
	},

	{ "GHEDisturbance1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GHEDisturbance",

		c_szNoReaction,
		"Bark",
	},

	{ "GHAPain1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GHAPain",

		c_szNoReaction,
		"Bark",
	},

	{ "GHADeath1st",

		c_szNoReaction,
		"Bark",
	},

	{ "GHADeath",

		c_szNoReaction,
		"Bark",
	},
};

int g_cAIDogReactions = sizeof(g_aAIDogReactions)/sizeof(REACTIONSTRUCT);