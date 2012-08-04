//----------------------------------------------------------------------------
//
//	MODULE:		RelationButeMgr.h
//
//	PURPOSE:	CRelationButeMgr declaration
//
//	CREATED:	28.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	Reads templates for relation system.  This allows systems
//				which are not AI to use this (previously was attached to
//				AIButeMgr)
//
//	Hacks:		1) The ObjectRelationMgr_Template is constructed from the same
//					data that the RelationSet is constructed from.  The
//					ObjectRelationMgr_Template should be constructed in its own
//
//
//
//----------------------------------------------------------------------------

#ifndef __RELATIONBUTEMGR_H__
#define __RELATIONBUTEMGR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#pragma warning( disable : 4786 )
#include <string>
#include <map>
#include <list>
#include <ostream>

#include "GameButeMgr.h"
#include "CharacterAlignment.h"
#include "AIUtils.h"
#include "RelationChangeObserver.h"

// Forward declarations
class CRelationButeMgr;
class CRelationMgr;
class CCollectiveRelationMgr;
class CRelationUser;
class CDataUser;
class RelationMomento;
class CObjectRelationMgr;

struct CRelationUser_Template;
struct CDataUser_Template;
struct CCollective_Template;

// Globals

// Statics

//
// MACROS:
//
#define RELATION_FILE "Attributes\\RelationData.txt"


struct RelationUser_Template
{
	RelationUser_Template()
	{
		m_nTemplateID = -1;
		m_szKey[0] = NULL;
	}

	int			m_nTemplateID;	// Template ID for lookups without a string compare.
	std::vector<RelationDescription>	m_vecRelationDescriptions;
	float		m_flTimeRelationsLast;			// How long will a new relation last?
	char		m_szKey[RELATION_VALUE_LENGTH]; // Name of the Template, used for lookups
};

struct DataUser_Template
{
	DataUser_Template()
	{
		m_nTemplateID = -1;
		m_szKey[0] = NULL;
	}

	int			m_nTemplateID;	// Template ID for lookups without a string compare.
	RelationData m_RelationData;				// Who am I anyway...
	char		m_szKey[RELATION_VALUE_LENGTH];	// Name of the Template, used for lookups
};

struct Collective_Template
{
	Collective_Template()
	{
		m_nTemplateID = -1;
		m_szKey[0] = NULL;
		m_szRelationSet[0] = NULL;
	}

	int		m_nTemplateID;	// Template ID for lookups without a string compare.
	char	m_szKey[RELATION_VALUE_LENGTH];			// Name of the Template, used for lookups

	char	m_szRelationSet[RELATION_VALUE_LENGTH]; // Name RelationUser, if any to default to
};

struct ObjectRelationMgr_Template
{
	ObjectRelationMgr_Template()
	{
		m_nTemplateID = -1;
		m_szKey[0] = NULL;
		m_szCollectiveName[0] = NULL;
		m_szRelationSet[0] = NULL;
		m_szDataSet[0] = NULL;
	}

	int		m_nTemplateID;	// Template ID for lookups without a string compare.

	char	m_szKey[RELATION_VALUE_LENGTH];				// Name of the Template, used for lookups

	char	m_szCollectiveName[RELATION_VALUE_LENGTH];	// Name of the Collective this ORM subscribes to
	char	m_szRelationSet[RELATION_VALUE_LENGTH]; // Name RelationUser, if any to default to
	char	m_szDataSet[RELATION_VALUE_LENGTH]; // Name RelationUser, if any to default to
};


//----------------------------------------------------------------------------
//
//	CLASS:		CRelationTools
//
//	PURPOSE:	Helper class to allow sharing of the Relation line parsing
//				functionality.  Example: butemgr and message passing both
//				need the ability to parse strings.
//
//----------------------------------------------------------------------------
class CRelationTools
{
public:
	static CRelationTools* GetInstance();

	void ParseRelation(const char* const pszLine, RelationDescription* pRD);
	void ParseData(const char* const pszLine, RelationTraits::eRelationTraits* pTr, char* pszValue, int nValueLen );

	CharacterAlignment ConvertAlignmentNameToEnum(const char* const szName) const;
	RelationTraits::eRelationTraits ConvertTraitNameToEnum(const char* const szName) const;
	const char* const ConvertAlignmentEnumToString(CharacterAlignment Alignment) const;
	const char* const ConvertTraitEnumToString(RelationTraits::eRelationTraits) const;

	CRelationTools(){}
	~CRelationTools(){}

protected:

private:
	typedef std::map<int, std::string> _mapTraitToName;
	static _mapTraitToName sm_mapRelationTraitToName;

	typedef std::map<int, std::string> _mapAlignmentToName;
	static _mapAlignmentToName sm_mapRelationAlignmentToName;

	static CRelationTools* sm_pInstance;
};

