// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectWeaponItem.h
//
// PURPOSE : 
//
// CREATED : 7/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AITARGETSELECTWEAPONITEM_H_
#define _AITARGETSELECTWEAPONITEM_H_

LINKTO_MODULE(AITargetSelectWeaponItem);

#include "AITargetSelectAbstract.h"

class CAI;
class CAIWMFact;

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAITargetSelectWeaponItem
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAITargetSelectWeaponItem : public CAITargetSelectAbstract
{
	typedef CAITargetSelectAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectWeaponItem, kTargetSelect_WeaponItem );

	// Ctor/Dtor

	CAITargetSelectWeaponItem();
	virtual ~CAITargetSelectWeaponItem();

	// CAIActionAbstract members.

	virtual bool	ValidatePreconditions( CAI* pAI );
	virtual void	Activate( CAI* pAI );
	virtual bool	Validate( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAITargetSelectWeaponItem);
};

#endif // _AITARGETSELECTWEAPONITEM_H_
