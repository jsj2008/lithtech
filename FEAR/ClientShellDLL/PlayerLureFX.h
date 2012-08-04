// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerLureFX.h
//
// PURPOSE : PlayerLureFX
//
// CREATED : 01/28/02
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYERLUREFX_H__
#define __PLAYERLUREFX_H__

#include "SpecialFX.h"
#include "ClientServerShared.h"


class PlayerLureFX : public CSpecialFX
{
	public :

		PlayerLureFX();
		~PlayerLureFX();

        virtual bool	Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual bool	Init(SFXCREATESTRUCT* /*psfxCreateStruct*/) { ASSERT( !"Not supported." ); return FALSE; }

		virtual uint32	GetSFXID() { return SFX_PLAYERLURE_ID; }

		// Resets the lure when the player is commanded to follow it.
		bool			Reset( );
		bool			Reset( const LTRigidTransform &offset ) { m_offsetTransform = offset; return true; }

		PlayerLureCameraFreedom GetCameraFreedom( ) const { return m_eCameraFreedom; }
		bool			GetAllowWeapon( ) const { return m_bAllowWeapon; }
		bool			GetAllowSwitchWeapon( ) const { return m_bAllowSwitchWeapon; }
		bool			GetAllowLean( ) const { return m_bAllowLean; }
		bool			GetAllowCrouch( ) const { return m_bAllowCrouch; }
		bool			GetAllowBodyRotation( ) const { return m_bAllowBodyRotation; }			
		bool			GetTrackPitch( ) const { return m_bTrackPitch; }
		bool			GetTrackYaw( ) const { return m_bTrackYaw; }
		bool			GetRetainOffsets( ) const { return m_bRetainOffsets; }
		uint32			GetPlayerLureId( ) const { return m_nPlayerLureId; }

		void			GetLimitedRanges( float& fLimitedYawLeft, float& fLimitedYawRight, 
							float& fLimitedPitchDown, float& fLimitedPitchUp ) const
		{
			fLimitedYawLeft = m_fLimitedYawLeft; 
			fLimitedYawRight = m_fLimitedYawRight; 
			fLimitedPitchDown = m_fLimitedPitchDown; 
			fLimitedPitchUp = m_fLimitedPitchUp; 
		}

		// Gets the offset to be used when following the lure.
		LTRigidTransform const&	GetOffsetTransform( ) const { return m_offsetTransform; }

		char const*		GetDeathFX( ) const { return m_sDeathFX.c_str( ); }

		// Get a playerlurefx object based on a playerlureid from the server.  The HOBJECT
		// may not be on the client when the player is first told to followlure, but should
		// sometime the same frame.
		static PlayerLureFX* GetPlayerLureFX( uint32 nPlayerLureId );

	private :

		PlayerLureCameraFreedom	m_eCameraFreedom;
		bool					m_bAllowWeapon;
		bool					m_bAllowSwitchWeapon;
		bool					m_bAllowLean;
		bool					m_bAllowCrouch;
		bool					m_bAllowBodyRotation;
		bool					m_bTrackPitch;
		bool					m_bTrackYaw;
		bool					m_bRetainOffsets;
		DWORD					m_nPlayerLureId;
		LTRigidTransform		m_offsetTransform;

		bool					m_bCalcInitialOffset;

		float					m_fLimitedYawLeft;
		float					m_fLimitedYawRight;
		float					m_fLimitedPitchDown;
		float					m_fLimitedPitchUp;

		// ClientFX to play if we die while using lure.
		std::string				m_sDeathFX;

		typedef std::vector< PlayerLureFX*, LTAllocator<PlayerLureFX*, LT_MEM_TYPE_CLIENTSHELL> > PlayerLureFXList;
		static PlayerLureFXList	m_lstPlayerLureFXs;
};


#endif // __PLAYERLUREFX_H__