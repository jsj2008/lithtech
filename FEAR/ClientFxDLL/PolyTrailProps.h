//---------------------------------------------------------------------------------
// PolyTrailProps.h
//
// This file contains all of the property information for an appropriate poly trail
// effect. 
//
//---------------------------------------------------------------------------------

#ifndef __POLYTRAILPROPS_H__
#define __POLYTRAILPROPS_H__

#ifndef __BASEFX_H__
#	include "BaseFx.h"
#endif

#ifndef __CLIENTFXSKYUTILS_H__
#	include "ClientFXSkyUtils.h"
#endif

class CPolyTrailProps : 
	public CBaseFXProps
{
public:

	enum {	knMaxTrackedNodes	= 3 };

	CPolyTrailProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//called after all the properties have been loaded to perform any custom initialization
	virtual bool PostLoadProperties();

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//the name of the material associated with this poly trail
	const char*				m_pszMaterial;

	//whether or not this lightning should be rendered solid
	bool					m_bRenderSolid;

	//whether or not this lightning should receive translucent lighting
	bool					m_bTranslucentLight;

	//should we have this object render in the player rendering layer?
	bool					m_bPlayerView;

	//should this effect be placed in the sky
	LTEnum<uint8, EFXSkySetting>	m_eInSky;

	//the node that is used for tracking distance
	uint8					m_nDistanceNode;

	//the width that is used when tracking a single node
	float					m_fSingleNodeWidth;

	//the frequency at which samples are taken, measured in seconds
	float					m_fSampleFrequencyS;

	//the the lifetime of the samples measured in seconds
	float					m_fSampleLifetimeS;

	//the distance that a single repeat of a texture consumes
	float					m_fTextureLength;

	//the color of a bolt over its lifetime
	TColor4fFunctionCurveI	m_cfcSampleColor;

	//this contains all the information for a single tracked node (note that this is not used for the
	//first node, which is what our object is attached to)
	struct STrackedNode
	{
		STrackedNode();

		//the name of the node being tracked
		const char*		m_pszNodeName;

		//the offset in the node space
		LTVector		m_vTrackedOffset;

		//the U coordinate to use for this node
		float			m_fUCoord;
	};

	//the additional nodes/sockets that can be tracked (the first one is our parent object pos, so don't
	//track that one)
	STrackedNode			m_TrackedNodes[knMaxTrackedNodes - 1];
};

#endif
