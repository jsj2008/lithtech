//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : PropList.cpp
//
//	PURPOSE	  : Implements CPropList and related classes.
//
//	CREATED	  : December 1 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "proplist.h"
#include "genericprop_setup.h"
#include "geomroutines.h"
#include "ltamgr.h"
#include "ltasaveutils.h"


// ----------------------------------------------------------------------- //
// CBaseProp.
// ----------------------------------------------------------------------- //

CBaseProp::CBaseProp( const char *pName )
{
	m_PropFlags = 0;

	m_pPropDef = NULL;

	if( pName )
		strncpy( m_Name, pName, MAX_PROPNAME_LEN );
	else
		m_Name[0] = 0;
}

void CBaseProp::Copy( CBaseProp *pOther )
{ 
	m_pPropDef = pOther->m_pPropDef; 
}

// ----------------------------------------------------------------------- //
// CStringProp.
// ----------------------------------------------------------------------- //

CStringProp::CStringProp( const char *pName ) : CBaseProp(pName)
{
	m_String[0] = 0;
	m_Type = LT_PT_STRING;
}

void CStringProp::LoadData( CAbstractIO &file )
{
	file.ReadString( m_String, MAX_STRINGPROP_LEN );
}

void CStringProp::SaveData( CAbstractIO &file )
{
	if( strlen( m_String ) < MAX_STRINGPROP_LEN )
		file.WriteString( m_String );
	else
		file.WriteString( "" );
}

uint32 CStringProp::DataSize( ) const
{
	if( strlen( m_String ) < MAX_STRINGPROP_LEN )
		return 2 + strlen(m_String);
	else
		return 2;
}

void CStringProp::LoadDataLTA( CLTANode* pNode )
{
	strncpy(m_String,
		PairCdr( CLTAUtil::ShallowFindList(pNode, "data" ) ),
			MAX_STRINGPROP_LEN);
}

void CStringProp::SaveDataLTA( CLTAFile* pFile, uint32 level )
{
	if( strlen(m_String) )
	{
		pFile->WriteStrF("( data \"%s\") ", m_String );
	}
}

void CStringProp::SaveDataLTA( CLTANodeBuilder &lb )
{
	if( strlen(m_String) )
	{
		SaveStringLTANamed("data",lb,m_String);
	}
}


void CStringProp::Copy( CBaseProp *pOther )
{
	CBaseProp::Copy(pOther);
	memcpy( m_String, ((CStringProp*)pOther)->m_String, MAX_STRINGPROP_LEN );
}

void CStringProp::SetupString(char *pStr)
{
	SAFE_STRCPY(m_String, pStr);
}

void CStringProp::SetString(const char *lpszString)
{
	if (!lpszString)
	{
		return;
	}

	int nLength=sizeof(m_String);

	if (nLength > 0)
	{
		strncpy(m_String, lpszString, sizeof(m_String));	
		m_String[sizeof(m_String)-1]='\0';
	}
}

void CStringProp::SetupGenericProp(GenericProp *pProp)
{
	gp_InitString(pProp, m_String);
}



// ----------------------------------------------------------------------- //
// CVectorProp.
// ----------------------------------------------------------------------- //

CVectorProp::CVectorProp( const char *pName ) : CBaseProp(pName)
{
	m_Vector.Init( 0.0f, 0.0f, 0.0f );
	m_Type = LT_PT_VECTOR;
}

void CVectorProp::LoadData( CAbstractIO &file )
{
	file >> m_Vector.x >> m_Vector.y >> m_Vector.z;
}

void CVectorProp::SaveData( CAbstractIO &file )
{
	file << m_Vector.x << m_Vector.y << m_Vector.z;
}

uint32 CVectorProp::DataSize( ) const
{
	return sizeof(m_Vector);
}

void CVectorProp::LoadDataLTA( CLTANode* pNode )
{
	CLTANode* pVectorNode = PairCdrNode(CLTAUtil::ShallowFindList( CLTAUtil::ShallowFindList(pNode, "data" ), "vector"));
	if( pVectorNode && pVectorNode->GetNumElements() == 3 )
	{
		m_Vector.x = GetFloat(pVectorNode->GetElement(0));
		m_Vector.y = GetFloat(pVectorNode->GetElement(1));
		m_Vector.z = GetFloat(pVectorNode->GetElement(2));
	}

}


void CVectorProp::SaveDataLTA( CLTAFile* pFile, uint32 level )
{
	pFile->WriteStrF("( data ( vector (%f %f %f) ) ) ", m_Vector.x, m_Vector.y, m_Vector.z );
}

