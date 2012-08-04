// ****************************************************************************************** //
//
// MODULE  : HeadBobMgr.h
//
// PURPOSE : Definition of Head Bob Manager
//
// CREATED : 08/23/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ****************************************************************************************** //

#ifndef __HEADBOBMGR_H__
#define __HEADBOBMGR_H__

// ****************************************************************************************** //
// The cycle direction callback will allow you to determine when the cycle changes directions.
// The callback is called each time cycle reaches the -1 min extent or the +1 max extent.  The
// value for 'bMaxExtent' will be true at +1 and false at -1.

typedef void ( *HeadBobCycleNotifyFn )( bool bMaxExtent, void* pUserData, bool bApproachFromCenter );

// ****************************************************************************************** //

class HeadBobMgr
{
	public:

		// ------------------------------------------------------------------------------------------ //
		// Construction / destruction

		HeadBobMgr();
		~HeadBobMgr();

		bool			Initialize();
		void			Release();
		void			OnEnterWorld();


		// ------------------------------------------------------------------------------------------ //
		// General updating

		void			Update( float fFrameTime );


		// ------------------------------------------------------------------------------------------ //
		// Parameter control interfaces

		// The record settings functions can be called per frame if necessary, because if the records
		// end up being the same, then no addition settings take affect.
		void			SetRecord( HRECORD hRecord, float fTransitionTime );
		void			SetRecord( const char* sRecord, float fTransitionTime );
		HRECORD			GetRecord() const { return m_hRecord; }

		// Sets the amount to cycle per second.  The cycle is a -1 to +1 range, so an entire cycle per second
		// would be considered a value of 2 for this setting.
		void			SetSpeed( float fSpeed );
		float			GetSpeed() const;

		// Overall scale for the head bob effects... this can be set from external sources
		// for modifying game options or zoom dampening.
		void			SetScale( float fScale );
		float			GetScale() const;

		// Sets the full cycle direction callback function
		void			SetCycleNotifyFn( HeadBobCycleNotifyFn pFn, void* pUserData );

		// Sets the custom cycle notify function.  The range value ( 0.0f to 1.0f ) determines when the
		// callback is used. A value of 0.3f will cause the callback to execute when the cycle reaches
		// one-third to the right and again one-third to the left.
		void			SetCustomCycleNotifyFn( HeadBobCycleNotifyFn pFn, void* pUserData, float fRange );


		// ------------------------------------------------------------------------------------------ //
		// Query interfaces

		LTVector		GetCameraOffsets() const;
		LTVector		GetCameraRotations() const;
		LTVector		GetWeaponOffsets() const;
		LTVector		GetWeaponRotations() const;


		// ------------------------------------------------------------------------------------------ //
		// State control interfaces

		void			Reset();


	public:

		// ------------------------------------------------------------------------------------------ //
		// Internal types

		// Represents parameters for an individual element of the head bob effect
		struct HeadBobElementData
		{
			HeadBobElementData()
			{
				Init();
			}

			void Init()
			{
				m_vWave.Init();
				m_vAmplitude.Init();
				m_nFlags = 0;
			}

			enum Flags
			{
				HBEDF_LINEAR			= 0x0001,
				HBEDF_SINE				= 0x0002,
			};

			LTVector2	m_vWave;
			LTVector2	m_vAmplitude;
			uint32		m_nFlags;
		};

		// The types of elements that we'll be modifying
		enum HeadBobElement
		{
			HBE_CAMERAOFFSET_X		= 0,
			HBE_CAMERAOFFSET_Y,
			HBE_CAMERAOFFSET_Z,
			HBE_CAMERAROTATION_X,
			HBE_CAMERAROTATION_Y,
			HBE_CAMERAROTATION_Z,

			HBE_WEAPONOFFSET_X,
			HBE_WEAPONOFFSET_Y,
			HBE_WEAPONOFFSET_Z,
			HBE_WEAPONROTATION_X,
			HBE_WEAPONROTATION_Y,
			HBE_WEAPONROTATION_Z,

			HBE_COUNT,
			HBE_FORCE32BIT			= 0x7FFFFFFF,
		};


	protected:

		// ------------------------------------------------------------------------------------------ //
		// Helper interfaces

		void			FillHeadBobElementData( HRECORD hRecord, HeadBobElementData* pData );


	protected:

		// Record which represent the currently active head bob parameters.
		HRECORD			m_hRecord;

		// Cycle tracking variables.
		float			m_fSpeed;
		float			m_fScale;
		float			m_fCyclePosition;
		float			m_fCycleDirection;

		// Time stamps for tracking effect behaviors.
		float			m_fTotalTransitionTime;
		float			m_fTransitionTime;

		// The sets for storing ramp and active parameter data.
		float				m_fSpeedScale;
		HeadBobElementData	m_aHBDRecord[ HBE_COUNT ];

		// The current amplitude values while running the effect.
		float				m_aAmpsTransFrom[ HBE_COUNT ];
		float				m_aAmpsActive[ HBE_COUNT ];

		// The cycle callback function
		HeadBobCycleNotifyFn	m_pCycleNotifyFn;
		void*					m_pCycleNotifyUserData;

		// Custom cycle callback function
		HeadBobCycleNotifyFn	m_pCustomCycleNotifyFn;
		void*					m_pCustomCycleNotifyUserData;
		float					m_fCustomCycleNotifyRange;
};

// ****************************************************************************************** //

#endif//__HEADBOBMGR_H__

