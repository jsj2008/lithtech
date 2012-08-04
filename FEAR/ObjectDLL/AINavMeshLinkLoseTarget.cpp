// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkLoseTarget.cpp
//
// PURPOSE : AI NavMesh Link LoseTarget class implementation.
//
// CREATED : 08/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkLoseTarget.h"


// WorldEdit

LINKFROM_MODULE( AINavMeshLinkLoseTarget );

// Hide this object in Dark.
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINAVMESHLINKLOSETARGET CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINAVMESHLINKLOSETARGET 0

#endif

BEGIN_CLASS( AINavMeshLinkLoseTarget )
	ADD_STRINGPROP_FLAG(SmartObject,		"None",		0|PF_HIDDEN, "SmartObject used to specify animations for traversing the link")
	ADD_BOOLPROP_FLAG(Active,				true,		0|PF_HIDDEN, "If true the AINavMeshLink will be Active.")
	ADD_STRINGPROP_FLAG(MinActiveAwareness,	"Relaxed",	0|PF_STATICLIST|PF_HIDDEN,	"Minimum awareness required to treat this as an active link.")
	ADD_STRINGPROP_FLAG(MaxActiveAwareness,	"Alert",	0|PF_STATICLIST|PF_HIDDEN,	"Maximum awareness required to treat this as an active link.")
END_CLASS_FLAGS( AINavMeshLinkLoseTarget, AINavMeshLinkAbstract, CF_HIDDEN_AINAVMESHLINKLOSETARGET, "If a target goes out of sight while a LoseTarget link falls between the AI and his target, the AI will consider his target lost." )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkLoseTarget )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkLoseTarget, AINavMeshLinkAbstract )


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkLoseTarget::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkLoseTarget::AINavMeshLinkLoseTarget()
{
}




