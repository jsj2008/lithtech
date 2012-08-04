#include "stdafx.h"
#include "LightningProps.h"
#include "resourceextensions.h"


//----------------------------------------------------------------------------------
// SComponentProps
//----------------------------------------------------------------------------------
CLightningProps::SComponentProps::SComponentProps() :
	m_eType(eBoltComponent_None),
	m_fAngularMin(0.0f),
	m_fAngularMax(0.0f)
{
}

//----------------------------------------------------------------------------------
// CLightningProps
//----------------------------------------------------------------------------------
CLightningProps::CLightningProps() :	
	CBaseFXProps(),
	m_pszNodeAttractors(NULL),
	m_pszSocketAttractors(NULL),
	m_fInnerAttractorRadius(0.0f),
	m_fAttractorRadius(0.0f),
	m_vAttractorConeDir(0.0f, 0.0f, 1.0f),
	m_fAttractorConeAngle(MATH_PI),
	m_pszMaterial(NULL),
	m_bRenderSolid(false),
	m_bTranslucentLight(false),
	m_bBlockedByGeometry(false),
	m_bPlayerView(false),
	m_eInSky(eFXSkySetting_None),
	m_fMinWidth(5.0f),
	m_fMaxWidth(5.0f),
	m_fSegmentLength(10.0f),
	m_fTextureLength(20.0f),
	m_fStartAttractionDist(5.0f),
	m_fEndAttractionDist(5.0f),
	m_fEmissionInterval(0.0f),
	m_fAmplitudeMin(1.0f),
	m_fAmplitudeMax(1.0f),
	m_fFrequencyMin(1.0f),
	m_fFrequencyMax(1.0f),
	m_fPitchVelMin(1.0f),
	m_fPitchVelMax(1.0f),
	m_fPitchMinOffset(0.0f),
	m_fPitchMaxOffset(0.0f)
{
}

//called after all the properties have been loaded to perform any custom initialization
bool CLightningProps::PostLoadProperties()
{
	if(!CBaseFXProps::PostLoadProperties())
		return false;

	if(m_fInnerAttractorRadius > m_fAttractorRadius)
		std::swap(m_fInnerAttractorRadius, m_fAttractorRadius);

	return true;
}


