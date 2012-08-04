// ----------------------------------------------------------------------- //
//
// MODULE  : TeleportPoint.cpp
//
// PURPOSE : TeleportPoint implementation
//
// CREATED : 4/21/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TeleportPoint.h"
#include "ServerUtilities.h"

BEGIN_CLASS(TeleportPoint)
    ADD_BOOLPROP(MoveToFloor, LTTRUE)
END_CLASS_DEFAULT(TeleportPoint, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeleportPoint::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 TeleportPoint::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
                g_pLTServer->GetPropRotationEuler("Rotation", &m_vPitchYawRoll);

				GenericProp genProp;
				if ( g_pLTServer->GetPropGeneric( "MoveToFloor", &genProp ) == LT_OK )
				{
					m_bMoveToFloor = genProp.m_Bool;
				}
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SetNextUpdate(m_hObject, 0.0f);
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

	return BaseClass::EngineMessageFn(messageID, pData, fData);;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeleportPoint::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TeleportPoint::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	SAVE_VECTOR(m_vPitchYawRoll);
	SAVE_BOOL(m_bMoveToFloor);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeleportPoint::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TeleportPoint::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	LOAD_VECTOR(m_vPitchYawRoll);
	LOAD_BOOL(m_bMoveToFloor);
}