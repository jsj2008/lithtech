// ----------------------------------------------------------------------- //
//
// MODULE  : AINodePickupWeapon.h
//
// PURPOSE : 
//
// CREATED : 7/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODEPICKUPWEAPON_H_
#define _AINODEPICKUPWEAPON_H_

LINKTO_MODULE(AINodePickupWeapon);

#include "AINode.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodePickupWeapon
//
//	PURPOSE:	This node represents a location an AI may move to when it 
//				desires to pick up a weapon.
//
// ----------------------------------------------------------------------- //

class AINodePickupWeapon : public AINodeSmartObject
{
	typedef AINodeSmartObject super;
	
public:

	// Ctor/Dtor

	AINodePickupWeapon();
	virtual ~AINodePickupWeapon();

	// Save/Load

	virtual void			Load(ILTMessage_Read *pMsg);
	virtual void			Save(ILTMessage_Write *pMsg);

	// AINode 

	virtual EnumAINodeType	GetType() const { return kNode_PickupWeapon; }
	virtual void			InitNode();
	virtual void			ReadProp(const GenericPropList *pProps);
	virtual bool			IsNodeValid( CAI* /*pAI*/, const LTVector& /*vPosAI*/, HOBJECT /*hThreat*/, EnumAIThreatPosition eThreatPos, uint32 /*dwStatusFlags*/ );
	virtual float			GetBoundaryRadiusSqr() const { return FLT_MAX; }
	virtual HOBJECT			GetWeaponItem() const;

private:
	PREVENT_OBJECT_COPYING(AINodePickupWeapon);
};


class AINodePickupWeaponPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodePickupWeaponPlugin()
	{
		AddValidNodeType(kNode_PickupWeapon);
	}
};

#endif // _AINODEPICKUPWEAPON_H_
