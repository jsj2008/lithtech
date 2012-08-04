//
// RatingMgr.cpp
//
// Written 11/10/01 by A. Megowan
//
// Class to encapsulate management of the Performance Ratings
#include "stdafx.h"
#include "RatingMgr.h"

// TODO
// differentiate between m_AddArray and m_PlayerAddArray

CRatingMgr* g_pRatingMgr = LTNULL;

#define ADDITIVE_TAG			"Additive"

#define ADDITIVE_NAME			"Name"
#define ADDITIVE_NAME_ID		"NameId"
#define ADDITIVE_DESC_ID		"DescriptionId"
#define ADDITIVE_SPRITE			"Sprite"


static char s_aTagName[30];
static char s_aAttName[30];

const char * szRatings[] = {
	"Health",
	"Energy",
	"Accuracy",
	"Stealth",
	"Efficiency",
};


ADDITIVE::ADDITIVE()
{
	szName[0]		= 0;
	nNameId			= 0;
	nDescriptionId	= 0;

	szSprite[0]		= 0;
	pSprite			= LTNULL;

	for (int i = 0; i < NUM_RATINGS; i++)
		Adjust[i] = 0;

	nSlot			= -1;
	nTempSlot		= -1;
}

CRatingMgr::CRatingMgr()
{
}

CRatingMgr::~CRatingMgr()
{
	Term();
}

LTBOOL CRatingMgr::Init(const char* szAttributeFile)
{
	if (g_pRatingMgr) return LTFALSE;

	// Set up global pointer...
	g_pRatingMgr = this;

	m_AddArray.clear();
	m_AddArray.reserve(5);

	m_PlayerAddArray.clear();

	// Check for legitimate additive file
    if (!szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile))
	{
		// If you broke on this assert,
		// you need to grab "attributes/additives.txt" out of SourceSafe
		ASSERT(LTFALSE);
		return LTFALSE;
	}
	
	// TODO for now, setting all ratings to "50" eventually set these elsewhere
	for (int i = 0; i < NUM_RATINGS; i++)
	{
		m_iBaseRating[i] = 50;
	}

	m_iRatingGoo = 0;

	// Read in all of the additives
	uint16 nNumAdditives = 0;

	sprintf(s_aTagName, "%s0", ADDITIVE_TAG);
	while (m_buteMgr.Exist(s_aTagName))
	{
		ADDITIVE* pNew = debug_new(ADDITIVE);
		m_buteMgr.GetString(s_aTagName, ADDITIVE_NAME, "", pNew->szName, sizeof(pNew->szName));
		pNew->nNameId = (uint16)m_buteMgr.GetInt(s_aTagName, ADDITIVE_NAME_ID, 0);
		pNew->nDescriptionId = (uint16)m_buteMgr.GetInt(s_aTagName, ADDITIVE_DESC_ID, 0);

		m_buteMgr.GetString(s_aTagName, ADDITIVE_SPRITE, "", pNew->szSprite, sizeof(pNew->szSprite));
		pNew->pSprite = LTNULL;

		for (int i = 0; i < NUM_RATINGS; i++)
		{
			pNew->Adjust[i] = (int8)m_buteMgr.GetInt(s_aTagName, szRatings[i], 0);
		}
		m_AddArray.push_back(pNew);

		nNumAdditives++;
		sprintf(s_aTagName, "%s%d", ADDITIVE_TAG, nNumAdditives);
	}
	m_buteMgr.Term();

	return (LTTRUE);
}

void CRatingMgr::Term()
{
	AdditiveArray::iterator iter = m_AddArray.begin();

	while (iter != m_AddArray.end())
	{
		debug_delete(*iter);
		iter++;
	}

	m_AddArray.clear();
	m_PlayerAddArray.clear();

	g_pRatingMgr = LTNULL;
}

void CRatingMgr::SetPerformanceRating(PerformanceRating p, int iVal)
{
	m_iBaseRating[p] = iVal;
	ComputeAdjustedRatings();
}

uint8 CRatingMgr::GetRating(PerformanceRating p)
{
	return (m_iAdjustedRating[p] > 100 ? 100: m_iAdjustedRating[p]);
}

uint8 CRatingMgr::GetBaseRating(PerformanceRating p)
{
	return (m_iBaseRating[p] > 100 ? 100: m_iBaseRating[p]);
}


void CRatingMgr::RemovePlayerAdditives()
{
	AdditiveArray::iterator iter = m_AddArray.begin();

	while (iter != m_AddArray.end())
	{
		(*iter)->nSlot = -1;
		iter++;
	}
	ComputeAdjustedRatings();
}

void CRatingMgr::GivePlayerAdditive(char * pszName)
{
	ADDITIVE * pAdditive = GetAdditive(pszName);

	if (!pAdditive)
	{
		// assert out here
		return;
	}
	GivePlayerAdditive(pAdditive);
}


void CRatingMgr::GivePlayerAdditive(ADDITIVE * pAdditive)
{
	if (!pAdditive)
	{
		// assert out here
		return;
	}
	// Add the Additive to the end of the list and recompute all adjustments
	m_AddArray.push_back(pAdditive);
	ComputeAdjustedRatings();
}

bool CRatingMgr::PlayerHasAdditive(ADDITIVE * pAdditive)
{
	return (pAdditive->nSlot != -1);
}

bool CRatingMgr::PlayerHasAdditive(char * pszName)
{
	ASSERT (pszName != LTNULL);

	ADDITIVE * pAdd = GetAdditive(pszName);

	if (!pAdd)
		return false;

	if (pAdd->nSlot == -1)
		return false;

	return true;
}

void CRatingMgr::PopulateRatings()
{
	// function to fill the ratings screen
}

ADDITIVE* CRatingMgr::GetAdditive(const char *pszName)
{
	if (!pszName || !pszName[0]) return LTNULL;
	AdditiveArray::iterator iter = m_AddArray.begin();
	while (iter != m_AddArray.end() && stricmp(pszName,(*iter)->szName) != 0)
	{
		iter++;
	}

	if (iter != m_AddArray.end())
		return (*iter);
	return LTNULL;
}

void CRatingMgr::ComputeAdjustedRatings()
{
	for (int i = 0; i <= (int)NUM_RATINGS; i++)
		m_iAdjustedRating[i] = m_iBaseRating[i];

	AdditiveArray::iterator iter = m_AddArray.begin();

	// Traverse the array of additives and add them to the adjusted stats
	while (iter != m_AddArray.end())
	{
		ADDITIVE * pAdd = *iter;
		for (int iter = 0; iter < NUM_RATINGS; iter++)
			m_iAdjustedRating[i] += pAdd->Adjust[iter];

		iter++;
	}
}

