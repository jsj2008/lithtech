// ----------------------------------------------------------------------- //
//
// MODULE  : SearchItemMgr.cpp
//
// PURPOSE : Attribute file manager for key item info
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SearchItemMgr.h"
#include "WeaponMgr.h"


#define ITEM_TAG				"SearchItem"
#define ITEM_NAME				"Name"
#define ITEM_TEXT_ID			"TextId"
#define ITEM_ICON				"Icon"

#define SET_TAG					"SearchSet"
#define SET_NAME				"Name"
#define SET_ITEMS				"Items"
#define SET_ITEM_PERCENT		"ItemPercent"
#define SET_AMMO				"Ammo"
#define SET_AMMO_PERCENT		"AmmoPercent"
#define SET_WEAPON				"Weapon"
#define SET_WEAPON_PERCENT		"WeaponPercent"
#define SET_GEAR				"Gear"
#define SET_GEAR_PERCENT		"GearPercent"

CSearchItemMgr* g_pSearchItemMgr = LTNULL;

#ifndef _CLIENTBUILD
#ifndef __PSX2
CSearchItemMgr CSearchItemMgrPlugin::sm_SearchItemMgr;
#endif
#endif

static char s_aTagName[30];
static char s_aAttName[30];

SEARCH_ITEM::SEARCH_ITEM()
{
	nId = SI_INVALID_ID;
	szName[0] = 0;
	nTextId = 0;
	szIcon[0] = 0;

}

