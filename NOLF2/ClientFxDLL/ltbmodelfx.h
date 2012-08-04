//------------------------------------------------------------------
//
//   MODULE  : LTBMODELFX.H
//
//   PURPOSE : Defines class CLTBModelFX
//
//   CREATED : On 12/3/98 At 6:34:45 PM
//
//------------------------------------------------------------------

#ifndef __LTBMODELFX__H_
	#define __LTBMODELFX__H_

	// Includes....

	#include "basefx.h"

	class CLTBModelProps : public CBaseFXProps
	{
	public:

		CLTBModelProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		char							m_szModelName[MAX_CS_FILENAME_LEN+1];
		char							m_szSkinName[5][MAX_CS_FILENAME_LEN+1];
		char							m_szRenderStyle[4][MAX_CS_FILENAME_LEN+1];
		char							m_szAnimName[MAX_CS_FILENAME_LEN+1];
		LTVector						m_vNorm;
		uint8							m_nFacing;
		LTBOOL							m_bShadow;
		LTBOOL							m_bOverrideAniLength;
		LTBOOL							m_bSyncToModelAnim;
		LTBOOL							m_bSyncToKey;
		LTFLOAT							m_fAniLength;
		LTBOOL							m_bRotate;
	};



	class CLTBModelFX : public CBaseFX
	{
		public :

			// Constuctor

											CLTBModelFX();

			// Destructor

										   ~CLTBModelFX();

			// Member Functions

			bool							Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
			bool							Update(float tmCur);
			void							Term();

			virtual bool					IsFinishedShuttingDown(); 

			// Accessors

		protected :

			const CLTBModelProps*			GetProps() { return (const CLTBModelProps*)m_pProps; }

			// Member Variables
			LTRotation						m_rRot;
			float							m_fAniRate;
			LTRotation						m_rNormalRot;
	};

#endif