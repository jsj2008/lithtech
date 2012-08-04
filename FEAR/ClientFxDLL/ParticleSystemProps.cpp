#include "stdafx.h"
#include "ParticleSystemProps.h"
#include "ClientFX.h"
#include "resourceextensions.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::CParticleSystemProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CParticleSystemProps::CParticleSystemProps() : 
	m_vEmissionDir			( 0.0f, 1.0f, 0.0f ),
	m_eEmissionType			( PS_eSphere ),
	m_eVelocityType			( PSV_eRandom ),
	m_eInSky				( eFXSkySetting_None ),
	m_bObjectSpace			( false ),
	m_bRotate				( false ),
	m_bStreak				( false ),
	m_bInfiniteLife			( false ),
	m_bSolid				( false ),
	m_bPlayerView			( false ),
	m_bTranslucentLight		( true ),
	m_bEnableBounceScale	( true ),
	m_fStreakScale			( 1.0f ),
	m_fEmissionInterval		( 0.0f ),
	m_fGroupCreationInterval( 0.0f ),
	m_fMaxParticlePadding	( 0.0f ),
	m_fBounceStrength		( 0.75f ),
	m_pszMaterial			( NULL ),
	m_pszSplatEffect		( NULL ),
	m_nNumImages			( 1 )
{
}

