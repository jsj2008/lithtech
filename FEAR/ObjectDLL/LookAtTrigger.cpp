// ----------------------------------------------------------------------- //
//
// MODULE  : LookAtTrigger.cpp
//
// PURPOSE : Trigger that executes commands based on a players view...
//
// CREATED : 3/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "LookAtTrigger.h"
#include "PlayerObj.h"
#include "SurfaceDB.h"
#include "SurfaceFunctions.h"

LINKFROM_MODULE( LookAtTrigger );


BEGIN_CLASS( LookAtTrigger )

	ADD_REALPROP_FLAG( ActivationRadius, 1000.0f, PF_RADIUS, "Specifies the distance, from the center of the object, that a player must be in order for them to be considered looking at the object." )
	
	// LookAt properties...
	ADD_LONGINTPROP( LookAtsMax, 1, "This value specifies the number of times the object will execute its LookAt command.  Set this value to -1 for infinite number of lookats." )
	ADD_BOOLPROP( LookAtLocked, false, "When LookAt is locked the LookAt properties will be ignored while still allowing the LookAway state to be activated." )
	ADD_REALPROP( LookAtThreshold, 0.0f, "Amount of time, in seconds, a player must be looking at the object before they are considered looking at it.  Set to 0.0 for immediate execution of LookAt command." )
	ADD_COMMANDPROP_FLAG( LookAtCommand, "", PF_NOTIFYCHANGE, "This command will be executed when a player is considered looking at the object." )

	// LookAway properties...
	ADD_LONGINTPROP( LookAwaysMax, 1, "This value specifies the number of times the object will execute its LookAway command.  Set this value to -1 for infinite number of lookaways." )
	ADD_BOOLPROP( LookAwayLocked, false, "When LookAway is locked the LookAway properties will be ignored while still allowing the LookAt state to be activated." )
	ADD_REALPROP( LookAwayThreshold, 0.0f, "Amount of time, in seconds, a player must be looking away from the object before they are considered looking away from it.  Set to 0.0 for immediate execution of LookAway command." )
	ADD_COMMANDPROP_FLAG( LookAwayCommand, "", PF_NOTIFYCHANGE, "This command will be executed when a player is considered looking away from the object." )

END_CLASS_FLAGS_PLUGIN( LookAtTrigger, GameBase, CF_WORLDMODEL, CLookAtTriggerPlugin, "Binding brushes to this object creates a space that will execute commands when looked at." )

static bool ValidateMsgLockUnlock( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_nArgs < 2 )
		return true;

	if( (LTStrIEquals( cpMsgParams.m_Args[1], "LookAt" )) ||
		(LTStrIEquals( cpMsgParams.m_Args[1], "LookAway" )) )
	{
		return true;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateMsgLockUnlock()" );
		pInterface->CPrint( "    MSG - %s - 2nd argument '%s' is not a valid bool value.", LTStrUpr(cpMsgParams.m_Args[0]), cpMsgParams.m_Args[1] );
	}

	return false;
}

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( LookAtTrigger )

	ADD_MESSAGE_ARG_RANGE( LOCK, 1, 2, ValidateMsgLockUnlock, MSG_HANDLER( LookAtTrigger, HandleLockMsg ), "LOCK <..., LookAt, LookAway>", "Locks the entire Trigger or individual components so that it ignores all commands other than UNLOCK and REMOVE and cannot be activated by the player", "msg Trigger LOCK;<BR>msg Trigger (LOCK LookAt);<BR>msg Trigger (LOCK LookAway);")
	ADD_MESSAGE_ARG_RANGE( UNLOCK, 1, 2, ValidateMsgLockUnlock, MSG_HANDLER( LookAtTrigger, HandleUnlockMsg ), "UNLOCK <...,, LookAt, LookAway>", "Unlocks the entire trigger or individual components so that the player or other messages can activate it again", "msg Trigger UNLOCK;<BR>msg Trigger (UNLOCK LookAt);<BR>msg Trigger (UNLOCK LookAway);" )
	ADD_MESSAGE( LOOKAT, 1, NULL, MSG_HANDLER( LookAtTrigger, HandleLookAtMsg ), "LOOKAT", "Sets the object to it�s LookAt state, ignoring threshold, executing the LookAt command and incrementing the number of lookats", "msg Trigger LOOKAT;" )
	ADD_MESSAGE( LOOKAWAY, 1, NULL, MSG_HANDLER( LookAtTrigger, HandleLookAwayMsg ), "LOOKAWAY", "Sets the object to it�s LookAway state, ignoring threshold, executing the LookAway", "msg Trigger LOOKAWAY;" )

