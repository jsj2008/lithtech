// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorPlayerTooClose.h
//
// PURPOSE : AISensorPlayerTooClose class definition
//
// CREATED : 10/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_PLAYER_TOO_CLOSE_H__
#define __AISENSOR_PLAYER_TOO_CLOSE_H__

#include "AISensorAbstract.h"


// Forward declarations.


class CAISensorPlayerTooClose : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorPlayerTooClose, kSensor_PlayerTooClose );

		CAISensorPlayerTooClose();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	protected:

		bool			IsCharacterTooClose( CCharacter* pChar, float fThreshold );
		void			CreateAvoidanceDesire( CCharacter* pChar );
};

#endif
