// ----------------------------------------------------------------------- //
//
// MODULE  : DoomsDayDevice.h
//
// PURPOSE : The object used to represent the doomsday device.
//
// CREATED : 12/19/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DOOMS_DAY_DEVICE_H__
#define __DOOMS_DAY_DEVICE_H__

//
// Includes...
//

#include "PropType.h"
#include "Timer.h"

class CPlayerObj;
class DoomsDayPiece;
	
LINKTO_MODULE( DoomsDayDevice );

class DoomsDayDevice : public PropType 
{
	public: // Methods...

		DoomsDayDevice( );
		~DoomsDayDevice( );

		// Fires the device.
		bool	Fire( CPlayerObj& playerObj );
		
		
		uint8	GetTeamID( )			const { return m_nOwningTeamID; }
		float	GetDropZoneRadius( )	const { return m_fDropZoneRadius; }

		bool	AddDoomsDayPiece( DoomsDayPiece *pDDPiece, CPlayerObj *pPlayer );
		bool	RemoveDoomsDayPiece( DoomsDayPiece *pDDPiece, CPlayerObj* pPlayer );

		typedef std::vector< DoomsDayDevice* > DoomsDayDeviceList;
		static DoomsDayDeviceList const& GetDoomsDayDeviceList( ) { return m_lstDoomsDayDevices; }

		typedef std::vector< DoomsDayPiece* > DoomsDayPieceList;
		DoomsDayPieceList const& GetDoomsDayPieceList( ) const { return m_lstPiecesOnDevice; }

	protected: // Methods... 

        uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

		bool	OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );

	private:

		void	InitialUpdate( );
		void	Update( );

		bool	BeginIdle( );
		void	UpdateIdle( );

		bool	BeginFire( );
		void	UpdateFire( );

		bool	BeginEnd( );
		void	UpdateEnd( );
		

	private:

		bool	ReadProp( ObjectCreateStruct* pStruct );

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// Called to give damage to victim teams.
		bool	DamageVictims( bool& bAllDead );

		enum DoomsDayDeviceState
		{
			kDoomsDayDeviceState_Idle,
			kDoomsDayDeviceState_Fire,
			kDoomsDayDeviceState_End,
		};

		// Current state of the device.
		DoomsDayDeviceState		m_eDoomsDayDeviceState;

		// Time to stay in fire state.
		CTimer					m_Timer;

		// Team that owns this device.
		uint8					m_nOwningTeamID;

		// Radius of the area considered the 'drop zone'. 
		// When a piece is dropped within this radius it will be aded to the device.
		float					m_fDropZoneRadius;
		
		typedef std::vector< LTObjRef >	TargetList;
		TargetList	m_lsthTargets;
		
		// List of all DoomsDayDevices within the game.
		static DoomsDayDeviceList		m_lstDoomsDayDevices;

		DoomsDayPieceList	m_lstPiecesOnDevice;
};

class CDoomsDayDevicePlugin : public CPropTypePlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList( 
			const char *szRezPath,
			const char *szPropName,
			char **aszStrings,
			uint32 *pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLen );

		virtual LTRESULT PreHook_Dims(
			const char *szRezPath,
			const char *szPropValue,
			char *szModelFilenameBuf,
			int nModelFilenameBufLen,
			LTVector &vDims );
};

#endif // __DOOMS_DAY_DEVICE_H__
