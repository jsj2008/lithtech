// ----------------------------------------------------------------------- //
//
// MODULE  : NamedObjectList.cpp
//
// PURPOSE : Class implementation for an aggregate used to include a list
//           of named objects as a member of another object.
//
// CREATED : 07/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "NamedObjectList.h"

// ----------------------------------------------------------------------- //

CMDMGR_BEGIN_REGISTER_CLASS( CNamedObjectList )
CMDMGR_END_REGISTER_CLASS( CNamedObjectList, IAggregate )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::CNamedObjectList()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CNamedObjectList::CNamedObjectList()
:	IAggregate			( "CNamedObjectList" ),
	m_hObject			( NULL )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::~CNamedObjectList()
//
//	PURPOSE:	Destructor - deallocate lists
//
// ----------------------------------------------------------------------- //

CNamedObjectList::~CNamedObjectList()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CNamedObjectList::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
	case MID_SAVEOBJECT :
		{
			Save( (ILTMessage_Write*)pData, (uint8)fData );
		}
		break;

	case MID_LOADOBJECT :
		{
			Load( (ILTMessage_Read*)pData, (uint8)fData );
		}
		break;
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void CNamedObjectList::ReadProp(const GenericPropList *pProps, const char* pszObjectType)
{
	// Read list of objects. This list may start at either 0 or 1, so ignore a
	// blank first string.

	uint32 iObject = 0;
	char szBuffer[128];
	LTSNPrintF( szBuffer, LTARRAYSIZE( szBuffer ), "%s%d", pszObjectType, iObject );

	const char* pszPropString = pProps->GetString( szBuffer, "" );
	while( pszPropString[0] || iObject == 0 )
	{
		if ( !LTStrEmpty(pszPropString) )
		{
			std::string sTmp;
			sTmp = pszPropString;
			m_lstObjectNames.push_back( sTmp );
		}

		++iObject;
		LTSNPrintF( szBuffer, LTARRAYSIZE( szBuffer ), "%s%d", pszObjectType, iObject );
		pszPropString = pProps->GetString( szBuffer, "" );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::InitNamedObjectList
//
//	PURPOSE:	Initialize the list of handles.
//
// ----------------------------------------------------------------------- //

void CNamedObjectList::InitNamedObjectList( HOBJECT hOwner )
{
	m_hObject = hOwner;

	// Reserve space for handles.

	m_lstObjectHandles.reserve( m_lstObjectNames.size() );

	// Find named objects.

	HOBJECT hObject = NULL;
	StringArray::iterator str_it;
	for ( str_it = m_lstObjectNames.begin(); str_it != m_lstObjectNames.end(); ++str_it )
	{
		if( LT_OK != FindNamedObject( str_it->c_str(), hObject, false ) )
		{
			LTASSERT_PARAM1( 0, "CNamedObjectList::InitNamedObjectList: Cannot find named object \"%s\"", str_it->c_str() );
			hObject = NULL;
		}

		// Add handle to the list.

		m_lstObjectHandles.push_back( hObject );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::ClearObjectHandle()
//
//	PURPOSE:	Clear the handle for an object at some index.
//
// ----------------------------------------------------------------------- //

void CNamedObjectList::ClearObjectHandle( uint32 iIndex )
{
	if( iIndex < m_lstObjectHandles.size() )
	{
		m_lstObjectHandles[iIndex] = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::GetObjectName()
//
//	PURPOSE:	Return the name for an object at some index.
//
// ----------------------------------------------------------------------- //

const char*	CNamedObjectList::GetObjectName( uint32 iIndex ) const
{
	if( iIndex < m_lstObjectHandles.size() )
	{
		return m_lstObjectNames[iIndex].c_str();
	}

	return "";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::GetObjectHandle()
//
//	PURPOSE:	Return the handle for an object at some index.
//
// ----------------------------------------------------------------------- //

HOBJECT	CNamedObjectList::GetObjectHandle( uint32 iIndex ) const
{
	if( iIndex < m_lstObjectHandles.size() )
	{
		return m_lstObjectHandles[iIndex];
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::DoesContain
//
//	PURPOSE:	Return true if the list does contain the specified handle.
//
// ----------------------------------------------------------------------- //

bool CNamedObjectList::DoesContain( HOBJECT hQueryObject )
{
	// Find a match.

	HOBJECT hObject;
	HOBJECT_LIST::iterator itObj;
	for( itObj = m_lstObjectHandles.begin(); itObj != m_lstObjectHandles.end(); ++itObj )
	{
		hObject = *itObj;
		if( hObject == hQueryObject )
		{
			return true;
		}
	}

	// No match found.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::Save()
//
//	PURPOSE:	Save lists
//
// ----------------------------------------------------------------------- //

void CNamedObjectList::Save( ILTMessage_Write *pMsg, uint8 nType )
{
	SAVE_HOBJECT( m_hObject );

	// Save object names.

	StringArray::iterator str_it;
	SAVE_DWORD( m_lstObjectNames.size() );
	for ( str_it = m_lstObjectNames.begin(); str_it != m_lstObjectNames.end(); ++str_it )
	{
		SAVE_STDSTRING( (*str_it) );
	}

	// Save object handles.

	HOBJECT hObject;
	HOBJECT_LIST::iterator h_it;
	SAVE_DWORD( m_lstObjectHandles.size() );
	for ( h_it = m_lstObjectHandles.begin(); h_it != m_lstObjectHandles.end(); ++h_it )
	{
		hObject = *h_it;
		SAVE_HOBJECT( hObject );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNamedObjectList::Load()
//
//	PURPOSE:	Load lists
//
// ----------------------------------------------------------------------- //

void CNamedObjectList::Load( ILTMessage_Read *pMsg, uint8 nType )
{
	LOAD_HOBJECT( m_hObject );

	// Load object names.

	std::string strObjectName;
	uint32 cObjectNames;
	LOAD_DWORD( cObjectNames );
	m_lstObjectNames.reserve( cObjectNames );
	for( uint32 iObjectName=0; iObjectName < cObjectNames; ++iObjectName )
	{
		LOAD_STDSTRING( strObjectName );
		m_lstObjectNames.push_back( strObjectName );
	}

	// Load object handles.

	HOBJECT hObject;
	uint32 cObjects;
	LOAD_DWORD( cObjects );
	m_lstObjectHandles.reserve( cObjects );
	for( uint32 iObject=0; iObject < cObjects; ++iObject )
	{
		LOAD_HOBJECT( hObject );
		m_lstObjectHandles.push_back( hObject );
	}
}

