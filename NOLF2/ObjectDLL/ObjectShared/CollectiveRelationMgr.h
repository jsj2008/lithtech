//----------------------------------------------------------------------------
//              
//	MODULE:		CollectiveRelationMgr.h
//              
//	PURPOSE:	CCollectiveRelationMgr declaration
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

#ifndef __COLLECTIVERELATIONMGR_H__
#define __COLLECTIVERELATIONMGR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "RelationButeMgr.h"

// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:		CCollectiveRelationMgr
//              
//	PURPOSE:	Group based collection of relationships which subscribers to a
//				collective all possess.  Acts as a syncing mechanism for
//				multiple ObjectRelationMgr instances.
//              
//----------------------------------------------------------------------------
class CCollectiveRelationMgr : public IMomentoUser
{
public:
	typedef std::list< RelationMomento* > _listMomentos;

	// Ctors/Dtors/etc

	CCollectiveRelationMgr();
	virtual ~CCollectiveRelationMgr();
	CCollectiveRelationMgr& operator=(const Collective_Template& rhs );
	int Save(ILTMessage_Write *pMsg);
	int Load(ILTMessage_Read *pMsg);

	// Initialization

	void Init( const char* const szName);

	// Modifying methods:
	
	void AddObjectRelationMgr( CObjectRelationMgr* pMember );
	void RemoveObjectRelationMgr( const CObjectRelationMgr* const pMember );

	void Sync(const CObjectRelationMgr* pObjectRelationMgr);
	void ResetRelationTime(CObjectRelationMgr* pORM);

	void Clear();
	void AddRelation(const RelationDescription& RD);

	// Non Modifying methods:
	
	const char* const GetKey() const { return m_szName;	}

protected:
	// IMomento Interface functions:
	friend RelationMomento;
	virtual void RemoveRelationCallback(const RelationDescription& RD);
	virtual void AddRelationCallback(const RelationDescription& RD);
	virtual CRelationUser*	GetRelationUser() { return m_pRelationUser; }

private:

	// Copy Constructor and Asignment Operator private to prevent 
	// automatic generation and inappropriate, unintentional use
	CCollectiveRelationMgr(const CCollectiveRelationMgr& rhs) {}
	CCollectiveRelationMgr& operator=(const CCollectiveRelationMgr& rhs ) {}

	// Save:
	char			m_szName[RELATION_VALUE_LENGTH];
	char			m_szRelationSet[RELATION_VALUE_LENGTH];
	LTFLOAT			m_flLastUpdate;
	CRelationUser*	m_pRelationUser;

	// Don't save:
	// list of ObjectRelationMgrs making up this collective
	typedef std::vector<CObjectRelationMgr*> _listMembers;
	_listMembers	m_listMembers;

};

#endif // __COLLECTIVERELATIONMGR_H__

