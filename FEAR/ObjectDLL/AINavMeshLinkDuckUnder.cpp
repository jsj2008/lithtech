// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkDuckUnder.cpp
//
// PURPOSE : AI NavMesh Link DuckUnder class implementation.
//
// CREATED : 11/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkDuckUnder.h"


// WorldEdit

LINKFROM_MODULE( AINavMeshLinkDuckUnder );

BEGIN_CLASS( AINavMeshLinkDuckUnder )
	ADD_STRINGPROP_FLAG(SmartObject,	"DuckUnderHigh",			0|PF_STATICLIST, "SmartObject used to specify animations for traversing the link")
END_CLASS_FLAGS_PLUGIN( AINavMeshLinkDuckUnder, AINavMeshLinkAbstractOneAnim, 0, AINavMeshLinkDuckUnderPlugin, "This link is used to specify that the brush contains a space the AI must duck under" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkDuckUnder )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkDuckUnder, AINavMeshLinkAbstractOneAnim )

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDuckUnder::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkDuckUnder::AINavMeshLinkDuckUnder()
{
}

