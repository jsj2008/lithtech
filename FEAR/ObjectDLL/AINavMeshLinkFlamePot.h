// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkFlamePot.h
//
// PURPOSE : 
//
// CREATED : 4/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINAVMESHLINKFLAMEPOT_H_
#define _AINAVMESHLINKFLAMEPOT_H_

LINKTO_MODULE(AINavMeshLinkFlamePot);

#include "AINavMeshLinkAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINavMeshLinkFlamePot
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINavMeshLinkFlamePot : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;
	
public:

	// Ctor/Dtor

	AINavMeshLinkFlamePot();
	virtual ~AINavMeshLinkFlamePot();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	virtual float GetNMLinkPathingWeight(CAI* pAI);
	virtual EnumAINavMeshLinkType GetNMLinkType() const { return kLink_FlamePot; }
	virtual bool AllowStraightPaths();	
	virtual bool AllowDynamicMovement( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(AINavMeshLinkFlamePot);
};

#endif // _AINAVMESHLINKFLAMEPOT_H_
