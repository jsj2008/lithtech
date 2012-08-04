#ifndef __DOOR_H__
#define __DOOR_H__


#include "basedefs_de.h"

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "SurfaceTypes.h"

#define DF_SELFTRIGGER			(1<<0)	// Self triggerable
#define DF_STARTOPEN			(1<<1)	// Start in open state
#define DF_TRIGGERCLOSE			(1<<2)  // Must trigger door to close
#define DF_REMAINSOPEN			(1<<3)  // Door remains open
#define DF_FORCECLOSE			(1<<4)	// Force closing door to close

enum DoorWaveTypes {
	DOORWAVE_LINEAR,
	DOORWAVE_SINE,
	DOORWAVE_SLOWOFF,
	DOORWAVE_SLOWON
};

DFLOAT GetDoorWaveValue( DFLOAT fSpeed, DFLOAT fPercent, DDWORD nWaveType );

class Trigger;

// Door structure
class Door : public BaseClass
{
	public:

		Door();
		virtual ~Door();

		DFLOAT	GetMoveDist()	const { return m_fMoveDist; }

	protected:

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		virtual void Open();
		virtual void Opening();
		virtual void Closed();
		virtual void Closing();

		void	TriggerHandler();
		void	TriggerClose();
		void	SetOpen(DBOOL bPlaySound=DTRUE);
		void	SetOpening();
		void	SetClosed(DBOOL bPlaySound=DTRUE);
		void	SetClosing();
		void	SpawnTrigger();
		void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);

		void	StartSound(HSTRING hstrSoundName, DBOOL bLoop);

		// places to keep the properties

		HSTRING m_hstrOpenStartSound;	// sound to play when opening starts
		HSTRING m_hstrCloseStartSound;	// sound to play when closing starts
		HSTRING m_hstrOpenBusySound;	// sound to play while opening
		HSTRING m_hstrOpenStopSound;	// sound to play when done opening
		HSTRING m_hstrCloseBusySound;	// sound to play while closing
		HSTRING m_hstrCloseStopSound;	// sound to play when done closing
		HSTRING m_hstrPortalName;		// Portal to open/close

		DVector m_vMoveDir;				// direction to open
		DVector m_vSoundPos;			// position to play sound (optional)
		DVector m_vTriggerDims;			// Dimensions for trigger box
		DVector m_vOpenPos;				// door's open position
		DVector m_vClosedPos;			// door's closed position

		DFLOAT	m_fSpeed;				// movement speed
		DFLOAT	m_fMoveDist;			// distance to open
		DFLOAT	m_fOpenWaitTime;		// length of time to stay open
		DFLOAT	m_fCloseWaitTime;		// length of time to stay closed
		DFLOAT	m_fClosingSpeed;		// movement speed while closing
		DFLOAT	m_fMoveStartTime;		// Time movement started
		DFLOAT	m_fMoveDelay;			// Time to wait to move
		DFLOAT	m_fDoorStopTime;		// time door stopped moving
		DFLOAT	m_fSoundRadius;			// Radius of sound

		DBOOL	m_bBoxPhysics;			// Use box physics
		DBOOL	m_bAITriggerable;		// AIs can trigger this baby
		DBOOL	m_bPlayedBusySound;		// Did we play the busy sound yet?

		DDWORD	m_dwStateFlags;			// Property info
		DDWORD	m_dwDoorState;			// current door state
		DDWORD	m_dwWaveform;

		HOBJECT m_hTriggerObj;			//  Used with self-triggering doors

		
	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		HSOUNDDE	m_sndLastSound;		// Handle of last sound playing
	
		SurfaceType m_eSurfaceType;
		DFLOAT		m_fMass;			// Door's mass

	private :

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(int nInfo);
		DBOOL	Update();

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();
};


#endif // __DOOR_H__