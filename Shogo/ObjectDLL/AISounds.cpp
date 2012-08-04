// ----------------------------------------------------------------------- //
//
// MODULE  : AISounds.cpp
//
// PURPOSE : AI Sounds related functions
//
// CREATED : 7/12/98
//
// ----------------------------------------------------------------------- //

#include "AISounds.h"
#include "BaseAI.h"
#include "RiotObjectUtilities.h"
#include "cpp_server_de.h"

static char* GetFactionDir(BaseAI* pAI);
static DBOOL CanPlaySound(BaseAI* pAI, AISoundType eType);
static DBOOL IsTrooper(BaseAI* pAI);
static DBOOL IsShockTrooper(BaseAI* pAI);
static DBOOL IsCivilian(BaseAI* pAI);
static DBOOL SetupFileBuffer(BaseAI* pAI);

static char* s_DeathSounds[]	= { "Death1.wav", "Death2.wav", "Death3.wav" };
static int   s_nNumDeathSounds	= (sizeof(s_DeathSounds)/sizeof(s_DeathSounds[0]));

static char* s_PainSounds[]		= { "Pain1.wav", "Pain2.wav", "Pain3.wav" };
static int   s_nNumPainSounds	= (sizeof(s_PainSounds)/sizeof(s_PainSounds[0]));

static char* s_SetAggressiveSounds[]	= { "SetAgg1.wav", "SetAgg2.wav", "SetAgg3.wav", "SetAgg4.wav", "SetAgg5.wav" };
static int   s_nNumSetAggressiveSounds	= (sizeof(s_SetAggressiveSounds)/sizeof(s_SetAggressiveSounds[0]));

static char* s_LostTargetSounds[]	= { "LostTar2.wav", "LostTar2.wav" };
static int   s_nNumLostTargetSounds	= (sizeof(s_LostTargetSounds)/sizeof(s_LostTargetSounds[0]));

static char* s_BumpedSounds[]	= { "Bumped1.wav", "Bumped2.wav", "Bumped3.wav", "Bumped4.wav", "Bumped5.wav" };
static int   s_nNumBumpedSounds	= (sizeof(s_BumpedSounds)/sizeof(s_BumpedSounds[0]));

static char* s_SetDefensiveSounds[]		= { "SetDef1.wav", "SetDef2.wav", "SetDef3.wav", "SetDef4.wav", "SetDef5.wav" };
static int   s_nNumSetDefensiveSounds	= (sizeof(s_SetDefensiveSounds)/sizeof(s_SetDefensiveSounds[0]));

static char* s_SetRetreatingSounds[]	= { "SetRet1.wav", "SetRet2.wav" };
static int   s_nNumSetRetreatingSounds	= (sizeof(s_SetRetreatingSounds)/sizeof(s_SetRetreatingSounds[0]));

static char* s_SetPanickedSounds[]		= { "SetPan1.wav", "SetPan2.wav", "SetPan3.wav" };
static int   s_nNumSetPanickedSounds	= (sizeof(s_SetPanickedSounds)/sizeof(s_SetPanickedSounds[0]));

static char* s_SetPsychoSounds[]	= { "SetPsy1.wav", "SetPsy2.wav" };
static int   s_nNumSetPsychoSounds	= (sizeof(s_SetPsychoSounds)/sizeof(s_SetPsychoSounds[0]));

static char* s_FollowLostSounds[]	= { "Follow1.wav", "Follow2.wav" };
static int   s_nNumFollowLostSounds	= (sizeof(s_FollowLostSounds)/sizeof(s_FollowLostSounds[0]));

static char* s_PanickedSounds[]		= { "Panick1.wav", "Panick2.wav" };
static int   s_nNumPanickedSounds	= (sizeof(s_PanickedSounds)/sizeof(s_PanickedSounds[0]));




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDeathSound()
//
//	PURPOSE:	Get the AI death sound
//
// ----------------------------------------------------------------------- //

char* GetDeathSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_DEATH)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	strcat(s_FileBuffer, s_DeathSounds[GetRandom(0, s_nNumDeathSounds - 1)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetDamageSound()
//
//	PURPOSE:	Get the AI damage sound
//
// ----------------------------------------------------------------------- //

char* GetDamageSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_DAMAGE)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	strcat(s_FileBuffer, s_PainSounds[GetRandom(0, s_nNumPainSounds - 1)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSetAggressiveSound()
//
//	PURPOSE:	Get the AI set aggressive sound
//
// ----------------------------------------------------------------------- //

char* GetSetAggressiveSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_SETAGGRESSIVE)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	strcat(s_FileBuffer, s_SetAggressiveSounds[GetRandom(0, s_nNumSetAggressiveSounds - 1)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetLostTargetSound()
//
//	PURPOSE:	Get the AI lost target sound
//
// ----------------------------------------------------------------------- //

char* GetLostTargetSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_LOSTTARGET)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	strcat(s_FileBuffer, s_LostTargetSounds[GetRandom(0, s_nNumLostTargetSounds - 1)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetBumpedSound()
//
//	PURPOSE:	Get the AI bumped sound
//
// ----------------------------------------------------------------------- //

char* GetBumpedSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_BUMPED)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	int nNumBumpedSounds = s_nNumBumpedSounds;
	if (IsMajorCharacter(pAI->m_hObject))
	{
		nNumBumpedSounds = 2;	
	}

	strcat(s_FileBuffer, s_BumpedSounds[GetRandom(0, nNumBumpedSounds - 1)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSetDefensiveSound()
//
//	PURPOSE:	Get the AI set defensive sound
//
// ----------------------------------------------------------------------- //

char* GetSetDefensiveSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_SETDEFENSIVE)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	strcat(s_FileBuffer, s_SetDefensiveSounds[GetRandom(0, s_nNumSetDefensiveSounds - 1)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSetRetreatingSound()
//
//	PURPOSE:	Get the AI set retreating sound
//
// ----------------------------------------------------------------------- //

char* GetSetRetreatingSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_SETRETREATING)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	strcat(s_FileBuffer, s_SetRetreatingSounds[GetRandom(0, s_nNumSetRetreatingSounds - 1)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSetPanickedSound()
//
//	PURPOSE:	Get the AI set panicked sound
//
// ----------------------------------------------------------------------- //

char* GetSetPanickedSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_SETPANICKED)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	if (IsCivilian(pAI))
	{
		strcat(s_FileBuffer, s_SetPanickedSounds[GetRandom(0, s_nNumSetPanickedSounds - 1)]);
	}
	else
	{
		strcat(s_FileBuffer, s_SetPanickedSounds[0]);
	}

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSetPsychoSound()
//
//	PURPOSE:	Get the AI set psycho sound
//
// ----------------------------------------------------------------------- //

char* GetSetPsychoSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_SETPSYCHO)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	strcat(s_FileBuffer, s_SetPsychoSounds[GetRandom(0, s_nNumSetPsychoSounds - 1)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetFollowLostSound()
//
//	PURPOSE:	Get the AI follow lost sound
//
// ----------------------------------------------------------------------- //

char* GetFollowLostSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_FOLLOWLOST)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	strcat(s_FileBuffer, s_FollowLostSounds[GetRandom(0, s_nNumFollowLostSounds - 1)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetPanickedSound()
//
//	PURPOSE:	Get the AI panicked sound
//
// ----------------------------------------------------------------------- //

char* GetPanickedSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_PANICKED)) return DNULL;
	if (!SetupFileBuffer(pAI)) return DNULL;

	strcat(s_FileBuffer, s_PanickedSounds[GetRandom(0, s_nNumPanickedSounds - 1)]);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSpotSound()
//
//	PURPOSE:	Get the AI spot sound
//
// ----------------------------------------------------------------------- //

char* GetSpotSound(BaseAI* pAI)
{
	if (!pAI || !CanPlaySound(pAI, AIS_SPOT)) return DNULL;

	SAFE_STRCPY(s_FileBuffer, "Sounds\\Enemies\\Spot\\");

	char* pSound = DNULL;
	switch (pAI->GetCharacterClass())
	{
		case CMC:
			pSound = "CMC.wav";
		break;
		case SHOGO:
			pSound = "Shogo.wav";
		break;
		case UCA:
			pSound = "UCA.wav";
		break;
		case UCA_BAD:
			pSound = "UCA_BAD.wav";
		break;
		case FALLEN:
			pSound = "FALLEN.wav";
		break;
		case CRONIAN:
			pSound = "CRONIAN.wav";
		break;
		case STRAGGLER:
			pSound = "Straggler.wav";
		break;
		case BYSTANDER:
			pSound = "Bystander.wav";
		break;
		case ROGUE:
			pSound = "Rogue.wav";
		break;
		case COTHINEAL:
			pSound = "Cothineal.wav";
		break;
		
		default :
			return DNULL;
		break;
	}
	
	strcat(s_FileBuffer, pSound);

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetFactionDir()
//
//	PURPOSE:	Get the name of the dir ai faction
//
// ----------------------------------------------------------------------- //

static char* GetFactionDir(BaseAI* pAI)
{
	if (!pAI) return DNULL;

	char* pRet = DNULL;

	if (IsTrooper(pAI) || IsShockTrooper(pAI))
	{
		CharacterClass cc = pAI->GetCharacterClass();
		switch (cc)
		{
			case CMC:
			case SHOGO:
				pRet = "_cmc";
			break;
			
			case UCA:
			case UCA_BAD:
				pRet = "_uca";
			break;
			
			case FALLEN:
			case CRONIAN:
			case STRAGGLER:
				pRet = "_fallen";
			break;
			
			default :
			break;
		}
	}

	return pRet;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CanPlaySound()
//
//	PURPOSE:	Determine if the AI can play the specific sound type
//
// ----------------------------------------------------------------------- //

static DBOOL CanPlaySound(BaseAI* pAI, AISoundType eType)
{
	if (!pAI) return DFALSE;

	DBOOL bRet = DFALSE;

	if (IsVehicle(pAI->m_hObject)) return DFALSE;

	switch (eType)
	{
		case AIS_DEATH:
		case AIS_DAMAGE:
		case AIS_SPOT:
		case AIS_SETPANICKED:
			bRet = DTRUE;
		break;

		case AIS_SETPSYCHO:
		case AIS_SETRETREATING:
		case AIS_SETDEFENSIVE:
		case AIS_LOSTTARGET:
			bRet = (IsTrooper(pAI) || IsShockTrooper(pAI));
		break;

		case AIS_BUMPED:
			bRet = (IsTrooper(pAI) || IsCivilian(pAI) || IsMajorCharacter(pAI->m_hObject));
		break;
		
		case AIS_SETAGGRESSIVE:
			bRet = (pAI->IsMecha() || IsTrooper(pAI) || IsShockTrooper(pAI));
		break;

		case AIS_PANICKED:
			bRet = IsCivilian(pAI);
		break;

		case AIS_FOLLOWLOST:
			bRet = IsMajorCharacter(pAI->m_hObject);
		break;

		default : break;
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsTrooper()
//
//	PURPOSE:	Determine if the AI is a trooper
//
// ----------------------------------------------------------------------- //

static DBOOL IsTrooper(BaseAI* pAI)
{
	if (!pAI) return DFALSE;

	DBYTE id = pAI->GetModelId();
	if (id == MI_AI_TROOPER_ID || id == MI_AI_ETROOPER_ID || 
		id == MI_AI_OFFICER_ID) return DTRUE;

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsShockTrooper()
//
//	PURPOSE:	Determine if the AI is a shock trooper
//
// ----------------------------------------------------------------------- //

static DBOOL IsShockTrooper(BaseAI* pAI)
{
	if (!pAI) return DFALSE;

	DBYTE id = pAI->GetModelId();
	return (id == MI_AI_STROOPER_ID || id == MI_AI_ESTROOPER_ID);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsCivilian()
//
//	PURPOSE:	Determine if the AI is a shock trooper
//
// ----------------------------------------------------------------------- //

static DBOOL IsCivilian(BaseAI* pAI)
{
	if (!pAI) return DFALSE;

	DBYTE id = pAI->GetModelId();
	if (id == MI_AI_LITTLEBOY_ID || 
		id == MI_AI_LITTLEGIRL_ID ||
		id == MI_AI_CIVILIAN1_ID ||
		id == MI_AI_CIVILIAN1B_ID ||
		id == MI_AI_CIVILIAN2_ID) return DTRUE;

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SetupFileBuffer()
//
//	PURPOSE:	Set up the file buffer
//
// ----------------------------------------------------------------------- //

static DBOOL SetupFileBuffer(BaseAI* pAI)
{
	if (!pAI) return DFALSE;

	SAFE_STRCPY(s_FileBuffer, "Sounds\\Enemies\\");

	if (IsMajorCharacter(pAI->m_hObject))
	{
		SAFE_STRCPY(s_FileBuffer, "Sounds\\MajorCharacter\\");	
	}
	else if (pAI->IsMecha())
	{
		strcat(s_FileBuffer, "MCA\\");
	}
	else
	{
		strcat(s_FileBuffer, "Onfoot\\");
	}

	char* pStr = GetModelName(pAI->GetModelId());
	if (!pStr) return DFALSE;
	strcat(s_FileBuffer, pStr);

	pStr = GetFactionDir(pAI);
	if (pStr)
	{
		strcat(s_FileBuffer, pStr);
	}

	strcat(s_FileBuffer, "\\");

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheAISounds()
//
//	PURPOSE:	Cache all the sounds used by the particular AI
//
// ----------------------------------------------------------------------- //

void CacheAISounds(BaseAI* pAI)
{
	if (!g_pServerDE || !pAI) return;

	if (CanPlaySound(pAI, AIS_DEATH))
	{
		for (int i=0; i < s_nNumDeathSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_DeathSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}

	if (CanPlaySound(pAI, AIS_DAMAGE))
	{
		for (int i=0; i < s_nNumPainSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_PainSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}

	if (CanPlaySound(pAI, AIS_SPOT))
	{
		char* pFile = GetSpotSound(pAI);
		if (pFile)
		{
			g_pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

/*	if (CanPlaySound(pAI, AIS_LOSTTARGET))
	{
		for (int i=0; i < s_nNumLostTargetSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_LostTargetSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}
*/
	if (CanPlaySound(pAI, AIS_BUMPED))
	{
		for (int i=0; i < s_nNumBumpedSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_BumpedSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}

	if (CanPlaySound(pAI, AIS_SETDEFENSIVE))
	{
		for (int i=0; i < s_nNumSetDefensiveSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_SetDefensiveSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}

	if (CanPlaySound(pAI, AIS_SETAGGRESSIVE))
	{
		for (int i=0; i < s_nNumSetAggressiveSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_SetAggressiveSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}

	if (CanPlaySound(pAI, AIS_SETRETREATING))
	{
		for (int i=0; i < s_nNumSetRetreatingSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_SetRetreatingSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}

	if (CanPlaySound(pAI, AIS_SETPANICKED))
	{
		for (int i=0; i < s_nNumSetPanickedSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_SetPanickedSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}

	if (CanPlaySound(pAI, AIS_SETPSYCHO))
	{
		for (int i=0; i < s_nNumSetPsychoSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_SetPsychoSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}

	if (CanPlaySound(pAI, AIS_FOLLOWLOST))
	{
		for (int i=0; i < s_nNumFollowLostSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_FollowLostSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}

	if (CanPlaySound(pAI, AIS_PANICKED))
	{
		for (int i=0; i < s_nNumPanickedSounds; i++)
		{
			if (!SetupFileBuffer(pAI)) break;
			strcat(s_FileBuffer, s_PanickedSounds[i]);

			g_pServerDE->CacheFile(FT_SOUND, s_FileBuffer);
		}
	}
}