//handles loading up a single property from the specified file
bool CLightningProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	//------------------------------------------------------------
	//Attractors

	if(LTStrIEquals(pszName, "NodeAttractors"))
	{
		m_pszNodeAttractors = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if(LTStrIEquals(pszName, "SocketAttractors"))
	{
		m_pszSocketAttractors = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if(LTStrIEquals(pszName, "InnerAttractorRadius"))
	{
		m_fInnerAttractorRadius = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "AttractorRadius"))
	{
		m_fAttractorRadius = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "AttractorConeDir"))
	{
		m_vAttractorConeDir = CFxProp_Vector::Load(pStream).GetUnit();
	}
	else if(LTStrIEquals(pszName, "AttractorConeAngle"))
	{
		m_fAttractorConeAngle = MATH_DEGREES_TO_RADIANS(CFxProp_Float::Load(pStream));
	}
	else if(LTStrIEquals(pszName, "StartAttractionDist"))
	{
		m_fStartAttractionDist = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "EndAttractionDist"))
	{
		m_fEndAttractionDist = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "BlockedByGeometry"))
	{
		m_bBlockedByGeometry = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "PlayerView" ) )
	{
		m_bPlayerView = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "InSky" ))
	{
		m_eInSky = (EFXSkySetting)CFxProp_Enum::Load(pStream);
	}

	//------------------------------------------------------------
	//Rendering

	else if(LTStrIEquals(pszName, "Material"))
	{
		m_pszMaterial = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if(LTStrIEquals(pszName, "Solid"))
	{
		m_bRenderSolid = CFxProp_EnumBool::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "TranslucentLight"))
	{
		m_bTranslucentLight = CFxProp_EnumBool::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "TextureLength"))
	{
		m_fTextureLength = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "UPanSpeed"))
	{
		m_ffcUPanSpeed.Load(pStream, pCurveData);
	}

	//------------------------------------------------------------
	//Bolt Emission

	else if(LTStrIEquals(pszName, "EmissionInterval"))
	{
		m_fEmissionInterval = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "BoltsPerEmission"))
	{
		m_nfcBoltsPerEmission.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "MinBoltLifetime"))
	{
		m_ffcMinBoltLifetime.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "MaxBoltLifetime"))
	{
		m_ffcMaxBoltLifetime.Load(pStream, pCurveData);
	}

	//------------------------------------------------------------
	//Bolt Properties

	else if(LTStrIEquals(pszName, "MinWidth"))
	{
		m_fMinWidth = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "MaxWidth"))
	{
		m_fMaxWidth = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "MinBoltAngle"))
	{
		m_ffcMinBoltAngle.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "MaxBoltAngle"))
	{
		m_ffcMaxBoltAngle.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "SegmentLength"))
	{
		m_fSegmentLength = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "BoltColor"))
	{
		m_cfcBoltColor.Load(pStream, pCurveData);
	}

	//------------------------------------------------------------
	//Bolt Component Scales

	else if(LTStrIEquals(pszName, "AmplitudeMin"))
	{
		m_fAmplitudeMin = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "AmplitudeMax"))
	{
		m_fAmplitudeMax = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "FrequencyMin"))
	{
		m_fFrequencyMin = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "FrequencyMax"))
	{
		m_fFrequencyMax = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "PitchVelMin"))
	{
		m_fPitchVelMin = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "PitchVelMax"))
	{
		m_fPitchVelMax = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "PitchMinOffset"))
	{
		m_fPitchMinOffset = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "PitchMaxOffset"))
	{
		m_fPitchMaxOffset = CFxProp_Float::Load(pStream);
	}
	else
	{
		for(uint32 nCurrComponent = 0; nCurrComponent < knMaxBoltComponents; nCurrComponent++)
		{
			char szPropName[64];

			//------------------------------------------------------------
			LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "Type%d", nCurrComponent);
			if(LTStrIEquals(pszName, szPropName))
			{
				m_Components[nCurrComponent].m_eType = (EBoltComponent)CFxProp_Enum::Load(pStream);
				return true;
			}

			LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "Amplitude%d", nCurrComponent);
			if(LTStrIEquals(pszName, szPropName))
			{
				m_Components[nCurrComponent].m_ffcAmplitude.Load(pStream, pCurveData);
				return true;
			}

			LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "Frequency%d", nCurrComponent);
			if(LTStrIEquals(pszName, szPropName))
			{
				m_Components[nCurrComponent].m_ffcFrequency.Load(pStream, pCurveData);
				return true;
			}

			LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "PitchVelocity%d", nCurrComponent);
			if(LTStrIEquals(pszName, szPropName))
			{
				m_Components[nCurrComponent].m_ffcPitchVelocity.Load(pStream, pCurveData);
				return true;
			}

			LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "AngularMin%d", nCurrComponent);
			if(LTStrIEquals(pszName, szPropName))
			{
				m_Components[nCurrComponent].m_fAngularMin = CFxProp_Float::Load(pStream);
				return true;
			}

			LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "AngularMax%d", nCurrComponent);
			if(LTStrIEquals(pszName, szPropName))
			{
				m_Components[nCurrComponent].m_fAngularMax = CFxProp_Float::Load(pStream);
				return true;
			}
		}

		//no one else handled it, so allow the base class to load
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CLightningProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectResource(m_pszMaterial);
}


