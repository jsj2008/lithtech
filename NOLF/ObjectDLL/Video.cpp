// (c) 1999 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "Video.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(Video)
    ADD_BOOLPROP(StartOn, LTTRUE)
	ADD_STRINGPROP(Video, "")
END_CLASS_DEFAULT(Video, CClientSFX, NULL, NULL)

// ----------------------------------------------------------------------- //

Video::Video() : CClientSFX()
{
    m_bOn = LTTRUE;
	m_hstrVideo = LTNULL;
}

// ----------------------------------------------------------------------- //

Video::~Video()
{
	FREE_HSTRING(m_hstrVideo);
}

// ----------------------------------------------------------------------- //

uint32 Video::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct *)pData;
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pStruct);
			}

			// TODO: ???
			pStruct->m_Flags = FLAG_FULLPOSITIONRES | FLAG_FORCECLIENTUPDATE;
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
			break;
		}

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //

uint32 Video::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleMsg(hSender, szMsg);
		}
		break;

		default : break;
	}

	return CClientSFX::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //

LTBOOL Video::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bOn = genProp.m_Bool;
	}

    if ( g_pLTServer->GetPropGeneric( "Video", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrVideo = g_pLTServer->CreateString( genProp.m_String );

    return LTTRUE;
}


// ----------------------------------------------------------------------- //

void Video::InitialUpdate(int nInfo)
{
	if (nInfo == INITIALUPDATE_SAVEGAME) return;

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

    uint32 dwUserFlags = m_bOn ? USRFLG_VISIBLE : 0;
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);

	// Tell the clients about the Video...

    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_VIDEO_ID);
    g_pLTServer->WriteToMessageHString(hMessage, m_hstrVideo);
    g_pLTServer->EndMessage(hMessage);
}

// --------------------------------------------------------------------------- //

void Video::HandleMsg(HOBJECT hSender, const char* szMsg)
{
	if (_stricmp(szMsg, "ON") == 0 && !m_bOn)
	{
        uint32 dwUserFlags = USRFLG_VISIBLE;
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
        m_bOn = LTTRUE;
	}
	else if (_stricmp(szMsg, "OFF") == 0 && m_bOn)
	{
        uint32 dwUserFlags = 0;
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
        m_bOn = LTFALSE;
	}
	else if (_stricmp(szMsg, "REMOVE") == 0)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}

// ----------------------------------------------------------------------- //

void Video::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    SAVE_BOOL(m_bOn);
	SAVE_HSTRING(m_hstrVideo);
}

// ----------------------------------------------------------------------- //

void Video::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    LOAD_BOOL(m_bOn);
	LOAD_HSTRING(m_hstrVideo);
}
