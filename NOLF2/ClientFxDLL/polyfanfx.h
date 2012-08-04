//------------------------------------------------------------------
//
//   MODULE  : POLYFANFX.H
//
//   PURPOSE : Defines class CPolyFanFX
//
//   CREATED : On 11/23/98 At 6:21:38 PM
//
//------------------------------------------------------------------

#ifndef __POLYFANFX__H_
	#define __POLYFANFX__H_

	// Includes....

	#include "basefx.h"

	class CPolyFanProps : public CBaseFXProps
	{
	public:

		CPolyFanProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		char							m_sPolyFanName[128];
		uint32							m_nAlongNormal;
		bool							m_bParentRotate;
		float							m_fYaw;
		LTVector						m_vRot;
	};



	class CPolyFanFX : public CBaseFX
	{
		public :

			// Constuctor

											CPolyFanFX();

			// Destructor

										   ~CPolyFanFX();

			// Member Functions

			bool							Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
			bool							Update(float tmCur);
			void							Term();

			// Accessors

		protected :

			const CPolyFanProps*			GetProps() { return (const CPolyFanProps*)m_pProps; }

			// Member Variables

	};

#endif