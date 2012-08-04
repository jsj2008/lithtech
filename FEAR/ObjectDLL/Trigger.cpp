// ----------------------------------------------------------------------- //
//
// MODULE  : Trigger.cpp
//
// PURPOSE : Trigger - Implementation
//
// CREATED : 10/6/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Trigger.h"
#include "iltserver.h"
#include "MsgIDs.h"
#include "PlayerObj.h"
#include "GameServerShell.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "ServerSoundMgr.h"
#include <stdio.h>
#include "AIUtils.h"
#include "GameModeMgr.h"

LINKFROM_MODULE( Trigger );


BEGIN_CLASS(Trigger)
	ADD_VECTORPROP_VAL_FLAG(Dims, 16.0f, 16.0f, 16.0f, PF_DIMS, "The dimensions of the trigger in the game. If the player has to walk into the trigger to fire it, you should set the dimensions. If the trigger is fired off by another object, then they don't need to be changed. Trigger dimensions are set as x, y and z values. You can make each of them as small as 0 or as large as you need. For example, for a trigger that fills a typical doorway you would set the dims to 128 128 4. Using the 4 is just to make the object easier to find. Triggers can be any size.")
	ADD_LONGINTPROP(NumberOfActivations, 1, "This value sets how many times the Trigger object can be activated. Setting this value to 0 allows the Trigger to be activated an infinite number of times.")
	ADD_REALPROP(SendDelay, 0.0f, "A specified amount of seconds between the time the Trigger object receives a trigger message, and the when it sends its command strings.")
	ADD_REALPROP(TriggerDelay, 0.0, "After being triggered, this is the amount of time before the Trigger object can be activated again.")

	PROP_DEFINEGROUP(Commands, PF_GROUP(1), "This is a subset of properties that are command strings the trigger object will process when triggered.")
		ADD_COMMANDPROP_FLAG(Command1, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "These are the command strings that will be sent.")
		ADD_COMMANDPROP_FLAG(Command2, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "These are the command strings that will be sent.")
		ADD_COMMANDPROP_FLAG(Command3, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "These are the command strings that will be sent.")
		ADD_COMMANDPROP_FLAG(Command4, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "These are the command strings that will be sent.")
		ADD_COMMANDPROP_FLAG(Command5, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "These are the command strings that will be sent.")
		ADD_COMMANDPROP_FLAG(Command6, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "These are the command strings that will be sent.")
		ADD_COMMANDPROP_FLAG(Command7, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "These are the command strings that will be sent.")
		ADD_COMMANDPROP_FLAG(Command8, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "These are the command strings that will be sent.")
		ADD_COMMANDPROP_FLAG(Command9, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "These are the command strings that will be sent.")
		ADD_COMMANDPROP_FLAG(Command10, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "These are the command strings that will be sent.")

	ADD_BOOLPROP(PlayerTriggerable, 1, "This flag toggles whether or not the players bounding box intersecting that of the Trigger will activate it.")
	ADD_BOOLPROP(AITriggerable, 0, "This flag toggles whether or not an AIs bounding box intersecting that of the Trigger will activate it.")
	ADD_STRINGPROP(AITriggerName, "", "Triggers that are set to be AI triggerable can be set up to only respond to a specific AI. Enter the name of the specific AI that you want to activate the trigger in this field.")
    ADD_BOOLPROP(TimedTrigger, false, "If this flag is set to true the Trigger object will continue to send its messages repeatedly based on the minimum and maximum times set in the MinTriggerTime and MaxTriggerTime fields.")
	ADD_REALPROP(MinTriggerTime, 0.0f, "If the trigger's TimedTrigger flag is set to true, this value will set the shortest delay before the trigger fires.")
	ADD_REALPROP(MaxTriggerTime, 10.0f, "If the trigger's TimedTrigger flag is set to true, this value will set the longest delay before the trigger fires.")
	ADD_LONGINTPROP(ActivationCount, 1, "This field is used to set an activation threshold. The Trigger object will need to receive a number of 'trigger' messages equal to the value entered in this field before its messages will be sent.")
	ADD_BOOLPROP(Locked, 0, "This property can be used if you only want to be able to trip the trigger after a certain event has happened. You would also need a Condition variable, or Unlock messages on the objects that can unlock the trigger.")
	ADD_STRINGPROP_FLAG(ActivationSound, "", PF_FILENAME, "Specifies a sound file to play when the trigger fires. If you want a player to know that they've just set something off, found a clue or entered a cut scene, this can be useful.")
	ADD_REALPROP_FLAG(SoundRadius, 200.0f, PF_RADIUS, "Determines the distance from the trigger that you can hear the ActivationSound play, if it has one.")
	ADD_STRINGPROP_FLAG(AttachToObject, "", PF_OBJECTLINK, "Attaches a Trigger to an object, such as a Prop or a WorldModel. If the object moves, so does the trigger.")
	ADD_REALPROP_FLAG(HUDLookAtDist, -1.0f, PF_RADIUS, "If this is greater than 0, a HUD icon indicating that the player is close to the trigger will appear when the player is looking at the trigger and is this distancs away from the dims of the object.")
	ADD_REALPROP_FLAG(HUDAlwaysOnDist, -1.0f, PF_RADIUS, "If this is greater than 0, a HUD icon indicating that the player is close to the trigger will appear when the player is this distance away form the center of the object.")
	ADD_STRINGPROP_FLAG( TriggerType, "<none>", PF_STATICLIST, "A dropdown list of 'TriggerTypes' from TriggerTypes.txt.  Selecting a TriggerType will display the icon specified in the text file when the player is within the HUDLookAtDist and HUDAlwaysOnDist.  Selecting <none> will display no icon.")
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST, "This is a dropdown that allows you to set which team can activate this trigger while playing a team game.")
END_CLASS_FLAGS_PLUGIN(Trigger, GameBase, 0, CTriggerPlugin, "Trigger objects are used to send a number of commands when the player touches the trigger or through controlled activation by sending the trigger an ON message." )


