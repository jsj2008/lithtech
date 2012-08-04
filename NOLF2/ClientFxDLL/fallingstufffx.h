//------------------------------------------------------------------
//
//   MODULE  : FALLINGSTUFFFX.H
//
//   PURPOSE : Defines class CFallingStuffFX
//
//   CREATED : On 10/26/98 At 4:00:00 PM
//
//------------------------------------------------------------------

#ifndef __FALLINGSTUFFFX__H_
	#define __FALLINGSTUFFFX__H_

	// Includes....

	#include "basefx.h"
	#include "linklist.h"
	#include "spritesystemfx.h"

	// Structures....

	struct FALLING_THING
	{
		float								m_tmElapsed;
		LTVector							m_vLastPos;
		LTVector							m_vPos;
		LTVector							m_vVel;
		HLOCALOBJ							m_hObject;
		bool								m_bSplash;
	};

	struct SPLASH 
	{
		HOBJECT								m_hObject;
		float								m_tmElapsed;
		float								m_scale;
	};

	// Defines....

	#define FS_PLANEUP						3

	#define FSPT_NONE						0
	#define FSPT_SINE						1
	#define FSPT_PENDULUM					2

	// Classes....

	class CFallingStuffProps : public CBaseFXProps
	{
	public:

		CFallingStuffProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		char							m_sSpriteName[128];
		float							m_fRadius;
		uint32							m_nFallingStuffFXEmission;
		float							m_tmFallingStuffFXEmission;
		float							m_tmSpriteLifespan;
		LTVector						m_vPlaneDir;
		float							m_fVel;
		LTVector						m_vWind;
		float							m_fWindAmount;
		float							m_fStretchMul;
		char							m_sImpactSpriteName[128];
		float							m_tmImpactLifespan;
		float							m_fImpactScale1;
		float							m_fImpactScale2;
		int								m_nImpactCreate;
		bool							m_bUseSpin;
	};

	class CFallingStuffFX : public CBaseFX
	{
		public :

			// Constuctor

											CFallingStuffFX();

			// Destructor

										   ~CFallingStuffFX();

			// Member Functions

			bool							Init(ILTClient *pClientDE, FX_BASEDATA *pData, const CBaseFXProps *pProps);
			void							Term();
			bool							Update(float tmCur);
			bool							IsFinishedShuttingDown() { return (m_collSprites.GetSize() == 0) ? true : false; }

			// Accessors

		protected :

			const CFallingStuffProps*		GetProps()	{ return (const CFallingStuffProps*)m_pProps; }

			// Member Variables
			CLinkList<FALLING_THING *>		m_collSprites;
			float							m_tmElapsedEmission;
			float							m_xRot;
			float							m_yRot;
			float							m_zRot;
			CLinkList<SPLASH *>				m_collSplashes;
			int								m_nImpactPerturbType;
			LTVector						m_vPlaneDir;
			LTVector						m_vUp;
			LTVector						m_vRight;
			LTVector						m_vLastPos;
	};

#endif