SEARCH_SET::SEARCH_SET()
{
	nId = SI_INVALID_ID;
	szName[0] = 0;
	nItems = 0;

	nItemPercent	= 100;
	nAmmoPercent	= 0;
	nWeaponPercent	= 0;
	nGearPercent	= 0;

	nAmmos = 0;
	nWeapons = 0;
	nGears = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SEARCH_SET::GetRandomSearchObjectType()
//
//	PURPOSE:	Based on our percentages, return a random search object type
//
// ----------------------------------------------------------------------- //

SEARCH_SET::SearchObjectType SEARCH_SET::GetRandomSearchObjectType(float fJunkModifier)
{
	if (fJunkModifier < 0.0f || fJunkModifier > 1.0f)
	{
		ASSERT(!"SEARCH_SET::GetRandomSearchObjectType(): bad skill modifier");
		fJunkModifier = 1.0f;
	}

	//reduce the chance of random junk
	uint8 nJunkChance = (uint8)( (float)nItemPercent * fJunkModifier );

	//figure out a new total
	uint8 nTotal = 100 - (nItemPercent - nJunkChance);

	//roll the die...
	uint8 nRandVal = GetRandom(1, nTotal);

	SearchObjectType eType = eUnknownObjectType;
	if (nRandVal <= nJunkChance)
	{
		eType = eItemObjectType;
	}
	else if (nRandVal <= nJunkChance + nAmmoPercent)
	{
		eType = eAmmoObjectType;
	}
	else if (nRandVal <= nJunkChance + nAmmoPercent + nWeaponPercent)
	{
		eType = eWeaponObjectType;
	}
	else
	{
		eType = eGearObjectType;
	}

	return eType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SEARCH_SET::GetRandomSearchObjectInfo()
//
//	PURPOSE:	Get random search object info
//
// ----------------------------------------------------------------------- //

bool SEARCH_SET::GetRandomSearchObjectInfo(SearchObjectResult & soResult, float fJunkModifier)
{
	soResult.eType	= GetRandomSearchObjectType(fJunkModifier);
	soResult.nId	= SI_INVALID_ID;

	switch (soResult.eType)
	{
		case eItemObjectType :
		{
			if (nItems > 0)
			{
				soResult.nId = anItems[GetRandom(0, nItems-1)];
			}
		}
		break;

		case eAmmoObjectType :
		{
			if (nAmmos > 0)
			{
				uint8 nIndex	 = GetRandom(0, nAmmos-1);
				soResult.nId	 = anAmmos[nIndex];
				soResult.nAmount = anAmmoAmounts[nIndex];
			}
		}
		break;

		case eWeaponObjectType :
		{
			if (nWeapons > 0)
			{
				soResult.nId = anWeapons[GetRandom(0, nWeapons-1)];
			}
		}
		break;

		case eGearObjectType :
		{
			if (nGears > 0)
			{
				soResult.nId = anGears[GetRandom(0, nGears-1)];
			}
		}
		break;

		case eUnknownObjectType :
		default :
			return false;
		break;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSearchItemMgr::CSearchItemMgr()
{
}

CSearchItemMgr::~CSearchItemMgr()
{
	Term();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchItemMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSearchItemMgr::Init(const char* szAttributeFile)
{
    if (g_pSearchItemMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

	// Set up global pointer...
	g_pSearchItemMgr = this;

	uint8 nNumItems = 0;

	sprintf(s_aTagName, "%s0", ITEM_TAG);
	while (m_buteMgr.Exist(s_aTagName))
	{
		SEARCH_ITEM* pNew = debug_new(SEARCH_ITEM);
		pNew->nId = nNumItems;
		m_buteMgr.GetString(s_aTagName, ITEM_NAME, "", pNew->szName, sizeof(pNew->szName));
		pNew->nTextId = (uint16)m_buteMgr.GetInt(s_aTagName, ITEM_TEXT_ID, 0);
		m_buteMgr.GetString(s_aTagName, ITEM_ICON, "", pNew->szIcon, sizeof(pNew->szIcon));

		m_ItemArray.push_back(pNew);

		nNumItems++;
		sprintf(s_aTagName, "%s%d", ITEM_TAG, nNumItems);
	}

	uint8 nNumSets = 0;
	char szBuffer[512] = "";
	sprintf(s_aTagName, "%s0", SET_TAG);
	while (m_buteMgr.Exist(s_aTagName))
	{
		SEARCH_SET* pNew = debug_new(SEARCH_SET);
		pNew->nId = nNumSets;
		m_buteMgr.GetString(s_aTagName, SET_NAME, "", pNew->szName, sizeof(pNew->szName));

		pNew->nItemPercent		= m_buteMgr.GetInt(s_aTagName, SET_ITEM_PERCENT, 100);
		pNew->nAmmoPercent		= m_buteMgr.GetInt(s_aTagName, SET_AMMO_PERCENT, 0);
		pNew->nWeaponPercent	= m_buteMgr.GetInt(s_aTagName, SET_WEAPON_PERCENT, 0);
		pNew->nGearPercent		= m_buteMgr.GetInt(s_aTagName, SET_GEAR_PERCENT, 0);

		int nTotalPercent = pNew->nItemPercent + pNew->nAmmoPercent + pNew->nWeaponPercent +
			pNew->nGearPercent;

		if (nTotalPercent != 100)
		{
			ASSERT(0);
			return LTFALSE;
		}

		int nIndex = 0;
		pNew->nAmmos = 0;
		sprintf(s_aAttName, "%s%d", SET_AMMO, 0);

		while (m_buteMgr.Exist(s_aTagName, s_aAttName) && nIndex < SI_MAX_OBJECTS)
		{
			szBuffer[0] = '\0';
			m_buteMgr.GetString(s_aTagName, s_aAttName, szBuffer, sizeof(szBuffer));
			
			// Pull out the ammo name...
			char* pszTok = strtok(szBuffer,",");

			if (szBuffer[0])
			{
				AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(pszTok);
				ASSERT(pAmmo);
				if (pAmmo)
				{
					pNew->anAmmos[pNew->nAmmos] = pAmmo->nId;

					// Pull out the ammo amount...
					pszTok = strtok(NULL,",");

					pNew->anAmmoAmounts[pNew->nAmmos] = ( pszTok ) ? (uint32) atol(pszTok) : 0;
				}
				else
				{
					return LTFALSE;
				}

				pNew->nAmmos++;
			}

			nIndex++;
			sprintf(s_aAttName, "%s%d", SET_AMMO, nIndex);
		}

		nIndex = 0;
		pNew->nWeapons = 0;
		sprintf(s_aAttName, "%s%d", SET_WEAPON, 0);

		while (m_buteMgr.Exist(s_aTagName, s_aAttName) && nIndex < SI_MAX_OBJECTS)
		{
			szBuffer[0] = '\0';
			m_buteMgr.GetString(s_aTagName, s_aAttName, szBuffer, sizeof(szBuffer));

			if (szBuffer[0])
			{
				WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(szBuffer);
				ASSERT(pWeapon);
				if (pWeapon)
				{
					pNew->anWeapons[pNew->nWeapons] = pWeapon->nId;
				}
				else
				{
					return LTFALSE;
				}

				pNew->nWeapons++;
			}

			nIndex++;
			sprintf(s_aAttName, "%s%d", SET_WEAPON, nIndex);
		}

		nIndex = 0;
		pNew->nGears = 0;
		sprintf(s_aAttName, "%s%d", SET_GEAR, 0);

		while (m_buteMgr.Exist(s_aTagName, s_aAttName) && nIndex < SI_MAX_OBJECTS)
		{
			szBuffer[0] = '\0';
			m_buteMgr.GetString(s_aTagName, s_aAttName, szBuffer, sizeof(szBuffer));

			if (szBuffer[0])
			{
				GEAR const *pGear = g_pWeaponMgr->GetGear(szBuffer);
				ASSERT(pGear);
				if (pGear)
				{
					pNew->anGears[pNew->nGears] = pGear->nId;
				}
				else
				{
					return LTFALSE;
				}

				pNew->nGears++;
			}

			nIndex++;
			sprintf(s_aAttName, "%s%d", SET_GEAR, nIndex);
		}

		
		m_buteMgr.GetString(s_aTagName, SET_ITEMS, "", szBuffer, sizeof(szBuffer));

		char* pszTok = strtok(szBuffer,",");
		pNew->nItems = 0;
		while (pszTok && pNew->nItems < SI_MAX_ITEMS)
		{
			SEARCH_ITEM *pItem = GetItem(pszTok);
			if (pItem)
			{
				pNew->anItems[pNew->nItems] = pItem->nId;
				pNew->nItems++;
			}
			else
			{
#ifdef _CLIENTBUILD
				g_pLTClient->CPrint("error in %s: can't find search item \"%s\"",szAttributeFile,pszTok);
#else
				g_pLTServer->CPrint("error in %s: can't find search item \"%s\"",szAttributeFile,pszTok);
#endif
				return LTFALSE;
			}

			pszTok = strtok(NULL,",");

		}

		m_SetArray.push_back(pNew);

		nNumSets++;
		sprintf(s_aTagName, "%s%d", SET_TAG, nNumSets);
	}



	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchItemMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CSearchItemMgr::Term()
{
    g_pSearchItemMgr = LTNULL;

	ItemArray::iterator iter = m_ItemArray.begin();
	while (iter != m_ItemArray.end())
	{
		debug_delete(*iter);
		iter++;
	}
	m_ItemArray.clear();

	SetArray::iterator sIter = m_SetArray.begin();
	while (sIter != m_SetArray.end())
	{
		debug_delete(*sIter);
		sIter++;
	}
	m_SetArray.clear();
}


SEARCH_ITEM* CSearchItemMgr::GetItem(uint8 nID)
{
	if (nID >= m_ItemArray.size()) return LTNULL;
	
	return m_ItemArray[nID];
}

SEARCH_ITEM* CSearchItemMgr::GetItem(const char *pszName)
{
	if (!pszName || !pszName[0]) return LTNULL;

	//skip leading whitespace
	while (*pszName == ' ' || *pszName == '\t')
			pszName++;

	//ignore trailing whitespace
	int nLen = strlen(pszName);
	const char* pszTmp = strpbrk(pszName," \t");
	if (pszTmp)
		nLen = (pszTmp-pszName);
	

	ItemArray::iterator iter = m_ItemArray.begin();
	while (iter != m_ItemArray.end() && strnicmp(pszName,(*iter)->szName,nLen) != 0)
	{
		iter++;
	}

	if (iter != m_ItemArray.end())
		return (*iter);
	return LTNULL;
}


SEARCH_SET* CSearchItemMgr::GetSet(uint8 nID)
{
	if (nID >= m_SetArray.size()) return LTNULL;
	
	return m_SetArray[nID];
}

SEARCH_SET* CSearchItemMgr::GetSet(const char *pszName)
{
	if (!pszName || !pszName[0]) return LTNULL;
	SetArray::iterator iter = m_SetArray.begin();
	while (iter != m_SetArray.end() && stricmp(pszName,(*iter)->szName) != 0)
	{
		iter++;
	}

	if (iter != m_SetArray.end())
		return (*iter);
	return LTNULL;
}




#ifndef _CLIENTBUILD
#ifndef __PSX2
////////////////////////////////////////////////////////////////////////////
//
// CSearchItemMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CSearchItemMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchItemMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CSearchItemMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pSearchItemMgr)
	{
		// This will set the g_pSearchItemMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, SI_DEFAULT_FILE);
        sm_SearchItemMgr.SetInRezFile(LTFALSE);
        sm_SearchItemMgr.Init(szFile);
	}

	if (!PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength))
	{
		return LT_UNSUPPORTED;
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchItemMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CSearchItemMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each Key type

	int nNumSets = g_pSearchItemMgr->GetNumSets();
	_ASSERT(nNumSets > 0);

    SEARCH_SET* pSet = LTNULL;

	for (int i=0; i < nNumSets; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pSet = g_pSearchItemMgr->GetSet(i);
		if (pSet && pSet->szName[0])
		{
            uint32 dwNameLen = strlen(pSet->szName);

			if (dwNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
			{
				strcpy(aszStrings[(*pcStrings)++], pSet->szName);
			}
		}
	}

    return LTTRUE;
}

#endif
#endif