CMDMGR_END_REGISTER_CLASS( LookAtTrigger, GameBase )


//
// LookAtTrigger plugin implementation...
//

LTRESULT CLookAtTriggerPlugin::PreHook_PropChanged( const char *szObjName,
												    const char *szPropName,
													const int nPropType,
													const GenericProp &gpPropValue,
													ILTPreInterface *pInterface,
													const char *szModifiers )
{
	// Let the command manager verify the commands...
	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, szPropName, nPropType, gpPropValue, pInterface, szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

static bool InvisibleSurfaceFilterFn( HPOLY hPoly, void *pUserData, const LTVector& vIntersectPoint)
{
	// Make sure we hit a surface type we care about...
	SurfaceType eSurfaceType = GetSurfaceType(hPoly);

	HSURFACE hSurface = g_pSurfaceDB->GetSurface( eSurfaceType );

	if( g_pSurfaceDB->GetBool( hSurface, SrfDB_Srf_bCanSeeThrough ))
		return false;

	return true;
}

static bool LookAwayFilterFn( HOBJECT hTest, void *pUserData )
{
	if( !IsWorldModel( hTest ) && !IsMainWorld( hTest ) && !IsKindOf( hTest, "LookAtTrigger" ))
		return false;

	// Filters out objects for a raycast.  pUserData is a list of HOBJECTS terminated
	// with a NULL HOBJECT.
	HOBJECT *hList = (HOBJECT*)pUserData;
	while(hList && *hList)
	{
		if(hTest == *hList)
            return false;
		++hList;
	}

	// The main world or a world model not in the filter list...
    return true;	
}

//
// LookAtTrigger class implementation...
// 

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::LookAtTrigger
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

LookAtTrigger::LookAtTrigger( )
:	GameBase				( OT_WORLDMODEL ),
	m_fActivationRadiusSqr	( 0.0f ),
	m_nLookAtsMax			( 0 ),
	m_nLookAtsNum			( 0 ),
	m_bLookAtsLocked		( false ),
	m_fLookAtThreshold		( 0.0f ),
	m_fLookAtTime			( 0.0f ),
	m_sLookAtCommand		( ),
	m_nLookAwaysMax			( 0 ),
	m_nLookAwaysNum			( 0 ),
	m_bLookAwaysLocked		( false ),
	m_fLookAwayThreshold	( 0.0f ),
	m_fLookAwayTime			( 0.0f ),
	m_sLookAwayCommand		( ),
	m_hLookAtPlayer			( NULL ),
	m_eState				( eStateNormal )
{ 
	m_hLookAtPlayer.SetReceiver( *this );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::~LookAtTrigger
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

LookAtTrigger::~LookAtTrigger( )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::EngineMessageFn
//
//  PURPOSE:	Handel messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 LookAtTrigger::EngineMessageFn(  uint32 messageID, void *pData, float fData )
{
	switch( messageID )
	{
		case MID_PRECREATE:
		{
			// Let the GameBase handle the message first...
			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;

			if( pOCS )
			{
				OnPrecreateMsg( pOCS, fData );
			}

			// Important!! - We already sent the message to the GameBase so DONT do it again...
			return dwRet;
		}
		break;

		case MID_OBJECTCREATED:
		{
			OnObjectCreatedMsg( fData );
		}
		break;

		case MID_UPDATE:
		{
			OnUpdateMsg( );
		}
		break;

		case MID_SAVEOBJECT:
		{
			OnSaveMsg( (ILTMessage_Write*)pData, (uint32)fData );
		}
		break;

		case MID_LOADOBJECT:
		{
			OnLoadMsg( (ILTMessage_Read*)pData, (uint32)fData );
		}
		break;

	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::OnPrecreateMsg
//
//  PURPOSE:	Handel the precreate messages from the engine...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::OnPrecreateMsg( ObjectCreateStruct* pOCS, float fType )
{
	// Read the properties from the world file...
	if( PRECREATE_WORLDFILE == fType )
	{
		const GenericPropList *pProps = &pOCS->m_cProperties;
		float fActivationRadius = pProps->GetReal( "ActivationRadius", m_fActivationRadiusSqr );
		m_fActivationRadiusSqr	= fActivationRadius * fActivationRadius;
		
		m_nLookAtsMax			= pProps->GetLongInt( "LookAtsMax", m_nLookAtsMax );
		m_bLookAtsLocked		= pProps->GetBool( "LookAtLocked", m_bLookAtsLocked );
		m_fLookAtThreshold		= pProps->GetReal( "LookAtThreshold", m_fLookAtThreshold );
		m_sLookAtCommand		= pProps->GetCommand( "LookAtCommand", m_sLookAtCommand.c_str( ));

		m_nLookAwaysMax			= pProps->GetLongInt( "LookAwaysMax", m_nLookAwaysMax );
		m_bLookAwaysLocked		= pProps->GetBool( "LookAwayLocked", m_bLookAwaysLocked );
		m_fLookAwayThreshold	= pProps->GetReal( "LookAwayThreshold", m_fLookAwayThreshold );
		m_sLookAwayCommand		= pProps->GetCommand( "LookAwayCommand", m_sLookAwayCommand.c_str( ));
	}

	if( pOCS && (PRECREATE_SAVEGAME != fType) )
	{
		// Init some data if it's not a saved game

		pOCS->SetFileName( pOCS->m_Name );
		pOCS->m_Flags |= FLAG_NOTINWORLDTREE;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::OnObjectCreatedMsg
//
//  PURPOSE:	Handle any initializing that requires the object to be created...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::OnObjectCreatedMsg( float fType )
{
	if( fType != OBJECTCREATED_SAVEGAME )
	{
		// Turn shadows off...
		g_pLTServer->SetObjectShadowLOD( m_hObject, eEngineLOD_Never );

		// Allow updates...
		SetNextUpdate( UPDATE_NEXT_FRAME );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::OnUpdateMsg
//
//  PURPOSE:	Update the object...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::OnUpdateMsg( )
{
	switch( m_eState )
	{
		case eStateNormal:
		{
			UpdateNormal( );
		}
		break;

		case eStateLookingAt:
		{
			UpdateLookingAt( );
		}
		break;

		case eStateLookedAt:
		{
			UpdateLookedAt( );
		}
		break;

		case eStateLookingAway:
		{
			UpdateLookingAway( );
		}
		break;

		default:
		{
			LTERROR( "LookAtTrigger::OnUpdateMsg - Invalid state!" );
		}
		break;
	}

	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::UpdateNormal
//
//  PURPOSE:	Handle updating the normal, idle state...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::UpdateNormal( )
{
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		// If the lookat is locked, then don't try to look at it.  The lookatlock and the 
		// lookawaylock are like 2 gates to the messages, first the lookatlock, then the lookawaylock.
		// If the lookatlock is set, you won't be able to send lookat or lookaway messages.  If the lookawaylock
		// is set, then you will be able to send lookat messages, but not lookaway messages.
		if( !m_bLookAtsLocked && CanPlayerLookAt( *iter ))
		{
			if( DoPlayerLookAt( *iter ))
			{
				// Immediately activated LookAt so go directly to LookAway state...
				SetState( eStateLookingAway );
			}
			else
			{
				// Not able to immediately activate LookAt try again next update... 
				SetState( eStateLookingAt );
			}

			// Found a player looking at the trigger, don't process any more...
			break;
		}

		++iter;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::UpdateLookingAt
//
//  PURPOSE:	Handle updating the looking at state...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::UpdateLookingAt( )
{
	CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hLookAtPlayer ));
	if( !pPlayer )
	{
		// Lost the player, go back to normal...
		SetState( eStateNormal );
		return;
	}

	if( IsPlayerLookingAt( pPlayer ))
	{
		// Continuely try to activate LookAt...
		if( DoPlayerLookAt( pPlayer ))
			SetState( eStateLookedAt );
	}
	else
	{
		// The player is no longer looking at the trigger...
		SetState( eStateLookingAway );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::UpdateLookedAt
//
//  PURPOSE:	Handle updating the looked at state...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::UpdateLookedAt( )
{
	CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hLookAtPlayer ));
	if( !pPlayer )
	{
		// Lost the player, go back to normal...
		SetState( eStateNormal );
		return;
	}

	if( !IsPlayerLookingAt( pPlayer ))
		SetState( eStateLookingAway );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::UpdateLookingAway
//
//  PURPOSE:	Handle updating the looking away state...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::UpdateLookingAway( )
{
	CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hLookAtPlayer ));
	if( !pPlayer )
	{
		// Lost the player, go back to normal...
		SetState( eStateNormal );
		return;
	}
	
	if( CanPlayerLookAt( pPlayer ))
	{
		// The player looked back at the trigger...
		SetState( eStateLookingAt );
	}
	else
	{
		// Continuely try to activate LookAway...
		if( DoPlayerLookAway( ))
			SetState( eStateNormal );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::IsPlayerLookingAt
//
//  PURPOSE:	Check if the player is in range and the object is in the players line of sight...
//
// ----------------------------------------------------------------------- //

bool LookAtTrigger::CanPlayerLookAt( CPlayerObj *pPlayer )
{
	bool bCanPlayerLookAt = false;
	if( !pPlayer )
		return bCanPlayerLookAt;

	if( m_hLookAtPlayer && (pPlayer->m_hObject != m_hLookAtPlayer) )
	{
		// Don't allow more than one player to LookAt...
		return bCanPlayerLookAt;
	}
	
	LTVector vPlayerPos;
	LTVector vTriggerPos;

	g_pLTServer->GetObjectPos( m_hObject, &vTriggerPos );
	g_pLTServer->GetObjectPos( pPlayer->m_hObject, &vPlayerPos );

	// Check if player is within the activation radius...
	if( vTriggerPos.DistSqr( vPlayerPos ) < m_fActivationRadiusSqr )
	{
		// Cast a ray from the players view to see if hes looking at us...
		LTRigidTransform tPlayerView;
		pPlayer->GetViewTransform( tPlayerView );

		IntersectQuery	iQuery;
		IntersectInfo	iInfo;

		iQuery.m_Flags	= INTERSECT_OBJECTS | IGNORE_NONSOLID;
		iQuery.m_From	= tPlayerView.m_vPos;
		iQuery.m_To		= tPlayerView.m_vPos + (tPlayerView.m_rRot.Forward( ) * m_fActivationRadiusSqr);

		// Filter out specific objects we don't want to hit...
		HOBJECT hFilterList[]	= {pPlayer->m_hObject, pPlayer->GetHitBox( ), NULL};
		iQuery.m_FilterFn		= ObjListFilterFn;
		iQuery.m_pUserData		= hFilterList;

		// Allow the trigger to recieve raycasts only while updating...
		AllowRayHits( true );

		if( g_pLTServer->IntersectSegment( iQuery, &iInfo ))
		{
			if( iInfo.m_hObject == m_hObject )
			{
				bCanPlayerLookAt = true;
			}
		}
	
		// Remove the ability to recieve ray casts so the trigger doesn't interfere with other systems...
		AllowRayHits( false );
	}

	return bCanPlayerLookAt;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::DoPlayerLookAt
//
//  PURPOSE:	Determine if the LookAtTrigger can be looked at by the player, and if so activate the LookAt state...
//
// ----------------------------------------------------------------------- //

bool LookAtTrigger::DoPlayerLookAt( CPlayerObj *pPlayer )
{
	if( !pPlayer )
		return false;

	// Cache the player looking at the trigger...
	m_hLookAtPlayer = pPlayer->m_hObject;
	
	// Make sure the correct amount of time has passed if a threshold was specified...
	if( m_fLookAtThreshold > 0.0f )
	{
		m_fLookAtTime += g_pLTServer->GetFrameTime();
		if( m_fLookAtTime < m_fLookAtThreshold )
			return false;
	}

	ActivateLookAt( m_hLookAtPlayer );
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::ActivateLookAt
//
//  PURPOSE:	Determine if the object can be looked at, and if so do it...
//
// ----------------------------------------------------------------------- //

bool LookAtTrigger::ActivateLookAt( HOBJECT hActivate )
{
	// Can't LookAt when locked, a player is already looking at or maxed out the number of LookAts...
	if( m_bLookAtsLocked || ((m_nLookAtsMax > -1) && (m_nLookAtsNum >= m_nLookAtsMax)) )
		return false;

	if( !m_sLookAtCommand.empty( ))
	{
		g_pCmdMgr->QueueCommand( m_sLookAtCommand.c_str( ), hActivate, hActivate );
	}

	++m_nLookAtsNum;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::IsPlayerLookingAt
//
//  PURPOSE:	Checks if the player is currently the LookAtPlayer and calculates if they are still looking at the LookAtTrigger...
//
// ----------------------------------------------------------------------- //

bool LookAtTrigger::IsPlayerLookingAt( CPlayerObj *pPlayer )
{
	bool bIsPlayerLookingAt = false;
	if( !pPlayer || !m_hLookAtPlayer || (pPlayer->m_hObject != m_hLookAtPlayer) )
		return bIsPlayerLookingAt;

	LTVector vPlayerPos;
	LTVector vTriggerPos;

	g_pLTServer->GetObjectPos( m_hObject, &vTriggerPos );
	g_pLTServer->GetObjectPos( pPlayer->m_hObject, &vPlayerPos );

	// Cast a ray from the players view to see if hes looking at us...
	LTRigidTransform tPlayerView;
	pPlayer->GetViewTransform( tPlayerView );

	IntersectQuery	iQuery;
	IntersectInfo	iInfo;

	iQuery.m_Flags	= INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iQuery.m_From	= tPlayerView.m_vPos;
	iQuery.m_To		= tPlayerView.m_vPos + (tPlayerView.m_rRot.Forward( ) * m_fActivationRadiusSqr);

	// Filter out specific objects we don't want to hit...
	HOBJECT hFilterList[]	= {pPlayer->m_hObject, pPlayer->GetHitBox( ), NULL};
	iQuery.m_FilterFn		= LookAwayFilterFn;
	iQuery.m_pUserData		= hFilterList;
    iQuery.m_PolyFilterFn	= InvisibleSurfaceFilterFn;

	// Allow the trigger to recieve raycasts only while updating...
	AllowRayHits( true );

	if( g_pLTServer->IntersectSegment( iQuery, &iInfo ))
	{
		if( iInfo.m_hObject == m_hObject )
		{
			bIsPlayerLookingAt = true;
		}
	}

	// Remove the ability to recieve ray casts so the trigger doesn't interfere with other systems...
	AllowRayHits( false );

	return bIsPlayerLookingAt;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::DoPlayerLookAway
//
//  PURPOSE:	Determines if the LookAtTrigger can be looked away from by the plaeyr, and if so activate the LookAway state...
//
// ----------------------------------------------------------------------- //

bool LookAtTrigger::DoPlayerLookAway( )
{
	// Make sure a player was actually looking at us...
	if( !m_hLookAtPlayer )
		return false;

	// Make sure the correct amount of time has passed if a threshold was specified...
	if( m_fLookAwayThreshold > 0.0f )
	{
		m_fLookAwayTime += g_pLTServer->GetFrameTime();
		if( m_fLookAwayTime < m_fLookAwayThreshold )
			return false;
	}

	ActivateLookAway( m_hLookAtPlayer );
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::DoPlayerLookAway
//
//  PURPOSE:	If allowed to, executes the LookAway command and increments the number of LookAways...
//
// ----------------------------------------------------------------------- //

bool LookAtTrigger::ActivateLookAway( HOBJECT hActivate )
{
	// Can't LookAt when locked, a player is already looking at or maxed out the number of LookAts...
	if( m_bLookAwaysLocked || ((m_nLookAwaysMax > -1) && (m_nLookAwaysNum >= m_nLookAwaysMax)) )
		return false;

	if( !m_sLookAwayCommand.empty( ))
	{
		g_pCmdMgr->QueueCommand( m_sLookAwayCommand.c_str( ), hActivate, hActivate );
	}

	++m_nLookAwaysNum;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::OnLinkBroken
//
//  PURPOSE:	Handle state clean up if the player that is looking at the LookAtTrigger gets removed...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( pRef == &m_hLookAtPlayer )
		SetState( eStateNormal );

	GameBase::OnLinkBroken( pRef, hObj );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::AllowRayHits
//
//  PURPOSE:	Turns on or off the ability of the LookAt trigger to recieve rayhits...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::AllowRayHits( bool bRayHits )
{
	if( bRayHits )
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT | FLAG_NOTINWORLDTREE );
	}
	else
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_NOTINWORLDTREE, FLAG_RAYHIT | FLAG_NOTINWORLDTREE );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::SetState
//
//  PURPOSE:	Switches to the specified state...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::SetState( LookAtTrigger::State eState )
{
	if( m_eState == eState )
		return;

	m_eState = eState;

	// Reset data...
	switch( m_eState )
	{
		case eStateNormal:
		{
			m_hLookAtPlayer = NULL;
			m_fLookAtTime = 0.0f;
			m_fLookAwayTime = 0.0f;
		}
		break;

		case eStateLookingAt:
		case eStateLookedAt:
		{
			m_fLookAtTime = 0.0f;
		}
		break;

		case eStateLookingAway:
		{
			m_fLookAwayTime = 0.0f;
		}
		break;

		default:
		{
			LTERROR( "LookAtTrigger::OnUpdateMsg - Invalid state!" );
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::OnSaveMsg
//
//  PURPOSE:	Save the object state when a MID_LOADOBJECT message is recieved...
//
// ----------------------------------------------------------------------- //

void LookAtTrigger::OnSaveMsg( ILTMessage_Write *pMsg, uint32 dwFlags )
{
	LTASSERT( pMsg != NULL, "LookAtTrigger::OnSaveMsg - NULL message!" );
	if( !pMsg )
		return;

	SAVE_FLOAT( m_fActivationRadiusSqr );
	SAVE_INT( m_nLookAtsMax );
	SAVE_INT( m_nLookAtsNum );
	SAVE_bool( m_bLookAtsLocked );
	SAVE_FLOAT( m_fLookAtThreshold );
	SAVE_FLOAT( m_fLookAtTime );
	SAVE_STDSTRING( m_sLookAtCommand );
	SAVE_INT( m_nLookAwaysMax );
	SAVE_INT( m_nLookAwaysNum );
	SAVE_bool( m_bLookAwaysLocked );
	SAVE_FLOAT( m_fLookAwayThreshold );
	SAVE_FLOAT( m_fLookAwayTime );
	SAVE_STDSTRING( m_sLookAwayCommand );
	SAVE_HOBJECT( m_hLookAtPlayer );
	SAVE_INT( m_eState );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::OnLoadMsg
//
//  PURPOSE:	Load the object state when a MID_LOADOBJECT message is recieved...
// 
// ----------------------------------------------------------------------- //

void LookAtTrigger::OnLoadMsg( ILTMessage_Read *pMsg, uint32 dwFlags )
{
	LTASSERT( pMsg != NULL, "LookAtTrigger::OnLoadMsg - NULL message!" );
	if( !pMsg )
		return;

	LOAD_FLOAT( m_fActivationRadiusSqr );
	LOAD_INT( m_nLookAtsMax );
	LOAD_INT( m_nLookAtsNum );
	LOAD_bool( m_bLookAtsLocked );
	LOAD_FLOAT( m_fLookAtThreshold );
	LOAD_FLOAT( m_fLookAtTime );
	LOAD_STDSTRING( m_sLookAtCommand );
	LOAD_INT( m_nLookAwaysMax );
	LOAD_INT( m_nLookAwaysNum );
	LOAD_bool( m_bLookAwaysLocked );
	LOAD_FLOAT( m_fLookAwayThreshold );
	LOAD_FLOAT( m_fLookAwayTime );
	LOAD_STDSTRING( m_sLookAwayCommand );
	LOAD_HOBJECT( m_hLookAtPlayer );
	LOAD_INT_CAST( m_eState, LookAtTrigger::State );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::HandleLockMsg
//
//  PURPOSE:	Handles an LOCK message...
// 
// ----------------------------------------------------------------------- //

void LookAtTrigger::HandleLockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_LookAt( "LookAt" );
	static CParsedMsg::CToken s_cTok_LookAway( "LookAway" );

	// Lock both LookAt and LookAway if neither was specified...

	if( (crParsedMsg.GetArg(1) == s_cTok_LookAt) || (crParsedMsg.GetArgCount() < 2) )
	{
		m_bLookAtsLocked = true;
	}

	if( (crParsedMsg.GetArg(1) == s_cTok_LookAway) || (crParsedMsg.GetArgCount() < 2) )
	{
		m_bLookAwaysLocked = true;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::HandleUnlockMsg
//
//  PURPOSE:	Handles an UNLOCK message...
// 
// ----------------------------------------------------------------------- //

void LookAtTrigger::HandleUnlockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_LookAt( "LookAt" );
	static CParsedMsg::CToken s_cTok_LookAway( "LookAway" );

	// Unock both LookAt and LookAway if neither was specified...

	if( (crParsedMsg.GetArg(1) == s_cTok_LookAt) || (crParsedMsg.GetArgCount() < 2) )
	{
		m_bLookAtsLocked = false;
	}

	if( (crParsedMsg.GetArg(1) == s_cTok_LookAway) || (crParsedMsg.GetArgCount() < 2) )
	{
		m_bLookAwaysLocked = false;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::HandleUnlockMsg
//
//  PURPOSE:	Handles an LOOKAT message...
// 
// ----------------------------------------------------------------------- //

void LookAtTrigger::HandleLookAtMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	ActivateLookAt( hSender );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LookAtTrigger::HandleLookAwayMsg
//
//  PURPOSE:	Handles an LOOKAWAY message...
// 
// ----------------------------------------------------------------------- //

void LookAtTrigger::HandleLookAwayMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	ActivateLookAway( hSender );
}
