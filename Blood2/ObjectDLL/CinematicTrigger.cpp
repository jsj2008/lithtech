// ----------------------------------------------------------------------- //
//
// MODULE  : CinematicTrigger.cpp
//
// PURPOSE : CinematicTrigger - Implementation
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include <string.h>
#include "CinematicTrigger.h"
#include "generic_msg_de.h"
#include "ObjectUtilities.h"
#include "ClientServerShared.h"
#include <mbstring.h>
#include "SoundTypes.h"

void SendTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg);

BEGIN_CLASS(CinematicTrigger)
	ADD_REALPROP(Delay1, 0.0f)
	ADD_STRINGPROP(Sound1, "")
	ADD_STRINGPROP(WhoPlaysSound1, "")
	ADD_STRINGPROP(StartSoundTriggerTarget1, "")
	ADD_STRINGPROP(StartSoundTriggerMsg1, "")
	ADD_REALPROP(Delay2, 0.0f)
	ADD_STRINGPROP(Sound2, "")
	ADD_STRINGPROP(WhoPlaysSound2, "")
	ADD_STRINGPROP(StartSoundTriggerTarget2, "")
	ADD_STRINGPROP(StartSoundTriggerMsg2, "")
	ADD_REALPROP(Delay3, 0.0f)
	ADD_STRINGPROP(Sound3, "")
	ADD_STRINGPROP(WhoPlaysSound3, "")
	ADD_STRINGPROP(StartSoundTriggerTarget3, "")
	ADD_STRINGPROP(StartSoundTriggerMsg3, "")
	ADD_REALPROP(Delay4, 0.0f)
	ADD_STRINGPROP(Sound4, "")
	ADD_STRINGPROP(WhoPlaysSound4, "")
	ADD_STRINGPROP(StartSoundTriggerTarget4, "")
	ADD_STRINGPROP(StartSoundTriggerMsg4, "")
	ADD_REALPROP(Delay5, 0.0f)
	ADD_STRINGPROP(Sound5, "")
	ADD_STRINGPROP(WhoPlaysSound5, "")
	ADD_STRINGPROP(StartSoundTriggerTarget5, "")
	ADD_STRINGPROP(StartSoundTriggerMsg5, "")
	ADD_REALPROP(Delay6, 0.0f)
	ADD_STRINGPROP(Sound6, "")
	ADD_STRINGPROP(WhoPlaysSound6, "")
	ADD_STRINGPROP(StartSoundTriggerTarget6, "")
	ADD_STRINGPROP(StartSoundTriggerMsg6, "")
	ADD_REALPROP(Delay7, 0.0f)
	ADD_STRINGPROP(Sound7, "")
	ADD_STRINGPROP(WhoPlaysSound7, "")
	ADD_STRINGPROP(StartSoundTriggerTarget7, "")
	ADD_STRINGPROP(StartSoundTriggerMsg7, "")
	ADD_REALPROP(Delay8, 0.0f)
	ADD_STRINGPROP(Sound8, "")
	ADD_STRINGPROP(WhoPlaysSound8, "")
	ADD_STRINGPROP(StartSoundTriggerTarget8, "")
	ADD_STRINGPROP(StartSoundTriggerMsg8, "")
	ADD_REALPROP(Delay9, 0.0f)
	ADD_STRINGPROP(Sound9, "")
	ADD_STRINGPROP(WhoPlaysSound9, "")
	ADD_STRINGPROP(StartSoundTriggerTarget9, "")
	ADD_STRINGPROP(StartSoundTriggerMsg9, "")
	ADD_REALPROP(Delay10, 0.0f)
	ADD_STRINGPROP(Sound10, "")
	ADD_STRINGPROP(WhoPlaysSound10, "")
	ADD_STRINGPROP(StartSoundTriggerTarget10, "")
	ADD_STRINGPROP(StartSoundTriggerMsg10, "")
	ADD_BOOLPROP_FLAG(TypeActionMode, DFALSE, PF_GROUP6)
