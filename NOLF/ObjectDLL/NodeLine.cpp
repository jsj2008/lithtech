// ----------------------------------------------------------------------- //
//
// MODULE  : NodeLine.cpp
//
// PURPOSE : Implementation of node line visualization debugging thingy
//
// CREATED : 2/11/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "NodeLine.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"

BEGIN_CLASS(NodeLine)
END_CLASS_DEFAULT(NodeLine, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NodeLine::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 NodeLine::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
            if (g_pLTServer) g_pLTServer->SetNextUpdate(m_hObject, 0.01f);
			break;
		}

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NodeLine::Setup()
//
//	PURPOSE:	Sends data down to client
//
// ----------------------------------------------------------------------- //

void NodeLine::Setup(const LTVector& vSource, const LTVector& vDestination)
{
    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_NODELINES_ID);
    g_pLTServer->WriteToMessageVector(hMessage, (LTVector*)&vSource);
    g_pLTServer->WriteToMessageVector(hMessage, (LTVector*)&vDestination);
    g_pLTServer->EndMessage(hMessage);
}