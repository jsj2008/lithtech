// ----------------------------------------------------------------------- //
//
// MODULE  : ExitTrigger.h
//
// PURPOSE : ExitTrigger - Implementation
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#include "ExitTrigger.h"
#include "cpp_server_de.h"
#include "RiotMsgIds.h"
#include "RiotServerShell.h"
#include "PlayerObj.h"
#include "RiotObjectUtilities.h"

extern CRiotServerShell* g_pRiotServerShellDE;

BEGIN_CLASS(ExitTrigger)
	ADD_STRINGPROP(DestinationWorld, "")
	ADD_STRINGPROP(StartPointName, "start")
	ADD_STRINGPROP(BumperScreen, "bumpy")
	ADD_REALPROP(BumperTextID, 0.0f)
	ADD_BOOLPROP(EndOfScenario, 0)
	
	ADD_REALPROP_FLAG(TriggerDelay, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ActivationSound, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(SoundRadius, 200.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName1, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName1, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TargetName2, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageName2, "", PF_HIDDEN)
	ADD_BOOLPROP_FLAG(TriggerTouch, 0 , PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageTouch, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(AITriggerName, "", PF_HIDDEN)
	ADD_BOOLPROP_FLAG(PlayerTriggerable, 1, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(AITriggerable, 0, PF_HIDDEN)
	ADD_REALPROP_FLAG(AccessGrantedMsg, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(AccessGrantedSound, "", PF_HIDDEN)
	ADD_BOOLPROP_FLAG(WeightedTrigger, DFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Message1Weight, .5, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(TimedTrigger, DFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(MinTriggerTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxTriggerTime, 10.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(ActivationCount, 1, PF_HIDDEN)
END_CLASS_DEFAULT(ExitTrigger, Trigger, NULL, NULL)

#define MAXWAITTIME		10.0f


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::ExitTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ExitTrigger::ExitTrigger()
{
	m_hstrDestinationWorld	= DNULL;
	m_hstrStartPointName	= DNULL;
	m_hstrBumperScreen		= DNULL;
	m_nBumperTextID			= 0.0f;
	m_bEndOfScenario		= DFALSE;

	m_bWaitingForDialogue	= DFALSE;
	m_fMaxWaitTime = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::~ExitTrigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

ExitTrigger::~ExitTrigger()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrDestinationWorld)
	{
		pServerDE->FreeString(m_hstrDestinationWorld);
	}
	if (m_hstrStartPointName)
	{
		pServerDE->FreeString(m_hstrStartPointName);
	}
	if (m_hstrBumperScreen)
	{
		pServerDE->FreeString(m_hstrBumperScreen);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ExitTrigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
				
			DDWORD dwRet = Trigger::EngineMessageFn(messageID, pData, fData);
			
			PostPropRead((ObjectCreateStruct*)pData);
			
			return dwRet;
		}

		case MID_UPDATE:
		{
			if( m_bWaitingForDialogue )
			{
				Update( );
				return LT_OK;
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}


	return Trigger::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL ExitTrigger::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	char buf[MAX_CS_FILENAME_LEN];

	buf[0] = '\0';
	pServerDE->GetPropString("DestinationWorld", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrDestinationWorld = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("StartPointName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrStartPointName = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("BumperScreen", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrBumperScreen = pServerDE->CreateString(buf);

	pServerDE->GetPropReal("BumperTextID", &m_nBumperTextID);

	pServerDE->GetPropBool("EndOfScenario", &m_bEndOfScenario);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::PostPropRead
//
//	PURPOSE:	Handle extra initialization
//
// ----------------------------------------------------------------------- //

void ExitTrigger::PostPropRead(ObjectCreateStruct *pStruct)
{
	m_bPlayerTriggerable = DTRUE;
	m_bAITriggerable	 = DFALSE;
	m_hstrAIName		 = DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void ExitTrigger::ObjectTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hObj || !IsPlayer(hObj)) return;

	CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(hObj);
	if (!pPlayer || pPlayer->IsDead()) return;

	Trigger::ObjectTouch(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Activate
//
//	PURPOSE:	Activate the exit trigger.
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Activate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !g_pRiotServerShellDE) return;

	if( !CanExit( ))
	{
		m_bWaitingForDialogue = DTRUE;
		m_fMaxWaitTime = g_pServerDE->GetTime( ) + MAXWAITTIME;

		pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
		pServerDE->SetDeactivationTime(m_hObject, 0.0f);
		
		return;
	}

	DoExit( );	

	Trigger::Activate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::CanExit
//
//	PURPOSE:	Checks if there is any dialogue that would prevent an
//				immediate exit.
//
// ----------------------------------------------------------------------- //
DBOOL ExitTrigger::CanExit( )
{
	CPlayerObj *pPlayerObj;

	pPlayerObj = g_pRiotServerShellDE->GetFirstPlayer( );
	if( pPlayerObj )
	{
		// Check if there is dialogue currently or if there is any queued.
		if( pPlayerObj->IsDialogActive( ) || pPlayerObj->GetDialogQueue( )->m_nElements )
		{
			return DFALSE;
		}
	}
	
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::DoExit
//
//	PURPOSE:	Does exit process
//
// ----------------------------------------------------------------------- //

void ExitTrigger::DoExit()
{
	// Tell the server shell what the name of the next start point
	// will be...
	if (m_hstrStartPointName)
	{
		g_pRiotServerShellDE->SetStartPointName(m_hstrStartPointName);
	}

	// If the game is single player or Co-op we want to tell all the 
	// clients to exit the world...If it is death match, we'll just
	// ignore the request...
	GameType eGameType = g_pRiotServerShellDE->GetGameType();
	if (eGameType == SINGLE || eGameType == COOPERATIVE)
	{
		SendClientsExitMsg();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Update
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //
void ExitTrigger::Update()
{
	if( !CanExit( ) && g_pServerDE->GetTime( ) < m_fMaxWaitTime )
	{
		g_pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
		g_pServerDE->SetDeactivationTime(m_hObject, 0.0f);
		return;
	}

	m_bWaitingForDialogue = DFALSE;

	DoExit( );	

	Trigger::Activate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::SendClientsExitMsg()
//
//	PURPOSE:	Tell the clients we wanna exit
//
// ----------------------------------------------------------------------- //

void ExitTrigger::SendClientsExitMsg()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMsg = pServerDE->StartMessage(DNULL, MID_PLAYER_EXITLEVEL);
	pServerDE->WriteToMessageHString(hMsg, m_hstrDestinationWorld);
	pServerDE->WriteToMessageHString(hMsg, m_hstrBumperScreen);
	pServerDE->WriteToMessageDWord(hMsg, (DDWORD)m_nBumperTextID);
	pServerDE->WriteToMessageByte(hMsg, m_bEndOfScenario);
	pServerDE->EndMessage(hMsg);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	DBYTE nFlags;
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageHString(hWrite, m_hstrDestinationWorld);
	pServerDE->WriteToMessageHString(hWrite, m_hstrStartPointName);
	pServerDE->WriteToMessageHString(hWrite, m_hstrBumperScreen);
	pServerDE->WriteToMessageFloat(hWrite, m_nBumperTextID);
	nFlags = 0;
	if( m_bEndOfScenario )
		nFlags |= 0x01;
	if( m_bWaitingForDialogue )
		nFlags |= 0x02;
	pServerDE->WriteToMessageByte( hWrite, nFlags );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	DBYTE nFlags;
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hstrDestinationWorld	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrStartPointName	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrBumperScreen		= pServerDE->ReadFromMessageHString(hRead);
	m_nBumperTextID			= pServerDE->ReadFromMessageFloat(hRead);
	nFlags					= pServerDE->ReadFromMessageByte(hRead);
	m_bEndOfScenario		= nFlags & 0x01;
	m_bWaitingForDialogue	= nFlags & 0x02;

	// If we were waiting for dialog to finish, then just exit now.
	if( m_bWaitingForDialogue )
	{
		m_bWaitingForDialogue = DFALSE;

		DoExit( );	

		Trigger::Activate();
	}
}