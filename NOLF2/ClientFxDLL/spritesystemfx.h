//------------------------------------------------------------------
//
//   MODULE  : SPRITESYSTEM.H
//
//   PURPOSE : Defines class CSpriteSystem
//
//   CREATED : On 10/26/98 At 4:00:00 PM
//
//------------------------------------------------------------------

#ifndef __SPRITESYSTEM__H_
	#define __SPRITESYSTEM__H_

	// Includes....

	#include "basefx.h"
	#include "linklist.h"
	#include "fastlist.h"
	#include "swaplist.h"

	// Structures....

	struct SPRITE
	{
		LTFLOAT						m_tmElapsed;
		LTFLOAT						m_fLifespan;
		LTVector					m_vPos;
		LTVector					m_vVel;
		LTVector					m_vRotAdd;
		LTRotation					m_rRot;
	};

	// Defines....

	enum
	{
		SS_SPHERE,
		SS_POINT,
		SS_CIRCLE,
		SS_CONE,
		SS_CYLINDER
	};


	enum
	{
		SS_ANIMLEN_SPRITEDEFAULT,
		SS_ANIMLEN_PARTICLE,
		SS_ANIMLEN_KEY
	};

	// Classes....

	class CSpriteSystemProps : public CBaseFXProps
	{
	public:

		CSpriteSystemProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		uint32							m_nSpritesPerEmission;
		uint32							m_nAnimLengthType;
		LTFLOAT							m_fAnimSpeed;
		LTFLOAT							m_fEmissionInterval;
		LTFLOAT							m_fGravity;
		LTFLOAT							m_fMinSpriteLifespan;
		LTFLOAT							m_fMaxSpriteLifespan;
		uint32							m_nEmissionType;
		LTFLOAT							m_fMinRadius;
		LTFLOAT							m_fMaxRadius;
		LTVector						m_vPlaneDir;
		LTVector						m_vPerp1;		
		LTVector						m_vPerp2;				
		LTVector						m_vMinSpriteVelocity;
		LTVector						m_vMaxSpriteVelocity;
		LTVector						m_vMinSpriteRotation;
		LTVector						m_vMaxSpriteRotation;
		LTBOOL							m_bInfiniteLife;
		uint32							m_nAlphaType;
		LTFLOAT							m_fStretchU;
		LTFLOAT							m_fStretchV;
		char							m_szFileName[128];
		LTBOOL							m_bUseSpin;
	};


	class CSpriteSystem : public CBaseFX
	{
		public :

			// Constuctor

											CSpriteSystem();

			// Destructor

										   ~CSpriteSystem();

			// Member Functions

			bool							Init(ILTClient *pClientDE, FX_BASEDATA *pData, const CBaseFXProps *pProps);
			void							Term();
			bool							Update(float tmFrameTime);
			bool							SuspendedUpdate(float tmFrameTime);
			bool							Render();
			bool							IsVisibleWhileSuspended()	{ return m_collSprites.GetUsed() != 0; }
			bool							IsFinishedShuttingDown()	{ return m_collSprites.GetUsed() == 0; }

		protected:

			void	AddSprites( );
			void	UpdateSprites( LTFLOAT tmFrameTime );
			void	SetupParticle(const LTVector& vPos, float fWidth, float fHeight, const LTRotation& rSpin, uint8 r, uint8 g, uint8 b, uint8 a);
		
			// Accessors

		protected :

			const CSpriteSystemProps*		GetProps() { return (const CSpriteSystemProps*)m_pProps; }

			// Member Variables
			LTFLOAT							m_tmElapsedEmission;
			LTVector						m_vRandomPoint;
			LTRotation						m_rSpin;
			LTFLOAT							m_fSpriteLen;
			uint32							m_nSpriteFrames;
			HTEXTURE					   *m_pTexArray;
			CSwapList<SPRITE>				m_collSprites;
	};

#endif