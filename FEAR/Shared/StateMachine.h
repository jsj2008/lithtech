// ----------------------------------------------------------------------- //
//
// MODULE  : StateMachine.h
//
// PURPOSE : Definition of StateMachine class.
//
// CREATED : 06/25/04
//
// (c) 1996-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_

// ----------------------------------------------------------------------- //
//
// class StateMachine
//
// Definition of base statemachine class.  The template T defines
// what type of object will be passed in as a context variable.  This
// allows clients to define what type of parameters are supported 
// in the event calls such as Update and OnMessage.  Clients must 
// derive from the StateObject in order to define specific behavior.
// Clients will usually use the StateMachine class directly.  
//
// ----------------------------------------------------------------------- //
template< typename T >
class StateMachine
{
	public:

		StateMachine( )
		{
			m_pCurrentState = NULL;
		}

	public:

		// StateObject defines the behavior for a single state.
		class StateObject
		{
			public:

				// Called when a state is entered.
				virtual bool Enter( T context ) { return true; }

				// Called when a state is left.
				virtual bool Exit( T context ) { return true; }

				// Called each frame by the StateMachine.
				virtual void Update( T context ) { }

				// Called when a message is received and passed on by the StateMachine.
				virtual bool OnMessage( T context, ILTMessage_Read& msg ) { return false; }
		};

		// Sets the current state.  If we were in a state, that
		// state's Exit function is called.  Then the new state's
		// Enter function is called.
		void SetCurrentState( T context, StateObject* pCurrentState )
		{
			if( m_pCurrentState )
			{
				m_pCurrentState->Exit( context );
			}

			m_pCurrentState = pCurrentState;

			if( m_pCurrentState )
			{
				m_pCurrentState->Enter( context );
			}
		}

		StateObject* GetCurrentState( ) const { return m_pCurrentState; }

		void Update( T context )
		{
			// Update our state machine.
			if( !m_pCurrentState )
				return;

			m_pCurrentState->Update( context );
		}

		bool OnMessage( T context, ILTMessage_Read& msg )
		{
			// Update our state machine.
			if( !m_pCurrentState )
				return false;

			return m_pCurrentState->OnMessage( context, msg );
		}

	private:

		StateObject* m_pCurrentState;
};


// ----------------------------------------------------------------------- //
//
// class MacroStateMachine
//
// Defines a statemachine with a set of macros so that separate
// classes are not needed.  Also allows defining complete statemachine
// in table so all states can be viewed together.
//
// ----------------------------------------------------------------------- //

//Example:
/*

class TestStateMachine : public MacroStateMachine
{
public:

	TestStateMachine( )
	{
		m_nCount = 0;
	}

	enum EStates
	{
		eState_First,
		eState_Second,
		eState_Third,
	};

	bool DoSpecialEvent( uint32 nValue )
	{
		SpecialEventParams specialEventParams;
		specialEventParams.m_nValue = nValue;
		return DoUserEvent( eEvent_Special, specialEventParams );
	}

protected:

	enum EEvents
	{
		eEvent_Special = EVENT_User,
	};

	struct SpecialEventParams : public EventParams
	{
		SpecialEventParams( )
		{
			m_nValue = 0;
		}

		uint32 m_nValue;
	}

	bool First_OnEnter( EventParams& eventParams )
	{
		printf( "%s\n", __FUNCTION__ );
		m_nCount = 0;
		return true;
	}
	bool First_OnUpdate( EventParams& eventParams )
	{
		printf( "%s\n", __FUNCTION__ );
		m_nCount++;
		if( m_nCount > 5 )
			SetState( eState_Second );
		return true;
	}
	bool First_OnSpecial( EventParams& eventParams )
	{
		SpecialEventParams& specialEventParams = ( SpecialEventParams& )eventParams;
		printf( "%s\n", __FUNCTION__ );
		m_nCount = specialEventParams.m_nValue;
		return true;
	}

	bool Second_OnUpdate( EventParams& eventParams )
	{
		printf( "%s\n", __FUNCTION__ );
		m_nCount--;
		if( m_nCount == 0 )
			SetState( eState_First );
		return true;
	}
	bool Second_OnExit( EventParams& eventParams )
	{
		printf( "%s\n", __FUNCTION__ );
		return true;
	}
	bool Second_OnMsg( EventParams& eventParams )
	{
		MessageEventParams& messageEventParams = ( MessageEventParams& )eventParams;
		printf( "%s %s\n, __FUNCTION__, messageEventParams.m_pMsg->ReadString( );
		return true;
	}

	MSM_DeclareTable( );

private:

	unsigned int m_nCount;
};

MSM_BeginTable( TestStateMachine )
	MSM_BeginState( eState_First )
		MSM_OnEnter( First_OnEnter )
		MSM_OnUpdate( First_OnUpdate )
		MSM_OnEvent( eEvent_Special, First_OnSpecial )
	MSM_EndState( )
	MSM_BeginState( eState_Second )
		MSM_OnUpdate( Second_OnUpdate )
		MSM_OnExit( Second_OnExit )
		MSM_OnMsg( Second_OnMsg )
	MSM_EndState( )
MSM_EndTable( )
*/

