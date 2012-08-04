
//----------------------------------------------------------------------------
//
//	MODULE:		RelationMgr.cpp
//
//	PURPOSE:	- implementation
//
//	CREATED:	30.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	Global RelationMgr which handled lookups of the individual
//				system.
//
//
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "RelationMgr.h"
#include "RelationButeMgr.h"
#include "CollectiveRelationMgr.h"
#include "AITypes.h"
#include "AIAssert.h"
#include "ObjectRelationMgr.h"

#include <algorithm>

// Forward declarations

// Globals

// Statics

// Singleton instance accessed through the static CRelationMgr::GetGlobalRelationMgr() call
CRelationMgr* CRelationMgr::m_pSingleInstance = NULL;


// Returns true if the collectives are the same (Keys are equivelent)
struct CollectiveMatch
: std::binary_function<CCollectiveRelationMgr*, CCollectiveRelationMgr*, bool>
{
	bool operator()( CCollectiveRelationMgr* pTest, CCollectiveRelationMgr* pTest2 ) const
	{
		UBER_ASSERT( pTest != NULL &&  pTest2 != NULL, "Testing for Name Collective with NULL Collective" );
		bool b = strcmp( pTest->GetKey(), pTest2->GetKey() ) == 0;
		return b;
	}
};

struct CollectiveKeyMatch
: std::binary_function<CCollectiveRelationMgr*, const char* const, bool>
{
	bool operator()( CCollectiveRelationMgr* pTest, const char* const szKey ) const
	{
		UBER_ASSERT( pTest != NULL, "Testing for Name Collective with NULL Collective" );
		return ( strcmp( pTest->GetKey(), szKey ) == 0 );
	}
};


