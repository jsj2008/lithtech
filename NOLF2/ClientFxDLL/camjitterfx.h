// ----------------------------------------------------------------------- //
//
// MODULE  : CamJitterFX.cpp
//
// PURPOSE : The CamJitterFX object
//
// CREATED : 12/30/98
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CAMJITTERFX__H_
	#define __CAMJITTERFX__H_

	// Includes....

	#include "basefx.h"

	class CCamJitterProps : public CBaseFXProps
	{
	public:

		CCamJitterProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		LTFLOAT		m_fInnerDistSqrd;
		LTFLOAT		m_fOuterDistSqrd;
	};

	class CCamJitterFX : public CBaseFX
	{
		public : // Methods...

			CCamJitterFX();
		   ~CCamJitterFX();

			bool	Init(ILTClient *pClientDE, FX_BASEDATA *pData, const CBaseFXProps *pProps);
			void	Term();
			bool	Update(float tmCur);

		protected:

			const CCamJitterProps*	GetProps()		{ return (const CCamJitterProps*)m_pProps; }
	};

#endif