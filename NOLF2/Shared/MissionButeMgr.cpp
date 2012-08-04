// ----------------------------------------------------------------------- //
//
// MODULE  : MissionButeMgr.cpp
//
// PURPOSE : MissionButeMgr implementation - Controls attributes of all
//			 missions
//
// CREATED : 07/26/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MissionButeMgr.h"
#include "CommonUtilities.h"
#include "WeaponMgr.h"


#define MMGR_MISSION_NAMEID				"NameId"
#define MMGR_MISSION_DESCID				"DescId"
#define MMGR_MISSION_BRIEFINGID			"BriefingId"
#define MMGR_MISSION_HELPID				"HelpId"
#define MMGR_MISSION_RESETPLAYER		"ResetPlayer"
#define MMGR_MISSION_OBJECTIVEIDS		"ObjectiveIds"
#define MMGR_MISSION_LAYOUT				"LayoutInfo"
#define MMGR_MISSION_BRIEFING_LAYOUT	"BriefingLayout"
#define MMGR_MISSION_WEAPONS_DEFAULT	"DefaultWeapons"
#define MMGR_MISSION_AMMO_DEFAULT		"DefaultAmmo"
#define MMGR_MISSION_MODS_DEFAULT		"DefaultMods"
#define MMGR_MISSION_SELECTED_WEAPON	"SelectedWeapon"
#define MMGR_MISSION_NAME_STR			"NameStr"
#define MMGR_MISSION_PHOTO				"Photo"