END_CLASS_DEFAULT(CinematicTrigger, B2BaseClass, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::CinematicTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CinematicTrigger::CinematicTrigger()
{
	for (int i=0;i < MAX_CT_MESSAGES; i++)
	{
		m_fDelay[i]				= 0.0f;
		m_hstrSound[i]			= DNULL;
		m_hstrWhoPlaysSound[i]	= DNULL;
		m_hstrTargetName[i]		= DNULL;
		m_hstrMessageName[i]	= DNULL;
	}

	m_nCurMessage	= 0;
	m_hCurSound		= DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::~CinematicTrigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

CinematicTrigger::~CinematicTrigger()
{
	if (!g_pServerDE) return;

	if (m_hCurSound)
	{
		g_pServerDE->KillSound(m_hCurSound);
		m_hCurSound = DNULL;
	}

	for (int i=0; i < MAX_CT_MESSAGES; i++)
	{
		if (m_hstrSound[i])
		{
			g_pServerDE->FreeString(m_hstrSound[i]);
		}

		if (m_hstrWhoPlaysSound[i])
		{
			g_pServerDE->FreeString(m_hstrWhoPlaysSound[i]);
		}

		if (m_hstrTargetName[i])
		{
			g_pServerDE->FreeString(m_hstrTargetName[i]);
		}

		if (m_hstrMessageName[i])
		{
			g_pServerDE->FreeString(m_hstrMessageName[i]);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL CinematicTrigger::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!g_pServerDE) return DFALSE;

	GenericProp genProp;

	for (int i=0; i < MAX_CT_MESSAGES; i++)
	{
		char key[40];

		sprintf(key, "Delay%d", i+1);
		if (g_pServerDE->GetPropGeneric(key, &genProp) == DE_OK)
		{
			m_fDelay[i] = genProp.m_Float;
		}

		sprintf(key, "Sound%d", i+1);
		if (g_pServerDE->GetPropGeneric(key, &genProp) == DE_OK)
		{
			if (genProp.m_String[0])
				 m_hstrSound[i] = g_pServerDE->CreateString(genProp.m_String);
		}

		sprintf(key, "WhoPlaysSound%d", i+1);
		if (g_pServerDE->GetPropGeneric(key, &genProp) == DE_OK)
		{
			if (genProp.m_String[0])
				 m_hstrWhoPlaysSound[i] = g_pServerDE->CreateString(genProp.m_String);
		}

		sprintf(key, "StartSoundTriggerTarget%d", i+1);
		if (g_pServerDE->GetPropGeneric(key, &genProp) == DE_OK)
		{
			if (genProp.m_String[0])
				 m_hstrTargetName[i] = g_pServerDE->CreateString(genProp.m_String);
		}

		sprintf(key, "StartSoundTriggerMsg%d", i+1);
		if (g_pServerDE->GetPropGeneric(key, &genProp) == DE_OK)
		{
			if (genProp.m_String[0])
				 m_hstrMessageName[i] = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

DBOOL CinematicTrigger::Update()
{
	if (!g_pServerDE) return DFALSE;

	// See if we are playing a sound...

	if (m_hCurSound)
	{
		// If sound is done, stop it and set next update to wait delay...

		DBOOL bDone = DFALSE;
		DRESULT dRes = g_pServerDE->IsSoundDone(m_hCurSound, &bDone);
		if (dRes != DE_OK || bDone)
		{
			g_pServerDE->KillSound(m_hCurSound);
			m_hCurSound = DNULL;

			m_nCurMessage++;
			if (m_nCurMessage < MAX_CT_MESSAGES)
			{
				g_pServerDE->SetNextUpdate(m_hObject, m_fDelay[m_nCurMessage] + 0.0001f);
				return DTRUE;
			}
			else
			{
				return DFALSE;
			}
		}
	}
	else 
	{
		// See if we're done...

		if (m_nCurMessage >= MAX_CT_MESSAGES || !m_hstrSound[m_nCurMessage])
		{
			return DFALSE;
		}

		// Start next sound...

		StartSound();
	}

	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CinematicTrigger::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update())
			{
				g_pServerDE->RemoveObject(m_hObject);
				return DFALSE;
			}
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				// Mark this object as savable
				DDWORD dwFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
				dwFlags |= USRFLG_SAVEABLE;
				g_pServerDE->SetObjectUserFlags(m_hObject, dwFlags);

				// Wait for a trigger message...
				g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
			}
			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD CinematicTrigger::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER :
		{
			// Start updating the object...

			g_pServerDE->SetNextUpdate(m_hObject, m_fDelay[0] + 0.0001f);
		}
		break;

		default : break;
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::SendMessages
//
//	PURPOSE:	Sends the CinematicTrigger messages
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::SendMessage()
{
	if (m_nCurMessage < MAX_CT_MESSAGES)
	{
		if (m_hstrTargetName[m_nCurMessage] && m_hstrMessageName[m_nCurMessage])
		{
			SendTriggerMsgToObjects(this, m_hstrTargetName[m_nCurMessage], m_hstrMessageName[m_nCurMessage]);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::StartSound
//
//	PURPOSE:	Start the appropriate sound
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::StartSound()
{
	char *pName;
	if (!g_pServerDE || m_nCurMessage >= MAX_CT_MESSAGES) return;

	if (m_hCurSound)
	{
		g_pServerDE->KillSound(m_hCurSound);
		m_hCurSound = DNULL;
	}

	SendMessage();

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT(playSoundInfo);

	playSoundInfo.m_dwFlags = PLAYSOUND_TIME | PLAYSOUND_GETHANDLE | PLAYSOUND_REVERB;

	pName = g_pServerDE->GetStringData(m_hstrWhoPlaysSound[m_nCurMessage]);
	if( !pName )
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_3D | PLAYSOUND_ATTACHED;
		playSoundInfo.m_hObject	= m_hObject;
	}
	else if( stricmp( pName, "player" ) == 0 )
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOCAL;
	else
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_3D | PLAYSOUND_ATTACHED;
		playSoundInfo.m_hObject = PlayedBy( pName );
	}


	char* pSound = g_pServerDE->GetStringData(m_hstrSound[m_nCurMessage]);
	if (!pSound) return;

	DFLOAT fRadius = 1000.0f;
	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pSound, _MAX_PATH);
	playSoundInfo.m_nPriority	 = SOUNDPRIORITY_PLAYER_HIGH;
	playSoundInfo.m_fOuterRadius = fRadius;
	playSoundInfo.m_fInnerRadius = fRadius * 0.25f;
	playSoundInfo.m_nVolume		 = 100;
	
	g_pServerDE->PlaySound(&playSoundInfo);

	m_hCurSound = playSoundInfo.m_hSound;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::PlayedBy
//
//	PURPOSE:	Get the object that plays the sound
//
// ----------------------------------------------------------------------- //

HOBJECT CinematicTrigger::PlayedBy( char *pszName )
{
	if (!g_pServerDE || m_nCurMessage >= MAX_CT_MESSAGES) return m_hObject;
	if( !pszName )
		return m_hObject;

	ObjectList*	pList = g_pServerDE->FindNamedObjects( pszName );
	if (!pList) return m_hObject;

	ObjectLink* pLink = pList->m_pFirstLink;
	while(pLink)
	{
		if (pLink)
		{
			return pLink->m_hObject;
		}

		pLink = pLink->m_pNext;
	}
	
	g_pServerDE->RelinquishList(pList);

	return m_hObject;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, m_nCurMessage);

	for (int i=0; i < MAX_CT_MESSAGES; i++)
	{
		pServerDE->WriteToMessageFloat(hWrite, m_fDelay[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrSound[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrWhoPlaysSound[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrTargetName[i]);
		pServerDE->WriteToMessageHString(hWrite, m_hstrMessageName[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CinematicTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CinematicTrigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_nCurMessage = pServerDE->ReadFromMessageByte(hRead);

	for (int i=0; i < MAX_CT_MESSAGES; i++)
	{
		m_fDelay[i]				= pServerDE->ReadFromMessageFloat(hRead);
		m_hstrSound[i]			= pServerDE->ReadFromMessageHString(hRead);
		m_hstrWhoPlaysSound[i]	= pServerDE->ReadFromMessageHString(hRead);
		m_hstrTargetName[i]		= pServerDE->ReadFromMessageHString(hRead);
		m_hstrMessageName[i]	= pServerDE->ReadFromMessageHString(hRead);
	}
}


