// ----------------------------------------------------------------------- //
//
// MODULE  : NoPlayerTrigger.h
//
// PURPOSE : NoPlayerTrigger - Definition
//
// CREATED : 4/5/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __NO_PLAYER_TRIGGER_H__
#define __NO_PLAYER_TRIGGER_H__

//
// Includes...
//

	#include "Trigger.h"

LINKTO_MODULE( NoPlayerTrigger )

class NoPlayerTrigger : public Trigger
{
	public : // Methods

		NoPlayerTrigger( );
		~NoPlayerTrigger( );


	protected : // Methods

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual LTBOOL  Activate();

	private: // Methods

		LTBOOL	Update();
};

#endif // __NO_PLAYER_TRIGGER_H__