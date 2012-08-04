// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkFlamePot.cpp
//
// PURPOSE : 
//
// CREATED : 4/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkFlamePot.h"

LINKFROM_MODULE(AINavMeshLinkFlamePot);

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINavMeshLinkFlamePot 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINavMeshLinkFlamePot CF_HIDDEN

#endif

BEGIN_CLASS( AINavMeshLinkFlamePot )
	// Hide properties not supported by this nav mesh link.
	ADD_STRINGPROP_FLAG(SmartObject,	"None",				0|PF_HIDDEN, "SmartObject used to specify animations for traversing the link")
	ADD_STRINGPROP_FLAG(MinActiveAwareness,		"Relaxed",	0|PF_STATICLIST|PF_HIDDEN,	"Minimum awareness required to treat this as an active link.")
	ADD_STRINGPROP_FLAG(MaxActiveAwareness,		"Alert",	0|PF_STATICLIST|PF_HIDDEN,	"Maximum awareness required to treat this as an active link.")
END_CLASS_FLAGS( AINavMeshLinkFlamePot, AINavMeshLinkAbstract, CF_HIDDEN_AINavMeshLinkFlamePot, "TODO:CLASSDESC" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkFlamePot )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkFlamePot, AINavMeshLinkAbstract )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINavMeshLinkFlamePot::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

AINavMeshLinkFlamePot::AINavMeshLinkFlamePot()
{
}

AINavMeshLinkFlamePot::~AINavMeshLinkFlamePot()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkFlamePot::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINavMeshLinkFlamePot
//              
//----------------------------------------------------------------------------

void AINavMeshLinkFlamePot::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void AINavMeshLinkFlamePot::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkFlamePot::GetNMLinkPathingWeight
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

float AINavMeshLinkFlamePot::GetNMLinkPathingWeight(CAI* pAI)
{
	// Make pathing through this link very expensive.  The current value is 
	// sufficient to keep AIs out of FlamePots in test situations, but may 
	// need to be reevaluated/elevated/moved into database/specified per 
	// instance once we know more about how this object will work in practice.
	return 1000.f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkFlamePot::AllowStraightPaths
//              
//	PURPOSE:	AI is allowed to plot straight paths through this link.  This 
//				enables straight line pathing tests.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkFlamePot::AllowStraightPaths()
{
	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkFlamePot::AllowDynamicMovement
//              
//	PURPOSE:	AI can always use dynamic movement in this link.  If dynamic
//				movement isn't allowed, an AI will walk in place if pinned 
//				against the link. This occurs because the AI starts and stops
//				navigation every frame.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkFlamePot::AllowDynamicMovement( CAI* pAI )
{
	return true;
}