void CVectorProp::SaveDataLTA( CLTANodeBuilder &lb )
{
	lb.Push("data");
		SaveLTVecLTANamed("vector", lb, m_Vector);
	lb.Pop();
}


void CVectorProp::Copy( CBaseProp *pOther )
{
	CBaseProp::Copy(pOther);
	m_Vector = ((CVectorProp*)pOther)->m_Vector;
}

void CVectorProp::SetupString(char *pStr)
{
	m_Vector.Init();
	sscanf(pStr, "%f %f %f", &m_Vector.x, &m_Vector.y, &m_Vector.z);
}

void CVectorProp::SetupGenericProp(GenericProp *pProp)
{
	gp_InitVector(pProp, &m_Vector);
}



// ----------------------------------------------------------------------- //
// CRotationProp.
// ----------------------------------------------------------------------- //

CRotationProp::CRotationProp(const char *pName) : CBaseProp(pName)
{
	m_Type = LT_PT_ROTATION;
}

void CRotationProp::LoadData( CAbstractIO &file )
{
	float fDummy;

	file >> m_EulerAngles.x;
	file >> m_EulerAngles.y;
	file >> m_EulerAngles.z;
	file >> fDummy; // Used to be stored as DRotations so we have to leave room.
}

void CRotationProp::SaveData( CAbstractIO &file )
{
	file << m_EulerAngles.x;
	file << m_EulerAngles.y;
	file << m_EulerAngles.z;
	file << (float)0.0f; // Used to be stored as DRotations so we have to leave room.
}

uint32 CRotationProp::DataSize( ) const
{
	return sizeof(m_EulerAngles) + sizeof(float);
}

void CRotationProp::LoadDataLTA( CLTANode* pNode )
{
	CLTANode* pRotationNode = PairCdrNode(CLTAUtil::ShallowFindList( CLTAUtil::ShallowFindList(pNode, "data" ), "eulerangles"));
	if( pRotationNode && pRotationNode->GetNumElements() == 3 )
	{
		m_EulerAngles.x = GetFloat(pRotationNode->GetElement(0));
		m_EulerAngles.y = GetFloat(pRotationNode->GetElement(1));
		m_EulerAngles.z = GetFloat(pRotationNode->GetElement(2));
	}
}

void CRotationProp::SaveDataLTA( CLTAFile* pFile, uint32 level )
{
	// PrependTabs(pFile, level);
	pFile->WriteStrF("( data (eulerangles (%f %f %f) ) ) ", 
		m_EulerAngles.x, 
		m_EulerAngles.y, 
		m_EulerAngles.z );
}

void CRotationProp::SaveDataLTA( CLTANodeBuilder &lb )
{
	lb.Push("data");
		SaveLTVecLTANamed("eulerangles", lb, m_EulerAngles );
	lb.Pop();
}

void CRotationProp::Copy(CBaseProp *pOther)
{
	CBaseProp::Copy(pOther);
	m_EulerAngles = ((CRotationProp*)pOther)->GetEulerAngles();
}

void CRotationProp::SetupString(char *pStr)
{
	sscanf(pStr, "%f %f %f", &m_EulerAngles.x, &m_EulerAngles.y, &m_EulerAngles.z);
}

void CRotationProp::SetupGenericProp(GenericProp *pProp)
{
	gp_InitRotation(pProp, &m_EulerAngles);
}



// ----------------------------------------------------------------------- //
// CColorProp.
// ----------------------------------------------------------------------- //

CColorProp::CColorProp( const char *pName ) : CVectorProp(pName)
{
	m_Type = LT_PT_COLOR;
}



// ----------------------------------------------------------------------- //
// CRealProp.
// ----------------------------------------------------------------------- //

CRealProp::CRealProp( const char *pName ) : CBaseProp(pName)
{
	m_Value = 0.0f;
	m_Type = LT_PT_REAL;
}

void CRealProp::LoadData( CAbstractIO &file )
{
	file >> m_Value;
}

void CRealProp::SaveData( CAbstractIO &file )
{
	file << m_Value;
}

uint32 CRealProp::DataSize( ) const
{
	return sizeof(m_Value);
}

void CRealProp::LoadDataLTA( CLTANode* pNode )
{
	m_Value = GetFloat( PairCdrNode(CLTAUtil::ShallowFindList(pNode, "data" )) );
}

