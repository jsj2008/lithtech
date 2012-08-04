// ----------------------------------------------------------------------- //
//
// MODULE  : ServerTrackedNodeMgr.cpp
//
// PURPOSE : ServerTrackedNodeMgr implementation
//
// CREATED : 3/6/02
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerTrackedNodeMgr.h"

// Global pointer to server tracked node mgr...

CServerTrackedNodeMgr*  g_pServerTrackedNodeMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerTrackedNodeMgr::CServerTrackedNodeMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CServerTrackedNodeMgr::CServerTrackedNodeMgr() : CTrackedNodeMgr( g_pLTServer )
{
	ASSERT( !g_pServerTrackedNodeMgr && "CServerTrackedNodeMgr: Singleton already exists." );
	g_pServerTrackedNodeMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerTrackedNodeMgr::~CServerTrackedNodeMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerTrackedNodeMgr::~CServerTrackedNodeMgr()
{
	g_pServerTrackedNodeMgr = LTNULL;
}