//------------------------------------------------------------------
//
//   FUNCTION : fxGetLightningProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetLightningProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;

	// Add the base props
	AddBaseProps(pList);

	//------------------------------------------------------------
	fxProp.SetupTextLine("Attractors");
	pList->AddTail(fxProp);

	fxProp.SetupString("NodeAttractors", "", eCurve_None, "This contains the listing of nodes that the lightning should be attracted to. This should be semicolon delimited.");
	pList->AddTail( fxProp );

	fxProp.SetupString("SocketAttractors", "", eCurve_None, "This contains the listing of sockets that the lightning should be attracted to. This should be semicolon delimited.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("InnerAttractorRadius", 0.0f, 0.0f, eCurve_None, "This is an optional additional radius that indicates an additional sphere around the target that a bolt can randomly attach to a point within. This should be less than the AttractorRadius, and the attractor is guaranteed to be this distance away.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("AttractorRadius", 0.0f, 0.0f, eCurve_None, "This is an optional additional radius that indicates an additional sphere around the target that a bolt can randomly attach to a point within.");
	pList->AddTail( fxProp );

	fxProp.SetupVector("AttractorConeDir", LTVector(0.0f, 0.0f, 1.0f), eCurve_None, "This is the direction that the cone for attractors can be oriented. This should be normalized.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMinMax("AttractorConeAngle", 180.0f, 0.0f, 180.0f, eCurve_None, "This is the angle of the cone that is used for attractors. Setting this to 180 causes the attractor points to be generated in a complete sphere.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("StartAttractionDist", 5.0f, 0.0f, eCurve_None, "This indicates the length in units over which the bolts will bend back towards the emitter. If this is set to zero, they will not bend back at all.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("EndAttractionDist", 5.0f, 0.0f, eCurve_None, "This indicates the length in units over which the bolts will bend back towards the target. If this is set to zero, they will not bend back at all.");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool("BlockedByGeometry", false, eCurve_None, "Determines whether or not lightning should be blocked by geometry. This is an expensive feature to enable");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "PlayerView", false, eCurve_None, "Determines if the sprite should be rendered in the player view, which means that it should have its Z values adjusted to not be clipped into nearby walls");
	pList->AddTail( fxProp );	

	fxProp.SetupEnum( "InSky", SKY_PROP_DEFAULT, SKY_PROP_ENUM, eCurve_None, SKY_PROP_DESCRIPTION);
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Rendering");
	pList->AddTail(fxProp);

	fxProp.SetupPath("Material", "", "Material Files (*." RESEXT_MATERIAL ")|*." RESEXT_MATERIAL "|All Files (*.*)|*.*||", eCurve_None, "The material to use when rendering the lightning. This material should be rigid, and if the alpha of the lightning is not 1, should also be translucent.");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool("Solid", false, eCurve_None, "Determines if this lightning should be treated as opaque or translucent" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool("TranslucentLight", true, eCurve_None, "For translucent objects, this determines if lighting should be approximated or if it should be fullbright" );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("TextureLength", 20.0f, 0.0f, eCurve_None, "This indicates the length of the texture and is measured in how many units before the texture repeats.");
	pList->AddTail( fxProp );

	fxProp.SetupFloat("UPanSpeed", 0.0f, eCurve_Linear, "This is the speed that the texture pans in the U direction, and is measured in units per second");
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Bolt Emission");
	pList->AddTail(fxProp);


	fxProp.SetupFloatMin("EmissionInterval", 5.0f, 0.0f, eCurve_None, "This indicates the period of time between emitting a batch of bolts. This is measured in seconds and should be set to zero to only emit a batch of bolts once.");
	pList->AddTail( fxProp );

	fxProp.SetupIntMin("BoltsPerEmission", 5, 0, eCurve_Linear, "Inidcates the number of bolts that should be created for a single emission.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("MinBoltLifetime", 1.0f, 0.0f, eCurve_Linear, "Indicates the minimum number of seconds that a bolt will live for.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("MaxBoltLifetime", 1.0f, 0.0f, eCurve_Linear, "Indicates the maximum number of seconds that a bolt will live for.");
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Bolt Properties");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("MinWidth", 5.0f, 0.0f, eCurve_None, "This indicates the minimum width that a bolt in this lightning system can be.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("MaxWidth", 5.0f, 0.0f, eCurve_None, "This indicates the maximum width that a bolt in this lightning system can be.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMinMax("MinBoltAngle", 0.0f, 0.0f, 360.0f, eCurve_Linear, "This is the minimum angle that any given bolt can twist around the vector to the attractor.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMinMax("MaxBoltAngle", 0.0f, 0.0f, 360.0f, eCurve_Linear, "This is the maximum angle that any given bolt can twist around the vector to the attractor.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("SegmentLength", 10.0f, 1.0f, eCurve_None, "Indicates the length of the segments that will be used to subdivide the bolt for rendering. Smaller numbers mean more finely tesselated bolts which look smoother, but are more expensive to render");
	pList->AddTail( fxProp );

	fxProp.SetupColor("BoltColor", 0xFFFFFFFF, eCurve_Linear, "This is the color of the bolt over the course of its lifetime.");
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Bolt Component Scales");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("AmplitudeMin", 1.0f, 0.0f, eCurve_None, "This value is calculated per bolt and allows for a random scale to be associated with the bolt");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("AmplitudeMax", 1.0f, 0.0f, eCurve_None, "This value is calculated per bolt and allows for a random scale to be associated with the bolt");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("FrequencyMin", 1.0f, 0.0f, eCurve_None, "This value is calculated per bolt and allows for a random scale to be associated with the bolt");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("FrequencyMax", 1.0f, 0.0f, eCurve_None, "This value is calculated per bolt and allows for a random scale to be associated with the bolt");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("PitchVelMin", 1.0f, 0.0f, eCurve_None, "This value is calculated per bolt and allows for a random scale to be associated with the bolt");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("PitchVelMax", 1.0f, 0.0f, eCurve_None, "This value is calculated per bolt and allows for a random scale to be associated with the bolt");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("PitchMinOffset", 0.0f, 0.0f, eCurve_None, "This is a random range that can be entered to cause bolts to start off offset based upon some specified pitch.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("PitchMaxOffset", 0.0f, 0.0f, eCurve_None, "This is a random range that can be entered to cause bolts to start off offset based upon some specified pitch.");
	pList->AddTail( fxProp );

	for(uint32 nCurrComponent = 0; nCurrComponent < CLightningProps::knMaxBoltComponents; nCurrComponent++)
	{
		char szPropName[64];

		//------------------------------------------------------------
		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "Component%d", nCurrComponent);
		fxProp.SetupTextLine(szPropName);
		pList->AddTail(fxProp);

		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "Type%d", nCurrComponent);
		fxProp.SetupEnum(szPropName, "None", "None,Sine,Sawtooth,Noise", eCurve_None, "This indicates the formula associated with this bolt component");
		pList->AddTail( fxProp );

		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "Amplitude%d", nCurrComponent);
		fxProp.SetupFloatMin(szPropName, 1.0f, 0.0f, eCurve_Linear, "This is the maximum height of the component over the course of the bolt's lifetime in game units");
		pList->AddTail( fxProp );

		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "Frequency%d", nCurrComponent);
		fxProp.SetupFloatMin(szPropName, 1.0f, 0.0f, eCurve_Linear, "This indicates the density of the function, or over how many units a function will repeat itself. For example, a frequency of 100 means that a sine wave will loop once every 100 units");
		pList->AddTail( fxProp );

		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "PitchVelocity%d", nCurrComponent);
		fxProp.SetupFloat(szPropName, 0.0f, eCurve_Linear, "This is the velocity of this component along the axis of the bolt over the course of the bolt's lifetime");
		pList->AddTail( fxProp );

		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "AngularMin%d", nCurrComponent);
		fxProp.SetupFloatMinMax(szPropName, 0.0f, 0.0f, 360.0f, eCurve_None, "This is the minimum angle that it can rotate the component relative to the bolt");
		pList->AddTail( fxProp );

		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "AngularMax%d", nCurrComponent);
		fxProp.SetupFloatMinMax(szPropName, 0.0f, 0.0f, 360.0f, eCurve_None, "This is the maximum angle that it can rotate the component relative to the bolt");
		pList->AddTail( fxProp );
	}
}

