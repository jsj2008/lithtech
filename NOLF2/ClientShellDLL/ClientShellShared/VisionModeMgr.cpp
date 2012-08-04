// ----------------------------------------------------------------------- //
//
// MODULE  : VisionModeMgr.cpp
//
// PURPOSE : Manages the switching and updating of vision modes.
//
// CREATED : 8/13/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
#include "stdafx.h"
#include "VisionModeMgr.h"
#include "PlayerMgr.h"



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVisionModeMgr::CVisionModeMgr
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CVisionModeMgr::CVisionModeMgr( )
:	m_eVisionMode		( eVM_OFF ),
	m_eLastVisionMode	( eVM_OFF )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVisionModeMgr::~CVisionModeMgr
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CVisionModeMgr::~CVisionModeMgr( )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVisionModeMgr::NextMode
//
//  PURPOSE:	Set the vision mode to be the next one
//
// ----------------------------------------------------------------------- //

void CVisionModeMgr::NextMode( )
{
	m_eVisionMode = (eVisionMode)(m_eVisionMode + 1);

	if( m_eVisionMode >= eVM_NUMVISIONMODES )
		m_eVisionMode = eVM_OFF;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVisionModeMgr::PrevMode
//
//  PURPOSE:	Set the vision mode to be the previous one
//
// ----------------------------------------------------------------------- //

void CVisionModeMgr::PrevMode( )
{
	m_eVisionMode = (eVisionMode)(m_eVisionMode - 1);

	if( m_eVisionMode < eVM_OFF )
		m_eVisionMode = (eVisionMode)(eVM_NUMVISIONMODES - 1);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVisionModeMgr::Update
//
//  PURPOSE:	Update the current vision mode.
//
// ----------------------------------------------------------------------- //

void CVisionModeMgr::Update( )
{
	// Only Disable/Enable modes if there was a change

	if( m_eVisionMode == m_eLastVisionMode )
		return;

	// Disable the last vision mode

	switch( m_eLastVisionMode )
	{
		case eVM_OFF :
			break;

		case eVM_SPY :
			g_pPlayerMgr->EndSpyVision();
			break;
	};

	// Enable the new vision mode

	switch( m_eVisionMode )
	{
		case eVM_OFF :
			break;

		case eVM_SPY :
			g_pPlayerMgr->BeginSpyVision();
			break;

	};

	m_eLastVisionMode = m_eVisionMode;
}