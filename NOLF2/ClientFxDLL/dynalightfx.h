//------------------------------------------------------------------
//
//   MODULE  : DYNALIGHTFX.H
//
//   PURPOSE : Defines class CDynaLightFX
//
//   CREATED : On 12/14/98 At 5:43:43 PM
//
//------------------------------------------------------------------

#ifndef __DYNALIGHTFX__H_
	#define __DYNALIGHTFX__H_

	// Includes....

	#include "basefx.h"

	class CDynaLightProps : public CBaseFXProps
	{
	public:

		CDynaLightProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		bool		m_bFlicker;
		bool		m_bForceLightWorld;
	};

	class CDynaLightFX : public CBaseFX
	{
		public :

			// Constuctor

											CDynaLightFX();

			// Destructor

										   ~CDynaLightFX();
			// Member Functions

			bool							Init(ILTClient *pClientDE, FX_BASEDATA *pData, const CBaseFXProps *pProps);
			void							Term();
			bool							Update(float tmCur);

			// Accessors

		protected :

			const CDynaLightProps*			GetProps()		{ return (const CDynaLightProps*)m_pProps; }

			// Member Variables
	};

#endif