//----------------------------------------------------------------------------
//
//	CLASS:		CRelationButeMgr
//
//	PURPOSE:	ButeMgr to
//
//----------------------------------------------------------------------------
class CRelationButeMgr : public CGameButeMgr
{
public:
	// Public members

	// Ctors/Dtors/etc
	CRelationButeMgr() { m_bInitialized = false; }
	~CRelationButeMgr() {}

	LTBOOL	Init(const char* szAttributeFile = RELATION_FILE);
	void	Term();

	// Templates

	void	CopyTemplate(const char* const, CDataUser*) const;
	void	CopyTemplate(const char* const, CRelationUser*) const;
	void	CopyTemplate(const char* const, CCollectiveRelationMgr*) const;
	void	CopyTemplate(const char* const, CObjectRelationMgr*) const;

	typedef std::map<std::string, int> _mapNameToID;
	typedef std::vector<RelationUser_Template*> _listRelationUserTemplates;
	typedef std::vector<DataUser_Template*> _listRelationDataTemplates;
	typedef std::vector<Collective_Template*> _listCollectiveTemplates;
	typedef std::vector<ObjectRelationMgr_Template*> _listObjectRelationMgrTemplates;

	// Data Access

	bool			IsInitialized() const { return m_bInitialized; }
	_mapNameToID*	GetMapObjectRelationMgrNameToID() { return &m_mapObjectRelationMgrNameToID; }
	int				GetObjectRelationMgrTemplateIDByName( const char* const pszName ) const { return GetTemplateIDByName( pszName, (CObjectRelationMgr*)NULL ); }
	const char*		GetObjectRelationMgrTemplateName(int nTemplateID);

	// Get Relationset independent of a character object.
	void FillRelationSet(const char* const szName, RelationSet* pOut);

protected:
	// Protected members

private:
	// Private members
	// Copy Constructor and Asignment Operator private to prevent
	// automatic generation and inappropriate, unintentional use
	CRelationButeMgr(const CRelationButeMgr& rhs) {}
	CRelationButeMgr& operator=(const CRelationButeMgr& rhs ) {}

	int GetTemplateIDByName( const char* const, CDataUser* ) const;
	int GetTemplateIDByName( const char* const, CRelationUser* ) const;
	int GetTemplateIDByName( const char* const, CCollectiveRelationMgr* ) const;
	int GetTemplateIDByName( const char* const, CObjectRelationMgr* ) const;

	void ReadTemplate(const char* const, Collective_Template* );
	void ReadTemplate(const char* const, DataUser_Template* );
	void ReadTemplate(const char* const, RelationUser_Template* );
	void ReadTemplate(const char* const, ObjectRelationMgr_Template* );

	_mapNameToID				m_mapDataNameToID;
	_listRelationDataTemplates	m_listpRelationDataTemplates;

	_mapNameToID				m_mapRelationNameToID;
	_listRelationUserTemplates	m_listpRelationUserTemplates;

	_mapNameToID				m_mapCollectiveNameToID;
	_listCollectiveTemplates	m_listpCollectiveTemplates;

	_mapNameToID				m_mapObjectRelationMgrNameToID;
	_listObjectRelationMgrTemplates	m_listpObjectRelationMgrTemplates;

	bool						m_bInitialized;
};

namespace CommReqs
{
	enum Flags
	{
		kCommReqs_Friend		= 0x00000001,
		kCommReqs_SameRegion	= 0x00000002,
		kCommReqs_Player		= 0x00000004,
		kCommReqs_Collective	= 0x00000008,
	};
};

	typedef std::list< RelationMomento* > _listMomentos;

//-------------------------------------------------------------------------
//
//	CLASS:		IMomentoUser
//
//	PURPOSE:	Defines an interface for the momento to do the callbacks.
//
//-------------------------------------------------------------------------
class IMomentoUser
{
public:

	// Called when a relation is added.  The User is responsible for doing
	// the RelationDescription addition.
	virtual void RemoveRelationCallback(const RelationDescription& RD) = 0;

	// Called when a relation is removed.  The User is responsible for
	// doing the removal of the RelationDescription itself.
	virtual void AddRelationCallback(const RelationDescription& RD) = 0;

	virtual CRelationUser* GetRelationUser() = 0;
};


//-------------------------------------------------------------------------
//
//	CLASS:		RelationMomento
//
//	PURPOSE:	Helper class to manage the addition and removal of Relation
//				mementos which time out.
//
//-------------------------------------------------------------------------
struct DeleteMomentoWithCallback;
struct Momento_RelationData_Equality;
class RelationMomento
{
	friend DeleteMomentoWithCallback;
	friend Momento_RelationData_Equality;
	friend std::ostream& operator << (std::ostream &os, const RelationMomento& Momento);

public:
	// Ctors/Dtors/Save/Loadetc

