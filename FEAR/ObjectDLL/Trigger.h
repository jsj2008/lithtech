// ----------------------------------------------------------------------- //
//
// MODULE  : Trigger.h
//
// PURPOSE : Trigger - Definition
//
// CREATED : 10/6/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include "GameBase.h"
#include "CommandMgr.h"
#include "SharedFXStructs.h"
#include "TriggerTypeDB.h"

#define MAX_NUM_COMMANDS 10

LINKTO_MODULE( Trigger );

class Trigger : public GameBase
{
	public :

		Trigger();
		~Trigger();

		void	SetLocked(bool bLocked=true)	{ m_bLocked = bLocked; }
		void	SetTriggerDelay(float fDelay)	{ m_fTriggerDelay = fDelay; }
		void	SetAITriggerable(bool bBool)	{ m_bAITriggerable = bBool; }

		uint8		GetTeamID()			const { return m_nTeamID; }

	protected :

		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, float fData);

		virtual void	UpdateDelayingActivate();

		virtual void	ObjectTouch(HOBJECT hObj);
		virtual bool	Activate();
		virtual void	RequestActivate();
		virtual void	DoTrigger(HOBJECT hObj);

		virtual	void	CreateSpecialFX( bool bUpdateClients = false );
		virtual void	SendLockedMsg();

		bool		m_bActive;				// Are we currently 'live'?
		float		m_fTriggerDelay;		// How long to wait after being triggered

		std::string	m_sActivationSound;		// Name of our activation sound
		float		m_fSoundRadius;			// Radius of activation sound

		StringArray	m_saCommands;			// Commands to execute

		LTObjRef	m_hTouchObject;				// Object that touched me

		bool		m_bPlayerTriggerable;	// Can the Player trigger me?
		bool		m_bAITriggerable;		// Can AI's trigger me?
		std::string	m_sAIName;				// Name AI's that can trigger me

		bool		m_bLocked;				// Is this trigger locked?

		bool		m_bDelayingActivate;	// Are we currently delaying activate
		double		m_fStartDelayTime;		// When did we start the delay
		float		m_fSendDelay;			// How long do we wait
		double		m_fLastTouchTime;		// Last time we were touched (and triggered)

		uint32		m_nActivationCount;		// How many times we are triggered before msgs are sent
		uint32		m_nCurrentActivation;	// Current value of count

		int			m_nNumActivations;		// How many times the trigger can be activated (<= 0 is infinite)
		int			m_nNumTimesActivated;	// How many times has the trigger been activated

		bool		m_bTimedTrigger;		// Is this a timed trigger
		float		m_fMinTriggerTime;		// Min time to wait to trigger
		float		m_fMaxTriggerTime;		// Max time to wait to trigger
		StopWatchTimer	m_TriggerTimer;		// Countdown for timed triggers...

		bool		m_bAttached;			// Is the trigger attached to an object
		
		bool		m_bSendTriggerFXMsg;	// Should we send the message to create a trigger fx.

		uint8		m_nTeamID;		// When in a team game this specifies which team can activate this trigger.

		// Allocate the trigger create struct...
		void CreateTriggerCreateStruct( );
		TRIGGERCREATESTRUCT *m_pTriggerCS;
		
	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		LTVector	m_vDims;				// Dims
		std::string	m_sAttachToObject;		// Name of object to attach to
		uint32		m_dwFlags;				// Our initial flags

		private :

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void	AttachToObject();

		bool	InitialUpdate();
		bool	Update();
		bool	ReadProp(const GenericPropList *pProps);


		// Messge handlers...

		DECLARE_MSG_HANDLER( Trigger, HandleLockMsg );
		DECLARE_MSG_HANDLER( Trigger, HandleUnlockMsg );
		DECLARE_MSG_HANDLER( Trigger, HandleOnMsg );
		DECLARE_MSG_HANDLER( Trigger, HandleTeamMsg );
};

class CTriggerPlugin : public IObjectPlugin
{
	public :

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

		virtual LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const char *szModifiers );

	protected :

		CCommandMgrPlugin m_CommandMgrPlugin;
		TriggerTypeDBPlugin m_TriggerTypeDBPlugin;
};

#endif // __TRIGGER_H__
