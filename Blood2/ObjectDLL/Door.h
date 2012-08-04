#ifndef __DOOR_H__
#define __DOOR_H__


#include "basedefs_de.h"

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "B2BaseClass.h"


// Defines....

#define DOORSTATE_CLOSED		1
#define DOORSTATE_CLOSING		2
#define DOORSTATE_OPEN			3
#define DOORSTATE_OPENING		4

// Door trigger flags..

#define DTF_SELFTRIGGER		0x001
#define DTF_TRIGGERCLOSED	0x002
#define DTF_STARTOPEN		0x004
#define DTF_REMAINSOPEN		0x008



enum DoorWaveTypes {
	DOORWAVE_LINEAR,
	DOORWAVE_SINE,
	DOORWAVE_SLOWOFF,
	DOORWAVE_SLOWON
};


class Trigger;

// Door structure
class Door : public B2BaseClass
{
	public:

		Door();
		virtual ~Door();
		DBOOL		IsFireThrough()  const { return m_bFireThrough; }
		DDWORD		GetDoorState()	const { return m_dwDoorState; }

	protected:

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, float lData);
		DDWORD		ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		virtual void Open();
		virtual void Opening();
		virtual void Closed();
		virtual void Closing();

		void		TriggerHandler();
		void		TriggerClose();
		void		SetOpen(DBOOL bPlaySound=DTRUE);
		void		SetOpening();
		void		SetClosed(DBOOL bPlaySound=DTRUE);
		void		SetClosing();
		void		SpawnTrigger();
		void		TriggerMsg(HOBJECT hSender, HSTRING hMsg);

		void		StartSound(HSTRING hstrSoundName, DBOOL bLoop);

		void		SetPortalState(DBOOL bOpen);
		DFLOAT		GetWaveValue( DFLOAT fSpeed, DFLOAT fPercent, DDWORD nWaveType );

		// places to keep the properties
		DDWORD		m_dwTriggerFlags;		//  Trigger type flags
		DFLOAT		m_fSpeed;				//  movement speed
		DFLOAT		m_fMoveDist;			//  distance to open
		DVector		m_vMoveDir;				//  direction to open
		HSTRING		m_hstrOpenBusySound;	//  sound to play while opening
		HSTRING		m_hstrOpenStopSound;	//  sound to play when done opening
		HSTRING		m_hstrCloseBusySound;	//  sound to play while closing
		HSTRING		m_hstrCloseStopSound;	//  sound to play when done closing
		DFLOAT		m_fWaitTime;			//  length of time to stay open
		DVector		m_vTriggerDims;			//  Dimensions for trigger box
		DFLOAT		m_fClosingSpeed;		//  movement speed while closing

		HOBJECT		m_hTriggerObj;			//	Handle to trigger object

		// Other variables
		DBOOL		m_bLocked;				//  door is locked flag
		DBOOL		m_bBoxPhysics;			//  door uses box physics
		DVector		m_vOpenPos;				//  door's open position
		DVector		m_vClosedPos;			//  door's closed position
		DDWORD		m_dwDoorState;			//  current door state
		DFLOAT		m_fDoorOpenTime;		//  door open timer

		HSOUNDDE	m_sndLastSound;		// Handle of last sound playing
		DFLOAT		m_fSoundRadius;		// Radius of sound
		DBOOL		m_bFirstUpdate;			// Do special stuff on the first update
		HSTRING		m_hstrPortal;			// A portal object for the door?
		DDWORD		m_dwFlags;			// Initial flags
		DDWORD		m_dwWaveform;

		DBOOL		m_bStartedMoving;	// Played the opening busy sound and opened the portal
		DFLOAT		m_fOpenDelay;		// Delay in opening

		// Trigger options

		DBOOL	m_bTrigTouchActivate;		// Trigger can be activated with TouchNotify
		DBOOL	m_bTrigPlayerActivate;		// Can be triggered by player
		DBOOL	m_bTrigAIActivate;			// Can be triggered by AI
		DBOOL	m_bTrigObjectActivate;		// Can be triggered by another object
		DBOOL	m_bTrigTriggerRelayActivate; // Can be triggered by another trigger
		DBOOL	m_bTrigNamedObjectActivate;	// Can it only be triggered by a specific object?
		HSTRING m_hstrTrigActivationObjectName;
		DBOOL	m_bTrigLocked;				// Trigger is locked.
		HSTRING	m_hstrTrigLockedMsg;		// Message to display when trigger is locked.
		HSTRING	m_hstrTrigLockedSound;		// Message to display when trigger is locked.
		HSTRING	m_hstrTrigUnlockedSound;		// Message to display when trigger is locked.
		HSTRING	m_hstrTrigKeyName;			// Name of key item needed to open the trigger.
		DBOOL	m_bFireThrough;

		DBOOL	m_bDoorBlocked;				// Door is blocked

	private :

		DBOOL		ReadProp(ObjectCreateStruct *pStruct);
		void		PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL		InitialUpdate(int nData);
		DBOOL		Update(DVector *pMovement);
		void		CacheFiles();
		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

};


#endif // __DOOR_H__