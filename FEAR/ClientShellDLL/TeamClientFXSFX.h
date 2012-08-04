// ----------------------------------------------------------------------- //
//
// MODULE  : TeamClientFXSFX.h
//
// PURPOSE : Client side representation on TeamClientFX
//
// CREATED : 05/25/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TEAMCLIENTFXSFX_H__
#define __TEAMCLIENTFXSFX_H__

//
// Includes...
//

#include "SpecialFX.h"
#include "SharedFXStructs.h"

class TeamClientFXSFX : public CSpecialFX
{
	public:	// Methods...

		TeamClientFXSFX( );
		virtual ~TeamClientFXSFX( );

		// Initialize the object from a message.  hServObj is the server side TeamClientFX object.
		virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
        		
		// Initialize the object from a createstruct.
		virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);

		// Terminate the object
		virtual void Term( )
		{
			CSpecialFX::Term( );
			ShutdownClientFX();
		}

		// Update the weapon associated with the turret...
		virtual bool Update( );

		// Retrieve the ID associated with TurretFX objects...
		virtual uint32 GetSFXID( ) { return SFX_TEAMCLIENTFX_ID; }

		// Handle a message recieved from the server side Turret object...
		virtual bool OnServerMessage( ILTMessage_Read *pMsg );

		// Creates a clientfx based on team. 
		static bool CreateTeamClientFX( HOBJECT hParent, HRECORD hTeamClientFxRec, uint8 nTeamId, CClientFXLink& clientFxLink );

	private:

		// Shutdown any ClientFX currently playing...
		void ShutdownClientFX( );

		// Creates the clientfx for the team.
		void CreateClientFX( );

	private : // Members...

		// Declare delegate to listen for player kill events.
		static void OnPlayerChangedTeamsEvent( TeamClientFXSFX* pTeamClientFXSFX, CClientInfoMgr* pClientInfoMgr, EventCaster::NotifyParams& notifyParams )
		{
			if( pTeamClientFXSFX->m_ClientFxLink.IsValid( ))
			{
				// Update our clientfx.
				pTeamClientFXSFX->CreateClientFX();
			}
		}
		Delegate< TeamClientFXSFX, CClientInfoMgr, OnPlayerChangedTeamsEvent > m_delegatePlayerChangedTeamsEvent;

		TEAMCLIENTFXCREATESTRUCT m_cs;

		CClientFXLink m_ClientFxLink;
};


#endif // __TEAMCLIENTFXSFX_H__