//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Trigger )

	ADD_MESSAGE( LOCK,		1,	NULL,	MSG_HANDLER( Trigger, HandleLockMsg ),		"LOCK", "Locks the Trigger so that it ignores all commands other than UNLOCK and REMOVE and cannot be activated by the player", "msg Trigger LOCK" )
	ADD_MESSAGE( UNLOCK,	1,	NULL,	MSG_HANDLER( Trigger, HandleUnlockMsg ),	"UNLOCK", "Unlocks the Trigger", "msg Trigger UNLOCK" )
	ADD_MESSAGE( ON,		1,	NULL,	MSG_HANDLER( Trigger, HandleOnMsg ),		"ON", "Tells the object to send its commands", "msg Trigger ON" )
	ADD_MESSAGE( TEAM,		2,	NULL,	MSG_HANDLER( Trigger, HandleTeamMsg ),		"TEAM <0, 1, -1>", "Specifies which team can activate this trigger", "msg Trigger (TEAM 1)" )

CMDMGR_END_REGISTER_CLASS( Trigger, GameBase )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Fill out the list of key types
//
// ----------------------------------------------------------------------- //

LTRESULT CTriggerPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if( !cMaxStrings || !cMaxStringLength )
		return LT_ERROR;

	if( LTStrICmp( szPropName, "TriggerType" ) == 0 )
	{
		if( !m_TriggerTypeDBPlugin.PopulateStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
		{
			return LT_UNSUPPORTED;
		}

		return LT_OK;
	}

	// Handle team...
	if( LTStrIEquals( "Team", szPropName ))
	{
		TeamPopulateEditStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
		return LT_OK;
	}


	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerPlugin::PreHook_PropChanged()
//
//	PURPOSE:	Check our commands
//
// ----------------------------------------------------------------------- //

LTRESULT CTriggerPlugin::PreHook_PropChanged( const char *szObjName,
											  const char *szPropName,
											  const int nPropType,
											  const GenericProp &gpPropValue,
											  ILTPreInterface *pInterface,
											  const char *szModifiers )
{
	// Just send down to the command mgr plugin...

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


// Static global variables...

#define UPDATE_DELTA					0.1f
#define TRIGGER_DEACTIVATION_TIME		0.001f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Trigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Trigger::Trigger()
:	GameBase					( OT_NORMAL ),
	m_vDims						( 5.0f, 5.0f, 5.0f ),
	m_fTriggerDelay				( 0.0f ),
	m_hTouchObject				( NULL ),
	m_sActivationSound			( ),
	m_sAttachToObject			( ),
	m_bAttached					( false ),
	m_fSoundRadius				( 200.0f ),
	m_bActive					( true ),
	m_bPlayerTriggerable		( true ),
	m_bAITriggerable			( false ),
	m_bLocked					( false ),
	m_sAIName					( ),
	m_bDelayingActivate			( false ),
	m_fStartDelayTime			( 0.0f ),
	m_fSendDelay				( 0.0f ),

	// If a trigger delay is set, we use compare the current time and 
	// m_fLastTouchTime + m_fTriggerDelay to determine if enough time has
	// elapsed to allow a trigger reactivation.  If we initialize 
	// m_fLastTouchTime to 0, the trigger cannot be touched at the start 
	// of the level for the first m_fStartDelayTime seconds, as game time 
	// starts at 0.

	m_fLastTouchTime			( -DBL_MAX ), 
	m_nCurrentActivation		( 0 ),
	m_nActivationCount			( 1 ),
	m_nNumActivations			( 1 ),
	m_nNumTimesActivated		( 0 ),
	m_bTimedTrigger				( false ),
	m_fMinTriggerTime			( 0.0f ),
	m_fMaxTriggerTime			( 1.0f ),
	m_bSendTriggerFXMsg			( false ),
	m_nTeamID					( INVALID_TEAM ),
	m_pTriggerCS				( NULL ),
	m_saCommands				( )
{
	m_dwFlags				= (FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD);
	
	m_TriggerTimer.SetEngineTimer( SimulationTimer::Instance( ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::~Trigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Trigger::~Trigger()
{
	if( m_pTriggerCS )
	{
		debug_delete( m_pTriggerCS );
		m_pTriggerCS = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Trigger::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update())
			{
				g_pLTServer->RemoveObject(m_hObject);
			}
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			ObjectTouch((HOBJECT)pData);
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct *)pData;
			if (!pStruct) return 0;

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(&pStruct->m_cProperties);
			}

			pStruct->m_UserData = USRFLG_IGNORE_PROJECTILES;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
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

		case MID_PARENTATTACHMENTREMOVED :
		{
			// Go away if our parent is removed...

			g_pLTServer->RemoveObject(m_hObject);
		}
		break;

		default : break;
	}


	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::HandleLockMsg
//
//	PURPOSE:	Handle a LOCK message...
//
// ----------------------------------------------------------------------- //

void Trigger::HandleLockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bLocked = true;
	
	if( m_bTimedTrigger )
	{
		// Don't update the timer while locked...
		m_TriggerTimer.Pause( );
	}

	SendLockedMsg();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::HandleUnlockMsg
//
//	PURPOSE:	Handle a UNLOCK message...
//
// ----------------------------------------------------------------------- //

void Trigger::HandleUnlockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bLocked = false;

	if( m_bTimedTrigger )
	{
		// Continue the countdown... 
		m_TriggerTimer.Resume( );
	}
	
	SendLockedMsg();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::HandleOnMsg
//
//	PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void Trigger::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	DoTrigger(hSender);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::HandleTeamMsg()
//
//	PURPOSE:	Handle a TEAM message...
//
// ----------------------------------------------------------------------- //

void Trigger::HandleTeamMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	int nTeamId = atoi( crParsedMsg.GetArg( 1 ));
	if( nTeamId < MAX_TEAMS )
	{
		ASSERT( nTeamId == ( uint8 )nTeamId );
		m_nTeamID = ( uint8 )LTCLAMP( nTeamId, 0, 255 );
	}
	else
	{
		m_nTeamID = INVALID_TEAM;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool Trigger::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return false;

	m_saCommands.reserve( MAX_NUM_COMMANDS );

	char szCommandName[50] = {0};
	const char *pszCommand = NULL;
	for( uint32 nCmd = 0; nCmd < MAX_NUM_COMMANDS; ++nCmd )
	{
		LTSNPrintF( szCommandName, ARRAY_LEN( szCommandName ), "Command%d", nCmd + 1 );
		pszCommand = pProps->GetCommand( szCommandName, "" );
		if( pszCommand && pszCommand[0] )
		{
			m_saCommands.push_back( pszCommand );
		}
	}

	// Shrink the array...
	StringArray( m_saCommands ).swap( m_saCommands );

	
	m_sActivationSound		= pProps->GetString( "ActivationSound", "" );
	m_sAttachToObject		= pProps->GetString( "AttachToObject", "" );
	m_vDims					= pProps->GetVector( "Dims", m_vDims );
	m_fTriggerDelay			= pProps->GetReal( "TriggerDelay", m_fTriggerDelay );
	m_fSendDelay			= pProps->GetReal( "SendDelay", m_fSendDelay );
	m_bPlayerTriggerable	= pProps->GetBool( "PlayerTriggerable", m_bPlayerTriggerable );
	m_bAITriggerable		= pProps->GetBool( "AITriggerable", m_bAITriggerable );
	m_bLocked				= pProps->GetBool( "Locked", m_bLocked );
	m_fSoundRadius			= pProps->GetReal( "SoundRadius", m_fSoundRadius );
	m_sAIName				= pProps->GetString( "AITriggerName", "" );
	m_nActivationCount		= pProps->GetLongInt( "ActivationCount", m_nActivationCount );
	m_nNumActivations		= pProps->GetLongInt( "NumberOfActivations", m_nNumActivations );
	m_bTimedTrigger			= pProps->GetBool( "TimedTrigger", m_bTimedTrigger );
	m_fMinTriggerTime		= pProps->GetReal( "MinTriggerTime", m_fMinTriggerTime );
	m_fMaxTriggerTime		= pProps->GetReal( "MaxTriggerTime", m_fMaxTriggerTime );

	// Start the countdown if timed...
	if( m_bTimedTrigger )
	{
		// Start a countdown...
		m_TriggerTimer.Start( GetRandom(m_fMinTriggerTime, m_fMaxTriggerTime ));

		if( m_bLocked )
		{
			// Pause the countdown while locked...
			m_TriggerTimer.Pause( );
		}
	}

	float fHUDLooakAtDis	= pProps->GetReal( "HUDLookAtDist", -1.0f );
	float fHUDAlwaysOnDist	= pProps->GetReal( "HUDAlwaysOnDist", -1.0f );
	HRECORD hTriggerType = TriggerTypeDB::Instance().GetTriggerRecord(pProps->GetString( "TriggerType", "" ));

	// Only create the trigger create struct if it is actualy needed...
	if( fHUDLooakAtDis > 0.0f || fHUDAlwaysOnDist > 0.0f || hTriggerType )
	{
		CreateTriggerCreateStruct( );
		if( m_pTriggerCS )
		{
			m_pTriggerCS->fHUDLookAtDist = fHUDLooakAtDis;
			m_pTriggerCS->fHUDAlwaysOnDist = fHUDAlwaysOnDist;
			m_pTriggerCS->hTriggerTypeRecord = hTriggerType;
		}
	}

	// Get the team the trigger belongs to, but only when in a team game...
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		m_nTeamID = TeamStringToTeamId( pProps->GetString( "Team", "" ));
	}
	else
	{
		m_nTeamID = INVALID_TEAM;
	}



	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void Trigger::ObjectTouch(HOBJECT hObj)
{
	// Only AI and players and bodies can trigger things...

	bool bIsPlayer	= (IsPlayer(hObj) ? true : false);
	bool bIsAI		= (IsAI(hObj) ? true : false);

	if (!bIsAI && !bIsPlayer)
	{
		return;
	}

	// If we're AI, make sure we can activate this trigger...

	if (m_bAITriggerable)
	{
		 if ( bIsAI )
		{
			if( !m_sAIName.empty() ) // See if only a specific AI can trigger it...
			{
				const char* pAIName  = m_sAIName.c_str();
				const char* pObjName = GetObjectName(hObj);

				if (pAIName && pObjName)
				{
					if ( LTStrICmp(pAIName, pObjName) != 0 )
					{
						return;
					}
				}
			}
		}
	}
	else  // Not AI triggerable
	{
		if ( bIsAI )
		{
			return;
		}
	}


	// Check if this is not supposed to be player triggerable.
	if (m_bPlayerTriggerable)
	{
		if (GameModeMgr::Instance( ).m_grbUseTeams && m_nTeamID != INVALID_TEAM) 
		{
			CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( hObj ));
			if( !pPlayer || pPlayer->GetTeamID() != m_nTeamID  )
				return;
			

		}
	}
	else
	{
		if( bIsPlayer )
		{
			return;
		}
	}

	DoTrigger(hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::DoTrigger
//
//	PURPOSE:	Determine if we can be triggered, and if so do it...
//
// ----------------------------------------------------------------------- //

void Trigger::DoTrigger(HOBJECT hObj)
{
	// Okay ready to trigger.  Make sure we've waited long enough before triggering...
    double fTime = g_pLTServer->GetTime();

	if (fTime < m_fLastTouchTime + m_fTriggerDelay)
	{
		return;	
	}
	m_fLastTouchTime = fTime;

	m_hTouchObject = hObj;

	if (m_bActive)
	{
		if (!m_bLocked)
		{
			RequestActivate();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

bool Trigger::InitialUpdate()
{
	g_pPhysicsLT->SetObjectDims(m_hObject, &m_vDims, 0);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags, FLAGMASK_ALL);
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_IGNORE_PROJECTILES, FLAGMASK_ALL);

	// If I'm not a timed trigger, my object touch notification
	// will trigger new updates until then, I don't care...

	if( m_bTimedTrigger || !m_sAttachToObject.empty() )
	{
		SetNextUpdate(UPDATE_DELTA);
	}
	else
	{
		SetNextUpdate(UPDATE_NEVER);
	}

	// Create the specialfx message...

	if( m_pTriggerCS && (m_pTriggerCS->fHUDLookAtDist > 0.0f || m_pTriggerCS->fHUDAlwaysOnDist > 0.0f ))
	{
		// Only send the message if we need to...
		
		m_bSendTriggerFXMsg = true;
	}

	if( m_bSendTriggerFXMsg )
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE , FLAG_FORCECLIENTUPDATE );
		CreateSpecialFX();
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

bool Trigger::Update()
{
	// Handle timed trigger...

	if( m_bTimedTrigger && m_TriggerTimer.IsTimedOut( ) )
	{
		// Restart the countdown...
		m_TriggerTimer.Start( GetRandom( m_fMinTriggerTime, m_fMaxTriggerTime ));
		DoTrigger(NULL);
	}


	// Attach the trigger to the object...

	if( !m_sAttachToObject.empty() && !m_bAttached )
	{
		AttachToObject();
		m_bAttached = true;
	}



	if (m_bDelayingActivate)
	{
		UpdateDelayingActivate();
	}
	else
	{
		m_bActive = true;

		// If not a timed trigger, my object touch notification will trigger
		// new updates until then, I don't care.

		if (m_bTimedTrigger)
		{
			SetNextUpdate(UPDATE_DELTA);
		}
		else
		{
			SetNextUpdate(UPDATE_NEVER);
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::RequestActivate
//
//	PURPOSE:	Request activation of the trigger
//
// ----------------------------------------------------------------------- //

void Trigger::RequestActivate()
{
	if (m_bActive)
	{
		m_fStartDelayTime		= g_pLTServer->GetTime();

		m_bDelayingActivate		= true;
		m_bActive				= false;
		if (m_fTriggerDelay > 0.0f)
			SetNextUpdate(UPDATE_NEXT_FRAME);
		else
			Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::UpdateDelayingActivate
//
//	PURPOSE:	Update the delaying (and possibly activate) the trigger
//
// ----------------------------------------------------------------------- //

void Trigger::UpdateDelayingActivate()
{
	if (!m_bDelayingActivate) return;

	double fTime = g_pLTServer->GetTime();

	if (fTime >= m_fStartDelayTime + m_fSendDelay)
	{
		Activate();
		m_bDelayingActivate	= false;
		m_bActive			= true;
	}
	else
	{
		SetNextUpdate(UPDATE_NEXT_FRAME);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Activate
//
//	PURPOSE:	Activate the trigger.
//
// ----------------------------------------------------------------------- //

bool Trigger::Activate()
{
	// Make us wait a bit before we can be triggered again...

	if (m_bTimedTrigger)
	{
		SetNextUpdate(UPDATE_DELTA);
	}
	else
	{
		SetNextUpdate(m_fTriggerDelay);
	}


	// If this is a counter trigger, determine if we can activate or not...

	if (++m_nCurrentActivation < m_nActivationCount)
	{
		return false;
	}
	else
	{
		m_nCurrentActivation = 0;
	}


	// Only allow the object to be activated the number of specified times...

	if (m_nNumActivations > 0)
	{
		if (m_nNumTimesActivated >= m_nNumActivations)
		{
			return false;
		}

		m_nNumTimesActivated++;
	}


	if( !m_sActivationSound.empty() )
	{
		const char* pSound = m_sActivationSound.c_str();
		if (pSound && pSound[0] != '\0')
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, NULL, m_fSoundRadius, SOUNDPRIORITY_MISC_HIGH,
				PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
				DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
		}
	}

	// Loop through the commands and execute them...

	StringArray::iterator iter;
	for( iter = m_saCommands.begin(); iter != m_saCommands.end(); ++iter )
	{
		if( !(*iter).empty() )
		{
			g_pCmdMgr->QueueCommand( (*iter).c_str(), m_hTouchObject, m_hTouchObject );
		}
	}

	// Clear the toucher.
	m_hTouchObject = NULL;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::AttachToObject()
//
//	PURPOSE:	Attach the trigger to an object
//
// ----------------------------------------------------------------------- //

void Trigger::AttachToObject()
{
	if( m_sAttachToObject.empty() )
		return;

	// Find object to attach to...

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
	g_pLTServer->FindNamedObjects( m_sAttachToObject.c_str(), objArray);

	if (!objArray.NumObjects()) return;

	HOBJECT hObj = objArray.GetObject(0);

	if (!hObj) return;

	LTVector vOffset(0, 0, 0);

	LTRotation rOffset;

	HATTACHMENT hAttachment;
	g_pLTServer->CreateAttachment(hObj, m_hObject, NULL, &vOffset, &rOffset, &hAttachment);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Trigger::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	if( m_pTriggerCS )
	{
		SAVE_BOOL( true );
		m_pTriggerCS->Write( pMsg );
	}
	else
	{
		SAVE_BOOL( false );
	}

	m_TriggerTimer.Save( *pMsg );

	SAVE_HOBJECT(m_hTouchObject);
	SAVE_BOOL(m_bAttached);
	SAVE_BOOL(m_bActive);
	SAVE_BOOL(m_bPlayerTriggerable);
	SAVE_BOOL(m_bAITriggerable);
	SAVE_BOOL(m_bLocked);
	SAVE_BOOL(m_bDelayingActivate);
	SAVE_BOOL(m_bTimedTrigger);

	SAVE_TIME(m_fStartDelayTime);
	SAVE_FLOAT(m_fSendDelay);
	SAVE_TIME(m_fLastTouchTime);
	SAVE_FLOAT(m_fMinTriggerTime);
	SAVE_FLOAT(m_fMaxTriggerTime);
	SAVE_FLOAT(m_fTriggerDelay);
	SAVE_FLOAT(m_fSoundRadius);
	SAVE_INT(m_nNumActivations);

	SAVE_DWORD(m_nNumTimesActivated);
	SAVE_DWORD(m_nActivationCount);
	SAVE_DWORD(m_nCurrentActivation);

	SAVE_STDSTRING(m_sAIName);
	SAVE_STDSTRING(m_sActivationSound);

	SAVE_bool(m_bSendTriggerFXMsg);

	SAVE_DWORD( m_saCommands.size() );

	StringArray::iterator iter;
	for( iter = m_saCommands.begin(); iter != m_saCommands.end(); ++iter )
	{
		SAVE_STDSTRING( (*iter) );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Trigger::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	bool bLoadTriggerCS = false;
	LOAD_BOOL( bLoadTriggerCS );
	
	if( bLoadTriggerCS )
	{
		CreateTriggerCreateStruct( );
		if( !m_pTriggerCS )
		{
			LTERROR( "Failed to create Trigger Create Struct on Load!" );
		}
		
		m_pTriggerCS->Read( pMsg );
	}
	
	m_TriggerTimer.Load( *pMsg );

	LOAD_HOBJECT(m_hTouchObject);

	LOAD_BOOL(m_bAttached);
	LOAD_BOOL(m_bActive);
	LOAD_BOOL(m_bPlayerTriggerable);
	LOAD_BOOL(m_bAITriggerable);
	LOAD_BOOL(m_bLocked);
	LOAD_BOOL(m_bDelayingActivate);
	LOAD_BOOL(m_bTimedTrigger);

	LOAD_TIME(m_fStartDelayTime);
	LOAD_FLOAT(m_fSendDelay);
	LOAD_TIME(m_fLastTouchTime);
	LOAD_FLOAT(m_fMinTriggerTime);
	LOAD_FLOAT(m_fMaxTriggerTime);
	LOAD_FLOAT(m_fTriggerDelay);
	LOAD_FLOAT(m_fSoundRadius);
	LOAD_INT(m_nNumActivations);

	LOAD_DWORD(m_nNumTimesActivated);
	LOAD_DWORD(m_nActivationCount);
	LOAD_DWORD(m_nCurrentActivation);

	LOAD_STDSTRING(m_sAIName);
	LOAD_STDSTRING(m_sActivationSound);

	LOAD_bool(m_bSendTriggerFXMsg);

	uint32 nSize;
	LOAD_DWORD( nSize );

	m_saCommands.clear();
	m_saCommands.resize( nSize );

	for( uint32 i=0; i < nSize; ++i )
	{
		LOAD_STDSTRING( m_saCommands[i] );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::CreateSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void Trigger::CreateSpecialFX( bool bUpdateClients /* =false */ )
{
	if( !m_pTriggerCS || !m_bSendTriggerFXMsg )
		return;

	m_pTriggerCS->bLocked	= !!m_bLocked;
	m_pTriggerCS->vDims		= m_vDims;

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SFX_TRIGGER_ID );
		m_pTriggerCS->Write( cMsg );
		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );
	}

	if( bUpdateClients )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_TRIGGER_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( TRIGFX_ALLFX_MSG );
		m_pTriggerCS->Write( cMsg );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::SendLockedMsg()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void Trigger::SendLockedMsg()
{
	if( !m_bSendTriggerFXMsg )
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_TRIGGER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( TRIGFX_LOCKED_MSG );
	cMsg.Writeuint8( m_bLocked );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::CreateTriggerCreateStruct()
//
//	PURPOSE:	Allocate the trigger create struct...
//
// ----------------------------------------------------------------------- //

void Trigger::CreateTriggerCreateStruct( )
{
	// Already allocated...
	if( m_pTriggerCS )
		return;

	m_pTriggerCS = debug_new(TRIGGERCREATESTRUCT);
}
