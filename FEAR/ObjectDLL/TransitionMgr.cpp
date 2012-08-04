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

#include "Stdafx.h"
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

	CTransitionMgr *g_pTransMgr = NULL;

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
	m_hTransArea		( NULL )
{
	ASSERT( g_pTransMgr == NULL );

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
	g_pTransMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::IsTransitionObject
//
//  PURPOSE:	Is the given object in our transition object list?
//
// ----------------------------------------------------------------------- //

bool CTransitionMgr::IsTransitionObject( HOBJECT hObj )
{
	if( !hObj )
		return false;

	TransitionObjectList::iterator iter = m_TransitionObjectList.begin( );
	while( iter != m_TransitionObjectList.end( ))
	{
		if( hObj == *iter )
			return true;

		iter++;
	}

	return false;
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

	ASSERT( hTransArea != NULL );

	// If we have a valid area we shouldn't try and transition any more...

	if( m_hTransArea )
		return;

	g_pLTServer->CPrint( "TRANSITIONING" );


	if( !IsKindOf( hTransArea, "TransitionArea" ))
	{
		g_pLTServer->CPrint( "ERROR! - Trying to transition form a non TransitionArea object." );
		m_hTransArea = NULL;
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
		m_hTransArea = NULL;
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

	bool	bTransPlayer = false;

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
			if( !pPlayer || !pPlayer->IsAlive() ) 
			{
				bSuccess = false;
				break;
			}

			// Tell the player to handle exiting...
			pPlayer->HandleExit( false );
			pPlayer->AddToObjectList( pTransObjList );
			//pPlayer->BuildKeepAlives( pTransObjList );

			bTransPlayer = true;
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
		m_hTransArea = NULL;
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

bool CTransitionMgr::PreTransitionLoad( char const* pSaveLevelFile )
{
	if( !m_hTransArea || !pSaveLevelFile )
		return false;

	// Save all transition objects...

	if( !SaveTransitionObjects() )
		return false;

	// Remove all the transition objects so they don't get saved indirectly
	// by the non-transition objects.
	if( !RemoveTransitionObjects() )
		return false;

	// Save all other level objects...

	if( !SaveNonTransitionObjects( pSaveLevelFile ))
		return false;
	
	// Clear our list of transition objects.
	m_TransitionObjectList.clear( );

	m_hTransArea = NULL;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::PostTransitionLoad
//
//  PURPOSE:	Handle transitioning to a level...
//
// ----------------------------------------------------------------------- //

bool CTransitionMgr::PostTransitionLoad( char const* pRestoreLevelFile )
{
	if( m_hTransArea ) return false;

	// Load all the level objects...

	if( !LoadNonTransitionObjects( pRestoreLevelFile ))
		return false;

	// Make sure we have a valid area to transition to...

	if( m_szTransAreaName[0] == '\0' ) return false;

	ObjArray<HOBJECT, 1> objArray;
	g_pLTServer->FindNamedObjects( m_szTransAreaName, objArray );

	if( objArray.NumObjects() != 1 )
	{
		g_pLTServer->CPrint( "ERROR! - No valid area to transition to!" );
		return false;
	}

	if( !IsKindOf( objArray.GetObject(0), "TransitionArea" ))
	{
		g_pLTServer->CPrint( "ERROR! - Trying to transition form a non TransitionArea object." );
		return false;
	}

	// Cache the trasition to area object so transition objects can use it...

	m_hTransArea = objArray.GetObject(0);
	
	TransitionArea *pTransArea = (TransitionArea*)g_pLTServer->HandleToObject( m_hTransArea );
	if( !pTransArea ) return false;

	// Make sure the dims are practiclly the same so the positions and rotations will be acurate...
	
	if( !pTransArea->GetDims().NearlyEquals( m_vTransAreaDims, 0.01f ))
	{
		g_pLTServer->CPrint( "ERROR! - Transition to area has different dims than Transition from area!" );
		return false;
	}

	// Load all transitioned objects...

	if( !LoadTransitionObjects( ))
		return false;

	m_hTransArea = NULL;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::SaveTransitionObjects
//
//  PURPOSE:	Save the transition objects...
//
// ----------------------------------------------------------------------- //

bool CTransitionMgr::SaveTransitionObjects( )
{

	ObjectList* pTransObjList = g_pLTServer->CreateObjectList();
	if( !pTransObjList )
	{
		ASSERT( !"CTransitionMgr::SaveTransitionObjects:  Could not create object list." );
		return false;
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

		LTRESULT res = g_pLTServer->SaveObjects( const_cast< char* >( g_pServerSaveLoadMgr->GetTransitionFile( g_pServerSaveLoadMgr->GetProfileName() ) ), pTransObjList, LOAD_TRANSITION, 0 );

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
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::LoadTransitionObjects
//
//  PURPOSE:	Restore the transition objects... 
//
// ----------------------------------------------------------------------- //

bool CTransitionMgr::LoadTransitionObjects( )
{
	uint8 nOldFlags = g_pGameServerShell->GetLGFlags( );
	g_pGameServerShell->SetLGFlags( LOAD_TRANSITION );

	LTRESULT res = g_pLTServer->RestoreObjects( const_cast< char* >( g_pServerSaveLoadMgr->GetTransitionFile( g_pServerSaveLoadMgr->GetProfileName() )), LOAD_TRANSITION, 0);
	
	g_pGameServerShell->SetLGFlags( nOldFlags );

	if( res != LT_OK )
	{
		g_pLTServer->CPrint( "ERROR! - Couldn't load transition objects." );
		return false;
	}


	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::RemoveTransitionObjects
//
//  PURPOSE:	Remove all the transition objects so they don't get
//				saved indirectly by the non-transition objects.
//
// ----------------------------------------------------------------------- //

bool CTransitionMgr::RemoveTransitionObjects( )
{
	TransitionObjectList::iterator iter = m_TransitionObjectList.begin( );
	while( iter != m_TransitionObjectList.end( ))
	{
		g_pLTServer->RemoveObject( *iter );
		iter++;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::SaveNonTransitionObjects
//
//  PURPOSE:	Save all objects in the level that are NOT in the transition list...
//
// ----------------------------------------------------------------------- //

bool CTransitionMgr::SaveNonTransitionObjects( char const *pSaveLevelFile )
{
	if( !pSaveLevelFile )
		return false;

	ObjectList	*pNonTransObjList = g_pLTServer->CreateObjectList();
	if( !pNonTransObjList )
		return false;

	// Save depends on the global WorldProperties object being valid (i.e.,
	// every level MUST have a WorldProperties object).

	if( !g_pWorldProperties )
		return false;

	// Add active objects to the list...

    HOBJECT hObj = g_pLTServer->GetNextObject( NULL );
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

    hObj = g_pLTServer->GetNextInactiveObject( NULL );
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
												 SAVEOBJECTS_SAVEGAMECONSOLE | SAVEOBJECTS_SAVEGAMESTATE);

		g_pGameServerShell->SetLGFlags( nOldFlags );

		if( res != LT_OK )
		{
			g_pLTServer->CPrint( "ERROR! - Couldn't Save Non-Transition objects." );
			return false;
		}
	}

	g_pLTServer->RelinquishList( pNonTransObjList );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTransitionMgr::LoadNonTransitionObjects
//
//  PURPOSE:	Restore objects in the level...
//
// ----------------------------------------------------------------------- //

bool CTransitionMgr::LoadNonTransitionObjects( char const *pRestoreLevelFile )
{
	// If we dont have a propper restore file we should have already loaded the world objects...
	
	if( !pRestoreLevelFile || !pRestoreLevelFile[0] ) 
		return true;

	uint8 nOldFlags = g_pGameServerShell->GetLGFlags( );
	g_pGameServerShell->SetLGFlags( LOAD_RESTORE_GAME );

	LTRESULT res = g_pLTServer->RestoreObjects(( char* ) pRestoreLevelFile, LOAD_RESTORE_GAME, 0 );
	if( res != LT_OK )
	{
		g_pLTServer->CPrint( "ERROR! - Couldn't load Non-Transition objects." );
		return false;
	}

	g_pGameServerShell->SetLGFlags( nOldFlags );

	return true;
}
