// ----------------------------------------------------------------------- //
//
// MODULE  : ExitTrigger.cpp
//
// PURPOSE : ExitTrigger - Implementation
//
// CREATED : 10/6/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ExitTrigger.h"
#include "iltserver.h"
#include "GameServerShell.h"
#include "ServerUtilities.h"
#include "PlayerObj.h"
#include "MsgIDs.h"
#include "ServerMissionMgr.h"

extern CGameServerShell* g_pGameServerShell;

LINKFROM_MODULE( ExitTrigger );


#pragma force_active on
BEGIN_CLASS(ExitTrigger)
	ADD_REALPROP_FLAG(FadeOutTime, 0.0f, 0)
	ADD_REALPROP_FLAG(TriggerDelay, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumberOfActivations, 1, PF_HIDDEN)
	ADD_REALPROP_FLAG(SendDelay, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ActivationSound, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(SoundRadius, 200.0f, PF_HIDDEN)
	PROP_DEFINEGROUP(Commands, PF_GROUP(1)|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Command1, "", PF_GROUP(1)|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Command2, "", PF_GROUP(1)|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Command3, "", PF_GROUP(1)|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Command4, "", PF_GROUP(1)|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Command5, "", PF_GROUP(1)|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Command6, "", PF_GROUP(1)|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Command7, "", PF_GROUP(1)|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Command8, "", PF_GROUP(1)|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Command9, "", PF_GROUP(1)|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(Command10, "", PF_GROUP(1)|PF_HIDDEN)
	ADD_BOOLPROP_FLAG(TriggerTouch, 0 , PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CommandTouch, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(AITriggerName, "", PF_HIDDEN)
	ADD_BOOLPROP_FLAG(PlayerTriggerable, 1, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(AITriggerable, 0, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(WeightedTrigger, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Message1Weight, .5, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(TimedTrigger, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(MinTriggerTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxTriggerTime, 10.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(ActivationCount, 1, PF_HIDDEN)
	ADD_BOOLPROP(ExitMission, 0)
END_CLASS_DEFAULT(ExitTrigger, Trigger, NULL, NULL)
#pragma force_active off

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( ExitTrigger )
CMDMGR_END_REGISTER_CLASS( ExitTrigger, Trigger )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::ExitTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ExitTrigger::ExitTrigger() : Trigger()
{
	m_fFadeOutTime	= 0.0f;
	m_dwFlags		&= ~FLAG_TOUCH_NOTIFY;  // Trigger activate only
	m_bExitMission = false;
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
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ExitTrigger::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

            uint32 dwRet = Trigger::EngineMessageFn(messageID, pData, fData);

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}

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

	return Trigger::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL ExitTrigger::ReadProp(ObjectCreateStruct *pData)
{
    if (!pData) return LTFALSE;

	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("FadeOutTime", &genProp) == LT_OK)
	{
		m_fFadeOutTime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("ExitMission", &genProp) == LT_OK)
	{
		m_bExitMission = !!genProp.m_Bool;
	}

    return LTTRUE;
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
    m_bPlayerTriggerable = LTTRUE;
    m_bAITriggerable     = LTFALSE;
    m_hstrAIName         = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Activate
//
//	PURPOSE:	Handle being activated
//
// ----------------------------------------------------------------------- //

LTBOOL ExitTrigger::Activate()
{
    if (!Trigger::Activate()) return LTFALSE;

	// Tell all the players to remove any bodies they are carrying and any vehicles they are riding...

	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;

		// Send the message if this is not the active player.
		if( pPlayerObj )
		{
			pPlayerObj->SetCarriedObject( LTNULL );
			pPlayerObj->RemoveVehicleModel();
		}

		iter++;
	}


	if( m_bExitMission )
	{
		if( !g_pServerMissionMgr->NextMission( ))
			return LTFALSE;
	}
	else
	{
		if( !g_pServerMissionMgr->ExitLevelSwitch( ))
			return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_FLOAT(m_fFadeOutTime);
	SAVE_bool(m_bExitMission);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

    LOAD_FLOAT(m_fFadeOutTime);
    LOAD_bool(m_bExitMission);
}