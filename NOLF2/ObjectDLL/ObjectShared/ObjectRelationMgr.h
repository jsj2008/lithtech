//----------------------------------------------------------------------------
//              
//	MODULE:		ObjectRelationMgr.h
//              
//	PURPOSE:	CObjectRelationMgr declaration
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

#ifndef __OBJECTRELATIONMGR_H__
#define __OBJECTRELATIONMGR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "RelationButeMgr.h"		

// Forward declarations
class CCollectiveRelationMgr;
class CRelationUser;
class CDataUser;
class CCharacter;
class IRelationChangeReceiver;

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:		CObjectRelationMgr
//              
//	PURPOSE:	Per instance class to manage a characters Relation
//				information.
//              
//----------------------------------------------------------------------------
class CObjectRelationMgr : public IMomentoUser, public ILTObjRefReceiver, public IRelationChangeSubject
{
public:
	// Ctors/Dtors/etc	
	CObjectRelationMgr();
	virtual ~CObjectRelationMgr();
	CObjectRelationMgr& operator=( const ObjectRelationMgr_Template& );
	virtual int Save(ILTMessage_Write* pMsg);
	virtual int Load(ILTMessage_Read* pMsg);

	// Initialization
	void Init( HOBJECT, const char* const );

	// Updating functions:

	void Update(bool bCanRemoveExpiredDurationRelations);
	void ClearRelationSystem(void);

	// Collective Management

	void SetCollectiveName(const char* const szCollective) { LTStrCpy( m_szCollectiveName, szCollective, sizeof(m_szCollectiveName) ); }
	void SetCollective(CCollectiveRelationMgr* pCollective);
	CCollectiveRelationMgr*	GetCollective()		{ return m_pCollective; }

	// Relation Handling:

	void CommunicateMessage(CCharacter* pSender, const RelationDescription&, uint32 dRestrictionFlags);
	virtual void ResetRelationTime(CObjectRelationMgr* pORM);
	void AddRelation(const RelationDescription& RD);
	
	// Mix of Aggregate accessors (poorly blobbed together -- ought to be used more cleanly)
	
	const RelationData& GetData() const { return m_pDataUser->GetData(); }
	RelationData& SetData() { return m_pDataUser->SetData(); }
	const int GetTemplateID() const { return m_nTemplateID; }

	// IMomento Interface functions:
	virtual CRelationUser* GetRelationUser() { return m_pRelationUser; }

	// ILTObjRefReceiver Interface functions:
	void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
	{
		m_hOwner = NULL;
	}

	// IRelationChangeReceiver Interface Functions
	virtual void UnregisterObserver(RelationChangeNotifier*);
	virtual void RegisterObserver(RelationChangeNotifier*);

	LTObjRefNotifier& GetOwningHObject(){ return m_hOwner; }

protected:
	// IMomento Interface functions:
	friend RelationMomento;
	virtual void RemoveRelationCallback(const RelationDescription& RD);
	virtual void AddRelationCallback(const RelationDescription& RD);

private:
	// Copy Constructor and Asignment Operator private to prevent 
	// automatic generation and inappropriate, unintentional use
	CObjectRelationMgr(const CObjectRelationMgr& rhs) {}
	CObjectRelationMgr& operator=(const CObjectRelationMgr& rhs ) {}

	// Non Modifying:
	const char* const GetCollectiveName() const			{ return m_szCollectiveName; }
	bool HasCollective() const							{ return m_pCollective != NULL; }
	bool HasCollectiveName() const						{ return ( strcmp( GetCollectiveName(), "" ) != 0 ); }

	// Modifying Methods:
	bool RecipientMeetsRestrictions(CCharacter* pSender, CCharacter* pReceiver, uint32 iRestrictionFlags);

	// Save:
	int				m_nTemplateID;
	char			m_szCollectiveName[RELATION_VALUE_LENGTH];
	char			m_szDataSet[RELATION_VALUE_LENGTH];
	char			m_szRelationSet[RELATION_VALUE_LENGTH];
	CRelationUser*	m_pRelationUser;
	CDataUser*		m_pDataUser;
	LTObjRefNotifier m_hOwner;


	// Don't Save:
	CCollectiveRelationMgr*		m_pCollective;
	std::list<RelationChangeNotifier*> listChangeReceivers;
};


#endif // __OBJECTRELATIONMGR_H__

