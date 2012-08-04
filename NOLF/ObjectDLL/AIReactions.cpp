// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AIUtils.h"
#include "AI.h"

REACTIONSTRUCT g_aAIReactions[] =
{
	{ "ISE1st",

		c_szNoReaction,
	},

	{ "ISE",

		c_szNoReaction,
	},

	{ "ISEFalse1st",

		c_szNoReaction,
	},

	{ "ISEFalse",

		c_szNoReaction,
	},

	{ "ISEFlashlight1st",

		c_szNoReaction,
	},

	{ "ISEFlashlight",

		c_szNoReaction,
	},

	{ "ISEFlashlightFalse1st",

		c_szNoReaction,
	},

	{ "ISEFlashlightFalse",

		c_szNoReaction,
	},

	{ "IHEFootstep1st",

		c_szNoReaction,
	},

	{ "IHEFootstep",

		c_szNoReaction,
	},

	{ "IHEFootstepFalse1st",

		c_szNoReaction,
	},

	{ "IHEFootstepFalse",

		c_szNoReaction,
	},

	{ "IHEWeaponFire1st",

		c_szNoReaction,
	},

	{ "IHEWeaponFire",

		c_szNoReaction,
	},

	{ "IHAWeaponFire1st",

		c_szNoReaction,
	},

	{ "IHAWeaponFire",

		c_szNoReaction,
	},

	{ "IHEWeaponImpact1st",

		c_szNoReaction,
	},

	{ "IHEWeaponImpact",

		c_szNoReaction,
	},

	{ "ISADeath1st",

		c_szNoReaction,
	},

	{ "ISADeath",

		c_szNoReaction,
	},

	{ "ISEFootprint1st",

		c_szNoReaction,
	},

	{ "ISEFootprint",

		c_szNoReaction,
	},

	{ "IHEDisturbance1st",

		c_szNoReaction,
	},

	{ "IHEDisturbance",

		c_szNoReaction,
	},

	{ "IHAPain1st",

		c_szNoReaction,
	},

	{ "IHAPain",

		c_szNoReaction,
	},

	{ "IHADeath1st",

		c_szNoReaction,
	},

	{ "IHADeath",

		c_szNoReaction,
	},

	{ "GSE1st",

		c_szNoReaction,
	},

	{ "GSE",

		c_szNoReaction,
	},

	{ "GSEFalse1st",

		c_szNoReaction,
	},

	{ "GSEFalse",

		c_szNoReaction,
	},

	{ "GSEFlashlight1st",

		c_szNoReaction,
	},

	{ "GSEFlashlight",

		c_szNoReaction,
	},

	{ "GSEFlashlightFalse1st",

		c_szNoReaction,
	},

	{ "GSEFlashlightFalse",

		c_szNoReaction,
	},

	{ "GHEFootstep1st",

		c_szNoReaction,
	},

	{ "GHEFootstep",

		c_szNoReaction,
	},

	{ "GHEFootstepFalse1st",

		c_szNoReaction,
	},

	{ "GHEFootstepFalse",

		c_szNoReaction,
	},

	{ "GHEWeaponFire1st",

		c_szNoReaction,
	},

	{ "GHEWeaponFire",

		c_szNoReaction,
	},

	{ "GHAWeaponFire1st",

		c_szNoReaction,
	},

	{ "GHAWeaponFire",

		c_szNoReaction,
	},

	{ "GHEWeaponImpact1st",

		c_szNoReaction,
	},

	{ "GHEWeaponImpact",

		c_szNoReaction,
	},

	{ "GSADeath1st",

		c_szNoReaction,
	},

	{ "GSADeath",

		c_szNoReaction,
	},

	{ "GSEFootprint1st",

		c_szNoReaction,
	},

	{ "GSEFootprint",

		c_szNoReaction,
	},

	{ "GHEDisturbance1st",

		c_szNoReaction,
	},

	{ "GHEDisturbance",

		c_szNoReaction,
	},

	{ "GHAPain1st",

		c_szNoReaction,
	},

	{ "GHAPain",

		c_szNoReaction,
	},

	{ "GHADeath1st",

		c_szNoReaction,
	},

	{ "GHADeath",

		c_szNoReaction,
	}
};

int g_cAIReactions = sizeof(g_aAIReactions)/sizeof(REACTIONSTRUCT);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::DoReaction()
//
//	PURPOSE:	Execute a reaction
//
// ----------------------------------------------------------------------- //

void CAI::DoReaction(HSTRING hstrReaction, CAISense* pAISense, LTBOOL bIndividual)
{
    char* szReaction = g_pLTServer->GetStringData(hstrReaction);
}