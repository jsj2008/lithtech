// ----------------------------------------------------------------------- //
//
// MODULE  : Alarm.cpp
//
// PURPOSE : Implementation of the alarm
//
// CREATED : 3/27/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Alarm.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "SoundMgr.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AI.h"
#include "AIStimulusMgr.h"
#include "AIUtils.h"
#include "resourceextensions.h"

LINKFROM_MODULE( Alarm );

extern CAIStimulusMgr* g_pAIStimulusMgr;

// Statics

static char *s_szActivate	= "ACTIVATE";
static char *s_szLock		= "LOCK";
static char *s_szUnlock		= "UNLOCK";

// ----------------------------------------------------------------------- //
//
//	CLASS:		Alarm
//
//	PURPOSE:	An alarm object
//
// ----------------------------------------------------------------------- //

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_Alarm 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_Alarm CF_HIDDEN

#endif

BEGIN_CLASS(Alarm)

	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN, "This value changes the size of the object. It is a multiplicative value based on the original size of the object. You can scale an object in all three vectors independently. The default scale is 1.0 1.0 1.0.")
	ADD_VISIBLE_FLAG(0, 0)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(0, 0)
	ADD_BOOLPROP_FLAG(MoveToFloor, false, PF_HIDDEN, "Tells a model to appear on the floor when the level opens, even if you placed it in the air inside WorldEdit. Useful for objects like trees and shrubs on terrain, where it's sometimes difficult to line up the exact floor level under an object.")
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN, "Sets overall translucency of the object, regardless of its alpha mask.")

	ADD_BOOLPROP(PlayerUsable, false, "This flag toggles whether the player is able to activate the alarm.")
	ADD_STRINGPROP_FLAG(PlayerActivateCommand, "", 0, "This is the command string that is sent when the Alarm is activated by a player.")
	ADD_REALPROP_FLAG(SoundTime, 10.0f, 0, "This allows you to specify the amount of time, in seconds, that the Alarm is active.")

	ADD_BOOLPROP_FLAG(CanTransition, false, PF_HIDDEN, "When this object is placed within a TransitionArea, it will be transitioned to the next level when a transition occurs.  Set this flag to flase if you do not want the object to transition.")
	
END_CLASS_FLAGS_PLUGIN(Alarm, Prop, CF_HIDDEN_Alarm, CAlarmPlugin, "An alarm is a prop (world model) that can be 'activated' by AI or player which will cause command to be executed.")

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Alarm )

	ADD_MESSAGE( LOCK, 1, NULL, MSG_HANDLER( Alarm, HandleLockMsg ), "LOCK", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( UNLOCK, 1, NULL, MSG_HANDLER( Alarm, HandleUnlockMsg ), "UNLOCK", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( ACTIVATE, 1, NULL, MSG_HANDLER( Alarm, HandleActivateMsg ), "Activate", "TODO:CMDDESC", "TODO:CMDEXP" )

CMDMGR_END_REGISTER_CLASS( Alarm, Prop )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAlarmPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CAlarmPlugin::PreHook_PropChanged( const char *szObjName,
											const char *szPropName, 
											const int  nPropType, 
											const GenericProp &gpPropValue,
											ILTPreInterface *pInterface,
											const char *szModifiers )
{
	// First send it down to our propplugin...

	if( CPropPlugin::PreHook_PropChanged( szObjName,
										  szPropName,
										  nPropType,
										  gpPropValue,
										  pInterface,
										  szModifiers ) == LT_OK )
	{
		return LT_OK;
	}
	
	// Only our command is marked for change notification so just send it to the CommandMgr..

	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
												szPropName, 
												nPropType, 
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Alarm()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Alarm::Alarm() : Prop ()
{
	m_eState = eStateOff;
	m_bPlayerUsable = false;
	m_bLocked = false;

	m_fAlarmSoundTime = 0.f;

	// Clear the removeondeath flag.
	m_damage.m_DestructibleModelFlags = m_damage.m_DestructibleModelFlags & ~CDestructibleModel::kDestructibleModelFlag_RemoveOnDeath;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::~Alarm()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Alarm::~Alarm()
{
	if( !g_pLTServer ) return;

	m_eState = eStateOff;
	m_bPlayerUsable = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Alarm::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			break;
		}
		
		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool Alarm::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return false;

	m_bPlayerUsable				= pProps->GetBool( "PlayerUsable", m_bPlayerUsable );
	m_fAlarmSoundTime			= pProps->GetReal( "SoundTime", m_fAlarmSoundTime );
	m_sPlayerActivateCommand	= pProps->GetString( "PlayerActivateCommand", "" );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Alarm::PostPropRead(ObjectCreateStruct *pStruct)
{
	if ( !pStruct ) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Alarm::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	if (!g_pLTServer) return 0;

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

			uint32 dwRet = Prop::ObjectMessageFn(hSender, pMsg);

			// Check to see if we have been destroyed

			if ( m_damage.IsDead() )
			{
				m_eState = eStateDestroyed;
			}

			// TODO: Check to see if we have been disbled

			return dwRet;
		}

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::HandleActivateMsg()
//
//	PURPOSE:	Handle a ACTIVATE message...
//
// ----------------------------------------------------------------------- //

void Alarm::HandleActivateMsg( HOBJECT hSender, const CParsedMsg& /*crParsedMsg*/ )
{
	// If the alarm is activated from a command, the sender is
	// NULL (dammit!).  So treat it as player-activated.
	// If the alarm is activated by something other than AI
	// (e.g. a command object) consider it player activated.

	if( !IsAI( hSender ) )
	{
		CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
		hSender = pPlayer->m_hObject;
	}

	HOBJECT hStimulus = hSender;

	if( IsPlayer(hSender) )
	{
		// Run the alarm's player-activate command.

		if( !m_sPlayerActivateCommand.empty() )
		{
			g_pCmdMgr->QueueCommand( m_sPlayerActivateCommand.c_str(), m_hObject, m_hObject );
		}
	}
	else
	{
		// The stimulus is the target of the AI who activated the alarm.

		CAI* pAI = (CAI*)g_pLTServer->HandleToObject(hSender);
		hStimulus = pAI->GetAIBlackBoard()->GetBBTargetObject();
	}

	AITRACE( AIShowAlarms, ( m_hObject, "Triggering alarm" ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::HandleLockMsg()
//
//	PURPOSE:	Handle a LOCK message...
//
// ----------------------------------------------------------------------- //

void Alarm::HandleLockMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	m_bLocked = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::HandleUnockMsg()
//
//	PURPOSE:	Handle a UNLOCK message...
//
// ----------------------------------------------------------------------- //

void Alarm::HandleUnlockMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	m_bLocked = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

bool Alarm::InitialUpdate()
{
	// Make sure we're non-solid...	

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_SOLID);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Alarm::Save(ILTMessage_Write *pMsg)
{
	if (!g_pLTServer || !pMsg) return;

	SAVE_DWORD(m_eState);
	SAVE_BOOL(m_bPlayerUsable);
	SAVE_BOOL(m_bLocked);
	SAVE_FLOAT(m_fAlarmSoundTime);
	SAVE_STDSTRING(m_sPlayerActivateCommand);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Alarm::Load(ILTMessage_Read *pMsg)
{
	if (!g_pLTServer || !pMsg) return;

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_BOOL(m_bPlayerUsable);
	LOAD_BOOL(m_bLocked);
	LOAD_FLOAT(m_fAlarmSoundTime);
	LOAD_STDSTRING( m_sPlayerActivateCommand );
}

