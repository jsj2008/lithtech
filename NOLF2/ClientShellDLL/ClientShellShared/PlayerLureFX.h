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

        virtual LTBOOL	Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL	Init(SFXCREATESTRUCT* psfxCreateStruct) { ASSERT( !"Not supported." ); return FALSE; }

		virtual uint32	GetSFXID() { return SFX_PLAYERLURE_ID; }

		// Resets the lure when the player is commanded to follow it.
		bool			Reset( );

		PlayerLureCameraFreedom GetCameraFreedom( ) { return m_eCameraFreedom; }
		bool			GetAllowWeapon( ) { return m_bAllowWeapon; }
		bool			GetRetainOffsets( ) { return m_bRetainOffsets; }
		uint32			GetPlayerLureId( ) { return m_nPlayerLureId; }
		bool			GetBicycle( ) { return m_bBicycle; }

		void			GetLimitedRanges( float& fLimitedYawLeft, float& fLimitedYawRight, float& fLimitedPitchDown, float& fLimitedPitchUp )
		{
			fLimitedYawLeft = m_fLimitedYawLeft; 
			fLimitedYawRight = m_fLimitedYawRight; 
			fLimitedPitchDown = m_fLimitedPitchDown; 
			fLimitedPitchUp = m_fLimitedPitchUp; 
		}

		// Gets the offset to be used when following the lure.
		LTransform const&	GetOffsetTransform( ) { return m_offsetTransform; }

		char const*		GetDeathFX( ) { return m_sDeathFX.c_str( ); }

		// Get a playerlurefx object based on a playerlureid from the server.  The HOBJECT
		// may not be on the client when the player is first told to followlure, but should
		// sometime the same frame.
		static PlayerLureFX* GetPlayerLureFX( uint32 nPlayerLureId );

	private :

		PlayerLureCameraFreedom	m_eCameraFreedom;
		bool					m_bAllowWeapon;
		bool					m_bRetainOffsets;
		bool					m_bBicycle;
		DWORD					m_nPlayerLureId;
		LTransform				m_offsetTransform;

		bool					m_bCalcInitialOffset;

		float					m_fLimitedYawLeft;
		float					m_fLimitedYawRight;
		float					m_fLimitedPitchDown;
		float					m_fLimitedPitchUp;

		// ClientFX to play if we die while using lure.
		std::string				m_sDeathFX;

		typedef std::vector< PlayerLureFX* > PlayerLureFXList;
		static PlayerLureFXList	m_lstPlayerLureFXs;
};


#endif // __PLAYERLUREFX_H__