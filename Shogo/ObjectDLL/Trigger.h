// ----------------------------------------------------------------------- //
//
// MODULE  : Trigger.h
//
// PURPOSE : Trigger - Definition
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include "cpp_engineobjects_de.h"
#include "Activation.h"

#define MAX_NUM_MESSAGES 10

#define UPDATE_DELTA					0.1f
#define TRIGGER_DEACTIVATION_TIME		0.001f


class Trigger : public BaseClass
{
	public :

		Trigger();
		~Trigger();

		void SetLocked(DBOOL bLocked=DTRUE) { m_bLocked = bLocked; }
		void SetActive(DBOOL bActive=DTRUE) { m_bActive = bActive; }
		void SetTargetName1(char* pName);
		void SetMessageName1(char* pMsg);
		void SetTriggerDelay(DFLOAT fDelay) { m_fTriggerDelay = fDelay; }
		void SetAITriggerable(DBOOL bBool)  { m_bAITriggerable = bBool; }

		void ToggleBoundingBoxes();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		virtual DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		virtual void  HandleInventoryQueryResponse(HOBJECT hSender, HMESSAGEREAD hRead);
		virtual void  HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead);
		virtual void  UpdateDelayingActivate();

		virtual void  ObjectTouch(HOBJECT hObj);
		virtual void  Unlock();
		virtual void  Activate();
		virtual void  RequestActivate();
		virtual void  DoTrigger (HOBJECT hObj, DBOOL bTouchNotify );

		void CreateBoundingBox();

		DBOOL	m_bActive;				// Are we currently 'live'?
		DFLOAT	m_fTriggerDelay;		// How long to wait after being triggered

		HSTRING m_hstrActivationSound;	// Name of our activation sound
		DFLOAT	m_fSoundRadius;			// Radius of activation sound

		HSTRING m_hstrTargetName[MAX_NUM_MESSAGES];	 // Names of our targets
		HSTRING m_hstrMessageName[MAX_NUM_MESSAGES]; // Message	 to send to our targets

		DBOOL	m_bTriggerTouch;		// Use touch trigger
		DBOOL	m_bTouchNotifyActivation;	// Activated by touch notify
		HSTRING m_hstrMessageTouch;		// Message to send to toucher
		HOBJECT m_hTouchObject;			// Object that touched me

		DBOOL	m_bPlayerTriggerable;	// Can the Player trigger me?
		DBOOL	m_bAITriggerable;		// Can AI's trigger me?
		HSTRING	m_hstrAIName;			// Name AI's that can trigger me
		
		DBOOL	m_bLocked;				// is this door locked?
		HSTRING	m_hstrUnlockKey;		// key name of key to unlock door

		DFLOAT	m_fAccessDeniedMsg;		// id of message for access denied
		DFLOAT	m_fAccessGrantedMsg;	// id of message for access granted

		HSTRING m_hstrAccessDeniedSound;	// sound to play for access denied
		HSTRING m_hstrAccessGrantedSound;	// sound to play for access granted

		DBOOL	m_bDelayingActivate;	// Are we currently delaying activate
		DFLOAT	m_fStartDelayTime;		// When did we start the delay
		DFLOAT	m_fSendDelay;			// How long do we wait
		DFLOAT	m_fLastTouchTime;		// Last time we were touched (and triggered)

		DDWORD	m_nActivationCount;		// How many times we are triggered before msgs are sent
		DDWORD	m_nCurrentActivation;	// Current value of count

		DBOOL	m_bWeightedTrigger;		// Is this a weighted trigger
		DFLOAT	m_fMessage1Weight;		// % of time message 1 is sent over message 2

		DBOOL	m_bTimedTrigger;		// Is this a timed trigger
		DFLOAT	m_fMinTriggerTime;		// Min time to wait to trigger
		DFLOAT	m_fMaxTriggerTime;		// Max time to wait to trigger
		DFLOAT	m_fNextTriggerTime;		// Next time to trigger object

		DBOOL	m_bAttached;			// Is the trigger attached to an object


	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		HOBJECT	m_hBoundingBox;			// Debug only
		DVector m_vDims;				// Dims
		CActivation m_activation;		// Handle activation

		HSTRING m_hstrAttachToObject;	// Name of object to attach to

	private :

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void	CacheFiles();

		void	AttachToObject();

		DBOOL	InitialUpdate();
		DBOOL	Update();
		DBOOL	ReadProp(ObjectCreateStruct *pData);
};

#endif // __TRIGGER_H__