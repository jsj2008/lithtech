// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerVehicle.h
//
// PURPOSE : An PlayerVehicle object
//
// CREATED : 8/31/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_VEHICLE_H__
#define __PLAYER_VEHICLE_H__

#include "ltengineobjects.h"
#include "Prop.h"
#include "Timer.h"
#include "SharedMovement.h"
#include "SharedFXStructs.h"

LINKTO_MODULE( PlayerVehicle );

class PlayerVehicle : public Prop
{
	public :

		PlayerVehicle();
		~PlayerVehicle();

		void	SetRidden(bool bRidden);

		PlayerPhysicsModel GetPhysicsModel() const { return m_ePPhysicsModel; }

		// Returns last time someone rode this vehicle.
		float	GetLastRideTime( ) { return m_fLastRideTime; }

		// Indicates if this vehicle has ever been ridden.
		bool	IsVirgin( ) { return m_bVirgin; }

		// Get the complete list of all bodies.
		typedef std::vector< PlayerVehicle* > PlayerVehicleList;
		static PlayerVehicleList const& GetPlayerVehicleList( ) { return m_lstPlayerVehicles; }

	protected :

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void	ReadProp(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);

		// Limits number of vehicles allowed in level.
		void	CapNumberOfVehicles( );

	private :

		void	InitialUpdate();
		void	Update();
		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);
		void	DoActivate(HOBJECT hSender);
		void	CreateSFXMsg();
		void	Respawn();

	private :

		CTimer		m_RespawnTimer;
        LTFLOAT     m_fRespawnTime;

		LTVector    m_vOriginalDims;
        LTVector    m_vOriginalPos;
        LTRotation  m_rOriginalRot;
		bool		m_bRidden;
		
		bool		m_bLocked;
		HSTRING		m_hstrLockedCommand;

		PlayerPhysicsModel m_ePPhysicsModel;  // Corresponds to vehicle type

		PVCREATESTRUCT	m_PlayerVehicleStruct;

		uint32		m_dwSavedFlags;

		float		m_fLastRideTime;
		bool		m_bVirgin;

		static PlayerVehicleList		m_lstPlayerVehicles;
};


class CPlayerVehiclePlugin : public CPropPlugin
{
  	public: // Methods...

		virtual LTRESULT PreHook_EditStringList( const char *szRezPath,
												 const char *szPropName,
												 char **aszStrings,
												 uint32 *pcStrings,
												 const uint32 cMaxStrings,
												 const uint32 cMaxStringLength );
		
		virtual LTRESULT PreHook_PropChanged( const	char		*szObjName,
											  const	char		*szPropName,
											  const	int			nPropType,
											  const	GenericProp	&gpPropValue,
													ILTPreInterface	*pInterface,
											  const char		*szModifiers );

	protected: // Members...

		CCommandMgrPlugin	m_CommandMgrPlugin;
};


#endif // __PLAYER_VEHICLE_H__