// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorAwarenessMod.h
//
// PURPOSE : AISensorAwarenessMod class definition
//
// CREATED : 08/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_AWARENESS_MOD_H__
#define __AISENSOR_AWARENESS_MOD_H__

#include "AISensorAbstract.h"


// Forward declarations.


class CAISensorAwarenessMod : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorAwarenessMod, kSensor_AwarenessMod );

		CAISensorAwarenessMod();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	private:

		bool			IsAIInjured();
		bool			CanAILimp();
		void			UpdateLimpTime();

	private:

		bool			m_bTestedLimp;
		bool			m_bCanLimp;
		float			m_fFullHealth;
};

#endif
