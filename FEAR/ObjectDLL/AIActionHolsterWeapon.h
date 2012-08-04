// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionHolsterWeapon.h
//
// PURPOSE : AIActionHolsterWeapon abstract class definition
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_HOLSTER_WEAPON_H__
#define __AIACTION_HOLSTER_WEAPON_H__


class CAIActionHolsterWeapon : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionHolsterWeapon, kAct_HolsterWeapon );

		CAIActionHolsterWeapon();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );

	protected:

};

// ----------------------------------------------------------------------- //

#endif
