// ----------------------------------------------------------------------- //
//
// MODULE  : MissionMgr.cpp
//
// PURPOSE : MissionMgr implementation - Controls attributes of all
//			 missions
//
// CREATED : 07/26/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MissionMgr.h"
#include "CommonUtilities.h"
#include "WeaponMgr.h"

#define MMGR_MISSIONRATING_TAG			"MissionRating"

#define MMGR_MISSIONRATING_SUCCESSPTS	"SuccessPoints"
#define MMGR_MISSIONRATING_INTELPTS		"IntelPoints"
#define MMGR_MISSIONRATING_REPINC		"ReputationInc"
#define MMGR_MISSIONRATING_REPMAX		"ReputationMax"
#define MMGR_MISSIONRATING_HEALTHINC	"HealthInc"
#define MMGR_MISSIONRATING_ARMORINC		"ArmorInc"
#define MMGR_MISSIONRATING_AMMOINC		"AmmoInc"
#define MMGR_MISSIONRATING_PERTURBINC	"PerturbInc"
#define MMGR_MISSIONRATING_DAMAGEINC	"DamageInc"
#define MMGR_MISSIONRATING_STEALTHINC	"StealthInc"
#define MMGR_MISSIONRATING_HEALTHMAX	"HealthMax"
#define MMGR_MISSIONRATING_ARMORMAX		"ArmorMax"
#define MMGR_MISSIONRATING_AMMOMAX		"AmmoMax"
#define MMGR_MISSIONRATING_DAMAGEMAX	"DamageMax"
#define MMGR_MISSIONRATING_PERTURBMIN	"PerturbMin"
#define MMGR_MISSIONRATING_STEALTHMIN	"StealthMin"


#define MMGR_RANK_TAG					"Rank"

#define MMGR_RANK_PERCENT				"Percent"
#define MMGR_RANK_ID					"Id"


#define MMGR_MISSION_TAG				"Mission"

#define MMGR_MISSION_NAMEID				"NameId"
#define MMGR_MISSION_DESCRIPTIONID		"DescriptionId"
#define MMGR_MISSION_BRIEFINGID			"BriefingId"
#define MMGR_MISSION_OBJECTIVEIDS		"ObjectiveIds"
#define MMGR_MISSION_PHOTO				"Photo"
#define MMGR_MISSION_WEAPONS_DEFAULT	"DefaultWeapons"
#define MMGR_MISSION_WEAPONS_ALLOWED	"AllowedWeapons"
#define MMGR_MISSION_WEAPONS_REQUIRED	"RequiredWeapons"
#define MMGR_MISSION_WEAPONS_ONETIME	"OneTimeWeapons"
#define MMGR_MISSION_WEAPONS_DENIED		"DeniedWeapons"
#define MMGR_MISSION_GADGETS_DEFAULT	"DefaultGadgets"
#define MMGR_MISSION_GADGETS_ALLOWED	"AllowedGadgets"
#define MMGR_MISSION_GADGETS_REQUIRED	"RequiredGadgets"
#define MMGR_MISSION_GADGETS_ONETIME	"OneTimeGadgets"
#define MMGR_MISSION_GADGETS_DENIED		"DeniedGadgets"
#define MMGR_MISSION_AMMO_ALLOWED		"AllowedAmmo"
#define MMGR_MISSION_AMMO_REQUIRED		"RequiredAmmo"
#define MMGR_MISSION_AMMO_ONETIME		"OneTimeAmmo"
#define MMGR_MISSION_AMMO_DENIED		"DeniedAmmo"
#define MMGR_MISSION_MODS_ALLOWED		"AllowedMods"
#define MMGR_MISSION_MODS_DEFAULT		"DefaultMods"
#define MMGR_MISSION_MODS_REQUIRED		"RequiredMods"
#define MMGR_MISSION_GEAR_ALLOWED		"AllowedGear"
#define MMGR_MISSION_GEAR_REQUIRED		"RequiredGear"
#define MMGR_MISSION_GEAR_ONETIME		"OneTimeGear"
#define MMGR_MISSION_GEAR_DENIED		"DeniedGear"
#define MMGR_MISSION_NUMWEAPONS			"NumWeapons"
#define MMGR_MISSION_NUMGADGETS			"NumGadgets"
#define MMGR_MISSION_NUMAMMO			"NumAmmo"
#define MMGR_MISSION_NUMSUPPLIES		"NumSupplies"
#define MMGR_MISSION_NUMWEAPONMODS		"NumWeaponMods"
#define MMGR_MISSION_NUMGEAR			"NumGear"
#define MMGR_MISSION_LEVEL				"Level"
#define MMGR_MISSION_RANK				"Rank"
#define MMGR_MISSION_AWARDS				"AllowAwards"
#define MMGR_MISSION_HIGHAMMO			"HighAmmoUsage"
#define MMGR_MISSION_LOWAMMO			"LowAmmoUsage"
#define MMGR_MISSION_MAXDETECT			"MaxDetections"