// If statemachine is defined in implementation file, then put this in the header
// of the statemachine class.
#define MSM_DeclareTable( )			bool DoEvent( uint32 nState, EStateMachineEvent eEvent, EventParams& eventParams );

// Begin block of statemachine states.
#define MSM_BeginTable( TClass )	bool TClass::DoEvent( uint32 nState, EStateMachineEvent eEvent, EventParams& eventParams ) \
									{ \
										switch( nState ) \
										{ 

// Begin block of specific state event handlers.
#define MSM_BeginState( state )			case state: \
											{ \
												switch( eEvent ) \
												{ 
// Helper for defining event handler.
#define MSM_OnEvent( event, func )				case event: \
													return func( eventParams ); \
													break;

// End block of specific state event handlers.
// Returns true if no handler.
#define MSM_EndState( )							default: \
													return true; \
													break; \
												} \
											} \
											break;
// End block of statemachine states.
// Returns false if invalid state.
#define MSM_EndTable( )					} \
										return false; \
									}

// Define Enter event handler.
#define MSM_OnEnter( func )			MSM_OnEvent( EVENT_Enter, func )
// Define Update event handler.
#define MSM_OnUpdate( func )		MSM_OnEvent( EVENT_Update, func )
// Define Exit event handler.
#define MSM_OnExit( func )			MSM_OnEvent( EVENT_Exit, func )
// Define ILTMessage_Read event handler.
#define MSM_OnMsg( func )			MSM_OnEvent( EVENT_Msg, func )

class MacroStateMachine
{
public:

	enum 
	{
		// Current state is intialized to invalid on construction.
		eInvalid_State = -1,
	};

	MacroStateMachine( )
	{
		m_nCurrentState = (uint32)eInvalid_State;
	}

	// Switch to the new state.  Return values from event handlers are passed back to calling code.
	// If an Exit handler returns false, then the state is not switched.
	virtual bool SetState( uint32 nNewState )
	{
		// See if we're already at the state.
		if( nNewState == m_nCurrentState )
			return true;

		// Do the exit event from the current state.
		if( m_nCurrentState != eInvalid_State )
		{
			// Don't proceed with state switch if return not true.  Client code must handle the error.
			EventParams cParams( nNewState, m_nCurrentState );
			if( !DoEvent( m_nCurrentState, EVENT_Exit, cParams ))
				return false;
		}

		// Switch to the new state.
		uint32 nPreviousState = m_nCurrentState;
		m_nCurrentState = nNewState;

		// Do the enter event for the new state.
		if( nNewState != eInvalid_State )
		{
			// State switch could not complete.  client code must handle the error.
			EventParams cParams( m_nCurrentState, nPreviousState );			
			if( !DoEvent( m_nCurrentState, EVENT_Enter, cParams ))
				return false;
		}

		return true;
	}

	// Get our current state.
	uint32 GetState( ) const { return m_nCurrentState; }

	// Update the state.
	bool Update( ) 
	{
		EventParams cParams( m_nCurrentState, m_nCurrentState );
		return DoEvent( m_nCurrentState, EVENT_Update, cParams );
	}

	// Send ILTMessage event to state.
	bool DoMessage( ILTMessage_Read& msg ) 
	{
		MessageEventParams cParams( msg );
		return DoEvent( m_nCurrentState, EVENT_Msg, cParams );
	}

protected:

	// Set of StateMachine events.
	enum EStateMachineEvent
	{
		// Entering state.
		EVENT_Enter,
		// Frame update for state.
		EVENT_Update,
		// Exiting state.
		EVENT_Exit,
		// State getting ILTMessage_Read.
		EVENT_Msg,
		// User events.  Assign first user event enum
		// to this value.
		EVENT_User,
	};

	// Defines event parameters passed to event handlers.  To pass data to user
	// events, derive from this struct, provide fields with data, and pass it
	// to the DoUserEvent method.
	struct EventParams
	{
		EventParams( )
		{
			m_nNewState = (uint32) eInvalid_State;
			m_nPreviousState = (uint32) eInvalid_State;
		}

		EventParams( uint32 nNewState, uint32 nPreviousState )
		{
			m_nNewState = nNewState;
			m_nPreviousState = nPreviousState;
		}

		// Current or new state.
		uint32 m_nNewState;
		// Previous state if just entering a new state.
		uint32 m_nPreviousState;
	};

	// Specialized event paramaters for handling a message event.
	struct MessageEventParams : public EventParams
	{
		MessageEventParams( )
		{
			m_pMsg = NULL;
		}

		MessageEventParams( ILTMessage_Read& msg )
		{
			m_pMsg = &msg;
		}

		// The message that is to be handled.
		ILTMessage_Read* m_pMsg;
	};

	// Send user event to state.
	bool DoUserEvent( uint32 nEvent, EventParams& eventParams )
	{
		eventParams.m_nNewState = m_nCurrentState;
		eventParams.m_nPreviousState = m_nCurrentState;
		return DoEvent( m_nCurrentState, ( EStateMachineEvent )nEvent, eventParams );
	}

	// Concrete statemachines must implement this function through macros.
	virtual bool DoEvent( uint32 nState, EStateMachineEvent eStateMachineEvent, EventParams& eventParams ) = 0;

private:

	uint32 m_nCurrentState;
};

#endif
