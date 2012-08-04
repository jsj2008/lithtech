// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystemFX.h
//
// PURPOSE : The ParticleSystemFX object
//
// CREATED : 4/10/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLESYSTEMFX__H_
	#define __PARTICLESYSTEMFX__H_

//
// Includes...
//

	#include "basefx.h"

//
// Defines...
//

	#define PS_DEFAULT_VISRADIUS	500
	#define	PS_OPTIMIZE_COUNT		60		// How many updates before we optimize again

	enum ePSType
	{
		PS_eSphere = 0,
		PS_ePoint,
		PS_eCircle,
		PS_eCone,
		PS_eCylinder
	};

	enum ePSVelocityType
	{
		PSV_eRandom,
		PSV_eCenter
	};

//
// Classes...
//

	class CParticleSystemProps : public CBaseFXProps
	{
	public:

		CParticleSystemProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		LTBOOL				m_bInfiniteLife;		// Whether or not the particles should live forever
		LTFLOAT				m_fPercentToSplat;		// Percentage of particles that should create splat effects when hitting world [0..100]
		LTBOOL				m_bKillOnSplat;			// Should particles that splat be instantly killed?
		char				m_szSplatEffect[128];	// The name of the splat effect to create
		LTVector			m_vMinVelocity;
		LTVector			m_vMaxVelocity;
		LTVector			m_vAcceleration;		//additional acceleration vector
		LTFLOAT				m_fFriction;			//Friction multiplier per second
		LTFLOAT				m_fEmissionInterval;		
		uint32				m_nParticlesPerEmission;
		LTFLOAT				m_fMinAngularVelocity;
		LTFLOAT				m_fMaxAngularVelocity;
		LTFLOAT				m_fMinLifeSpan;
		LTFLOAT				m_fMaxLifeSpan;
		LTFLOAT				m_fMinRadius;
		LTFLOAT				m_fMaxRadius;
		ePSVelocityType		m_eVelocityType;		// The method of generation for the velocity
		LTBOOL				m_bCollideModels;		// Should this attempt to collide against models?
		LTBOOL				m_bObjectSpace;			// Do the particles move with the system?
		uint32				m_dwBlendMode;			// Specifys what blend operation to preform
		ePSType				m_eType;				// The emmission method
		LTFLOAT				m_fPercentToBounce;		// Percentage of particles that should bounce [0..100]
		LTBOOL				m_bFlipOrder;			// Allows for the flipping of the rendering order
		LTBOOL				m_bLight;				// Determines if lighting should e done on the particles (slower)
		LTBOOL				m_bRotate;				// Determines if the angle of the particles should be used for rotation (slower)
		LTVector			m_vPlaneDir;			// direction of the emission
		LTVector			m_vPerp1;				// These two are perpindicular to the emission dir
		LTVector			m_vPerp2;				// to form a plane
		char				m_szFileName[128];		// can .spr or .dtx
		LTBOOL				m_bSwarm;
	};


	class CParticleSystemFX : public CBaseFX
	{
		protected:

			LTFLOAT				m_fVisRadius;			
			LTFLOAT				m_tmElapsedEmission;			
			LTMatrix			m_matSwarm;
			LTVector			m_vRandomPoint;			// Random for the system not per particle
						
			uint8				m_nOptCount;			// we only want to optimize the system every so often...
														// this keeps track of when.
			uint32				m_nNumParticles;		// the number of currently outstanding particles
			uint32				m_nNumBounceParticles;	// the number of currently outstanding particles that bounce
			uint32				m_nNumSplatParticles;	// the number of currently outstanding particles that splat

			bool				m_bRendered;			// has this particle system been rendered before? If so we can use the visible flag as an indicator

		public:
			
			CParticleSystemFX( void );
			~CParticleSystemFX( void ); 

			bool	Init( ILTClient *pLTClient, FX_BASEDATA *pData, const CBaseFXProps *pProps);
			bool	Update( float tmFrameTime );
			bool	Render();
			bool	SuspendedUpdate( float tmFrameTime );
			void	Term( void );

			bool	IsVisibleWhileSuspended()	{ return m_nNumParticles != 0; }
			bool	IsFinishedShuttingDown()	{ return m_nNumParticles == 0; }

			
		protected:

			const CParticleSystemProps*	GetProps()		{ return (const CParticleSystemProps*)m_pProps; }

			void	RemoveParticle(LTParticle* pParticle);

			void	UpdateParticleColor(LTParticle* pParticle);
			void	UpdateParticleScale(LTParticle* pParticle);

			void	ReadProps( CLinkList<FX_PROP> *pProps );
			void	AddParticles( );
			void	UpdateParticles( LTFLOAT tmCur );
	};

#endif // __PARTICLESYSTEMFX__H_
