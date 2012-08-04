/****************************************************************************
;
;	MODULE:			BinaryBit.cpp
;
;	PURPOSE:		Binary Bit class implementation for TRON
;
;	HISTORY:		2/14/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/


#include "stdafx.h"
#include "BinaryBit.h"
#include "CommonUtilities.h"
#include "ObjectMsgs.h"
#include "PSets.h"
#include "MsgIDs.h"
#include "PlayerObj.h"
#include "ServerSoundMgr.h"

#define ENERGY_UNITS_XFER_PER_SECOND 100.0f
#define BB_MODEL		"Props/Models/BinaryBit.ltb"
#define BB_RS_IDLE		"props\\rs\\BinaryBit_Idle.ltb"
#define BB_RS_YES		"props\\rs\\BinaryBit_Yes.ltb"
#define BB_RS_NO		"props\\rs\\BinaryBit_No.ltb"
#define	KEY_BUTE_SOUND	"BUTE_SOUND_KEY"

BEGIN_CLASS(BinaryBit)
	// Hide the model & skin since we're doing this ourselves
	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_HIDDEN)

	ADD_PSETS_PROP(PF_GROUP(1))
	ADD_LONGINTPROP(EnergyRequired, 0)
	ADD_BOOLPROP(Locked, 0)
	ADD_BOOLPROP(StartOnInSocket, 1)
	PROP_DEFINEGROUP(Commands, PF_GROUP(2))
		ADD_STRINGPROP_FLAG(PowerUpCommand, "", PF_GROUP(2) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(TurnOnCommand, "", PF_GROUP(2) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(TurnOffCommand, "", PF_GROUP(2) | PF_NOTIFYCHANGE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(BinaryBit, GameBase, NULL, NULL, 0, CBinaryBitPlugin)

CMDMGR_BEGIN_REGISTER_CLASS(BinaryBit)
CMDMGR_END_REGISTER_CLASS(BinaryBit, GameBase)

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CBinaryBitPlugin::PreHook_PropChanged
//
//  PURPOSE:	Make sure the Command is valid
//
// ----------------------------------------------------------------------- //
LTRESULT CBinaryBitPlugin::PreHook_PropChanged( const	char		*szObjName,
											    const	char		*szPropName,
											    const	int			nPropType,
											    const	GenericProp	&gpPropValue,
											    ILTPreInterface	*pInterface )
{
	if( LT_OK == m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
														 szPropName,
														 nPropType, 
														 gpPropValue,
														 pInterface ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::BinaryBit()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //
BinaryBit::BinaryBit() : GameBase(OT_MODEL)
{
	m_bLocked = LTFALSE;
	m_eState = BBState_PoweredDown;
	m_nOriginalEnergyRequired = 0;
	m_nCurrentEnergyRequired = 0;
	m_bStartOnInSocket = LTTRUE;
	m_hstrPowerUpCommand = NULL;
	m_hstrTurnOnCommand = NULL;
	m_hstrTurnOffCommand = NULL;
	m_byPSets = 0;
	m_bPlayingTransitionAnim = LTFALSE;
	m_hFX = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::~BinaryBit()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //
BinaryBit::~BinaryBit()
{
	// Free our strings
	TERMSTRING(m_hstrPowerUpCommand);
	TERMSTRING(m_hstrTurnOnCommand);
	TERMSTRING(m_hstrTurnOffCommand);

	// Note: We don't remove the FX here because that's done explicitly
	// before this object is removed. The only other way for this object
	// to be removed is by the engine when it removes ALL objects
	// so we don't need to remove our FX in that case.
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //
uint32 BinaryBit::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// First we call down to the base class.
			// This isn't currently done in all objects, but 
			// it really is good practice to do it this way.
			LTRESULT ret = GameBase::EngineMessageFn(messageID, pData, fData);

			if((fData == PRECREATE_WORLDFILE) || (fData == PRECREATE_STRINGPROP))
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			return ret;
		}

		case MID_INITIALUPDATE:
		{
			// Let our function determine whether or not we call the base class
			if(InitialUpdate())
				return LT_OK;

			return(GameBase::EngineMessageFn(messageID, pData, fData));
		}

		case MID_UPDATE:
		{
			// If we're going to the socket and we have an FX object, we need
			// to move that FX object to our location because we're on a 
			// keyframed path, and the FX object isn't.
			if(m_eState == BBState_GoingToSocket)
			{
				if(m_hFX)
				{
					LTVector vPos;
					g_pLTServer->GetObjectPos(m_hObject,&vPos);
					g_pLTServer->SetObjectPos(m_hFX,&vPos);
					SetNextUpdate(0.001f);
				}

				return LT_OK;
			}
			
			// If we're here, then we're in one of our "Yes" or "No" animations.
			// Let's sanity check just to be sure
			ASSERT(m_bPlayingTransitionAnim && (m_eState == BBState_On) || (m_eState == BBState_Off));

			if(g_pLTServer->GetModelPlaybackState(m_hObject) == MS_PLAYDONE)
			{
				if(m_eState == BBState_On)
				{
					TurnOn();
				}
				else if(m_eState == BBState_Off)
				{
					TurnOff();
				}
				else
				{
					// Uh-oh...
					g_pLTServer->CPrint("BinaryBit::MID_UPDATE - ERROR - Binary Bit was in (%d) state!\n",m_eState);
				}

				// We're idle again
				HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hObject, "Idle");
				g_pLTServer->SetModelAnimation(m_hObject, hAni);
				g_pLTServer->SetModelLooping(m_hObject, LTTRUE);

				m_bPlayingTransitionAnim = LTFALSE;
				
				// We can now activate this object
				g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

				SetNextUpdate(0.0f);
			}
			else
			{
				// Still going
				SetNextUpdate(0.001f);
			}

			return LT_OK;
		}

		case MID_MODELSTRINGKEY:
		{
			if(HandleModelString((ArgList*)pData))
				return LT_OK;

			break;
		}

		case MID_SAVEOBJECT:
		{
			// NOTE: If you want to save the base class first,
			// just switch the following two lines of code.
            Save((ILTMessage_Write*)pData, (uint32)fData);
			LTRESULT ret = GameBase::EngineMessageFn(messageID, pData, fData);

			return ret;
		}

		case MID_LOADOBJECT:
		{
			// NOTE: If you want to load the base class first,
			// just switch the following two lines of code.
            Load((ILTMessage_Read*)pData, (uint32)fData);
			LTRESULT ret = GameBase::EngineMessageFn(messageID, pData, fData);

			return ret;
		}

		default: break;
	}

	return(GameBase::EngineMessageFn(messageID, pData, fData));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
LTBOOL BinaryBit::ReadProp(ObjectCreateStruct *pData)
{
	// Sanity check
    if (!pData) return LTFALSE;

	// Read them in
	GenericProp genProp;

	// Read the permission sets in
	HANDLE_READ_PSETS_PROPS(m_byPSets)

	if(g_pLTServer->GetPropGeneric("EnergyRequired", &genProp) == LT_OK)
	{
	    m_nOriginalEnergyRequired = genProp.m_Long;
		m_nCurrentEnergyRequired = m_nOriginalEnergyRequired;
	}

	if(g_pLTServer->GetPropGeneric("Locked", &genProp) == LT_OK)
	{
	    m_bLocked = genProp.m_Bool;
	}

	if(g_pLTServer->GetPropGeneric("StartOnInSocket", &genProp) == LT_OK)
	{
	    m_bStartOnInSocket = genProp.m_Bool;
	}

	// Model/skin/renderstyle
	SAFE_STRCPY(pData->m_Filename, BB_MODEL);
	strncpy(pData->m_RenderStyleNames[0], BB_RS_IDLE, 256);
	strncpy(pData->m_RenderStyleNames[1], BB_RS_YES, 256);
	strncpy(pData->m_RenderStyleNames[2], BB_RS_NO, 256);

	if(g_pLTServer->GetPropGeneric("PowerUpCommand", &genProp) == LT_OK)
	{
	    if(genProp.m_String[0])
		{
			m_hstrPowerUpCommand = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if(g_pLTServer->GetPropGeneric("TurnOnCommand", &genProp) == LT_OK)
	{
	    if(genProp.m_String[0])
		{
			m_hstrTurnOnCommand = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if(g_pLTServer->GetPropGeneric("TurnOffCommand", &genProp) == LT_OK)
	{
	    if(genProp.m_String[0])
		{
			m_hstrTurnOffCommand = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	// Make sure we get some flags mang'
	pData->m_Flags |= FLAG_MODELKEYS | FLAG_RAYHIT;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //
void BinaryBit::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	// Save the important stuff
	SAVE_BOOL(m_bLocked);
	SAVE_BOOL(m_bStartOnInSocket);
	SAVE_BYTE((BYTE)m_eState);
	SAVE_DWORD(m_nOriginalEnergyRequired);
	SAVE_DWORD(m_nCurrentEnergyRequired);
	SAVE_BOOL(m_bPlayingTransitionAnim);
	SAVE_HSTRING(m_hstrPowerUpCommand);
	SAVE_HSTRING(m_hstrTurnOnCommand);
	SAVE_HSTRING(m_hstrTurnOffCommand);
	SAVE_BYTE(m_byPSets);

	SAVE_HOBJECT(m_hFX);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //
void BinaryBit::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	// Load the important stuff
	LOAD_BOOL(m_bLocked);
	LOAD_BOOL(m_bStartOnInSocket);
	LOAD_BYTE_CAST(m_eState);
	LOAD_DWORD(m_nOriginalEnergyRequired);
	LOAD_DWORD(m_nCurrentEnergyRequired);
	LOAD_BOOL(m_bPlayingTransitionAnim);
	LOAD_HSTRING(m_hstrPowerUpCommand);
	LOAD_HSTRING(m_hstrTurnOnCommand);
	LOAD_HSTRING(m_hstrTurnOffCommand);
    LOAD_BYTE(m_byPSets);

	LOAD_HOBJECT(m_hFX);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
uint32 BinaryBit::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)pMsg->Readuint32();

			// ConParse does not destroy szMsg, so this is safe
			ConParse parse;
			parse.Init((char*)szMsg);

            while (g_pCommonLT->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if(_stricmp(parse.m_Args[0], "ACTIVATE") == 0)
					{
						// We've just received an activate message
						HandleActivateMessage(hSender);
					}
					else if(_stricmp(parse.m_Args[0], "LOCK") == 0)
					{
						m_bLocked = LTTRUE;
					}
					else if(_stricmp(parse.m_Args[0], "UNLOCK") == 0)
					{
						m_bLocked = LTFALSE;
					}
					else if(_stricmp(parse.m_Args[0], "ATSOCKET") == 0)
					{
						// Check to see if we were going to the socket
						if(m_eState != BBState_GoingToSocket)
						{
							g_pLTServer->CPrint("BinaryBit::MID_TRIGGER - ERROR - Binary Bit tried to arrive at socket from state: %d\n",m_eState);
							ASSERT(FALSE);
						}

						HandleArriveAtSocket();
					}
				}
			}

			return LT_OK;
		}
		case MID_STOP_ENERGY_TRANSFER:
		{
			// If we want to do any special handling here, we can.
			return LT_OK;
		}
		case MID_ENERGY_TRANSFER_COMPLETE:
		{
			// Let's power up!
			if(m_eState == BBState_PoweredDown)
				DoPowerUp();

			return LT_OK;
		}
		case MID_QUERY_TARGET_PROPERTIES:
		{
			uint8 nNumProps = pMsg->ReadByte();
			uint8 nNumPropsProcessed = 0;

			// See what we need to send down
			if(nNumProps > 0)
			{
				// Start the message
				CAutoMessage cTempMsg;

				// Loop through the props
				uint8 iProp;
				for(int i=0;i<nNumProps;i++)
				{
					// Get the prop
					iProp = pMsg->ReadByte();
					switch(iProp)
					{
						case TARGET_PROP_ENERGY_REQUIRED:
						{
							// Write the ID, the value, and inc the number of props processed
							cTempMsg.WriteByte(iProp);
							cTempMsg.WriteDWord(m_nCurrentEnergyRequired);
							nNumPropsProcessed++;

							break;
						}
					}
				}

				// Build the actual message to send back down to the client
				if(nNumPropsProcessed > 0)
				{
					// Get the player
					CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
					if(pPlayer)
					{
						CAutoMessage cPropMsg;
						cPropMsg.WriteObject(m_hObject);
						cPropMsg.WriteByte(nNumPropsProcessed);
						cPropMsg.WriteMessage(cTempMsg);
						g_pLTServer->SendToClient(cPropMsg, MID_QUERY_TARGET_PROPERTIES, pPlayer->GetClient(), MESSAGE_GUARANTEED);
					}
				}
			}
			
			return LT_OK;
		}
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::InitialUpdate
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //
LTBOOL BinaryBit::InitialUpdate()
{
	// Make sure we can activate the object
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE | USRFLG_REQUIRES_ENERGY, USRFLG_CAN_ACTIVATE | USRFLG_REQUIRES_ENERGY);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

	// Set the permission sets
	SET_OBJECT_PSETS_USER_FLAGS(m_hObject,m_byPSets)

	// Set the starting anim
	HMODELANIM hAni;
	hAni = g_pLTServer->GetAnimIndex(m_hObject, "PoweredDown");
	g_pLTServer->SetModelAnimation(m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::ChangeState
//
//	PURPOSE:	What to do when we need to change state
//
// ----------------------------------------------------------------------- //
LTBOOL BinaryBit::ChangeState(BinaryBitState eNewState)
{
	// Make sure we're actually changing state
	if(eNewState == m_eState)
		return LTTRUE;
	
	// Welcome to the state machine whose sole purpose is to
	// insure that the level designers aren't complete monkeys.
	// ... or that there could possibly be a bug in the code
	// (which is much less likely) ;)
	switch(eNewState)
	{
		case BBState_PoweredDown:
		{
			// They want to power us down
			g_pLTServer->CPrint("BinaryBit::ChangeState - ERROR - Powering down a Binary Bit is not currently supported. Please fill out an engineering feature request. Have a nice day! :)\n");
			ASSERT(FALSE);
			break;
		}
		case BBState_GoingToSocket:
		{
			// We're about to take a trip
			if(m_eState != BBState_PoweredDown)
			{
				g_pLTServer->CPrint("BinaryBit::ChangeState - ERROR - Binary Bit tried to go to socket from state: %d\n",m_eState);
				ASSERT(FALSE);
			}

			HandleGoingToSocket();
			break;
		}
		case BBState_On:
		{
			// Turn me on, baby
			if(m_eState == BBState_Off)
			{
				// Start us on
				HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hObject, "Yes");
				g_pLTServer->SetModelAnimation(m_hObject, hAni);
				g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

				// We need to update the object until this animation is done
				m_bPlayingTransitionAnim = LTTRUE;
				
				// Temporarily turn off activation for the object
				g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);

				SetNextUpdate(0.001f);
			}
			else if(m_eState != BBState_GoingToSocket)
			{
				g_pLTServer->CPrint("BinaryBit::ChangeState - ERROR - Binary Bit tried to turn ON from state: %d\n",m_eState);
				ASSERT(FALSE);
			}
			else // Just arriving at socket
			{
				// Change color
				SetObjectRenderStyle(m_hObject,0,BB_RS_YES);
			}

			break;
		}
		case BBState_Off:
		{
			// Turning off
			if(m_eState == BBState_On)
			{
				// Start us off
				HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hObject, "No");
				g_pLTServer->SetModelAnimation(m_hObject, hAni);
				g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

				// We need to update the object until this animation is done
				m_bPlayingTransitionAnim = LTTRUE;

				// Temporarily turn off activation for the object
				g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);

				SetNextUpdate(0.001f);
			}
			else if(m_eState != BBState_GoingToSocket)
			{
				g_pLTServer->CPrint("BinaryBit::ChangeState - ERROR - Binary Bit tried to turn OFF from state: %d\n",m_eState);
				ASSERT(FALSE);
			}
			else // Just arriving at socket
			{
				// Change color
				SetObjectRenderStyle(m_hObject,0,BB_RS_NO);
			}

			break;
		}
	}

	// If we're here, then all is good in the world so it's okay to change the state
	m_eState = eNewState;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::TurnOn
//
//	PURPOSE:	What to do when we turn on
//
// ----------------------------------------------------------------------- //
void BinaryBit::TurnOn()
{
	// Send out the "on" trigger message
	if(!m_hstrTurnOnCommand)
		return;

	const char *pCmd;
	pCmd = g_pLTServer->GetStringData(m_hstrTurnOnCommand);
	if(g_pCmdMgr->IsValidCmd(pCmd))
	{
		g_pCmdMgr->Process(pCmd, m_hObject, NULL);
	}
	else
	{
		g_pLTServer->CPrint("BinaryBit::HandleOn - ERROR - Invalid command: %s\n",pCmd);
		ASSERT(FALSE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::TurnOff
//
//	PURPOSE:	What to do when we turn off
//
// ----------------------------------------------------------------------- //
void BinaryBit::TurnOff()
{
	// Send out the "off" trigger messages
	if(!m_hstrTurnOffCommand)
		return;

	const char *pCmd;
	pCmd = g_pLTServer->GetStringData(m_hstrTurnOffCommand);
	if(g_pCmdMgr->IsValidCmd(pCmd))
	{
		g_pCmdMgr->Process(pCmd, m_hObject, NULL);
	}
	else
	{
		g_pLTServer->CPrint("BinaryBit::HandleOff - ERROR - Invalid command: %s\n",pCmd);
		ASSERT(FALSE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::HandleGoingToSocket
//
//	PURPOSE:	What to do when we start going to the socket
//
// ----------------------------------------------------------------------- //
void BinaryBit::HandleGoingToSocket()
{
	// Turn off activation for the object
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE | USRFLG_REQUIRES_ENERGY);

	// And start the trailFX

	// The string for the props of the SpecialFX object
	ObjectCreateStruct ocs;
	LTVector vPos;
	BaseClass* pObj;
	char szProps[1024];
	sprintf(szProps,"FxName BinaryBit_Trail;Loop 1;SmoothShutdown 1");

	// Get our position
	g_pLTServer->GetObjectPos(m_hObject,&vPos);

	INIT_OBJECTCREATESTRUCT(ocs);
	ocs.m_Rotation.Init();
	ocs.m_Pos = vPos;

	HCLASS hClass = g_pLTServer->GetClass("SpecialFX");
	if(hClass)
	{
		// Here's where we actually create the object
		pObj = (BaseClass*)g_pLTServer->CreateObjectProps(hClass, &ocs, szProps);
		if(pObj)
		{
			// Store the HOBJECT in our array
			m_hFX = pObj->m_hObject;

			// Let's get updates
			SetNextUpdate(0.001f);
		}
		else
		{
			// Couldn't create the object
			ASSERT(FALSE);
		}
	}
	else
	{
		// Couldn't find the class
		ASSERT(FALSE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::HandleArriveAtSocket
//
//	PURPOSE:	What to do when we arrive at the socket
//
// ----------------------------------------------------------------------- //
void BinaryBit::HandleArriveAtSocket()
{
	// Don't need updates anymore
	SetNextUpdate(0.0f);

	// We're at the socket.
	if(m_bStartOnInSocket)
	{
		ChangeState(BBState_On);
	}
	else
	{
		ChangeState(BBState_Off);
	}

	// We can now activate this object
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
	
	// And kill our trailFX
	if(m_hFX)
	{
		g_pLTServer->RemoveObject(m_hFX);
		m_hFX = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::HandleActivateMessage
//
//	PURPOSE:	What to do when we get an activate message
//
// ----------------------------------------------------------------------- //
void BinaryBit::HandleActivateMessage(HOBJECT hSender)
{
	// See what state we're in
	switch(m_eState)
	{
		case BBState_PoweredDown:
		{
			// See if we have energy to transfer
			if(m_nCurrentEnergyRequired > 0)
			{
				// Get the client ID from the player (sender)
				ASSERT(IsPlayer(hSender));
				CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
				if(!pPlayer)
				{
					// This came from the console
					DoPowerUp();
				}
				else
				{
					// Send down the energy requirement to the client
					CAutoMessage cMsg;
					cMsg.Writeuint8(MID_START_ENERGY_TRANSFER);
					cMsg.Writeuint32(m_nCurrentEnergyRequired);
					g_pLTServer->SendToClient(cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED);
				}
			}
			else
			{
				// No energy required, just turn us on
				DoPowerUp();
			}
			
			break;
		}
		case BBState_Off:
		{
			// We're off, need to turn on
			if(m_bLocked)
				return;

			ChangeState(BBState_On);
			break;
		}
		case BBState_On:
		{
			// We're on, need to turn off
			if(m_bLocked)
				return;

			ChangeState(BBState_Off);
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::DoPowerUp
//
//	PURPOSE:	We're done transferring energy. Let's power up.
//
// ----------------------------------------------------------------------- //
void BinaryBit::DoPowerUp()
{
	ChangeState(BBState_GoingToSocket);

	// We're all done, send the "PowerUp" trigger message
	if(m_hstrPowerUpCommand)
	{
		const char *pCmd;
		pCmd = g_pLTServer->GetStringData(m_hstrPowerUpCommand);
		if(g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, NULL);
		}
		else
		{
			g_pLTServer->CPrint("BinaryBit::DoPowerUp - ERROR - Invalid command: %s\n",pCmd);
			ASSERT(FALSE);
		}
	}

	// Animate us!
	HMODELANIM hAni;
	hAni = g_pLTServer->GetAnimIndex(m_hObject, "Idle");
	g_pLTServer->SetModelAnimation(m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BinaryBit::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //
LTBOOL BinaryBit::HandleModelString(ArgList* pArgList)
{
    if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0)
		return LTFALSE;

	LTBOOL bHandled = LTFALSE;

	// start the index at the first argument
	int i = 0;
	int nRS;

	// while there are arguments
	while((i < pArgList->argc) &&
	        ('\0' != pArgList->argv[i][0]))
	{
		// Check for renderstyle model key
		if(stricmp(pArgList->argv[i],RENDERSTYLE_MODEL_KEY) == 0)
		{
			// Check params
			if(pArgList->argc >= i+3)
			{
				nRS = (atoi)(pArgList->argv[i+1]);
				SetObjectRenderStyle(m_hObject,nRS,pArgList->argv[i+2]);
			}
			else
			{
				// Not enough params
				g_pLTServer->CPrint("BinaryBit::HandleModelString - ERROR - Not enough RS arguments! Syntax: RS <RSNum> <RSName>\n");
			}

			bHandled = LTTRUE;

			// Move past all arguments of this key
			i += 3;
		}
		else if(!stricmp(pArgList->argv[i], KEY_BUTE_SOUND))
		{
			if((pArgList->argc > (i+1)) && (pArgList->argv[i+1]))
			{
				LTVector vPos;
				g_pLTServer->GetObjectPos(m_hObject, &vPos);
				g_pServerSoundMgr->PlaySoundFromPos(vPos, pArgList->argv[i+1]);
			}

			// Go to the next string
			i++;
		}
		else
		{
			// Go to the next string
			i++;
		}
	}

	return bHandled;
}
