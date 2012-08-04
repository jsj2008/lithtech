//-------------------------------------------------------------------------
//
// MODULE  : AdditiveCtrl.cpp
//
// PURPOSE : GUI element for interacting with additives
//
// CREATED : 4/4/02 - for TRON
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#include "stdafx.h"
#include "AdditiveCtrl.h"

//////////////////////////////////////////////////////////////////////
// CAdditiveCtrl Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAdditiveCtrl::CAdditiveCtrl()
{
	m_pHotAdditive = LTNULL;
}

CAdditiveCtrl::~CAdditiveCtrl()
{
	Term();
}


void CAdditiveCtrl::Term()
{
}

LTBOOL CAdditiveCtrl::OnMouseMove(int mx, int my)
{
	return LTTRUE;
}
