// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerTrigger.h
//
// PURPOSE : PlayerTrigger - Definition
//
// CREATED : 3/26/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_TRIGGER_H__
#define __PLAYER_TRIGGER_H__

//
// Includes...
//

	#include "Trigger.h"

LINKTO_MODULE( PlayerTrigger )

class PlayerTrigger : public Trigger
{
	public : // Methods

		PlayerTrigger( );
		~PlayerTrigger( );
	
		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);


	protected : // Methods

		virtual LTBOOL  Activate();

	private : // Methods

		bool	InitialUpdate();
		bool	ReadProps( ObjectCreateStruct *pOCS );
};

#endif // __PLAYER_TRIGGER_H__