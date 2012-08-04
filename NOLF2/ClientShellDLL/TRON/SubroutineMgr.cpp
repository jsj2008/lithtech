// ----------------------------------------------------------------------- //
//
// MODULE  : SubroutineMgr.cpp
//
// PURPOSE : Attribute file manager for but-related subroutine info
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// TODO
// Set up code to create a concatenated string detailing everything about a subroutine.
// to be used in the popup box
// load and save code.


#include "stdafx.h"
#include "SubroutineMgr.h"
#include "ScreenSubroutines.h"
#include "ScreenRatings.h"
#include "InterfaceMgr.h"
#include "TronPlayerStats.h"
#include "TronMissionButeMgr.h"

#define SUBROUTINE_TAG				"Subroutine"

#define SUBROUTINE_NAME				"Name"
#define SUBROUTINE_NAME_ID			"NameId"
#define SUBROUTINE_DESC				"DescriptionId"
#define SUBROUTINE_SPRITE			"Sprite"
#define SUBROUTINE_FUNCTION			"Function"

// defense-specific fields
#define SUBROUTINE_DEFLECTION		"Deflection"
#define SUBROUTINE_ANTIVIRUS		"Antivirus"
#define SUBROUTINE_ARMORPIECE		"ArmorPiece"

// utility-specific fields
#define SUBROUTINE_CONTINUOUS_DRAIN "ContinuousDrain"
#define SUBROUTINE_BURST_DRAIN		"BurstDrain"

#define PROCEDURAL_TAG				"Procedural"

#define PROCEDURAL_NAME				"Name"
#define PROCEDURAL_NAME_ID			"NameId"
#define PROCEDURAL_DESC				"DescriptionId"
#define PROCEDURAL_EFFECT			"Effect" // corresponds to subroutine "Status"
#define PROCEDURAL_TIME				"Time" // seconds to work
#define PROCEDURAL_SLOT				"Slot" // which of the 5 empty spots does it go in?
#define PROCEDURAL_INTROFX			"IntroFX"
#define PROCEDURAL_COMPLETION		"CompletionId" // string to use when procedural is done

#define PROCEDURAL_RADIUS			"Radius"
#define PROCEDURAL_CENTERPOS		"CenterPos"

#define PROCEDURAL_IDLESKIN			"IdleSkin"
#define PROCEDURAL_WORKSKIN			"WorkSkin"
#define PROCEDURAL_CONDITIONSKIN	"ConditionSkin"
#define PROCEDURAL_PROGRESSSKIN		"ProgressSkin"

#define PRIMITIVE_TAG				"Primitive"
#define PRIMITIVE_NAME				"Name"
#define PRIMITIVE_NAME_ID			"NameId"
           
CSubroutineMgr* g_pSubroutineMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[30];

char * szRingTex[] =
{
	"Interface\\Subroutines\\Sprtex\\sub_optring_alpha.dtx",
	"Interface\\Subroutines\\Sprtex\\sub_optring_beta.dtx",
	"Interface\\Subroutines\\Sprtex\\sub_optring_gold.dtx",
};

// Default constructors
TronSubroutine::TronSubroutine()
{
	szName[0]		= 0;
	nNameId			= 0;
	nDescriptionId	= 0;
	szSprite[0]		= 0;

	eFunction		= FUNCTION_COMBAT;

	// Defensive-only fields
	nDeflection		= 0;
	nAntivirus		= 0;
	nArmorPiece		= -1;

	// Utility-only fields
	fContinuousEnergyDrain = 0.0f;
	fBurstEnergyDrain = 0.0f;
}

PlayerSubroutine::PlayerSubroutine()
{
	pTronSubroutine = LTNULL;
	eVersion		= VERSION_ALPHA;
	nSlot			= -1;
	nTempSlot		= -1;
	fPercentCorrupt = 0.0f;
	fPercentDone	= 0.0f;
	eState			= SUBSTATE_OKAY;
	bWorking		= false;

	// field for utility subroutine
	// TODO set this back to false when we have the mechanism for activating!
	bActive			= true;

	// hack for base code
	bPrevious		= false;
	bNext			= false;
}

