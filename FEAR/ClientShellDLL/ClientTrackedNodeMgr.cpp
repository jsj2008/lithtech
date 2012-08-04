// ----------------------------------------------------------------------- //
//
// MODULE  : ClientTrackedNodeMgr.cpp
//
// PURPOSE : ClientTrackedNodeMgr implementation
//
// CREATED : 3/6/02
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientTrackedNodeMgr.h"

// Global pointer to client tracked node mgr...

CClientTrackedNodeMgr*  g_pClientTrackedNodeMgr = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientTrackedNodeMgr::CClientTrackedNodeMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CClientTrackedNodeMgr::CClientTrackedNodeMgr() : CTrackedNodeMgr( g_pLTClient )
{
	ASSERT( !g_pClientTrackedNodeMgr && "CClientTrackedNodeMgr: Singleton already exists." );
	g_pClientTrackedNodeMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientTrackedNodeMgr::~CClientTrackedNodeMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CClientTrackedNodeMgr::~CClientTrackedNodeMgr()
{
	g_pClientTrackedNodeMgr = NULL;
}