void CRealProp::SaveDataLTA( CLTAFile* pFile, uint32 level )
{
	pFile->WriteStrF("( data %f )", m_Value ); 	// file << m_Value;
}

void CRealProp::SaveDataLTA( CLTANodeBuilder &lb  )
{
	SaveFloatLTANamed("data", lb, m_Value );
}

void CRealProp::Copy( CBaseProp *pOther )
{
	CBaseProp::Copy(pOther);
	m_Value = ((CRealProp*)pOther)->m_Value;
}

void CRealProp::SetupString(char *pStr)
{
	m_Value = (float)atof(pStr);
}

void CRealProp::SetupGenericProp(GenericProp *pProp)
{
	gp_InitFloat(pProp, m_Value);
}



// ----------------------------------------------------------------------- //
// CBoolProp.
// ----------------------------------------------------------------------- //

CBoolProp::CBoolProp( const char *pName ) : CBaseProp(pName)
{
	m_Value = 0;
	m_Type = LT_PT_BOOL;
}

void CBoolProp::LoadData( CAbstractIO &file )
{
	file >> m_Value;
}

void CBoolProp::SaveData( CAbstractIO &file )
{
	file << m_Value;
}

uint32 CBoolProp::DataSize( ) const
{
	return sizeof(m_Value);
}

void CBoolProp::LoadDataLTA( CLTANode* pNode )
{
	m_Value = GetBool( PairCdrNode(CLTAUtil::ShallowFindList(pNode, "data" )) );
}

void CBoolProp::SaveDataLTA( CLTAFile* pFile, uint32 level )
{
	pFile->WriteStrF("( data %d )", m_Value ? 1 : 0 ); 	// file << m_Value;
}

void CBoolProp:: SaveDataLTA( CLTANodeBuilder &lb )
{
	SaveBoolLTANamed("data",lb,m_Value);
}

void CBoolProp::Copy( CBaseProp *pOther )
{
	CBaseProp::Copy(pOther);
	m_Value = ((CBoolProp*)pOther)->m_Value;
}

void CBoolProp::SetupString(char *pStr)
{
	m_Value = (char)atof(pStr);
}

void CBoolProp::SetupGenericProp(GenericProp *pProp)
{
	gp_InitFloat(pProp, (float)m_Value);
}




// ----------------------------------------------------------------------- //
//      ROUTINE:        CPropList functions.
// ----------------------------------------------------------------------- //

CPropList::CPropList()
{
}


CPropList::~CPropList()
{
	Term();
}


void CPropList::Term()
{
	DeleteAndClearArray(m_Props);
}


CBaseProp* CPropList::GetProp(const char *pName, BOOL bAssert, uint32 *pIndex)
{
	for(DWORD i=0; i < m_Props; i++)
	{
		if( strcmp(m_Props[i]->m_Name, pName) == 0 )
		{
			if(pIndex)
				*pIndex = i;
			
			return m_Props[i];
		}
	}

	if( bAssert )
	{
		ASSERT( FALSE );
	}

	return NULL;
}

// Find prop with only part of it's name
CBaseProp* CPropList::GetPropPartial(const char *pName, BOOL bAssert, uint32 *pIndex)
{
	char szPartial[MAX_PROPNAME_LEN+1];
	char szProp[MAX_PROPNAME_LEN+1];

	strncpy( szPartial, pName, MAX_PROPNAME_LEN );
	_strupr( szPartial );

	for(DWORD i=0; i < m_Props; i++)
	{
		strncpy( szProp, m_Props[i]->m_Name, MAX_PROPNAME_LEN );
		_strupr( szProp );
		if( strstr( szProp, szPartial ))
		{
			if(pIndex)
				*pIndex = i;
			
			return m_Props[i];
		}
	}

	if( bAssert )
	{
		ASSERT( FALSE );
	}

	return NULL;
}


CBaseProp* CPropList::GetMatchingProp(const char *pName, int type)
{
	CBaseProp	*pRet;

	if(!(pRet = GetProp(pName)))
		return NULL;

	if(pRet->m_Type == type)
		return pRet;

	return NULL;
}


CBaseProp* CPropList::GetPropByFlagsAndType(uint32 flags, int type, uint32 *index)
{
	DWORD i;
	CBaseProp *pProp;

	for(i=0; i < m_Props; i++)
	{
		pProp = m_Props[i];
	
		if(pProp->m_Type == type && ((pProp->m_PropFlags & flags) == flags))
		{
			if(index)
				*index = i;

			return pProp;
		}
	}

	return NULL;
}


