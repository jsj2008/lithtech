// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMountedFlashlight.h
//
// PURPOSE : AIGoalMountedFlashlight class definition
//
// CREATED : 06/24/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_MOUNTED_FLASHLIGHT_H__
#define __AIGOAL_MOUNTED_FLASHLIGHT_H__

#include "AIGoalAbstract.h"


class CAIGoalMountedFlashlight : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalMountedFlashlight, kGoal_MountedFlashlight);

		// Con/Destructor.

		 CAIGoalMountedFlashlight();

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Updating.

		virtual void RecalcImportance();

	protected:

		LTBOOL HasRangedWeapon();

	protected:

		LTBOOL		m_bLightOn;
		LTBOOL		m_bCheckedForRangedWeapon;
		LTBOOL		m_bHasRangedWeapon;
};


#endif