#define MMGR_AWARDS_TAG					"Awards"

#define MMGR_AWARDS_ACC_PCT				"Accuracy"
#define MMGR_AWARDS_MARK_PCT			"Marksman"
#define MMGR_AWARDS_HIGHAMMO			"HighAmmoAward"
#define MMGR_AWARDS_LOWAMMO				"LowAmmoAward"
#define MMGR_AWARDS_ACCURACY			"MarksmanAward"
#define MMGR_AWARDS_MARKSMAN			"AccuracyAward"
#define MMGR_AWARDS_NONINJURY			"NonInjuryAward"
#define MMGR_AWARDS_NOTSHOT				"NotShotAward"
#define MMGR_AWARDS_STEALTH				"StealthAward"


static char s_aTagName[30];
static char s_aAttName[100];
static char s_FileBuffer[MAX_CS_FILENAME_LEN];

CMissionMgr* g_pMissionMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::CMissionMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMissionMgr::CMissionMgr()
{
    m_MissionList.Init(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::~CMissionMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CMissionMgr::~CMissionMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CMissionMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pMissionMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	// Set up global pointer...

	g_pMissionMgr = this;


	// Read in the mission rating properties...

	m_MissionRating.Init(m_buteMgr, MMGR_MISSIONRATING_TAG);

	// Read in the mission awards properties...

	m_MissionAwards.Init(m_buteMgr, MMGR_AWARDS_TAG);


	// Read in the properties for each mission...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", MMGR_MISSION_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		MISSION* pMission = debug_new(MISSION);

		if (pMission && pMission->Init(m_buteMgr, s_aTagName))
		{
			pMission->nId = nNum;
			m_MissionList.AddTail(pMission);
		}
		else
		{
			debug_delete(pMission);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", MMGR_MISSION_TAG, nNum);
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CMissionMgr::Term()
{
    g_pMissionMgr = LTNULL;

	m_MissionList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::GetMission
//
//	PURPOSE:	Get the specified mission
//
// ----------------------------------------------------------------------- //

MISSION* CMissionMgr::GetMission(int nMissionId)
{
    if (nMissionId < 0 || nMissionId > m_MissionList.GetLength()) return LTNULL;

    MISSION** pCur  = LTNULL;

	pCur = m_MissionList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nMissionId)
		{
			return *pCur;
		}

		pCur = m_MissionList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::IsMissionLevel()
//
//	PURPOSE:	Determine if the passed in string is a level of one of the
//				missions.
//
//	RETURNS:	True if a mission level, else false.  Also nMissionId and
//				nLevel are filled in with the correct values.
//
// ----------------------------------------------------------------------- //

LTBOOL CMissionMgr::IsMissionLevel(char* pWorldFile, int & nMissionId, int & nLevel)
{
    if (!pWorldFile) return LTFALSE;

    MISSION** pCur  = LTNULL;

	pCur = m_MissionList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur)
		{
			for (int i=0; i < (*pCur)->nNumLevels; i++)
			{
				if (stricmp((*pCur)->aLevels[i].szLevel, pWorldFile) == 0)
				{
					nMissionId = (*pCur)->nId;
					nLevel = i;
                    return LTTRUE;
				}
			}
		}

		pCur = m_MissionList.GetItem(TLIT_NEXT);
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::MISSION
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

MISSION::MISSION()
{
	nId					= -1;

	nNameId				= -1;
	nDescriptionId		= -1;
	nBriefingId			= -1;
	nNumObjectives		= 0;
	aObjectiveIds[0]	= -1;

	nNumWeapons			= -1;
	nNumGadgets			= -1;
	nNumAmmo			= -1;
	nNumSupplies		= -1;
	nNumWeaponMods		= -1;
	nNumGear			= -1;

	szPhoto[0]			= '\0';

	nNumDefaultWeapons	= 0;
	nNumAllowedWeapons	= 0;
	nNumRequiredWeapons	= 0;
	nNumOneTimeWeapons	= 0;
	nNumDeniedWeapons	= 0;

	nNumDefaultGadgets	= 0;
	nNumAllowedGadgets	= 0;
	nNumRequiredGadgets	= 0;
	nNumOneTimeGadgets	= 0;
	nNumDeniedGadgets	= 0;

	nNumAllowedAmmo		= 0;
	nNumRequiredAmmo	= 0;
	nNumOneTimeAmmo		= 0;
	nNumDeniedAmmo		= 0;

	aDefaultWeapons[0]	= WMGR_INVALID_ID;
	aAllowedWeapons[0]	= WMGR_INVALID_ID;
	aRequiredWeapons[0] = WMGR_INVALID_ID;
	aOneTimeWeapons[0]	= WMGR_INVALID_ID;
	aDeniedWeapons[0]	= WMGR_INVALID_ID;

	aDefaultGadgets[0]	= WMGR_INVALID_ID;
	aAllowedGadgets[0]	= WMGR_INVALID_ID;
	aRequiredGadgets[0] = WMGR_INVALID_ID;
	aOneTimeGadgets[0]	= WMGR_INVALID_ID;
	aDeniedGadgets[0]	= WMGR_INVALID_ID;

	aAllowedAmmo[0]		= WMGR_INVALID_ID;
	aRequiredAmmo[0]	= WMGR_INVALID_ID;
	aOneTimeAmmo[0]		= WMGR_INVALID_ID;
	aDeniedAmmo[0]		= WMGR_INVALID_ID;

    bAllowAwards        = LTFALSE;
	nHighAmmo			= -1;
	nLowAmmo			= -1;
	nMaxDetect			= -1;


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::Init
//
//	PURPOSE:	Build the mission struct
//
// ----------------------------------------------------------------------- //

LTBOOL MISSION::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nNameId			= buteMgr.GetInt(aTagName, MMGR_MISSION_NAMEID);
	nDescriptionId	= buteMgr.GetInt(aTagName, MMGR_MISSION_DESCRIPTIONID);
	nBriefingId		= buteMgr.GetInt(aTagName, MMGR_MISSION_BRIEFINGID);

	CString str		= buteMgr.GetString(aTagName, MMGR_MISSION_OBJECTIVEIDS);
	nNumObjectives	= BuildObjectivesList(str, aObjectiveIds, ARRAY_LEN(aObjectiveIds));

	nNumWeapons		= buteMgr.GetInt(aTagName, MMGR_MISSION_NUMWEAPONS);
	nNumGadgets		= buteMgr.GetInt(aTagName, MMGR_MISSION_NUMGADGETS);
	nNumAmmo		= buteMgr.GetInt(aTagName, MMGR_MISSION_NUMAMMO);
	nNumSupplies	= buteMgr.GetInt(aTagName, MMGR_MISSION_NUMSUPPLIES);
	nNumWeaponMods	= buteMgr.GetInt(aTagName, MMGR_MISSION_NUMWEAPONMODS);
	nNumGear		= buteMgr.GetInt(aTagName, MMGR_MISSION_NUMGEAR);

    bAllowAwards    = (LTBOOL) buteMgr.GetInt(aTagName, MMGR_MISSION_AWARDS);
	nHighAmmo		= buteMgr.GetInt(aTagName, MMGR_MISSION_HIGHAMMO);
	nLowAmmo		= buteMgr.GetInt(aTagName, MMGR_MISSION_LOWAMMO);
	nMaxDetect		= buteMgr.GetInt(aTagName, MMGR_MISSION_MAXDETECT);

	str = buteMgr.GetString(aTagName, MMGR_MISSION_PHOTO);
	if (!str.IsEmpty())
	{
		strncpy(szPhoto, (char*)(LPCSTR)str, ARRAY_LEN(szPhoto));
	}

	str = buteMgr.GetString(aTagName, MMGR_MISSION_WEAPONS_DEFAULT);
	nNumDefaultWeapons = BuildWeaponsList(str, aDefaultWeapons, ARRAY_LEN(aDefaultWeapons));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_WEAPONS_ALLOWED);
	nNumAllowedWeapons = BuildWeaponsList(str, aAllowedWeapons, ARRAY_LEN(aAllowedWeapons));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_WEAPONS_REQUIRED);
	nNumRequiredWeapons = BuildWeaponsList(str, aRequiredWeapons, ARRAY_LEN(aRequiredWeapons));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_WEAPONS_ONETIME);
	nNumOneTimeWeapons = BuildWeaponsList(str, aOneTimeWeapons, ARRAY_LEN(aOneTimeWeapons));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_WEAPONS_DENIED);
	nNumDeniedWeapons = BuildWeaponsList(str, aDeniedWeapons, ARRAY_LEN(aDeniedWeapons));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_GADGETS_DEFAULT);
	nNumDefaultGadgets = BuildGadgetsList(str, aDefaultGadgets, ARRAY_LEN(aDefaultGadgets));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_GADGETS_ALLOWED);
	nNumAllowedGadgets = BuildGadgetsList(str, aAllowedGadgets, ARRAY_LEN(aAllowedGadgets));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_GADGETS_REQUIRED);
	nNumRequiredGadgets = BuildGadgetsList(str, aRequiredGadgets, ARRAY_LEN(aRequiredGadgets));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_GADGETS_ONETIME);
	nNumOneTimeGadgets = BuildGadgetsList(str, aOneTimeGadgets, ARRAY_LEN(aOneTimeGadgets));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_GADGETS_DENIED);
	nNumDeniedGadgets = BuildGadgetsList(str, aDeniedGadgets, ARRAY_LEN(aDeniedGadgets));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_AMMO_ALLOWED);
	nNumAllowedAmmo = BuildAmmoList(str, aAllowedAmmo, ARRAY_LEN(aAllowedAmmo));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_AMMO_REQUIRED);
	nNumRequiredAmmo = BuildAmmoList(str, aRequiredAmmo, ARRAY_LEN(aRequiredAmmo));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_AMMO_ONETIME);
	nNumOneTimeAmmo = BuildAmmoList(str, aOneTimeAmmo, ARRAY_LEN(aOneTimeAmmo));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_AMMO_DENIED);
	nNumDeniedAmmo = BuildAmmoList(str, aDeniedAmmo, ARRAY_LEN(aDeniedAmmo));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_MODS_ALLOWED);
	nNumAllowedMods = BuildModsList(str, aAllowedMods, ARRAY_LEN(aAllowedMods));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_MODS_DEFAULT);
	nNumDefaultMods = BuildModsList(str, aDefaultMods, ARRAY_LEN(aDefaultMods));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_MODS_DEFAULT);
	nNumDefaultMods = BuildModsList(str, aDefaultMods, ARRAY_LEN(aDefaultMods));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_MODS_REQUIRED);
	nNumRequiredMods = BuildModsList(str, aRequiredMods, ARRAY_LEN(aRequiredMods));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_GEAR_ALLOWED);
	nNumAllowedGear = BuildGearList(str, aAllowedGear, ARRAY_LEN(aAllowedGear));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_GEAR_REQUIRED);
	nNumRequiredGear = BuildGearList(str, aRequiredGear, ARRAY_LEN(aRequiredGear));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_GEAR_ONETIME);
	nNumOneTimeGear = BuildGearList(str, aOneTimeGear, ARRAY_LEN(aOneTimeGear));

	str = buteMgr.GetString(aTagName, MMGR_MISSION_GEAR_DENIED);
	nNumDeniedGear = BuildGearList(str, aDeniedGear, ARRAY_LEN(aDeniedGear));

	nNumLevels = 0;
	sprintf(s_aAttName, "%s%d", MMGR_MISSION_LEVEL, nNumLevels);

	while (buteMgr.Exist(aTagName, s_aAttName) && nNumLevels < MMGR_MAX_MISSION_LEVELS)
	{
		CString str = buteMgr.GetString(s_aTagName, s_aAttName);
		if (!str.IsEmpty())
		{
			strncpy(aLevels[nNumLevels].szLevel, (char*)(LPCSTR)str, ARRAY_LEN(aLevels[nNumLevels].szLevel));
		}

		nNumLevels++;
		sprintf(s_aAttName, "%s%d", MMGR_MISSION_LEVEL, nNumLevels);
	}

	char szTemp[256];
	char* pszTok;
	char* pszNum;
	for (int nRank = 0; nRank < MMGR_MAX_RANKS; nRank++)
	{
		sprintf(s_aAttName, "%s%d", MMGR_MISSION_RANK, nRank);

		if (buteMgr.Exist(aTagName, s_aAttName))
		{
			CString str = buteMgr.GetString(s_aTagName, s_aAttName);
			if (!str.IsEmpty())
			{
				strncpy(szTemp, (char*)(LPCSTR)str, ARRAY_LEN(szTemp));


				pszTok = strtok(szTemp,",");

				while (pszTok)
				{
					pszTok += strspn(pszTok," \t");
					pszNum = strpbrk(pszTok,"0123456789");

					if (*pszTok && pszNum)
					{
						switch (toupper(*pszTok))
						{
						case ('R'):
							aRankBonus[nRank].nReputationPoints = (uint8)atoi(pszNum);
							break;
						case ('H'):
							aRankBonus[nRank].nHealthPoints = (uint8)atoi(pszNum);
							break;
						case ('A'):
							if (toupper(*(pszTok+1)) == 'R')
								aRankBonus[nRank].nArmorPoints = (uint8)atoi(pszNum);
							else if (toupper(*(pszTok+1)) == 'M')
								aRankBonus[nRank].nAmmoPoints = (uint8)atoi(pszNum);
							break;
						case ('P'):
							aRankBonus[nRank].nPerturbPoints = (uint8)atoi(pszNum);
							break;
						case ('D'):
							aRankBonus[nRank].nDamagePoints = (uint8)atoi(pszNum);
							break;
						case ('S'):
							aRankBonus[nRank].nStealthPoints = (uint8)atoi(pszNum);
							break;
						}
					}

					pszTok = strtok(NULL,",");
				}


			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildObjectivesList()
//
//	PURPOSE:	Builds an array of ints from the string associated with
//					the given string;
//
//	RETURNS:	Number of objectives added to list
//
// ----------------------------------------------------------------------- //

int MISSION::BuildObjectivesList(CString str, int* pArray, int nArrayLen)
{
	if (!pArray) return 0;

	int  nNumObj = 0;
	char buf[512] = "";

	if (!str.IsEmpty())
	{
		strncpy(buf, (char*)(LPCSTR)str, 512);
		char *pObj = strtok(buf,",");
		while (pObj && nNumObj < nArrayLen)
		{
			int objId = atoi(pObj);
			if (objId > 0)
			{
				pArray[nNumObj] = objId;
				nNumObj++;
			}
			pObj = strtok(NULL,",");
		}
	}

	return nNumObj;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildWeaponsList()
//
//	PURPOSE:	Builds an array of ints from the string associated with
//					the given string;
//
//	RETURNS:	Number of weapons added to list
//
// ----------------------------------------------------------------------- //

int MISSION::BuildWeaponsList(CString str, int* pArray, int nArrayLen)
{
	if (!pArray) return 0;

	int  nNumWeapons = 0;
	char buf[512] = "";

	if (!str.IsEmpty())
	{
		strncpy(buf, (char*)(LPCSTR)str, 512);
		char *pWpnName = strtok(buf,",");
		while (pWpnName && nNumWeapons < nArrayLen)
		{
			WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(pWpnName);
			if (pWeapon)
			{
				if (!pWeapon->IsAGadget())
				{
					pArray[nNumWeapons] = pWeapon->nId;
					nNumWeapons++;
				}
				else
				{
#ifdef _CLIENTBUILD
                    g_pLTClient->CPrint("Error in MISSIONS.TXT: %s is not a weapon.",pWpnName);
#else
                    g_pLTServer->CPrint("Error in MISSIONS.TXT: %s is not a weapon.",pWpnName);
#endif
				}

			}
			else
			{
#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in MISSIONS.TXT: Unknown weapon: %s",pWpnName);
#else
                g_pLTServer->CPrint("Error in MISSIONS.TXT: Unknown weapon: %s",pWpnName);
#endif
            }
			pWpnName = strtok(NULL,",");
		}
	}

	return nNumWeapons;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildGadgetsList()
//
//	PURPOSE:	Builds an array of ints from the string associated with
//					the given string;
//
//	RETURNS:	Number of gadgets added to list
//
// ----------------------------------------------------------------------- //

int MISSION::BuildGadgetsList(CString str, int* pArray, int nArrayLen)
{
	if (!pArray) return 0;

	int  nNumGadgets = 0;
	char buf[512] = "";

	if (!str.IsEmpty())
	{
		strncpy(buf, (char*)(LPCSTR)str, 512);
		char *pGadName = strtok(buf,",");
		while (pGadName && nNumGadgets < nArrayLen)
		{
			WEAPON* pGadget = g_pWeaponMgr->GetWeapon(pGadName);
			if (pGadget)
			{
				if (pGadget->IsAGadget())
				{
					pArray[nNumGadgets] = pGadget->nId;
					nNumGadgets++;
				}
				else
				{
#ifdef _CLIENTBUILD
                    g_pLTClient->CPrint("Error in MISSIONS.TXT: %s is not a gadget.",pGadName);
#else
                    g_pLTServer->CPrint("Error in MISSIONS.TXT: %s is not a gadget.",pGadName);
#endif
				}
			}
			else
			{
#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in MISSIONS.TXT: Unknown gadget: %s",pGadName);
#else
                g_pLTServer->CPrint("Error in MISSIONS.TXT: Unknown gadget: %s",pGadName);
#endif
			}

			pGadName = strtok(NULL,",");
		}
	}

	return nNumGadgets;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildAmmoList()
//
//	PURPOSE:	Builds an array of ints from the string associated with
//					the given string;
//
//	RETURNS:	Number of ammo added to list
//
// ----------------------------------------------------------------------- //

int MISSION::BuildAmmoList(CString str, int* pArray, int nArrayLen)
{
	if (!pArray) return 0;

	int  nNumAmmo = 0;
	char buf[512] = "";

	if (!str.IsEmpty())
	{
		strncpy(buf, (char*)(LPCSTR)str, 512);
		char *pAmmoName = strtok(buf,",");
		while (pAmmoName && nNumAmmo < nArrayLen)
		{
			AMMO *pAmmo = g_pWeaponMgr->GetAmmo(pAmmoName);
			if (pAmmo)
			{
				pArray[nNumAmmo] = pAmmo->nId;
				nNumAmmo++;
			}
			else
			{
#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in MISSIONS.TXT: Unknown ammo: %s",pAmmoName);
#else
                g_pLTServer->CPrint("Error in MISSIONS.TXT: Unknown ammo: %s",pAmmoName);
#endif
			}

			pAmmoName = strtok(NULL,",");
		}
	}

	return nNumAmmo;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildModsList()
//
//	PURPOSE:	Builds an array of ints from the string associated with
//					the given string;
//
//	RETURNS:	Number of mods added to list
//
// ----------------------------------------------------------------------- //

int MISSION::BuildModsList(CString str, int* pArray, int nArrayLen)
{
	if (!pArray) return 0;

	int  nNumMods = 0;
	char buf[512] = "";

	if (!str.IsEmpty())
	{
		strncpy(buf, (char*)(LPCSTR)str, 512);
		char *pModName = strtok(buf,",");
		while (pModName && nNumMods < nArrayLen)
		{
			MOD *pMod = g_pWeaponMgr->GetMod(pModName);
			if (pMod)
			{
				pArray[nNumMods] = pMod->nId;
				nNumMods++;
			}
			else
			{
#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in MISSIONS.TXT: Unknown weapon mod: %s",pModName);
#else
                g_pLTServer->CPrint("Error in MISSIONS.TXT: Unknown weapon mod: %s",pModName);
#endif
			}

			pModName = strtok(NULL,",");
		}
	}

	return nNumMods;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::BuildGearList()
//
//	PURPOSE:	Builds an array of ints from the string associated with
//					the given string;
//
//	RETURNS:	Number of gear types added to list
//
// ----------------------------------------------------------------------- //

int MISSION::BuildGearList(CString str, int* pArray, int nArrayLen)
{
	if (!pArray) return 0;

	int  nNumGear = 0;
	char buf[512] = "";

	if (!str.IsEmpty())
	{
		strncpy(buf, (char*)(LPCSTR)str, 512);
		char *pGearName = strtok(buf,",");
		while (pGearName && nNumGear < nArrayLen)
		{
			GEAR *pGear = g_pWeaponMgr->GetGear(pGearName);
			if (pGear)
			{
				pArray[nNumGear] = pGear->nId;
				nNumGear++;
			}
			else
			{

#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in MISSIONS.TXT: Unknown gear item: %s",pGearName);
#else
                g_pLTServer->CPrint("Error in MISSIONS.TXT: Unknown gear item: %s",pGearName);
#endif
			}

			pGearName = strtok(NULL,",");
		}
	}

	return nNumGear;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::GetRankBonus()
//
//	PURPOSE:	Calculate the various bonusses granted by the mission
//				for a given score (0.0 to 1.0).
//
// ----------------------------------------------------------------------- //
void MISSION::GetRankBonus(LTFLOAT fScorePercent,RANKBONUS *pBonus)
{
	_ASSERT(pBonus);
	pBonus->Reset();

	int nRank = g_pMissionMgr->GetMissionRating()->GetRank(fScorePercent);

	for (int i =0; i <= nRank; i++)
	{
		pBonus->nReputationPoints	+= aRankBonus[i].nReputationPoints;
		pBonus->nHealthPoints		+= aRankBonus[i].nHealthPoints;
		pBonus->nArmorPoints		+= aRankBonus[i].nArmorPoints;
		pBonus->nAmmoPoints			+= aRankBonus[i].nAmmoPoints;
		pBonus->nPerturbPoints		+= aRankBonus[i].nPerturbPoints;
		pBonus->nDamagePoints		+= aRankBonus[i].nDamagePoints;
		pBonus->nStealthPoints		+= aRankBonus[i].nStealthPoints;

	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONRATING::MISSIONRATING
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

MISSIONRATING::MISSIONRATING()
{
	nSuccessPts		= 0;
	nIntelPts		= 0;
	nReputationInc	= 0;
	nReputationMax	= 0;


	fHealthInc	= 0.0f;
	fArmorInc	= 0.0f;
	fAmmoInc	= 0.0f;
	fPerturbInc	= 0.0f;
	fDamageInc	= 0.0f;
	fStealthInc	= 0.0f;
	fPerturbMin	= 0.0f;
	fStealthMin	= 0.0f;
	fHealthMax	= 0.0f;
	fArmorMax	= 0.0f;
	fAmmoMax	= 0.0f;
	fDamageMax	= 0.0f;

	nNumRanks	= 0;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONRATING::Init
//
//	PURPOSE:	Build the missionrating struct
//
// ----------------------------------------------------------------------- //

LTBOOL MISSIONRATING::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

    nSuccessPts     = (uint8) buteMgr.GetInt(aTagName, MMGR_MISSIONRATING_SUCCESSPTS);
    nIntelPts       = (uint8) buteMgr.GetInt(aTagName, MMGR_MISSIONRATING_INTELPTS);
    nReputationInc  = (uint8) buteMgr.GetInt(aTagName, MMGR_MISSIONRATING_REPINC);
    nReputationMax  = (uint8) buteMgr.GetInt(aTagName, MMGR_MISSIONRATING_REPMAX);

    fHealthInc      = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_HEALTHINC);
    fArmorInc       = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_ARMORINC);
    fAmmoInc        = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_AMMOINC);
    fPerturbInc     = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_PERTURBINC);
    fDamageInc      = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_DAMAGEINC);
    fStealthInc     = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_STEALTHINC);

    fPerturbMin     = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_PERTURBMIN);
    fStealthMin     = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_STEALTHMIN);

    fHealthMax      = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_HEALTHMAX);
    fArmorMax       = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_ARMORMAX);
    fAmmoMax        = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_AMMOMAX);
    fDamageMax      = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_MISSIONRATING_DAMAGEMAX);

	nNumRanks = 0;
	sprintf(s_aTagName, "%s%d", MMGR_RANK_TAG, nNumRanks);

	while (buteMgr.Exist(aTagName) && nNumRanks < MMGR_MAX_MISSION_LEVELS)
	{
        aRanks[nNumRanks].fPercent = (LTFLOAT) buteMgr.GetDouble(s_aTagName, MMGR_RANK_PERCENT);
		aRanks[nNumRanks].nNameId =  buteMgr.GetInt(s_aTagName, MMGR_RANK_ID);

		nNumRanks++;
		sprintf(s_aTagName, "%s%d", MMGR_RANK_TAG, nNumRanks);
	}


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONRATING::GetRank
//
//	PURPOSE:	Get the rank associated with the given score
//
// ----------------------------------------------------------------------- //
int MISSIONRATING::GetRank(LTFLOAT fScorePercent)
{
	fScorePercent =LTCLAMP(fScorePercent,0.0f,1.0f);
	int nRank = 0;
	while (nRank < (nNumRanks-1) && (fScorePercent > aRanks[nRank].fPercent))
		nRank++;

	return nRank;
}

