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

#include "cpp_server_de.h"
#include "cpp_engineobjects_de.h"
#include "B2BaseClass.h"

#define MAX_TRIGGER_MESSAGES	10


// Trigger class
class Trigger : public B2BaseClass
{
	public:
		
		Trigger();
		~Trigger();

		void	SetActive(DBOOL bActive) { m_bActive = bActive; }
		void	SetTargetName(int index, char *szTarget);
		void	SetMessageName(int index, char *szTrigger);
		void	Setup(DBOOL bActive, char *pszTarget, char *pszMessage, DBOOL bTouchActivate,
					DBOOL bPlayerActivate, DBOOL bAIActivate, DBOOL bObjectActivate, 
					DBOOL bTriggerRelayActivate, DBOOL bNamedObjectActivate, 
					HSTRING hstrActivationObjectName, DBOOL bTrigLocked,
					HSTRING hstrTrigLockedMsg, HSTRING hstrTrigLockedSound, HSTRING hstrTrigUnlockedMsg,
					HSTRING hstrTrigUnlockedSound, HSTRING hstrTrigKeyName);

	protected:

		virtual void HandleKeyQueryResponse(HOBJECT hSender, HMESSAGEREAD hRead);
		virtual void HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead);
		virtual void SendMessage(int nSlot);
		virtual void SendMessages();

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	ObjectTouch (HOBJECT hObj);
		DBOOL	InitialUpdate(DVector *position);
		DBOOL	Update(DVector *position);
		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		DBOOL	ValidateSender(HOBJECT hObj);
		void	PlayActivationSound();
	
	private:

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

	protected:

		DBOOL	m_bActive;				// Are we currently 'live'?
		DBOOL	m_bDelay[MAX_TRIGGER_MESSAGES];			// Currently delaying message sending
		DVector m_vDims;				// Size of the object
		DBOOL	m_bLocked;				// Trigger is locked.
		HSTRING	m_hstrLockedMsg;		// Message to display when trigger is locked.
		HSTRING	m_hstrLockedSound;		// Message to display when trigger is locked.
		HSTRING	m_hstrUnlockedMsg;		// Message to display when trigger is locked.
		HSTRING	m_hstrUnlockedSound;	// Message to display when trigger is locked.
		HSTRING	m_hstrKeyName;			// Name of key item needed to open the trigger.

		DFLOAT	m_fTriggerResetTime;	// How long to wait after being triggered
		DFLOAT	m_fTriggerMessageDelay[MAX_TRIGGER_MESSAGES];	// Time to wait to send trigger message
		DFLOAT	m_fMaxTriggerMessageDelay;
		DFLOAT	m_fDelayStartTime;		// Time a message delay started

		HSTRING m_hstrActivationSound;	// Name of our activation sound
		DFLOAT	m_fSoundRadius;			// Radius of activation sound

		HSTRING m_hstrTargetName[MAX_TRIGGER_MESSAGES];		// Name of our first target
		HSTRING m_hstrMessageName[MAX_TRIGGER_MESSAGES];	// Message to send to our first target

		HSTRING m_hstrActivationObjectName;	// Message to send to our second target

		DBOOL	m_bTouchActivate;		// Trigger can be activated with TouchNotify
		DBOOL	m_bPlayerActivate;		// Can be triggered by player
		DBOOL	m_bAIActivate;			// Can be triggered by AI
		DBOOL	m_bObjectActivate;		// Can be triggered by another object
		DBOOL	m_bTriggerRelayActivate;	// Can be triggered by another trigger
		DBOOL	m_bNamedObjectActivate;	// Can it only be triggered by a specific object?
		HOBJECT m_hLastSender;			// Last object we were triggered by

		DDWORD	m_nActivationCount;
		DDWORD	m_nCurrentActivation;

		DBOOL	m_bSending;
};


inline void Trigger::SetTargetName(int index, char *szTarget)
{
	if (index < 0 || index >= MAX_TRIGGER_MESSAGES) return;

	if (m_hstrTargetName[index])
		g_pServerDE->FreeString(m_hstrTargetName[index]);
	m_hstrTargetName[index] = g_pServerDE->CreateString(szTarget);
}


inline void Trigger::SetMessageName(int index, char *szMessage)
{
	if (index < 0 || index >= MAX_TRIGGER_MESSAGES) return;

	if (m_hstrMessageName[index])
		g_pServerDE->FreeString(m_hstrMessageName[index]);
	m_hstrMessageName[index] = g_pServerDE->CreateString(szMessage);
}

void SendTriggerMsgToObjects(LPBASECLASS pSender, HSTRING hName, HSTRING hMsg);
void SendTriggerMsgToClass(LPBASECLASS pSender, HCLASS hClass, HSTRING hMsg);



// BigTrigger class - obsolete
class BigTrigger : public Trigger
{
};


// Toggle trigger class
// Trigger class
class ToggleTrigger : public Trigger
{
	public:
		
		ToggleTrigger();
		~ToggleTrigger();

	protected:

		virtual void SendMessages();

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float lData);
	
	private:

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

	protected:

		DBOOL	m_bOn;					// Are we currently on?

		HSTRING m_hstrOffMessageName[MAX_TRIGGER_MESSAGES];	// Message to send to our first target
};




#endif // __TRIGGER_H__