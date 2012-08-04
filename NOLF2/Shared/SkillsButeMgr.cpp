// ----------------------------------------------------------------------- //
//
// MODULE  : SkillsButeMgr.cpp
//
// PURPOSE : Implementation of bute mgr to manage skill related data
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SkillsButeMgr.h"


#ifdef _CLIENTBUILD

#include "clientresshared.h"
#include "ClientUtilities.h"

int GetSkillNameId(eSkill skl)
{
	static int nSkillIds[kNumSkills] = 
	{
		IDS_SKILL_STEALTH,
		IDS_SKILL_STAMINA,
		IDS_SKILL_MARKSMANSHIP,
		IDS_SKILL_CARRY,
		IDS_SKILL_ARMOR,
		IDS_SKILL_WEAPON,
		IDS_SKILL_GADGET,
		IDS_SKILL_SEARCH
	};

	return nSkillIds[skl];
}
const char* GetSkillName(eSkill skl)
{
	return (const char *)LoadTempString(GetSkillNameId(skl));
}
int GetSkillDescriptionId(eSkill skl)
{
	static int nSkillIds[kNumSkills] = 
	{
		IDS_SKILL_STEALTH_DESC,
		IDS_SKILL_STAMINA_DESC,
		IDS_SKILL_MARKSMANSHIP_DESC,
		IDS_SKILL_CARRY_DESC,
		IDS_SKILL_ARMOR_DESC,
		IDS_SKILL_WEAPON_DESC,
		IDS_SKILL_GADGET_DESC,
		IDS_SKILL_SEARCH_DESC
	};

	return nSkillIds[skl];
}
const char* GetSkillDescription(eSkill skl)
{
	return (const char *)LoadTempString(GetSkillDescriptionId(skl));
}
const char* GetSkillLevelName(eSkillLevel lvl)
{
	static int nIds[kNumSkillLevels] = 
	{
		IDS_SKILL_NOVICE,
		IDS_SKILL_AMATEUR,
		IDS_SKILL_SKILLED,
		IDS_SKILL_EXPERT,
		IDS_SKILL_MASTER,
	};

	return (const char *)LoadTempString(nIds[lvl]);
}

static uint16 nModNames[kNumSkills][kMaxModifiers] =
{
	{	IDS_SKILL_2110,	IDS_SKILL_2111,	IDS_SKILL_2112,	0,				0				},
	{	IDS_SKILL_2113,	IDS_SKILL_2114,	IDS_SKILL_2115,	IDS_SKILL_2116,	IDS_SKILL_2117	},
	{	IDS_SKILL_2118,	IDS_SKILL_2119,	IDS_SKILL_2120,	0,				0				},
	{	IDS_SKILL_2121,	IDS_SKILL_2122,	0,				0,				0				},
	{	IDS_SKILL_2123,	IDS_SKILL_2124,	0,				0,				0				},
	{	IDS_SKILL_2125,	IDS_SKILL_2126,	0,				0,				0				},
	{	IDS_SKILL_2127,	IDS_SKILL_2128,	0,				0,				0				},
	{	IDS_SKILL_2129,	IDS_SKILL_2130,	0,				0,				0				}

};

static uint16 nModDesc[kNumSkills][kMaxModifiers] =
{
	{	IDS_SKILL_HELP_2150,	IDS_SKILL_HELP_2151,	IDS_SKILL_HELP_2152,	0,						0					},
	{	IDS_SKILL_HELP_2153,	IDS_SKILL_HELP_2154,	IDS_SKILL_HELP_2155,	IDS_SKILL_HELP_2156,	IDS_SKILL_HELP_2157	},
	{	IDS_SKILL_HELP_2158,	IDS_SKILL_HELP_2159,	IDS_SKILL_HELP_2160,	0,						0					},
	{	IDS_SKILL_HELP_2161,	IDS_SKILL_HELP_2162,	0,						0,						0					},
	{	IDS_SKILL_HELP_2163,	IDS_SKILL_HELP_2164,	0,						0,						0					},
	{	IDS_SKILL_HELP_2165,	IDS_SKILL_HELP_2166,	0,						0,						0					},
	{	IDS_SKILL_HELP_2167,	IDS_SKILL_HELP_2168,	0,						0,						0					},
	{	IDS_SKILL_HELP_2169,	IDS_SKILL_HELP_2170,	0,						0,						0					}
};

#endif

#define SMGR_RANK_TAG				"Ranks"
#define SMGR_RANK_DATA				"Rank"

#define SMGR_COST_TAG				"Cost"
#define SMGR_MP_TAG					"Multiplayer"

namespace
{
	char s_aTagName[30];
	char s_aAttName[100];

	char *szInternalSkillNames[kNumSkills] =
	{
		"Stealth",
		"Stamina",
		"Aim",
		"Carry",
		"Armor",
		"Weapon",
		"Gadget",
		"Search",
	};

}


