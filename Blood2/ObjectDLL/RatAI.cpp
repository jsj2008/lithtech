// ----------------------------------------------------------------------- //
//
// MODULE  : RatAI.cpp
//
// PURPOSE : RatAI - Definition
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "ratAI.h"
#include "cpp_server_de.h"

BEGIN_CLASS(RatAI)
	ADD_BASEAI_AGGREGATE()
    ADD_REALPROP(HitPoints, 1.0f)     \
    ADD_REALPROP(RandomHitPoints, 0.0f) \
END_CLASS_DEFAULT(RatAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		RatAI::m_bLoadAnims = DTRUE;
CAnim_Sound	RatAI::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

RatAI::RatAI() : AI_Mgr()
{
	_mbscpy((unsigned char*)m_szAIState, (const unsigned char*)"IDLE" );
  	_mbscpy((unsigned char*)m_szAIBrain, (const unsigned char*)"WEAK" );
  	_mbscpy((unsigned char*)m_szAIWeapon[0], (const unsigned char*)"MELEE" );

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_nMoveFlags = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD RatAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
		break;
        
		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
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

void RatAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return;

	char* pFilename = "Models\\Enemies\\rat.abc";
	char* pSkin = "Skins\\Enemies\\rat.dtx";
	
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

DBOOL RatAI::InitialUpdate(DVector *pMovement)
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE) return DFALSE;

	m_hObject = m_pServerDE->ObjectToHandle(this);
	if (!m_hObject) return DFALSE;

	//Load up animation indexes if first model instance
    if(m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
	    m_Anim_Sound.SetSoundRoot("sounds\\enemies\\rat");
		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: RatAI::AI_STATE_Escape_RunAway
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void RatAI::AI_STATE_Escape_RunAway()
{
	//SCHLEGZ 4/22/98 4:51:21 PM: sanity check
	if(m_hTarget == DNULL)
	{
		SetNewState(STATE_Idle);
		return;
	}

	switch(Metacmd)
	{
		case 2:		MC_FaceTarget();							break;
		case 3:		m_fRadsLeft = 3.14f;	Metacmd++;			break;
		case 4:		MC_Turn();									break;
		case 5:		MC_Run();									break;
		case 6:		ComputeState();								break;
	}

	return;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: RatAI::AI_STATE_Idle
// DESCRIPTION	: 
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void RatAI::AI_STATE_Idle()
{
	if(ComputeStimuli())
	{
		ComputeState();
		m_pServerDE->SetNextUpdate(m_hObject,0.1f);
		return;
	}
	else
		m_pServerDE->SetNextUpdate(m_hObject,0.2f);

	switch(Metacmd)
	{
		case 2:		MC_Idle();						break;
		case 3:		m_fRadsLeft = m_pServerDE->Random(-1.56f,1.56f);	
					Metacmd++;						break;
		case 4:		MC_Turn();						break;
		case 5:		MC_Walk();						break;
		case 6:		SetNewState(STATE_Idle);		break;
	}

	return;
}