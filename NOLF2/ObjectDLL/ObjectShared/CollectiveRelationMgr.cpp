//----------------------------------------------------------------------------
//
//	MODULE:		CollectiveRelationMgr.cpp
//
//	PURPOSE:	CCollectiveRelationMgr implementation
//
//	CREATED:	21.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//
//
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "CollectiveRelationMgr.h"
#include "ObjectRelationMgr.h"
#include "RelationMgr.h"
#include "AIAssert.h"
#include "ServerUtilities.h"

#include <algorithm>

// Forward declarations

// Globals

// Statics


struct RemoveRelationFromObjectRelationMgr :
public std::binary_function< IMomentoUser*, const RelationDescription, bool>
{
public:
	bool operator()(IMomentoUser* pIUser, const RelationDescription RD) const
	{
		pIUser->RemoveRelationCallback(RD);
		return true;
	}
};

//----------------------------------------------------------------------------
//
//	ROUTINE:	CCollectiveRelationMgr::CCollectiveRelationMgr()
//
//	PURPOSE:	Link the Collective up with the RelationMgrs list of
//				collectives.
//
//----------------------------------------------------------------------------
CCollectiveRelationMgr::CCollectiveRelationMgr()
{
	// Construct
	m_flLastUpdate = 0;
	m_pRelationUser = debug_new1( CRelationUser, this );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CCollectiveRelationMgr::~CCollectiveRelationMgr()
//
//	PURPOSE:
//				ToDo!: Should notify the members (if any) that this collective
//				no longer exists to avoid dangling pointers!!!  Currently
//				passable as collectives are destroyed when the game level
//				ends, but still nasty
//
//----------------------------------------------------------------------------
CCollectiveRelationMgr::~CCollectiveRelationMgr()
{
	// Destruct
	debug_delete( m_pRelationUser );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CCollectiveRelationMgr::operator=()
//
//	PURPOSE:	Create a CCollectiveRelationMgr from a template
//
//----------------------------------------------------------------------------
CCollectiveRelationMgr& CCollectiveRelationMgr::operator=(const Collective_Template& rhs )
{
	LTStrCpy( m_szName, rhs.m_szKey, sizeof(m_szName) );
	LTStrCpy( m_szRelationSet, rhs.m_szRelationSet, sizeof(m_szRelationSet) );

	return *this;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CCollectiveRelationMgr::Save()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
int CCollectiveRelationMgr::Save(ILTMessage_Write *pMsg)
{
	m_pRelationUser->Save( pMsg );
	SAVE_CHARSTRING( m_szName );
	SAVE_CHARSTRING( m_szRelationSet );
	SAVE_TIME(m_flLastUpdate);
	return 0;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CCollectiveRelationMgr::Load()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
int CCollectiveRelationMgr::Load(ILTMessage_Read *pMsg)
{
	m_pRelationUser->Load( pMsg, this );
	LOAD_CHARSTRING( m_szName, sizeof(m_szName) );
	LOAD_CHARSTRING( m_szRelationSet, sizeof(m_szRelationSet) );
	LOAD_TIME(m_flLastUpdate);
	return 0;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CCollectiveRelationMgr::Init()
//
//	PURPOSE:	Initialize the CollectiveRelationMgr, reading it from the bute
//				file.  Then Initialize its aggregate RelationSet instance.
//
//----------------------------------------------------------------------------
void CCollectiveRelationMgr::Init(const char* const szName)
{
	AIASSERT( m_pRelationUser != NULL, NULL, "CCollectiveRelationMgr::Init m_pRelationUser is NULL." );
	AIASSERT( CRelationMgr::GetGlobalRelationMgr() != NULL, NULL, "CCollectiveRelationMgr::Init GetGlobalRelationMgr() is NULL." );
	AIASSERT( CRelationMgr::GetGlobalRelationMgr()->GetButeMgr() != NULL, NULL, "CCollectiveRelationMgr::Init CRelationMgr..GetButeMgr is NULL." );

	CRelationMgr::GetGlobalRelationMgr()->GetButeMgr()->CopyTemplate( szName, this );
	AIASSERT( strcmp(m_szRelationSet, "" ) != 0, NULL, "CCollectiveRelationMgr::Init Now RelationSet specified." );

	m_pRelationUser->ClearRelationUser();
	m_pRelationUser->InitRelations( m_szRelationSet );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CCollectiveRelationMgr::Sync()
//
//	PURPOSE:	For each relation in the collective, remove any momentos this
//				Instance contained with the matching description, then add
//				the collective version of the relation.
//
//	PURPOSE:	Do an update IF it is time to (as this function may be called
//				several times per frame since the members of the collective
//				call the collectives update when they sync with it.)
//
//----------------------------------------------------------------------------
void CCollectiveRelationMgr::Sync(const CObjectRelationMgr* pObjectRelationMgr)
{
	if ( m_flLastUpdate < g_pLTServer->GetTime() )
	{
		m_pRelationUser->Update( true );
		m_flLastUpdate = g_pLTServer->GetTime() + 0.1f;
	}

	m_pRelationUser->Sync(pObjectRelationMgr);
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CCollectiveRelationMgr::AddRelation()
//
//	PURPOSE:	Collectives add relationships to themselves only, allowing the
//				members to sync to the Collective (at which point we add the
//				relation directly to the object, skipping over the Personal
//				relation momentos.
//
//----------------------------------------------------------------------------
void CCollectiveRelationMgr::AddRelation(const RelationDescription& RD)
{
	// Remember this relation so that we can delete it when its time is up
	m_pRelationUser->AddRelation(RD);
}

/*virtual*/ void CCollectiveRelationMgr::AddRelationCallback(const RelationDescription& RD)
{
	AITRACE(AIShowRelations, ( (HOBJECT)NULL, "Collective adding: %s %s %s", CRelationTools::GetInstance()->ConvertAlignmentEnumToString(RD.eAlignment), CRelationTools::GetInstance()->ConvertTraitEnumToString(RD.eTrait), RD.szValue) );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CCollectiveRelationMgr::RemoveRelationCallback()
//
//	PURPOSE:	For each member, remove from their relation state the
//				described relation.  Note that this ignores any personal
//				state, and blindly erases the value.
//
//----------------------------------------------------------------------------
void CCollectiveRelationMgr::RemoveRelationCallback(const RelationDescription& RD)
{
	using std::for_each;
	using std::bind2nd;

	AITRACE(AIShowRelations, ( (HOBJECT)NULL, "Collective removing: %s %s %s", CRelationTools::GetInstance()->ConvertAlignmentEnumToString(RD.eAlignment), CRelationTools::GetInstance()->ConvertTraitEnumToString(RD.eTrait), RD.szValue) );

	// Copy the RelationDesciption, because we can't pass a reference into
	// the search
	RelationDescription InstRD = RD;
	for_each(m_listMembers.begin(),  m_listMembers.end(),  bind2nd( RemoveRelationFromObjectRelationMgr(), InstRD));

}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCollectiveRelationMgr::AddObjectRelationMgr()
//              
//	PURPOSE:	Adds a non NULL member which is not already in the member list
//				to the list
//              
//----------------------------------------------------------------------------
void CCollectiveRelationMgr::AddObjectRelationMgr(CObjectRelationMgr* pMember)
{
	using std::find;
	AIASSERT( pMember!=NULL, NULL, "Attempted to add NULL member to collective" );
	AIASSERT( find( m_listMembers.begin(), m_listMembers.end(), pMember ) == m_listMembers.end(), NULL, "only one membership allowed" );
	m_listMembers.push_back( pMember );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCollectiveRelationMgr::RemoveObjectRelationMgr()
//              
//	PURPOSE:	Removes the passed member from the list.  Assumes they are on
//				it, asserting if they are not or if the pointer is NULL
//              
//----------------------------------------------------------------------------
void CCollectiveRelationMgr::RemoveObjectRelationMgr(const CObjectRelationMgr* const pMember)
{
	using std::find;
	AIASSERT( pMember!=NULL, NULL, "Attempted to remove NULL member to collective" );
	AIASSERT( std::find( m_listMembers.begin(), m_listMembers.end(), pMember ) != m_listMembers.end(), NULL, "Attempted to remove non member" );
	m_listMembers.erase( find( m_listMembers.begin(), m_listMembers.end(), pMember ) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCollectiveRelationMgr::Clear()
//              
//	PURPOSE:	Clears out the information in the Collective
//              
//----------------------------------------------------------------------------
void CCollectiveRelationMgr::Clear()
{
	m_pRelationUser->ClearRelationUser();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCollectiveRelationMgr::ResetRelationTime()
//              
//	PURPOSE:	Resets the Relation currently passed into the collective
//              
//----------------------------------------------------------------------------
void CCollectiveRelationMgr::ResetRelationTime(CObjectRelationMgr* pORM)
{
	m_pRelationUser->ResetRelationTime(pORM);
}

