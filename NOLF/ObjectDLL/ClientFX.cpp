//------------------------------------------------------------------
//
//   MODULE  : CLIENTFX.CPP
//
//   PURPOSE : Implements class CClientFX
//
//   CREATED : On 1/25/99 At 3:59:19 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "clientfx.h"
#include "msgIds.h"

BEGIN_CLASS(CClientFX)
	ADD_STRINGPROP(FxName, "")
	ADD_BOOLPROP(Loop, TRUE)
END_CLASS_DEFAULT(CClientFX, BaseClass, NULL, NULL)

//------------------------------------------------------------------
//
//   FUNCTION : CClientFX()
//
//   PURPOSE  : Standard constructor
//
//------------------------------------------------------------------

CClientFX::CClientFX()
{
	m_bLoop		 = TRUE;
	m_hstrFxName = NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : CClientFX()
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CClientFX::~CClientFX()
{
	if (m_hstrFxName)
	{
        g_pLTServer->FreeString(m_hstrFxName);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : EngineMessageFn()
//
//   PURPOSE  : LithTech object message handler
//
//------------------------------------------------------------------

uint32 CClientFX::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch (messageID)
	{
		case MID_PRECREATE :
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				GenericProp prop;

                g_pLTServer->GetPropGeneric("FxName", &prop);
				if (prop.m_String[0])
				{
                    m_hstrFxName = g_pLTServer->CreateString(prop.m_String);
				}

                g_pLTServer->GetPropGeneric("Loop", &prop);
				m_bLoop = prop.m_Bool;
			}
		}
		break;

		case MID_INITIALUPDATE :
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SendFXMessage();
			}
		}
		break;

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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


//------------------------------------------------------------------
//
//   FUNCTION : SendFXMessage()
//
//   PURPOSE  : Send the special fx message to the client...
//
//------------------------------------------------------------------

void CClientFX::SendFXMessage()
{
	// TEMP!!!
	// Don't create client fx - need to upgrade to use new Sanity code...
	return;


    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

    HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_CLIENTFX_CREATE);
    g_pLTServer->WriteToMessageString(hWrite, g_pLTServer->GetStringData(m_hstrFxName));
    g_pLTServer->WriteToMessageVector(hWrite, &vPos);
    g_pLTServer->WriteToMessageDWord(hWrite, m_bLoop);
    g_pLTServer->EndMessage(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CClientFX::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrFxName);
    g_pLTServer->WriteToMessageByte(hWrite, m_bLoop);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CClientFX::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    m_hstrFxName = g_pLTServer->ReadFromMessageHString(hRead);
    m_bLoop      = (BOOL) g_pLTServer->ReadFromMessageByte(hRead);

	// Send the special fx message to the client(s)...

	SendFXMessage();
}