// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPointMgr.cpp
//
// PURPOSE : Manages gamestartpoint functions.
//
// CREATED : 03/03/06
//
// (c) 1997-2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "GameStartPointMgr.h"
#include "GameStartPoint.h"
#include "ParsedMsg.h"
#include "ServerConnectionMgr.h"
#include "Character.h"
#include "PlayerObj.h"
#include "EngineTimer.h"
#include "ltintersect.h"
#include "ProjectileTypes.h"
#include "GameModeMgr.h"
#include "DebugLineSystem.h"
#include "EngineTimer.h"

#ifndef _FINAL
static void DebugStartPtCB(int argc, char **argv);
#endif // _FINAL

VarTrack			g_vtSpawn_PlayerDimsScale;
VarTrack			g_vtSpawn_BuddyDamageTimeout;
VarTrack			g_vtSpawn_BuddyDamageMag;
VarTrack			g_vtSpawn_BuddyMinDistance;
VarTrack			g_vtSpawn_BuddyHalfDistance;
VarTrack			g_vtSpawn_BuddyMaxDistance;
VarTrack			g_vtSpawn_BuddyMaxHeight;
VarTrack			g_vtSpawn_BuddyHeightMag;
VarTrack			g_vtSpawn_BuddyRespawnGraceTime;
VarTrack			g_vtSpawn_EnemyMag;
VarTrack			g_vtSpawn_EnemyHalfDistance;
VarTrack			g_vtSpawn_EnemyMaxDistance;
VarTrack			g_vtSpawn_TopNumCandidates;


GameStartPointMgr::GameStartPointMgr( )
{
#ifndef _FINAL
	//register our console program so we can get info
	g_pLTServer->RegisterConsoleProgram("debugstartpt", DebugStartPtCB);
#endif // _FINAL

	// How big to scale the player dims to invalidate spawn points.
	g_vtSpawn_PlayerDimsScale.Init( g_pLTServer, "Spawn_PlayerDimsScale", NULL, 4.0f);
	// Time in seconds that a buddy taking damage will have a negative effect on spawn gravity.
	g_vtSpawn_BuddyDamageTimeout.Init( g_pLTServer, "Spawn_BuddyDamageTimeout", NULL, 1.5f);
	// The scale magnitude of a buddy taking damage does to the spawn gravity.
	g_vtSpawn_BuddyDamageMag.Init( g_pLTServer, "Spawn_BuddyDamageMag", NULL, 0.5f);
	// The minimum distance a buddy will apply positive gravity to spawn points.  Less than this value,
	// the buddy will apply negative gravity.
	g_vtSpawn_BuddyMinDistance.Init( g_pLTServer, "Spawn_BuddyMinDistance", NULL, 400.0f );
	// The distance that a buddy's positive effect to gravity will be cut by half.
	g_vtSpawn_BuddyHalfDistance.Init( g_pLTServer, "Spawn_BuddyHalfDistance", NULL, 1000.0f );
	// The maximum distance that a buddy's positive effect to gravity applies.
	g_vtSpawn_BuddyMaxDistance.Init( g_pLTServer, "Spawn_BuddyMaxDistance", NULL, 2000.0f );
	// The maximum height a spawn point can be above or below a buddy without being adversely affected.
	g_vtSpawn_BuddyMaxHeight.Init( g_pLTServer, "Spawn_BuddyMaxHeight", NULL, 350.0f );
	// The scale magnitude a spawn outside the max height will apply to the gravity.
	g_vtSpawn_BuddyHeightMag.Init( g_pLTServer, "Spawn_BuddyHeightMag", NULL, 0.75f );
	// Amount of time to not consider a newly spawned buddy a negative affect on gravity.
	g_vtSpawn_BuddyRespawnGraceTime.Init( g_pLTServer, "Spawn_BuddyRespawnGraceTime", NULL, 0.25f );
	// The magnitude of the gravity an enemy applies to gravity.
	g_vtSpawn_EnemyMag.Init( g_pLTServer, "Spawn_EnemyMag", NULL, 2.0f );
	// The distance that a enemy's negative effect to gravity will be cut by half.
	g_vtSpawn_EnemyHalfDistance.Init( g_pLTServer, "Spawn_EnemyHalfDistance", NULL, 500.0f );
	// The maximum distance that a enemy's negative effect to gravity applies.
	g_vtSpawn_EnemyMaxDistance.Init( g_pLTServer, "Spawn_EnemyMaxDistance", NULL, 2000.0f );
	// The number in the list of top candidates to randomly choose from.
	g_vtSpawn_TopNumCandidates.Init( g_pLTServer, "Spawn_TopNumCandidates", NULL, 3.0f );
}

