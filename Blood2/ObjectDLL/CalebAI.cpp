// ----------------------------------------------------------------------- //
//
// MODULE  : CalebAI.cpp
//
// PURPOSE : CalebAI - Definition
//
// CREATED : 10/4/98
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "CalebAI.h"
#include "cpp_server_de.h"

BEGIN_CLASS(CalebAI)
	ADD_BASEAI_AGGREGATE()
    ADD_REALPROP(HitPoints, 30.0f)
    ADD_REALPROP(RandomHitPoints, 0.0f)
    ADD_REALPROP(ArmorPoints, 0.0f)
	ADD_STRINGPROP(AIState, "IDLE")
	ADD_STRINGPROP(AIWeapon, "NONE")
END_CLASS_DEFAULT(CalebAI, AI_Mgr, NULL, NULL)

//static data member initialization
DBOOL		CalebAI::m_bLoadAnims = DTRUE;
CAnim_Sound	CalebAI::m_Anim_Sound;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CalebAI::CalebAI()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CalebAI::CalebAI() : AI_Mgr()
{
	m_fHearingDist	= 0.0f;
	m_fSensingDist	= 200.0f;
	m_fSmellingDist	= 0.0f;
	m_fSeeingDist	= 1000.0f;

	m_fWalkSpeed	= 75.0f;
	m_fRunSpeed		= 180.0f;
	m_fRollSpeed	= 3.5f;

	m_fAIMass		= AI_DEFAULT_MASS;

	m_nAIStrength	= 5;

	m_nState = STATE_Idle; 
	m_nLastState = STATE_Idle; 
	m_dwFlags = 0;

	m_bCabal = DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CalebAI::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CalebAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we set the skin/filename...

			DDWORD dwRet = AI_Mgr::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)"Models\\Characters\\caleb.abc");
				_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)"Skins\\Characters\\caleb1.dtx");
			}

			return dwRet;
		}
		break;
        
		case MID_INITIALUPDATE:
		{
			FirstUpdate();
		}
		break;

		default : break;
	}


	return AI_Mgr::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FirstUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL CalebAI::FirstUpdate()
{
	m_pServerDE = BaseClass::GetServerDE();
	if (!m_pServerDE || !m_hObject) return DFALSE;

	//Load up animation indexes if first model instance

    if (m_bLoadAnims)
	{
	    m_Anim_Sound.SetAnimationIndexes(m_hObject);
		m_Anim_Sound.GenerateHitSpheres(m_hObject);
		m_Anim_Sound.SetSoundRoot("sounds\\chosen\\caleb");

		m_bLoadAnims = DFALSE;
	}

	AI_Mgr::InitStatics(&m_Anim_Sound);
	
	return DTRUE;
}

