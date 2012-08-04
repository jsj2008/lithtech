//------------------------------------------------------------------
//
//   MODULE  : PLAYSOUNDFX.H
//
//   PURPOSE : Defines class CPlaySoundFX
//
//   CREATED : On 12/15/98 At 5:06:02 PM
//
//------------------------------------------------------------------

#ifndef __PLAYSOUNDFX__H_
	#define __PLAYSOUNDFX__H_

	// Includes....

	#include "basefx.h"

	class CPlaySoundProps : public CBaseFXProps
	{
	public:

		CPlaySoundProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		char							m_sSoundName[128];
		int								m_nPriority;
		LTFLOAT							m_fOuterRadius;
		LTFLOAT							m_fInnerRadius;
		LTBOOL							m_bLoop;
		LTBOOL							m_bPlayLocal;
		int								m_nVolume;
		LTFLOAT							m_fPitch;
	};


	class CPlaySoundFX : public CBaseFX
	{
		public :

			// Constuctor

											CPlaySoundFX();

			// Destructor

										   ~CPlaySoundFX();
			// Member Functions

			bool							Init(ILTClient *pClientDE, FX_BASEDATA *pData, const CBaseFXProps *pProps);
			void							Term();
			bool							Update(float tmCur);
			bool							SuspendedUpdate(float tmFrameTime);

			// Accessors

		protected :

			const CPlaySoundProps*			GetProps()		{ return (const CPlaySoundProps*)m_pProps; }


			// Member Variables

			HLTSOUND						m_hSound;
			bool							m_bFirstUpdate;

			void							PlaySound();
	};

#endif