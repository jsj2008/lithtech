// ----------------------------------------------------------------------- //
//
// MODULE  : TeleportPoint.cpp
//
// PURPOSE : TeleportPoint implementation
//
// CREATED : 4/21/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "TeleportPoint.h"
#include "ServerUtilities.h"

LINKFROM_MODULE( TeleportPoint );

BEGIN_CLASS(TeleportPoint)
    ADD_BOOLPROP(MoveToFloor, true, "This flag determines whether the object moved to this point will have the bottom of their bounding box aligned with the floor.")
END_CLASS(TeleportPoint, GameBase, "Teleportpoints are used to teleport Character objects to a specific point in the level." )

CMDMGR_BEGIN_REGISTER_CLASS( TeleportPoint )
CMDMGR_END_REGISTER_CLASS( TeleportPoint, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeleportPoint::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 TeleportPoint::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ObjectCreateStruct *pOCS = (ObjectCreateStruct *)pData;
				const GenericPropList *pProps = &pOCS->m_cProperties;
				m_bMoveToFloor	= pProps->GetBool( "MoveToFloor", m_bMoveToFloor );
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SetNextUpdate( UPDATE_NEVER );
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeleportPoint::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TeleportPoint::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_BOOL(m_bMoveToFloor);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeleportPoint::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TeleportPoint::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_BOOL(m_bMoveToFloor);
}
