// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.h
//
// PURPOSE : Item that any player can walk across and potentially pick up
//
// CREATED : 10/1/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PICKUPITEM_H__
#define __PICKUPITEM_H__

#include "GameBase.h"
#include "ClientServerShared.h"

class PickupItem : public GameBase
{
	public :

		PickupItem();
		~PickupItem();

	protected :

        uint32          EngineMessageFn (uint32 messageID, void *pData, LTFLOAT lData);
        uint32          ObjectMessageFn (HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        LTBOOL          Setup(ObjectCreateStruct *pData);
		virtual void	PickedUp(HMESSAGEREAD hRead);

		// dervied classes will override this function to send a specific
		// message to the object that touched us

		virtual void	ObjectTouch (HOBJECT hObject) {}

		virtual void	PlayPickedupSound();

	protected:

		HOBJECT		m_hPlayerObj;			// Player object that touched us

        LTFLOAT		m_fRespawnDelay;            // How quickly we respawn
        LTBOOL		m_bRotate;                  // Do we bounce
        LTBOOL		m_bBounce;                  // Do we rotate
        LTBOOL		m_bMoveToFloor;				// Move item to floor?
		uint32		m_dwFlags;                  // Copy of flags

        uint32		m_dwUserFlags;              // User flags (glowing?)

		HSTRING		m_hstrPickupTriggerTarget;	// Object we trigger
		HSTRING		m_hstrPickupTriggerMessage;	// Trigger message
		HSTRING		m_hstrSoundFile;			// Sound to play when picked up
		HSTRING		m_hstrRespawnSoundFile;		// Sound to play when item respawns

		uint8		m_nPlayerTeamFilter;

		LTVector	m_vScale;

		void SetPlayerObj(HOBJECT hObj);

		// NOTE:  m_bFirstUpdate  was added in update 1.002 so that pickup 
		// items wouldn't fall through WorldModels when MoveToFloor is 
		// called.  The current implementation works without saving the 
		// value out so that save games from version 1.001 would still work.  
		// In the future this value should be loaded/saved.  
		LTBOOL		m_bFirstUpdate;

	private :

        LTBOOL ReadProp(ObjectCreateStruct *pData);
		void  PostPropRead(ObjectCreateStruct* pData);
        LTBOOL InitialUpdate();
        LTBOOL Update();

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void CacheFiles();
};

#endif // __PICKUPITEM_H__