//---------------------------------------------------------------------------------
// LightningProps.h
//
// This file contains all of the property information for an appropriate lightning
// effect. 
//
//---------------------------------------------------------------------------------

#ifndef __LIGHTNINGPROPS_H__
#define __LIGHTNINGPROPS_H__

#ifndef __BASEFX_H__
#	include "BaseFx.h"
#endif

#ifndef __CLIENTFXSKYUTILS_H__
#	include "ClientFXSkyUtils.h"
#endif

//the listing of the available bolt component types
enum EBoltComponent
{
	eBoltComponent_None,
	eBoltComponent_Sine,
	eBoltComponent_Sawtooth,
	eBoltComponent_Noise
};

class CLightningProps : 
	public CBaseFXProps
{
public:

	CLightningProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//called after all the properties have been loaded to perform any custom initialization
	virtual bool PostLoadProperties();

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//the listing of nodes and sockets that should act as attractors
	const char*				m_pszNodeAttractors;
	const char*				m_pszSocketAttractors;

	//the radius to use around the target position
	float					m_fInnerAttractorRadius;
	float					m_fAttractorRadius;

	//the center of the cone of the attractor
	LTVector				m_vAttractorConeDir;
	float					m_fAttractorConeAngle;

	//the name of the material associated with this bolt of lightning
	const char*				m_pszMaterial;

	//whether or not this lightning should be rendered solid
	bool					m_bRenderSolid;

	//whether or not this lightning should receive translucent lighting
	bool					m_bTranslucentLight;

	//determines if geometry should block lightning going from the emitter to the destination
	bool					m_bBlockedByGeometry;

	//should we have this object render in the player rendering layer?
	bool					m_bPlayerView;

	//should this effect be placed in the sky
	EFXSkySetting			m_eInSky;

	//the width range to create the bolts using
	float					m_fMinWidth;
	float					m_fMaxWidth;

	//the segment length to use when rendering bolts
	float					m_fSegmentLength;

	//the length in units that a texture should repeat. This is in units/repeat
	float					m_fTextureLength;

	//the distance over which the bolts will be attracted back to the start and end points of
	//the lightning segment appropriately (0 = no attraction back)
	float					m_fStartAttractionDist;
	float					m_fEndAttractionDist;

	//the amount of time to elapse before emitting another batch of lightning in seconds
	float					m_fEmissionInterval;

	//the number of bolts to create per emission
	TIntFunctionCurveI		m_nfcBoltsPerEmission;

	//the lifespan for a bolt
	TFloatFunctionCurveI	m_ffcMinBoltLifetime;
	TFloatFunctionCurveI	m_ffcMaxBoltLifetime;

	//the range that the lightning bolts can be created in around the line of the lightning
	TFloatFunctionCurveI	m_ffcMinBoltAngle;
	TFloatFunctionCurveI	m_ffcMaxBoltAngle;

	//the color of a bolt over its lifetime
	TColor4fFunctionCurveI	m_cfcBoltColor;

	//the panning speed of the texture applied to the texture in repeats/s
	TFloatFunctionCurveI	m_ffcUPanSpeed;			

	//the listing of components for each bolt component
	static const uint32 knMaxBoltComponents = 4;

	struct SComponentProps
	{
		SComponentProps();
		
		//the type of this component
		EBoltComponent			m_eType;

		//the amplitude of this component over the bolt lifetime
		TFloatFunctionCurveI	m_ffcAmplitude;

		//the frequency of this component over the bolt lifetime
		TFloatFunctionCurveI	m_ffcFrequency;

		//the pitch velocity of this component over the bolt lifetime
		TFloatFunctionCurveI	m_ffcPitchVelocity;

		//the angular range that this component can be within
		float					m_fAngularMin;
		float					m_fAngularMax;
	};

	//the components that are used to make up a bolt
	SComponentProps		m_Components[knMaxBoltComponents];

	//the amplitude range, which is a scale of the base component amplitudes
	float				m_fAmplitudeMin;
	float				m_fAmplitudeMax;

	//the frequency range
	float				m_fFrequencyMin;
	float				m_fFrequencyMax;

	//the pitch velocity
	float				m_fPitchVelMin;
	float				m_fPitchVelMax;

	//the pitch range
	float				m_fPitchMinOffset;
	float				m_fPitchMaxOffset;
};

#endif
