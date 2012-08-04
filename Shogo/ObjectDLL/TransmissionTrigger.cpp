// ----------------------------------------------------------------------- //
//
// MODULE  : TransmissionTrigger.cpp
//
// PURPOSE : TransmissionTrigger implementation
//
// CREATED : 3/21/98
//
// ----------------------------------------------------------------------- //

#include "TransmissionTrigger.h"
#include "PlayerObj.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "RiotObjectUtilities.h"
#include "ClientServerShared.h"
#include "RiotServerShell.h"

extern CRiotServerShell* g_pRiotServerShellDE;


BEGIN_CLASS(TransmissionTrigger)
	ADD_ACTIVATION_AGGREGATE()
	ADD_VECTORPROP_VAL_FLAG(Dims, 5.0f, 5.0f, 5.0f, PF_DIMS) 
	ADD_BOOLPROP(PlayerTriggerable, 0)
	ADD_BOOLPROP(AITriggerable, 0)
	ADD_STRINGPROP(AITriggerName, "")
	ADD_STRINGPROP(ImageFilename, "")
	ADD_REALPROP(StringID, 0.0f)
	ADD_REALPROP(SendDelay, 0.0f)
END_CLASS_DEFAULT(TransmissionTrigger, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TransmissionTrigger::TransmissionTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

TransmissionTrigger::TransmissionTrigger() : BaseClass ()
{
	AddAggregate(&m_activation);
	
	VEC_SET( m_vDims, 5.0f, 5.0f, 5.0f );
	
	m_bPlayerTriggerable = DFALSE;

	m_bAITriggerable = DFALSE;
	m_hstrAIName = DNULL;
		
	m_bTriggered = DFALSE;

	m_hImageFilename = DNULL;
	m_fStringID = 0.0f;
	m_fSendDelay = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TransmissionTrigger::~TransmissionTrigger()
//
//	PURPOSE:	Free object
//
// ----------------------------------------------------------------------- //

TransmissionTrigger::~TransmissionTrigger()
{
	CServerDE* pServerDE = GetServerDE();

	if (m_hstrAIName) pServerDE->FreeString (m_hstrAIName);
	if (m_hImageFilename) pServerDE->FreeString (m_hImageFilename);

	if (!pServerDE) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TransmissionTrigger::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD TransmissionTrigger::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CPlayerObj *pPlayerObj;

	switch (messageID)
	{
		case MID_TRIGGER:
		{
			if (!m_bTriggered) Trigger();
		}
		break;

		case MID_PLAYDIALOG:
		{
			DialogQueueTransmission *pDialogQueueTransmission;

			pDialogQueueTransmission = ( DialogQueueTransmission * )g_pServerDE->ReadFromMessageDWord( hRead );
			pPlayerObj = g_pRiotServerShellDE->GetFirstPlayer( );
			if( pDialogQueueTransmission && pPlayerObj )
			{
				pPlayerObj->SetDialogActive( DTRUE );

				HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, MID_COMMAND_TRANSMISSION);
				g_pServerDE->WriteToMessageDWord(hMessage, pDialogQueueTransmission->m_nStringID );
				g_pServerDE->WriteToMessageString (hMessage, pDialogQueueTransmission->m_szDialogFile);
				g_pServerDE->EndMessage (hMessage);
			}

			g_pServerDE->RemoveObject(m_hObject);
		}
		break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TransmissionTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engineEngineMessageFn messages
//
// ----------------------------------------------------------------------- //

DDWORD TransmissionTrigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch (messageID)
	{
		case MID_INITIALUPDATE:
		{
			if (fData == INITIALUPDATE_SAVEGAME) break;

			pServerDE->SetObjectDims(m_hObject, &m_vDims);
			pServerDE->SetObjectFlags(m_hObject, FLAG_TOUCH_NOTIFY);
			pServerDE->SetObjectUserFlags(m_hObject, USRFLG_IGNORE_PROJECTILES);

			pServerDE->SetNextUpdate(m_hObject, 0.0f);
		}
		break;

		case MID_UPDATE:
		{
			if( !m_bTriggered )
				Trigger();
		}
		break;
		
		case MID_TOUCHNOTIFY:
		{
			if (!m_bTriggered) ObjectTouch((HOBJECT)pData);
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (!pStruct) return 0;

			if (fData == 1.0f)
			{
				ReadProp(pStruct);
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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TransmissionTrigger::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

DBOOL TransmissionTrigger::ReadProp(ObjectCreateStruct *pData)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	char buf[MAX_CS_FILENAME_LEN];

	pServerDE->GetPropVector("Dims", &m_vDims);

	pServerDE->GetPropBool("PlayerTriggerable", &m_bPlayerTriggerable);

	pServerDE->GetPropBool("AITriggerable", &m_bAITriggerable);
	
	buf[0] = '\0';
	pServerDE->GetPropString("AITriggerName", buf, MAX_CS_FILENAME_LEN);
	if (buf[0] && strlen(buf)) m_hstrAIName = pServerDE->CreateString(buf);
	
	buf[0] = '\0';
	pServerDE->GetPropString("ImageFilename", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hImageFilename = pServerDE->CreateString(buf);
	
	pServerDE->GetPropReal("StringID", &m_fStringID);
	
	pServerDE->GetPropReal("SendDelay", &m_fSendDelay);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TransmissionTrigger::ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void TransmissionTrigger::ObjectTouch (HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// ignore everything but characters derived from BaseCharacter

	if (!IsBaseCharacter (hObj)) return;

	HCLASS hClassObj = pServerDE->GetObjectClass(hObj);
	
	// If we're AI, make sure we can activate this trigger...

	if (!m_bAITriggerable)
	{
		HCLASS hClassAI = pServerDE->GetClass("BaseAI");
		if ( pServerDE->IsKindOf(hClassObj, hClassAI) )
		{
			return;
		}
	}
	else if (m_hstrAIName) // See if only a specific AI can trigger it...
	{
		char* pAIName  = pServerDE->GetStringData(m_hstrAIName);
		char* pObjName = pServerDE->GetObjectName(hObj);

		if (pAIName && pObjName)
		{
			if ( stricmp(pAIName, pObjName) != 0)
			{
				return;
			}
		}
	}
	
	// If we're the player, make sure we can activate this trigger...

	if (!m_bPlayerTriggerable)
	{
		HCLASS hClassPlayer = pServerDE->GetClass("CPlayerObj");

		if ( pServerDE->IsKindOf(hClassObj, hClassPlayer) )
		{
			return;
		}
	}
	
	// if there's a send delay, start the timer
	// otherwise, send the dialog message now

	if (m_fSendDelay)
	{
		pServerDE->SetNextUpdate (m_hObject, m_fSendDelay);
	}
	else
	{
		Trigger();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TransmissionTrigger::Trigger
//
//	PURPOSE:	Sends a transmission message to the client of the player
//				that triggered us
//
// ----------------------------------------------------------------------- //

void TransmissionTrigger::Trigger()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	CPlayerObj *pPlayerObj;
	char *pString;
	
	m_bTriggered = DTRUE;

	pPlayerObj = g_pRiotServerShellDE->GetFirstPlayer( );
	pString = g_pServerDE->GetStringData( m_hImageFilename );

	if( pPlayerObj && pString )
	{
		DialogQueueTransmission *pDialogQueueTransmission;
		DialogQueueElement *pDialogQueueElement;

		pDialogQueueElement = new DialogQueueElement;
		pDialogQueueElement->m_hObject = m_hObject;
		pDialogQueueTransmission= new DialogQueueTransmission;
		pDialogQueueElement->m_pData = pDialogQueueTransmission;
		SAFE_STRCPY( pDialogQueueTransmission->m_szDialogFile, pString );
		pDialogQueueTransmission->m_nStringID = ( DDWORD )m_fStringID;

		dl_AddTail( pPlayerObj->GetDialogQueue( ), &pDialogQueueElement->m_Link, pDialogQueueElement );
		g_pServerDE->CreateInterObjectLink( pPlayerObj->m_hObject, m_hObject );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TransmissionTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TransmissionTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageVector(hWrite, &m_vDims);	
	pServerDE->WriteToMessageByte(hWrite, m_bPlayerTriggerable);	
	pServerDE->WriteToMessageByte(hWrite, m_bAITriggerable);	
	pServerDE->WriteToMessageByte(hWrite, m_bTriggered);	
	pServerDE->WriteToMessageHString(hWrite, m_hstrAIName);	
	pServerDE->WriteToMessageHString(hWrite, m_hImageFilename);	
	pServerDE->WriteToMessageFloat(hWrite, m_fStringID);	
	pServerDE->WriteToMessageFloat(hWrite, m_fSendDelay);	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TransmissionTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TransmissionTrigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromMessageVector(hRead, &m_vDims);	
	m_bPlayerTriggerable = (DBOOL) pServerDE->ReadFromMessageByte(hRead);	
	m_bAITriggerable	 = (DBOOL) pServerDE->ReadFromMessageByte(hRead);	
	m_bTriggered		 = (DBOOL) pServerDE->ReadFromMessageByte(hRead);	
	m_hstrAIName		 = pServerDE->ReadFromMessageHString(hRead);	
	m_hImageFilename	 = pServerDE->ReadFromMessageHString(hRead);	
	m_fStringID			 = pServerDE->ReadFromMessageFloat(hRead);	
	m_fSendDelay		 = pServerDE->ReadFromMessageFloat(hRead);	
}
