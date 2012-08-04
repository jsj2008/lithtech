// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerLure.h
//
// PURPOSE : PlayerLure - Definition
//
// CREATED : 4/21/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_LURE_H__
#define __PLAYER_LURE_H__

#include "ltengineobjects.h"
#include "GameBase.h"
#include "SharedMovement.h"
#include "SFXMsgIds.h"

LINKTO_MODULE( PlayerLure );

class PlayerLure : public GameBase
{
	public :

		PlayerLure( );

		bool WriteVehicleMessage( ILTMessage_Write& msg );

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, float lData);

	private :

		void ReadProp(const GenericPropList *pProps);
		void InitialUpdate( );

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// What freedom of rotation the camera has.
		PlayerLureCameraFreedom	m_eCameraFreedom;

		// The range for limited camera.
		float	m_fLimitedYawLeft;
		float	m_fLimitedYawRight;
		float	m_fLimitedPitchDown;
		float	m_fLimitedPitchUp;

		// Allow weapon to be up during lure.
		bool	m_bAllowWeapon;
		bool	m_bAllowSwitchWeapon;

		bool	m_bAllowLean;
		bool	m_bAllowCrouch;
		bool	m_bAllowBodyRotation;
		bool	m_bTrackYaw;
		bool	m_bTrackPitch;

		// Player retains original offsets from playerlure.
		bool	m_bRetainOffsets;

		// ClientFX to play when we die while on lure.
		std::string		m_sDeathFX;

		// Our playerlure id.
		uint32			m_nPlayerLureId;

		// Next playerlure id.
		static uint32	s_nNextPlayerLureId;
};


#endif // __PLAYER_LURE_H__
