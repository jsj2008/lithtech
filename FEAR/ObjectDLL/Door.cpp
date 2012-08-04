// ----------------------------------------------------------------------- //
//
// MODULE  : DOOR.CPP
//
// PURPOSE : a Door object
//
// CREATED : 8/5/97 5:07:00 PM
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "Door.h"
	#include "ParsedMsg.h"
	#include "AIPathMgrNavMesh.h"
	#include "AINavMesh.h"
	#include "AINavMeshLinkDoor.h"
	#include "AIUtils.h"
	#include "CharacterMgr.h"
	#include "PrefetchUtilities.h"

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

	ADD_REALPROP_FLAG(Mass, DOOR_DEFAULT_MASS, PF_GROUP(1), "This value sets the mass of the object within the game.")
    ADD_BOOLPROP_FLAG(NeverDestroy, true, PF_GROUP(1), "Toggles whether the object can be destroyed.")

	// Override the options group...

	ADD_BOOLPROP_FLAG(PlayerActivate, true, PF_GROUP(3), "If TRUE the player can directly interact with this object by pressing use.")
	ADD_BOOLPROP_FLAG(AIActivate, true, PF_GROUP(3), "If TRUE this lets the AI know they can interact with this object.  When FALSE the AI will treat the object like it doesn't exist.")
	ADD_BOOLPROP_FLAG(StartOn, false, PF_GROUP(3) | PF_HIDDEN, "When set to TRUE the object will be turnning on as soon as the game loads.")
	ADD_BOOLPROP_FLAG(StartOpen, false, PF_GROUP(3), "When set to true the Door will be opening as soon as the game loads.")
	ADD_BOOLPROP_FLAG(TriggerOff, true, PF_GROUP(3) | PF_HIDDEN, "If this is set to FALSE the player can not directly turn the object off by pressing use.  The object can however be turned off by a message from another object like a switch.  If set to TRUE the player can directly turn off or close the object by pressing use.")
	ADD_BOOLPROP_FLAG(TriggerClose, true, PF_GROUP(3), "If this is set to FALSE the player can not directly close the Door by pressing use.  The Door can however be closed by a message from another object like a switch.  If set to TRUE the player can directly close the Door by pressing use.")
	ADD_BOOLPROP_FLAG(RemainOn, true, PF_GROUP(3) | PF_HIDDEN, "If this is FALSE the Object will start turnning itself off or close as soon as it turns on or opens.  If TRUE the object will stay on or open untill told to turn off, either by the player or a message.")
	ADD_BOOLPROP_FLAG(RemainOpen, true, PF_GROUP(3), "If set to FALSE the Door will try to start closing as soon as it becomes fully open.  If TRUE the Door will stay open untill told to close, either by the player or a message from another object.")
	ADD_BOOLPROP_FLAG(ForceMove, false, PF_GROUP(3), "If set to TRUE, the object will not be stopped by the player or AI.  It will continue on its path like there is nothing in the way.")
	ADD_BOOLPROP_FLAG(Locked, false, PF_GROUP(3), "When set to TRUE the object starts in a ""Locked"" state.  It can't be activated or triggered by a message unless it is unlocked.  To unlock an object send it an UNLOCK message.  To lock it again send a LOCK message." )
	ADD_BOOLPROP_FLAG(RotateAway, true, PF_GROUP(3) | PF_HIDDEN, "If set to TRUE RotatingWorldModels will rotate away from the player or AI that activated it.")
	ADD_BOOLPROP_FLAG(OpenAway, true, PF_GROUP(3), "If set to TRUE the RotatingDoor will open away from the player or AI that activated it.")
	ADD_STRINGPROP_FLAG(Waveform, "Linear", PF_STATICLIST | PF_GROUP(3), "A list of predefined wave types for movement.  Linear is a constant rate, objects will move at the same rate throught its whole path or rotation. SlowOn means the object starts moving slowly then picks up pace and will then stay constant.  SlowOff will move constantly at first but will slow down at the end of the movement or rotation.  Sine starts and ends slowly but looks very smooth and natural.")

	// Override the sounds group...

	ADD_STRINGPROP_FLAG(PowerOnSound, "", PF_FILENAME | PF_GROUP(4) | PF_HIDDEN, "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object starts to turn on.")
	ADD_STRINGPROP_FLAG(OpeningSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the Door starts opening.")
	ADD_STRINGPROP_FLAG(OnSound, "", PF_FILENAME | PF_GROUP(4) | PF_HIDDEN, "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object is fully turned on.")
	ADD_STRINGPROP_FLAG(OpenSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the Door is fully opened.")
	ADD_STRINGPROP_FLAG(PowerOffSound, "", PF_FILENAME | PF_GROUP(4) | PF_HIDDEN, "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object starts to turn off.")
	ADD_STRINGPROP_FLAG(ClosingSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the Door starts closing.")
	ADD_STRINGPROP_FLAG(OffSound, "", PF_FILENAME | PF_GROUP(4) | PF_HIDDEN, "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object is fully turned off.")
	ADD_STRINGPROP_FLAG(ClosedSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  This sound will begin playing as soon as the object is fully closed.")
	ADD_STRINGPROP_FLAG(LockedSound, "", PF_FILENAME | PF_GROUP(4), "The path of any .wav file can be entered here.  If this object is locked and the player or another object tries to activate it, this sound will begin to play.")
	ADD_VECTORPROP_VAL_FLAG(SoundPos, 0.0f, 0.0f, 0.0f, PF_GROUP(4), "The position that the sounds will be played from, relative to the objects position.  If this is (0.0 0.0 0.0) the sounds will play at the center of the object, (0.0 25.0 0.0) will place the sounds 25 units above the objects center.")
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS | PF_GROUP(4), "This is the extent of the sounds, they will not be heard beyond this distance from the SoundPos.")
	ADD_BOOLPROP_FLAG(LoopSounds, false, PF_GROUP(4), "When set to TRUE, the sounds will comtinue to loop untill they change state.  When FALSE the sounds will play their full length once and will then be destroyed.")
	
	// Override the commands group...

	ADD_COMMANDPROP_FLAG(OnCommand, "", PF_GROUP(5) | PF_HIDDEN, "A command that this object will execute when it is fully on.")
	ADD_COMMANDPROP_FLAG(OpenCommand, "", PF_GROUP(5)  | PF_NOTIFYCHANGE, "A command that this Door will execute when it is fully open.")
	ADD_COMMANDPROP_FLAG(OffCommand, "", PF_GROUP(5) | PF_HIDDEN, "A command that this object will execue when it is fully off.")
	ADD_COMMANDPROP_FLAG(ClosedCommand, "", PF_GROUP(5)  | PF_NOTIFYCHANGE, "A command that this Door will execue when it is fully closed.")
	ADD_COMMANDPROP_FLAG(PowerOnCommand, "", PF_GROUP(5) | PF_HIDDEN, "A command that this object will execute when starting to turn on.")
	ADD_COMMANDPROP_FLAG(OpeningCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that this Door will execute when starting to open.")
	ADD_COMMANDPROP_FLAG(PowerOffCommand, "", PF_GROUP(5) | PF_HIDDEN, "A command that this object will execute when starting to turn off.")
	ADD_COMMANDPROP_FLAG(ClosingCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that this Door will execute when starting to close.")
	ADD_COMMANDPROP_FLAG(LockedCommand, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "If this object is locked and the player or another object tries to activate it, this command will be executed.")

	// Override the movement properties...

	ADD_VECTORPROP_VAL_FLAG(MoveDir, 0.0f, 1.0f, 0.0f, 0, "This is the direction of movement this object will go when turned on.  This is relative to the objects local coordinates so a MoveDir of (0.0 1.0 0.0) will always move in the objects positive Y direction.")
	ADD_REALPROP_FLAG(MoveDist, 64.0f, 0, "This is the distance the object will move in the direction specified in MoveDir.")

	// Override the rotation properties...

	ADD_STRINGPROP_FLAG(RotationPoint, "", PF_OBJECTLINK, "The name of a ""Point"" object which this WorldModel will rotate around.  If no valid object name is given the WorldModel will rotate around the position of the bound object." )
	ADD_VECTORPROP_VAL_FLAG(RotationAngles, 0.0f, 90.0f, 0.0f, 0, "These represent how far the WorldModel will rotate around the RotationPoint in the specified axi when turned on.  (0.0 90.0 0.0) will rotate the WorldModel about the RotationPoint 90 degrees around the WorldModels local Y axis.")
	ADD_VECTORPROP_VAL_FLAG(StartingAngles, 0.0f, 0.0f, 0.0f, 0, "Specifes an optional initial rotation of the WorldModel.  These are in degrees relative to the rotation of the WorldModel when in the off state.  A StartingAngle of (0.0, 15.0, 0.0) will load the WorldModel with a rotation of 15 degrees around the WorldModels' local Y axis in the on direction." )
	
	// Override the common props...

	ADD_REALPROP_FLAG(PowerOnTime, 0.0f, PF_HIDDEN, "Sets the time in seconds for how long it takes the WorldModel to go from the Off state to the on state.")
	ADD_REALPROP_FLAG(OpeningTime, AWM_DEFAULT_POWERONTIME, 0, "Sets the time in seconds for how long it takes the WorldModel to go from the Off state to the on state." )
	ADD_REALPROP_FLAG(PowerOffSpeed, 0.0f, PF_HIDDEN, "TODO:PROPDESC")
	ADD_REALPROP_FLAG(ClosingTime, AWM_DEFAULT_POWEROFFTIME, 0, "If other than 0.0, sets the time in seconds for how long it takes the WorldModel to go from the On state to the off state.  If this is 0.0 then the OpeningTime value is used." )
	ADD_REALPROP_FLAG(MoveDelay, AWM_DEFAULT_MOVEDELAY, 0, "Amount of delay in seconds between the time the WorldModel is triggered and when it begins its movement.")
	ADD_REALPROP_FLAG(OnWaitTime, 0.0f, PF_HIDDEN, "Amount of time in seconds that the WorldModel will remain on before turnning off automatically, and the amount of time before the WorldModel can be triggered on again." )
	ADD_REALPROP_FLAG(OpenWaitTime, AWM_DEFAULT_ONWAITTIME, 0, "Amount of time in seconds that the Door will remain open before automatically closing, and the amount of time before the Door can be triggered again." )
	ADD_REALPROP_FLAG(OffWaitTime, 0.0f, PF_HIDDEN, "Amount of time in secomds before the WorldModel can be turned on after being turned off.")
	ADD_REALPROP_FLAG(CloseWaitTime, AWM_DEFAULT_OFFWAITTIME, 0, "Amount of time in secomds before the Door can be opened after being closed.")

	// New props...

	ADD_STRINGPROP_FLAG(DoorLink, "", PF_OBJECTLINK, "Use this to link two Doors together that you would like to open at the same time (i.e., Door1's DoorLink would be Door2, and Door2's DoorLink would be Door1).  NOTE--Only the opening of linked doors is supported. Each door may be closed individually.")

	ADD_STRINGPROP(Sector1, "", "The brush name of a sector that will be turned on and off by this door as it opens and closes.")
	ADD_STRINGPROP(Sector2, "", "The brush name of a sector that will be turned on and off by this door as it opens and closes.")

	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PREFETCH(Door, ActiveWorldModel, CF_HIDDEN | CF_WORLDMODEL, DefaultPrefetch<Door>, "This object is used to define the behavior of a world model as a door")

//
// Register the calss with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Door )

	ADD_MESSAGE( CLOSE, 1, NULL, MSG_HANDLER( Door, HandleCloseMsg ), "CLOSE", "TODO:CMDDESC", "TODO:CMDEXP" )

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
	m_sDoorLink( ),
	m_hDoorLink( NULL ),
	m_bSectorsActive( true ),
	m_eNMLinkID( kNMLink_Invalid )
{
	for(uint32 nCurrSector = 0; nCurrSector < knNumSectors; nCurrSector++)
		m_nSectorID[nCurrSector] = 0; 
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

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::ReadProps
//
//  PURPOSE:	Read in property values
//
// ----------------------------------------------------------------------- //

void Door::ReadProps( const GenericPropList *pProps )
{
	LTASSERT( pProps != NULL, "Invalid ObjectCreateStruct!" );

	// Read base class props first

	ActiveWorldModel::ReadProps( pProps );

		
	// Develop Property option flags...

	pProps->GetBool( "StartOpen", false ) ? m_dwPropFlags |= AWM_PROP_STARTON : m_dwPropFlags &= ~AWM_PROP_STARTON;
	pProps->GetBool( "TriggerClose", true ) ? m_dwPropFlags |= AWM_PROP_TRIGGEROFF : m_dwPropFlags &= ~AWM_PROP_TRIGGEROFF;
	pProps->GetBool( "RemainOpen", true ) ? m_dwPropFlags |= AWM_PROP_REMAINON : m_dwPropFlags &= ~AWM_PROP_REMAINON;
	pProps->GetBool( "OpenAway", true ) ? m_dwPropFlags |= AWM_PROP_ROTATEAWAY : m_dwPropFlags &= ~AWM_PROP_ROTATEAWAY;
	
	// Read sound options...

	m_sPowerOnSnd	= pProps->GetString( "OpeningSound", "" );
	m_sOnSnd		= pProps->GetString( "OpenSound", "" );
	m_sPowerOffSnd	= pProps->GetString( "ClosingSound", "" );
	m_sOffSnd		= pProps->GetString( "ClosedSound", "" );

	// Read Commands to send...
	
	m_sOnCmd		= pProps->GetCommand( "OpenCommand", "" );
	m_sOffCmd		= pProps->GetCommand( "ClosedCommand", "" );
	m_sPowerOnCmd	= pProps->GetCommand( "OpeningCommand", "" );
	m_sPowerOffCmd	= pProps->GetCommand( "ClosingCommand", "" );

	m_fPowerOnTime	= pProps->GetReal( "OpeningTime", m_fPowerOnTime );
	m_fPowerOffTime	= pProps->GetReal( "ClosingTime", m_fPowerOffTime );
	m_fOnWaitTm		= pProps->GetReal( "OpenWaitTime", m_fOnWaitTm );
	m_fOffWaitTm	= pProps->GetReal( "CloseWaitTime", m_fOffWaitTm );

	// Read the door link object...
	m_sDoorLink = pProps->GetString( "DoorLink", "" );

	// Read the sector info
	m_sSectorName[0] = pProps->GetString( "Sector1", "" );
	m_sSectorName[1] = pProps->GetString( "Sector2", "" );
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

	if( !m_sDoorLink.empty() )
	{
		ObjArray<HOBJECT, 1>	objArray;
		g_pLTServer->FindNamedObjects( m_sDoorLink.c_str(), objArray );

		if( objArray.NumObjects() > 0 )
		{
			m_hDoorLink = objArray.GetObject( 0 );
		}
	}

	return 1;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::HandleCloseMsg
//
//  PURPOSE:	Handle a CLOSE message...
//
// ----------------------------------------------------------------------- //

void Door::HandleCloseMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	if( m_nCurState == AWM_STATE_ON || m_nCurState == AWM_STATE_POWERON ||
		m_nCurState == AWM_STATE_INITIAL || m_nCurState == AWM_STATE_ROTATE )
	{
		if( g_pLTServer->GetTime() > m_fMoveStopTm + m_fOnWaitTm )
		{
			SetPowerOff( m_fPowerOffTime, m_nInitWaveform );
		}
	}
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
		if( g_pCharacterMgr->FindCharactersWithinRadius( NULL, vPos, 130.f, NULL, CCharacterMgr::kList_AIs ) )
		{
			return;
		}
	}

	// Normal activation handling.

	ActiveWorldModel::Activate( hObj );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::SetStartingAngles
//
//  PURPOSE:	Override to ensure sectors are turned on/off appropriately.
//
// ----------------------------------------------------------------------- //

void Door::SetStartingAngles( const LTVector& vStartingAngles )
{
	Super::SetStartingAngles( vStartingAngles );

	// Only need to update the sectors if actually in the initial rotation state.
	if( m_nCurState == AWM_STATE_INITIAL )
		UpdateSector( true );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::UpdateSector
//
//  PURPOSE:	Updates the sector state
//
// ----------------------------------------------------------------------- //

void Door::UpdateSector( bool bSectorActive )
{
	for(uint32 nCurrSector = 0; nCurrSector < knNumSectors; nCurrSector++)
	{
		// Do nothing if we don't have a sector
		if( m_sSectorName[nCurrSector].empty() )
			return;

		// Just in case we get in here before we expect to, update the sector ID..
		if( !m_nSectorID[nCurrSector] )
		{
			if( g_pLTServer->GetSectorID( m_sSectorName[nCurrSector].c_str(), &m_nSectorID[nCurrSector] ) != LT_OK )
			{
				// If we can't find the sector, dump a warning and forget...
				char aNameBuff[256];
				g_pLTServer->GetObjectName( m_hObject, aNameBuff, LTARRAYSIZE(aNameBuff) );
				g_pLTServer->CPrint( "Invalid sector specified in door %s: %s", aNameBuff, m_sSectorName[nCurrSector].c_str() );
				m_sSectorName[nCurrSector].clear();
				return;
			}
		}

		m_bSectorsActive = bSectorActive;

		// Send the dynamic sector message to all connected clients...
		// This is temporary until we get the new networking architecture in place.
		// We can't do it with an FX object because activeworldmodels already have an FX object...
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_DYNAMIC_SECTOR );
		cMsg.Writeuint32( m_nSectorID[nCurrSector] );
		cMsg.Writebool( bSectorActive );

		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::ToggleState
//
//  PURPOSE:	Toggles the state of the door...
//
// ----------------------------------------------------------------------- //

void Door::ToggleState( float fTime /*= -1.0f*/, uint8 nWaveform /*= (uint8)-1*/ )
{
	// Send to an internal method

	HandelLinkToggleState( true );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::Lock
//
//  PURPOSE:	Handle a LOCK or UNLOCK command.
//
// ----------------------------------------------------------------------- //

void Door::Lock(bool bLock)
{
	// Send to base class first...

	ActiveWorldModel::Lock( bLock );

	// All AIs need to clear existing knowledge of paths,
	// because locking and unlocking doors changes the connectivity.

	g_pAIPathMgrNavMesh->InvalidatePathKnowledge( m_hObject );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::HandelLinkToggleState
//
//  PURPOSE:	Does the actual handeling for toggling state and takes care of the link
//
// ----------------------------------------------------------------------- //

void Door::HandelLinkToggleState( bool bTriggerLink, float fTime /* = -1.0f */, uint8 nWaveform /* = (uint8)-1 */ )
{
	// Let our base hadle it first

	ActiveWorldModel::ToggleState( fTime, nWaveform );

	// If we are locked then dont do anything else

	if( m_dwPropFlags & AWM_PROP_LOCKED )
	{

		return;
	}

	if( bTriggerLink )
	{
		// Let our link know that we are opening...
		// AIs ignore links and open both doors.

		if( m_nCurState == AWM_STATE_POWERON )
		{
			if( m_hDoorLink  )
			{
				Door *pDoorLink = (Door*)g_pLTServer->HandleToObject( m_hDoorLink );
				if( pDoorLink )
				{
					pDoorLink->ToggleLink( m_hActivateObj, fTime, nWaveform );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::ToggleLink
//
//  PURPOSE:	The Door that has us as its link toggled us
//
// ----------------------------------------------------------------------- //

void Door::ToggleLink( HOBJECT hActivateObj, float fTime /* = -1.0f */, uint8 nWaveform /* = (uint8)-1 */ )
{
	// Save the object that ativated us
	
	SetActiveObj( hActivateObj );

	// Only activate if we are closed or closing

	if( m_nCurState == AWM_STATE_OFF || m_nCurState == AWM_STATE_POWEROFF )
	{
		HandelLinkToggleState( false, fTime, nWaveform );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::SetPowerOn
//
//  PURPOSE:	Start opening the door
//
// ----------------------------------------------------------------------- //

void Door::SetPowerOn( double fTime, uint8 nWaveform )
{
	// Let base class handle it first..

	ActiveWorldModel::SetPowerOn( fTime, nWaveform );

	// Turn on the sector
	UpdateSector(true);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::SetPowerOff
//
//  PURPOSE:	Start closing the door
//
// ----------------------------------------------------------------------- //

void Door::SetPowerOff( double fTime, uint8 nWaveform )
{
	// Let base class handle it first...

	ActiveWorldModel::SetPowerOff( fTime, nWaveform );
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::SetOff
//
//  PURPOSE:	Finish closing the door
//
// ----------------------------------------------------------------------- //

void Door::SetOff( bool bInitialState )
{
	// Let base class handle it first...

	ActiveWorldModel::SetOff( bInitialState );

	// Turn off the sector
	UpdateSector(false);

	// Let link handle it.

	if( m_eNMLinkID != kNMLink_Invalid )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_eNMLinkID );
		if( pLink && pLink->GetNMLinkType() == kLink_Door )
		{
			AINavMeshLinkDoor* pLinkDoor = (AINavMeshLinkDoor*)pLink;
			pLinkDoor->HandleDoorClosed( this );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::SetOn
//
//  PURPOSE:	Finish opening the door
//
// ----------------------------------------------------------------------- //

void Door::SetOn( bool bInitialState )
{
	// Let base class handle it first...

	ActiveWorldModel::SetOn( bInitialState );

	// Let link handle it.

	if( m_eNMLinkID != kNMLink_Invalid )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( m_eNMLinkID );
		if( pLink && pLink->GetNMLinkType() == kLink_Door )
		{
			AINavMeshLinkDoor* pLinkDoor = (AINavMeshLinkDoor*)pLink;
			pLinkDoor->HandleDoorOpened( this );
		}
	}
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

	SAVE_STDSTRING( m_sDoorLink );
	SAVE_bool( m_bSectorsActive );
	SAVE_DWORD( m_eNMLinkID );

	for(uint32 nCurrSector = 0; nCurrSector < knNumSectors; nCurrSector++)
		SAVE_STDSTRING( m_sSectorName[nCurrSector] );

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

	LOAD_STDSTRING( m_sDoorLink );
	LOAD_bool( m_bSectorsActive );
	LOAD_DWORD_CAST( m_eNMLinkID, ENUM_NMLinkID );

	for(uint32 nCurrSector = 0; nCurrSector < knNumSectors; nCurrSector++)
		LOAD_STDSTRING( m_sSectorName[nCurrSector] );

	UpdateSector( m_bSectorsActive );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Door::IsLockedForCharacter
//
//  PURPOSE:	Can the passed in character open the door
//
// ----------------------------------------------------------------------- //

bool Door::IsLockedForCharacter( HOBJECT /*hChar*/ ) const
{
	if( !IsLocked() )
		return false;

	// If it is locked check to see if they have the keys to open it

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::GetPrefetchResourceList
//
//	PURPOSE:	Determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void Door::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// get the world model sounds
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "OpeningSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "OpenSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "ClosingSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "ClosedSound");
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "LockedSound");
}
