// ----------------------------------------------------------------------- //
//
// MODULE  : MissionMgr.h
//
// PURPOSE : MissionMgr definition - Controls attributes of all Missions
//
// CREATED : 07/26/99
//
// ----------------------------------------------------------------------- //

#ifndef __MISSION_MGR_H__
#define __MISSION_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "TemplateList.h"

#define MISSION_DEFAULT_FILE		"Attributes\\Missions.txt"


#define MMGR_INVALID_ID			255	// Invalid id
#define MISSION_DEFAULT_ID		0	// First Level in Mission attribute file

class CMissionMgr;
extern CMissionMgr* g_pMissionMgr;

#define MMGR_MAX_FILE_PATH			64
#define MMGR_MAX_LEVEL_FILE_PATH	34
#define MMGR_MAX_MISSION_LEVELS		10
#define MMGR_MAX_WEAPONS			25
#define MMGR_MAX_GADGETS			10
#define MMGR_MAX_AMMO				25
#define MMGR_MAX_MODS				25
#define MMGR_MAX_GEAR				10
#define MMGR_MAX_OBJECTIVES			10
#define	MMGR_MAX_RANKS				10
#define	MMGR_MAX_AWARDS				10

struct LEVEL
{
	LEVEL() { szLevel[0] = '\0'; }

	char	szLevel[MMGR_MAX_LEVEL_FILE_PATH];
};

struct RANKBONUS
{
	RANKBONUS();
	void Reset();

	uint8 nReputationPoints;
	uint8 nHealthPoints;
	uint8 nArmorPoints;
	uint8 nAmmoPoints;
	uint8 nPerturbPoints;
	uint8 nDamagePoints;
	uint8 nStealthPoints;
};

struct MISSION
{
	MISSION();
    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

    LTBOOL   IsOneTimeAmmo(uint8 nAmmoId);
    LTBOOL   IsOneTimeWeapon(uint8 nWeaponId);
    LTBOOL   IsOneTimeGadget(uint8 nGadgetId);
    LTBOOL   IsOneTimeGear(uint8 nGearId);
    void    GetRankBonus(LTFLOAT fScorePercent, RANKBONUS *pBonus);

	int		nId;
	int		nNameId;
	int		nDescriptionId;
	int		nBriefingId;
	int		nNumObjectives;
	int		aObjectiveIds[MMGR_MAX_OBJECTIVES];

	int		nNumWeapons;
	int		nNumGadgets;
	int		nNumAmmo;
	int		nNumSupplies;
	int		nNumWeaponMods;
	int		nNumGear;

	char	szPhoto[MMGR_MAX_FILE_PATH];

	int		nNumDefaultWeapons;
	int		aDefaultWeapons[MMGR_MAX_WEAPONS];

	int		nNumAllowedWeapons;
	int		aAllowedWeapons[MMGR_MAX_WEAPONS];

	int		nNumRequiredWeapons;
	int		aRequiredWeapons[MMGR_MAX_WEAPONS];

	int		nNumOneTimeWeapons;
	int		aOneTimeWeapons[MMGR_MAX_WEAPONS];

	int		nNumDeniedWeapons;
	int		aDeniedWeapons[MMGR_MAX_WEAPONS];

	int		nNumDefaultGadgets;
	int		aDefaultGadgets[MMGR_MAX_GADGETS];

	int		nNumAllowedGadgets;
	int		aAllowedGadgets[MMGR_MAX_GADGETS];

	int		nNumRequiredGadgets;
	int		aRequiredGadgets[MMGR_MAX_GADGETS];

	int		nNumOneTimeGadgets;
	int		aOneTimeGadgets[MMGR_MAX_GADGETS];

	int		nNumDeniedGadgets;
	int		aDeniedGadgets[MMGR_MAX_GADGETS];

	int		nNumAllowedAmmo;
	int		aAllowedAmmo[MMGR_MAX_AMMO];

	int		nNumRequiredAmmo;
	int		aRequiredAmmo[MMGR_MAX_AMMO];

	int		nNumOneTimeAmmo;
	int		aOneTimeAmmo[MMGR_MAX_AMMO];

	int		nNumDeniedAmmo;
	int		aDeniedAmmo[MMGR_MAX_AMMO];

	int		nNumAllowedMods;
	int		aAllowedMods[MMGR_MAX_MODS];

	int		nNumDefaultMods;
	int		aDefaultMods[MMGR_MAX_MODS];

	int		nNumRequiredMods;
	int		aRequiredMods[MMGR_MAX_MODS];

	int		nNumAllowedGear;
	int		aAllowedGear[MMGR_MAX_GEAR];

	int		nNumRequiredGear;
	int		aRequiredGear[MMGR_MAX_GEAR];

	int		nNumOneTimeGear;
	int		aOneTimeGear[MMGR_MAX_GEAR];

	int		nNumDeniedGear;
	int		aDeniedGear[MMGR_MAX_GEAR];

	int		nNumLevels;
	LEVEL	aLevels[MMGR_MAX_MISSION_LEVELS];

