// ----------------------------------------------------------------------- //
//
// MODULE  : TurretFX.h
//
// PURPOSE : Client side representation on Turret objets, used for activation and weapons...
//
// CREATED : 07/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TURRETFX_H__
#define __TURRETFX_H__

//
// Includes...
//

#include "SpecialFX.h"
#include "SharedFXStructs.h"

class CTurretFX : public CSpecialFX
{
	public:	// Methods...

		CTurretFX( );
		virtual ~CTurretFX( );

		// Initialize the client side turret class...
		virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );
        		
		// Update the weapon associated with the turret...
		virtual bool Update( );

		// Retrieve the ID associated with TurretFX objects...
		virtual uint32 GetSFXID( ) { return SFX_TURRET_ID; }

		// Handle a message recieved from the server side Turret object...
		virtual bool OnServerMessage( ILTMessage_Read *pMsg );

		// Query to see if someone else is operating the turret...
		bool IsInUse( ) const { return (m_csTurret.m_hOperatingObject != NULL); }

		// Retrieve the object currently operating the turret...
		HOBJECT GetOperatingObject( ) const { return m_csTurret.m_hOperatingObject; }

		// Query to see if the turret can be activated...
		bool CanActivate( );

		// Activates the turret and sets the player up for using it...
		bool Activate( );

		// Deactivate the turret...
		bool Deactivate( );

		// Retrieve the record of the turret...
		HTURRET GetTurretRecord( ) const { return m_csTurret.m_hTurret; }

		// Retrieve the turret weapon object
		HOBJECT GetTurretWeapon() const { return m_csTurret.m_hTurretWeapon; }

	
	private:

		// Release the player from using this turret...
		void ReleasePlayer();

		// Shutdown any ClientFX currently playing...
		void ShutdownClientFX( );
		
		// Setup the correct damage FX based on the current damage state...
		void SetDamageState( );

	private : // Members...

		TURRETCREATESTRUCT m_csTurret;

		// ClientFX played while operating a turret...
		CClientFXLink	m_fxLoop;

		// ClientFX played when damaged...
		CClientFXLink	m_fxDamage;

		// List of FX cleared and created when entering a new damage state...
		CClientFXLinkNode	m_DamageStateFX;

		// FX in this list are not cleared when entering a new damage state...
		CClientFXLinkNode	m_PersistentDamageStateFX;
};

#endif // __TURRETFX_H__