void CPropList::CopyValues( CPropList *pOtherList )
{
	CBaseProp	*pProp, *pOther;

	Term();
	m_Props.SetSize( pOtherList->m_Props );
	for( DWORD i=0; i < pOtherList->m_Props; i++ )
	{
		pOther = pOtherList->m_Props[i];
		pProp = pOther->CreateSameKind();
		m_Props[i] = pProp;

		ASSERT( pProp );
		
		pProp->m_Type = pOther->m_Type;
		strncpy( pProp->m_Name, pOther->m_Name, MAX_PROPNAME_LEN );
		pProp->m_PropFlags = pOther->m_PropFlags;
		pProp->Copy( pOther );
	}
}

// Copies the matching values (same name and type) from one list to another.
void CPropList::CopyMatchingValues(CPropList *pSourceList)
{
	// Loop through the property list copying common values
	int i;
	for (i=0; i < pSourceList->GetSize(); i++)
	{
		// The property for this index
		CBaseProp *pSrcProp=pSourceList->GetAt(i);

		// Find the matching property for this index
		CBaseProp *pMatchingProp=GetMatchingProp(pSrcProp->GetName(), pSrcProp->GetType());

		// Check to see if we found a property
		if (pMatchingProp)
		{
			// Copy the property from the old class to the new class
			pMatchingProp->Copy(pSrcProp);
		}
	}
}