#define MMGR_MISSION_LEVEL				"Level"
#define MMGR_MISSION_REWARD				"Reward"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::CMissionButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMissionButeMgr::CMissionButeMgr()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::~CMissionButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CMissionButeMgr::~CMissionButeMgr()
{
	g_pMissionButeMgr = NULL;

	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CMissionButeMgr::Term()
{
	MissionList::iterator it = m_MissionList.begin( );
	while( it != m_MissionList.end( ))
	{
		DestroyMission( *it );
		it++;
	}
	m_MissionList.clear();

	CGameButeMgr::Term( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::GetMission
//
//	PURPOSE:	Get the specified mission
//
// ----------------------------------------------------------------------- //

MISSION* CMissionButeMgr::GetMission(int nMissionId)
{
    if (nMissionId < 0 || nMissionId > ( int )m_MissionList.size( )) return LTNULL;

	MissionList::iterator it = m_MissionList.begin( );
	for( ; it != m_MissionList.end( ); it++ )
	{
		MISSION* pMission = *it;
		if( !pMission )
			continue;

		if( pMission->nId == nMissionId )
			return pMission;
	}

    return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::GetLevel
//
//	PURPOSE:	Get the specified level
//
// ----------------------------------------------------------------------- //

LEVEL* CMissionButeMgr::GetLevel( int nMissionId, int nLevelId )
{
	MISSION *pMission = GetMission( nMissionId );
	if( !pMission )
		return LTNULL;

	if( nLevelId < 0 || nLevelId > pMission->nNumLevels )
		return LTNULL;

	return &pMission->aLevels[nLevelId];
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgr::IsMissionLevel()
//
//	PURPOSE:	Determine if the passed in string is a level of one of the
//				missions.
//
//	RETURNS:	True if a mission level, else false.  Also nMissionId and
//				nLevel are filled in with the correct values.
//
// ----------------------------------------------------------------------- //

bool CMissionButeMgr::IsMissionLevel(char const* pWorldFile, int & nMissionId, int & nLevel)
{
	if (!pWorldFile) 
		return false;


	//strip path (if there is one) from world name
	char const* pTmp = strrchr(pWorldFile,'\\');
	if (pTmp)
	{
		pTmp++;
		pWorldFile = pTmp;
	}

	MissionList::iterator it = m_MissionList.begin( );
	for( ; it != m_MissionList.end( ); it++)
	{
		MISSION* pMission = *it;
		if( !pMission )
			continue;

		for (int i=0; i < pMission->nNumLevels; i++)
		{

			//strip path (if there is one) from each levels name
			char const* pTmp = strrchr(pMission->aLevels[i].szLevel,'\\');
			if (pTmp)
			{
				pTmp++;
			}
			else
			{
				pTmp = pMission->aLevels[i].szLevel;
			}

			if (stricmp( pTmp, pWorldFile ) == 0)
			{
				nMissionId = pMission->nId;
				nLevel = i;
                return true;
			}
		}
	}

    return false;
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
	nDescId				= 0;
	bResetPlayer		= true;

	szLayout[0]			= '\0';
	szBriefLayout[0]			= '\0';

	nNumDefaultWeapons	= 0;
	nNumDefaultAmmo		= 0;
	nNumDefaultMods		= 0;

	nSelectedWeapon		= -1;

	aDefaultWeapons[0]	= WMGR_INVALID_ID;
	aDefaultAmmo[0]		= WMGR_INVALID_ID;
	aDefaultMods[0]		= WMGR_INVALID_ID;

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

	char aAttName[100];

	nNameId			= buteMgr.GetInt(aTagName, MMGR_MISSION_NAMEID);
	nDescId			= buteMgr.GetInt(aTagName, MMGR_MISSION_DESCID);
	bResetPlayer	= !!buteMgr.GetInt(aTagName, MMGR_MISSION_RESETPLAYER, 1 );

	char szStr[512] ="";
	buteMgr.GetString(aTagName, MMGR_MISSION_OBJECTIVEIDS,szStr,sizeof(szStr));

	buteMgr.GetString(aTagName, MMGR_MISSION_LAYOUT, szLayout, ARRAY_LEN(szLayout));
	buteMgr.GetString(aTagName, MMGR_MISSION_BRIEFING_LAYOUT, szBriefLayout, ARRAY_LEN(szBriefLayout));

	buteMgr.GetString(aTagName, MMGR_MISSION_WEAPONS_DEFAULT,szStr,sizeof(szStr));
	nNumDefaultWeapons = BuildWeaponsList(szStr, aDefaultWeapons, ARRAY_LEN(aDefaultWeapons));

	buteMgr.GetString(aTagName, MMGR_MISSION_AMMO_DEFAULT,szStr,sizeof(szStr));
	nNumDefaultAmmo = BuildAmmoList(szStr, aDefaultAmmo, ARRAY_LEN(aDefaultAmmo));

	buteMgr.GetString(aTagName, MMGR_MISSION_MODS_DEFAULT,szStr,sizeof(szStr));
	nNumDefaultMods = BuildModsList(szStr, aDefaultMods, ARRAY_LEN(aDefaultMods));

	szStr[0] = '\0';
	buteMgr.GetString(aTagName, MMGR_MISSION_SELECTED_WEAPON, szStr, sizeof(szStr));
	if (szStr[0])
	{
		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(szStr);
		if (pWeapon)
		{
			nSelectedWeapon = pWeapon->nId;
		}
	}

	//these values are only used for multiplayer games:
	buteMgr.GetString(aTagName, MMGR_MISSION_NAME_STR,szStr,sizeof(szStr));
	sName = szStr;

	buteMgr.GetString(aTagName, MMGR_MISSION_PHOTO,szStr,sizeof(szStr));
	sPhoto = szStr;


	nNumLevels = 0;

	while( nNumLevels < MMGR_MAX_MISSION_LEVELS )
	{

		sprintf(aAttName, "%s%d", MMGR_MISSION_LEVEL, nNumLevels);
		buteMgr.GetString(aTagName, aAttName, "", aLevels[nNumLevels].szLevel, ARRAY_LEN(aLevels[nNumLevels].szLevel));
		if( !buteMgr.Success( ))
			break;

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_NAMEID );
		aLevels[nNumLevels].nNameId	= buteMgr.GetInt( aTagName, aAttName, -1 );

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_BRIEFINGID );
		aLevels[nNumLevels].nBriefingId	= buteMgr.GetInt( aTagName, aAttName, -1 );

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_HELPID );
		aLevels[nNumLevels].nHelpId	= buteMgr.GetInt( aTagName, aAttName, -1 );

		// Level default weapons...

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_WEAPONS_DEFAULT );
		buteMgr.GetString( aTagName, aAttName, " ", szStr, sizeof(szStr) );
		aLevels[nNumLevels].nNumDefaultWeapons = BuildWeaponsList( szStr, aLevels[nNumLevels].aDefaultWeapons,
																   ARRAY_LEN(aLevels[nNumLevels].aDefaultWeapons) );

		// Level default ammo...

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_AMMO_DEFAULT );
		buteMgr.GetString( aTagName, aAttName, " ", szStr, sizeof(szStr) );
		aLevels[nNumLevels].nNumDefaultAmmo = BuildAmmoList( szStr, aLevels[nNumLevels].aDefaultAmmo, 
															 ARRAY_LEN(aLevels[nNumLevels].aDefaultAmmo) );

		// Level default mods...

		sprintf( aAttName, "%s%d%s", MMGR_MISSION_LEVEL, nNumLevels, MMGR_MISSION_MODS_DEFAULT );
		buteMgr.GetString( aTagName, aAttName, " ", szStr, sizeof(szStr) );
		aLevels[nNumLevels].nNumDefaultMods = BuildModsList( szStr, aLevels[nNumLevels].aDefaultMods,
															 ARRAY_LEN(aLevels[nNumLevels].aDefaultMods) );

		nNumLevels++;
	}

	nNumRewards = 0;
	while( nNumRewards < MMGR_MAX_MISSION_REWARDS)
	{
		sprintf(aAttName, "%s%d", MMGR_MISSION_REWARD, nNumRewards);
		buteMgr.GetString(aTagName, aAttName, "", aRewards[nNumRewards].szName, ARRAY_LEN(aRewards[nNumRewards].szName));
		if( !buteMgr.Success( ))
			break;

		char *pTok = strtok(aRewards[nNumRewards].szName,",");
		pTok = strtok(LTNULL,",");
		if (pTok)
			aRewards[nNumRewards].nVal = (uint32)atol(pTok);

		pTok = strtok(LTNULL,"");
		if (pTok)
			aRewards[nNumRewards].nDescriptionId = (uint32)atol(pTok);

		nNumRewards++;
	}




    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSION::Save
//
//	PURPOSE:	Save the mission struct
//
// ----------------------------------------------------------------------- //

LTBOOL MISSION::Save(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	char aAttName[100];


//***********************************************
//*jrg  NOTE:Only saving level info at this point
//***********************************************

	for (int n = 0; n < nNumLevels; n++)
	{
		sprintf(aAttName, "%s%d", MMGR_MISSION_LEVEL, n);
		buteMgr.SetString(aTagName, aAttName,aLevels[n].szLevel);
	}
    return LTTRUE;
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

int MISSION::BuildWeaponsList(char* pszStr, int* pArray, int nArrayLen)
{
	if (!pArray || !pszStr) return 0;

	int  nNumWeapons = 0;
	char buf[512] = "";

	while (*pszStr == ' ' || *pszStr == '\t')
		pszStr++;

	if (strlen(pszStr))
	{
		SAFE_STRCPY(buf, pszStr);
		char *pWpnName = strtok(buf,",");
		while (pWpnName && nNumWeapons < nArrayLen)
		{
			WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(pWpnName);
			if (pWeapon)
			{
				pArray[nNumWeapons] = pWeapon->nId;
				nNumWeapons++;
			}
			else
			{
#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in %s: Unknown weapon: %s", g_pMissionButeMgr->GetAttributeFile( ),pWpnName);
#else
                g_pLTServer->CPrint("Error in %s: Unknown weapon: %s", g_pMissionButeMgr->GetAttributeFile( ),pWpnName);
#endif
            }
			pWpnName = strtok(NULL,",");
		}
	}

	return nNumWeapons;
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

int MISSION::BuildAmmoList(char *pszStr, int* pArray, int nArrayLen)
{
	if (!pArray || !pszStr) return 0;

	int  nNumAmmo = 0;
	char buf[512] = "";

	while (*pszStr == ' ' || *pszStr == '\t')
		pszStr++;


	if (strlen(pszStr))
	{
		SAFE_STRCPY(buf, pszStr);
		char *pAmmoName = strtok(buf,",");
		while (pAmmoName && nNumAmmo < nArrayLen)
		{
			AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(pAmmoName);
			if (pAmmo)
			{
				pArray[nNumAmmo] = pAmmo->nId;
				nNumAmmo++;
			}
			else
			{
#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in %s: Unknown ammo: %s", g_pMissionButeMgr->GetAttributeFile( ),pAmmoName);
#else
                g_pLTServer->CPrint("Error in %s: Unknown ammo: %s", g_pMissionButeMgr->GetAttributeFile( ),pAmmoName);
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

int MISSION::BuildModsList(char *pszStr, int* pArray, int nArrayLen)
{
	if (!pArray || !pszStr) return 0;

	int  nNumMods = 0;
	char buf[512] = "";

	while (*pszStr == ' ' || *pszStr == '\t')
		pszStr++;

	if (strlen(pszStr))
	{
		SAFE_STRCPY(buf, pszStr);
		char *pModName = strtok(buf,",");
		while (pModName && nNumMods < nArrayLen)
		{
			MOD const *pMod = g_pWeaponMgr->GetMod(pModName);
			if (pMod)
			{
				pArray[nNumMods] = pMod->nId;
				nNumMods++;
			}
			else
			{
#ifdef _CLIENTBUILD
                g_pLTClient->CPrint("Error in %s: Unknown weapon mod: %s", g_pMissionButeMgr->GetAttributeFile( ),pModName);
#else
                g_pLTServer->CPrint("Error in %s: Unknown weapon mod: %s", g_pMissionButeMgr->GetAttributeFile( ),pModName);
#endif
			}

			pModName = strtok(NULL,",");
		}
	}

	return nNumMods;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LEVEL::LEVEL
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

LEVEL::LEVEL()
{ 
	nNameId			= -1;
	nBriefingId		= -1;
	nHelpId			= -1;

	szLevel[0] = '\0';

	nNumDefaultWeapons	= 0;
	nNumDefaultAmmo		= 0;
	nNumDefaultMods		= 0;

	aDefaultWeapons[0]	= WMGR_INVALID_ID;
	aDefaultAmmo[0]		= WMGR_INVALID_ID;
	aDefaultMods[0]		= WMGR_INVALID_ID;
}