CSkillsButeMgr* g_pSkillsButeMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkillsButeMgr::CSkillsButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSkillsButeMgr::CSkillsButeMgr()
{
	memset(m_nCosts,0,sizeof(m_nCosts));
	m_nIntelBonus = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkillsButeMgr::~CSkillsButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSkillsButeMgr::~CSkillsButeMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkillsButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSkillsButeMgr::Init(const char* szAttributeFile)
{
    if (g_pSkillsButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

	// Set up global pointer...

	g_pSkillsButeMgr = this;

	// Read in the properties for each mission...

	m_nNumRanks = 0;

	while (m_nNumRanks < SMGR_MAX_NUM_RANKS )
	{
		char szStr[512] ="";
		sprintf(s_aAttName, "%s%d", SMGR_RANK_DATA, m_nNumRanks);
		m_buteMgr.GetString(SMGR_RANK_TAG, s_aAttName, "", szStr,sizeof(szStr));
		if( !m_buteMgr.Success( ))
			break;

		char *pId = strtok(szStr,",");
		char *pMin = strtok(LTNULL,"");

		if (*pId && &pMin)
		{
			m_Ranks[m_nNumRanks].nNameId = atoi(pId);
			m_Ranks[m_nNumRanks].nMinScore = atoi(pMin);
		}	
		m_nNumRanks++;
		sprintf(s_aAttName, "%s%d", SMGR_RANK_DATA, m_nNumRanks);
	}

	int skl;
	for (skl = 0; skl < kNumSkills; skl++)
	{
		char szStr[512] ="";
		m_buteMgr.GetString(SMGR_COST_TAG, szInternalSkillNames[skl],szStr,sizeof(szStr));
		int lvl = 1;
		char *pCost = strtok(szStr,",");
		uint32 cost = 0;
			
		while (lvl < kNumSkillLevels && pCost)
		{
			cost = atol(pCost);
			m_nCosts[skl][lvl] = cost;
			pCost = strtok(NULL,",");
			lvl++;

		}

	}

	for (int lvl = 0; lvl < kNumSkillLevels; lvl++)
	{
		sprintf(s_aTagName, "Stealth%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Stealth[lvl].fModifier[StealthModifiers::eRadius] = (float)m_buteMgr.GetDouble(s_aTagName,"Radius",1.0);
			m_Stealth[lvl].fModifier[StealthModifiers::eHideTime] = (float)m_buteMgr.GetDouble(s_aTagName,"HideTime",1.0);
			m_Stealth[lvl].fModifier[StealthModifiers::eEvasion] = (float)m_buteMgr.GetDouble(s_aTagName,"Evasion",1.0);
		}

		sprintf(s_aTagName, "Stamina%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Stamina[lvl].fModifier[StaminaModifiers::eMaxHealth] = (float)m_buteMgr.GetDouble(s_aTagName,"MaxHealth",1.0);
			m_Stamina[lvl].fModifier[StaminaModifiers::eMoveDamage] = (float)m_buteMgr.GetDouble(s_aTagName,"MoveDamage",1.0);
			m_Stamina[lvl].fModifier[StaminaModifiers::eResistance] = (float)m_buteMgr.GetDouble(s_aTagName,"Resistance",1.0);
			m_Stamina[lvl].fModifier[StaminaModifiers::eHealthPickup] = (float)m_buteMgr.GetDouble(s_aTagName,"HealthPickup",1.0);
			m_Stamina[lvl].fModifier[StaminaModifiers::eDamage] = (float)m_buteMgr.GetDouble(s_aTagName,"Damage",1.0);
		}

		sprintf(s_aTagName, "Aim%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Aim[lvl].fModifier[AimModifiers::eAccuracy] = (float)m_buteMgr.GetDouble(s_aTagName,"Accuracy",1.0);
			m_Aim[lvl].fModifier[AimModifiers::eZoomSway] = (float)m_buteMgr.GetDouble(s_aTagName,"ZoomSway",1.0);
			m_Aim[lvl].fModifier[AimModifiers::eCorrection] = (float)m_buteMgr.GetDouble(s_aTagName,"Correction",1.0);
		}

		sprintf(s_aTagName, "Carry%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Carry[lvl].fModifier[CarryModifiers::eMaxAmmo] = (float)m_buteMgr.GetDouble(s_aTagName,"MaxAmmo",1.0);
			m_Carry[lvl].fModifier[CarryModifiers::eCarryBodyMovement] = (float)m_buteMgr.GetDouble(s_aTagName,"CarryBodyMovement",1.0);
		}

		sprintf(s_aTagName, "Armor%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Armor[lvl].fModifier[ArmorModifiers::eMaxArmor] = (float)m_buteMgr.GetDouble(s_aTagName,"MaxArmor",1.0);
			m_Armor[lvl].fModifier[ArmorModifiers::eArmorPickup] = (float)m_buteMgr.GetDouble(s_aTagName,"ArmorPickup",1.0);
		}

		sprintf(s_aTagName, "Weapon%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Weapon[lvl].fModifier[WeaponModifiers::eDamage] = (float)m_buteMgr.GetDouble(s_aTagName,"Damage",1.0);
			m_Weapon[lvl].fModifier[WeaponModifiers::eReload] = (float)m_buteMgr.GetDouble(s_aTagName,"Reload",1.0);
		}

		sprintf(s_aTagName, "Gadget%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Gadget[lvl].fModifier[GadgetModifiers::eEffect] = (float)m_buteMgr.GetDouble(s_aTagName,"Effect",1.0);
			m_Gadget[lvl].fModifier[GadgetModifiers::eSelect] = (float)m_buteMgr.GetDouble(s_aTagName,"Select",1.0);
		}

		sprintf(s_aTagName, "Search%d", lvl);
		if (m_buteMgr.Exist(s_aTagName))
		{
			m_Search[lvl].fModifier[SearchModifiers::eSearchSpeed] = (float)m_buteMgr.GetDouble(s_aTagName,"SearchSpeed",1.0);
			m_Search[lvl].fModifier[SearchModifiers::eFindJunk] = (float)m_buteMgr.GetDouble(s_aTagName,"FindJunk",1.0);
		}


	}

	

	m_nMPPool = 0;
	for (skl = 0; skl < kNumSkills; skl++)
	{
		int l = m_buteMgr.GetInt(SMGR_MP_TAG, szInternalSkillNames[skl],0);
		if (l >= kNumSkillLevels)
			l = kNumSkillLevels - 1;
		for (int lvl = 0; lvl <= l; lvl++)
		{
			m_nMPPool += m_nCosts[skl][lvl];
		}
		m_MPSkillLevel[skl] = (eSkillLevel)l;

	}

	m_nIntelBonus = m_buteMgr.GetInt("Bonus", "Intel", 0);


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkillsButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CSkillsButeMgr::Term()
{
    g_pSkillsButeMgr = LTNULL;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSkillsButeMgr::GetRank()
//
//	PURPOSE:	Calculate a rank based on a score
//
// ----------------------------------------------------------------------- //
const RANK* CSkillsButeMgr::GetRank(uint32 nScore)
{
	int n = m_nNumRanks-1;
	while (n > 0 && m_Ranks[n].nMinScore > nScore)
		n--;

	if (n < 0)
		return LTNULL;
	return &m_Ranks[n];
}

const SkillModifiers* CSkillsButeMgr::GetModifiers(eSkill skl, eSkillLevel lvl)
{
	switch (skl)
	{
		case SKL_STEALTH:
			return &m_Stealth[lvl];
		case SKL_STAMINA:
			return &m_Stamina[lvl];
		case SKL_AIM:
			return &m_Aim[lvl];
		case SKL_CARRY:
			return &m_Carry[lvl];
		case SKL_ARMOR:
			return &m_Armor[lvl];
		case SKL_WEAPON:
			return &m_Weapon[lvl];
		case SKL_GADGET:
			return &m_Gadget[lvl];
		case SKL_SEARCH:
			return &m_Search[lvl];
	}

	ASSERT(!"Invalid Skill specified.");
	return NULL;
}

float CSkillsButeMgr::GetModifier(eSkill skl, eSkillLevel lvl, uint8 nMod)
{
	uint8 nMax = GetNumModifiers(skl);

	if (nMod > nMax)
	{
		ASSERT(!"Invalid modifier specified.");
		return 1.0f;
	}

	const SkillModifiers* pMods = GetModifiers(skl, lvl);

	return pMods->fModifier[nMod];
}

#ifdef _CLIENTBUILD
uint16 CSkillsButeMgr::GetModifierNameId(eSkill skl, uint8 nMod)
{
	uint8 nMax = GetNumModifiers(skl);

	if (nMod > nMax)
	{
		ASSERT(!"Invalid modifier specified.");
		return 0;
	}


	return nModNames[skl][nMod];
}

uint16 CSkillsButeMgr::GetModifierDescriptionId(eSkill skl, uint8 nMod)
{
	uint8 nMax = GetNumModifiers(skl);

	if (nMod > nMax)
	{
		ASSERT(!"Invalid modifier specified.");
		return 0;
	}

	return nModDesc[skl][nMod];
}
#endif

uint8 CSkillsButeMgr::GetNumModifiers(eSkill skl)
{
	switch (skl)
	{
		case SKL_STEALTH:
			return StealthModifiers::kNumModifiers;
		case SKL_STAMINA:
			return StaminaModifiers::kNumModifiers;
		case SKL_AIM:
			return AimModifiers::kNumModifiers;
		case SKL_CARRY:
			return CarryModifiers::kNumModifiers;
		case SKL_ARMOR:
			return ArmorModifiers::kNumModifiers;
		case SKL_WEAPON:
			return WeaponModifiers::kNumModifiers;
		case SKL_GADGET:
			return GadgetModifiers::kNumModifiers;
		case SKL_SEARCH:
			return SearchModifiers::kNumModifiers;
	}

	ASSERT(!"Invalid Skill specified.");
	return 0;
}


