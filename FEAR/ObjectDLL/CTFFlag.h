// ----------------------------------------------------------------------- //
//
// MODULE  : CTFFlag.h
//
// PURPOSE : CTFFlag object to place in CTF level.
//
// CREATED : 05/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CTFFLAG_H__
#define __CTFFLAG_H__

//
// Includes...
//

#include "GameBase.h"
#include "SharedFXStructs.h"

LINKTO_MODULE( CTFFlag );

class FlagStateMachine;

class CTFFlag : public GameBase
{
	public: // Methods...

		DEFINE_CAST( CTFFlag );

		CTFFlag( );
		virtual ~CTFFlag( );

		// Tells flag that it has been stolen.
		bool FlagStolen( HOBJECT hStealingPlayer );

		// Tells flag that it was captured.
		bool FlagCapture( );

		// Tells flag that it can do a capture assist.
		bool CaptureAssist( HOBJECT hFlagCapPlayer );

		// Accessor to the flagbase that owns this flag.
		HOBJECT GetFlagBase( ) const { return m_hFlagBase; }

	protected: // Methods... 

		// Handle messages from the engine...
		uint32 EngineMessageFn( uint32 messageID, void *pData, float fData );

		void OnLinkBroken( LTObjRefNotifier * pRef, HOBJECT hObj );

		// Send relevant information to clients...
		void CreateSpecialFX( bool bUpdateClients );

	private: // Methods...

		// Handle a MID_INITIALUPDATE message from the engine....
		void InitialUpdate( );

		// Read in the properties of the object... 
		bool ReadProp( const GenericPropList *pProps );
		
		// Update the create struct of the object
		bool PostReadProp( ObjectCreateStruct *pStruct );

		// Per-frame update.
		void Update( );

		// Touch notify handler.
		void HandleTouchNotify( HOBJECT hToucher );

	protected: // Members...

		// Handles state machine.
		friend class FlagStateMachine;
		FlagStateMachine* m_pFlagStateMachine;

		// Flagbase object that owns the flag.
		LTObjRef m_hFlagBase;

		// Player that is carrying the flag.
		LTObjRefNotifier m_hFlagCarrier;

		DECLARE_MSG_HANDLER( CTFFlag, HandleActivateMsg );
};

#endif // __CTFFLAG_H__