//determines if the property lists are the same, but ignores properties listed in the filter
bool CPropList::SameAs(CPropList* pPropList, uint32 nNumFilters, const char** pszFilter)
{
	uint32 mysize = GetSize();
	uint32 othersize = pPropList->GetSize();
	if( mysize == othersize )
	{
		for( uint32 i = 0; i < mysize; i++ )
		{
			//see if we should filter this property out

			bool bFilter = false;
			for(uint32 nCurrFilter = 0; nCurrFilter < nNumFilters; nCurrFilter++)
			{
				if(stricmp(pszFilter[nCurrFilter], m_Props[i]->GetName()) == 0)
				{
					bFilter = true;
					break;
				}
			}

			if(!bFilter)
			{
				if( !m_Props[i]->SameAs(pPropList->m_Props[i]) )
				{
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool CPropList::SameAs(CPropList* pPropList)
{
	uint32 mysize = GetSize();
	uint32 othersize = pPropList->GetSize();
	if( mysize == othersize )
	{
		for( uint32 i = 0; i < mysize; i++ )
		{
			if( !m_Props[i]->SameAs(pPropList->m_Props[i]) )
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

CPropListContainer::CPropListContainer()
{
	Init();
}

void CPropListContainer::Init()
{
	m_propList.Term();
	m_propList.Init();
}

uint32 CPropListContainer::AddPropList(CPropList* propList, uint32 nNumFilter, const char** pszFilter)
{
	const uint32 size = m_propList.GetSize();

	for( uint32 i = 0; i < size; i++ )
	{
		if( m_propList[i]->SameAs(propList, nNumFilter, pszFilter) )
			return i;
	}

	m_propList.Push(propList);
	return size;
}

uint32 CPropListContainer::AddPropList(CPropList* propList, bool alwaysAdd)
{
	const uint32 size = m_propList.GetSize();

	// don't look for matching properties if alwaysAdd is true
	if( !alwaysAdd )
	{
		for( uint32 i = 0; i < size; i++ )
		{
			if( m_propList[i]->SameAs(propList) )
				return i;
		}
	}

	m_propList.Push(propList);
	return size;
}

void CPropListContainer::UnloadFromMemory()
{
	
	uint32 size = m_propList.GetSize();
	for( uint32 i = 0; i < size; i++ )
	{
		delete m_propList[i];
	}
	Init();
	
}

void CPropListContainer::SaveTBW( CAbstractIO& OutFile )
{
	//write out the size
	uint32 nNumGroups = m_propList.GetSize();
	OutFile << nNumGroups;

	for(uint32 nCurrGroup = 0; nCurrGroup < nNumGroups; nCurrGroup++)
	{
		CPropList* pPropList = m_propList[nCurrGroup];

		OutFile << pPropList->m_Props.GetSize();

		for(uint32 k=0; k	< pPropList->m_Props; k++ )
		{
			CBaseProp* pProp = pPropList->m_Props[k];

			OutFile << (BYTE)pProp->m_Type;
			OutFile.WriteString( pProp->m_Name );
			
			OutFile << (uint32)pProp->m_PropFlags;

			pProp->SaveData( OutFile );
		}	
	}
}


void CPropListContainer::SaveLTA( CLTAFile* pFile, uint32 level )
{
	
	uint32 size = m_propList.GetSize();

	CBaseProp		*pProp;

	PrependTabs(pFile, level);
	pFile->WriteStr("( globalproplist (");
	for( uint32 i = 0; i < size; i++ )
	{
		PrependTabs(pFile, level+1);
		pFile->WriteStr("( proplist ( ");
			for( uint32 k=0; k	< m_propList[i]->m_Props; k++ )
			{
				pProp = m_propList[i]->m_Props[k];	
				PrependTabs(pFile, level+2 );
				pFile->WriteStr("( " );
					switch(pProp->m_Type)
					{
					case LT_PT_STRING:		pFile->WriteStr(" string ");	break;
					case LT_PT_VECTOR:		pFile->WriteStr(" vector ");	break;
					case LT_PT_COLOR:		pFile->WriteStr(" color ");		break;
					case LT_PT_REAL:		pFile->WriteStr(" real ");		break;
					case LT_PT_FLAGS:		pFile->WriteStr(" flags ");		break;
					case LT_PT_BOOL:		pFile->WriteStr(" bool ");		break;
					case LT_PT_LONGINT:	pFile->WriteStr(" longint ");   break;
					case LT_PT_ROTATION:	pFile->WriteStr(" rotation ");	break; 
					default:
						pFile->WriteStr(" invalid ");
					}

					pFile->WriteStrF("\"%s\"", pProp->m_Name );

					pFile->WriteStr(" ( " );
						if( pProp->m_PropFlags & PF_HIDDEN )
							pFile->WriteStr("hidden " );
						if( pProp->m_PropFlags & PF_RADIUS )
							pFile->WriteStr("radius " );
						if( pProp->m_PropFlags & PF_ORTHOFRUSTUM )
							pFile->WriteStr("orthofrustum " );
						if( pProp->m_PropFlags & PF_MODEL )
							pFile->WriteStr("model " );
						if( pProp->m_PropFlags & PF_DISTANCE )
							pFile->WriteStr("distance " );
						if( pProp->m_PropFlags & PF_DIMS )
							pFile->WriteStr("dims " );
						if( pProp->m_PropFlags & PF_FIELDOFVIEW )
							pFile->WriteStr("fieldofview " );
						if( pProp->m_PropFlags & PF_LOCALDIMS )
							pFile->WriteStr("localdims " );
						if( pProp->m_PropFlags & PF_GROUPOWNER )
							pFile->WriteStr("groupowner " );
						if( pProp->m_PropFlags & PF_GROUPMASK )
							pFile->WriteStrF("group%d ", GET_PF_GROUP(pProp->m_PropFlags));
						if( pProp->m_PropFlags & PF_COMPOSITETYPE )
							pFile->WriteStr("compositetype " );
						if( pProp->m_PropFlags & PF_EVENT )
							pFile->WriteStr("event " );
						if( pProp->m_PropFlags & PF_FOVRADIUS )
							pFile->WriteStr("fovradius " );
						if( pProp->m_PropFlags & PF_OBJECTLINK )
							pFile->WriteStr("objectlink " );
						if( pProp->m_PropFlags & PF_FILENAME )
							pFile->WriteStr("filename " );
						if( pProp->m_PropFlags & PF_TEXTUREEFFECT)
							pFile->WriteStr("textureeffect");
						if( pProp->m_PropFlags & PF_BEZIERPREVTANGENT )
							pFile->WriteStr("bezierprevtangent " );
						if( pProp->m_PropFlags & PF_BEZIERNEXTTANGENT )
							pFile->WriteStr("beziernexttangent " );
						if( pProp->m_PropFlags & PF_STATICLIST )
							pFile->WriteStr("staticlist " );
						if( pProp->m_PropFlags & PF_DYNAMICLIST )
							pFile->WriteStr("dynamiclist " );
						if( pProp->m_PropFlags & PF_NOTIFYCHANGE )
							pFile->WriteStr("notifychange " );
					pFile->WriteStr(" ) " );
					pProp->SaveDataLTA( pFile, level+3 );

				// PrependTabs(pFile, level+2 );
				pFile->WriteStr(") " );
			}
		PrependTabs(pFile, level+1);
		pFile->WriteStr(") )");
	}
	PrependTabs(pFile, level);
	pFile->WriteStr(" ) )");	
}
