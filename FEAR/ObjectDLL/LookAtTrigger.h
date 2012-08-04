// ----------------------------------------------------------------------- //
//
// MODULE  : LookAtTrigger.h
//
// PURPOSE : Trigger that executes commands based on a players view...
//
// CREATED : 3/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LOOK_AT_TRIGGER_H__
#define __LOOK_AT_TRIGGER_H__

#include "GameBase.h"

LINKTO_MODULE( LookAtTrigger );

class LookAtTrigger : public GameBase 
{
	public:	// Methods...

		LookAtTrigger( );
		~LookAtTrigger( );

		
	private:
	
		enum State
		{
			eStateNormal,		// Idle, no player is looking at or away...
			eStateLookingAt,	// A player is currently looing at but has not yet activated LookAt...
			eStateLookedAt,		// Player activated LookAt but has not yet looked away...
			eStateLookingAway,	// Player is currently looking away but not yet activate LookAway...
		};


	private: // Methods...

		// Engine overrides... 

		// Recieve messages from the engine...
		uint32 EngineMessageFn( uint32 messageID, void *pData, float fData );

		// Handle state clean up if the player that is looking at the LookAtTrigger gets removed...
		void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Read in the properties and fill in the create struct when a MID_PRECREATE message is recieved...
		void OnPrecreateMsg( ObjectCreateStruct* pOCS, float fType );

		// Handle any initializing that requires the LookAtTrigger object to be created...
		void OnObjectCreatedMsg( float fType );

		// Update messge recieved...
		void OnUpdateMsg( );

		// Save the state when a MID_LOADOBJECT message is recieved...
		void OnSaveMsg( ILTMessage_Write *pMsg, uint32 dwFlags );

		// Load the state when a MID_LOADOBJECT message is recieved...
		void OnLoadMsg( ILTMessage_Read *pMsg, uint32 dwFlags );

		// Check if the player is in range and the LookAtTrigger is in the players line of sight...
		bool CanPlayerLookAt( CPlayerObj *pPlayer );

		// Determine if the LookAtTrigger can be looked at by the player, and if so activate the LookAt state...
		bool DoPlayerLookAt( CPlayerObj *pPlayer );

		// If allowed to, executes the LookAt command and increments the number of LookAts...
		bool ActivateLookAt( HOBJECT hActivate );

		// Checks if the player is currently the LookAtPlayer and calculates if they are still looking at the LookAtTrigger...
		bool IsPlayerLookingAt( CPlayerObj *pPlayer );

		// Determines if the LookAtTrigger can be looked away from by the plaeyr, and if so activate the LookAway state...
		bool DoPlayerLookAway( );

		// If allowed to, executes the LookAway command and increments the number of LookAways..
		bool ActivateLookAway( HOBJECT hActivate );

		// Handle updating the normal, idle state...
		void UpdateNormal( );
		
		// Handle updating the looking at state...
		void UpdateLookingAt( );

		// Handle updating the looked at state...
		void UpdateLookedAt( );

		// Handle updating the looking away state...
		void UpdateLookingAway( );

		// Turns on or off the ability of the LookAt trigger to recieve rayhits...
		void AllowRayHits( bool bRayHits );

		// Switches to the specified state...
		void SetState( LookAtTrigger::State eState );

	
	private: // Members...

		// Players must be this distance from the center of the LookAtTrigger to be considered looking at it...
		float		m_fActivationRadiusSqr;

		// Maximum number of LookAts...
		int32		m_nLookAtsMax;

		// Current number of times the LookAtTrigger has been looked at...
		int32		m_nLookAtsNum;

		// When locked, no LookAts will occur...
		bool		m_bLookAtsLocked;

		// Amount of time, in seconds, a player must be looking at the LookAtTrigger before they are considered looking at it...
		float		m_fLookAtThreshold;

		// The current number of seconds a player has been looking at the LookAtTrigger...
		float		m_fLookAtTime;

		// Command to execute when a player is considered looking at the LookAtTrigger...
		std::string	m_sLookAtCommand;

		// Maximum number of LookAways...
		int32		m_nLookAwaysMax;

		// Current number of times the LookAtTrigger has been looked away from...
		int32		m_nLookAwaysNum;

		// When locked, no LookAways will occur...
		bool		m_bLookAwaysLocked;

		// Amount of time, in seconds, a player must be looking away from the LookAtTrigger before they are considered looking away from it...
		float		m_fLookAwayThreshold;

		// The current number of seconds a player has been looking away from the LookAtTrigger...
		float		m_fLookAwayTime;

		// Command to execute when a player is considered looking away from the LookAtTrigger...
		std::string	m_sLookAwayCommand;

		// The current player looking at the LookAtTrigger...
		LTObjRefNotifier m_hLookAtPlayer;

		// What state the LookAtTrigger is in...
		State		m_eState;


		// Message Handlers...

		DECLARE_MSG_HANDLER( LookAtTrigger, HandleLockMsg );
		DECLARE_MSG_HANDLER( LookAtTrigger, HandleUnlockMsg );
		DECLARE_MSG_HANDLER( LookAtTrigger, HandleLookAtMsg );
		DECLARE_MSG_HANDLER( LookAtTrigger, HandleLookAwayMsg );
};

// LookAtTrigger plugin class for interfacing with the world editor...

class CLookAtTriggerPlugin : public IObjectPlugin 
{
	public: // Methods...

		virtual LTRESULT PreHook_PropChanged( const	char		*szObjName,
											  const	char		*szPropName,
											  const	int			nPropType,
											  const	GenericProp	&gpPropValue,
											  ILTPreInterface	*pInterface,
											  const char		*szModifiers );

	protected: // Members...

		CCommandMgrPlugin		m_CommandMgrPlugin;
};

#endif // __LOOK_AT_TRIGGER_H__
