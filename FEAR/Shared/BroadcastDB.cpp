// ----------------------------------------------------------------------- //
//
// MODULE  : BroadcastDB.cpp
//
// PURPOSE : Database interface for player broadcast messages
//
// CREATED : 12/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "BroadcastDB.h"
#include "StringUtilities.h"


uint32 _DatabaseCategory_Broadcast::GetRandomLineID(HRECORD hRec) const
{
	if (!hRec) return INVALID_STRINGEDIT_INDEX;

	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(hRec,"Line");
	if (!hAtt) return INVALID_STRINGEDIT_INDEX;

	uint32 nLines = g_pLTDatabase->GetNumValues(hAtt);
	if (!nLines) return INVALID_STRINGEDIT_INDEX;

	return IndexFromStringID(g_pLTDatabase->GetString(hAtt,GetRandom(0,nLines-1),NULL));
}

eNavMarkerPlacement _DatabaseCategory_Broadcast::GetPlacement(HRECORD hRec) const
{
	if (!hRec) return kNavMarkerLocation;

	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(hRec,"NavMarkerPlacement");
	if (!hAtt) return kNavMarkerLocation;

	const char* pVal = g_pLTDatabase->GetString(hAtt,0,"");

	if (LTStrIEquals(pVal,"Attached"))
	{
		return kNavMarkerAttached;
	}
	if (LTStrIEquals(pVal,"Projected"))
	{
		return kNavMarkerProjected;
	}

	return kNavMarkerLocation;

}

