// ----------------------------------------------------------------------- //
//
// MODULE  : TextureFX.cpp
//
// PURPOSE : Grouping objects for lighting animations
//
// CREATED : 10/03/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "TextureFX.h"

#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"

LINKFROM_MODULE( TextureFX );

BEGIN_CLASS( TextureFX )

    ADD_STRINGPROP(TextureEffect, "")
	
END_CLASS_DEFAULT_FLAGS( TextureFX, BaseClass, NULL, NULL, 0 )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateMsgColor
//
//  PURPOSE:	Validate the color message...
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateSetVarMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_nArgs != 4 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateSetVarMsg()" );
			pInterface->CPrint( "    MSG - SETVAR - Invalid number of arguments." );
		}
		
		return LTFALSE;
	}

	return LTTRUE;
}

CMDMGR_BEGIN_REGISTER_CLASS( TextureFX )

	CMDMGR_ADD_MSG( SETVAR, -1, ValidateSetVarMsg, "SETVAR <stage> <variable number> <value>" ) 

CMDMGR_END_REGISTER_CLASS( TextureFX, BaseClass )

TextureFX::TextureFX() :
	m_bClientNeedsUpdate(false)
{
}

TextureFX::~TextureFX()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TextureFX::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 TextureFX::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pOCS = (ObjectCreateStruct*)pData;
			if (!pOCS)
				break;

			ReadProp(pOCS);
			PostReadProp(pOCS);
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate();
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
				SetNextUpdate(UPDATE_NEVER);
			}
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TextureFX::ReadProp
//
//	PURPOSE:	Read the properties of the object
//
// ----------------------------------------------------------------------- //

LTBOOL TextureFX::ReadProp(ObjectCreateStruct *pOCS)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("TextureEffect", &genProp) == LT_OK)
	{
		// Get the ID from the name of the object and stage
		for(uint32 nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
		{
			g_pLTServer->GetTextureEffectVarID(genProp.m_String, nCurrStage, &m_Stages[nCurrStage].m_nID);
			m_Stages[nCurrStage].m_nChanged = 0;
		}
	}


	if (g_pLTServer->GetPropGeneric("Name", &genProp) == LT_OK)
	{
		SAFE_STRCPY(pOCS->m_Name, genProp.m_String);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TextureFX::PostReadProp
//
//	PURPOSE:	Take action after having read the properties of the object
//
// ----------------------------------------------------------------------- //

LTBOOL TextureFX::PostReadProp(ObjectCreateStruct *pOCS)
{

	// Make sure this object is always in the visible object list
	pOCS->m_Flags |= FLAG_FORCECLIENTUPDATE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TextureFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TextureFX::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
 	if (!pMsg) 
		return;

	//write out the changed flag and the values
	for(uint32 nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
	{
		//write out the stage ID
		SAVE_DWORD(m_Stages[nCurrStage].m_nID);

		//write out the changed
		SAVE_BYTE((uint8)m_Stages[nCurrStage].m_nChanged);

		//write out the variables
		for(uint32 nCurrVar = 0; nCurrVar < NUM_VARS; nCurrVar++)
		{
			//see if this variable was changed, if it wasn't, don't bother saving
			//it out
			if(m_Stages[nCurrStage].m_nChanged & (1 << nCurrVar))
			{
				SAVE_FLOAT(m_Stages[nCurrStage].m_fVars[nCurrVar]);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TextureFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TextureFX::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
 	if (!pMsg) 
		return;

	//read in the changed flag and the values
	for(uint32 nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
	{
		//read in the ID
		LOAD_DWORD(m_Stages[nCurrStage].m_nID);

		//read in the changed
		LOAD_BYTE(m_Stages[nCurrStage].m_nChanged);

		//read in the variables
		for(uint32 nCurrVar = 0; nCurrVar < NUM_VARS; nCurrVar++)
		{
			if(m_Stages[nCurrStage].m_nChanged & (1 << nCurrVar))
			{
				//only read it in if it was flagges as being changed
				LOAD_FLOAT(m_Stages[nCurrStage].m_fVars[nCurrVar]);
			}
		}
	}

	m_bClientNeedsUpdate = true;
	SetNextUpdate(UPDATE_NEXT_FRAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TextureFX::WriteStageInfo
//
//	PURPOSE:	writes the stage information to a specified message 
//
// ----------------------------------------------------------------------- //

void TextureFX::WriteStageInfo(ILTMessage_Write& cMsg) const
{
	//write out each stage
	for(uint32 nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
	{
		//write out what has changed
		cMsg.Writeuint8((uint8)m_Stages[nCurrStage].m_nChanged);

		//if nothing has changed, don't bother to write out any more info about this stage
		if(m_Stages[nCurrStage].m_nChanged == 0)
			continue;
		
		cMsg.Writeuint32(m_Stages[nCurrStage].m_nID);

		//write out each of the changed values
		for(uint32 nCurrParam = 0; nCurrParam < NUM_VARS; nCurrParam++)
		{
			if(m_Stages[nCurrStage].m_nChanged & (1 << nCurrParam))
			{
				//a changed value, make sure to write it out
				cMsg.Writefloat(m_Stages[nCurrStage].m_fVars[nCurrParam]);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TextureFX::UpdateClients
//
//	PURPOSE:	Sends the current TextureFX status to the clients
//
// ----------------------------------------------------------------------- //

void TextureFX::UpdateClients()
{
	{
		// Set up the update message
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_TEXTUREFX_ID);
		cMsg.WriteObject(m_hObject);
		WriteStageInfo(cMsg);

		// Send the message to all connected clients
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
	
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_TEXTUREFX_ID);
		WriteStageInfo(cMsg);

		// Make sure new clients will get the message
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}

	m_bClientNeedsUpdate = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TextureFX::InitialUpdate
//
//	PURPOSE:	MID_INITIALUPDATE handler
//
// ----------------------------------------------------------------------- //

void TextureFX::InitialUpdate()
{
	UpdateClients();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TextureFX::OnTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //

bool TextureFX::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_SetVar("SETVAR");

	bool bUpdate = false;

	if ((cMsg.GetArg(0) == s_cTok_SetVar) && (cMsg.GetArgCount() >= 4))
	{
		uint32 nStage	= (uint32)atoi(cMsg.GetArg(1)) - 1;
		uint32 nVar		= (uint32)atoi(cMsg.GetArg(2)) - 1;
		float  fVal		= (float)atof(cMsg.GetArg(3));

		if((nStage < NUM_STAGES) && (nVar < NUM_VARS))
		{
			//save the variable
			m_Stages[nStage].m_fVars[nVar] = fVal;

			//mark it as changes
			m_Stages[nStage].m_nChanged |= (1 << nVar);

			//we need to update the clients
			bUpdate = true;
		}
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	// Tell the clients about the change
	if(bUpdate)
		UpdateClients();

	return true;
}

