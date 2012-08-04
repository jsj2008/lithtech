// ----------------------------------------------------------------------- //
//
// MODULE  : DOOR.CPP
//
// PURPOSE : a Door object
//
// CREATED : 8/5/97 5:07:00 PM
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "Door.h"
	#include "ParsedMsg.h"
	#include "KeyMgr.h"
	#include "AIUtils.h"
	#include "AIPathMgr.h"
	#include "DoorKnob.h"
	#include "GadgetTarget.h"
	#include "CharacterMgr.h"

//
// Defines....
//

	#define UPDATE_DELTA					0.01f

	#define	DOOR_DEFAULT_BLOCKING_PRIORITY	255
	#define DOOR_DEFAULT_MASS				10000.0f

	#define DOOR_DEFAULT_ACTIVATE_TYPE		"OpenClose"

LINKFROM_MODULE( Door );

//
// Add props...
//

BEGIN_CLASS( Door )

	// Set AWM Type

	AWM_SET_TYPE_STATIC

	// Overrides...

	ADD_REALPROP_FLAG(Mass, DOOR_DEFAULT_MASS, PF_GROUP(1))
    ADD_BOOLPROP_FLAG(NeverDestroy, LTTRUE, PF_GROUP(1))

	// Override the options group...

	ADD_BOOLPROP_FLAG(PlayerActivate, LTTRUE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(AIActivate, LTTRUE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(StartOn, LTFALSE, PF_GROUP(3) | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(StartOpen, LTFALSE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(TriggerOff, LTTRUE, PF_GROUP(3) | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(TriggerClose, LTTRUE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(RemainOn, LTTRUE, PF_GROUP(3) | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(RemainOpen, LTTRUE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(ForceMove, LTFALSE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(Locked, LTFALSE, PF_GROUP(3))
	ADD_BOOLPROP_FLAG(RotateAway, LTTRUE, PF_GROUP(3) | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(OpenAway, LTTRUE, PF_GROUP(3))
	ADD_STRINGPROP_FLAG(Waveform, "Linear", PF_STATICLIST | PF_GROUP(3))
	ADD_STRINGPROP_FLAG(ActivateType, DOOR_DEFAULT_ACTIVATE_TYPE, PF_STATICLIST | PF_GROUP(3))

	// Override the sounds group...

	ADD_STRINGPROP_FLAG(PowerOnSound, "", PF_FILENAME | PF_GROUP(4) | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(OpeningSound, "", PF_FILENAME | PF_GROUP(4))
	ADD_STRINGPROP_FLAG(OnSound, "", PF_FILENAME | PF_GROUP(4) | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(OpenSound, "", PF_FILENAME | PF_GROUP(4))
	ADD_STRINGPROP_FLAG(PowerOffSound, "", PF_FILENAME | PF_GROUP(4) | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ClosingSound, "", PF_FILENAME | PF_GROUP(4))
	ADD_STRINGPROP_FLAG(OffSound, "", PF_FILENAME | PF_GROUP(4) | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ClosedSound, "", PF_FILENAME | PF_GROUP(4))
	ADD_STRINGPROP_FLAG(LockedSound, "", PF_FILENAME | PF_GROUP(4))
	ADD_VECTORPROP_VAL_FLAG(SoundPos, 0.0f, 0.0f, 0.0f, PF_GROUP(4))
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS | PF_GROUP(4))
	ADD_BOOLPROP_FLAG(LoopSounds, LTFALSE, PF_GROUP(4))
	
	// Override the commands group...

	ADD_STRINGPROP_FLAG(OnCommand, "", PF_GROUP(5) | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(OpenCommand, "", PF_GROUP(5)  | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(OffCommand, "", PF_GROUP(5) | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ClosedCommand, "", PF_GROUP(5)  | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(PowerOnCommand, "", PF_GROUP(5) | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(OpeningCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(PowerOffCommand, "", PF_GROUP(5) | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ClosingCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(LockedCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE)

	// Override the movement properties...

	ADD_VECTORPROP_VAL_FLAG(MoveDir, 0.0f, 1.0f, 0.0f, 0)
	ADD_REALPROP_FLAG(MoveDist, 64.0f, 0)

	// Override the rotation properties...

	ADD_STRINGPROP_FLAG(RotationPoint, "", PF_OBJECTLINK)
	ADD_VECTORPROP_VAL_FLAG(RotationAngles, 0.0f, 90.0f, 0.0f, 0)
	
	// Override the common props...

	ADD_REALPROP_FLAG(PowerOnTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(OpeningTime, AWM_DEFAULT_POWERONTIME, 0 )
	ADD_REALPROP_FLAG(PowerOffSpeed, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(ClosingTime, AWM_DEFAULT_POWEROFFTIME, 0)
	ADD_REALPROP_FLAG(MoveDelay, AWM_DEFAULT_MOVEDELAY, 0)
	ADD_REALPROP_FLAG(OnWaitTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(OpenWaitTime, AWM_DEFAULT_ONWAITTIME, 0)
	ADD_REALPROP_FLAG(OffWaitTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(CloseWaitTime, AWM_DEFAULT_OFFWAITTIME, 0)

	// New props...

	ADD_STRINGPROP_FLAG(DoorLink, "", PF_OBJECTLINK)

END_CLASS_DEFAULT_FLAGS(Door, ActiveWorldModel, NULL, NULL, CF_HIDDEN | CF_WORLDMODEL)

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Door )

	CMDMGR_ADD_MSG( TRIGGERCLOSE, 1, NULL, "TRIGGERCLOSE" )

CMDMGR_END_REGISTER_CLASS( Door, ActiveWorldModel )


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::Door
//
//  PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Door::Door( )
:	ActiveWorldModel( ),
	m_hstrDoorLink( LTNULL ),
	m_hDoorLink( LTNULL ),
	m_hAIUser( LTNULL )
{
 
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::~Door
//
//  PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

Door::~Door( )
{
	// Freea our strings...

	FREE_HSTRING( m_hstrDoorLink );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::ReadProps
//
//  PURPOSE:	Read in property values
//
// ----------------------------------------------------------------------- //

void Door::ReadProps( ObjectCreateStruct *pOCS )
{
	_ASSERT( pOCS != LTNULL );

	bool		bFlag;
	GenericProp	GenProp;

	// Read base class props first

	ActiveWorldModel::ReadProps( pOCS );

		
	// Develop Property option flags...

	g_pLTServer->GetPropBool( "StartOpen", &bFlag );
	bFlag ? m_dwPropFlags |= AWM_PROP_STARTON : m_dwPropFlags &= ~AWM_PROP_STARTON;

	g_pLTServer->GetPropBool( "TriggerClose", &bFlag );
	bFlag ? m_dwPropFlags |= AWM_PROP_TRIGGEROFF : m_dwPropFlags &= ~AWM_PROP_TRIGGEROFF;

	g_pLTServer->GetPropBool( "RemainOpen", &bFlag );
	bFlag ? m_dwPropFlags |= AWM_PROP_REMAINON : m_dwPropFlags &= ~AWM_PROP_REMAINON;

	g_pLTServer->GetPropBool( "OpenAway", &bFlag );
	bFlag ? m_dwPropFlags |= AWM_PROP_ROTATEAWAY : m_dwPropFlags &= ~AWM_PROP_ROTATEAWAY;
	
	// Read sound options...

	if( g_pLTServer->GetPropGeneric( "OpeningSound", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrPowerOnSnd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "OpenSound", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrOnSnd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "ClosingSound", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrPowerOffSnd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "ClosedSound", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrOffSnd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}


	// Read Commands to send...

	if( g_pLTServer->GetPropGeneric( "OpenCommand", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrOnCmd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "ClosedCommand", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrOffCmd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "OpeningCommand", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrPowerOnCmd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "ClosingCommand", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrPowerOffCmd = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	g_pLTServer->GetPropReal( "OpeningTime", &m_fPowerOnTime );
	g_pLTServer->GetPropReal( "ClosingTime", &m_fPowerOffTime );
	g_pLTServer->GetPropReal( "OpenWaitTime", &m_fOnWaitTm );
	g_pLTServer->GetPropReal( "CloseWaitTime", &m_fOffWaitTm );

	// Read the door link object...

	if( g_pLTServer->GetPropGeneric( "DoorLink", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrDoorLink = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	// Get the activate type...
	
	if( m_dwPropFlags & AWM_PROP_PLAYERACTIVATE )
	{
		if( g_pLTServer->GetPropGeneric( "ActivateType", &GenProp ) == LT_OK )
		{
			if( GenProp.m_String[0] )
			{
				m_ActivateTypeHandler.SetActivateType( GenProp.m_String );
			}
		}
		else
		{
			// If the property wasn't found use the default...
			
			m_ActivateTypeHandler.SetActivateType( DOOR_DEFAULT_ACTIVATE_TYPE );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::OnAllObjectsCreated
//
//  PURPOSE:	Now that all objects are loaded we can look for our Door link object
//
// ----------------------------------------------------------------------- //

uint32 Door::OnAllObjectsCreated( )
{
	// Send to base class first...

	ActiveWorldModel::OnAllObjectsCreated( );

	// Create a link to our door link object if we have one...

	if( m_hstrDoorLink )
	{
		ObjArray<HOBJECT, 1>	objArray;
		g_pLTServer->FindNamedObjects( g_pLTServer->GetStringData( m_hstrDoorLink ), objArray );

		if( objArray.NumObjects() > 0 )
		{
			m_hDoorLink = objArray.GetObject( 0 );
		}
	}

	return 1;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::OnTrigger
//
//  PURPOSE:	Handle recieving a trigger msg from another object
//
// ----------------------------------------------------------------------- //

bool Door::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_TriggerClose("TRIGGERCLOSE");

	// Let the base class handle the message first

	bool bResult = ActiveWorldModel::OnTrigger( hSender, cMsg );

	if( cMsg.GetArg(0) == s_cTok_TriggerClose )
	{
		TriggerClose();
	}
	else
		return bResult;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::Activate
//
//  PURPOSE:	Handle recieving an activate msg from another object
//
// ----------------------------------------------------------------------- //

void Door::Activate( HOBJECT hObj )
{
	// Do not allow anyone to close a door if AI are near by.
	// This is necessary because AI are non-solid, and will not 
	// stop the door themselves.

	if( ( m_nCurState == DOORSTATE_OPEN ) ||
		( m_nCurState == DOORSTATE_OPENING ) )
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
		if( g_pCharacterMgr->FindCharactersWithinRadius( LTNULL, vPos, 64.f, CCharacterMgr::kList_AIs ) )
		{
			return;
		}
	}

	// Normal activation handling.

	ActiveWorldModel::Activate( hObj );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::HandleTriggerMsg
//
//  PURPOSE:	Handles a trigger command
//
// ----------------------------------------------------------------------- //

void Door::HandleTriggerMsg( )
{
	// Send to an internal method

	HandelLinkTriggerMsg( LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::HandleLock
//
//  PURPOSE:	Handle a LOCK or UNLOCK command.
//
// ----------------------------------------------------------------------- //

void Door::HandleLock(LTBOOL bLock)
{
	// Send to base class first...

	ActiveWorldModel::HandleLock( bLock );

	// All AIs need to clear existing knowledge of paths,
	// because locking and unlocking doors changes the connectivity.

	g_pAIPathMgr->InvalidatePathKnowledge();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::HandelLinkTriggerMsg
//
//  PURPOSE:	Does the actual handeling of a trigger message and takes care of the link
//
// ----------------------------------------------------------------------- //

void Door::HandelLinkTriggerMsg( LTBOOL bTriggerLink )
{
	// Let our base hadle it first

	ActiveWorldModel::HandleTriggerMsg( );

	// If we are locked then dont do anything else

	if( m_dwPropFlags & AWM_PROP_LOCKED )
	{
		if( m_nCurState == AWM_STATE_OFF || m_nCurState == AWM_STATE_ON )
		{
			// Tell any doorknobs attached to us to play the "locked" animation...

			PlayDoorKnobAni( "Locked" );
		}

		return;
	}

	if( bTriggerLink )
	{
		// Let our link know that we are opening...
		// AIs ignore links and open both doors.

		if( m_nCurState == AWM_STATE_POWERON )
		{
			if( m_hDoorLink && !IsAI( m_hActivateObj ) )
			{
				Door *pDoorLink = (Door*)g_pLTServer->HandleToObject( m_hDoorLink );
				if( pDoorLink )
				{
					pDoorLink->TriggerLink( m_hActivateObj );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::TriggerLink
//
//  PURPOSE:	The Door that has us as its link triggered us
//
// ----------------------------------------------------------------------- //

void Door::TriggerLink( HOBJECT hActivateObj )
{
	// Save the object that ativated us
	
	SetActiveObj( hActivateObj );

	// Only activate if we are closed or closing

	if( m_nCurState == AWM_STATE_OFF || m_nCurState == AWM_STATE_POWEROFF )
	{
		HandelLinkTriggerMsg( LTFALSE );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::TriggerClose
//
//  PURPOSE:	We were triggered close
//
// ----------------------------------------------------------------------- //

void Door::TriggerClose( )
{
	if( m_nCurState == AWM_STATE_ON || m_nCurState == AWM_STATE_POWERON )
	{
		if( g_pLTServer->GetTime() > m_fMoveStopTm + m_fOnWaitTm )
		{
			SetPowerOff();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::SetPowerOn
//
//  PURPOSE:	Start opening the door
//
// ----------------------------------------------------------------------- //

void Door::SetPowerOn( )
{
	// Let base class handle it first..

	ActiveWorldModel::SetPowerOn( );

	PlayDoorKnobAni( "Open" );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::SetPowerOff
//
//  PURPOSE:	Start closing the door
//
// ----------------------------------------------------------------------- //

void Door::SetPowerOff( )
{
	// Let base class handle it first...

	ActiveWorldModel::SetPowerOff( );
	
	PlayDoorKnobAni( "Open" );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::OnSave
//
//  PURPOSE:	Save the Door
//
// ----------------------------------------------------------------------- //

void Door::OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg ) return;

	// Save base vars first

	ActiveWorldModel::OnSave( pMsg, dwSaveFlags );

	SAVE_HSTRING( m_hstrDoorLink );
	SAVE_HOBJECT( m_hAIUser );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::OnLoad
//
//  PURPOSE:	Load the Door
//
// ----------------------------------------------------------------------- //

void Door::OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg ) return;

	// Load base vars first

	ActiveWorldModel::OnLoad( pMsg, dwSaveFlags );

	LOAD_HSTRING( m_hstrDoorLink );
	LOAD_HOBJECT( m_hAIUser );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::PlayDoorKnobAni
//
//  PURPOSE:	Tell all door knobs attached to us to play the animation
//
// ----------------------------------------------------------------------- //

void Door::PlayDoorKnobAni( char *pAniName )
{
	if( !pAniName ) return;

	// Look for all door knobs attached to us...

	for( ObjRefNotifierList::iterator iter = m_AttachmentList.begin( ); iter != m_AttachmentList.end( ); iter++ )
	{
		bool bCanPlay = false;
		HOBJECT hObj = *iter;

		DoorKnob* pDoorKnob = dynamic_cast< DoorKnob* >( g_pLTServer->HandleToObject( hObj ));
		if( pDoorKnob )
		{
			bCanPlay = true;
		}
		else
		{
			// No DoorKnob object look for a door knob GadgetTarget...

			GadgetTarget *pGT = dynamic_cast<GadgetTarget*>(g_pLTServer->HandleToObject( hObj ));
			if( pGT )
			{
				if( pGT->GetType() == eDoorKnob )
				{
					bCanPlay = true;
				}
			}
		}

		if( bCanPlay )
		{
			HMODELANIM	hAnim = g_pLTServer->GetAnimIndex( hObj, pAniName );
			if( hAnim != INVALID_MODEL_ANIM )
			{
				g_pLTServer->SetModelLooping( hObj, LTFALSE );
				g_pLTServer->SetModelAnimation( hObj, hAnim );
				g_pLTServer->ResetModelAnimation( hObj );
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::IsLockedForCharacter
//
//  PURPOSE:	Can the passed in character open the door
//
// ----------------------------------------------------------------------- //

LTBOOL Door::IsLockedForCharacter( HOBJECT hChar ) const
{
	if( !IsLocked() )
		return LTFALSE;

	// If it is locked check to see if they have the keys to open it

	return !g_pKeyMgr->CanCharacterControlObject( hChar, m_hObject );
}
