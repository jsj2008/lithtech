// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.h
//
// PURPOSE : Item that any player can walk across and potentially pick up
//
// CREATED : 10/1/97
//
// ----------------------------------------------------------------------- //

#ifndef __PICKUPITEM_H__
#define __PICKUPITEM_H__

#include "cpp_engineobjects_de.h"
#include "ClientServerShared.h"

class PickupItem : public BaseClass
{
	public :

		PickupItem();
		~PickupItem();

	protected :

		DDWORD			EngineMessageFn (DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD			ObjectMessageFn (HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		DBOOL			ReadProp (ObjectCreateStruct *pData);
		DBOOL			Setup( ObjectCreateStruct *pData );
		void			PostPropRead (ObjectCreateStruct* pData);
		DBOOL			InitialUpdate (DVector *pMovement);
		DBOOL			Update (DVector* pMovement);
		virtual void	PickedUp (HMESSAGEREAD hRead);
		
		// base classes will override this function	to send a specific
		// message to the object that touched us

		virtual void	ObjectTouch (HOBJECT hObject) {}
	
	protected:

		HCLIENT			m_hClient;		  // Client of player that touched us
		PickupItemType	m_eType;		  // Type of pickup item
		DBOOL			m_bInformClient;  // Tell client when picked up

		DFLOAT	m_fRespawnDelay;	// How quickly we respawn
		DBOOL	m_bRotate;			// Do we bounce
		DBOOL	m_bBounce;			// Do we rotate
		DFLOAT	m_fYaw;				// Angular position around Y axis
		DFLOAT	m_fLastTime;		// Last update time
		DDWORD	m_dwFlags;			// Copy of flags
		DFLOAT	m_fBounce;			// Bounce acceleration
		DBOOL	m_bBouncing;		// Bounce indicator
		DVector m_vRestPos;			// Position when not bouncing

		DDWORD	m_dwUserFlags;		// User flags (glowing?)

		HSTRING m_hstrPickupTriggerTarget;	// Object we trigger
		HSTRING m_hstrPickupTriggerMessage;	// Trigger message
		HSTRING m_hstrSoundFile;			// Sound to play when picked up
		HSTRING m_hstrRespawnSoundFile;		// Sound to play when item respawns

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();
};

#endif // __PICKUPITEM_H__
