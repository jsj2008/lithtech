// ----------------------------------------------------------------------- //
//
// MODULE  : Powerup.h
//
// PURPOSE : Powerup definition
//
// CREATED : 10/1/97
//
// ----------------------------------------------------------------------- //

#ifndef __POWERUP_H__
#define __POWERUP_H__

#include "cpp_engineobjects_de.h"
#include "PickupItem.h"
#include "ClientServerShared.h"

struct TimedPowerup
{
	TimedPowerup (DFLOAT fExpires=0.0f, PickupItemType type=PIT_UNKNOWN)	
	{ 
		fExpirationTime = fExpires; 
		eType = type; 
	}

	DFLOAT				fExpirationTime;
	PickupItemType		eType;

	void Save(HMESSAGEWRITE hWrite)
	{
		if (!g_pServerDE || !hWrite) return;

		g_pServerDE->WriteToMessageFloat(hWrite, fExpirationTime);
		g_pServerDE->WriteToMessageByte(hWrite, eType);
	}

	void Load(HMESSAGEREAD hRead)
	{
		if (!g_pServerDE || !hRead) return;

		fExpirationTime = g_pServerDE->ReadFromMessageFloat(hRead);
		eType = (PickupItemType)g_pServerDE->ReadFromMessageByte(hRead);
	}
};

class Powerup : public PickupItem
{
	public :

		Powerup();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		virtual void ObjectTouch(HOBJECT hObject);
	
		DBOOL		m_bTimed;		// Does this powerup have a time limit?
		DFLOAT		m_fTimeLimit;	// Time limit, if applicable

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwFlags);
};

#endif // __POWERUP_H__
