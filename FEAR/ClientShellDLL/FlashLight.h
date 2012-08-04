// ****************************************************************************************** //
//
// MODULE  : FlashLight.h
//
// PURPOSE : FlashLight class - Definition
//
// CREATED : 09/01/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ****************************************************************************************** //

#ifndef __FLASHLIGHT_H__
#define __FLASHLIGHT_H__

#include "ClientFXSequence.h"

class CVolumetricLightFX;

// ****************************************************************************************** //

#define MAX_FLASHLIGHT_LIGHTS				2

// ****************************************************************************************** //

class Flashlight
{
	public:

		// ------------------------------------------------------------------------------------------ //
		// Construction / destruction

		Flashlight();
		virtual ~Flashlight();

		bool			Initialize( HOBJECT hFollowObject, HRECORD hRecord );
		bool			Initialize( HOBJECT hFollowObject, const char* sRecord );


		// ------------------------------------------------------------------------------------------ //
		// General updating

		void			Update( float fFrameTime );


		// ------------------------------------------------------------------------------------------ //
		// State control

		// Prevents/enables the flashlight from being turned on...
		void			Enable( bool bEnable, float fFlickerDuration );

		// Turns on/off the flashlight...
		bool			TurnOn();
		bool			TurnOff();

		// Plays a special effect...
		void			PlayFlicker( float fDuration, bool bTemporary );
		void			PlayShake( float fDuration, float fStrength, float fSpeed );
		void			PlaySpecialSound( uint32 nIndex );

		// Affects the speed at which the waver pattern gets played...
		void			SetWaverSpeedScale( float fScale );


		// ------------------------------------------------------------------------------------------ //
		// Query interfaces

		bool			IsOn() const			{ return m_bOn; }
		bool			IsEnabled() const		{ return m_bEnabled; }

		bool			IsRecordType( HRECORD hRecord );
		bool			IsRecordType( const char* sRecord );

		LTRotation		GetLagDelta()			{ return m_rLagDelta; }

		float			GetMaxCharge() const	{ return m_fMaxCharge; }
		float			GetRechargeRate() const	{ return m_fRechargeRate; }


		// ------------------------------------------------------------------------------------------ //
		// Save / Load

		void			Save( ILTMessage_Write* pMsg, SaveDataState eSaveDataState );
		void			Load( ILTMessage_Read* pMsg, SaveDataState eLoadDataState );


	protected:

		// ------------------------------------------------------------------------------------------ //
		// Internal helper interfaces

		void			Helper_CreateLight();
		void			Helper_SetLightProperties();
		void			Helper_GetTransform( LTRigidTransform& iTrans );
		void			Helper_OffsetTranform( LTRigidTransform& iTrans, uint32 nLight );

		void			Update_Waver( LTRotation& rOffset, float fFrameTime );
		void			Update_Flicker( float fFrameTime );
		void			Update_Shake( LTRotation& rOffset, float fFrameTime );


	protected:

		// ------------------------------------------------------------------------------------------ //
		// Internal data structures

		// Holds information about an individual light
		struct LightData
		{
			LightData() { Init(); }

			void Init()
			{
				m_hLight = NULL;
				m_pVolumetrictLight = NULL;
				m_bCreateLight = true;
				m_sLightTexture = NULL;
				m_sLightType = NULL;
				m_vLightRadiusRange.Init();
				m_nLightColor = 0xFFFFFFFF;
				m_vLightPositionOffset.Init();
				m_vLightRotationOffset.Init();
				m_vLightIntensity.Init();
				m_vLightFOV.Init();
				m_sLightShadowLOD = NULL;
				m_eVolumetricLOD = eEngineLOD_Never;
			}

			HOBJECT					m_hLight;
			CVolumetricLightFX*		m_pVolumetrictLight;
			bool					m_bCreateLight;
			const char*				m_sLightTexture;
			const char*				m_sLightType;
			LTVector2				m_vLightRadiusRange;
			int32					m_nLightColor;
			LTVector				m_vLightPositionOffset;
			LTVector				m_vLightRotationOffset;
			LTVector2				m_vLightIntensity;
			LTVector2				m_vLightFOV;
			const char*				m_sLightShadowLOD;
			EEngineLOD				m_eVolumetricLOD;
			float					m_fVolumetricNoiseIntensity;
			float					m_fVolumetricNoiseScale;
			int32					m_nVolumetricColor;
			const char*				m_sVolumetricTexture;
			float					m_fVolumetricAttenuation;
			bool					m_bVolumetricAdditive;
			float					m_fVolumetricDepth;
			bool					m_bVolumetricShadow;
		};


	private:

		// Object that the light is associated with
		LTObjRef		m_hFollowObject;
		HRECORD			m_hRecord;

		// State variables
        bool			m_bOn;
		bool			m_bWasOn;
		bool			m_bEnabled;

		// Parameters gathered from the database
		LightData		m_aLights[ MAX_FLASHLIGHT_LIGHTS ];
		uint32			m_nLights;

		const char*		m_sSocket;
		bool			m_bSendOnOffStateToServer;

		bool			m_bTemporaryFlicker;
		bool			m_bAlwaysFlicker;
		LTVector2		m_vFlickerInterval;
		LTVector2		m_vFlickerDuration;

		bool			m_bLag;
		float			m_fLagCorrectionSpeed;
		bool			m_bLagInverseDirection;

		bool			m_bWaver;
		float			m_fWaverPerimeterSize;
		float			m_fWaverTangentLength;
		float			m_fWaverSpeed;
		float			m_fWaverSpeedScale;

		LTVector2		m_vShakeWaveRange;

		// Runtime updating variables
		int32			m_nFlickerPattern;
		float			m_fFlickerIntervalRemaining;
		float			m_fFlickerDurationTotal;
		float			m_fFlickerDurationRemaining;
		bool			m_bFlickerOut;

		LTRotation		m_rLagDelta;
		LTRotation		m_rLagTracker;

		LTVector		m_vWaverPathPt1;
		LTVector		m_vWaverPathPt2;
		LTVector		m_vWaverPathPt3;
		LTVector		m_vWaverPathPt4;
		float			m_fWaverDurationTotal;
		float			m_fWaverDurationRemaining;

		float			m_fShakeStrength;
		LTVector2		m_vShakeWaveLengths;
		float			m_fShakeDurationTotal;
		float			m_fShakeDurationRemaining;

		CClientFXSequence m_fxFlashlightSequence;

		float			m_fMaxCharge;
		float			m_fRechargeRate;
};

// ****************************************************************************************** //

#endif//__FLASHLIGHT_H__

