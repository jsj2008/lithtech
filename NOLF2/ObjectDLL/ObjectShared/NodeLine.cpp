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

LINKFROM_MODULE( NodeLine );

#pragma force_active on
BEGIN_CLASS(NodeLine)
END_CLASS_DEFAULT(NodeLine, BaseClass, NULL, NULL)
#pragma force_active off

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
            if (g_pLTServer) SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
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
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_NODELINES_ID);
	cMsg.WriteLTVector(vSource);
	cMsg.WriteLTVector(vDestination);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}