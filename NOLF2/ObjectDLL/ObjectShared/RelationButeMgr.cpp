
//----------------------------------------------------------------------------
//
//	MODULE:		RelationButeMgr.cpp
//
//	PURPOSE:	- implementation
//
//	CREATED:	29.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//
//
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#include "RelationButeMgr.h"
#include "ButeTools.h"
#include "AIUtils.h"
#include "RelationMgr.h"
#include "AI.h"
#include "CharacterAlignment.h"
#include "ObjectRelationMgr.h"
#include "CollectiveRelationMgr.h"
#include "AIAssert.h"

#include <algorithm>

// Forward declarations

// Globals

// Statics

struct MomentoIsNull
{
	bool operator()( RelationMomento* pMomento )
	{
		return ( pMomento == NULL );
	}
};

struct DeleteMomentoWithCallback
{
	RelationMomento* operator()( RelationMomento* pMomento )
	{
		AIASSERT( pMomento != NULL, NULL, "DeleteMomentoWithCallback without valid momento" );
		if ( pMomento->IsExpired( g_pLTServer->GetTime() ) )
		{
			pMomento->DoRemoveRelationCallback();
			debug_delete( pMomento );
			pMomento = NULL;
		}

		return pMomento;
	}
};

struct DeleteMomentoWithoutCallback
{
	RelationMomento* operator()( RelationMomento* pMomento )
	{
		AIASSERT( pMomento != NULL, NULL, "Attempted to delete a NULL momento" );
		debug_delete( pMomento );
		pMomento = NULL;

		return pMomento;
	}
};

struct Momento_RelationData_Equality :
std::binary_function<RelationMomento*, const RelationDescription* const, bool>
{
	bool operator() (RelationMomento* pMomento,
		const RelationDescription* const pRD) const
	{
		AIASSERT( pMomento!=NULL, NULL, "Attempted description match with NULL Momento" );
		AIASSERT( pRD!=NULL, NULL, "Attempted description match on NULL ORM" );
		return ( pMomento->GetDescription() == *pRD );
	}
};

struct ORMMatchesDescription :
public std::binary_function< RelationMomento*, CObjectRelationMgr*, bool>
{
	bool operator()( RelationMomento* pMomento, const CObjectRelationMgr* const pORM ) const
	{
		AIASSERT( pMomento!=NULL, NULL, "Attempted description match with NULL Momento" );
		AIASSERT( pORM!=NULL, NULL, "Attempted description match on NULL ORM" );
		char szValue[RELATION_VALUE_LENGTH];
		pORM->GetData().GetTraitValue(pMomento->GetDescription().eTrait, szValue, sizeof(szValue));
		return ( strcmp( pMomento->GetDescription().szValue, szValue ) == 0);
	}
};


//----------------------------------------------------------------------------
//
//	CRelationTools

