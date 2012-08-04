//------------------------------------------------------------------
//
//   MODULE  : CAMWOBBLEFX.H
//
//   PURPOSE : Defines class CCamWobbleFX
//
//   CREATED : On 12/30/98 At 3:28:28 PM
//
//------------------------------------------------------------------

#ifndef __CAMWOBBLEFX__H_
	#define __CAMWOBBLEFX__H_

	// Includes....

	#include "basefx.h"

	class CCamWobbleProps : public CBaseFXProps
	{
	public:

		CCamWobbleProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		LTFLOAT		m_fPeriod;
		LTFLOAT		m_xMultiplier;
		LTFLOAT		m_yMultiplier;
		LTFLOAT		m_fInnerDistSqrd;
		LTFLOAT		m_fOuterDistSqrd;
	};



	class CCamWobbleFX : public CBaseFX
	{
		protected : // Members...

			const CCamWobbleProps*		GetProps() { return (const CCamWobbleProps*)m_pProps; }

			LTFLOAT		m_xFovAnchor;
			LTFLOAT		m_yFovAnchor;

		public : // Methods...

			CCamWobbleFX();
			~CCamWobbleFX();

			bool	Init(ILTClient *pClientDE, FX_BASEDATA *pData, const CBaseFXProps *pProps);
			void	Term();
			bool	Update(float tmCur);
	};

#endif