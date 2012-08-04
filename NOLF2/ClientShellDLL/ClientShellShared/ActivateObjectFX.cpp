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
:	m_hObject	( LTNULL ),
	m_nId		( ATMGR_INVALID_ID ),
	m_bDisabled	( false ),
	m_eState	( ACTIVATETYPE::eOn )
{

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

LTBOOL CActivateObjectFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !hServObj ) return LTFALSE;
	if( !CSpecialFX::Init( hServObj, pMsg )) return LTFALSE;

	// Init our handler...
	
	if( !m_ActivateObjectHandler.Init( hServObj ))
		return LTFALSE;

	// Read the data into our handler...

	m_ActivateObjectHandler.m_nId			= pMsg->Readuint8();
	m_ActivateObjectHandler.m_bDisabled		= pMsg->Readbool();
	m_ActivateObjectHandler.m_eState		= (ACTIVATETYPE::State)pMsg->Readuint8();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActivateObjectFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CActivateObjectFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg )) return LTFALSE;
	
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
			return LTFALSE;
	}
	 
	return LTTRUE;
}