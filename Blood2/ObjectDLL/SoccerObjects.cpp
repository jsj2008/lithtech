/****************************************************************************
;
;	 MODULE:		SOCCEROBJECTS (.CPP)
;
;	PURPOSE:		Soccer objects for zombie-head soccer
;
;	HISTORY:		1/26/99 [bp] This file was created
;
;	COMMENT:		Copyright (c) 1999, Monolith Productions Inc.
;
****************************************************************************/

#include "SoccerObjects.h"
#include "ObjectUtilities.h"
#include <mbstring.h>
#include "SoundTypes.h"
#include "ObjectUtilities.h"
#include "LMessage.h"
#include "ClientGibFx.h"
#include "ClientServerShared.h"
#include "Trigger.h"
#include "BloodServerShell.h"
#include "NetDefs.h"

#define TIMEBETWEENBOUNCESOUNDS		0.1f
#define MAXBOUNCEFACTOR				0.5f
#define MINBOUNCEFACTOR				0.2f
#define MAXBALLVEL					1000.0f
#define MINBALLVEL					20.0f
#define BALLDRAG					0.8f
#define MINFORCESOUND				100.0f
#define BALLRESPAWNTIME				60.0f

//////////////////// SOCCER GOAL ///////////////////////////

DList SoccerGoal::m_GoalList =
{
	0, 
	{ DNULL, DNULL, DNULL }
};

BEGIN_CLASS(SoccerGoal)
	ADD_LONGINTPROP( TeamID, 1 )
	ADD_BOOLPROP( Directional, DFALSE )
	ADD_ROTATIONPROP( GoalDirection )
	ADD_BOOLPROP( BoxPhysics, DTRUE )
	ADD_STRINGPROP( ScoreSound, "" )
	ADD_REALPROP( ScoreSoundRadius, 1000.0f )
	ADD_STRINGPROP( ScoreTarget, "" )
	ADD_STRINGPROP( ScoreMsg, "" )