// Static Members:
CRelationTools::_mapTraitToName CRelationTools::sm_mapRelationTraitToName;
CRelationTools::_mapAlignmentToName CRelationTools::sm_mapRelationAlignmentToName;
CRelationTools* CRelationTools::sm_pInstance;

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationTools::GetInstance()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
CRelationTools* CRelationTools::GetInstance()
{
	using std::make_pair;

	if ( sm_pInstance == NULL )
	{
		static CRelationTools Inst;
		sm_pInstance = &Inst;

		for ( int x = 0; x < RelationTraits::kTrait_Count; x++ )
		{
			sm_mapRelationTraitToName.insert( make_pair( x, std::string(RelationTraits::s_aszAITraitTypes[x]) ) );
		}

		sm_mapRelationAlignmentToName.insert( make_pair( LIKE, std::string("LIKE") ) );
		sm_mapRelationAlignmentToName.insert( make_pair( TOLERATE, std::string("TOLERATE") ) );
		sm_mapRelationAlignmentToName.insert( make_pair( HATE, std::string("HATE") ) );
		sm_mapRelationAlignmentToName.insert( make_pair( UNDETERMINED, std::string("UNDETERMINED") ) );
		sm_mapRelationAlignmentToName.insert( make_pair( INVALID, std::string("INVALID") ) );
	}

	return sm_pInstance;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationTools::ParseRelation()
//
//	PURPOSE:	Breaks a line down into the Enum Trait, the Enum Alignment,
//				and the Value.
//
//				Format:		TraitName Alignment ObjectID
//
//----------------------------------------------------------------------------
void CRelationTools::ParseRelation(const char* const pszLine, RelationDescription* pRD )
{
	// There is no server if Dedit is trying to read the RelationBute file.
	if( !g_pLTServer )
	{
		return;
	}

	AIASSERT( pszLine != NULL, NULL, "Attempted to parse NULL Relation" );

	char tokenSpace[512];
	char* pTokens[3];
	const char *pCommandPos;

	int nArgs;
	g_pLTServer->Parse(pszLine, &pCommandPos, tokenSpace, pTokens, &nArgs);
	if ( nArgs != 3 )
	{
		char szError[1024];
		sprintf( szError, "ParseRelation: Invalid Relation: %s", pszLine );
		AIASSERT( 0, NULL, szError );
	}

	const char* const szTrait		=	pTokens[0];
	const char* const szAlignment	=	pTokens[1];
	const char* const szObjectID	=	pTokens[2];

	// Convert the Trait Name to an enumeration.
	pRD->eTrait = ConvertTraitNameToEnum(szTrait);
	if ( pRD->eTrait == RelationTraits::kTrait_Invalid )
	{
		char szError[1024];
		sprintf( szError, "Invalid Trait Name: %s", szTrait );
		AIASSERT( 0, NULL, szError );
	}

	// Convert the Alignment Name to an enumeration.
	pRD->eAlignment = ConvertAlignmentNameToEnum(szAlignment);
	if ( pRD->eAlignment == INVALID )
	{
		char szError[1024];
		sprintf( szError, "Invalid Alignment Name: %s", szAlignment );
		AIASSERT( 0, NULL, szError );
	}

	SAFE_STRCPY(pRD->szValue, szObjectID );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationTools::ParseData()
//
//	PURPOSE:	Breaks a line down into the Enum Trait and Value.
//
//				Format:		TraitName ObjectID
//
//----------------------------------------------------------------------------
void CRelationTools::ParseData(const char* const pszLine, RelationTraits::eRelationTraits* pTr, char* pszValue, int nValueLen)
{
	// There is no server if Dedit is trying to read the RelationBute file.
	if( !g_pLTServer )
	{
		return;
	}

	char tokenSpace[512];
	char* pTokens[2];
	const char *pCommandPos;

	int nArgs;
	g_pLTServer->Parse(pszLine, &pCommandPos, tokenSpace, pTokens, &nArgs);
	if ( nArgs != 2 )
	{
		char szError[1024];
		sprintf( szError, "ParseData: Invalid Data: %s", pszLine );
		AIASSERT( 0, NULL, szError );
	}

	const char* const szTrait		=	pTokens[0];
	const char* const szObjectID	=	pTokens[1];

	// Convert the Trait Name to an enumeration.
	*pTr = ConvertTraitNameToEnum( szTrait );
	if ( *pTr == RelationTraits::kTrait_Invalid )
	{
		char szError[1024];
		sprintf( szError, "Invalid Trait Name: %s", szTrait );
		AIASSERT( 0, NULL, szError );
	}

	SAFE_STRCPY(pszValue, szObjectID );
}


//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationTools::ConvertAlignmentNameToEnum()
//
//	PURPOSE:	Convert a string into a CharacterAlignment.
//
//				TODO: May want to convert CharacterAlignment Iteration to
//				Jeffs macro expansion?
//
//----------------------------------------------------------------------------
CharacterAlignment CRelationTools::ConvertAlignmentNameToEnum(const char* const szName) const
{
	return ( ::ConvertAlignmentNameToEnum(szName) );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationTools::ConvertTraitNameToEnum()
//
//	PURPOSE:	Converts a string into an Trait
//
//----------------------------------------------------------------------------
RelationTraits::eRelationTraits CRelationTools::ConvertTraitNameToEnum(const char* const szName) const
{
	if ( szName != NULL )
	{
		for(uint32 nIterator=0; nIterator < RelationTraits::kTrait_Count; ++nIterator)
		{
			if (0 == stricmp(szName, RelationTraits::s_aszAITraitTypes[nIterator]) )
			{
				return (RelationTraits::eRelationTraits)nIterator;
			}
		}
	}

	return RelationTraits::kTrait_Invalid;
}

const char* const CRelationTools::ConvertAlignmentEnumToString(CharacterAlignment Alignment) const
{
	using std::find;
	_mapAlignmentToName::const_iterator it = sm_mapRelationAlignmentToName.find( static_cast<int>(Alignment) );
	UBER_ASSERT(it != sm_mapRelationAlignmentToName.end(), "Unable to find CharacterAlignment name" );
	return (*it).second.c_str();
}

const char* const CRelationTools::ConvertTraitEnumToString(RelationTraits::eRelationTraits eTrait) const
{
	using std::find;
	_mapTraitToName::const_iterator it = sm_mapRelationTraitToName.find(static_cast<int>(eTrait));
	UBER_ASSERT(it != sm_mapRelationTraitToName.end(), "Unable to find CharacterTrait name" );
	return (*it).second.c_str();
}


//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::Init()
//
//	PURPOSE:	Initializes the Bute Mgr.
//
//----------------------------------------------------------------------------
LTBOOL CRelationButeMgr::Init(const char* const szAttributeFile)
{
	AIASSERT( szAttributeFile != NULL, NULL, "CRelationButeMgr::Init called with NULL szAttributeFile." );

    if ( !Parse(szAttributeFile) )
	{
		char szError[1024];
		sprintf( szError, "Failed to parse %s", szAttributeFile );
		AIASSERT( 0, NULL, szError );
		return LTFALSE;
	}

	char szTagName[RELATION_VALUE_LENGTH];

	// Read the Collectives.
	sprintf(szTagName, "%s%d", "Collective", m_listpCollectiveTemplates.size());
	while (m_buteMgr.Exist(szTagName))
	{
		Collective_Template* pCC_Template = debug_new( Collective_Template );
		ReadTemplate(szTagName, pCC_Template);
		// Add the CRelationUser to our map so we can find it later.
		m_mapCollectiveNameToID.insert( _mapNameToID::value_type(std::string(pCC_Template->m_szKey), m_listpCollectiveTemplates.size()) );
		m_listpCollectiveTemplates.push_back( pCC_Template );

		sprintf(szTagName, "%s%d", "Collective", m_listpCollectiveTemplates.size());
	}

	// Read the ObjectRelationMgrs.
	sprintf(szTagName, "%s%d", "RelationalObject", m_listpObjectRelationMgrTemplates.size());
	while (m_buteMgr.Exist(szTagName))
	{
		ObjectRelationMgr_Template* pORM_Template = debug_new( ObjectRelationMgr_Template );
		ReadTemplate(szTagName, pORM_Template);
		// Add the CRelationUser to our map so we can find it later.
		m_mapObjectRelationMgrNameToID.insert( _mapNameToID::value_type(std::string(pORM_Template->m_szKey), m_listpObjectRelationMgrTemplates.size()) );
		m_listpObjectRelationMgrTemplates.push_back( pORM_Template );

		sprintf(szTagName, "%s%d", "RelationalObject", m_listpObjectRelationMgrTemplates.size());
	}

	// Read Relation Maintainers.
	sprintf(szTagName, "%s%d", "RelationSet", m_listpRelationUserTemplates.size());
	while (m_buteMgr.Exist(szTagName))
	{
		RelationUser_Template* pRU_Template = debug_new(RelationUser_Template);
		ReadTemplate(szTagName, pRU_Template);
		// Add the CRelationUser to our map so we can find it later.
		m_mapRelationNameToID.insert( _mapNameToID::value_type(std::string(pRU_Template->m_szKey), m_listpRelationUserTemplates.size()) );
		m_listpRelationUserTemplates.push_back( pRU_Template );

		sprintf(szTagName, "%s%d", "RelationSet", m_listpRelationUserTemplates.size());
	}

	// Read the Relational Data Sets.
	sprintf(szTagName, "%s%d", "RelationData", m_listpRelationDataTemplates.size());
	while (m_buteMgr.Exist(szTagName))
	{
		DataUser_Template* pDU_Template = debug_new(DataUser_Template);
		ReadTemplate(szTagName, pDU_Template);
		// Add the CRelationUser to our map so we can find it later.
		m_mapDataNameToID.insert( _mapNameToID::value_type(std::string(pDU_Template->m_szKey), m_listpRelationDataTemplates.size()) );
		m_listpRelationDataTemplates.push_back(pDU_Template);

		sprintf(szTagName, "%s%d", "RelationData", m_listpRelationDataTemplates.size());
	}

	m_buteMgr.Term();

	m_bInitialized = true;

    return LTTRUE;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::GetObjectRelationMgrTemplateName()
//
//	PURPOSE:	Returns the name of the template, or "" if none exists.
//
//----------------------------------------------------------------------------

const char*	CRelationButeMgr::GetObjectRelationMgrTemplateName(int nTemplateID)
{
	ObjectRelationMgr_Template* pTemplate;
	_listObjectRelationMgrTemplates::iterator it;
	for( it = m_listpObjectRelationMgrTemplates.begin(); it != m_listpObjectRelationMgrTemplates.end(); ++it )
	{
		pTemplate = *it;
		if( pTemplate->m_nTemplateID == nTemplateID )
		{
			return pTemplate->m_szKey;
		}
	}

	return "";
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::GetTemplateIDByName()
//
//	PURPOSE:	Returns the Data template ID, -1 if none exists
//
//----------------------------------------------------------------------------
int CRelationButeMgr::GetTemplateIDByName( const char* const pszName, CDataUser* ) const
{
	_mapNameToID::const_iterator it = m_mapDataNameToID.find( std::string(pszName) );
	if ( it != m_mapDataNameToID.end() )
	{
		return (*it).second;
	}

	AIASSERT1( 0, NULL, "Unable to find Relation data with name: %s in RelationData.txt", pszName );

	return -1;
}


//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::GetTemplateIDByName()
//
//	PURPOSE:	Returns the Relation template ID, -1 if none exists
//
//----------------------------------------------------------------------------
int CRelationButeMgr::GetTemplateIDByName(const char* const pszName, CRelationUser*) const
{
	_mapNameToID::const_iterator it = m_mapRelationNameToID.find( std::string(pszName) );
	if ( it != m_mapRelationNameToID.end() )
	{
		return (*it).second;
	}

	AIASSERT1( 0, NULL, "Unable to find Relation with name: %s in RelationData.txt", pszName );

	return -1;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::GetCollectiveTemplateIDByName()
//
//	PURPOSE:	Returns the Data template ID, -1 if none exists
//
//----------------------------------------------------------------------------
int CRelationButeMgr::GetTemplateIDByName( const char* const pszName, CCollectiveRelationMgr* ) const
{
	_mapNameToID::const_iterator it = m_mapCollectiveNameToID.find( std::string(pszName) );
	if ( it != m_mapCollectiveNameToID.end() )
	{
		return (*it).second;
	}

	AIASSERT1( 0, NULL, "Unable to find Collective with name: %s in RelationData.txt", pszName );

	return -1;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::GetTemplateIDByName()
//
//	PURPOSE:	Returns the Data template ID, -1 if none exists
//
//----------------------------------------------------------------------------
int CRelationButeMgr::GetTemplateIDByName( const char* const pszName, CObjectRelationMgr* ) const
{
	_mapNameToID::const_iterator it = m_mapObjectRelationMgrNameToID.find( std::string(pszName) );
	if ( it != m_mapObjectRelationMgrNameToID.end() )
	{
		return (*it).second;
	}

	AIASSERT1( 0, NULL, "Unable to find ObjectRelationMgr with name: %s in RelationData.txt", pszName );

	return -1;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::CopyTemplate()
//
//	PURPOSE:	Overload to set only the RelationData
//
//----------------------------------------------------------------------------
void CRelationButeMgr::CopyTemplate(const char* const szName,
									CObjectRelationMgr* pORM ) const
{
	uint32 nTemplateID = GetTemplateIDByName( szName, pORM );
	if ( nTemplateID < 0 || nTemplateID > m_listpObjectRelationMgrTemplates.size() )
	{
		AIASSERT1( 0, NULL, "Out of range RelationDataID: %d", nTemplateID );
		return;
	}

	// Copy the Class out to the out-pointer
	*pORM = *m_listpObjectRelationMgrTemplates[nTemplateID];
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::CopyTemplate()
//
//	PURPOSE:	Overload to set only the RelationData
//
//----------------------------------------------------------------------------
void CRelationButeMgr::CopyTemplate(const char* const szName,
									CDataUser* pDataUser ) const
{
	uint32 nTemplateID = GetTemplateIDByName( szName, pDataUser );
	if ( nTemplateID < 0 || nTemplateID > m_listpRelationDataTemplates.size() )
	{
		AIASSERT1( 0, NULL, "Out of range RelationDataID: %d", nTemplateID );
		return;
	}

	// Copy the Class out to the out-pointer
	*pDataUser = *m_listpRelationDataTemplates[nTemplateID];
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::CopyTemplate()
//
//	PURPOSE:	Overload to set only the CCollectiveRelationMgr
//
//----------------------------------------------------------------------------
void CRelationButeMgr::CopyTemplate(const char* const szName,
									CCollectiveRelationMgr* pCollective ) const
{
	uint32 nTemplateID = GetTemplateIDByName( szName, pCollective );
	if ( nTemplateID < 0 || nTemplateID > m_listpCollectiveTemplates.size() )
	{
		AIASSERT1( 0, NULL, "Out of range Collective: %d", nTemplateID );
		return;
	}

	// Copy the Class out to the out-pointer
	*pCollective = *m_listpCollectiveTemplates[nTemplateID];
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::CopyTemplate()
//
//	PURPOSE:	Overload to set only the RelationUser
//
//----------------------------------------------------------------------------
void CRelationButeMgr::CopyTemplate(const char* const szName,
									CRelationUser* pRelationUser) const
{
	uint32 nTemplateID = GetTemplateIDByName( szName, pRelationUser );
	if ( nTemplateID < 0 || nTemplateID > m_listpRelationUserTemplates.size() )
	{
		AIASSERT1( 0, NULL, "Out of range RelationSetID: %d", nTemplateID );
		return;
	}

	// Copy the Relation out to the out-pointer
	*pRelationUser = *m_listpRelationUserTemplates[nTemplateID];
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::ReadTemplate()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
void CRelationButeMgr::ReadTemplate(const char* const szTagName,
									Collective_Template* pCC_Template )
{
	// Get and set the name
	CButeTools::GetValidatedString(m_buteMgr, szTagName, "Name", pCC_Template->m_szKey, sizeof(pCC_Template->m_szKey) );
	CButeTools::GetValidatedString(m_buteMgr, szTagName, "RelationSet", pCC_Template->m_szRelationSet, sizeof(pCC_Template->m_szRelationSet), "" );

	// Keep track of the template ID, for lookups later without a string compare.
	pCC_Template->m_nTemplateID = m_listpCollectiveTemplates.size();
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::ReadTemplate()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
void CRelationButeMgr::ReadTemplate(const char* const szTagName,
									ObjectRelationMgr_Template* pORM_Template)
{
	// Get and set the name
	CButeTools::GetValidatedString(m_buteMgr, szTagName, "Name", pORM_Template->m_szKey, sizeof(pORM_Template->m_szKey) );
	CButeTools::GetValidatedString(m_buteMgr, szTagName, "Collective", pORM_Template->m_szCollectiveName, sizeof(pORM_Template->m_szCollectiveName), "" );
	CButeTools::GetValidatedString(m_buteMgr, szTagName, "RelationSet", pORM_Template->m_szRelationSet, sizeof(pORM_Template->m_szRelationSet), "" );
	CButeTools::GetValidatedString(m_buteMgr, szTagName, "RelationData", pORM_Template->m_szDataSet, sizeof(pORM_Template->m_szDataSet), "" );

	// Keep track of the template ID, for lookups later without a string compare.
	pORM_Template->m_nTemplateID = m_listpObjectRelationMgrTemplates.size();
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::ReadTemplate()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
void CRelationButeMgr::ReadTemplate(const char* const szTagName, RelationUser_Template* pRU_Template)
{
	pRU_Template->m_flTimeRelationsLast = CButeTools::GetValidatedDouble(m_buteMgr, szTagName,"RelationTimeout", 50.0);
	CButeTools::GetValidatedString(m_buteMgr, szTagName, "Name", pRU_Template->m_szKey, sizeof(pRU_Template->m_szKey) );

	// Keep track of the template ID, for lookups later without a string compare.
	pRU_Template->m_nTemplateID = m_listpRelationUserTemplates.size();

	// Fill the new relation Maintainer with all relations listed under it.
	int		nRelation = 0;
	char	szAttName[RELATION_VALUE_LENGTH];
	char	szLine[512];	// Stores the line associated with the AttName
	while ( true )
	{
		// Retrieve the string from the buteMgr.
		sprintf(szAttName, "Relation%d", nRelation);
		m_buteMgr.GetString(szTagName, szAttName, "", szLine, sizeof(szLine) );
		if( !m_buteMgr.Success( ))
			break;

		// Retreive the values from the string.
		RelationDescription RD;
		CRelationTools::GetInstance()->ParseRelation( szLine, &RD );

		// Add the new RelationDescription to the list we are maintaining.
		pRU_Template->m_vecRelationDescriptions.push_back(RD);

		// Get the next string
		nRelation++;
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::ReadTemplate()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
void CRelationButeMgr::ReadTemplate(const char* const szTagName, DataUser_Template* pRD_Template)
{
	// Get the name so that we can convert it to an unique ID representing
	// the Maintainer for later lookups.

	CButeTools::GetValidatedString(m_buteMgr, szTagName, "Name", pRD_Template->m_szKey, sizeof(pRD_Template->m_szKey) );

	// Keep track of the template ID, for lookups later without a string compare.
	pRD_Template->m_nTemplateID = m_listpRelationDataTemplates.size();

	// Fill the new relation Maintainer with all relations listed under it.

	// Variables used to store the retreived output data the ParseData call.
	RelationTraits::eRelationTraits eTrait = RelationTraits::kTrait_Invalid;
	char szValue[128];
	szValue[0] = '\0';

	// Used to iterate through the data in the bute file
	int nData = 0;
	char szAttName[RELATION_VALUE_LENGTH];

	// Stores the line associated with the AttName
	char szLine[512];

	while ( true )
	{
		// Retrieve the string from the buteMgr.
		sprintf(szAttName, "Data%d", nData);
		m_buteMgr.GetString(szTagName, szAttName, "", szLine, sizeof(szLine) );
		if( !m_buteMgr.Success( ))
			break;

		// Retreive the values from the string.
		CRelationTools::GetInstance()->ParseData( szLine, &eTrait, &(*szValue), sizeof( szValue ) );

		// Add the new triple to the CRelationUser.
		if( szValue[0] && ( eTrait != RelationTraits::kTrait_Invalid ) )
		{
			pRD_Template->m_RelationData.SetTraitValue( eTrait, szValue );
		}

		// Get the next string
		nData++;
	}
}


//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::Term()
//
//	PURPOSE:	Deallocate any allocated memory (all memory allocated by the
//				CRelationButeMgr should be deallocated here)
//
//----------------------------------------------------------------------------
void CRelationButeMgr::Term()
{
	// Delete all of the pointers to Users
	_listRelationUserTemplates::iterator itpRelationUser = m_listpRelationUserTemplates.begin();
	while ( itpRelationUser != m_listpRelationUserTemplates.end() )
	{
		debug_delete ( (*itpRelationUser) );
		m_listpRelationUserTemplates.erase( itpRelationUser );
		itpRelationUser = m_listpRelationUserTemplates.begin();
	}

	// Delete all of the pointers to Relations
	_listRelationDataTemplates::iterator itpData = m_listpRelationDataTemplates.begin();
	while ( itpData != m_listpRelationDataTemplates.end() )
	{
		debug_delete ( (*itpData) );
		m_listpRelationDataTemplates.erase( itpData );
		itpData = m_listpRelationDataTemplates.begin();
	}

	// Delete all of the pointers to Collectives
	_listCollectiveTemplates::iterator itpCollective = m_listpCollectiveTemplates.begin();
	while ( itpCollective != m_listpCollectiveTemplates.end() )
	{
		debug_delete ( (*itpCollective) );
		m_listpCollectiveTemplates.erase(itpCollective);
		itpCollective = m_listpCollectiveTemplates.begin();
	}

	// Delete all of the pointers to ObjectRelationMgrs
	_listObjectRelationMgrTemplates::iterator itObjectRelation = m_listpObjectRelationMgrTemplates.begin();
	while ( itObjectRelation != m_listpObjectRelationMgrTemplates.end() )
	{
		debug_delete ( (*itObjectRelation) );
		m_listpObjectRelationMgrTemplates.erase(itObjectRelation);
		itObjectRelation = m_listpObjectRelationMgrTemplates.begin();
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationButeMgr::FillRelationSet()
//
//	PURPOSE:	For when we want to extract a relation independant of a
//				character, object or system.  This is useful for determining
//				how a player would feel about someone else for instance
//				without having an instance of the player.
//
//				Returns by parameter a RelationSet representing how a
//				prototypical instance of szTemplateName.
//
//----------------------------------------------------------------------------
void CRelationButeMgr::FillRelationSet(const char* const szTemplateName, RelationSet* pOut)
{
	// Insure that we have a valid output paramenter.
	ASSERT(pOut);
	if (!pOut)
		return;

	ASSERT(szTemplateName);
	if (!szTemplateName)
		return;

	// Insure that the name maps validly.  If it doesn't, then assert and return.
	_mapNameToID::const_iterator RelationTemplateIterator = m_mapRelationNameToID.find( std::string(szTemplateName) );
	if (RelationTemplateIterator == m_mapRelationNameToID.end())
	{
		AIASSERT1( 0, NULL, "Unable to find template with name %s", szTemplateName );
	}

	uint32 nTemplateID = RelationTemplateIterator->second;
	if ( nTemplateID < 0 || nTemplateID > m_listpRelationUserTemplates.size() )
	{
		AIASSERT1( 0, NULL, "Out of range RelationSetID: %d", nTemplateID );
		return;
	}

	// Copy all of the relations to the Set.
	{for (std::vector<RelationDescription>::iterator iter = m_listpRelationUserTemplates[nTemplateID]->m_vecRelationDescriptions.begin();
		 iter != m_listpRelationUserTemplates[nTemplateID]->m_vecRelationDescriptions.end();
		 iter++)
	{
		pOut->AddRelation(*iter);
	}}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::CRelationUser()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
CRelationUser::CRelationUser(IMomentoUser* pMomentoCallbackReceiver) :
			m_pMomentoCallbackReceiver( pMomentoCallbackReceiver ),
			m_flTimeRelationsLast( 0 )
{
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::~CRelationUser()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
CRelationUser::~CRelationUser()
{
	RemoveAllRelationMomentos();
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::operator=()
//
//	PURPOSE:	Handles
//
//----------------------------------------------------------------------------
CRelationUser& CRelationUser::operator=(const RelationUser_Template& rhs )
{
	AIASSERT( m_pMomentoCallbackReceiver, NULL, "No Callback Reciever Specified!" );

	// Clear out all of the relations we have, as the last thing we want to
	// do is end up in an undefined state because we have two sets of
	// relations overlapping.  This can particularly happen for the player,
	// who reinits frequently.
	ClearRelationUser();

	// Loop through all of the relationdescriptions, turning them into momentos.
	// this will allow us a consistent interface to the system -- relationships
	// will only be added through momentos.
	std::vector<RelationDescription>::const_iterator iter = rhs.m_vecRelationDescriptions.begin();
	for ( ; iter != rhs.m_vecRelationDescriptions.end(); ++iter )
	{
		AIASSERT( CanAddRelation( (*iter) ), NULL, "Failed to add relation on Template Copy." );
		if ( CanAddRelation( (*iter) ) )
		{
			// Add the Relation with a NULL owner, indicating that this is NOT
			// a temp relation.  It is permanent, and should never be removed.
			AddRelation( (*iter), true );
		}
	}

	m_flTimeRelationsLast = rhs.m_flTimeRelationsLast;

	return *this;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::Save()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
int CRelationUser::Save(ILTMessage_Write *pMsg)
{
	m_RelationSet.Save(pMsg);

	SAVE_INT( m_Momentos.size() );

	// Save each of the Active Relationships
	std::for_each( m_Momentos.begin(),
		m_Momentos.end(),
		std::bind2nd( std::mem_fun1(&RelationMomento::Save), pMsg ));

	SAVE_TIME( m_flTimeRelationsLast );

	return 0;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::Load()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
int CRelationUser::Load(ILTMessage_Read *pMsg, IMomentoUser* pUser)
{
	m_RelationSet.Load(pMsg);

	m_pMomentoCallbackReceiver = pUser;

	// Load each of the relationships, passing it the RelationUser that
	// the they are hooked to, load the rest of the information, then add
	// them to the list of active momentos
	int nActiveRelations;
	LOAD_INT( nActiveRelations );
	for ( int x = 0; x < nActiveRelations; x++ )
	{
		RelationMomento* pMomento = debug_new1( RelationMomento, m_pMomentoCallbackReceiver );
		pMomento->Load(pMsg);
		m_Momentos.push_back(pMomento);
	}

	LOAD_TIME( m_flTimeRelationsLast );

	return 0;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::RemoveAllRelationMomentos()
//
//	PURPOSE:	Function to remove all RelationMomentos from the manager.
//
//				This is a cleanup function, so don't do any callbacks on
//				momento deletion
//
//----------------------------------------------------------------------------
void CRelationUser::RemoveAllRelationMomentos()
{
	std::transform(
		m_Momentos.begin(),
		m_Momentos.end(),
		m_Momentos.begin(),
		DeleteMomentoWithoutCallback()
		);

	m_Momentos.clear();
}


//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::CanAddRelation()
//
//	PURPOSE:	Returns true if the relation can be added, false if it cannot
//
//----------------------------------------------------------------------------
bool CRelationUser::CanAddRelation(const RelationDescription& RD) const
{
	return ( !HasMatchingRelationMomento(RD) && m_RelationSet.CanAddRelation(RD) );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::CanRemoveRelation()
//
//	PURPOSE:	Returns true if the relation can be removed, false if it cannot
//
//----------------------------------------------------------------------------
bool CRelationUser::CanRemoveRelation(const RelationDescription& RD) const
{
	return ( m_RelationSet.CanRemoveRelation(RD) );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::AddRelation()
//
//	PURPOSE:	Adds a new relation to our list.
//
//----------------------------------------------------------------------------
void CRelationUser::AddRelation(const RelationDescription& RD, bool bPermanent )
{
	AIASSERT( m_pMomentoCallbackReceiver, NULL, "No CallbackReceiver specified" );
	AIASSERT( CanAddRelation(RD), NULL, "Cannot add relation -- check before calling" );
	float flExperationTime = m_flTimeRelationsLast + g_pLTServer->GetTime();
	RelationMomento* pMomento = debug_new3( RelationMomento, m_pMomentoCallbackReceiver, flExperationTime, RD );
	if (bPermanent == true)
	{
		pMomento->SetTypePermanent();
	}
	pMomento->AddMomento( m_Momentos );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::HasMatchingRelationMomento()
//
//	PURPOSE:	returns true if the relation specified by the momento is
//				currently Maintainer, false if it is not.
//
//----------------------------------------------------------------------------
bool CRelationUser::HasMatchingRelationMomento(const RelationDescription& RD) const
{
	RelationDescription InstRD = RD;

	_listMomentos::const_iterator itFound = std::find_if(
		m_Momentos.begin(),
		m_Momentos.end(),
		std::bind2nd( Momento_RelationData_Equality(), &InstRD )
		);

	// If we are not at the end, then we did find a matching momento, so return
	// true;
	return ( itFound != m_Momentos.end() );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::RemoveRelation()
//
//	PURPOSE:	Removes a Momento matching the description from the list.
//				Assumes a matching momento exists
//
//----------------------------------------------------------------------------
void CRelationUser::RemoveRelation(const RelationDescription& RD)
{
	RelationDescription InstRD = RD;
	_listMomentos::iterator itFound = std::find_if( m_Momentos.begin(), m_Momentos.end(), std::bind2nd( Momento_RelationData_Equality(), &InstRD ) );
	AIASSERT( itFound != m_Momentos.end(), NULL, "Attempted to remove relation momento which does not exist in list" );

	// Delete the momento, then remove it from the list.
	RelationMomento* pMomento = (*itFound);
	DeleteMomentoWithCallback()( pMomento );
	m_Momentos.erase( itFound );
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::ClearRelationUser()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
void CRelationUser::ClearRelationUser(void)
{
	m_RelationSet.ClearRelations();
	RemoveAllRelationMomentos();
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::InitRelations()
//
//	PURPOSE:	Initializes the Relation with the RelationTemplate matching
//				the Key, asserts if there is no matching Template.
//
//----------------------------------------------------------------------------
void CRelationUser::InitRelations(const char* const szKey)
{
	CRelationMgr::GetGlobalRelationMgr()->GetButeMgr()->CopyTemplate(szKey, this);
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::ResetRelationTime()
//
//	PURPOSE:
//
//----------------------------------------------------------------------------
void CRelationUser::ResetRelationTime(CObjectRelationMgr* pORM)
{
	_listMomentos::const_iterator it = std::find_if(
		m_Momentos.begin(),
		m_Momentos.end(),
		std::bind2nd( ORMMatchesDescription(), pORM)  );

	if ( it != m_Momentos.end() )
	{
		// Reset the timer!
//		g_pLTServer->CPrint( "ResetRelationTime: %s", (*it)->GetDescription().szValue );
		(*it)->ResetExpiration( m_flTimeRelationsLast + g_pLTServer->GetTime() );
	}
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::Update()()
//
//	PURPOSE:	Added function to check all of the relations to see if any
//				need to be expired.  This only needs to be done once per frame,
//				though frequency is non critical.
//
//				Deleted expired relation momentos by deallocating them, and then
//              removing all NULL pointers from the list.
//
//				All deleted Momentos ought to do callbacks, as the owner should
//				still be valid, and should be able to handle the callback
//
//----------------------------------------------------------------------------
void CRelationUser::Update(bool bCanRemoveExpiredRelations)
{
	if ( bCanRemoveExpiredRelations )
	{
		std::transform(
			m_Momentos.begin(),
			m_Momentos.end(),
			m_Momentos.begin(),
			DeleteMomentoWithCallback()
			);

		m_Momentos.remove_if( MomentoIsNull() );
	}
}


struct SyncObjectRelationMgr :
public std::binary_function<RelationMomento*, CObjectRelationMgr*, bool>
{
	bool operator()(RelationMomento* pMomento, CObjectRelationMgr* pObjectRelationMgr) const
	{
		if (!pObjectRelationMgr->GetRelationUser()->HasMatchingRelationMomento( pMomento->GetDescription() ))
		{
			// Add the new collective relationship which the collective will manage
			pObjectRelationMgr->AddRelation(pMomento->GetDescription());
		}
		return true;
	}
};

//----------------------------------------------------------------------------
//
//	ROUTINE:	CRelationUser::Sync()
//
//	PURPOSE:	Sync the passed in AI to match self.
//
//----------------------------------------------------------------------------
void CRelationUser::Sync(const CObjectRelationMgr* pObjectRelationMgr)
{
	CObjectRelationMgr *pTakeIt = const_cast<CObjectRelationMgr*>(pObjectRelationMgr);
	std::for_each(m_Momentos.begin(), m_Momentos.end(),
		std::bind2nd( SyncObjectRelationMgr(), pTakeIt));

	m_Momentos.remove_if( MomentoIsNull() );
}


//----------------------------------------------------------------------------
//
//	ROUTINE:	CDataUser::operator=()
//
//	PURPOSE:	Copies to the DataUser the specified template
//
//----------------------------------------------------------------------------
CDataUser& CDataUser::operator=( const DataUser_Template& RU_Template )
{
	m_RelationData = RU_Template.m_RelationData;
	return *this;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CDataUser::InitData()
//
//	PURPOSE:	Initializes the Data with the Template Name matching the Key,
//				and the owner name.  Asserts if the key is not found
//
//----------------------------------------------------------------------------
void CDataUser::InitData(const char* const szKey, HOBJECT hOwner)
{
	UBER_ASSERT( hOwner != NULL, "CDataUser::InitData: NULL HOBJECT" );

	// Asserts if the Key is not found
	CRelationMgr::GetGlobalRelationMgr()->GetButeMgr()->CopyTemplate( szKey, this );
	m_RelationData.SetTraitValue( RelationTraits::kTrait_Name, ToString(hOwner) );
}

int CDataUser::Save(ILTMessage_Write *pMsg)
{
	m_RelationData.Save(pMsg);
	return 0;
}

int CDataUser::Load(ILTMessage_Read *pMsg)
{
	m_RelationData.Load(pMsg);
	return 0;
}

std::ostream& operator << (std::ostream& os, const CRelationUser& User)
{
	for( _listMomentos::const_iterator it = User.m_Momentos.begin(); it != User.m_Momentos.end(); ++it )
	{
		const RelationMomento& Momento = *(*it);
		os << Momento;
	}

	return (os);
}





//----------------------------------------------------------------------------
//
//	ROUTINE:	RelationMomento()
//
//	PURPOSE:	Constructor for Loading, when we want to create a momento
//				without fully initializing it.
//
//----------------------------------------------------------------------------
RelationMomento::RelationMomento(IMomentoUser* pUser) :
	m_pMomentoUser( pUser ),
	m_flExpiration( 0 ),
	m_eRelationMomentoType(eTemporary)
{
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	RelationMomento::RelationMomento()
//
//	PURPOSE:	Constructor adds the relation to the RelationUser
//
//----------------------------------------------------------------------------
RelationMomento::RelationMomento(IMomentoUser* pUser, float time, const RelationDescription& RD ) :
	m_pMomentoUser( pUser ),
	m_flExpiration( time ),
	m_eRelationMomentoType(eTemporary),
	m_RD( RD )
{
}

// Save/Load:
int RelationMomento::Save(ILTMessage_Write *pMsg)
{
	SAVE_CHARSTRING( m_RD.szValue );
	SAVE_DWORD( m_RD.eTrait );
	SAVE_DWORD( m_RD.eAlignment );
	SAVE_TIME( m_flExpiration );
	SAVE_BYTE( m_eRelationMomentoType);

	return 0;
}

int RelationMomento::Load(ILTMessage_Read *pMsg)
{
	LOAD_CHARSTRING( m_RD.szValue, sizeof(m_RD.szValue) );
	LOAD_DWORD_CAST( m_RD.eTrait, RelationTraits::eRelationTraits );
	LOAD_DWORD_CAST( m_RD.eAlignment, CharacterAlignment );
	LOAD_TIME( m_flExpiration );
	LOAD_BYTE_CAST(m_eRelationMomentoType, eRelationMomentoType);

	return 0;
}

void RelationMomento::AddMomento( _listMomentos& listAddTo )
{
	listAddTo.push_back( this );
	DoAddRelationCallback();
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	RelationMomento::IsExpired()
//
//	PURPOSE:	Returns true if the RelationMomento is expired, false if it is
//				not
//
//----------------------------------------------------------------------------
bool RelationMomento::IsExpired(float flCurrentTime) const
{
	if (m_eRelationMomentoType==ePermanent)
	{
		return false;
	}
	if (m_flExpiration >  flCurrentTime)
	{
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	RelationMomento::DoAddRelationCallback()
//
//	PURPOSE:	Informs the owner of the Relation that the relation is being
//				added.  This allows the owner to respond correctly, as well
//				as
//
//----------------------------------------------------------------------------
void RelationMomento::DoAddRelationCallback(void)
{
	AIASSERT( m_pMomentoUser, NULL, "RelationMomento:DoRemoveCallback: No Owner!" );

	if ( m_pMomentoUser->GetRelationUser()->GetRelations().CanAddRelation(m_RD))
	{
		m_pMomentoUser->GetRelationUser()->SetRelations().AddRelation(m_RD);
	}

	m_pMomentoUser->AddRelationCallback(m_RD);
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	RelationMomento::DoRemoveRelationCallback()
//
//	PURPOSE:	Informs the owner of the Relation that the relation expired by
//				removing the Relation with the Momentos relation description
//
//----------------------------------------------------------------------------
void RelationMomento::DoRemoveRelationCallback(void)
{
	AIASSERT( m_pMomentoUser, NULL, "RelationMomento:DoRemoveCallback: No Owner!" );

	if ( m_pMomentoUser->GetRelationUser()->GetRelations().CanRemoveRelation(m_RD))
	{
		m_pMomentoUser->GetRelationUser()->SetRelations().RemoveSpecificRelation(m_RD);
	}

	m_pMomentoUser->RemoveRelationCallback(m_RD);
}


std::ostream& operator << (std::ostream &os, const RelationMomento& Momento)
{
	os << " " << CRelationTools::GetInstance()->ConvertAlignmentEnumToString(Momento.m_RD.eAlignment);
	os << " " << CRelationTools::GetInstance()->ConvertTraitEnumToString(Momento.m_RD.eTrait);
	os << " " << Momento.m_RD.szValue;
	os << " (" << ((Momento.m_eRelationMomentoType==RelationMomento::ePermanent) ? "Perm" : "Temp") << ")" << '\n';
	return os;
}


