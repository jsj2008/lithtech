// ----------------------------------------------------------------------- //
//
// MODULE  : HUDEvidence.cpp
//
// PURPOSE : HUDItem to display evidence info
//
// CREATED : 03/29/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDEvidence.h"

HRECORD	CHUDEvidence::GetLayout()
{
	return g_pLayoutDB->GetHUDRecord("HUDEvidence");
}

