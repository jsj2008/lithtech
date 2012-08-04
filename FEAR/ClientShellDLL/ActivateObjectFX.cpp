// ----------------------------------------------------------------------- //
//
// MODULE  : ActivateObjectFX.h
//
// PURPOSE : ActivateObject special fx class - Definition
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
	#include "ActivateObjectFX.h"


CActivateObjectHandler::ActivateObjList CActivateObjectHandler::m_lstActivateObjs;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateObjectHandler::CActivateObjectHandler
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CActivateObjectHandler::CActivateObjectHandler()
: m_hObject		( NULL )
, m_nId			( ACTIVATETYPE::INVALID_ID )
, m_bDisabled	( false )
, m_eState		( ACTIVATETYPE::eOn )
{
	m_hObject.SetReceiver( *this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateObjectHandler::~CActivateObjectHandler
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CActivateObjectHandler::~CActivateObjectHandler()
{
	// Remove this handler from the global list...
	
	ActivateObjList::iterator iter = m_lstActivateObjs.begin();
	while( iter != m_lstActivateObjs.end() )
	{
		if( *iter == this )
		{
			m_lstActivateObjs.erase( iter );
			break;
		}

		++iter;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateObjectHandler::Init
//
//	PURPOSE:	Set up the handler for the activate object...
//
// ----------------------------------------------------------------------- //

bool CActivateObjectHandler::Init( HOBJECT hObject )
{
	if( !hObject || m_hObject )
		return false;

	m_hObject = hObject;

	// Add this handler to the global list...
	
	m_lstActivateObjs.push_back( this );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateObjectHandler::FindActivateObject
//
//	PURPOSE:	Return the ActivateObjectHandler that is handling
//				activation for the provided object.
//
// ----------------------------------------------------------------------- //

const CActivateObjectHandler* CActivateObjectHandler::FindActivateObject( HOBJECT hHandledObject )
{
	for( ActivateObjList::const_iterator iter = m_lstActivateObjs.begin(); iter != m_lstActivateObjs.end(); iter++ )
		if( (*iter)->m_hObject == hHandledObject )
			return(*iter);

	return NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateObjectHandler::OnLinkBroken
//
//	PURPOSE:	Handle when link to HOBJECT goes away.
//
// ----------------------------------------------------------------------- //
void CActivateObjectHandler::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	m_nId = ACTIVATETYPE::INVALID_ID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateObjectFX::CActivateObjectFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CActivateObjectFX::CActivateObjectFX()
:	CSpecialFX		()
{
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateObjectFX::~CActivateObjectFX
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CActivateObjectFX::~CActivateObjectFX()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateObjectFX::Init
//
//	PURPOSE:	Init the ActivateObject fx
//
// ----------------------------------------------------------------------- //

bool CActivateObjectFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !hServObj ) return false;
	if( !CSpecialFX::Init( hServObj, pMsg )) return false;

	// Init our handler...
	
	if( !m_ActivateObjectHandler.Init( hServObj ))
		return false;

	// Read the data into our handler...

	m_ActivateObjectHandler.m_nId			= pMsg->Readuint8();
	m_ActivateObjectHandler.m_bDisabled		= pMsg->Readbool();
	m_ActivateObjectHandler.m_eState		= (ACTIVATETYPE::State)pMsg->Readuint8();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateObjectFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

bool CActivateObjectFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg )) return false;
	
	uint8 nMsgId = pMsg->Readuint8();

	switch( nMsgId )
	{
		case ACTIVATEFX_DISABLED :
		{
			m_ActivateObjectHandler.m_bDisabled = pMsg->Readbool();
		}
		break;

		case ACTIVATEFX_STATE :
		{
			m_ActivateObjectHandler.m_eState = (ACTIVATETYPE::State)pMsg->Readuint8();
		}
		break;

		default:
			return false;
	}
	 
	return true;
}
