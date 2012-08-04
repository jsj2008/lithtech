// ----------------------------------------------------------------------- //
//
// MODULE  : ClientFXSequence.h
//
// PURPOSE : Creates ClientFX, one after the other, in sequence...
//
// CREATED : 4/20/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTFX_SEQUENCE_H__
#define __CLIENTFX_SEQUENCE_H__

//
// Includes...
//

	#include "StateMachine.h"

class CClientFXSequence : public MacroStateMachine
{
	public: // Methods...

		CClientFXSequence( )
			:	m_hFXSequenceRecord( NULL )
			,	m_ClientFXLink( )
			,	m_hParentObject( NULL )
		{ }

		void SetClientFXSequenceRecord( HRECORD hRecod ) { m_hFXSequenceRecord = hRecod; }

		// Associate a parent object with the ClientFX...
		void SetParentObject( HOBJECT hObject ) { m_hParentObject = hObject; }

		// Save/Load of the sequence...
		void Save( ILTMessage_Write &rMsg );
		void Load( ILTMessage_Read &rMsg );
		
		enum EState
		{
			eState_Begin,
			eState_Loop,
			eState_End,
		};

	private: // Methods...

		// Statemachine handlers for the Begin state...
		bool Begin_OnEnter( MacroStateMachine::EventParams &rEventParams );
		bool Begin_OnUpdate( MacroStateMachine::EventParams &rEventParams );
		bool Begin_OnExit( MacroStateMachine::EventParams &rEventParams );

		// Statemachine handlers for the Loop state...
		bool Loop_OnEnter( MacroStateMachine::EventParams &rEventParams );
		bool Loop_OnExit( MacroStateMachine::EventParams &rEventParams );

		// Statemachine handlers for the End state...
		bool End_OnEnter( MacroStateMachine::EventParams &rEventParams );
		bool End_OnUpdate( MacroStateMachine::EventParams &rEventParams );
		bool End_OnExit( MacroStateMachine::EventParams &rEventParams );

		// Define the statemachine...
		MSM_BeginTable( CClientFXSequence )
			MSM_BeginState( eState_Begin )
				MSM_OnEnter( Begin_OnEnter )
				MSM_OnUpdate( Begin_OnUpdate )
				MSM_OnExit( Begin_OnExit )
			MSM_EndState( )
			MSM_BeginState( eState_Loop )
				MSM_OnEnter( Loop_OnEnter )
				MSM_OnExit( Loop_OnExit )
			MSM_EndState( )
			MSM_BeginState( eState_End )
				MSM_OnEnter( End_OnEnter )
				MSM_OnUpdate( End_OnUpdate )
				MSM_OnExit( End_OnExit )
			MSM_EndState( )
		MSM_EndTable( )


	private: // Members...

		// ClientFXSequence record that specifies which FX to play for the different states...
		HRECORD			m_hFXSequenceRecord;

		CClientFXLink	m_ClientFXLink;

		// Optional object to attach the ClientFX to...
		LTObjRef		m_hParentObject;

};

#endif // __CLIENTFX_SEQUENCE_H__

// EOF