Procedural::Procedural()
{
	szName[0]		= 0;
	nNameId			= 0;
	nDescriptionId	= 0;
	nCompletionId	= 0;
	iProcSlot		= 0;
	bPlayerHasThis	= false;
	fTimeRequired	= 1.0f;
	szIntroFX[0]	= 0;

	CenterPos.x		= 320;
	CenterPos.y		= 240;
	iRadius			= 16;

	szIdleSkin[0]		= 0;
	szWorkSkin[0]		= 0;
	szConditionSkin[0]	= 0;
	szProgressSkin[0]	= 0;

	eAffectState	= SUBSTATE_OKAY;
	pSub			= LTNULL;
}

Primitive::Primitive()
{
	szName[0]		= 0;
	nNameId			= 0;
	bPlayerHasThis	= false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSubroutineMgr::CSubroutineMgr()
{
	m_SystemMemoryArray = LTNULL;
	m_fLastUpdateTime = 0.0f;
}

CSubroutineMgr::~CSubroutineMgr()
{
	Term();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSubroutineMgr::Init(const char* szAttributeFile)
{
	int iTemp;
	float fTemp;

	if (g_pSubroutineMgr)
		return LTFALSE;

	// Set up global pointer...
	g_pSubroutineMgr = this;

	m_PlayerSubroutineArray.clear();
	m_SubroutineArray.clear();
	m_ProceduralArray.clear();
	m_PrimitiveArray.clear();

    if (!szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile))
	{
		// If you broke on this assert,
		// you need to grab "attributes/subroutines.txt" out of SourceSafe
		ASSERT(LTFALSE);
		return LTFALSE;
	}


	// Read in all of the Subroutine descriptions
	uint16 nNumSubroutines = 0;

	while (nNumSubroutines < 100)
	{
		sprintf(s_aTagName, "%s%d", SUBROUTINE_TAG, nNumSubroutines);
		if (m_buteMgr.Exist(s_aTagName))
		{
			TronSubroutine* pNew = debug_new(TronSubroutine);
			m_buteMgr.GetString(s_aTagName, SUBROUTINE_NAME, "", pNew->szName, sizeof(pNew->szName));
			pNew->nNameId = (uint16)m_buteMgr.GetInt(s_aTagName, SUBROUTINE_NAME_ID, 0);
			pNew->nDescriptionId = (uint16)m_buteMgr.GetInt(s_aTagName, SUBROUTINE_DESC, 0);
			m_buteMgr.GetString(s_aTagName, SUBROUTINE_SPRITE, "", pNew->szSprite, sizeof(pNew->szSprite));

			// Combat/defense/util/Unusable
			iTemp = m_buteMgr.GetInt(s_aTagName, SUBROUTINE_FUNCTION, 0);
			if (iTemp >= (int)FUNCTION_COMBAT && iTemp < (int)FUNCTION_LAST)
				pNew->eFunction = (SubFunction)iTemp;

			// Defense-specific fields
			if (pNew->eFunction == FUNCTION_DEFENSE)
			{
				iTemp = m_buteMgr.GetInt(s_aTagName, SUBROUTINE_DEFLECTION, 0);
				if (iTemp >= 0 && iTemp <= 100)
					pNew->nDeflection = (uint8)iTemp;

				iTemp = m_buteMgr.GetInt(s_aTagName, SUBROUTINE_ANTIVIRUS, 0);
				if (iTemp >= 0 && iTemp <= 100)
					pNew->nAntivirus = (uint8)iTemp;

				iTemp = m_buteMgr.GetInt(s_aTagName, SUBROUTINE_ARMORPIECE, 0);
				if (iTemp >= 0 && iTemp <= 100)
					pNew->nArmorPiece = (uint8)iTemp;
			}
			else if (pNew->eFunction == FUNCTION_UTILITY)
			{
				fTemp = m_buteMgr.GetFloat(s_aTagName, SUBROUTINE_CONTINUOUS_DRAIN, 0.0f);
				if (fTemp >= 0.0f && fTemp < 100.0f)
					pNew->fContinuousEnergyDrain = fTemp;

				fTemp = m_buteMgr.GetFloat(s_aTagName, SUBROUTINE_BURST_DRAIN, 0.0f);
				if (fTemp >= 0.0f && fTemp < 100.0f)
					pNew->fBurstEnergyDrain = fTemp;
			}

			m_SubroutineArray.push_back(pNew);

		}
		nNumSubroutines++;
	}
	uint16 nNumProcedurals = 0;
	sprintf(s_aTagName, "%s0", PROCEDURAL_TAG);
	while (m_buteMgr.Exist(s_aTagName))
	{
		Procedural* pNew = debug_new(Procedural);

		m_buteMgr.GetString(s_aTagName, PROCEDURAL_NAME, "", pNew->szName, sizeof(pNew->szName));
		pNew->nNameId = (uint16)m_buteMgr.GetInt(s_aTagName, PROCEDURAL_NAME_ID, 0);
		pNew->nDescriptionId = (uint16)m_buteMgr.GetInt(s_aTagName, PROCEDURAL_DESC, 0);
		pNew->nCompletionId = (uint16)m_buteMgr.GetInt(s_aTagName, PROCEDURAL_COMPLETION, 0);
		m_buteMgr.GetString(s_aTagName, PROCEDURAL_INTROFX, "", pNew->szIntroFX, sizeof(pNew->szIntroFX));

		CPoint pos = m_buteMgr.GetPoint(s_aTagName, PROCEDURAL_CENTERPOS);
		pNew->CenterPos.x = pos.x;
		pNew->CenterPos.y = pos.y;

		pNew->iRadius = (uint16)m_buteMgr.GetInt(s_aTagName, PROCEDURAL_RADIUS, 16);

		m_buteMgr.GetString(s_aTagName, PROCEDURAL_IDLESKIN,		"", pNew->szIdleSkin,		sizeof(pNew->szIdleSkin));
		m_buteMgr.GetString(s_aTagName, PROCEDURAL_WORKSKIN,		"", pNew->szWorkSkin,		sizeof(pNew->szWorkSkin));
		m_buteMgr.GetString(s_aTagName, PROCEDURAL_CONDITIONSKIN,	"", pNew->szConditionSkin,	sizeof(pNew->szConditionSkin));
		m_buteMgr.GetString(s_aTagName, PROCEDURAL_PROGRESSSKIN,	"", pNew->szProgressSkin,	sizeof(pNew->szProgressSkin));
		
		// What kind of sub does it work on
		iTemp = m_buteMgr.GetInt(s_aTagName, PROCEDURAL_EFFECT, 0);
		if (iTemp >= (int)SUBSTATE_OKAY && iTemp < (int)SUBSTATE_LAST)
			pNew->eAffectState = (SubState)iTemp;

		// Time required
		fTemp = m_buteMgr.GetFloat(s_aTagName, PROCEDURAL_TIME, 1.0f);
		if (fTemp >= 1.0f && fTemp < 10000.0f)
			pNew->fTimeRequired = fTemp;

		// Slot on the sub screen
		iTemp = m_buteMgr.GetInt(s_aTagName, PROCEDURAL_SLOT, 1);
		if (iTemp >= 1 && iTemp <= 5)
			pNew->iProcSlot = iTemp;

		m_ProceduralArray.push_back(pNew);

		nNumProcedurals++;
		sprintf(s_aTagName, "%s%d", PROCEDURAL_TAG, nNumProcedurals);
	}

	// Read all the primitives available
	uint16 nNumPrimitives = 0;
	sprintf(s_aTagName, "%s0", PRIMITIVE_TAG);
	while (m_buteMgr.Exist(s_aTagName))
	{
		Primitive * pNewPrim = debug_new(Primitive);

		m_buteMgr.GetString(s_aTagName, PRIMITIVE_NAME, "", pNewPrim->szName, sizeof(pNewPrim->szName));
		pNewPrim->nNameId = (uint16)m_buteMgr.GetInt(s_aTagName, PRIMITIVE_NAME_ID, 0);

		m_PrimitiveArray.push_back(pNewPrim);
		nNumPrimitives++;
		sprintf(s_aTagName, "%s%d", PRIMITIVE_TAG, nNumPrimitives);
	}

	// clean up memory used for parsing the bute file
	m_buteMgr.Term();
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CSubroutineMgr::Term()
{
    g_pSubroutineMgr = LTNULL;
	CleanArrays();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::Pause()
//
//	PURPOSE:	Note the pausing and unpausing of the game.
//
// ----------------------------------------------------------------------- //

void CSubroutineMgr::Pause(LTBOOL bPause)
{
	m_bPause = bPause;
	if (!m_bPause)
	{
		m_fLastUpdateTime = g_pLTClient->GetTime();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::Update()
//
//	PURPOSE:	Note the passage of time when the game is not paused
//				Advance the progress on any active procedurals and handle
//				completion of any procedural events
//
// ----------------------------------------------------------------------- //

void CSubroutineMgr::Update()
{
	if (m_bPause)
		return;

	m_bProceduralActive = false;

	float fNow = g_pLTClient->GetTime();

	unsigned short iVersion;
	g_pTronPlayerStats->GetJetVersion(iVersion);
	float fPlayerLevel = (float)(iVersion - (iVersion % 100)) / 100.0f;

	// advance any timers.
	float fDeltaTime = fNow - m_fLastUpdateTime;
	m_fLastUpdateTime = fNow;

	ProceduralArray::iterator iter = m_ProceduralArray.begin();
	while (iter != m_ProceduralArray.end())
	{
		Procedural *pProc = *iter;
		if (pProc->bPlayerHasThis)
		{
			PlayerSubroutine * pSub = pProc->pSub;
			if (pSub)
			{
				m_bProceduralActive = true;
				// advance the progress based upon how much the local timer has advanced
				// compute the advancement rate.  Start with the base procedural speed.
				// then reduce by 10% for each level that the player has advanced?
				float fAdjustedTime = pProc->fTimeRequired * (1.0f - (0.1f * fPlayerLevel));
				float fAdvancementPerSecond = 1.0f / fAdjustedTime;

				pSub->fPercentDone += (fAdvancementPerSecond * fDeltaTime);

				if (pSub->fPercentDone >= 1.0f)
				{
					ProceduralFinished(pProc);
				}
			}
		}
		iter++;
	}
	if (m_bProceduralActive)
	{
		g_pHUDMgr->QueueUpdate(kHUDProcedurals);
	}
	// TODO advance any corruption UNLESS being worked on by a procedural
	// TODO subtract any energy for continuously draining subroutines
	// TODO write a function for burst-style energy drainage
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::ProceduralFinished()
//
//	PURPOSE:	Handle the specific details of a procedural's completion
//
// ----------------------------------------------------------------------- //

void CSubroutineMgr::ProceduralFinished(Procedural * pProc)
{
	PlayerSubroutine * pSub = pProc->pSub;
	pSub->bWorking = false;

	// Detach the subroutine from this procedural
	pProc->pSub = LTNULL;

	switch (pProc->eAffectState)
	{
	case SUBSTATE_FOREIGN:			// port
		{
			pSub->eState = SUBSTATE_OKAY;
			// put up a message that this subroutine has been successfully ported
		}
		break;

	case SUBSTATE_CORRUPT:			// disinfect
		{
			pSub->eState = SUBSTATE_OKAY;
			pSub->fPercentCorrupt = 0.0f;
			// put up a message that this subroutine has been successfully disinfected
		}
		break;

	case SUBSTATE_UNUSABLE:			// bad block
		{
			g_pLTClient->CPrint("Bad Block Removed\n");
			// REMOVE the subroutine from system memory
			PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
			while (iter != m_PlayerSubroutineArray.end())
			{
				if (*iter == pSub)
				{
					m_PlayerSubroutineArray.erase(iter);
					break;
				}
				++iter;
			}
			// put up a message that this block has been repaired
		}
		break;

	case SUBSTATE_DELETED:			// undeleter
		{
			pSub->eState = SUBSTATE_OKAY;
			// put up a message that this subroutine has been restored
		}
		break;

	case SUBSTATE_OKAY:				// this was a "shredder", and should never happen
	default:
		break;
	}
}

void CSubroutineMgr::CleanArrays()
{
	SubroutineArray::iterator iter = m_SubroutineArray.begin();

	while (iter != m_SubroutineArray.end())
	{
		debug_delete(*iter);
		iter++;
	}

	m_SubroutineArray.clear();

	PlayerSubroutineArray::iterator iter2 = m_PlayerSubroutineArray.begin();

	while (iter2 != m_PlayerSubroutineArray.end())
	{
		debug_delete(*iter2);
		iter2++;
	}

	m_PlayerSubroutineArray.clear();

	ProceduralArray::iterator iter3 = m_ProceduralArray.begin();

	while (iter3 != m_ProceduralArray.end())
	{
		debug_delete(*iter3);
		iter3++;
	}

	m_ProceduralArray.clear();

	PrimitiveArray::iterator iter4 = m_PrimitiveArray.begin();

	while (iter4 != m_PrimitiveArray.end())
	{
		debug_delete(*iter4);
		iter4++;
	}
}

TronSubroutine* CSubroutineMgr::GetSubroutine(const char *pszName)
{
	if (!pszName || !pszName[0]) return LTNULL;
	SubroutineArray::iterator iter = m_SubroutineArray.begin();
	while (iter != m_SubroutineArray.end() && stricmp(pszName,(*iter)->szName) != 0)
	{
		iter++;
	}

	if (iter != m_SubroutineArray.end())
		return (*iter);

	return LTNULL;
}

void CSubroutineMgr::ClearPlayerSubroutines()
{
/*
	// Wipe the array of player subroutines, because the Subroutine Screen is about to spit a new list back
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();

	while (iter != m_PlayerSubroutineArray.end())
	{
		debug_delete(*iter);
		iter++;
	}
*/
	m_PlayerSubroutineArray.clear();
}

void CSubroutineMgr::AddPlayerSubroutine(PlayerSubroutine * pSub)
{
	// assume that all duplicates have been removed
	if (pSub)
		m_PlayerSubroutineArray.push_back(pSub);
}


void CSubroutineMgr::GivePlayerSubroutine(char * name, char * state, char * condition)
{
	// Get a pointer to the subroutine template, if any
	TronSubroutine * pSub = GetSubroutine(name);
	if (pSub == LTNULL)
		return;
/* DUPLICATE subs are now allowed 3/28/02
	// iterate through player's subroutines and make sure that they don't already have it
	PlayerSubroutineArray::iterator playerIter = m_PlayerSubroutineArray.begin();
	while (playerIter != m_PlayerSubroutineArray.end())
	{
		if ((*playerIter)->pTronSubroutine == pSub)
		{
			// send a message to screen that the player already has this sub
			g_pLTClient->CPrint("Player already has '%s'.\n");
			return;
		}
		playerIter++;
	}
*/
	// Create a new PlayerSubroutine and link it to the TronSubroutine
	PlayerSubroutine *pPlayerSub = debug_new(PlayerSubroutine);
	pPlayerSub->pTronSubroutine		= pSub;

	// Set the state based on parameters passed to this function
	pPlayerSub->eVersion			= VERSION_ALPHA;

	if (!strcmpi(state,"beta"))
		pPlayerSub->eVersion = VERSION_BETA;
	else if (!strcmpi(state, "gold"))
		pPlayerSub->eVersion = VERSION_GOLD;

	pPlayerSub->eState				= SUBSTATE_OKAY;

	if (!strcmpi(condition,"infected"))
		pPlayerSub->eState = SUBSTATE_CORRUPT;
	else if (!strcmpi(condition,"noncompatible"))
		pPlayerSub->eState = SUBSTATE_FOREIGN;

	pPlayerSub->nSlot				= -1;

	m_PlayerSubroutineArray.push_back(pPlayerSub);

	// send a message that this subroutine has been successfully acquired
	g_pLTClient->CPrint("Acquired %s subroutine '%s' (%s)", state, name, condition);
/*
	char szName[64];
	LoadString(nNameId,szName,sizeof(szName));
	if (!strlen(szName)) return;

	char szMsg[128];
	FormatString(IDS_GEARPICKUP,szMsg,sizeof(szMsg),szName);
	g_pPickupMsgs->AddMessage(szMsg,pGear->szIcon);
*/
}

void CSubroutineMgr::PopulateSubroutineScreen()
{
	// Wipe the subroutine screen
	CScreenSubroutines *pSubScreen = (CScreenSubroutines *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_SUBROUTINES);
	pSubScreen->ClearScreen();

	ProceduralArray::iterator iter = m_ProceduralArray.begin();
	while (iter != m_ProceduralArray.end())
	{
			pSubScreen->AddProcedural(*iter);
		iter++;
	}

	if (m_PlayerSubroutineArray.size())
	{
		PlayerSubroutineArray::iterator playerIter = m_PlayerSubroutineArray.begin();
		while (playerIter != m_PlayerSubroutineArray.end())
		{
			pSubScreen->AddSubroutine(*playerIter);
			playerIter++;
		}
	}
}

bool CSubroutineMgr::IsUtilitySubroutineActive(char * name)
{
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		PlayerSubroutine * pSub = *iter;
		// Look at installed subs
		if (pSub->nSlot >= 0 && pSub->nSlot < 100)
		{
			// look for a name match.  Should only be one.
			if (!strcmpi(name, pSub->pTronSubroutine->szName))
			{
				if (pSub->bActive)
					return true;
			}
		}
		iter++;
	}
	return false;
}

SubVersion CSubroutineMgr::GetActiveSubroutineVersion(char * name)
{
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		PlayerSubroutine * pSub = *iter;
		// Look at installed subs
		if (pSub->nSlot >= 0 && pSub->nSlot < 100)
		{
			// look for a name match.  Should only be one.
			if (!strcmpi(name, pSub->pTronSubroutine->szName))
			{
				if (pSub->bActive)
				{
					return (pSub->eVersion);
				}
			}
		}
		iter++;
	}
	return VERSION_ALPHA;
}

float CSubroutineMgr::GetActiveSubroutineCorruption(char * name)
{
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		PlayerSubroutine * pSub = *iter;
		// Look at installed subs
		if (pSub->nSlot >= 0 && pSub->nSlot < 100)
		{
			// look for a name match.  Should only be one.
			if (!strcmpi(name, pSub->pTronSubroutine->szName))
			{
				if (pSub->bActive)
				{
					if (pSub->eState == SUBSTATE_CORRUPT)
					{
						return (pSub->fPercentCorrupt);
					}
					else
					{
						return 0.0f;
					}
				}
			}
		}
		iter++;
	}
	return 0.0f;
}

Procedural* CSubroutineMgr::GetProcedural(const char *pszName)
{
	if (!pszName || !pszName[0]) return LTNULL;
	ProceduralArray::iterator iter = m_ProceduralArray.begin();
	while (iter != m_ProceduralArray.end() && stricmp(pszName,(*iter)->szName) != 0)
	{
		iter++;
	}

	if (iter != m_ProceduralArray.end())
		return (*iter);

	return LTNULL;
}

void CSubroutineMgr::GivePlayerProcedural(char * name)
{
	// Get a pointer to the procedural, if any
	Procedural * pProc = GetProcedural(name);
	if (pProc == LTNULL)
		return;
	// see if we have this proc already
	if (pProc->bPlayerHasThis)
	{
		// Put up a message that the player already has this procedural
		return;
	}

	pProc->bPlayerHasThis = true;
	// Put up a message that the player has acquired the procedural
}

bool CSubroutineMgr::CanProcHookSub(Procedural *pProc, PlayerSubroutine *pSub)
{
	// Each procedural can fix one kind of effect.
	// Each subroutine can suffer from one kind of effect.
	// When we have a match, then we're good.
	if (pProc->eAffectState == pSub->eState)
		return true;

	return false;
}

void CSubroutineMgr::HookProcToSub(Procedural *pProc, PlayerSubroutine *pSub)
{
	// Clear out any previous hook
	if (pProc->pSub)
	{
		pProc->pSub->bWorking = false;
		pProc->pSub = LTNULL;
	}

	if (pSub)
	{
		pSub->bWorking = pProc ? true : false;

		if (!CanProcHookSub(pProc, pSub))
			return;
	}

	pProc->pSub = pSub;
}

Procedural * CSubroutineMgr::WhoHookedToSub(PlayerSubroutine *pSub)
{
	// Iterate through the array of procs and determine who, if anyone, is hooked
	// to this sub
	ProceduralArray::iterator iter = m_ProceduralArray.begin();
	while (iter != m_ProceduralArray.end())
	{
		if ((*iter)->pSub == pSub)
			return (*iter);
		iter++;
	}
	return LTNULL;
}

void CSubroutineMgr::ComputeArmorCoverage()
{
	float fArmorCoverage	= 0.0f;
	float fVirusCoverage	= 0.0f;
	float fArmorScalar		= 0.0f;

	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		// For each installed subroutine
		if ((*iter)->nSlot >= 0 && (*iter)->nSlot < 100)
		{
			// For each installed subroutine that's DEFENSE
			TronSubroutine *pTronSub = (*iter)->pTronSubroutine;
			if (pTronSub->eFunction == FUNCTION_DEFENSE)
			{
//---------------------------------
				float fWeight = 0.0f;
				float fDeflect = (float)pTronSub->nDeflection / 100.0f;

				// Percentage of damage converted to energy drain
				fArmorCoverage += fDeflect;

				// Computation of scalar to energy cost
				if ((*iter)->eVersion == VERSION_ALPHA)
					fWeight = 1.0f;
				else if ((*iter)->eVersion == VERSION_BETA)
					fWeight = 0.75f;
				else if ((*iter)->eVersion == VERSION_GOLD)
					fWeight = 0.5f;

				fArmorScalar += fDeflect * fWeight;

				// Virus coverage
				fVirusCoverage += (float)pTronSub->nAntivirus / 100.0f;
//---------------------------------
			}
		}
		iter++;
	}
	fArmorScalar /= fArmorCoverage;

	g_pTronPlayerStats->UpdateSubroutineArmor(fArmorCoverage, fVirusCoverage, fArmorScalar);
}

bool CSubroutineMgr::HaveArmorPiece(int iPiece)
{
	PlayerSubroutineArray::iterator iter = m_PlayerSubroutineArray.begin();
	while (iter != m_PlayerSubroutineArray.end())
	{
		if ((*iter)->nSlot >= 0 && (*iter)->nSlot < 100)
		{
			TronSubroutine *pTronSub = (*iter)->pTronSubroutine;
			if (pTronSub->eFunction == FUNCTION_DEFENSE)
			{
				if (pTronSub->nArmorPiece == iPiece)
				{
					return true;
				}
			}
		}
		iter++;
	}
	return false;
}

void CSubroutineMgr::GivePlayerPrimitive(char * name)
{
	if (!name)
		return;
	if (!name[0])
		return;

	PrimitiveArray::iterator iter = m_PrimitiveArray.begin();
	while (iter != m_PrimitiveArray.end())
	{
		if (!strcmpi((*iter)->szName, name))
		{
			if ((*iter)->bPlayerHasThis)
			{
				g_pLTClient->CPrint("Player already has primitive: %s\n",(*iter)->szName);
			}
			else
			{
				(*iter)->bPlayerHasThis = true;
				g_pLTClient->CPrint("Player acquired primitive: %s\n",(*iter)->szName);
			}
			return;
		}
		iter++;
	}
}

bool CSubroutineMgr::DoesPlayerHavePrimitive(char * name)
{
	PrimitiveArray::iterator iter = m_PrimitiveArray.begin();
	while (iter != m_PrimitiveArray.end())
	{
		if (!strcmpi((*iter)->szName, name))
		{
			return true;
		}
		iter++;
	}
	return false;
}

void CSubroutineMgr::Compile()
{
	// Compute armor, armor efficiency and sent to PlayerStats
	ComputeArmorCoverage();

	// TODO add any other Compile-related tasks here

	g_pTronPlayerStats->Compile();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::SetSystemMemoryConfiguration
//
//	PURPOSE:	To set the system memory configuration
//				(bad blocks, base code, etc...)
//
//	   NOTE:	The pSystemMemoryConfig is made up of:
//				'.'			= a normal block
//				'b' or 'B'	= a bad block
//				'c' or 'C'	= a base code block
//
// ----------------------------------------------------------------------- //
bool CSubroutineMgr::SetSystemMemoryConfiguration(const char* pSystemMemoryConfig)
{
	if (m_SystemMemoryArray)
		debug_delete(m_SystemMemoryArray);

	int iLen = 0;

	if(pSystemMemoryConfig)
	{
		m_SystemMemoryArray = strdup(pSystemMemoryConfig);
		iLen = strlen(pSystemMemoryConfig);
	}
	else
	{
		m_SystemMemoryArray = LTNULL;
	}

	// loop through the string and create the bad blocks.
	if (iLen > 24) iLen = 24;
	
	for (int i = 0; i < iLen; i++)
	{
		char c = pSystemMemoryConfig[i];

		if (c == 'b' || c == 'B')
		{
			AddBadBlock(i);
		}
		else if (c == 'C' || c == 'c')
		{
			char cPrev = pSystemMemoryConfig[(i+23) % 24];
			char cNext = pSystemMemoryConfig[(i+1) % 24];

			bool bPrev = (cPrev == 'c' || cPrev == 'C');
			bool bNext = (cNext == 'c' || cNext == 'C');

			AddBaseCodeBlock(i, bPrev, bNext);
		}
	}
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::AddBadBlock
//
//	PURPOSE:	Create a new subroutine that's just a Bad Block
// ----------------------------------------------------------------------- //

void CSubroutineMgr::AddBadBlock(int iSectorNumber)
{
	// Get a pointer to the subroutine template, if any
	TronSubroutine * pSub = GetSubroutine("bad_block");

	if (pSub == LTNULL)
		return;

	// Create a new PlayerSubroutine and link it to the TronSubroutine
	PlayerSubroutine *pPlayerSub = debug_new(PlayerSubroutine);

	pPlayerSub->pTronSubroutine		= pSub;
	pPlayerSub->eVersion			= VERSION_GOLD;
	pPlayerSub->eState				= SUBSTATE_UNUSABLE;

	int iRealSector = ((5 - iSectorNumber) + 24) % 24;
	pPlayerSub->nSlot				= iRealSector;

	m_PlayerSubroutineArray.push_back(pPlayerSub);

	// send a message that this subroutine has been successfully acquired
//	g_pLTClient->CPrint("Acquired bad block on sector (%d)", iSectorNumber);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSubroutineMgr::AddBaseCodeBlock
//
//	PURPOSE:	Create a new subroutine that's Base Code
// ----------------------------------------------------------------------- //

void CSubroutineMgr::AddBaseCodeBlock(int iSectorNumber, bool bPrev, bool bNext)
{
	// Get a pointer to the subroutine template for base code
	TronSubroutine * pSub = GetSubroutine("base_code");

	if (pSub == LTNULL)
		return;

	// Create a new PlayerSubroutine and link it to the TronSubroutine
	PlayerSubroutine *pPlayerSub = debug_new(PlayerSubroutine);

	pPlayerSub->pTronSubroutine		= pSub;
	pPlayerSub->eVersion			= VERSION_GOLD;
	pPlayerSub->eState				= SUBSTATE_UNUSABLE;

	int iRealSector = ((5 - iSectorNumber) + 24) % 24;
	pPlayerSub->nSlot				= iRealSector;

	pPlayerSub->bPrevious = bPrev;
	pPlayerSub->bNext = bNext;

	m_PlayerSubroutineArray.push_back(pPlayerSub);

	// send a message that this subroutine has been successfully acquired
//	g_pLTClient->CPrint("Acquired base code on sector (%d)", iSectorNumber);
}