//called after all properties have been loaded to allow for post processing of parameters
bool CParticleSystemProps::PostLoadProperties()
{
	//we need to find the largest size that particles can get in this system
	m_fMaxParticlePadding = 0.0f;
	for(uint32 nCurrKey = 0; nCurrKey < m_ffcParticleScale.GetNumKeys(); nCurrKey++)
	{
		m_fMaxParticlePadding = LTMAX(m_fMaxParticlePadding, m_ffcParticleScale.GetKey(nCurrKey));
	}

	//scale the particle padding so that the screen orientation and rotation of the particle are considered
	//and also the fact that the size is the full size and we want the half size
	 m_fMaxParticlePadding *= 0.5f * MATH_SQRT2;

	return CBaseFXProps::PostLoadProperties();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CParticleSystemFX::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CParticleSystemProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	if( LTStrIEquals( pszName, "Material" ) )
	{
		m_pszMaterial = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if (LTStrIEquals( pszName, "NumImages" ) )
	{
		m_nNumImages = (uint32)CFxProp_Int::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "PlayerView" ) )
	{
		m_bPlayerView = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "InSky" ))
	{
		m_eInSky = (EFXSkySetting)CFxProp_Enum::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "ParticleColor" ) )
	{
		m_cfcParticleColor.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "ParticleScale" ) )
	{
		m_ffcParticleScale.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "ParticlesPerEmission" ) )
	{
		m_nfcParticlesPerEmission.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MinParticleLifeSpan" ) )
	{
		m_ffcMinLifetime.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MaxParticleLifeSpan" ) )
	{	
		m_ffcMaxLifetime.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "AdditionalAcceleration") )
	{
		m_vfcAcceleration.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "GravityScale") )
	{
		m_ffcGravityScale.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "EmissionInterval" ) )
	{
		m_fEmissionInterval = CFxProp_Float::Load(pStream);
		//if this is above zero, make sure that it is set to a reasonable limit to prevent
		//effects from performing way too many emissions
		if(m_fEmissionInterval > 0.0f)
			m_fEmissionInterval = LTMAX(m_fEmissionInterval, 0.01f);
	}
	else if( LTStrIEquals( pszName, "GroupCreationInterval" ) )
	{
		m_fGroupCreationInterval = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "EmissionDir" ) )
	{
		m_vEmissionDir = CFxProp_Vector::Load(pStream);	

		//handle the case of if an artist cleared it. They aren't supposed to, but happens occasionally
		if(m_vEmissionDir.NearlyEquals(LTVector::GetIdentity(), 0.01f))
			m_vEmissionDir.Init(0.0f, 1.0f, 0.0f);

		m_vEmissionDir.Normalize();

		// Get the perpindicular vectors to this plane
		FindPerps(m_vEmissionDir, m_vEmissionPerp1, m_vEmissionPerp2);
	}
	else if( LTStrIEquals( pszName, "EmissionOffset" ) )
	{
		m_vfcEmissionOffset.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "EmissionDims" ) )
	{
		m_vfcEmissionDims.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "EmissionMinRadius" ) )
	{
		m_ffcMinRadius.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "EmissionMaxRadius" ) )
	{
		m_ffcMaxRadius.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MinParticleVelocity" ) )
	{
		m_vfcMinVelocity.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MaxParticleVelocity" ) )
	{
		m_vfcMaxVelocity.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "EmissionType" ) )
	{
		m_eEmissionType = (ePSEmissionType)CFxProp_Enum::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "PercentToBounce" ) )
	{
		m_ffcPercentToBounce.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "RotateParticles" ) )
	{
		m_bRotate = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "MoveParticlesWithSystem" ) )
	{
		m_bObjectSpace = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "MinAngularVelocity" ) )
	{
		m_fMinAngularVelocity = MATH_DEGREES_TO_RADIANS(CFxProp_Float::Load(pStream));
	}
	else if( LTStrIEquals( pszName, "MaxAngularVelocity" ) )
	{
		m_fMaxAngularVelocity = MATH_DEGREES_TO_RADIANS(CFxProp_Float::Load(pStream));
	}
	else if( LTStrIEquals( pszName, "Streak") )
	{
		m_bStreak = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "StreakScale") )
	{
		m_fStreakScale = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Drag" ) )
	{
		m_ffcDrag.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "BounceStrength" ) )
	{
		m_fBounceStrength = LTMAX(CFxProp_Float::Load(pStream), 0.0f);
	}
	else if( LTStrIEquals( pszName, "Solid" ) )
	{
		m_bSolid = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "TranslucentLight" ) )
	{
		m_bTranslucentLight = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "EnableBounceScale" ) )
	{
		m_bEnableBounceScale = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "VelocityType" ) )
	{
		m_eVelocityType = (ePSVelocityType)CFxProp_Enum::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "InfiniteLife" ) )
	{
		m_bInfiniteLife = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "SplatEffect" ) )
	{
		m_pszSplatEffect = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if( LTStrIEquals( pszName, "SplatPercent" ) )
	{
		m_ffcPercentToSplat.Load(pStream, pCurveData);
	}
	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;	
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CParticleSystemProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectResource(m_pszMaterial);
	Collector.CollectEffect(m_pszSplatEffect);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxGetParticleSystemProps
//
//  PURPOSE:	Returns a list of properties for ths effect to be used in FXEdit
//
// ----------------------------------------------------------------------- //

void fxGetParticleSystemProps( CFastList<CEffectPropertyDesc> *pList )
{
	CEffectPropertyDesc	fxProp;

	LTVector vY(0, 1.0f, 0);
	LTVector vZero(0, 0, 0);

	// Add the generic "every effect has these" props
	AddBaseProps( pList );

	// Add specific Particle System Props
	
	//------------------------------------------------------------
	fxProp.SetupTextLine("System Rendering");
	pList->AddTail(fxProp);

	fxProp.SetupPath( "Material", "", "Material Files (*." RESEXT_MATERIAL ")|*." RESEXT_MATERIAL "|All Files (*.*)|*.*||", eCurve_None, "The material to use when rendering the particles. This material should be rigid, and if the alpha of the particle is not 1, should also be translucent." );
	pList->AddTail( fxProp );

	fxProp.SetupIntMinMax( "NumImages", 1, 1, CParticleSystemProps::knMaxParticleImages, eCurve_None, "The number of images contained within the textures used by the particle. These must be laid out in a horizontal strip, with each image occupying the same width." );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "Solid", false, eCurve_None, "Determines if this particle system should be treated as opaque or translucent" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "TranslucentLight", true, eCurve_None, "For translucent objects, this determines if lighting should be approximated or if it should be fullbright" );
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("System Behavior");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( "PlayerView", false, eCurve_None, "Determines if the sprite should be rendered in the player view, which means that it should have its Z values adjusted to not be clipped into nearby walls");
	pList->AddTail( fxProp );	

	fxProp.SetupEnum( "InSky", SKY_PROP_DEFAULT, SKY_PROP_ENUM, eCurve_None, SKY_PROP_DESCRIPTION);
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "MoveParticlesWithSystem", false, eCurve_None, "If this is enabled, the particles will move along with the emitter. This will cause the particle positions, velocity, and additional acceleration to all be specified in object space." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "GroupCreationInterval", 0.0f, 0.0f, eCurve_None, "This indicates the amount of time that will elapse before a new group is created for the particles to be placed within. This should be set to zero to disable creation of multiple groups");
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Particle Emission");
	pList->AddTail(fxProp);

	fxProp.SetupFloat( "EmissionInterval", 0.1f, eCurve_None, "This is the rate that batches of particles will be emitted from the particle system. This is measured in seconds, so .1 will emit every .1 seconds, or 10 times a second. This should be set to zero if only a single emission should occur." );
	pList->AddTail( fxProp );

	fxProp.SetupIntMin( "ParticlesPerEmission", 5, 0, eCurve_Linear, "Indicates the number of particles that will be emitted per batch." );
	pList->AddTail( fxProp );

	fxProp.SetupEnum( "EmissionType", "Sphere", "Sphere, Point, Cone, Cylinder, Box", eCurve_None, "The primitive to use for emitting the particles." );
	pList->AddTail( fxProp );

	fxProp.SetupVector( "EmissionDir", vY, eCurve_None, "The direction to aim the emission primitive. This is used for cone and cylinder to aim the direction of them." );
	pList->AddTail( fxProp );

	fxProp.SetupVector( "EmissionOffset", vZero, eCurve_Linear, "An additional offset from the position of the object that will be applied to where particles are emitted. For example, if a particle system is attached to a model, but should be offset in the Y direction this can be used to offset the point of emission." );
	pList->AddTail( fxProp );

	fxProp.SetupVectorMin( "EmissionDims", vZero, 0.0f, eCurve_Linear, "This indicates the dimensions of the area to emit in. For box all three are used to determine the half dimensions of the box, for cylinder and cone though, the Y component is used to indicate the height." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "EmissionMinRadius", 0.0f, 0.0f, eCurve_Linear, "The minimum radius to use for the sphere, cone, and cylinder emission types." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "EmissionMaxRadius", 10.0f, 0.0f, eCurve_Linear, "The maximum radius to use for the sphere, cone, and cylinder emission types." );
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Particle Properties");
	pList->AddTail(fxProp);

	fxProp.SetupColor( "ParticleColor", 0xFFFFFFFF, eCurve_Linear, "The color of the particles over the course of each particle's lifetime." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("ParticleScale", 10.0f, 0.0f, eCurve_Linear, "This controls the size of the particles over the course of each particle's lifetime");
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Particle Lifetime");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin( "MinParticleLifeSpan", 2.0f, 0.0f, eCurve_Linear, "The minimum length of time that a particle will live for, measured in seconds. Ignored if the particles have infinite life." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "MaxParticleLifeSpan", 3.0f, 0.0f, eCurve_Linear, "The maximum length of time that a particle will live for, measured in seconds. Ignored if the particles have infinite life." );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "InfiniteLife", false, eCurve_None, "Specifies whether or not the particles created should last until the end of the effect, or disappear when their lifetime expires" );
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Particle Gravity");
	pList->AddTail(fxProp);

	fxProp.SetupFloat( "GravityScale", 1.0f, eCurve_Linear, "Indicates how much gravity will influence the particles. Zero indicates that gravity will not effect the system, and one indicates that gravity will have normal effect. Values outside of this range can be used for other effects such as negative gravity, or exceptionally fast gravity" );
	pList->AddTail( fxProp );

	fxProp.SetupVector( "AdditionalAcceleration", vZero, eCurve_Linear, "Indicates an additional acceleration to apply to the particles. This acts very much like additional gravity, but can be specified in any direction and can change over time." );
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Particle Velocity");
	pList->AddTail(fxProp);

	fxProp.SetupEnum( "VelocityType", "Random", "Random, FromCenter", eCurve_None, "The method to use when determining the initial linear velocity to use for the particles. Random will pick a random value within the range specified below, while FromCenter will use the direction from the center as the velocity direction, and the X component of the below values to determine the speed" );
	pList->AddTail( fxProp );

	fxProp.SetupVector( "MinParticleVelocity", vY, eCurve_Linear, "The minimum range for the velocities to use when randomly determining the particle velocity. This is in cm per second, and only the X is used for FromCenter" );
	pList->AddTail( fxProp );

	fxProp.SetupVector( "MaxParticleVelocity", vY, eCurve_Linear, "The maximum range for the velocities to use when randomly determining the particle velocity. This is in cm per second, and only the X is used for FromCenter" );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMinMax( "Drag", 0.0f, 0.0f, 1.0f, eCurve_Linear, "Indicates the amount of drag on the particles. This ranges from zero to one, where zero means no drag, and one means full drag, stopping the particles from moving." );
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Particle Rotation");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( "RotateParticles", false, eCurve_None, "Determines whether or not the particles will rotate. This cannot be used with streaks and does incur some additional cost." );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "MinAngularVelocity", 0.0f, eCurve_None, "If RotateParticles is enabled, this will indicate the minimum rotational velocity of the particles in radians per second." );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "MaxAngularVelocity", 360.0f, eCurve_None, "If RotateParticles is enabled, this will indicate the maximum rotational velocity of the particles in radians per second." );
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Particle Streaks");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( "Streak", false, eCurve_None, "Determines if the particles should be streaked based upon their current velocity. This cannot be used with rotating particles." );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "StreakScale", 1.0f, eCurve_None, "The scale of the streak to apply. The larger this number, the more the particles will streak. At 1.0, a particle going 1 cm/s will stream one unit, so setting this to 10 will make it streak ten times as large and so on.");
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Particle Bounce");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( "EnableBounceScale", true, eCurve_None, "Determines whether or not the global scaling of particle bouncing should be applied to the bounce and splat scales specified for this particle system." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMinMax( "PercentToBounce", 0.0f, 0.0f, 100.0f, eCurve_Linear, "Indicates the percentage of particles that should bounce when they impact a surface. This can range from 0 to 100 where 0 means none bounce, and 100 means all bounce." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "BounceStrength", 0.75f, 0.0f, eCurve_None, "Specifies the amount of energy left when bouncing. 1 means an object will bounce as high as it was dropped, and 0 means it will not bounce at all" );
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Particle Splat");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMinMax( "SplatPercent", 0.0f, 0.0f, 100.0f, eCurve_Linear, "Indicates the percentage of particles that will create the impact effect named below when they impact a surface. This can range from 0 to 100 where 0 means none will create effects, and 100 means all will create effects." );
	pList->AddTail( fxProp );

	fxProp.SetupClientFXRef( "SplatEffect", "", eCurve_None, "The name of the impact effect to create when a particle impacts a surface.");
	pList->AddTail( fxProp );

}

