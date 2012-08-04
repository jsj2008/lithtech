//------------------------------------------------------------------
//
//   MODULE  : NULLFX.H
//
//   PURPOSE : Defines class NullFX
//
//   CREATED : On 12/3/98 At 6:34:45 PM
//
//------------------------------------------------------------------

#ifndef __NULLFX__H_
	#define __NULLFX__H_

	// Includes....

	#include "basefx.h"
	#include "ClientFX.h"

	class CNullProps : public CBaseFXProps
	{
	public:

		CNullProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		LTFLOAT				m_fGravity;
		LTVector			m_vMinVel;
		LTVector			m_vMaxVel;
		LTBOOL				m_bBounce;
	};


	class CNullFX : public CBaseFX
	{
		private: // Members

			const CNullProps*	GetProps()		{ return (const CNullProps*)m_pProps; }

			LTVector			m_vVelocity;
			LTVector			m_vPosition;

		public :
	
			CNullFX();
			~CNullFX();

			// Member Functions

			bool	Init( ILTClient *pLTClient, FX_BASEDATA *pData, const CBaseFXProps *pProps);
			bool	Update( float tmCur );
			void	Term( void );
	};

#endif