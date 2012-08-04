//--------------------------------------------------------------------------
// ParticleSystemProps.h
//
// Provides the definition for the particle system properties. This handles 
// defining the properties for FXEdit, loading them in, and validating them.
//
//--------------------------------------------------------------------------

#ifndef __PARTICLESYSTEMPROPS_H__
#define __PARTICLESYSTEMPROPS_H__

#ifndef __BASEFX_H__
#	include "BaseFx.h"
#endif

#ifndef __CLIENTFXPROP_H__
#	include "ClientFxProp.h"
#endif

#ifndef __CLIENTFXSKYUTILS_H__
#	include "ClientFXSkyUtils.h"
#endif

//The different types of emission primitives that are supported
enum ePSEmissionType
{
	PS_eSphere,
	PS_ePoint,
	PS_eCone,
	PS_eCylinder,
	PS_eBox,
};

//The different types of velocity setups
enum ePSVelocityType
{
	PSV_eRandom,
	PSV_eCenter
};


//Properties for the particle system
class CParticleSystemProps : 
	public CBaseFXProps
{
public:

	//the maximum number of particle images supported
	enum	{ knMaxParticleImages	= 16 };

	CParticleSystemProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//called after all the properties have been loaded to perform any custom initialization
	virtual bool PostLoadProperties();

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//Constant parameters

	const char*				m_pszSplatEffect;			// The name of the splat effect to create
	const char*				m_pszMaterial;				// RESEXT_MATERIAL material file
	LTVector				m_vEmissionDir;				// direction of the emission
	LTVector				m_vEmissionPerp1;			// These two are perpendicular to the emission dir
	LTVector				m_vEmissionPerp2;			// to form a plane
	float					m_fMinAngularVelocity;		// Minimum angular velocity in radians/second
	float					m_fMaxAngularVelocity;		// Maximum angular velocity in radians/second
	float					m_fEmissionInterval;		// Period of time between emitting batches of particles
	float					m_fGroupCreationInterval;	// Period of time between creating groups
	float					m_fBounceStrength;			// The coefficient of restitution for the particles
	float					m_fMaxParticlePadding;		// The amount of padding that should be added onto particles for visibility purposes to accomodate size and rotation
	float					m_fStreakScale;				// The scale to adjust the streaking by. The larger the number, the more the streaking
	bool					m_bObjectSpace;				// Do the particles move with the system?
	bool					m_bRotate;					// Determines if the angle of the particles should be used for rotation (slower)
	bool					m_bStreak;					// Determines if the particles should streak based upon their velocity
	bool					m_bInfiniteLife;			// Whether or not the particles should live forever
	bool					m_bSolid;					// Should this particle system be rendered as solid?
	bool					m_bTranslucentLight;		// Should this be lit when translucent?
	bool					m_bEnableBounceScale;		// Should we scale the bounce scale?
	bool					m_bPlayerView;				// Should we have this object render in the player rendering layer?
	uint8					m_nNumImages;				// the number of images encoded horizontally in the textures

	LTEnum<uint8, ePSVelocityType>	m_eVelocityType;	// The method of generation for the velocity
	LTEnum<uint8, ePSEmissionType>	m_eEmissionType;	// The emission method
	LTEnum<uint8, EFXSkySetting>	m_eInSky;			// The sky settings for this particle system

	//Function curve parameters

	TVectorFunctionCurveI	m_vfcMinVelocity;			// Minimum velocity to use for generation
	TVectorFunctionCurveI	m_vfcMaxVelocity;			// Minimum velocity to use for generation

	TVectorFunctionCurveI	m_vfcAcceleration;			// Additional acceleration to apply to the particles

	TVectorFunctionCurveI	m_vfcEmissionOffset;		// Offset for emission
	TVectorFunctionCurveI	m_vfcEmissionDims;			// Emission dimensions, depending upon primitive type

	TFloatFunctionCurveI	m_ffcGravityScale;			// The scale for gravity

	TFloatFunctionCurveI	m_ffcMinLifetime;			// Minimum length a particle can live for
	TFloatFunctionCurveI	m_ffcMaxLifetime;			// Maximum length a particle can live for

	TFloatFunctionCurveI	m_ffcMinRadius;				// Minimum radius, used for some generation primitives
	TFloatFunctionCurveI	m_ffcMaxRadius;				// Maximum radius, used for some generation primitives

	TFloatFunctionCurveI	m_ffcDrag;					// The friction on these particles over time

	TFloatFunctionCurveI	m_ffcPercentToBounce;		// The percentage of particles to bounce
	TFloatFunctionCurveI	m_ffcPercentToSplat;		// The percentage of particles to splat

	TIntFunctionCurve		m_nfcParticlesPerEmission;	// The number of particles to emit each frame

	TColor4fFunctionCurveI	m_cfcParticleColor;			// The color of the particle over its lifetime
	TFloatFunctionCurveI	m_ffcParticleScale;			// The scale of the particle over its lifetime
};

#endif
