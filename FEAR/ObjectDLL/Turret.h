// ----------------------------------------------------------------------- //
//
// MODULE  : Turret.h
//
// PURPOSE : Turrets create a weapon to be used by a player through activation...
//
// CREATED : 07/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TURRET_H__
#define __TURRET_H__

//
// Includes...
//

#include "GameBase.h"
#include "Arsenal.h"
#include "SharedFXStructs.h"
#include "DestructibleModel.h"


LINKTO_MODULE( Turret );

class Turret : public GameBase
{
	public: // Methods...

		Turret( );
		virtual ~Turret( );

		// Query to see if someone else is operating the turret...
		bool IsInUse( ) const { return (m_hOperatingObject != NULL); }
		
		// Retrieve the record for the turret...
		HTURRET GetTurretRecord( ) const { return m_hTurret; }

		CWeapon* GetTurretWeapon( ) { return m_Arsenal.GetCurWeapon( ); }
		
		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );
		
	protected: // Methods... 

		// Handle messages from the engine...
		uint32 EngineMessageFn( uint32 messageID, void *pData, float fData );

		// Handle messages from other objects...
		uint32 ObjectMessageFn( HOBJECT hSender, ILTMessage_Read *pMsg );

		void OnLinkBroken( LTObjRefNotifier * pRef, HOBJECT hObj );

		// Send relevant information to clients...
		void CreateSpecialFX( bool bUpdateClients );

        // Enable the sender to operate the turret...
		virtual void Activate( HOBJECT hSender );

		// Stop using the turret...
		virtual void Deactivate( );

		// Allow sub-classes to update the client data before sending...
		virtual void PreCreateSpecialFX( TURRETCREATESTRUCT &rTurretCS ) { }

		// Handle any cleanup required when the turret gets destroyed...
		virtual void OnDeath( );

		// Handle reactivating after loading a saved game...
		virtual void PostLoadActivate( HOBJECT hOperatingObject );

		
	private: // Methods...

		// Handle a MID_INITIALUPDATE message from the engine....
		void InitialUpdate( );

		// Read in the properties of the object... 
		bool ReadProp( const GenericPropList *pProps );
		
		// Update the create struct of the object
		void PostReadProp( ObjectCreateStruct *pStruct );
		
		void Update( );

		// Save the object...
		void Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );

		// Load the object...
		void Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		virtual void	SaveSFXMessage( ILTMessage_Write *pMsg, uint32 dwFlags );
		virtual void	LoadSFXMessage( ILTMessage_Read *pMsg, uint32 dwFlags );

		// Create the actual weapon used for the turret...
		bool CreateWeapon( );

				
	protected: // Members...

		
		// Record of the turret type used to create the turret...
		HTURRET m_hTurret;

		// Needed to create our weapon object...
        CArsenal m_Arsenal;

		// Object that is currently operating the turret...
		LTObjRefNotifier m_hOperatingObject;

		// Damage aggregate...
		CDestructibleModel m_Damage;

		// Teleports the object to the floor once it is created...
		bool m_bMoveToFloor;

		// Commands to process when the turret is activated or deactivated...
		std::string m_sActivateCommand;
		std::string m_sDeactivateCommand;

		// Delay turret deactivation after the turret has been destroyed...
		StopWatchTimer m_swtDestroyedDeactivationDelay;

		uint32 m_nCurDamageState;

		bool m_bPostLoadActivate;


		// Message Handlers...

		DECLARE_MSG_HANDLER( Turret, HandleActivateMsg );
		DECLARE_MSG_HANDLER( Turret, HandleDeactivateMsg );
		DECLARE_MSG_HANDLER( Turret, HandleHiddenMsg );
		DECLARE_MSG_HANDLER( Turret, HandleRemoveMsg );
};

// Plugin class for hooking into the level editor for displaying entries in listboxes and displaying the model...
class TurretPlugin : public IObjectPlugin
{
	public: // Methods...

		virtual LTRESULT PreHook_EditStringList( 
			const char *szRezPath,
			const char *szPropName,
			char **aszStrings,
			uint32 *pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLen );

		virtual LTRESULT PreHook_Dims(
			const char* szRezPath,
			const char* szPropName, 
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims,
			const char* pszObjName, 
			ILTPreInterface *pInterface);

		virtual LTRESULT PreHook_PropChanged( 
			const	char		*szObjName,
			const	char		*szPropName,
			const	int			nPropType,
			const	GenericProp	&gpPropValue,
			ILTPreInterface		*pInterface,
			const	char		*szModifiers );

	private: // Members...

		CDestructibleModelPlugin	m_DestructibleModelPlugin;
		CCommandMgrPlugin			m_CommandMgrPlugin;
};

#endif // __TURRET_H__

// EOF