//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationMgr::CRelationMgr()
//
//	PURPOSE:	Constructor to handle any initialization
//
//----------------------------------------------------------------------------
CRelationMgr::CRelationMgr() :
	m_pRelationButeMgr(NULL )
{
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationMgr::~CRelationMgr()
//
//	PURPOSE:	Destructor to handle any cleanup.  Free all resources, just in
//				case Term was not called.
//
//----------------------------------------------------------------------------
CRelationMgr::~CRelationMgr()
{
	// This is probably insurance code.  The Term should always have
	// already cleaned everything this class allocated.
	FreeAllResources();
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationMgr::Save()/Load()
//
//	PURPOSE:	Handle the saving and loading of the RelationMgr.  This will
//				handle saving and restoring the collectives.
//
//----------------------------------------------------------------------------
void CRelationMgr::Save(ILTMessage_Write *pMsg)
{
	// Save each of the collectives
	SAVE_INT( m_listCollectives.size() );
	std::for_each( m_listCollectives.begin(),
		m_listCollectives.end(),
		std::bind2nd( std::mem_fun1(&CCollectiveRelationMgr::Save), pMsg ));
}
void CRelationMgr::Load(ILTMessage_Read *pMsg)
{
	// Load the Collective count, then allocate and load each
	int nCollectives;
	LOAD_INT( nCollectives );
	for ( int x = 0; x < nCollectives; x++ )
	{
		CCollectiveRelationMgr* pCollective = debug_new( CCollectiveRelationMgr );
		pCollective->Load(pMsg);
		m_listCollectives.push_back(pCollective);
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationMgr::GetGlobalRelationMgr()
//
//	PURPOSE:	Created a new instance of the RelationMgr, and returns it if
//				one hasn't been created yet.  Otherwise, just returns it.
//
//----------------------------------------------------------------------------
CRelationMgr* CRelationMgr::GetGlobalRelationMgr()
{
	if ( m_pSingleInstance == NULL )
	{
		m_pSingleInstance = debug_new ( CRelationMgr );
	}

	return m_pSingleInstance;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationMgr::GetButeMgr()
//
//	PURPOSE:	Returns a pointer to the RelationMgrs bute parser
//
//----------------------------------------------------------------------------
CRelationButeMgr* CRelationMgr::GetButeMgr()
{
	UBER_ASSERT( m_pRelationButeMgr != NULL, "Attempting to retreive NULL ButeMgr" );
	return m_pRelationButeMgr;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationMgr::Init()
//
//	PURPOSE:	Sets up and initializes the RelationMgr.
//
//----------------------------------------------------------------------------
LTBOOL CRelationMgr::Init()
{
	m_pRelationButeMgr = debug_new ( CRelationButeMgr );
	m_pRelationButeMgr->Init();
	return LTTRUE;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationMgr::Term()
//
//	PURPOSE:	Cleans up the RelationMgr instance
//
//----------------------------------------------------------------------------
void CRelationMgr::Term()
{
	FreeAllResources();

	// Clean up the Singleton instance
	debug_delete( m_pSingleInstance );
	m_pSingleInstance = NULL;
}


//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationMgr::PerWorldInit()
//
//	PURPOSE:	Does any per world setup.
//
//----------------------------------------------------------------------------
void CRelationMgr::PerWorldInit()
{
	;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationMgr::PerWorldTerm()
//
//	PURPOSE:	Does any per world cleanup.  Currently, it deallocates the
//				collectives, and clears out the RelationMgr lists.
//
//----------------------------------------------------------------------------
void CRelationMgr::PerWorldTerm()
{
	_listCollectives::iterator itCollective;
	do
	{
		itCollective = m_listCollectives.begin();
		if ( itCollective != m_listCollectives.end() )
		{
			DeleteAndRemove( itCollective );
			itCollective = m_listCollectives.begin();
		}
	}
	while ( itCollective != m_listCollectives.end() );
	
	_listpObjectRelationMgr::iterator itRelationInstance;
	do
	{
		itRelationInstance = m_listpObjectRelationMgrs.begin();
		if ( itRelationInstance != m_listpObjectRelationMgrs.end() )
		{
			RemoveObjectRelationMgr( itRelationInstance );
			itRelationInstance = m_listpObjectRelationMgrs.begin();
		}
	}
	while ( itRelationInstance != m_listpObjectRelationMgrs.end() );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::FreeAllResources()
//              
//	PURPOSE:	Method to centralize the deletion of all allocated resources.
//				1) Deletes the RelationButeMgr
//				2) Removes all of the registered Collectives
//              3) Removes all of the registered ObjectRelationMgr instances
//              
//----------------------------------------------------------------------------
void CRelationMgr::FreeAllResources()
{
	PerWorldTerm();

	if ( m_pRelationButeMgr != NULL )
	{
		m_pRelationButeMgr->Term();
		debug_delete ( m_pRelationButeMgr );
		m_pRelationButeMgr = NULL;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::AddCollective()
//              
//	PURPOSE:	Adds a new collective to the list of known collectives.
//              
//----------------------------------------------------------------------------
void CRelationMgr::AddCollective( CCollectiveRelationMgr* pCollective )
{
	_listCollectives::const_iterator itBegin = m_listCollectives.begin();
	_listCollectives::const_iterator itEnd = m_listCollectives.end();
	_listCollectives::const_iterator itFound = std::find_if(itBegin, itEnd, std::bind2nd( CollectiveMatch(), pCollective ) );

	UBER_ASSERT( itFound == m_listCollectives.end(), "AddCollective: Attempted duplicate addition of collective" );
	m_listCollectives.push_back(pCollective);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::AddObjectRelationMgrInstance()
//              
//	PURPOSE:	Adds a new ObjectRelationMgr instance to the list of instances.
//              
//----------------------------------------------------------------------------
void CRelationMgr::AddObjectRelationMgr(CObjectRelationMgr* pObjectRelationMgr, HOBJECT hObject )
{
	AIASSERT( !IsObjectRelationMgrLinked(pObjectRelationMgr), hObject, "AddCollective: Attempted duplicate addition of collective" );
	m_listpObjectRelationMgrs.push_back( const_cast<CObjectRelationMgr*>(pObjectRelationMgr) );
	LinkObjectRelationMgr( pObjectRelationMgr, hObject );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::LinkObjectRelationMgr()
//              
//	PURPOSE:	Links the ObjectRelationMgr instance to the RelationMgrs' name
//				and object based lookups.
//              
//----------------------------------------------------------------------------
void CRelationMgr::LinkObjectRelationMgr(CObjectRelationMgr* pORM, HOBJECT hObject)
{
	// Make sure they aren't inserting a null object.
	if( !hObject )
	{
		UBER_ASSERT( hObject, "CRelationMgr::LinkORM: NULL HOBJECT" );
		return;
	}

	// Check if we already have this object.
	if( m_mapObjectToORM.find( hObject ) != m_mapObjectToORM.end( ))
	{
		UBER_ASSERT( 0,	"CRelationMgr::LinkORM: Duplicate HObject insertion attempted" );
		return;
	}

	// Put it in our maps.
	ObjectToORMMapEntry& entry = m_mapObjectToORM[hObject];
	entry.m_pORM = pORM;
	entry.m_hObjRef.SetReceiver( *this );
	entry.m_hObjRef = hObject;

	m_mapStringToORM.insert( std::make_pair( std::string(ToString(hObject)), pORM ));
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::IsObjectRelationMgrLinked()
//              
//	PURPOSE:	Returns true if the passed in ObjectRelationMgr instance is
//				linked, false if it is not.
//              
//----------------------------------------------------------------------------
bool CRelationMgr::IsObjectRelationMgrLinked(const CObjectRelationMgr* const pORM) const
{
	_listpObjectRelationMgr::const_iterator itBegin = m_listpObjectRelationMgrs.begin();
	_listpObjectRelationMgr::const_iterator itEnd = m_listpObjectRelationMgrs.end();
	_listpObjectRelationMgr::const_iterator itFound = std::find(itBegin, itEnd, pORM);
	return ( itFound != m_listpObjectRelationMgrs.end() );
}
											 

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::UnlinkObjectRelationMgr
//              
//	PURPOSE:	Fairly ugly way to remove from the Lookup maps the passed in
//				ORM.  A faster approach would be nice, but may not be critical.
//				Iterates over the map, removing all instances of the pointer
//				(there should only be 1!)
//              
//----------------------------------------------------------------------------
void CRelationMgr::UnlinkObjectRelationMgr(const CObjectRelationMgr* const pORM)
{
	_mapObjectToORM::iterator itMapObject = m_mapObjectToORM.begin();
	while ( itMapObject != m_mapObjectToORM.end() )
	{
		// Convert to pointer for debugging clarity
		const CObjectRelationMgr* const pFound = (*itMapObject).second.m_pORM;
		if ( &(*pFound) == &(*pORM) )
		{
			m_mapObjectToORM.erase(itMapObject);
			itMapObject = m_mapObjectToORM.begin();
		}
		else
		{
			itMapObject++;
		}
	}

	_mapStringToORM::iterator itMapString = m_mapStringToORM.begin();
	while ( itMapString != m_mapStringToORM.end() )
	{
		// Convert to pointer for debugging clarity
		const CObjectRelationMgr* const pFound = (*itMapString).second;
		if ( &(*pFound) == &(*pORM) )
		{
			m_mapStringToORM.erase(itMapString);
			itMapString = m_mapStringToORM.begin();
		}
		else
		{
			itMapString++;
		}
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::RemoveCollective()
//              
//	PURPOSE:	Removes 
//              
//----------------------------------------------------------------------------
void CRelationMgr::RemoveCollective(CCollectiveRelationMgr* pCollective)
{
	UBER_ASSERT( pCollective!=NULL, "Attempted to remove a null Collective" );
	
	_listCollectives::iterator it = std::find(
		m_listCollectives.begin(),
		m_listCollectives.end(),
		pCollective );

	if ( it == m_listCollectives.end() )
	{
		UBER_ASSERT1( 0, "RemoveCollective: Attempted to remove collective not in list: %s", pCollective->GetKey() );
	}

	m_listCollectives.erase( it );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::RemoveObjectRelationMgr()
//              
//	PURPOSE:	Removes and unlinks from the lookups the passed in ORM if one 
//				exists.
//              
//----------------------------------------------------------------------------
void CRelationMgr::RemoveObjectRelationMgr(CObjectRelationMgr* pORM)
{
	UBER_ASSERT( pORM!=NULL, "Attempted to remove a null Collective" );

	_listpObjectRelationMgr::iterator it = std::find(
		m_listpObjectRelationMgrs.begin(),
		m_listpObjectRelationMgrs.end(),
		pORM );

	RemoveObjectRelationMgr( it );
}

void CRelationMgr::RemoveObjectRelationMgr(_listpObjectRelationMgr::iterator it)
{
	if ( it != m_listpObjectRelationMgrs.end() )
	{
		UnlinkObjectRelationMgr( (*it) );
		m_listpObjectRelationMgrs.erase( it );
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::FindCollective()
//              
//	PURPOSE:	Passes back a pointer to a collective with the same name as the
//				Key.  If not found, creates and returns it.
//              
//----------------------------------------------------------------------------
CCollectiveRelationMgr* CRelationMgr::FindCollective(const char* const szName)
{
	// Check to see if it is in our existing list and return it if it is.
	_listCollectives::iterator itFound = std::find_if(
		m_listCollectives.begin(),
		m_listCollectives.end(),
		std::bind2nd( CollectiveKeyMatch(), szName ) );

	if ( itFound != m_listCollectives.end() )
	{
		return (*itFound);
	}

	// Check to see if we have a template that will allow the creation of it.
	// If we do, then create it, add it, and return it.
	// Otherwise GetCollectiveTemplateIDByName will assert if it does not exist
	CCollectiveRelationMgr* pCollective = debug_new( CCollectiveRelationMgr );
	pCollective->Init( szName );
	AddCollective( pCollective );

	return pCollective;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::GetObjectRelationMgr()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
CObjectRelationMgr* CRelationMgr::GetObjectRelationMgr(const char* const szName)
{
	if ( szName == NULL )
	{
		return NULL;
	}

	_mapStringToORM::iterator it = m_mapStringToORM.find(std::string((char*)szName));
	if ( it == m_mapStringToORM.end() )
	{
		return NULL;
	}
	else
	{
		CObjectRelationMgr* pFound = (*it).second;
		HOBJECT hObj = pFound->GetOwningHObject();
		return pFound;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::GetObjectRelationMgr()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
CObjectRelationMgr* CRelationMgr::GetObjectRelationMgr(HOBJECT HInstance)
{
	_mapObjectToORM::iterator it = m_mapObjectToORM.find(HInstance);

	if ( it == m_mapObjectToORM.end() )
	{
		return NULL;
	}
	else
	{
		CObjectRelationMgr* pFound = (*it).second.m_pORM;
		return pFound;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CRelationMgr::DeleteAndRemove()
//              
//	PURPOSE:	Deallocate and removes a collective
//              
//----------------------------------------------------------------------------
void CRelationMgr::DeleteAndRemove(_listCollectives::iterator it)
{
	UBER_ASSERT( it != m_listCollectives.end(), "Attempted to erase an invalid Collective" );

	CCollectiveRelationMgr* pCollective = *it;
	UBER_ASSERT( pCollective, "Attempted to erase an NULL Collective pointer" );

	m_listCollectives.erase( it );
	debug_delete( pCollective );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CRelationMgr::OnLinkBroken
//
//  PURPOSE:	Handle the case of a link going bad.
//
// ----------------------------------------------------------------------- //

void CRelationMgr::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	// Remove it from our ORM map.  We'll clean it out of our store
	// map later.  We can't do it now since the ref object needs to be
	// valid after returning from this function.
	// Take the object out of our maps.
	CObjectRelationMgr* pORM = GetObjectRelationMgr( hObj );
	if( !pORM )
		return;

	UnlinkObjectRelationMgr( pORM );
}
