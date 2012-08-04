//------------------------------------------------------------------
//
//   MODULE  : PLAYRANDOMSOUNDFX.H
//
//   PURPOSE : Defines class CPlayRandomSoundFX
//
//   CREATED : On 12/15/98 At 5:06:02 PM
//
//------------------------------------------------------------------

#ifndef __PLAYRANDOMSOUNDFX__H_
	#define __PLAYRANDOMSOUNDFX__H_

	// Includes....

	#include "basefx.h"

	class CPlayRandomSoundProps : public CBaseFXProps
	{
	public:

		CPlayRandomSoundProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		char							m_sSoundName[128];
		uint32							m_nPriority;
		float							m_fOuterRadius;
		float							m_fInnerRadius;
		LTBOOL							m_bLoop;
		LTBOOL							m_bPlayLocal;
		uint32							m_nVolume;
		uint32							m_nRand;	
	};

	class CPlayRandomSoundFX : public CBaseFX
	{
		public :

			// Constuctor

											CPlayRandomSoundFX();

			// Destructor

										   ~CPlayRandomSoundFX();
			// Member Functions

			bool							Init(ILTClient *pClientDE, FX_BASEDATA *pData, const CBaseFXProps *pProps);
			void							Term();
			bool							Update(float tmCur);
			bool							SuspendedUpdate(float tmFrameTime);

			// Accessors

		protected :

			const CPlayRandomSoundProps*	GetProps()		{ return (const CPlayRandomSoundProps*)m_pProps; }


			// Member Variables

			HLTSOUND						m_hSound;
			bool							m_bFirstUpdate;

			void							PlaySound();
	};

#endif