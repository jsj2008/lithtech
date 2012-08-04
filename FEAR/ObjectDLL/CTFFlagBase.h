// ----------------------------------------------------------------------- //
//
// MODULE  : CTFFlagBase.h
//
// PURPOSE : CTFFlagBase object to place in CTF level.
//
// CREATED : 05/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CTFFLAGBASE_H__
#define __CTFFLAGBASE_H__

//
// Includes...
//

#include "GameBase.h"
#include "SharedFXStructs.h"

LINKTO_MODULE( CTFFlagBase );

class FlagBaseStateMachine;

class CTFFlagBase : public GameBase
{
	public: // Methods...

		DEFINE_CAST( CTFFlagBase );

		CTFFlagBase( );
		virtual ~CTFFlagBase( );

		// Accessor to the flagbase db record used.
		HRECORD GetFlagBaseRec( ) const { return m_hFlagBaseRec; }

		// Accessor to the team.
		uint8 GetTeamId( ) const { return m_nTeamId; }

		// Returns the flag to the base.
		bool ReturnFlag( );

		// Removes the flag.
		bool RemoveFlag( );

		// Prefetch resources for this object.
		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

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

		// Creates its flag object.
		bool CreateFlag( );

	protected: // Members...

		// Handles state machine.
		friend class FlagBaseStateMachine;
		FlagBaseStateMachine* m_pFlagBaseStateMachine;

		// Object that is currently operating the turret...
		LTObjRefNotifier m_hFlag;

		// CTFFlagBase record to use.
		HRECORD m_hFlagBaseRec;

		// Team.
		uint8 m_nTeamId;

		// Teleports the object to the floor once it is created...
		bool m_bMoveToFloor;

		// Allow flag steals from this flagbase.
		bool m_bAllowSteals;
};

#endif // __CTFFLAGBASE_H__
