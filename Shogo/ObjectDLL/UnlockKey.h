// ----------------------------------------------------------------------- //
//
// MODULE  : UnlockKey.h
//
// PURPOSE : Key definition for keys that unlock objects
//
// CREATED : 10/1/97
//
// ----------------------------------------------------------------------- //

#ifndef __UNLOCKKEY_H__
#define __UNLOCKKEY_H__

#include "Powerup.h"

class UnlockKey : public Powerup
{
	public :

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		virtual void AddPowerup(HOBJECT hObject);
};

#endif // __UNLOCKKEY_H__