GameStartPointMgr::~GameStartPointMgr( )
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsObjectOnPoint
//
//	PURPOSE:	Check if object is at this position
//
// ----------------------------------------------------------------------- //

inline static bool IsObjectOnPoint( HOBJECT hObject, LTVector const& vPos )
{
	LTVector vObjPos, vDims;
	g_pLTServer->GetObjectPos( hObject, &vObjPos );
	g_pPhysicsLT->GetObjectDims( hObject, &vDims );

	// Increase the size of the dims to account for the players
	// dims overlapping...
	vDims *= g_vtSpawn_PlayerDimsScale.GetFloat( 2.0f );

	return LTIntersect::AABB_Point( LTRect3f( LTVector( vObjPos - vDims ), LTVector( vObjPos + vDims )), vPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ValidGameStartPoint()
//
//	PURPOSE:	Used when finding a valid gamestartpoint to respawn to.
//
// ----------------------------------------------------------------------- //
struct ValidGameStartPoint
{
	ValidGameStartPoint( )
	{
		m_pStartPt = NULL;
		m_ePositionOccupied = ePositionOccupied_Unknown;
		// Start gravity off with a little randomness, so if we
		// can't find a start point deterministically, we can
		// still have randomness.
		m_fGravity = 0.0f;
	}

	GameStartPoint* m_pStartPt;
	float			m_fGravity;

	enum PositionOccupied
	{
		ePositionOccupied_Unknown,
		ePositionOccupied_Yes,
		ePositionOccupied_No,
	};
	PositionOccupied	m_ePositionOccupied;
};

typedef std::vector< ValidGameStartPoint > ValidGameStartPointList;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FindValidGameStartPoints
//
//	PURPOSE:	Finds the list of valid gamestartpoints.
//
// ----------------------------------------------------------------------- //
static bool FindValidGameStartPoints( ValidGameStartPointList& lst, bool bStartPointsOnly, CPlayerObj& spawningPlayer )
{
	// Make room for all start points, even though all of them may not be used.
	lst.reserve( GameStartPoint::GetStartPointList().size( ));

	LTVector vStartPtPos;

	uint8 nTeamId = spawningPlayer.GetTeamID();

	// Iterate through all the start points and keep the one's that are valid for this search.
	GameStartPoint::StartPointList::const_iterator iter = GameStartPoint::GetStartPointList().begin();
	for( ; iter != GameStartPoint::GetStartPointList().end(); iter++ )
	{
		GameStartPoint* pStartPt = *iter;

		// Make sure it's not locked.
		if( pStartPt->IsLocked())
		{
			continue;
		}

		// Make sure it's the right type of startpoint.
		if( bStartPointsOnly )
		{
			if( !pStartPt->IsStartPoint( ))
			{
				continue;
			}
		}
		else
		{
			if( !pStartPt->IsSpawnPoint( ))
			{
				continue;
			}
		}

		// Don't allow spawning from other team's spawn points.
		if( pStartPt->GetTeamID() != INVALID_TEAM && nTeamId != pStartPt->GetTeamID())
		{
			continue;
		}

		g_pLTServer->GetObjectPos( pStartPt->m_hObject, &vStartPtPos );

		// Make sure we're not near any proxi grenades.
		bool bEnemyGrenadeOccupied = false;
		for( CGrenadeProximity::CGrenadeProximityList::iterator iter = CGrenadeProximity::GetList( ).begin( ); 
			iter != CGrenadeProximity::GetList( ).end( ); iter++ )
		{
			CGrenadeProximity* pGrenadeProximity = *iter;

			// Ignore friendly grenades.
			if( !pGrenadeProximity->IsEnemyOf( spawningPlayer.m_hObject ))
				continue;

			if( !pGrenadeProximity->OccupiesPosition( vStartPtPos ))
				continue;

			bEnemyGrenadeOccupied = true;
			break;
		}

		if( bEnemyGrenadeOccupied )
			continue;

		// Add to our list which we'll sort.
		ValidGameStartPoint validGameStartPoint;
		validGameStartPoint.m_pStartPt = pStartPt;
		lst.push_back( validGameStartPoint );
	}

	return !lst.empty( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShuffleGameStartPoints
//
//	PURPOSE:	Shuffles list of valid gamestartpoints so that start point
//				selection may be non-deterministic.
//
// ----------------------------------------------------------------------- //
static void ShuffleGameStartPoints( ValidGameStartPointList& lst, uint32 nNumPoints )
{
	ValidGameStartPoint swapTemp;

	//step through each element
	for (uint32 n = 0; n < nNumPoints; ++n)
	{
		//choose another element at random
		uint32 x = GetRandom(0,nNumPoints-1);

		//swap the two elements
		swapTemp = lst[n];
		lst[n] = lst[x];
		lst[x] = swapTemp;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ApplyGravitySourceToSpawnPoint
//
//	PURPOSE:	Applies GravitySource influence to all valid start points.
//
// ----------------------------------------------------------------------- //
static void ApplyGravitySourceToSpawnPoint( CPlayerObj const& spawningPlayer, GameStartPointMgr::GravitySource& gravitySource, ValidGameStartPointList& lst )
{
	// Apply positive gravity if the players are on the same team.
	bool bBuddy = false;
	bool bEnemy = true;
	if( GameModeMgr::Instance().m_grbUseTeams )
	{
		bBuddy = ( spawningPlayer.GetTeamID( ) != INVALID_TEAM && spawningPlayer.GetTeamID( ) == gravitySource.m_eTeamId );
		bEnemy = ( spawningPlayer.GetTeamID( ) != INVALID_TEAM && gravitySource.m_eTeamId != INVALID_TEAM && spawningPlayer.GetTeamID( ) != gravitySource.m_eTeamId );
	}

	// Cache the position of the other character.
	LTVector vGravitySourcePos;
	g_pLTServer->GetObjectPos( gravitySource.m_hObject, &vGravitySourcePos );

	// Iterate through all the valid start points and calculate the
	// gravity contribution from the player.
	LTVector vStartPtPos;
	ValidGameStartPointList::iterator iter = lst.begin( );
	for( ; iter != lst.end( ); iter++ )
	{
		ValidGameStartPoint& validGameStartPoint = *iter;

		g_pLTServer->GetObjectPos( validGameStartPoint.m_pStartPt->m_hObject, &vStartPtPos );

		// If we already know this position is occupied, then skip it.
		if( validGameStartPoint.m_ePositionOccupied == ValidGameStartPoint::ePositionOccupied_Yes )
		{
			continue;
		}

		// Check if this player is on this point.
		if( IsObjectOnPoint( gravitySource.m_hObject, vStartPtPos ))
		{
			// Mark the start point as unfavorable.
			validGameStartPoint.m_fGravity = -FLT_MAX;
			// Remember that this point is occupied so we don't need to test again.
			validGameStartPoint.m_ePositionOccupied = ValidGameStartPoint::ePositionOccupied_Yes;
			continue;
		}

		// Calculate the gravity.
		float fGravity = 0.0f;

		// Gravity is applied proportial to 1/e^x.  It rolls off like a half-life, so it has the form:
		// y = 1 / ( e^(k*x) ), where k is ( -ln(.5f) / distance_mag_is_half ), and x is the 
		// current distance.  -ln(.5f) is 0.69314718f, so this is precalculated below.
		if( bBuddy )
		{
			// Get relative vector between other player and startpt.
			LTVector vPosToStart = vGravitySourcePos - vStartPtPos;

			// Calc distance between start and other character.
			float fDistance = vPosToStart.Mag( );

			// Make sure buddy is within max distance.
			if( g_vtSpawn_BuddyMaxDistance.GetFloat( ) <= 0.0f || fDistance <= g_vtSpawn_BuddyMaxDistance.GetFloat( ))
			{
				// Check if the buddy is beyond the max height.
				float fHeightModifier = 1.0f;
				if( fabs( vPosToStart.y ) > g_vtSpawn_BuddyMaxHeight.GetFloat( 1.0f ))
				{
					fHeightModifier = g_vtSpawn_BuddyHeightMag.GetFloat( 1.0f );
				}


				// Calculate the gravity using the roll off method.
				fGravity = ( gravitySource.m_fBuddyModifier * fHeightModifier ) / ( exp( fDistance * 0.69314718f / g_vtSpawn_BuddyHalfDistance.GetFloat( 1.0f )));

				// If within the min distance, then make it negative gravity to push characters away.  This only
				// applies if the buddy has respawned more than BuddyRespawnGraceTime.
				if( fDistance < g_vtSpawn_BuddyMinDistance.GetFloat( 1.0f ))
				{
					CPlayerObj* pBuddyPlayerObj = CPlayerObj::DynamicCast( gravitySource.m_hObject );
					if( pBuddyPlayerObj )
					{
						// If lastrespawntime is negative, they haven't respawned yet, so it's ok to spawn next to them.
						// If it's been more than BuddyRespawnGraceTime since they respawned, use negative gravity.
						double fLastRespawnTime = pBuddyPlayerObj->GetLastRespawnTime();
						double fCurrentGameTime = GameTimeTimer::Instance( ).GetTimerAccumulatedS( );
						if( fLastRespawnTime >= 0.0f && fCurrentGameTime  > fLastRespawnTime + g_vtSpawn_BuddyRespawnGraceTime.GetFloat( ))
						{
							fGravity *= -1.0f;
						}
					}
				}
			}
		}
		// Enemy.
		else if( bEnemy )
		{
			float fDistance = vGravitySourcePos.Dist( vStartPtPos );

			// Make sure Enemy is within max distance.
			if( g_vtSpawn_EnemyMaxDistance.GetFloat( ) <= 0.0f || fDistance <= g_vtSpawn_EnemyMaxDistance.GetFloat( ))
			{
				fGravity = -1.0f * gravitySource.m_fEnemyModifier * g_vtSpawn_EnemyMag.GetFloat( 1.0f ) / ( exp( fDistance * 0.69314718f / g_vtSpawn_EnemyHalfDistance.GetFloat( 1.0f )));
			}
		}

		// Apply this players gravity contribution.
		validGameStartPoint.m_fGravity += fGravity;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ApplyPlayerGravityToSpawnPoint
//
//	PURPOSE:	Applies gravity from other players to all valid start points.
//
// ----------------------------------------------------------------------- //
static void ApplyPlayerGravityToSpawnPoint( CPlayerObj const& spawningPlayer, ValidGameStartPointList& lst )
{
	double fCurrentTime = SimulationTimer::Instance().GetTimerAccumulatedS( );

	// Now iterate through each character, letting them apply their "gravity".  Teammates
	// apply positive gravity.  Enemy apply negative gravity.
	for( uint32 i = 0; i < CCharacter::GetCharacterList().size(); i++)
	{
		CCharacter const* pOtherCharacter = CCharacter::GetCharacterList()[i];

		// Skip the designated player.
		if( pOtherCharacter == &spawningPlayer )
			continue;

		// Skip dead characters.
		if( !pOtherCharacter->IsAlive( ))
			continue;

		// Skip players without clients.
		CPlayerObj* pOtherPlayer = CPlayerObj::DynamicCast( pOtherCharacter->m_hObject );
		if( pOtherPlayer && !pOtherPlayer->GetClient())
			continue;

		GameStartPointMgr::GravitySource gravitySource;
		gravitySource.m_hObject = ( HOBJECT )pOtherCharacter->m_hObject;
		gravitySource.m_eTeamId = ( ETeamId )pOtherCharacter->GetTeamID();

		// Check if our buddy has been damaged recently.  If so, we wan't
		// to weight his gravity lower so we don't spawn near firefights.
		if( fCurrentTime < const_cast< CCharacter* >( pOtherCharacter )->GetDestructible()->GetLastDamageTime() + 
			g_vtSpawn_BuddyDamageTimeout.GetFloat( ))
		{
			gravitySource.m_fBuddyModifier = g_vtSpawn_BuddyDamageMag.GetFloat( );
		}

		// Find the closest start point to this player and mark it.
		ApplyGravitySourceToSpawnPoint( spawningPlayer, gravitySource, lst );
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ApplyManagedGravitySourcesToSpawnPoint
//
//	PURPOSE:	Applies gravity from managed gravitysources to all valid start points.
//
// ----------------------------------------------------------------------- //
static void ApplyManagedGravitySourcesToSpawnPoint( CPlayerObj const& spawningPlayer, ValidGameStartPointList& lst )
{
	GameStartPointMgr::TManagedGravitySourceList lstGravitySources = GameStartPointMgr::Instance().GetMangedGravitySourceList( );

	// Look for the gravity source based on the object.
	for( GameStartPointMgr::TManagedGravitySourceList::iterator iter = lstGravitySources.begin( ); iter != lstGravitySources.end( ); iter++ )
	{
		GameStartPointMgr::GravitySource& gravitySource = *iter;
		if( gravitySource.m_hObject == NULL )
			continue;

		ApplyGravitySourceToSpawnPoint( spawningPlayer, gravitySource, lst );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpawnByGravityCompare
//
//	PURPOSE:	Binary predicate functor for sorting ValidGameStartPoint's by gravity.
//
// ----------------------------------------------------------------------- //
struct SpawnByGravityCompare
{
	bool operator()( ValidGameStartPoint const& lhs, ValidGameStartPoint const& rhs) const
	{
		// Consider higher gravity to be less than lower gravity so that higher gravity is 
		// first on the list.
		return lhs.m_fGravity > rhs.m_fGravity;
	}
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FindStartPointSpawnByGravity
//
//	PURPOSE:	Finds a start point by applying a "gravity" score
//				to each valid start point from each live player 
//				in the world.
//
// ----------------------------------------------------------------------- //
static GameStartPoint* FindStartPointByGravity( CPlayerObj& player )
{
	// We need at least one start point...
	if( GameStartPoint::GetStartPointList().empty( ))
	{
		return NULL;
	}

	// Filled in by search.
	GameStartPoint* pUseThis = NULL;

	GameClientData* pGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( player.GetClient( ));

	// Build a list of unlocked start points...
	ValidGameStartPointList lstValidStartPts;
	bool bUsedStartPointAlready = pGameClientData->GetLastStartPoint() != NULL;
	if( FindValidGameStartPoints( lstValidStartPts, !bUsedStartPointAlready, player ))
	{
		ShuffleGameStartPoints(lstValidStartPts, lstValidStartPts.size( ));
		// Find the closest start point to this player and mark it.
		ApplyPlayerGravityToSpawnPoint( player, lstValidStartPts );
		ApplyManagedGravitySourcesToSpawnPoint( player, lstValidStartPts );

		// Sort spawn points based on gravity.
		std::sort(lstValidStartPts.begin(),lstValidStartPts.end(), SpawnByGravityCompare( ));

		// Shuffle the top set of spawn points.
		uint32 nTopPoints = ( uint32 )g_vtSpawn_TopNumCandidates.GetFloat( 1.0f );
		nTopPoints = LTMIN( nTopPoints, lstValidStartPts.size( ));
		ShuffleGameStartPoints( lstValidStartPts, nTopPoints );

		// Use the one with the best gravity score.  But if it's the one we last used, then
		// go to the next one.
		for( uint32 nStartPointIndex = 0; nStartPointIndex < nTopPoints; nStartPointIndex++ )
		{
			pUseThis = lstValidStartPts[nStartPointIndex].m_pStartPt;
			if( pUseThis->m_hObject != pGameClientData->GetLastStartPoint( ))
				break;
		}
	}

	// If we didn't find one, then just choose one randomly.
	if( !pUseThis )
	{
		pUseThis = GameStartPoint::GetStartPointList()[GetRandom( 0, GameStartPoint::GetStartPointList( ).size( ) - 1 )];
	}

	pUseThis->SetLastUse( SimulationTimer::Instance().GetTimerAccumulatedS( ));


	return pUseThis;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::FindStartPoint()
//
//	PURPOSE:	Find a good start point.
//
// ----------------------------------------------------------------------- //

GameStartPoint* GameStartPointMgr::FindStartPoint(CPlayerObj* pPlayer)
{
	// We need at least one start point...

	if( GameStartPoint::GetStartPointList().size() == 0 )
	{
		return NULL;
	}

	// Initialize our search table.
	enum SpawnMethod
	{
		eSpawnMethod_First,
		eSpawnMethod_Farthest,
		eSpawnMethod_NearBuddies,
	};
	struct StringToEnum
	{
		CParsedMsg::CToken	m_tokString;
		SpawnMethod			m_eValue;
	};
	static StringToEnum table[] = 
	{
		{ "SpawnFirst", eSpawnMethod_First },
		{ "SpawnFarthest", eSpawnMethod_Farthest },
		{ "SpawnNearBuddies", eSpawnMethod_NearBuddies },
	};
	static uint32 nTableCount = LTARRAYSIZE( table );

	// Find the string location and return the enum.
	CParsedMsg::CToken tokValue = (const char*)GameModeMgr::Instance( ).m_greSpawnPointSelect;
	uint32 i;
	for( i = 0; i < nTableCount; i++ )	
	{
		if( table[i].m_tokString == tokValue )
			break;
	}

	if( i == nTableCount )
		i = 0;

	switch( table[i].m_eValue )
	{
	case eSpawnMethod_First:
		{
			return GameStartPoint::GetStartPointList()[0];
		}
		break;
	case eSpawnMethod_Farthest:
		{
			return FindStartPointByGravity( *pPlayer );
		}
		break;
	case eSpawnMethod_NearBuddies:
		{
			return FindStartPointByGravity( *pPlayer );
		}
		break;
	default:
		{
			LTERROR( "CGameServerShell::FindStartPoint - invalid game mode." );
			return NULL;
		}
		break;
	}
}

#ifndef _FINAL
// Callback function for console command "debugstartpt [<targetplayername>]".  This will
// draw debug lines for spawn points.  Each spawn point will show its name
// and gravity that it is applying to the target player.  Due to the limitations
// of the debugline system and the potential large number of objects it can create,
// the values will not dynamically update.  Instead, just re-run the command.

// Define a container for referencing our debuglines.
struct DebugStartPt
{
	DebugStartPt( )
	{
		m_pDebugLineSystem = NULL;
		m_pGameStartPoint = NULL;
	}
	typedef std::vector< DebugStartPt > List;

	DebugLineSystem* m_pDebugLineSystem;
	GameStartPoint* m_pGameStartPoint;
};

static void DebugStartPtCB(int argc, char **argv)
{

	// List of debugline containers.
	static DebugStartPt::List s_lstDebugStartPts;

	// Find the target of the start point debug information.
	HOBJECT hTarget = NULL;
	if( argc >= 1 && argv[0][0] )
	{
		// Iterate through the clients and find the player.
		ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
		ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
		for( ; iter != gameClientDataList.end( ); iter++ )
		{
			GameClientData* pGameClientData = *iter;
			if( !pGameClientData->GetClient( ))
				continue;

			// See if the name matches the argument.
			if( LTStrIEquals( argv[0], MPW2A( pGameClientData->GetUniqueName()).c_str()))
			{
				hTarget = pGameClientData->GetPlayer();
			}
		}
	}

	// Remove all the old debug information.
	for( DebugStartPt::List::iterator iterStartPt = s_lstDebugStartPts.begin( ); 
		iterStartPt != s_lstDebugStartPts.end( ); iterStartPt++ )
	{
		DebugStartPt& debugStartPt = *iterStartPt;
		g_pLTServer->RemoveObject( debugStartPt.m_pDebugLineSystem->m_hObject );
	}
	s_lstDebugStartPts.clear();

	// Make sure we have a valid target.
	CPlayerObj* pTarget = CPlayerObj::DynamicCast( hTarget );
	if( !pTarget )
		return;

	GameClientData* pGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( pTarget->GetClient( ));

	char szDebugString[256];
	char szName[256];

	// Build a list of valid start points...
	ValidGameStartPointList lstValidStartPts;
	bool bUsedStartPointAlready = pGameClientData->GetLastStartPoint() != NULL;
	if( !FindValidGameStartPoints( lstValidStartPts, !bUsedStartPointAlready, *pTarget ))
		return;

	ShuffleGameStartPoints(lstValidStartPts,lstValidStartPts.size());

	// Find the closest start point to this player and mark it.
	ApplyPlayerGravityToSpawnPoint( *pTarget, lstValidStartPts );
	ApplyManagedGravitySourcesToSpawnPoint( *pTarget, lstValidStartPts );

	// Make a debugline for each start point.  
	s_lstDebugStartPts.reserve( lstValidStartPts.size( ));

	// Write the gravity value as the debug string for each valid startpt.
	DebugStartPt debugStartPt;
	LTVector vPos;
	for( ValidGameStartPointList::iterator iterSP = lstValidStartPts.begin( ); iterSP != lstValidStartPts.end( ); iterSP++ )
	{
		ValidGameStartPoint& validGameStartPoint = *iterSP;

		// Get the name from the start point and make it the root of the debugline name.
		g_pLTServer->GetObjectName( validGameStartPoint.m_pStartPt->m_hObject, szName, LTARRAYSIZE( szName ));
		LTSNPrintF( szDebugString, LTARRAYSIZE( szDebugString ), "%s_dbls", szName );

		// Spawn the new debugline object.
		debugStartPt.m_pDebugLineSystem = DebugLineSystem::Spawn( szDebugString, true );

		// Set the debugline to the same position as the start point.
		g_pLTServer->GetObjectPos( validGameStartPoint.m_pStartPt->m_hObject, &vPos );
		g_pLTServer->SetObjectPos( debugStartPt.m_pDebugLineSystem->m_hObject, vPos );

		// Use a sphere for the debugline.
		debugStartPt.m_pDebugLineSystem->AddSphere( LTVector::GetIdentity(), 20.0f, 4, 4 );

		// Set the debug string from the debugline to be the name of the start point and the gravity.
		float fGravity = LTCLAMP( validGameStartPoint.m_fGravity, -1.0f, 10000.0f );
		LTSNPrintF( szDebugString, LTARRAYSIZE( szDebugString ), "%s (%f)", szName, fGravity );
		debugStartPt.m_pDebugLineSystem->SetDebugString( szDebugString );
		debugStartPt.m_pDebugLineSystem->SetDebugStringPos(LTVector::GetIdentity());

		s_lstDebugStartPts.push_back( debugStartPt );
	}
}
#endif // _FINAL

GameStartPointMgr::TManagedGravitySourceList::iterator GameStartPointMgr::FindGravitySource( HOBJECT hObject )
{
	// Look for the gravity source based on the object.
	TManagedGravitySourceList::iterator iter;
	for( iter = m_lstManagedGravitySources.begin( ); iter != m_lstManagedGravitySources.end( ); iter++ )
	{
		GravitySource& gravitySource = *iter;
		if( gravitySource.m_hObject == hObject )
			break;
	}

	return iter;
}

// Implementing classes will have this function called
// when HOBJECT ref points to gets deleted.
void GameStartPointMgr::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	RemoveManagedGravitySource( hObj );
}

// Adds a gravity source to managed list.
void GameStartPointMgr::AddManagedGravitySource( GravitySource const& gravitySource )
{
	if( !gravitySource.m_hObject )
	{
		LTERROR( "GravitySource object not valid." );
		return;
	}

	// See if it's already in the list.
	TManagedGravitySourceList::iterator iter = FindGravitySource( gravitySource.m_hObject );
	if( iter != m_lstManagedGravitySources.end( ))
	{
		LTERROR( "GravitySource already in managed list." );
		return;
	}

	// Add the gravity source to our list.
	m_lstManagedGravitySources.push_back( gravitySource );
	GravitySource& addedGravitySource = m_lstManagedGravitySources.back( );
	addedGravitySource.m_hObject.SetReceiver( *this );
}

// Removes gravity source based on object.
void GameStartPointMgr::RemoveManagedGravitySource( HOBJECT hObject )
{
	TManagedGravitySourceList::iterator iter = FindGravitySource( hObject );
	if( iter == m_lstManagedGravitySources.end( ))
		return;

	m_lstManagedGravitySources.erase( iter );
}

// Gets the gravity source entry.
GameStartPointMgr::GravitySource* GameStartPointMgr::GetManagedGravitySource( HOBJECT hObject )
{
	TManagedGravitySourceList::iterator iter = FindGravitySource( hObject );
	if( iter == m_lstManagedGravitySources.end( ))
		return NULL;

	GravitySource& gravitySource = *iter;
	return &gravitySource;
}
