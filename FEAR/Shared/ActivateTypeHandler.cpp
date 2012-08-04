// ----------------------------------------------------------------------- //
//
// MODULE  : ActivateTypeHandler.cpp
//
// PURPOSE : ActivateTypeHandler implementation
//
// CREATED : 7/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "ActivateTypeHandler.h"
#include "SFXMsgIds.h"
#include "MsgIDs.h"

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
: m_hParent		( NULL )
, m_nId			( ACTIVATETYPE::INVALID_ID )
, m_bDisabled	( false )
, m_eState		( ACTIVATETYPE::eOn )
{
	m_eExecutionShell = GetCurExecutionShellContext();
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
	Term();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::Init
//
//  PURPOSE:	CActivateTypeHandlers are not valid until their type is set.
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::Init( HOBJECT hObject, const char *pInitActivateTypeStr )
{
	SetParent( hObject );
	if( pInitActivateTypeStr )
		SetActivateType( pInitActivateTypeStr );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::Term
//
//  PURPOSE:	Shutdown and cleanup the object 
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::Term()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::SetParent
//
//  PURPOSE:	Set the parent so we know what object we are 
//				doing the handling for.
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::SetParent( HOBJECT hObject )
{
	LTASSERT(hObject, "CActivateTypeHandler::SetParent - invalid parent object" );
	m_hParent = hObject;
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
	LTASSERT(pName, "CActivateTypeHandler::SetActivateType - bad activation type");

	// Ignore selections of none...
	if( LTStrEmpty( pName ) || LTStrIEquals( pName, "<none>" ))
		return;

	// Make sure this is a valid type...
	
	HRECORD hRecord = DATABASE_CATEGORY( Activate ).GetRecord( DATABASE_CATEGORY( Activate ).GetCategory(), pName);
	if( hRecord )
	{
		m_nId = DATABASE_CATEGORY( Activate ).GetRecordIndex( hRecord );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::IsValid
//
//  PURPOSE:	Check to see if the object has been properly initialized
//				and has a valid activation type
//
// ----------------------------------------------------------------------- //

bool CActivateTypeHandler::IsValid( bool bVerifyTypeInDB )
{
	return( m_hParent &&
			(m_nId != ACTIVATETYPE::INVALID_ID) &&
			(!bVerifyTypeInDB || DATABASE_CATEGORY( Activate ).GetRecordByIndex( m_nId ) != NULL) );
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
	if( !IsValid() )
		return;

	// Send the parent object first...
	
	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
	WriteActivateTypeMsg( cMsg );
	g_pLTServer->SetObjectSFXMessage( m_hParent, cMsg.Read() );

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

	pMsg->Writeint8( m_nId );
	pMsg->Writebool( m_bDisabled );
	pMsg->Writeint8( m_eState );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::SetDisabled
//
//  PURPOSE:	Set if this activate type is disabled or not...
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::SetDisabled( bool bDis, bool bSendToClient, HCLIENT hClient )
{
	if( !IsValid() )
		return;

	//!! This might mask proper initialization if the passed in disabled value equals the default.
	// Change the bool to a enum?  Establish disabled state in the initialization?
	if( m_bDisabled == bDis )
		return;
	
	m_bDisabled	= bDis;

	// Send a message to the clients about the change...

	if( bSendToClient )
	{
		// Send the parent object first...

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
		cMsg.WriteObject( m_hParent );
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

void CActivateTypeHandler::SetState( ACTIVATETYPE::State eState, bool bSendToClient, HCLIENT hClient )
{
	LTASSERT( m_hParent, "CActivateTypeHandler::SetState - no parent object" );
	
	if( m_eState == eState )
		return;

	m_eState = eState;
	
	// Send a message to the clients about the change...

	if( bSendToClient )
	{
		// Send the parent object first...
		
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
		cMsg.WriteObject( m_hParent );
		cMsg.Writeuint8( ACTIVATEFX_STATE);
		cMsg.Writeint8( m_eState );
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
			cMsg.Writeint8( m_eState );
			g_pLTServer->SendToClient( cMsg.Read(), hClient, MESSAGE_GUARANTEED );
		}
	}

	// Recreate the SpecialFX message so new clients will also get the change...

	CreateActivateTypeMsg();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CActivateTypeHandler::SetCanActivate
//
//  PURPOSE:	Set the canactivate flag for this activate type...
//
// ----------------------------------------------------------------------- //

void CActivateTypeHandler::SetCanActivate( bool bCanActivate )
{
	// Set the parent object first...
	g_pCommonLT->SetObjectFlags( m_hParent, OFT_User, (bCanActivate ? USRFLG_CAN_ACTIVATE : 0), USRFLG_CAN_ACTIVATE );


	// And then all of the inherited objects...
	ObjRefNotifierList::iterator iter;
	for( iter = m_lstInheritedObjs.begin(); iter != m_lstInheritedObjs.end(); ++iter )
	{
		g_pCommonLT->SetObjectFlags( (*iter), OFT_User, (bCanActivate ? USRFLG_CAN_ACTIVATE : 0), USRFLG_CAN_ACTIVATE );
	}
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
	// If our object goes invalid, then terminate, since
	// we can't assume any of our responses are still valid.
	// As this is an engine callback, we need to make sure we activate 
	// the correct shell.  If we don't, we will crash if the Term 
	// function uses engine interfaces.

	CLIENT_CODE
	(
		CGameClientShell::CClientShellScopeTracker  cScopeTracker;
	)

	SERVER_CODE
	(
		CGameServerShell::CServerShellScopeTracker  cScopeTracker;
	)

	Term( );

#if 0
	// nothing to do
	if( !pRef )
		return;

	// Look for the object in our inherited object list...

	ObjRefNotifierList::iterator iter;
	for( iter = m_lstInheritedObjs.begin(); iter != m_lstInheritedObjs.end(); ++iter )
	{
		if( pRef == &(*iter) )
		{
			// If we can and should reset the ActivateType message on the object then do so...

			CAutoMessage cMsg;
			cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
			cMsg.Writeuint8( ACTIVATETYPE::INVALID_ID );
			cMsg.Writebool( true );
			cMsg.Writeuint8( ACTIVATETYPE::eOff );
			g_pLTServer->SetObjectSFXMessage( (*iter), cMsg.Read() );

			// Remove it 
			m_lstInheritedObjs.erase( iter );

			break;
		}
	}
#endif
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
			// If we can and should reset the ActivateType message on the object then do so...
			CAutoMessage cMsg;
			cMsg.Writeuint8( SFX_ACTIVATEOBJECT_ID );
			cMsg.Writeuint8( ACTIVATETYPE::INVALID_ID );
			cMsg.Writebool( true );
			cMsg.Writeuint8( ACTIVATETYPE::eOff );
			g_pLTServer->SetObjectSFXMessage( hObj, cMsg.Read() );

			m_lstInheritedObjs.erase( iter );

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

	SAVE_HOBJECT( m_hParent );
	SAVE_BYTE( m_nId );
	SAVE_bool( m_bDisabled );
	SAVE_BYTE( m_eState );

	SAVE_HOBJECT( m_hParent );

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
	if( !pMsg )
		return;

	LOAD_HOBJECT( m_hParent );
	LOAD_BYTE( m_nId );
	LOAD_bool( m_bDisabled );
	LOAD_BYTE_CAST( m_eState, ACTIVATETYPE::State );

	LOAD_HOBJECT( m_hParent );

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
