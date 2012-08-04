// ----------------------------------------------------------------------- //
//
// MODULE  : LightGroup.cpp
//
// PURPOSE : Grouping objects for lighting animations
//
// CREATED : 10/03/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "lightgroup.h"

#include "MsgIDs.h"
#include "ObjectMsgs.h"

LINKFROM_MODULE( LightGroup );


BEGIN_CLASS( LightGroup )

    ADD_BOOLPROP(StartOn, LTTRUE)
	ADD_COLORPROP(StartColor, 255, 255, 255)

END_CLASS_DEFAULT_FLAGS( LightGroup, Engine_LightGroup, NULL, NULL, 0 )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateMsgColor
//
//  PURPOSE:	Validate the color message...
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateColorMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_nArgs != 2 && cpMsgParams.m_nArgs != 4 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMsgColor()" );
			pInterface->CPrint( "    MSG - COLOR - Invalid number of arguments." );
		}
		
		return LTFALSE;
	}

	return LTTRUE;
}

CMDMGR_BEGIN_REGISTER_CLASS( LightGroup )

	CMDMGR_ADD_MSG( TOGGLE, 1, NULL, "TOGGLE" )
	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( COLOR, -1, ValidateColorMsg, "COLOR <value> or <RValue> <GValue> <BValue>" ) 

CMDMGR_END_REGISTER_CLASS( LightGroup, Engine_LightGroup )

LightGroup::LightGroup() :
	m_vColor(1.0f, 1.0f, 1.0f),
	m_bClientNeedsUpdate(false)
{
}

LightGroup::~LightGroup()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 LightGroup::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pOCS = (ObjectCreateStruct*)pData;
			if (!pOCS)
				break;

			if( fData == PRECREATE_WORLDFILE )
			{
				ReadProp(pOCS);
			}

			PostReadProp(pOCS);
		}
		break;

		case MID_INITIALUPDATE:
		{
			if( fData != INITIALUPDATE_SAVEGAME )
			{
				InitialUpdate();
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

		case MID_UPDATE:
		{
			// Send a new update if we were waiting for the client to do something
			if (m_bClientNeedsUpdate)
			{
				UpdateClients();
				// Turn back off
				SetNextUpdate(m_hObject, UPDATE_NEVER);
			}
		}
		break;

		default : break;
	}

	return Engine_LightGroup::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 LightGroup::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			HandleTrigger(hSender, pMsg);
		}
		break;

		default : break;
	}

	return Engine_LightGroup::ObjectMessageFn (hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::ReadProp
//
//	PURPOSE:	Read the properties of the object
//
// ----------------------------------------------------------------------- //

LTBOOL LightGroup::ReadProp(ObjectCreateStruct *pOCS)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
		m_bOn = genProp.m_Bool != LTFALSE;

    if (g_pLTServer->GetPropGeneric("StartColor", &genProp) == LT_OK)
		m_vColor = genProp.m_Color / 255.0f;

	if (g_pLTServer->GetPropGeneric("Name", &genProp) == LT_OK)
	{
		SAFE_STRCPY(pOCS->m_Name, genProp.m_String);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::PostReadProp
//
//	PURPOSE:	Take action after having read the properties of the object
//
// ----------------------------------------------------------------------- //

LTBOOL LightGroup::PostReadProp(ObjectCreateStruct *pOCS)
{
	// Get the ID from the name of the object
	g_pLTServer->GetLightGroupID(pOCS->m_Name, &m_nID);

	// Make sure this object is always in the visible object list
	pOCS->m_Flags |= FLAG_FORCECLIENTUPDATE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void LightGroup::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
 	if (!pMsg) return;

	SAVE_bool(m_bOn);
	SAVE_VECTOR(m_vColor);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void LightGroup::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
 	if (!pMsg) return;

	LOAD_bool(m_bOn);
	LOAD_VECTOR(m_vColor);

	m_bClientNeedsUpdate = true;
	SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::UpdateClients
//
//	PURPOSE:	Sends the current lightgroup status to the clients
//
// ----------------------------------------------------------------------- //

void LightGroup::UpdateClients()
{
	// Calculate our current color
	LTVector vColor = (m_bOn) ? m_vColor : LTVector(0.0f, 0.0f, 0.0f);

	{
		// Set up the update message
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_LIGHTGROUP_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint32(m_nID);
		cMsg.WriteLTVector(vColor);

		// Send the message to all connected clients
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
	
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_LIGHTGROUP_ID);

		cMsg.Writeuint32(m_nID);
		cMsg.WriteLTVector(vColor);

		// Make sure new clients will get the message
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}

	m_bClientNeedsUpdate = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::InitialUpdate
//
//	PURPOSE:	MID_INITIALUPDATE handler
//
// ----------------------------------------------------------------------- //

void LightGroup::InitialUpdate()
{
	SetNextUpdate(m_hObject, UPDATE_NEVER);
	UpdateClients();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightGroup::HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //

void LightGroup::HandleTrigger( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	const char* szMsg = (const char*)pMsg->Readuint32();

	ConParse	cParse;
	cParse.Init(szMsg);
	while (g_pCommonLT->Parse(&cParse) == LT_OK)
	{
		if (_stricmp(cParse.m_Args[0], "TOGGLE") == 0)
		{
			// Toggle the flag
			m_bOn = !m_bOn;
		}
		else if (_stricmp(cParse.m_Args[0], "ON") == 0)
		{
			m_bOn = true;
		}
		else if (_stricmp(cParse.m_Args[0], "OFF") == 0)
		{
			m_bOn = false;
		}
		else if ((_stricmp(cParse.m_Args[0], "COLOR") == 0) && ((cParse.m_nArgs == 2) || (cParse.m_nArgs == 4)))
		{
			if (cParse.m_nArgs == 2)
			{
				float fValue = (float)atof(cParse.m_Args[1]) / 255.0f;
				m_vColor.Init(fValue, fValue, fValue);
			}
			else if (cParse.m_nArgs == 4)
			{
				float fValueR = (float)atof(cParse.m_Args[1]) / 255.0f;
				float fValueG = (float)atof(cParse.m_Args[2]) / 255.0f;
				float fValueB = (float)atof(cParse.m_Args[3]) / 255.0f;
				if (fValueR < 0.0f)
					fValueR = m_vColor.x;
				if (fValueG < 0.0f)
					fValueG = m_vColor.y;
				if (fValueB < 0.0f)
					fValueB = m_vColor.z;
				m_vColor.Init(fValueR, fValueG, fValueB);
			}
		}
	}

	// Tell the clients about the change
	UpdateClients();
}

