// ----------------------------------------------------------------------- //
//
// MODULE  : RemoteTurret.h
//
// PURPOSE : RemoteTurret create a weapon to be used by a player through messages...
//
// CREATED : 07/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __REMOTE_TURRET_H__
#define __REMOTE_TURRET_H__

#include "Turret.h"

LINKTO_MODULE( RemoteTurret );

class RemoteTurret : public Turret
{
	public: // Methods...

		RemoteTurret( );
		virtual ~RemoteTurret( );


	protected: // Methods... 

		// Handle messages from the engine...
		uint32 EngineMessageFn( uint32 messageID, void *pData, float fData );

		// Update the client data before sending...
		void PreCreateSpecialFX( TURRETCREATESTRUCT &rTurretCS );

		// Stop using the turret...
		virtual void Deactivate( );

		// Handle any cleanup required when the turret gets destroyed...
		virtual void OnDeath( );

		// Handle reactivating after loading a saved game...
		virtual void PostLoadActivate( HOBJECT hOperatingObject );


	private: // Methods...

		// Read in the properties of the object... 
		bool ReadProp( const GenericPropList *pProps );

		void Update( );

		// Save the object...
		void Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );

		// Load the object...
		void Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		// Activte the remote turret...
		void RemoteActivate( HOBJECT hSender );

		// Establish the previous and next remote turret links...
		void LinkRemoteTurrets( );

		// Allow another remote turret to set the previous link... 
		void SetPreviousRemoteTurret( HOBJECT hPrev );

		// Allow another remote turret to set the next link... 
		void SetNextRemoteTurret( HOBJECT hNext );


	private: // Members...

		// Links to next and prev RemoteTurret to cycle to once current RemoteTurret is deactivated...
		LTObjRef	m_hNextRemoteTurret;
		LTObjRef	m_hPrevRemoteTurret;

		std::string	m_sNextRemoteTurretName;

		
		// Message Handlers...
        
		DECLARE_MSG_HANDLER( RemoteTurret, HandleOnMsg );
};

#endif //__REMOTE_TURRET_H__
