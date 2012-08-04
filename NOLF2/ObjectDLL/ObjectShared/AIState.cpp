// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AI.h"
#include "AIState.h"
#include "AINodeMgr.h"
#include "AIVolumeMgr.h"
#include "AITarget.h"
#include "ParsedMsg.h"
#include "AIUtils.h"
#include "AIMovement.h"


//DEFINE_SPECIFIC_AI_FACTORY(State);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIState::CAIState()
{
	m_bNoCinematics = LTTRUE;

	m_pAI = LTNULL;
	m_fElapsedTime = 0.0f;
	m_bFirstUpdate = LTTRUE;
	m_bPlayFirstSound = LTFALSE;
	m_hstrReturn = LTNULL;
	m_hstrFirst = LTNULL;
	m_cNexts = 0;
	for ( int iNext = 0 ; iNext < kMaxNexts ; iNext++ )
	{
		m_ahstrNexts[iNext] = LTNULL;
	}

	m_eStateStatus = kSStat_Uninitialized;
}

CAIState::~CAIState()
{
///	m_pAI->GetSenseMgr()->Clear();

	// It is very strange to destruct a state without an AI.
	// Any constructed State should have Init called to set the pointer.

	AIASSERT( m_pAI, LTNULL, "CAIState::~CAIState: State has NULL m_pAI" );

	FREE_HSTRING(m_hstrReturn);
	FREE_HSTRING(m_hstrFirst);

	for ( int iNext = 0 ; iNext < kMaxNexts ; iNext++ )
	{
		FREE_HSTRING(m_ahstrNexts[iNext]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

LTBOOL CAIState::Init(CAI* pAI)
{
	m_pAI = pAI;

	if( m_pAI->HasTarget() )
	{
		m_pAI->GetTarget()->SetPushSpeed( 0.f );
	}

	m_pAI->GetAIMovement()->Clear();

	m_eStateStatus = kSStat_Initialized;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::PreUpdate
//
//	PURPOSE:	PreUpdate of the state
//
// ----------------------------------------------------------------------- //

void CAIState::PreUpdate()
{
	// Increase our elapsed state time

    m_fElapsedTime += g_pLTServer->GetFrameTime();

	// Kill any cinematic shit if we don't want it in this state

	if ( m_bNoCinematics && m_pAI->IsControlledByDialogue() )
	{
		m_pAI->StopDialogue();
		m_pAI->KillDialogueSound();
	}

	// Do our "first" command if we have one (this implicitly will only happen on the first update)

	if ( /*m_bFirstUpdate &&*/ m_hstrFirst )
	{
		SendMixedTriggerMsgToObject(m_pAI, m_pAI->m_hObject, m_hstrFirst);
		FREE_HSTRING(m_hstrFirst);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Update
//
//	PURPOSE:	Updates the state
//
// ----------------------------------------------------------------------- //

void CAIState::Update()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::UpdateMusic
//
//	PURPOSE:	Updates the music
//
// ----------------------------------------------------------------------- //

void CAIState::UpdateMusic()
{
	CMusicMgr::Mood eMood = GetMusicMood();
	if( eMood < m_pAI->GetMusicMoodMin() )
	{
		eMood = m_pAI->GetMusicMoodMin();
	}
	else if( eMood > m_pAI->GetMusicMoodMax() )
	{
		eMood = m_pAI->GetMusicMoodMax();
	}

	g_pMusicMgr->DoMood( eMood );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::PostUpdate
//
//	PURPOSE:	PostUpdate of the state
//
// ----------------------------------------------------------------------- //

void CAIState::PostUpdate()
{
	m_bFirstUpdate = LTFALSE;
	m_bPlayFirstSound = LTFALSE;
}

// ----------------------------------------------------------------------- //

bool CAIState::HandleCommand(const CParsedMsg &cMsg)
{
	// If the command starts with the type name if this state,
	// handle it.
	if( cMsg.GetArg(0) == s_aszStateTypes[GetStateType()] )
	{
		// First token specifies the state.  Skip it.

		char szTempBuffer[256];
		char* szName;
		char* szValue;
		for(uint8 iToken=1; iToken < cMsg.GetArgCount(); ++iToken)
		{
			SAFE_STRCPY(szTempBuffer, cMsg.GetArg(iToken));
			szName  = strtok(szTempBuffer, "=");
			szValue = strtok(LTNULL, "");

			// Check for legal NAME=VALUE pair.
			AIASSERT(szName && szValue, LTNULL, "CAIState::HandleCommand: Name or Value is NULL.");
			if ( !(szName && szValue) )
			{
		        g_pLTServer->CPrint("Garbage name/value pair = %s", cMsg.GetArg(iToken).c_str());
				continue;
			}

			HandleNameValuePair(szName, szValue);
		}	

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Load
//
//	PURPOSE:	Restores the state
//
// ----------------------------------------------------------------------- //

void CAIState::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST(m_eStateStatus, EnumAIStateStatus);
	LOAD_BOOL(m_bNoCinematics);
	LOAD_FLOAT(m_fElapsedTime);
	LOAD_BOOL(m_bFirstUpdate);
	LOAD_BOOL(m_bPlayFirstSound);
	LOAD_HSTRING(m_hstrReturn);
	LOAD_HSTRING(m_hstrFirst);
	LOAD_INT(m_cNexts);
	for ( int iNext = 0 ; iNext < kMaxNexts ; iNext++ )
	{
		LOAD_HSTRING(m_ahstrNexts[iNext]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Save
//
//	PURPOSE:	Saves the state
//
// ----------------------------------------------------------------------- //

void CAIState::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(m_eStateStatus);
	SAVE_BOOL(m_bNoCinematics);
	SAVE_FLOAT(m_fElapsedTime);
	SAVE_BOOL(m_bFirstUpdate);
	SAVE_BOOL(m_bPlayFirstSound);
	SAVE_HSTRING(m_hstrReturn);
	SAVE_HSTRING(m_hstrFirst);
	SAVE_INT(m_cNexts);
	for ( int iNext = 0 ; iNext < kMaxNexts ; iNext++ )
	{
		SAVE_HSTRING(m_ahstrNexts[iNext]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::HandleNameValuePair
//
//	PURPOSE:	Sets data from name/value pairs
//
// ----------------------------------------------------------------------- //

void CAIState::HandleNameValuePair(char *szName, char *szValue)
{
	_ASSERT(szName && szValue);
	if ( !szName || !szValue ) return;

	if ( !_stricmp(szName, "RETURN") )
	{
		FREE_HSTRING(m_hstrReturn);
        m_hstrReturn = g_pLTServer->CreateString(szValue);
	}
	else if ( !_stricmp(szName, "NEXT") )
	{
		if ( m_cNexts >= kMaxNexts )
		{
			_ASSERT(!"NEXT= buffer overflow");
			return;
		}

        m_ahstrNexts[m_cNexts++] = g_pLTServer->CreateString(szValue);
	}
	else if ( !_stricmp(szName, "FIRST") )
	{
        FREE_HSTRING(m_hstrFirst);
        m_hstrFirst = g_pLTServer->CreateString(szValue);
	}
	else if ( !_stricmp(szName, "FIRSTSOUND") )
	{
		m_bPlayFirstSound = IsTrueChar(*szValue);
	}
	else if ( !_stricmp(szName, "NOCINEMATICS") )
	{
		m_bNoCinematics = IsTrueChar(*szValue);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::NextOr
//
//	PURPOSE:	Send us our Next string, or uses the default
//
// ----------------------------------------------------------------------- //

void CAIState::NextOr(const char *szState)
{
	if ( m_cNexts > 0 )
	{
		char szBuffer[4096] = "";

		for ( int iNext = 0 ; iNext < m_cNexts ; iNext++ )
		{
            strcat(szBuffer, g_pLTServer->GetStringData(m_ahstrNexts[iNext]));
			strcat(szBuffer, ";");

			FREE_HSTRING(m_ahstrNexts[iNext]);
		}

		m_cNexts = 0;

		szBuffer[strlen(szBuffer)-1] = 0;

		m_pAI->ChangeState(szBuffer);
	}
	else
	{
		m_pAI->ChangeState(szState);
	}
}

HMODELANIM CAIState::GetDeathAni(LTBOOL bFront)
{
	return INVALID_ANI;
}
