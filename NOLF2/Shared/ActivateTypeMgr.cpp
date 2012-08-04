// ----------------------------------------------------------------------- //
//
// MODULE  : ActivateTypeMgr.cpp
//
// PURPOSE : ActivateTypeMgr implementation
//
// CREATED : 7/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "WeaponMgr.h"
	#include "ActivateTypeMgr.h"
	#include "SFXMsgIds.h"
	#include "MsgIds.h"

//
// Defines...
//

	#define	ATMGR_TAG		"ActivateType"
	
	#define ATMGR_NAME		"Name"
	#define ATMGR_STATEON	"StateON"
	#define ATMGR_STATEOFF	"StateOFF"

//
// Globals...
//

	CActivateTypeMgr	*g_pActivateTypeMgr = LTNULL;
	
	static char s_aTagName[32];
	static char	s_aAttName[100];
	static char s_aBuffer[100];
	
#ifndef _CLIENTBUILD
	
	bool CActivateTypeMgrPlugin::sm_bInitted = false;

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeMgr::CActivateTypeMgr
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CActivateTypeMgr::CActivateTypeMgr()
:	CGameButeMgr()
{
	m_ActivateTypeList.Init( LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateTypeMgr::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

CActivateTypeMgr& CActivateTypeMgr::Instance( )
{
	// Putting the singleton as a static function variable ensures that this
	// object is only created if it is used.
	static CActivateTypeMgr sSingleton;
	return sSingleton;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeMgr::~CActivateTypeMgr
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CActivateTypeMgr::~CActivateTypeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeMgr::Term
//
//  PURPOSE:	Clean up after ourselfs
//
// ----------------------------------------------------------------------- //

void CActivateTypeMgr::Term( )
{
	g_pActivateTypeMgr = LTNULL;

	m_ActivateTypeList.Clear();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeMgr::Init
//
//  PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CActivateTypeMgr::Init( const char *szAttributeFile /* = ATMGR_DEFAULT_FILE  */ )
{
	if( g_pActivateTypeMgr || !szAttributeFile ) return LTFALSE;
	if( !Parse( szAttributeFile )) return LTFALSE;

	// Set the singelton ptr

	g_pActivateTypeMgr = this;

	
	// Read in the properies for each Activate Type Record...

	int nNum = 0;
	sprintf( s_aTagName, "%s%i", ATMGR_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		ACTIVATETYPE	*pActivateType = debug_new( ACTIVATETYPE );

		if( pActivateType && pActivateType->Init( m_buteMgr, s_aTagName ))
		{
			// Set the ID and add it to the list...
			
			pActivateType->nId = nNum;
			m_ActivateTypeList.AddTail( pActivateType );
		}
		else
		{
			debug_delete( pActivateType );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%i", ATMGR_TAG, nNum );
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CActivateTypeMgr::GetActivateType
//
//  PURPOSE:	Get the specified Activate Type record
//
// ----------------------------------------------------------------------- //

ACTIVATETYPE *CActivateTypeMgr::GetActivateType( uint32 nId )
{
	ACTIVATETYPE **ppCur = LTNULL;

	ppCur = m_ActivateTypeList.GetItem( TLIT_FIRST );

	while( ppCur )
	{
		if( *ppCur && (*ppCur)->nId == nId )
		{
			return *ppCur;
		}

		ppCur = m_ActivateTypeList.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CActivateTypeMgr::GetActivateType
//
//  PURPOSE:	Get the specified Activate Type record
//
// ----------------------------------------------------------------------- //

ACTIVATETYPE *CActivateTypeMgr::GetActivateType( const char *pName )
{
	if( !pName ) return LTNULL;
	
	ACTIVATETYPE **ppCur = LTNULL;

	ppCur = m_ActivateTypeList.GetItem( TLIT_FIRST );

	while( ppCur )
	{
		if( *ppCur && (*ppCur)->szName[0] && (!_stricmp( (*ppCur)->szName, pName )) )
		{
			return *ppCur;
		}

		ppCur = m_ActivateTypeList.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ACTIVATETYPE::ACTIVATETYPE
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ACTIVATETYPE::ACTIVATETYPE()
:	nId				( ATMGR_INVALID_ID ),
	szName			( LTNULL )
{
	for( int i = 0; i < eMaxStates; ++i )
	{
		dwStateID[i] = ATMGR_INVALID_ID;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ACTIVATETYPE::~ACTIVATETYPE
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ACTIVATETYPE::~ACTIVATETYPE()
{
	debug_delete( szName );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ACTIVATETYPE::Init
//
//  PURPOSE:	Build the Activate Type struct
//
// ----------------------------------------------------------------------- //

bool ACTIVATETYPE::Init( CButeMgr &ButeMgr, char *aTagName )
{
	if( !aTagName ) return false;

	szName	= GetString( ButeMgr, aTagName, ATMGR_NAME, ATMGR_MAX_NAME_LENGTH );

	dwStateID[eOn]	= (uint32)ButeMgr.GetInt( aTagName, ATMGR_STATEON, ATMGR_INVALID_ID );
	dwStateID[eOff]	= (uint32)ButeMgr.GetInt( aTagName, ATMGR_STATEOFF, ATMGR_INVALID_ID );

	return true;
}

#ifndef _CLIENTBUILD
////////////////////////////////////////////////////////////////////////////
//
// CActivateTypeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use ActivateTypeMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateTypeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CActivateTypeMgrPlugin::PreHook_EditStringList( const char* szRezPath,
														 const char* szPropName,
														 char** aszStrings,
														 uint32* pcStrings,
														 const uint32 cMaxStrings,
														 const uint32 cMaxStringLength )
{
	if( !sm_bInitted )
	{
		char szFile[256] = {0};
		sprintf( szFile, "%s\\%s", szRezPath, ATMGR_DEFAULT_FILE );
		
		CActivateTypeMgr& ActivateType = CActivateTypeMgr::Instance( );
        
		ActivateType.SetInRezFile( LTFALSE );
        ActivateType.Init( szFile );
        
		sm_bInitted = true;

		if( !g_pActivateTypeMgr )
		{
			strcpy( aszStrings[(*pcStrings)++], "<Error - Activate Type Mgr not Inited!>" );
		}
	}

	if( !PopulateStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
	{
		strcpy( aszStrings[(*pcStrings)++], "<Error - Populating String List!>" );
	}
	else
	{
		// Sort the list...
			
		qsort( aszStrings, *pcStrings, sizeof( char * ), CaseInsensitiveCompare );
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateTypeMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

bool CActivateTypeMgrPlugin::PopulateStringList( char** aszStrings,
												 uint32* pcStrings,
												 const uint32 cMaxStrings,
												 const uint32 cMaxStringLength )
{
	ASSERT( aszStrings && pcStrings && g_pActivateTypeMgr ); 
	if( !aszStrings || !pcStrings || !g_pActivateTypeMgr ) return false;

	// always make the first option an invalid Activate Type...

	strcpy( aszStrings[(*pcStrings)++], "<none>" );
	 
	// Add an entry for each Activate Type

	int nNumActivateTypes = g_pActivateTypeMgr->GetNumActivateTypes();

	ACTIVATETYPE *pActivateType = LTNULL;

	for( int i = 0; i < nNumActivateTypes; ++i )
	{
		ASSERT(cMaxStrings > (*pcStrings) + 1);

		pActivateType = g_pActivateTypeMgr->GetActivateType( i );
		if( pActivateType && pActivateType->szName[0] )
		{
			uint32 dwATNameLen = strlen( pActivateType->szName );
			if( dwATNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings )
			{
				strcpy( aszStrings[(*pcStrings)++], pActivateType->szName );
			}
		}
	}
	
	return true;
}

////////////////////////////////////////////////////////////////////////////
//
// CActivateTypeHandler is used to keep the functionality for handling an
// ActivateType within a single class.
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::CActivateTypeHandler
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CActivateTypeHandler::CActivateTypeHandler()
:	m_pBase		( LTNULL ),
	m_nId		( ATMGR_INVALID_ID ),
	m_bDisabled	( false ),
	m_eState	( ACTIVATETYPE::eOn )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::~CActivateTypeHandler
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CActivateTypeHandler::~CActivateTypeHandler()
{
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::Init
//
//  PURPOSE:	Set the base class pointer so we know what object we are 
//				doing the handling for.
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::Init( LPBASECLASS pObj )
{
	if( !pObj ) return;

	m_pBase		= pObj;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::SetActivateType
//
//  PURPOSE:	Set the Id for the ActivateType.  Dont send the message!
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::SetActivateType( uint8 nId )
{
	if( m_nId == nId || !g_pActivateTypeMgr)
		return;

	// Make sure this is a valid type...

	ACTIVATETYPE *pType = g_pActivateTypeMgr->GetActivateType( nId );
	if( pType )
	{
		m_nId = pType->nId;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::SetActivateType
//
//  PURPOSE:	Set the Id for the ActivateType.  Dont send the message!
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::SetActivateType( const char *pName )
{
	if( !g_pActivateTypeMgr || !pName )
		return;

	// Make sure this is a valid type...

	ACTIVATETYPE *pType = g_pActivateTypeMgr->GetActivateType( pName );
	if( pType )
	{
		m_nId = pType->nId;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::CreateActivateTypeMsg
//
//  PURPOSE:	Set the objects specialfx message to create a new 
//				ActivateObject on the client.  Use this method ONLY
//				if the object doesn't already have a SpecialFX message.
//				Otherwise call WriteActivateTypeMsg() from within the
//				objects CreatSFX() method.
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::CreateActivateTypeMsg()
{
	if( !m_pBase || !m_pBase->m_hObject || (m_nId == (uint8)ATMGR_INVALID_ID) )
		return;

	// Send the parent object first...
	
	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
	WriteActivateTypeMsg( cMsg );
	g_pLTServer->SetObjectSFXMessage( m_pBase->m_hObject, cMsg.Read() );

	// And then all of the inherited objects...

	ObjRefNotifierList::iterator iter;
	for( iter = m_lstInheritedObjs.begin(); iter != m_lstInheritedObjs.end(); ++iter )
	{
		if( (*iter) )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
			WriteActivateTypeMsg( cMsg );
			g_pLTServer->SetObjectSFXMessage( (*iter), cMsg.Read() );
		}
	}
}

void CActivateTypeHandler::WriteActivateTypeMsg( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	pMsg->Writeuint8( m_nId );
	pMsg->Writebool( m_bDisabled );
	pMsg->Writeuint8( m_eState );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::SetDisabled
//
//  PURPOSE:	Set if this activate type is disabled or not...
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::SetDisabled( bool bDis, bool bSendToClient /* = true */, HCLIENT hClient /* = LTNULL  */ )
{
	if( m_bDisabled == bDis || m_nId == (uint8)ATMGR_INVALID_ID || !m_pBase || !m_pBase->m_hObject )
		return;

	m_bDisabled	= bDis;

	// Send a message to the clients about the change...

	if( bSendToClient )
	{
		// Send the parent object first...

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
		cMsg.WriteObject( m_pBase->m_hObject );
		cMsg.Writeuint8( ACTIVATEFX_DISABLED );
		cMsg.Writebool( m_bDisabled );
		g_pLTServer->SendToClient( cMsg.Read(), hClient, MESSAGE_GUARANTEED );

		// And then all of the inherited objects...

		ObjRefNotifierList::iterator iter;
		for( iter = m_lstInheritedObjs.begin(); iter != m_lstInheritedObjs.end(); ++iter )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_SFX_MESSAGE );
			cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
			cMsg.WriteObject( (*iter) );
			cMsg.Writeuint8( ACTIVATEFX_DISABLED );
			cMsg.Writebool( m_bDisabled );
			g_pLTServer->SendToClient( cMsg.Read(), hClient, MESSAGE_GUARANTEED );
		}
	}

	// Recreate the SpecialFX message so new clients will also get the change...

	CreateActivateTypeMsg();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::SetDisabled
//
//  PURPOSE:	Set the state for this activate type...
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::SetState( ACTIVATETYPE::State eState, bool bSendToClient /* = true */, HCLIENT hClient /* = LTNULL  */ )
{
	if( m_eState == eState || m_nId == (uint8)ATMGR_INVALID_ID || !m_pBase || !m_pBase->m_hObject )
		return;

	m_eState = eState;
	
	// Send a message to the clients about the change...

	if( bSendToClient )
	{
		// Send the parent object first...
		
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
		cMsg.WriteObject( m_pBase->m_hObject );
		cMsg.Writeuint8( ACTIVATEFX_STATE);
		cMsg.Writeuint8( m_eState );
		g_pLTServer->SendToClient( cMsg.Read(), hClient, MESSAGE_GUARANTEED );

		// And then all of the inherited objects...

		ObjRefNotifierList::iterator iter;
		for( iter = m_lstInheritedObjs.begin(); iter != m_lstInheritedObjs.end(); ++iter )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_SFX_MESSAGE );
			cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
			cMsg.WriteObject( (*iter) );
			cMsg.Writeuint8( ACTIVATEFX_STATE );
			cMsg.Writeuint8( m_eState );
			g_pLTServer->SendToClient( cMsg.Read(), hClient, MESSAGE_GUARANTEED );
		}
	}

	// Recreate the SpecialFX message so new clients will also get the change...

	CreateActivateTypeMsg();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::OnLinkBroken
//
//  PURPOSE:	An object has broken its link with us
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	// Look for the object in our inherited object list...

	ObjRefNotifierList::iterator iter;
	for( iter = m_lstInheritedObjs.begin(); iter != m_lstInheritedObjs.end(); ++iter )
	{
		if( pRef == &(*iter) )
		{

			// If we can and should reset the ActivateType message on the object then do so...

			if ( !( !m_pBase || !m_pBase->m_hObject || (m_nId == (uint8)ATMGR_INVALID_ID) || !(*iter) ) )
			{

				CAutoMessage cMsg;
				cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
				cMsg.Writeuint8( ATMGR_INVALID_ID );
				cMsg.Writebool( true );
				cMsg.Writeuint8( ACTIVATETYPE::eOff );
				g_pLTServer->SetObjectSFXMessage( (*iter), cMsg.Read() );
			}

			// Remove it 
			m_lstInheritedObjs.erase( iter );

			break;
		}
	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::InheritObject
//
//  PURPOSE:	Add an object to our inherited list...
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::InheritObject( HOBJECT hObj )
{
	if( !hObj )
		return;

	// Don't add it more than once...
	
	ObjRefNotifierList::iterator iter;
	for( iter = m_lstInheritedObjs.begin(); iter != m_lstInheritedObjs.end(); ++iter )
	{
		if( hObj == (*iter) )
		{
			// The object is already in our list
			return;
		}
	}
	
	// Add it to our list...

	LTObjRefNotifier ref( *this );
	ref = hObj;
	m_lstInheritedObjs.push_back( ref );

	// If we can and should set the ActivateType message on the new object then do so...

	if( !m_pBase || !m_pBase->m_hObject || (m_nId == (uint8)ATMGR_INVALID_ID) )
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
	WriteActivateTypeMsg( cMsg );
	g_pLTServer->SetObjectSFXMessage( hObj, cMsg.Read() );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::InheritObject
//
//  PURPOSE:	Remove an object from our inherited list...
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::DisownObject( HOBJECT hObj )
{
	if( !hObj )
		return;

	// Try and find it in our inherited list and then remove it...

	ObjRefNotifierList::iterator iter;
	for( iter = m_lstInheritedObjs.begin(); iter != m_lstInheritedObjs.end(); ++iter )
	{
		if( hObj == (*iter) )
		{
			m_lstInheritedObjs.erase( iter );

			// If we can and should reset the ActivateType message on the object then do so...

			if( !m_pBase || !m_pBase->m_hObject || (m_nId == (uint8)ATMGR_INVALID_ID) )
				return;

			CAutoMessage cMsg;
			cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
			cMsg.Writeuint8( ATMGR_INVALID_ID );
			cMsg.Writebool( true );
			cMsg.Writeuint8( ACTIVATETYPE::eOff );
			g_pLTServer->SetObjectSFXMessage( hObj, cMsg.Read() );

			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::Save
//
//  PURPOSE:	Save the data...
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	SAVE_BYTE( m_nId );
	SAVE_bool( m_bDisabled );
	SAVE_BYTE( m_eState );

	// Save our inherited objects

	SAVE_DWORD( m_lstInheritedObjs.size() );
	ObjRefNotifierList::iterator iter;
	for( iter = m_lstInheritedObjs.begin(); iter != m_lstInheritedObjs.end(); ++iter )
	{
		SAVE_HOBJECT( *iter );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::Load
//
//  PURPOSE:	Load the data...
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg || !m_pBase )
		return;

	LOAD_BYTE( m_nId );
	LOAD_bool( m_bDisabled );
	LOAD_BYTE_CAST( m_eState, ACTIVATETYPE::State );

	// Load our inherited list

	uint32 nObjs = 0;
	LOAD_DWORD( nObjs );

	m_lstInheritedObjs.clear();
	if( nObjs > 0 )
	{
		LTObjRefNotifier ref( *this );
		for( uint32 i = 0; i < nObjs; ++i )
		{
			LOAD_HOBJECT( ref );
			if( ref )
			{
				m_lstInheritedObjs.push_back( ref );
			}
		}
	}
}

#endif // _CLIENTBUILD