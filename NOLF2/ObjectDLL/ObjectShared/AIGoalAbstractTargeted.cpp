// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstractTargeted.cpp
//
// PURPOSE : AIGoalAbstractTargeted abstract class implementation
//
// CREATED : 8/16/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAbstractTargeted.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractTargeted::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractTargeted::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hTarget);
}

void CAIGoalAbstractTargeted::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hTarget);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractTargeted::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAbstractTargeted::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "TARGET") )
	{
		HOBJECT hObj = m_hTarget;
		FindNamedObject(szValue, hObj);
		m_hTarget = hObj;
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractStimulated::OnLinkBroken
//
//	PURPOSE:	Handles a deleted object reference.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractTargeted::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( &m_hTarget == pRef )
		SetCurImportance( 0.0f );
}

