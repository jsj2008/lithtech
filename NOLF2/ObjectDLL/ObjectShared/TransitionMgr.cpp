// ----------------------------------------------------------------------- //
//
// MODULE  : TransitionMgr.cpp
//
// PURPOSE : The TransitionMgr implementation
//
// CREATED : 11/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "PlayerObj.h"
#include "WorldProperties.h"
#include "TransitionArea.h"
#include "TransitionMgr.h"
#include "AIStimulusMgr.h"
#include "ServerMissionMgr.h"
#include "ServerSaveLoadMgr.h"

//
// Defines...
//

	#define	TRANSAM_MAX_TRANSOBJS	256


//
// Globals...
//

	CTransitionMgr *g_pTransMgr = LTNULL;

// Prints debug messages during save load.
//#define PRINTDEBUGMESSAGES

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::CTransitionMgr
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTransitionMgr::CTransitionMgr( )
:	m_vTransAreaDims	( 0, 0, 0 ),
	m_hTransArea		( LTNULL )
{
	ASSERT( g_pTransMgr == LTNULL );

	g_pTransMgr = this;
	m_szTransAreaName[0] = '\0';
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::~CTransitionMgr
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CTransitionMgr::~CTransitionMgr( )
{
	g_pTransMgr = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::IsTransitionObject
//
//  PURPOSE:	Is the given object in our transition object list?
//
// ----------------------------------------------------------------------- //

LTBOOL CTransitionMgr::IsTransitionObject( HOBJECT hObj )
{
	if( !hObj )
		return LTFALSE;

	TransitionObjectList::iterator iter = m_TransitionObjectList.begin( );
	while( iter != m_TransitionObjectList.end( ))
	{
		if( hObj == *iter )
			return LTTRUE;

		iter++;
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::TransitionFromArea
//
//  PURPOSE:	Handle the transitioning from an area
//
// ----------------------------------------------------------------------- //

void CTransitionMgr::TransitionFromArea( HOBJECT hTransArea )
{
	// Make sure we have a valid TransitionArea...

	ASSERT( hTransArea != LTNULL );

	// If we have a valid area we shouldn't try and transition any more...

	if( m_hTransArea )
		return;

	g_pLTServer->CPrint( "TRANSITIONING" );


	if( !IsKindOf( hTransArea, "TransitionArea" ))
	{
		g_pLTServer->CPrint( "ERROR! - Trying to transition form a non TransitionArea object." );
		m_hTransArea = LTNULL;
		return;
	}

	// Save the trasition from area object so transition objects can use it...

	m_hTransArea = hTransArea;
	
	TransitionArea *pTransArea = (TransitionArea*)g_pLTServer->HandleToObject( m_hTransArea );
	if( !pTransArea ) return;

	// Save the name and dims of the Transition Area so we know which area to transition to...

	g_pLTServer->GetObjectName( m_hTransArea, m_szTransAreaName, sizeof( m_szTransAreaName ));
	m_vTransAreaDims = pTransArea->GetDims();
	
	// Make sure the level we want to transition to is valid...

	int nTransLevel = pTransArea->GetTransitionLevel();
	if( nTransLevel < 0 )
	{
		g_pLTServer->CPrint( "ERROR! - Trying to transition to an invalid level." );
		m_hTransArea = LTNULL;
		return;
	}

	// Get all objects contained in the Transition Area...

	HOBJECT	hTransObjs[TRANSAM_MAX_TRANSOBJS];

	uint32 dwNumTransObjs = g_pLTServer->GetContainedObjects( m_hTransArea, hTransObjs, TRANSAM_MAX_TRANSOBJS );

	// If there are too many tranistion objects, clip it at the max.
	if( dwNumTransObjs >= TRANSAM_MAX_TRANSOBJS )
	{
		g_pLTServer->CPrint( "Too many transition objects.  Max is %d.  Count is %d.", 
			TRANSAM_MAX_TRANSOBJS, dwNumTransObjs );
		ASSERT( !"CTransitionMgr::TransitionFromArea:  Too many transition objects." );

		dwNumTransObjs = TRANSAM_MAX_TRANSOBJS;
	}
	
	// We must have a player to transition...

	LTBOOL	bTransPlayer = LTFALSE;

	// Make an object list to feed into the addobject functions.
	ObjectList* pTransObjList = g_pLTServer->CreateObjectList();
	if( !pTransObjList )
	{
		ASSERT( !"CTransitionMgr::TransitionFromArea:  Could not create object list." );
		return;
	}

	// Add the transition objects to the list...
	bool bSuccess = true;
	for( uint32 i = 0; i < dwNumTransObjs && bSuccess; ++i )
	{
		if( IsPlayer( hTransObjs[i] ))
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hTransObjs[i] );
			if( !pPlayer || pPlayer->IsDead() ) 
			{
				bSuccess = false;
				break;
			}

			// [RP] 9/15/02 - Because of too many client disconnection issues while transitioning
			//		and carrying Cates body the official decision is to drop bodies while transitioning
			//		in a multiplayer game :(
		
			if( IsMultiplayerGame() )
			{
				pPlayer->DropCarriedObject( true );
			}
			
			// Tell the player to handle exiting...
			pPlayer->HandleExit( false );
			pPlayer->AddToObjectList( pTransObjList );
			//pPlayer->BuildKeepAlives( pTransObjList );

			bTransPlayer = LTTRUE;
		}
		else
		{
			ASSERT( IsGameBase( hTransObjs[i] ));

			if( IsGameBase( hTransObjs[i] ))
			{
				GameBase *pObject = (GameBase*)g_pLTServer->HandleToObject( hTransObjs[i] );
				if( !pObject ) continue;

				// Add it if it's a transitionable...

				if( pObject->CanTransition() )
				{
					g_pLTServer->CPrint( "Transitioning Object %i: %s", i, GetObjectName( hTransObjs[i] ));

					pObject->AddToObjectList( pTransObjList );
				}
			}
		}
	}

	// Clear our list of transition objects.
	m_TransitionObjectList.clear( );
	
	if( bSuccess )
	{
		// Put the objects in the objectlist into our persistant list of LTObjRef's.  We
		// need the list to be objref's because it could be a couple frames before we actually
		// save them out.
		ObjectLink *pObjLink = pTransObjList->m_pFirstLink;
		while( pObjLink && pObjLink->m_hObject )
		{
			m_TransitionObjectList.push_back( pObjLink->m_hObject );
			pObjLink = pObjLink->m_pNext;
		}
	}

	g_pLTServer->RelinquishList( pTransObjList );
	pTransObjList = NULL;

	if( !bSuccess )
		return;

	if( !bTransPlayer )
	{
		g_pLTServer->CPrint( "ERROR! - Trying to transition without a player." );
		m_hTransArea = LTNULL;
		return;
	}

	g_pServerMissionMgr->ExitLevelTransition( nTransLevel );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::PreTransitionLoad
//
//  PURPOSE:	Handle transitioning from a level...
//
// ----------------------------------------------------------------------- //

LTBOOL CTransitionMgr::PreTransitionLoad( char const* pSaveLevelFile )
{
	if( !m_hTransArea || !pSaveLevelFile )
		return LTFALSE;

	// Save all transition objects...

	if( !SaveTransitionObjects() )
		return LTFALSE;

	// Remove all the transition objects so they don't get saved indirectly
	// by the non-transition objects.
	if( !RemoveTransitionObjects() )
		return LTFALSE;

	// Save all other level objects...

	if( !SaveNonTransitionObjects( pSaveLevelFile ))
		return LTFALSE;
	
	// Clear our list of transition objects.
	m_TransitionObjectList.clear( );

	m_hTransArea = LTNULL;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::PostTransitionLoad
//
//  PURPOSE:	Handle transitioning to a level...
//
// ----------------------------------------------------------------------- //

LTBOOL CTransitionMgr::PostTransitionLoad( char const* pRestoreLevelFile )
{
	if( m_hTransArea ) return LTFALSE;

	// Load all the level objects...

	if( !LoadNonTransitionObjects( pRestoreLevelFile ))
		return LTFALSE;

	// Make sure we have a valid area to transition to...

	if( m_szTransAreaName[0] == '\0' ) return LTFALSE;

	ObjArray<HOBJECT, 1> objArray;
	g_pLTServer->FindNamedObjects( m_szTransAreaName, objArray );

	if( objArray.NumObjects() != 1 )
	{
		g_pLTServer->CPrint( "ERROR! - No valid area to transition to!" );
		return LTFALSE;
	}

	if( !IsKindOf( objArray.GetObject(0), "TransitionArea" ))
	{
		g_pLTServer->CPrint( "ERROR! - Trying to transition form a non TransitionArea object." );
		return LTFALSE;
	}

	// Cache the trasition to area object so transition objects can use it...

	m_hTransArea = objArray.GetObject(0);
	
	TransitionArea *pTransArea = (TransitionArea*)g_pLTServer->HandleToObject( m_hTransArea );
	if( !pTransArea ) return LTFALSE;

	// Make sure the dims are practiclly the same so the positions and rotations will be acurate...
	
	if( !pTransArea->GetDims().NearlyEquals( m_vTransAreaDims, 0.01f ))
	{
		g_pLTServer->CPrint( "ERROR! - Transition to area has different dims than Transition from area!" );
		return LTFALSE;
	}

	// Load all transitioned objects...

	if( !LoadTransitionObjects( ))
		return LTFALSE;

	m_hTransArea = LTNULL;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::SaveTransitionObjects
//
//  PURPOSE:	Save the transition objects...
//
// ----------------------------------------------------------------------- //

LTBOOL CTransitionMgr::SaveTransitionObjects( )
{

	ObjectList* pTransObjList = g_pLTServer->CreateObjectList();
	if( !pTransObjList )
	{
		ASSERT( !"CTransitionMgr::SaveTransitionObjects:  Could not create object list." );
		return LTFALSE;
	}
	
	// Transfer the objects from the ltobjref list to the ObjectList.
	TransitionObjectList::iterator iter = m_TransitionObjectList.begin( );
	while( iter != m_TransitionObjectList.end( ))
	{
		// We don't have to check for duplicates, since they have been weeded
		// out when we created the list in the first place.
		::AddObjectToList( pTransObjList, *iter, eObjListDuplicatesOK );
		iter++;
	}

	// Save the transition objects...

	bool bSuccess = true;
	if( pTransObjList->m_nInList > 0 )
	{
		uint8 nOldFlags = g_pGameServerShell->GetLGFlags( );
		g_pGameServerShell->SetLGFlags( LOAD_TRANSITION );

		LTRESULT res = g_pLTServer->SaveObjects( const_cast< char* >( g_pServerSaveLoadMgr->GetTransitionFile( )), pTransObjList, LOAD_TRANSITION, SAVEOBJECTS_SAVEGAMECONSOLE );

		g_pGameServerShell->SetLGFlags( nOldFlags );
		
		if( res != LT_OK )
		{
			g_pLTServer->CPrint( "ERROR! - Couldn't save transition objects." );
			bSuccess = false;
		}

		g_pGameServerShell->SetLGFlags( nOldFlags );
	}
	

	g_pLTServer->RelinquishList( pTransObjList );
	pTransObjList = NULL;

	if( !bSuccess )
		return LTFALSE;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::LoadTransitionObjects
//
//  PURPOSE:	Restore the transition objects... 
//
// ----------------------------------------------------------------------- //

LTBOOL CTransitionMgr::LoadTransitionObjects( )
{
	uint8 nOldFlags = g_pGameServerShell->GetLGFlags( );
	g_pGameServerShell->SetLGFlags( LOAD_TRANSITION );

	LTRESULT res = g_pLTServer->RestoreObjects( const_cast< char* >( g_pServerSaveLoadMgr->GetTransitionFile( )), LOAD_TRANSITION, 0);
	
	g_pGameServerShell->SetLGFlags( nOldFlags );

	if( res != LT_OK )
	{
		g_pLTServer->CPrint( "ERROR! - Couldn't load transition objects." );
		return LTFALSE;
	}


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::RemoveTransitionObjects
//
//  PURPOSE:	Remove all the transition objects so they don't get
//				saved indirectly by the non-transition objects.
//
// ----------------------------------------------------------------------- //

LTBOOL CTransitionMgr::RemoveTransitionObjects( )
{
	TransitionObjectList::iterator iter = m_TransitionObjectList.begin( );
	while( iter != m_TransitionObjectList.end( ))
	{
		g_pLTServer->RemoveObject( *iter );
		iter++;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::SaveNonTransitionObjects
//
//  PURPOSE:	Save all objects in the level that are NOT in the transition list...
//
// ----------------------------------------------------------------------- //

LTBOOL CTransitionMgr::SaveNonTransitionObjects( char const *pSaveLevelFile )
{
	if( !pSaveLevelFile )
		return LTFALSE;

	ObjectList	*pNonTransObjList = g_pLTServer->CreateObjectList();
	if( !pNonTransObjList )
		return LTFALSE;

	// Save depends on the global WorldProperties object being valid (i.e.,
	// every level MUST have a WorldProperties object).

	if( !g_pWorldProperties )
		return LTFALSE;

	// Add active objects to the list...

	int i = 0;
    HOBJECT hObj = g_pLTServer->GetNextObject( LTNULL );
	while( hObj )
	{
		if( hObj != g_pWorldProperties->m_hObject && !IsTransitionObject( hObj ))
		{
#ifdef PRINTDEBUGMESSAGES
            g_pLTServer->CPrint( "SAVING - Non-Trans Object %i: %s", i++, GetObjectName( hObj ));
#endif // PRINTDEBUGMESSAGES
			g_pLTServer->AddObjectToList( pNonTransObjList, hObj );
		}

        hObj = g_pLTServer->GetNextObject( hObj );
	}

	// Add inactive objects to the list...

    hObj = g_pLTServer->GetNextInactiveObject( LTNULL );
	while( hObj )
	{
		if( hObj != g_pWorldProperties->m_hObject && !IsTransitionObject( hObj ))
		{
#ifdef PRINTDEBUGMESSAGES
            g_pLTServer->CPrint( "SAVING - Non-Trans Object %i: %s", i++, GetObjectName( hObj ));
#endif // PRINTDEBUGMESSAGES
			g_pLTServer->AddObjectToList( pNonTransObjList, hObj );
		}

        hObj = g_pLTServer->GetNextInactiveObject( hObj );
	}


	// Make sure the WorldProperties object is saved FIRST, this way all the global
	// data will be available for the other objects when they get restored.
	// (ServerDE::AddObjectsToList() adds to the front of the list, so we
	// need to add it last ;)...

#ifdef PRINTDEBUGMESSAGES
    g_pLTServer->CPrint( "SAVING - Non-Trans Object %i: %s", i++, GetObjectName( g_pWorldProperties->m_hObject ));
#endif // PRINTDEBUGMESSAGES
	g_pLTServer->AddObjectToList( pNonTransObjList, g_pWorldProperties->m_hObject );


	if( pNonTransObjList && pNonTransObjList->m_nInList > 0 )
	{
		uint8 nOldFlags = g_pGameServerShell->GetLGFlags( );
		g_pGameServerShell->SetLGFlags( LOAD_RESTORE_GAME );

        LTRESULT res = g_pLTServer->SaveObjects(( char* )pSaveLevelFile, pNonTransObjList, LOAD_RESTORE_GAME,
												 SAVEOBJECTS_SAVEGAMECONSOLE);

		g_pGameServerShell->SetLGFlags( nOldFlags );

		if( res != LT_OK )
		{
			g_pLTServer->CPrint( "ERROR! - Couldn't Save Non-Transition objects." );
			return LTFALSE;
		}
	}

	g_pLTServer->RelinquishList( pNonTransObjList );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::LoadNonTransitionObjects
//
//  PURPOSE:	Restore objects in the level...
//
// ----------------------------------------------------------------------- //

LTBOOL CTransitionMgr::LoadNonTransitionObjects( char const *pRestoreLevelFile )
{
	// If we dont have a propper restore file we should have already loaded the world objects...
	
	if( !pRestoreLevelFile || !pRestoreLevelFile[0] ) 
		return LTTRUE;

	uint8 nOldFlags = g_pGameServerShell->GetLGFlags( );
	g_pGameServerShell->SetLGFlags( LOAD_RESTORE_GAME );

	LTRESULT res = g_pLTServer->RestoreObjects(( char* ) pRestoreLevelFile, LOAD_RESTORE_GAME, 0 );
	if( res != LT_OK )
	{
		g_pLTServer->CPrint( "ERROR! - Couldn't load Non-Transition objects." );
		return LTFALSE;
	}

	g_pGameServerShell->SetLGFlags( nOldFlags );

	return LTTRUE;
}