	RANKBONUS	aRankBonus[MMGR_MAX_RANKS];

    LTBOOL   bAllowAwards;
	int		nHighAmmo;
	int		nLowAmmo;
	int		nMaxDetect;

private :

	int		BuildObjectivesList(CString str, int* pArray, int nArrayLen);
	int		BuildWeaponsList(CString str, int* pArray, int nArrayLen);
	int		BuildGadgetsList(CString str, int* pArray, int nArrayLen);
	int		BuildAmmoList(CString str, int* pArray, int nArrayLen);
	int		BuildModsList(CString str, int* pArray, int nArrayLen);
	int		BuildGearList(CString str, int* pArray, int nArrayLen);
};

inline LTBOOL MISSION::IsOneTimeAmmo(uint8 nAmmoId)
{
	for (int i=0; i < nNumOneTimeAmmo; i++)
	{
        if (aOneTimeAmmo[i] == nAmmoId) return LTTRUE;
	}

    return LTFALSE;
}

inline LTBOOL MISSION::IsOneTimeWeapon(uint8 nWeaponId)
{
	for (int i=0; i < nNumOneTimeWeapons; i++)
	{
        if (aOneTimeWeapons[i] == nWeaponId) return LTTRUE;
	}

    return LTFALSE;
}
inline LTBOOL MISSION::IsOneTimeGadget(uint8 nGadgetId)
{
	for (int i=0; i < nNumOneTimeGadgets; i++)
	{
        if (aOneTimeGadgets[i] == nGadgetId) return LTTRUE;
	}

    return LTFALSE;
}
inline LTBOOL MISSION::IsOneTimeGear(uint8 nGearId)
{
	for (int i=0; i < nNumOneTimeGear; i++)
	{
        if (aOneTimeGear[i] == nGearId) return LTTRUE;
	}

    return LTFALSE;
}



struct RANKDATA
{
	RANKDATA();

	int		nNameId;
    LTFLOAT  fPercent;
};

struct MISSIONRATING
{
	MISSIONRATING();
    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

    uint8   nSuccessPts;
    uint8   nIntelPts;

    uint8   nReputationInc;
    uint8   nReputationMax;

    LTFLOAT  fHealthInc;
    LTFLOAT  fHealthMax;
    LTFLOAT  fArmorInc;
    LTFLOAT  fArmorMax;
    LTFLOAT  fAmmoInc;
    LTFLOAT  fAmmoMax;
    LTFLOAT  fPerturbInc;
    LTFLOAT  fPerturbMin;
    LTFLOAT  fDamageInc;
    LTFLOAT  fDamageMax;
    LTFLOAT  fStealthInc;
    LTFLOAT  fStealthMin;

	int			nNumRanks;
	RANKDATA	aRanks[MMGR_MAX_RANKS];

    int         GetRank(LTFLOAT fScorePercent);
    int         GetRankId(LTFLOAT fScorePercent) {return aRanks[GetRank(fScorePercent)].nNameId;}

};

struct MISSIONAWARDS
{
	MISSIONAWARDS();
    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

    LTFLOAT  fAccuracyPct;
    LTFLOAT  fMarksmanPct;

	int		nNumHighAmmoAwards;
	int		aHighAmmoAwards[MMGR_MAX_AWARDS];

	int		nNumLowAmmoAwards;
	int		aLowAmmoAwards[MMGR_MAX_AWARDS];

	int		nNumMarksmanAwards;
	int		aMarksmanAwards[MMGR_MAX_AWARDS];

	int		nNumAccuracyAwards;
	int		aAccuracyAwards[MMGR_MAX_AWARDS];

	int		nNumNonInjuryAwards;
	int		aNonInjuryAwards[MMGR_MAX_AWARDS];

	int		nNumNotShotAwards;
	int		aNotShotAwards[MMGR_MAX_AWARDS];

	int		nNumStealthAwards;
	int		aStealthAwards[MMGR_MAX_AWARDS];

private:

	int		BuildAwardsList(CString str, int* pArray, int nArrayLen);

};


typedef CTList<MISSION*> MissionList;

class CMissionMgr : public CGameButeMgr
{
	public :

		CMissionMgr();
		~CMissionMgr();

        LTBOOL           Init(ILTCSBase *pInterface, const char* szAttributeFile=MISSION_DEFAULT_FILE);
		void			Term();

        void            Reload(ILTCSBase *pInterface) { Term(); Init(pInterface); }

		int				GetNumMissions() const { return m_MissionList.GetLength(); }
		MISSION*		GetMission(int nMissionId);

        LTBOOL           IsMissionLevel(char* pWorldFile, int & nMissionId, int & nLevel);

		MISSIONRATING*	GetMissionRating() {return &m_MissionRating;}
		MISSIONAWARDS*	GetMissionAwards() {return &m_MissionAwards;}

	protected :

		MissionList		m_MissionList;
		MISSIONRATING	m_MissionRating;
		MISSIONAWARDS	m_MissionAwards;
};

#endif // __MISSION_MGR_H__