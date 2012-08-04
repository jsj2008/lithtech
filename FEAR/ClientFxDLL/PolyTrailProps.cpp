#include "stdafx.h"
#include "PolyTrailProps.h"
#include "resourceextensions.h"

//----------------------------------------------------------------------------------
// STrackedNode
//----------------------------------------------------------------------------------
CPolyTrailProps::STrackedNode::STrackedNode() :
	m_pszNodeName(NULL),
	m_vTrackedOffset(LTVector::GetIdentity()),
	m_fUCoord(1.0f)
{
}

//----------------------------------------------------------------------------------
// CPolyTrailProps
//----------------------------------------------------------------------------------
CPolyTrailProps::CPolyTrailProps() :	
	CBaseFXProps(),
	m_pszMaterial(NULL),
	m_bRenderSolid(false),
	m_bTranslucentLight(false),
	m_bPlayerView(false),
	m_eInSky(eFXSkySetting_None),
	m_fSingleNodeWidth(20.0f),
	m_fSampleFrequencyS(0.2f),
	m_fSampleLifetimeS(2.0f),
	m_fTextureLength(100.0f),
	m_nDistanceNode(0)
{
}

//called after all the properties have been loaded to perform any custom initialization
bool CPolyTrailProps::PostLoadProperties()
{
	return CBaseFXProps::PostLoadProperties();
}


//handles loading up a single property from the specified file
bool CPolyTrailProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	//------------------------------------------------------------
	//Rendering

	if( LTStrIEquals( pszName, "PlayerView" ) )
	{
		m_bPlayerView = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "InSky" ))
	{
		m_eInSky = (EFXSkySetting)CFxProp_Enum::Load(pStream);
	}
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
	else if(LTStrIEquals(pszName, "DistanceNode"))
	{
		m_nDistanceNode = CFxProp_Enum::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "SingleNodeWidth"))
	{
		m_fSingleNodeWidth = CFxProp_Float::Load(pStream);
	}

	//------------------------------------------------------------
	//Samples

	else if(LTStrIEquals(pszName, "SampleFrequency"))
	{
		m_fSampleFrequencyS = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "SampleLifetime"))
	{
		m_fSampleLifetimeS = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "SampleColor"))
	{
		m_cfcSampleColor.Load(pStream, pCurveData);
	}

	//------------------------------------------------------------
	//Tracked Nodes

	else
	{
		for(uint32 nCurrTracked = 1; nCurrTracked < knMaxTrackedNodes; nCurrTracked++)
		{
			char szPropName[64];

			//------------------------------------------------------------
			LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "TrackedNode%d", nCurrTracked);
			if(LTStrIEquals(pszName, szPropName))
			{
				m_TrackedNodes[nCurrTracked - 1].m_pszNodeName = CFxProp_String::Load(pStream, pszStringTable);
				return true;
			}

			LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "NodeOffset%d", nCurrTracked);
			if(LTStrIEquals(pszName, szPropName))
			{
				m_TrackedNodes[nCurrTracked - 1].m_vTrackedOffset = CFxProp_Vector::Load(pStream);
				return true;
			}

			LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "UTexCoord%d", nCurrTracked);
			if(LTStrIEquals(pszName, szPropName))
			{
				m_TrackedNodes[nCurrTracked - 1].m_fUCoord = CFxProp_Float::Load(pStream);
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
void CPolyTrailProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectResource(m_pszMaterial);
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetPolyTrailProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetPolyTrailProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;

	// Add the base props
	AddBaseProps(pList);


	//------------------------------------------------------------
	fxProp.SetupTextLine("Rendering");
	pList->AddTail(fxProp);

	fxProp.SetupPath("Material", "", "Material Files (*." RESEXT_MATERIAL ")|*." RESEXT_MATERIAL "|All Files (*.*)|*.*||", eCurve_None, "The material to use when rendering the lightning. This material should be rigid, and if the alpha of the lightning is not 1, should also be translucent.");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "PlayerView", false, eCurve_None, "Determines if the sprite should be rendered in the player view, which means that it should have its Z values adjusted to not be clipped into nearby walls");
	pList->AddTail( fxProp );	

	fxProp.SetupEnum( "InSky", SKY_PROP_DEFAULT, SKY_PROP_ENUM, eCurve_None, SKY_PROP_DESCRIPTION);
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool("Solid", false, eCurve_None, "Determines if this lightning should be treated as opaque or translucent" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool("TranslucentLight", true, eCurve_None, "For translucent objects, this determines if lighting should be approximated or if it should be fullbright" );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("TextureLength", 100.0f, 0.0f, eCurve_None, "This indicates the length of the texture and is measured in how many units before the texture repeats.");
	pList->AddTail( fxProp );

	fxProp.SetupEnum("DistanceNode", "ObjectPos", "ObjectPos,Node1,Node2", eCurve_None, "This indicates which node should be used for determining the distance that will be used for the texture coordinate generation. For example, when tracking an elbow hinging up, you most likely would want to have the distance of the hand node generate distance since the elbow is not going to move much");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("SingleNodeWidth", 20.0f, 0.0f, eCurve_None, "This indicates the width that will be used for the polygon trail when no additional nodes are being tracked");
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Samples");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("SampleFrequency", 0.2f, 0.0f, eCurve_None, "The frequency at which samples will be taken from the nodes. This will never sample at a rate higher than the frame rate.");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("SampleLifetime", 2.0f, 0.0f, eCurve_None, "The duration in seconds that the samples will live for.");
	pList->AddTail( fxProp );

	fxProp.SetupColor("SampleColor", 0xFFFFFFFF, eCurve_Linear, "The color of samples as they progress through their lifetime. Typically this should end with alpha being zero otherwise the ending segments can be seen popping out.");
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Tracked Nodes");
	pList->AddTail(fxProp);

	for(uint32 nCurrTracked = 1; nCurrTracked < CPolyTrailProps::knMaxTrackedNodes; nCurrTracked++)
	{
		char szPropName[64];

		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "TrackedNode%d", nCurrTracked);
		fxProp.SetupString(szPropName, "", eCurve_None, "Indicates an additional node that should be tracked in addition to the main object position. This can be left blank to indicate that no node should be tracked.");
		pList->AddTail( fxProp );

		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "NodeOffset%d", nCurrTracked);
		fxProp.SetupVector(szPropName, LTVector::GetIdentity(), eCurve_None, "Indicates the offset within the node space to use for the tracked point.");
		pList->AddTail( fxProp );

		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "UTexCoord%d", nCurrTracked);
		fxProp.SetupFloat(szPropName, 1.0f, eCurve_None, "The U texture coordinate that will be used when rendering samples from this node. This should be greater than previous nodes that are tracked.");
		pList->AddTail( fxProp );
	}
}

