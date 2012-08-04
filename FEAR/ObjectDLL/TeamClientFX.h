// ----------------------------------------------------------------------- //
//
// MODULE  : TeamClientFX.h
//
// PURPOSE : Places team specific clientfx in a level.
//
// CREATED : 05/25/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TEAMCLIENTFX_H__
#define __TEAMCLIENTFX_H__

//
// Includes...
//

#include "GameBase.h"
#include "SharedFXStructs.h"

LINKTO_MODULE( TeamClientFX );

class TeamClientFX : public GameBase
{
	public: // Methods...

		DEFINE_CAST( TeamClientFX );

		TeamClientFX( );
		virtual ~TeamClientFX( );

		// Accessor to the team.
		uint8 GetTeamId( ) const { return m_nTeamId; }

	protected: // Methods... 

		// Handle messages from the engine...
		uint32 EngineMessageFn( uint32 messageID, void *pData, float fData );

		// Send relevant information to clients...
		void CreateSpecialFX( bool bUpdateClients );

	private: // Methods...

		// Handle a MID_INITIALUPDATE message from the engine....
		void InitialUpdate( );

		// Read in the properties of the object... 
		bool ReadProp( const GenericPropList *pProps );
		
		// Update the create struct of the object
		bool PostReadProp( ObjectCreateStruct *pStruct );

	protected: // Members...

		// Record to the teamclientfx.
		HRECORD m_hTeamClientFXRec;

		// Team.
		uint8 m_nTeamId;
};

#endif // __TEAMCLIENTFX_H__
