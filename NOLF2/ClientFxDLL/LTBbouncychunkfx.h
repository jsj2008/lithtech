//------------------------------------------------------------------
//
//   MODULE  : LTBBOUNCYCHUNKFX.H
//
//   PURPOSE : Defines class CLTBBouncyChunkFX
//
//   CREATED : On 12/3/98 At 6:34:45 PM
//
//------------------------------------------------------------------

#ifndef __LTBBOUNCYCHUNKFX__H_
	#define __LTBBOUNCYCHUNKFX__H_

	// Includes....

	#include "basefx.h"

	class CLTBBouncyChunkProps : public CBaseFXProps
	{
	public:
	
		CLTBBouncyChunkProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		char							m_sModelName[128];
		char							m_sSkinName[128];
		
		bool							m_bPlayImpactSound;
		char							m_sImpactSound[128];

		LTVector						m_vGravity;
		float							m_fGravityAmount;

		float							m_fChunkSpeed;
		float							m_fChunkSpread;

		LTVector						m_vChunkDir;		
	};

	class CLTBBouncyChunkFX : public CBaseFX
	{
		public :

			// Constuctor

											CLTBBouncyChunkFX();

			// Destructor

										   ~CLTBBouncyChunkFX();

			// Member Functions

			bool							Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
			bool							Update(float tmCur);
			void							ReadProps( CLinkList<FX_PROP> *pProps );
			void							Term();

			// Accessors

		protected :

			const CLTBBouncyChunkProps*		GetProps()		{ return (const CLTBBouncyChunkProps*)m_pProps; }

			// Member Variables

			HOBJECT							m_hBouncyChunk;
			HLTSOUND						m_hImpactSound;

			LTVector						m_vVel;
	};

#endif