END_CLASS_DEFAULT_FLAGS(SoccerGoal, B2BaseClass, NULL, NULL, CF_ALWAYSLOAD)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::SoccerGoal
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SoccerGoal::SoccerGoal() : B2BaseClass( OT_WORLDMODEL )
{
	m_nTeamID = TEAM_1;
	m_bDirectional = DFALSE;
	VEC_SET( m_vGoalDirection, 0.0f, 0.0f, 1.0f );
	m_hstrScoreSound = DNULL;
	m_fRadius = 1000.0f;
	m_hstrScoreTarget = DNULL;
	m_hstrScoreMsg = DNULL;
	m_bBoxPhysics = DTRUE;
	m_hSoccerBall = DNULL;
	m_nNumBallsToMake = 0;
	m_bWaitOneFrame = DFALSE;

	if( m_GoalList.m_nElements == 0 )
		dl_InitList( &m_GoalList );

	dl_AddTail( &m_GoalList, &m_Link, this );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::~SoccerGoal
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SoccerGoal::~SoccerGoal()
{
	dl_RemoveAt( &m_GoalList, &m_Link );

	if( m_hstrScoreSound )
		g_pServerDE->FreeString( m_hstrScoreSound );

	if( m_hstrScoreTarget )
		g_pServerDE->FreeString( m_hstrScoreTarget );

	if( m_hstrScoreMsg )
		g_pServerDE->FreeString( m_hstrScoreMsg );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::EngineMessageFn
//
//	PURPOSE:	Handler for engine messages
//
// ----------------------------------------------------------------------- //

DDWORD SoccerGoal::EngineMessageFn( DDWORD messageID, void *pData, DFLOAT fData )
{
	DDWORD dwRet;

	// Handle the engine messages we're interested in...
	switch( messageID )
	{
		case MID_PRECREATE:
		{
			dwRet = B2BaseClass::EngineMessageFn( messageID, pData, fData );

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
			break;
		}

		case MID_INITIALUPDATE:
		{
			OnInitialUpdate(pData, fData);
			break;
		}

		case MID_UPDATE:
		{
			Update( );
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			OnTouchNotify((HOBJECT)pData);
			break;
		}
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::ReadProp
//
//	PURPOSE:	Reads SoccerGoal properties
//
// ----------------------------------------------------------------------- //

void SoccerGoal::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;
	DVector vUp, vRight;

	if( g_pServerDE->GetPropGeneric( "TeamID", &genProp ) == DE_OK )
	{
		m_nTeamID = ( int )genProp.m_Long;
	}

	if( g_pServerDE->GetPropGeneric( "Directional", &genProp ) == DE_OK )
	{
		m_bDirectional = genProp.m_Bool;
	}

	if( g_pServerDE->GetPropGeneric( "GoalDirection", &genProp ) == DE_OK )
	{
		g_pServerDE->GetRotationVectors( &genProp.m_Rotation, &vUp, &vRight, &m_vGoalDirection );
	}
	
	if( g_pServerDE->GetPropGeneric( "BoxPhysics", &genProp ) == DE_OK )
	{
		m_bBoxPhysics = genProp.m_Bool;
	}

	if( m_hstrScoreSound )
	{
		g_pServerDE->FreeString( m_hstrScoreSound );
		m_hstrScoreSound = DNULL;
	}
	if( g_pServerDE->GetPropGeneric( "ScoreSound", &genProp ) == DE_OK )
	{
		if( genProp.m_String[0] )
			m_hstrScoreSound = g_pServerDE->CreateString( genProp.m_String );
	}

	if( g_pServerDE->GetPropGeneric( "ScoreSoundRadius", &genProp ) == DE_OK )
	{
		m_fRadius = genProp.m_Float;
	}

	if( m_hstrScoreTarget )
	{
		g_pServerDE->FreeString( m_hstrScoreTarget );
		m_hstrScoreTarget = DNULL;
	}
	if( g_pServerDE->GetPropGeneric( "ScoreTarget", &genProp ) == DE_OK && genProp.m_String[0] )
	{
		if( genProp.m_String[0] )
			m_hstrScoreTarget = g_pServerDE->CreateString( genProp.m_String );
	}

	if( m_hstrScoreMsg )
	{
		g_pServerDE->FreeString( m_hstrScoreMsg );
		m_hstrScoreMsg = DNULL;
	}
	if( g_pServerDE->GetPropGeneric( "ScoreMsg", &genProp ) == DE_OK )
	{
		m_hstrScoreMsg = g_pServerDE->CreateString( genProp.m_String );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::PostPropRead()
//
//	PURPOSE:	Updates the properties now that they have been read
//
// ----------------------------------------------------------------------- //

void SoccerGoal::PostPropRead(ObjectCreateStruct* pStruct)
{
	// Sanity checks...
	if( !pStruct )
		return;

	_mbscpy(( unsigned char * )pStruct->m_Filename, ( const unsigned char * )pStruct->m_Name );

	// Set the flags we want...
	pStruct->m_Flags = FLAG_TOUCH_NOTIFY;
	if( m_bBoxPhysics )
		pStruct->m_Flags |= FLAG_BOXPHYSICS;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::OnInitialUpdate()
//
//	PURPOSE:	Handles the MID_INITIALUPDATE engine message
//
// ----------------------------------------------------------------------- //

void SoccerGoal::OnInitialUpdate(void* pData, DFLOAT fData)
{
	m_hSoccerBall = g_pServerDE->GetClass( "SoccerBall" );

	g_pServerDE->SetNextUpdate( m_hObject, 0.0f );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::ObjectMessageFn
//
//	PURPOSE:	Handle messages from objects
//
// ----------------------------------------------------------------------- //
/*
DDWORD SoccerGoal::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		default: break;
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::OnTouchNotify()
//
//	PURPOSE:	Handles the MID_TOUCHNOTIFY engine messsage
//
// ----------------------------------------------------------------------- //

void SoccerGoal::OnTouchNotify( HOBJECT hObj )
{
	CollisionInfo colInfo;
	DVector vBallVel;
	DBOOL bGoal;
	SoccerBall *pSoccerBall;
	DVector vPos, vDir, vDims;
	HOBJECT hPlayer;
	LMessage *pMsg;
	
	if( g_pServerDE->IsKindOf( g_pServerDE->GetObjectClass(hObj), m_hSoccerBall ))
	{
		pSoccerBall = ( SoccerBall * )g_pServerDE->HandleToObject( hObj );
		if( !pSoccerBall )
			return;

		// Already recorded this goal.  Ball should delete itself soon.
		if( pSoccerBall->IsMadeGoal( ))
			return;

		// Ball has to enter from correct side for directional goals
		if( m_bDirectional )
		{
			// Assume no goal
			bGoal = DFALSE;

			g_pServerDE->GetVelocity( hObj, &vBallVel );

			// Check if going in the right direction
			if( VEC_DOT( vBallVel, m_vGoalDirection ) > 0.0f )
			{
				bGoal = DTRUE;
			}
		}
		else
			bGoal = DTRUE;

		if( bGoal )
		{
			if(( hPlayer = pSoccerBall->GetLastPlayerTouched( )) == DNULL )
				return;
			// Send message to player and ball
			if( g_pServerDE->Common( )->CreateMessage( pMsg ) != LT_OK )
				return;
			pMsg->WriteByte( m_nTeamID );
			g_pServerDE->SendToObject( *pMsg, MID_GOAL, m_hObject, hPlayer, 0 );
			g_pServerDE->SendToObject( *pMsg, MID_GOAL, m_hObject, hObj, 0 );

			pMsg->Release();

			// Create special effects
			g_pServerDE->GetObjectPos( hObj, &vPos );
			g_pServerDE->GetVelocity( hObj, &vDir );
			VEC_MULSCALAR( vDir, vDir, -1.0f );
			VEC_SET( vDims, 25.0f, 25.0f, 25.0f );
			SetupClientGibFX( &vPos, &vDir, &vDims, ( SURFTYPE_FLESH/10 ) | SIZE_SMALL | TRAIL_BLOOD, 1.0f, 5 );

			// Play the sound
			if( m_hstrScoreSound )
			{
				g_pServerDE->GetObjectPos( m_hObject, &vPos );
				PlaySoundFromPos( &vPos, g_pServerDE->GetStringData( m_hstrScoreSound ), m_fRadius, SOUNDPRIORITY_MISC_MEDIUM );
			}

			SendTrigger( );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::Update()
//
//	PURPOSE:	Updates the goal
//
// ----------------------------------------------------------------------- //

void SoccerGoal::Update( )
{
	DLink *pCur;
	int nGoalIndex;
	SoccerGoal *pGoal;

	// This is needed just in case the world is shutting down.  If it is
	// then we don't want to make a new ball.  If we wait one frame,
	// then the world will have completely gone away, and we won't make
	// a new ball.
	if( m_bWaitOneFrame )
	{
		m_bWaitOneFrame = DFALSE;
		g_pServerDE->SetNextUpdate( m_hObject, 0.001f );
		return;
	}

	while( m_nNumBallsToMake )
	{
		// Choose a random goal to send trigger
		if( !m_GoalList.m_nElements )
			return;
		nGoalIndex = ( int )( g_pServerDE->IntRandom( 0, m_GoalList.m_nElements - 1 ));
		pCur = m_GoalList.m_Head.m_pNext;
		while( pCur != &m_GoalList.m_Head && nGoalIndex > 0 )
		{
			pCur = pCur->m_pNext;
			nGoalIndex--;
		}
		pGoal = ( SoccerGoal * )pCur->m_pData;
		if( pGoal )
			pGoal->SendTrigger( );

		m_nNumBallsToMake--;
	}

	g_pServerDE->SetNextUpdate( m_hObject, 0.0f );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::SendTrigger()
//
//	PURPOSE:	Sends trigger message
//
// ----------------------------------------------------------------------- //

void SoccerGoal::SendTrigger( )
{
	// Send the message
	if( m_hstrScoreTarget && m_hstrScoreMsg )
	{
		SendTriggerMsgToObjects( this, m_hstrScoreTarget, m_hstrScoreMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerGoal::SpawnBall()
//
//	PURPOSE:	Goal must spawn a new ball, cuz something bad happened
//				to the other ball
//
// ----------------------------------------------------------------------- //

void SoccerGoal::SpawnBall( )
{
	m_nNumBallsToMake++;
	g_pServerDE->SetNextUpdate( m_hObject, 0.001f );
	m_bWaitOneFrame = DTRUE;
}


//////////////////// SOCCER BALL ///////////////////////////

BEGIN_CLASS( SoccerBall )
END_CLASS_DEFAULT( SoccerBall, B2BaseClass, NULL, NULL )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerBall::SoccerBall
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SoccerBall::SoccerBall() : B2BaseClass( OT_MODEL )
{
	m_bOnGround = DFALSE;
	m_fLastTimeOnGround = 0.0f;
	m_fLastBounceTime = 0.0f;
	m_bBounced = DFALSE;
	VEC_INIT( m_vLastPos );
	VEC_SET( m_vLastNormal, 0.0f, 1.0f, 0.0f );
	m_hLastPlayer = DNULL;
//	m_hLight = DNULL;
//	m_hLightAttachment = DNULL;
	m_bMadeGoal = DFALSE;
	m_fRespawnTime = g_pServerDE->GetTime( ) + BALLRESPAWNTIME;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerBall::~SoccerBall
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SoccerBall::~SoccerBall()
{
	SoccerGoal *pGoal;
	DList *pList;
	DLink *pCur;

	// The ball is being removed normally
	if( m_bMadeGoal )
		return;

	// If the ball isn't being removed because of a goal, then something
	// has happened and we need to spawn a new ball.  Tell the first
	// goal to do this.
	pList = SoccerGoal::GetGoalList( );
	if( !pList || pList->m_nElements == 0 )
		return;

	pCur = pList->m_Head.m_pNext;
	pGoal = ( SoccerGoal * )pCur->m_pData;
	if( pGoal )
		pGoal->SpawnBall( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerBall::EngineMessageFn
//
//	PURPOSE:	Handler for engine messages
//
// ----------------------------------------------------------------------- //

DDWORD SoccerBall::EngineMessageFn( DDWORD messageID, void *pData, DFLOAT fData )
{
	// Handle the engine messages we're interested in...

	switch( messageID )
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			break;
		}

		case MID_INITIALUPDATE:
		{
			OnInitialUpdate(pData, fData);
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			OnTouchNotify(( HOBJECT )pData, fData );
			break;
		}

		case MID_UPDATE:
		{
			Update( );
			break;
		}

		case MID_LINKBROKEN:
		{
			if( m_hLastPlayer == ( HOBJECT )pData )
			{
				m_hLastPlayer = DNULL;
			}
/*			else if( m_hLight == ( HOBJECT )pData )
			{
				g_pServerDE->RemoveAttachment( m_hLightAttachment );
				m_hLightAttachment = DNULL;
				m_hLight = DNULL;
			}
*/
			break;
		}
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerBall::ReadProp
//
//	PURPOSE:	Reads SoccerBall properties
//
// ----------------------------------------------------------------------- //

void SoccerBall::ReadProp(ObjectCreateStruct *pStruct)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStand::PostPropRead()
//
//	PURPOSE:	Updates the properties now that they have been read
//
// ----------------------------------------------------------------------- //

void SoccerBall::PostPropRead(ObjectCreateStruct* pStruct)
{
	const NetGame *pNetGame;

	// Sanity checks...
	if( !pStruct )
		return;

	pNetGame = g_pBloodServerShell->GetNetGameInfo();
	if( !pNetGame || pNetGame->m_nSocBallSkin == SOCBALL_SKIN_ZOMBIE )
	{
		SAFE_STRCPY( pStruct->m_Filename, "models_ao\\worldobjects_ao\\headball.abc" );
		SAFE_STRCPY( pStruct->m_SkinName, "skins_ao\\worldobjects_ao\\headball.dtx" );
	}
	else
	{
		SAFE_STRCPY( pStruct->m_Filename, "models_ao\\worldobjects_ao\\soccerball.abc" );
		SAFE_STRCPY( pStruct->m_SkinName, "skins_ao\\worldobjects_ao\\soccerball.dtx" );
	}

	// Set the flags we want...
	pStruct->m_Flags = FLAG_TOUCH_NOTIFY | FLAG_GRAVITY | FLAG_SOLID | FLAG_VISIBLE | FLAG_REMOVEIFOUTSIDE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerBall::OnInitialUpdate()
//
//	PURPOSE:	Handles the MID_INITIALUPDATE engine message
//
// ----------------------------------------------------------------------- //

void SoccerBall::OnInitialUpdate(void* pData, DFLOAT fData)
{
	DVector vDims;
	DDWORD dwFlags;

	g_pServerDE->SetForceIgnoreLimit( m_hObject, 0.0f );
//	g_pServerDE->SetFrictionCoefficient( m_hObject, 10.0f );

	g_pServerDE->GetModelAnimUserDims( m_hObject, &vDims, 0 );
	if( VEC_MAGSQR( vDims ) < 1.0f )
		VEC_SET( vDims, 1.0f, 1.0f, 1.0f );
	g_pServerDE->SetObjectDims( m_hObject, &vDims );
	m_fRadius = ( vDims.x + vDims.y + vDims.z ) / 3.0f;

	g_pServerDE->SetNextUpdate( m_hObject, 0.001f );

//	CreateLight( );

	// Mark this object as savable
	dwFlags = g_pServerDE->GetObjectUserFlags( m_hObject );
	dwFlags |= USERFLG_NIGHTGOGGLESGLOW;
	g_pServerDE->SetObjectUserFlags( m_hObject, dwFlags );

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerBall::ObjectMessageFn
//
//	PURPOSE:	Handle messages from objects
//
// ----------------------------------------------------------------------- //

DDWORD SoccerBall::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_GOAL:
		{
			m_bMadeGoal = DTRUE;
			g_pServerDE->RemoveObject( m_hObject );
		}
		break;

		default: break;
	}

	return B2BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerBall::Update()
//
//	PURPOSE:	Updates the ball
//
// ----------------------------------------------------------------------- //

void SoccerBall::Update( )
{
	DVector vVel, vAccel, vAccelAdd, vPos, vForward, vCross, vTemp, vTemp2;
	CollisionInfo collInfo;
	float fVelMag, fDistTraveled, fTime, fRotAmount, fExp;
	DRotation rRot;

	g_pServerDE->GetObjectPos( m_hObject, &vPos );
	g_pServerDE->GetVelocity( m_hObject, &vVel );
	fVelMag = VEC_MAG( vVel );
	fTime = g_pServerDE->GetTime( );

	// Remove the ball if it's been sitting around for a while.
	if( fTime > m_fRespawnTime )
	{
		g_pServerDE->RemoveObject( m_hObject );
		return;
	}

	// Update the on ground info
	g_pServerDE->GetStandingOn( m_hObject, &collInfo );
	m_bOnGround = ( collInfo.m_hObject ) ? DTRUE : DFALSE;
	if( m_bOnGround )
	{
		m_fLastTimeOnGround = fTime;
	}

	// Get how far we've traveled.
	VEC_SUB( vForward, vPos, m_vLastPos );
	fDistTraveled = VEC_MAG( vForward );
	VEC_COPY( m_vLastPos, vPos );

	// Rotate the ball
	if( fDistTraveled > 0.0f )
	{
		VEC_MULSCALAR( vForward, vForward, 1.0f / fDistTraveled );

		if( m_bOnGround )
		{
			VEC_COPY( m_vLastNormal, collInfo.m_Plane.m_Normal );
			VEC_CROSS( vCross, vForward, m_vLastNormal );
			fRotAmount = VEC_MAG( vCross ) * fDistTraveled / m_fRadius;
		}
		else
		{
			VEC_CROSS( vCross, vForward, m_vLastNormal );
			fRotAmount = VEC_MAG( vCross ) * fDistTraveled / m_fRadius;
		}

		if( fRotAmount > 0.0f )
		{
			VEC_NORM( vCross );
			g_pServerDE->GetObjectRotation( m_hObject, &rRot );
			g_pServerDE->RotateAroundAxis( &rRot, &vCross, fRotAmount );
			g_pServerDE->SetObjectRotation( m_hObject, &rRot );
		}
	}

	// Adjust the velocity and accel
	if( fVelMag < MINBALLVEL )
	{
		VEC_INIT( vVel );
		g_pServerDE->SetVelocity( m_hObject, &vVel );
	}
	else if( fVelMag > MAXBALLVEL )
	{
		VEC_MULSCALAR( vVel, vVel, MAXBALLVEL / fVelMag );				
		g_pServerDE->SetVelocity( m_hObject, &vVel );
	}
	else
	{
		// new velocity is given by:		v = ( a / k ) + ( v_0 - a / k ) * exp( -k * t )
		g_pServerDE->GetAcceleration( m_hObject, &vAccel );
		fExp = ( float )exp( -BALLDRAG * g_pServerDE->GetFrameTime( ));
		VEC_DIVSCALAR( vTemp, vAccel, BALLDRAG );
		VEC_SUB( vTemp2, vVel, vTemp );
		VEC_MULSCALAR( vTemp2, vTemp2, fExp );
		VEC_ADD( vVel, vTemp2, vTemp );
		g_pServerDE->SetVelocity( m_hObject, &vVel );
	}

	// Make sure we're rolling if we're on a slope.  This counteracts the way the
	// engine stops objects on slopes.
	if( m_bOnGround )
	{
		if( collInfo.m_Plane.m_Normal.y < 0.9f && fabs( vVel.y ) < 50.0f )
		{
			g_pServerDE->GetGlobalForce( &vAccelAdd );
			vAccel.y += vAccelAdd.y * 0.5f;
			g_pServerDE->SetAcceleration( m_hObject, &vAccel );
		}
	}

	// Play a bounce sound if enough time has elapsed
	if( m_bBounced )
	{
		if( fTime > m_fLastBounceTime + TIMEBETWEENBOUNCESOUNDS )
		{
			// Play a bounce sound...
			PlaySoundFromPos( &vPos, "Sounds_ao\\events\\soccerball.wav", 750, SOUNDPRIORITY_MISC_MEDIUM );
		}

		m_bBounced = DFALSE;
	}

	g_pServerDE->SetNextUpdate( m_hObject, 0.001f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerBall::OnTouchNotify()
//
//	PURPOSE:	Handles the MID_TOUCHNOTIFY engine messsage
//
// ----------------------------------------------------------------------- //

void SoccerBall::OnTouchNotify( HOBJECT hObj, float fForce )
{
	CollisionInfo colInfo;
	DVector vVel, vOtherVel, vEffectiveVel, vPos, vNewVel;
	DVector vNormal;
	float fEffectiveVelMag, fMultiplier, fVelMag;
	DRotation rRot;
	DVector vUp, vRight;
	DBOOL bIsPlayer;
	
	g_pServerDE->GetVelocity( m_hObject, &vVel );
	g_pServerDE->GetVelocity( hObj, &vOtherVel );

	g_pServerDE->GetObjectPos( m_hObject, &vPos );

	if( hObj == g_pServerDE->GetWorldObject( ))
	{
		VEC_SUB( vEffectiveVel, vOtherVel, vVel );
		fEffectiveVelMag = VEC_MAG( vEffectiveVel );
	
		g_pServerDE->GetLastCollision( &colInfo );

		// Compute new velocity reflected off of the surface.
		if( VEC_MAGSQR( colInfo.m_Plane.m_Normal ) > 0.0f )
		{
			VEC_COPY( vNormal, colInfo.m_Plane.m_Normal );
			if( fEffectiveVelMag > MAXBALLVEL )
				fMultiplier = MAXBOUNCEFACTOR;
			else
				fMultiplier = MINBOUNCEFACTOR + ( MAXBOUNCEFACTOR - MINBOUNCEFACTOR ) * fEffectiveVelMag / MAXBALLVEL;
			VEC_MULSCALAR( vNormal, vNormal, fEffectiveVelMag * fMultiplier );
			VEC_ADD( vNewVel, vVel, vNormal );
			if( fabs( vNewVel.y + vVel.y ) < 100.0f )
				vNewVel.y = vVel.y;
			g_pServerDE->SetVelocity( m_hObject, &vNewVel );
		}

		if( fabs( fForce ) > MINFORCESOUND )
			m_bBounced = DTRUE;
	}
	else
	{
		// Ignore non-solid objects
		if( !( g_pServerDE->GetObjectFlags( hObj ) & FLAG_SOLID ))
			return;

		bIsPlayer = IsPlayer( hObj );
		if( bIsPlayer )
		{
			if( m_hLastPlayer && hObj != m_hLastPlayer )
			{
				g_pServerDE->BreakInterObjectLink( m_hObject, m_hLastPlayer );
			}

			m_hLastPlayer = hObj;
			g_pServerDE->CreateInterObjectLink( m_hObject, m_hLastPlayer );

			m_fRespawnTime = g_pServerDE->GetTime( ) + BALLRESPAWNTIME;
		}

		VEC_ADD( vVel, vVel, vOtherVel );
		if( m_bOnGround && vVel.y < 0.0f )
			vVel.y = 0.0f;
		if( bIsPlayer )
			vVel.y += g_pServerDE->Random( 150.0f, 300.0f );
		fVelMag = VEC_MAG( vVel );
		g_pServerDE->AlignRotation( &rRot, &vVel, DNULL );
		g_pServerDE->EulerRotateX( &rRot, g_pServerDE->Random( -0.1f, 0.1f ));
		g_pServerDE->EulerRotateY( &rRot, g_pServerDE->Random( -0.1f, 0.1f ));
		g_pServerDE->EulerRotateZ( &rRot, g_pServerDE->Random( -0.1f, 0.1f ));
		g_pServerDE->GetRotationVectors( &rRot, &vUp, &vRight, &vVel );
		VEC_MULSCALAR( vVel, vVel, fVelMag );
		g_pServerDE->SetVelocity( m_hObject, &vVel );

		if( fabs( fForce ) > MINFORCESOUND || bIsPlayer )
			m_bBounced = DTRUE;
	}
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoccerBall::CreateLight()
//
//	PURPOSE:	Put a light on the soccer ball to make it more visible
//
// ----------------------------------------------------------------------- //
void SoccerBall::CreateLight( )
{
	DVector vOffset;
	DRotation rRot;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	if( m_hLightAttachment )
	{
		g_pServerDE->RemoveAttachment( m_hLightAttachment );
		m_hLightAttachment = DNULL;
	}

	if( m_hLight )
	{
		g_pServerDE->RemoveObject( m_hLight );
		m_hLight = DNULL;
	}

	ocStruct.m_Flags = FLAG_VISIBLE;
	ocStruct.m_ObjectType = OT_LIGHT; 

	HCLASS hClass = g_pServerDE->GetClass("BaseClass");
	if (!hClass) return;

	LPBASECLASS	pLight = g_pServerDE->CreateObject(hClass, &ocStruct);
	if (!pLight) return;

	m_hLight = pLight->m_hObject;
	g_pServerDE->SetLightRadius( m_hLight, BALLLIGHTRADIUS );
	g_pServerDE->SetLightColor( m_hLight, 0.25f, 1.0f, 0.25f );
	VEC_INIT( vOffset );
	ROT_INIT( rRot );

	g_pServerDE->CreateAttachment( m_hObject, m_hLight, DNULL, &vOffset, &rRot, &m_hLightAttachment );
	g_pServerDE->CreateInterObjectLink( m_hObject, m_hLight );
}
*/