RANKBONUS::RANKBONUS()
{
	Reset();
};

void RANKBONUS::Reset()
{
	nReputationPoints	= 0;
	nHealthPoints		= 0;
	nArmorPoints		= 0;
	nAmmoPoints			= 0;
	nPerturbPoints		= 0;
	nDamagePoints		= 0;
	nStealthPoints		= 0;
};


RANKDATA::RANKDATA()
{
	nNameId		= 0;
	fPercent	= 0.0f;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONAWARDS::MISSIONAWARDS
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

MISSIONAWARDS::MISSIONAWARDS()
{
	fAccuracyPct = 0.0f;
	fMarksmanPct = 0.0f;

	nNumHighAmmoAwards = 0;
	nNumLowAmmoAwards = 0;
	nNumMarksmanAwards = 0;
	nNumAccuracyAwards = 0;
	nNumNonInjuryAwards = 0;
	nNumNotShotAwards = 0;
	nNumStealthAwards = 0;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONAWARDS::BuildAwardsList()
//
//	PURPOSE:	Builds an array of ints from the string associated with
//					the given string;
//
//	RETURNS:	Number of Awards types added to list
//
// ----------------------------------------------------------------------- //

int MISSIONAWARDS::BuildAwardsList(CString str, int* pArray, int nArrayLen)
{
	if (!pArray) return 0;

	int  nNumAwards = 0;
	char buf[512] = "";

	if (!str.IsEmpty())
	{
		strncpy(buf, (char*)(LPCSTR)str, 512);
		char *pAward = strtok(buf,",");
		while (pAward && nNumAwards < nArrayLen)
		{
			pArray[nNumAwards] = atoi(pAward);
			pAward = strtok(NULL,",");
			nNumAwards++;
		}
	}

	return nNumAwards;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONAWARDS::Init
//
//	PURPOSE:	Build the MISSIONAWARDS struct
//
// ----------------------------------------------------------------------- //

LTBOOL MISSIONAWARDS::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

    fAccuracyPct    = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_AWARDS_ACC_PCT);
    fMarksmanPct    = (LTFLOAT) buteMgr.GetDouble(aTagName, MMGR_AWARDS_MARK_PCT);

	CString str = buteMgr.GetString(aTagName, MMGR_AWARDS_HIGHAMMO);
	nNumHighAmmoAwards = BuildAwardsList(str, aHighAmmoAwards, ARRAY_LEN(aHighAmmoAwards));

	str = buteMgr.GetString(aTagName, MMGR_AWARDS_LOWAMMO);
	nNumLowAmmoAwards = BuildAwardsList(str, aLowAmmoAwards, ARRAY_LEN(aLowAmmoAwards));

	str = buteMgr.GetString(aTagName, MMGR_AWARDS_MARKSMAN);
	nNumMarksmanAwards = BuildAwardsList(str, aMarksmanAwards, ARRAY_LEN(aMarksmanAwards));

	str = buteMgr.GetString(aTagName, MMGR_AWARDS_ACCURACY);
	nNumAccuracyAwards = BuildAwardsList(str, aAccuracyAwards, ARRAY_LEN(aAccuracyAwards));

	str = buteMgr.GetString(aTagName, MMGR_AWARDS_NONINJURY);
	nNumNonInjuryAwards = BuildAwardsList(str, aNonInjuryAwards, ARRAY_LEN(aNonInjuryAwards));

	str = buteMgr.GetString(aTagName, MMGR_AWARDS_NOTSHOT);
	nNumNotShotAwards = BuildAwardsList(str, aNotShotAwards, ARRAY_LEN(aNotShotAwards));

	str = buteMgr.GetString(aTagName, MMGR_AWARDS_STEALTH);
	nNumStealthAwards = BuildAwardsList(str, aStealthAwards, ARRAY_LEN(aStealthAwards));

    return LTTRUE;
}