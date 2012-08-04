//----------------------------------------------------------------------------
//
//	MODULE:		ObjectRelationMgr.cpp
//
//	PURPOSE:	- implementation
//
//	CREATED:	21.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	TODO:	Convert the Relation Change Notification to pass the
//			RelationDescription.  That way we can do FULL checks to see if
//			we should change how we are acting.
//
//			Do this through a new 'MatchesDescription() function test.
//
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "ObjectRelationMgr.h"
#include "AIRegion.h"
#include "AIVolume.h"
#include "RelationMgr.h"
#include "CollectiveRelationMgr.h"
#include "AIAssert.h"
#include "Character.h"
#include "CharacterMgr.h"
#include "RelationChangeObserver.h"
#include <list>
#include <algorithm>

// Forward declarations

// Globals

// Statics

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::CObjectRelationMgr()
//
//	PURPOSE:	Constructor to handle any required initialization
//
//----------------------------------------------------------------------------
CObjectRelationMgr::CObjectRelationMgr()
{
	AIASSERT( CRelationMgr::GetGlobalRelationMgr(), NULL, "No GlobalRelationMgr" );

	m_nTemplateID = -1;

	m_pCollective = NULL;
	m_szCollectiveName[0] = '\0';

	m_szDataSet[0] = '\0';
	m_szRelationSet[0] = '\0';

	m_pRelationUser = debug_new1(CRelationUser, this);
	m_pDataUser = debug_new(CDataUser);

	m_hOwner = NULL;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::~CObjectRelationMgr()
//
//	PURPOSE:	Unsubstrive the object from the GlobalRelationMgr and the
//				CollectiveRelationMgr.  Destructor to clean up all allocated
//				memory.
//
//----------------------------------------------------------------------------
CObjectRelationMgr::~CObjectRelationMgr()
{
	AIASSERT( CRelationMgr::GetGlobalRelationMgr(), NULL, "No GlobalRelationMgr" );

	CRelationMgr::GetGlobalRelationMgr()->RemoveObjectRelationMgr(this);

	if ( HasCollective() )
	{
		GetCollective()->RemoveObjectRelationMgr( this );
	}
	if ( m_pRelationUser )
	{
		debug_delete( m_pRelationUser );
		m_pRelationUser = NULL;
	}
	if ( m_pDataUser )
	{
		debug_delete( m_pDataUser );
		m_pDataUser = NULL;
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::operator=()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
CObjectRelationMgr& CObjectRelationMgr::operator=(const ObjectRelationMgr_Template& rhs)
{
	LTStrCpy(m_szCollectiveName,	rhs.m_szCollectiveName, sizeof(m_szCollectiveName) );
	LTStrCpy(m_szDataSet,			rhs.m_szDataSet,		sizeof(m_szDataSet) );
	LTStrCpy(m_szRelationSet,		rhs.m_szRelationSet,	sizeof(m_szRelationSet) );

	m_nTemplateID = rhs.m_nTemplateID;

	return *this;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::Save()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
int CObjectRelationMgr::Save(ILTMessage_Write *pMsg)
{
	AIASSERT(m_pRelationUser, NULL, "CObjectRelationMgr::Save: No RelationUser");
	AIASSERT(m_pDataUser, NULL, "CObjectRelationMgr::Save: No DataUser");

	m_pRelationUser->Save(pMsg);
	m_pDataUser->Save(pMsg);
	SAVE_DWORD( m_nTemplateID );
	SAVE_CHARSTRING( m_szCollectiveName );
	SAVE_CHARSTRING( m_szDataSet );
	SAVE_CHARSTRING( m_szRelationSet );
	return 0;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::Load()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
int CObjectRelationMgr::Load(ILTMessage_Read *pMsg)
{
	AIASSERT(m_pRelationUser, NULL, "CObjectRelationMgr::Load: No RelationUser");
	AIASSERT(m_pDataUser, NULL, "CObjectRelationMgr::Load: No DataUser");

	m_pRelationUser->Load(pMsg, this);
	m_pDataUser->Load(pMsg);

	LOAD_DWORD( m_nTemplateID );
	LOAD_CHARSTRING( m_szCollectiveName, sizeof(m_szCollectiveName)  );
	LOAD_CHARSTRING( m_szDataSet, sizeof(m_szDataSet) );
	LOAD_CHARSTRING( m_szRelationSet, sizeof(m_szRelationSet) );

	if ( HasCollectiveName() )
	{
		// If we have a collective string, then set the pointer to the
		// collective by setting the pointer to the looked up name
		SetCollective( CRelationMgr::GetGlobalRelationMgr()->FindCollective( GetCollectiveName() ) );
	}

	return 0;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::Init()
//
//	PURPOSE:	Sets up this instance of the ObjectRelationMgr.  First unlinks
//				the ObjectRelationMgr, then updates the information and
//				relinks it.  In most cases, the unlinking is not required
//				as Init should really be called only once for a character
//				except in exceptional cases (like the player).
//
//----------------------------------------------------------------------------
void CObjectRelationMgr::Init(HOBJECT hObject, const char* const szKey)
{
	AIASSERT(hObject != NULL, NULL, "CDataUser::InitData: NULL HOBJECT" );
	AIASSERT(CRelationMgr::GetGlobalRelationMgr(), NULL, "CObjectRelationMgr::Init: No GlobalRelationMgr" );
	AIASSERT(m_pRelationUser, NULL, "CObjectRelationMgr::Init: No RelationUser");
	AIASSERT(m_pDataUser, NULL, "CObjectRelationMgr::Init: No DataUser");

	CRelationMgr::GetGlobalRelationMgr()->RemoveObjectRelationMgr(this);
	CRelationMgr::GetGlobalRelationMgr()->AddObjectRelationMgr(this, hObject);

	CRelationMgr::GetGlobalRelationMgr()->GetButeMgr()->CopyTemplate(szKey, this);
	m_pRelationUser->InitRelations( m_szRelationSet );
	m_pDataUser->InitData( m_szDataSet, hObject );

	m_hOwner.SetReceiver( *this );
	m_hOwner = hObject;

	if ( HasCollectiveName() )
	{
		// If we have a collective string, then set the pointer to the
		// collective by setting the pointer to the looked up name
		SetCollective( CRelationMgr::GetGlobalRelationMgr()->FindCollective( GetCollectiveName() ) );
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::Update()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
void CObjectRelationMgr::Update(bool bCanRemoveExpiredRelations /*=false*/)
{
	AIASSERT(m_pRelationUser, NULL, "CObjectRelationMgr::Update: No RelationUser");

	m_pRelationUser->Update(bCanRemoveExpiredRelations);
	if ( HasCollective() )
	{
		GetCollective()->Sync( this );
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::ClearRelationSystem()
//
//	PURPOSE:	Clears the Relations and the RelationData for this instance.
//
//----------------------------------------------------------------------------
void CObjectRelationMgr::ClearRelationSystem(void)
{
	AIASSERT(m_pRelationUser, NULL, "CObjectRelationMgr::Clear: No RelationUser");
	AIASSERT(m_pDataUser, NULL, "CObjectRelationMgr::Clear: No DataUser");

	m_pRelationUser->ClearRelationUser();
	m_pDataUser->ClearDataUser();
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::SetCollective()
//
//	PURPOSE:	Sets RelationUsers pointer to a collective, and then informs
//				the Collective that it is joining.
//
//----------------------------------------------------------------------------
void CObjectRelationMgr::SetCollective(CCollectiveRelationMgr* pCollective)
{
	AIASSERT( m_pCollective == NULL, NULL, "No support for leaving collective" );
	m_pCollective = pCollective;
	m_pCollective->AddObjectRelationMgr( this );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::CommunicateMessage()
//
//	PURPOSE:	Added a Message to the ObjectRelationMgr, and any collective
//				it subscribes to.
//
//	NOTE:		If in string form, first convert to RelationDescription
//				using CRelationTools
//
//----------------------------------------------------------------------------
void CObjectRelationMgr::CommunicateMessage(CCharacter* pSender,
											const RelationDescription& RD,
											uint32 dRestrictionFlags)
{
	// Send the message to the collective (if you have one)
	if ( dRestrictionFlags & CommReqs::kCommReqs_Collective )
	{
		if (HasCollective())
		{
			GetCollective()->AddRelation(RD);
		}
		else
		{
			Warn( "AI: ORM attempted to add a relation when none existed." );
		}
	}

	CTList<CCharacter*>* lstChars	= LTNULL;
    CCharacter** pCur				= LTNULL;
    CCharacter** pSelected			= LTNULL;

	// For each character, see if we like them.

	// Loop thru character lists.

	int cCharLists = g_pCharacterMgr->GetNumCharacterLists();
	for ( int iList = 0 ; iList < cCharLists ; ++iList )
	{
		// Loop thru characters.
		lstChars = g_pCharacterMgr->GetCharacterList(iList);
		pCur = lstChars->GetItem(TLIT_FIRST);
		while (pCur)
		{
			if ( RecipientMeetsRestrictions( pSender, *pCur, dRestrictionFlags ) )
			{
				(*pCur)->GetRelationMgr()->AddRelation(RD);
			}

			pCur = lstChars->GetItem(TLIT_NEXT);
		}
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::RecipientMeetsRestrictions()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
bool CObjectRelationMgr::RecipientMeetsRestrictions(CCharacter* pSender,
													CCharacter* pReceiver,
													uint32 iRestrictionFlags)
{
	// Just plain skip sending a message to anyone who is dead
	if( pReceiver->IsDead() )
	{
		return false;
	}

	// This transmition requires that the the sender considers the reciever
	// to be a friend.
	if ( iRestrictionFlags & CommReqs::kCommReqs_Friend )
	{
		if ( LIKE != GetAlignment( pSender->GetRelationSet(),
			pReceiver->GetRelationData() ))
		{
			return false;
		}
	}

	// If the sender and reciever have to be in the same region, then abort
	// if they are not
	if ( iRestrictionFlags & CommReqs::kCommReqs_SameRegion )
	{
		AIVolume* pSenderVolume = pSender->GetLastVolume();
		AIVolume* pRecipientVolume = pReceiver->GetLastVolume();
		if ( pSenderVolume == NULL || pRecipientVolume == NULL )
		{
			return false;
		}

		AIRegion* pSenderRegion = pSenderVolume->GetRegion();
		AIRegion* pRecipientRegion = pRecipientVolume->GetRegion();
		if ( pSenderRegion == NULL || pRecipientRegion == NULL )
		{
			return false;
		}

		if (pSenderVolume->GetRegion()->GetHOBJECT() !=
			pRecipientVolume->GetRegion()->GetHOBJECT())
		{
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::AddRelation()
//
//	PURPOSE:	Adds a new relation to the AI if the AI doesn't yet have this
//				exact relation, or if the AI already has a more extreme
//				relation (HATE has priority over everything else)
//
//	NOTES:		First construct a RelationDescription, parsing from a string
//				with CRelationTools if needed
//
//----------------------------------------------------------------------------
void CObjectRelationMgr::AddRelation(const RelationDescription& RD)
{
	if ( m_pRelationUser->CanAddRelation(RD) )
	{
		m_pRelationUser->AddRelation(RD);
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::AddRelationCallback()
//
//	PURPOSE:	Called when a momento attempts to add a relation.  Called
//				regardless of the creator -- could be local or could be the
//				creative
//
//----------------------------------------------------------------------------
void CObjectRelationMgr::AddRelationCallback(const RelationDescription& RD)
{
	using std::for_each;
	using std::bind2nd;
	using std::bind1st;
	using std::mem_fun;

	AITRACE(AIShowRelations, ( m_hOwner, " adding relation: %s %s %s", CRelationTools::GetInstance()->ConvertTraitEnumToString(RD.eTrait), CRelationTools::GetInstance()->ConvertAlignmentEnumToString(RD.eAlignment), RD.szValue ) );

	// TODO!!!!!
	// Allow lookups here based on things besides name.  This extremely incomplete!
	if ( RD.eTrait == RelationTraits::kTrait_Name)
	{
		CObjectRelationMgr* pORM = CRelationMgr::GetGlobalRelationMgr()->GetObjectRelationMgr( RD.szValue );
		if ( pORM )
		{
			HOBJECT hChanged = pORM->GetOwningHObject();
			if ( hChanged )
			{
				std::list<RelationChangeNotifier*>::iterator it;
				for ( it = listChangeReceivers.begin(); it != listChangeReceivers.end(); it++ )
				{
					(*it)->DoNotification( hChanged );
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::RemoveRelationCallback()
//
//	PURPOSE:	Clears out the Relation.  Called when a Memento is destructed.
//
//----------------------------------------------------------------------------
void CObjectRelationMgr::RemoveRelationCallback(const RelationDescription& RD)
{
	using std::for_each;
	using std::bind2nd;
	using std::mem_fun;

	AITRACE(AIShowRelations, ( m_hOwner, " removing relation: %s %s %s", CRelationTools::GetInstance()->ConvertTraitEnumToString(RD.eTrait), CRelationTools::GetInstance()->ConvertAlignmentEnumToString(RD.eAlignment), RD.szValue ) );

	// TODO!!!!!
	// Allow lookups here based on things besides name.  This extremely incomplete!
	if ( RD.eTrait == RelationTraits::kTrait_Name)
	{
		CObjectRelationMgr* pORM = CRelationMgr::GetGlobalRelationMgr()->GetObjectRelationMgr( RD.szValue );
		if ( pORM )
		{
			HOBJECT hChanged = pORM->GetOwningHObject();
			if ( hChanged )
			{
				// Set up the listener.
				std::list<RelationChangeNotifier*>::iterator it;
				for ( it = listChangeReceivers.begin(); it != listChangeReceivers.end(); it++ )
				{
					(*it)->DoNotification( hChanged );
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::ResetRelationTime()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
void CObjectRelationMgr::ResetRelationTime(CObjectRelationMgr* pORM)
{
	AIASSERT( pORM, m_hOwner, "NULL ObjectRelationMgr passed in" );
	if ( pORM == NULL )
	{
		return;
	}

	if ( HasCollective() )
	{
		GetCollective()->ResetRelationTime(pORM);
	}

	m_pRelationUser->ResetRelationTime(pORM);
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::RegisterObserver()
//
//	PURPOSE:	Adds a new callback
//
//----------------------------------------------------------------------------
/*virtual*/ void CObjectRelationMgr::RegisterObserver(RelationChangeNotifier* pRCN )
{
	AIASSERT( pRCN, GetOwningHObject(), "NULL RelationChangeNotifer passed in to Register" );
	AIASSERT( std::find(listChangeReceivers.begin(), listChangeReceivers.end(), pRCN) == listChangeReceivers.end(), GetOwningHObject(), "Attempted second insertation of RelationCallbackNotifier" );
	if (std::find(listChangeReceivers.begin(), listChangeReceivers.end(), pRCN) == listChangeReceivers.end())
	{
		listChangeReceivers.push_back( pRCN );
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CObjectRelationMgr::UnregisterObserver()
//
//	PURPOSE:	Removes the callback from the list.
//
//----------------------------------------------------------------------------
/*virtual*/ void CObjectRelationMgr::UnregisterObserver(RelationChangeNotifier* pRCN)
{
	using std::find;
	AIASSERT( pRCN, GetOwningHObject(), "NULL RelationChangeNotifer passed in to Unregister" );
	AIASSERT( find(listChangeReceivers.begin(),listChangeReceivers.end(), pRCN) != listChangeReceivers.end(), GetOwningHObject(), "Attempted second insertation of RelationCallbackNotifier" );
	listChangeReceivers.remove(pRCN);
}


