// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLink.cpp
//
// PURPOSE : AI NavMesh Link class implementation.
//
// CREATED : 01/08/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLink.h"


// WorldEdit

LINKFROM_MODULE( AINavMeshLink );

BEGIN_CLASS( AINavMeshLink )
	ADD_STRINGPROP_FLAG(SmartObject,	"None",				0|PF_HIDDEN, "SmartObject used to specify animations for traversing the link")
	ADD_BOOLPROP_FLAG(Active,			true,				0|PF_HIDDEN, "If true the AINavMeshLink will be Active.")
	ADD_STRINGPROP_FLAG(MinActiveAwareness,		"Relaxed",	0|PF_STATICLIST|PF_HIDDEN,	"Minimum awareness required to treat this as an active link.")
	ADD_STRINGPROP_FLAG(MaxActiveAwareness,		"Alert",	0|PF_STATICLIST|PF_HIDDEN,	"Maximum awareness required to treat this as an active link.")
END_CLASS_FLAGS( AINavMeshLink, AINavMeshLinkAbstract, 0, "This object attaches special information to a AINavMesh brush" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLink )
CMDMGR_END_REGISTER_CLASS( AINavMeshLink, AINavMeshLinkAbstract )


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLink::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLink::AINavMeshLink()
{
}




