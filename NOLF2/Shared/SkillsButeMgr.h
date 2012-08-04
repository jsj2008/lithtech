// ----------------------------------------------------------------------- //
//
// MODULE  : SkillsButeMgr.h
//
// PURPOSE : Definition of bute mgr to manage skill related data
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SKILLS_BUTE_MGR_
#define _SKILLS_BUTE_MGR_

#include "GameButeMgr.h"
#include "ltbasetypes.h"

#define SMGR_DEFAULT_FILE		"Attributes\\Skills.txt"

enum eSkill
{
	SKL_STEALTH,
	SKL_STAMINA,
	SKL_AIM,
	SKL_CARRY,
	SKL_ARMOR,
	SKL_WEAPON,
	SKL_GADGET,
	SKL_SEARCH,
	kNumSkills
};

enum eSkillLevel
{
	SKL_NOVICE,
	SKL_AMATEUR,
	SKL_SKILLED,
	SKL_EXPERT,
	SKL_MASTER,
	kNumSkillLevels
};

#ifdef _CLIENTBUILD
const char* GetSkillName(eSkill skl);
const char* GetSkillDescription(eSkill skl);
const char* GetSkillLevelName(eSkillLevel lvl);
int			GetSkillNameId(eSkill skl);
int			GetSkillDescriptionId(eSkill skl);
#endif

#define SMGR_MAX_NUM_RANKS 10
struct RANK
{
	int		nNameId;
	uint16	nMinScore;
};


const uint8 kMaxModifiers = 5;

class SkillModifiers
{
public:
	SkillModifiers()
		{ for (uint8 m = 0; m < kMaxModifiers; ++m) fModifier[m] = 1.0f;}
	float	fModifier[kMaxModifiers];
};



class StealthModifiers : public SkillModifiers
{
public:
	StealthModifiers() {}
	enum eMods
	{
		eRadius,		//	Sound radii for footsteps, weapon reload, etc.
		eHideTime,		//	How quickly player disappears into shadows
		eEvasion,		//	How effectively player ditches pursuers
		kNumModifiers
	};

};

class StaminaModifiers : public SkillModifiers
{
public:
	StaminaModifiers() {}
	enum eMods
	{
		eMaxHealth,			//	The size of player health meter
		eMoveDamage,		//	How much player movement is affected by damage
		eResistance,		//	Resistance to certain status effects (i.e. poison, burn damage, bleeding, etc.)
		eHealthPickup,		//	Multiplier for how many health points the player gets from a helth pickup
		eDamage,			//	Multiplier for how much damage the player takes
		kNumModifiers
	};

};

class AimModifiers : public SkillModifiers
{
public:
	AimModifiers() {}
	enum eMods
	{
		eAccuracy,		//	The amount of weapon perturb
		eZoomSway,		//	How much player's view swims when zoomed in
		eCorrection,	//	How quickly player's perturb recovers after moving.
		kNumModifiers
	};
};

class CarryModifiers : public SkillModifiers
{
public:
	CarryModifiers() {}
	enum eMods
	{
		eMaxAmmo,				//	How much ammunition the player can carry
		eCarryBodyMovement,		//	Affects movement modifiers for carrying bodies
		kNumModifiers
	};
};

class ArmorModifiers : public SkillModifiers
{
public:
	ArmorModifiers() {}
	enum eMods
	{
		eMaxArmor,		//	The size of the player armor meter
		eArmorPickup,	//	Multiplier for how many armor points the player gets from an armor pickup
		kNumModifiers
	};
};

class WeaponModifiers : public SkillModifiers
{
public:
	WeaponModifiers() {}
	enum eMods
	{
		eDamage,		//	The amount of damage caused by a weapon
		eReload,		//	Weapon reload and select times
		kNumModifiers
	};
};

class GadgetModifiers : public SkillModifiers
{
public:
	GadgetModifiers() {}
	enum eMods
	{
		eEffect,		//	Effectiveness of the gadget
		eSelect,		//	Gadget select times
		kNumModifiers
	};
};

class SearchModifiers : public SkillModifiers
{
public:
	SearchModifiers() {}
	enum eMods
	{
		eSearchSpeed,	//	How quickly player searches things
		eFindJunk,		//	Reduces chance of finding worthless items when searching
		kNumModifiers
	};
};


class CSkillsButeMgr : public CGameButeMgr
{
public :

	CSkillsButeMgr();
	~CSkillsButeMgr();

    LTBOOL      Init(const char* szAttributeFile=SMGR_DEFAULT_FILE);
	void		Term();

    void        Reload() { Term(); Init(); }

	const RANK*	GetRank(uint32 nScore);

	uint32		GetIntelSkillBonus() {return m_nIntelBonus; }

	uint32		GetCostToUpgrade(eSkill skl, eSkillLevel tgtLevel) {return m_nCosts[skl][tgtLevel];}

	bool		IsAvailable(eSkill skl) {return (m_nCosts[skl][SKL_AMATEUR] > 0);}

	const SkillModifiers*	GetModifiers(eSkill skl, eSkillLevel lvl);
	float					GetModifier(eSkill skl, eSkillLevel lvl, uint8 nMod);
	uint8					GetNumModifiers(eSkill skl);

#ifdef _CLIENTBUILD
	uint16					GetModifierNameId(eSkill skl, uint8 nMod);
	uint16					GetModifierDescriptionId(eSkill skl, uint8 nMod);
#endif

	uint32		GetMultiplayerPool() {return m_nMPPool;}
	eSkillLevel GetDefaultMultiplayerLevel(eSkill skl) {return m_MPSkillLevel[skl];}


private:

	int			m_nNumRanks;
	RANK		m_Ranks[SMGR_MAX_NUM_RANKS];

	uint32		m_nCosts[kNumSkills][kNumSkillLevels];

	uint32		m_nIntelBonus;

	uint32		m_nMPPool;
	eSkillLevel m_MPSkillLevel[kNumSkills];

	StealthModifiers	m_Stealth[kNumSkillLevels];
	StaminaModifiers	m_Stamina[kNumSkillLevels];
	AimModifiers		m_Aim[kNumSkillLevels];
	CarryModifiers		m_Carry[kNumSkillLevels];
	ArmorModifiers		m_Armor[kNumSkillLevels];
	WeaponModifiers		m_Weapon[kNumSkillLevels];
	GadgetModifiers		m_Gadget[kNumSkillLevels];
	SearchModifiers		m_Search[kNumSkillLevels];
};

extern CSkillsButeMgr* g_pSkillsButeMgr;	



#endif