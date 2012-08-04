// ----------------------------------------------------------------------- //
//
// MODULE  : TO2LayoutMgr.cpp
//
// PURPOSE : Attribute file manager for interface layout info
//			 TO2-specific functionality
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2LayoutMgr.h"

CLayoutMgr* g_pLayoutMgr = LTNULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CTO2LayoutMgr::CTO2LayoutMgr() : CLayoutMgr()
{
	g_pLayoutMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronLayoutMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CTO2LayoutMgr::Init(const char* szAttributeFile)
{
	if (!CLayoutMgr::Init(szAttributeFile))
		return LTFALSE;

	return LTTRUE;
}


