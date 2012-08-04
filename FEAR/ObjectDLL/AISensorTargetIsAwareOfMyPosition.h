// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTargetIsAwareOfMyPosition.h
//
// PURPOSE : Handles an AI sensing the fact that their target knows where they are.
//
// CREATED : 5/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSORTARGETISAWAREOFMYPOSITION_H_
#define __AISENSORTARGETISAWAREOFMYPOSITION_H_

#include "AISensorAbstract.h"

class CAISensorTargetIsAwareOfMyPosition : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTargetIsAwareOfMyPosition, kSensor_TargetIsAwareOfMyPosition );

		CAISensorTargetIsAwareOfMyPosition();
		virtual ~CAISensorTargetIsAwareOfMyPosition();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// Updating.

		virtual bool	UpdateSensor();

	private:
		bool			TargetIsLookingAtMe();

		double			m_flLastUpdateTime;
};

#endif // __AISENSORTARGETISAWAREOFMYPOSITION_H_
