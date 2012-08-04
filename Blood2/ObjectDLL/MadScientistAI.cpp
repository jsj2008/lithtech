// ----------------------------------------------------------------------- //
//
// MODULE  : MadScientistAI.cpp
//
// PURPOSE : MadScientistAI - Definition
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //


#include <stdio.h>
#include "MadScientistAI.h"
#include "cpp_server_de.h"


BEGIN_CLASS(MadScientistAI)
    ADD_REALPROP(HitPoints, 30.0f)     \
    ADD_REALPROP(RandomHitPoints, 0.0f) \
    ADD_REALPROP(ArmorPoints, 0.0f)     \
	ADD_STRINGPROP(AIState, "IDLE")     \
	ADD_STRINGPROP(AIWeapon, "NONE")   \
END_CLASS_DEFAULT(MadScientistAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		MadScientistAI::m_bLoadAnims = DTRUE;
CAnim_Sound	MadScientistAI::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

MadScientistAI::MadScientistAI() : AI_Mgr()
{
	m_fHearingDist	= 0.0f;
	m_fSensingDist	= 200.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 1000.0f;

	m_fWalkSpeed	= 75.0f;
	m_fRunSpeed		= 180.0f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = 0;

	m_bCabal = DTRUE;

	// [blg]
	m_fAIHitPoints   = 30;
	m_fAIRandomHP    = 0;
	m_fAIArmorPoints = 0;

	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD MadScientistAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			if(fData != PRECREATE_SAVEGAME)
				PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
		break;
        
		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			CacheFiles();
			break;
		}

		default : break;
	}


	return AI_Mgr::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void MadScientistAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = "Models\\Enemies\\madscientist.abc";
	char* pSkin = "Skins\\Enemies\\madscientist.dtx";
	_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pFilename);
	_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pSkin);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL MadScientistAI::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	//Load up animation indexes if first model instance
    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\madscientist");
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: MadScientistAI::ComputeState
// DESCRIPTION	: Compute actual substate
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void MadScientistAI::ComputeState(int nStimType)
{
	int nStim = nStimType;

	if(nStimType == -1)
		nStim = ComputeStimuli();

	if(nStim == 0)
	{
		SetNewState(StateStrToInt(m_szAIState));
	}
	else
	{
		SetNewState(STATE_Escape_Hide);
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: MadScientistAI::AI_STATE_Escape_Hide
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void MadScientistAI::AI_STATE_Escape_Hide()
{
	if(m_hTarget == DNULL)
	{
		ComputeState();
		return;
	}

	MC_FaceTarget();
	Metacmd--;

	switch(Metacmd)
	{
		case 1:		MC_Taunt_Beg();		break;
		case 2:		ComputeState();		break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: MadScientistAI::AI_STATE_Idle
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void MadScientistAI::AI_STATE_Idle()
{
	int nStimType = ComputeStimuli();

	if(nStimType > 0)
	{
		m_pServerDE->SetNextUpdate(m_hObject, 0.001f);
		ComputeState(nStimType);
		return;
	}

	m_pServerDE->SetNextUpdate(m_hObject, 0.1f);

	switch(Metacmd)
	{
		case 1:		MC_Idle();			break;
		case 2:		ComputeState();		break;
	}

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void MadScientistAI::CacheFiles()
{
	// Sanity checks...

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if(!(pServerDE->GetServerFlags() & SS_CACHING))
	{
		return;
	}

	if (!m_hObject) return;


	// Get the model filenames...

	char sModel[256] = { "" };
	char sSkin[256]  = { "" };

	pServerDE->GetModelFilenames(m_hObject, sModel, 255, sSkin, 255);


	// Cache models...

	pServerDE->CacheFile(FT_MODEL, sModel);


	// Cache textures...

	pServerDE->CacheFile(FT_TEXTURE, sSkin);
}