	RelationMomento(IMomentoUser* pUser, float time, const RelationDescription&);
	RelationMomento(IMomentoUser* pUser);
	~RelationMomento() {}
	int Save(ILTMessage_Write *pMsg);
	int Load(ILTMessage_Read *pMsg);

	void AddMomento( _listMomentos& listAddTo );
	void ResetExpiration( float flTime ) { m_flExpiration = flTime; }
	const RelationDescription& GetDescription( void ) const	{ return m_RD; }
	IMomentoUser* GetOwner() { return m_pMomentoUser; }

	enum eRelationMomentoType { ePermanent, eTemporary };
	void SetTypePermanent() { m_eRelationMomentoType = ePermanent; }
	void SetTypeTemporary() { m_eRelationMomentoType = eTemporary; }

protected:
	// Modifying methods:

	void DoAddRelationCallback(void);
	void DoRemoveRelationCallback(void);
	bool IsExpired(float flCurrentTime) const;

private:
	// Copy Constructor and Asignment Operator private to prevent
	// automatic generation and inappropriate, unintentional use
	RelationMomento(const RelationMomento& rhs) {}
	RelationMomento& operator=(const RelationMomento& rhs ) {}

	// Accessor functions:

	// Data:
	eRelationMomentoType m_eRelationMomentoType;
	IMomentoUser*		m_pMomentoUser;	// Backpointer to the owner Maintainer
	float				m_flExpiration;	// Time the RelationDescription becomes becomes invalid
	RelationDescription	m_RD;			// Description of the actual Relation
};

std::ostream& operator << (std::ostream &os, const RelationMomento& Momento);

//----------------------------------------------------------------------------
//
//	CLASS:		CRelationUser
//
//	PURPOSE:	Per instance class to manage a characters Relation
//				information.
//
//----------------------------------------------------------------------------
class CRelationUser
{
public:
	friend std::ostream& operator << (std::ostream& os, const CRelationUser& User);

	friend struct SyncObjectRelationMgr;
	friend RelationMomento;

	// Ctors/Dtors/etc
	CRelationUser(IMomentoUser* pCallbackReceiver);
	~CRelationUser();
	CRelationUser& operator=(const RelationUser_Template& rhs );
	int Save(ILTMessage_Write *pMsg);
	int Load(ILTMessage_Read *pMsg,IMomentoUser* pUser);

	void ResetRelationTime(CObjectRelationMgr* pORM);

	// Initialization

	void InitRelations(const char* const szKey);

	// Updating

	void Update(bool bCanRemoveExpiredRelations);

	// Relation manipulation

	bool CanAddRelation(const RelationDescription& RD) const;
	void AddRelation(const RelationDescription& RD, bool bPermanent = false);
	bool HasMatchingRelationMomento(const RelationDescription&) const;

	void RemoveRelation(const RelationDescription&);
	bool CanRemoveRelation(const RelationDescription& RD) const;

	void ClearRelationUser(void);

	void Sync(const CObjectRelationMgr* pObjectRelationMgr);

	const RelationSet& GetRelations() { return m_RelationSet; }

protected:

	// Copy Constructor and Asignment Operator private to prevent
	// automatic generation and inappropriate, unintentional use
	CRelationUser(const CRelationUser& rhs) {}
	CRelationUser& operator=(const CRelationUser& rhs ) {}

	RelationSet& SetRelations() { return m_RelationSet; }

private:

	void RemoveAllRelationMomentos(void);

	// Data:

	//Save
	RelationSet			m_RelationSet;	// What do I think about others?
	_listMomentos		m_Momentos;		// List of Dynamic Relationships
	float				m_flTimeRelationsLast;

	// Don't save:
	IMomentoUser*		m_pMomentoCallbackReceiver;
};

std::ostream& operator << (std::ostream& os, const CRelationUser& User);


//----------------------------------------------------------------------------
//
//	CLASS:		CDataUser
//
//	PURPOSE:	Per instance class to manage a characters Relation
//				information.
//
//----------------------------------------------------------------------------
class CDataUser
{
public:
	// Ctors/Dtors/etc
	CDataUser(){}
	~CDataUser() {}
	CDataUser& operator=( const DataUser_Template& RU_Template );
	int Save(ILTMessage_Write *pMsg);
	int Load(ILTMessage_Read *pMsg);

	void InitData(const char* const szKey, HOBJECT hOwner);
	void ClearDataUser()		{ m_RelationData.Clear(); }

	const RelationData&	GetData() const { return m_RelationData; }
	RelationData& SetData()	{ return m_RelationData; }

protected:
	// Initialization

private:
	// Copy Constructor and Asignment Operator private to prevent
	// automatic generation and inappropriate, unintentional use
	CDataUser(const CDataUser& rhs) {}
	CDataUser& operator=(const CDataUser& rhs ) {}

	// Data:
	// Save:
	RelationData		m_RelationData;			// Who am I anyway...
};



#endif // __RELATIONBUTEMGR_H__
