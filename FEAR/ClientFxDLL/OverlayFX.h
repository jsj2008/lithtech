//--------------------------------------------------------------------------------
// OverlayFX.h
//
// Provides the definition for the overlay effect which allows for rendering of
// a material over the entire surface of a render target. This handles controlling
// color and material properties as well as proper aspect ratio handling for the
// overlays
//
//--------------------------------------------------------------------------------
#ifndef __OVERLAYFX_H__
#define __OVERLAYFX_H__

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

//----------------------------------------------------------------
// COverlayProps
// The properties for the overlay effect
//----------------------------------------------------------------
class COverlayProps :
	public CBaseFXProps
{
public:

	COverlayProps();
	virtual ~COverlayProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//the layer that this overlay is associated with
	uint32					m_nLayer;

	//whether or not to allow higher layers
	bool					m_bAllowHigherLayers;

	//the color of the overlay over the overlay lifetime
	TColor4fFunctionCurveI	m_cfcColor;

	//the material to use for the primary overlay
	const char*				m_pszOverlayMaterial;

	//the material to use for the edges around the overlay
	const char*				m_pszEdgeMaterial;

	//the dimensions that the overlay should use (only used for aspect ratio determination). Zero
	//for either indicates that the render target aspect ratio should be used
	TFloatFunctionCurveI	m_ffcAspectWidth;
	TFloatFunctionCurveI	m_ffcAspectHeight;

	//the percentage in the Y direction that the overlay should occupy [0..1]
	TFloatFunctionCurveI	m_ffcYHeight;

	//the listing of the material parameters that can be effected
	struct SFloatParam
	{
		SFloatParam();

		//the name of the parameter that this maps to
		const char*				m_pszMaterialParam;

		//the value to use for this parameter over the overlay's lifetime
		TFloatFunctionCurveI	m_ffcParamValue;
	};

	//the number of floating point parameters we support
	enum { knNumFloatParams		= 2 };

	//our listing of floating point parameters
	SFloatParam			m_FloatParams[knNumFloatParams];
};

//called to get the appropriate properties for the overlay effect
void fxGetOverlayProps(CFastList<CEffectPropertyDesc> *pList);

//----------------------------------------------------------------
class COverlayFX :
	public CBaseFX,
	public IClientFXOverlay
{
public:

	COverlayFX();
	virtual ~COverlayFX();

	//initializes the effect based upon the passed in data
	virtual bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps);

	//terminates the effect
	virtual void	Term();

	//called to update the effect
	virtual bool	Update(float tmFrameTime);

	//called to determine if this effect supports overlays. If it doesn't, it should return false. If it
	//does, then it should provide the layer value and whether or not it allows higher layers to be displayed
	virtual bool	GetOverlayLayer(uint32& nLayer, bool& bAllowHigher);

	//called to render the overlay. This should return false if the effect does not support overlays
	virtual bool	RenderOverlay();


private:

	//provide type safe access to our properties
	const COverlayProps* GetProps() const	{ return (COverlayProps*)m_pProps; }

	//called to render the overlay to the current render target
	void			RenderOverlay(float fUnitLifetime);

	//called to update the parameters assocaited with the materials
	void			UpdateMaterialParams(float fUnitLifetime);

	//the main overlay material
	HMATERIAL		m_hOverlayMaterial;

	//the edge material to fill the rest of the screen
	HMATERIAL		m_hEdgeMaterial;

	//the link for our overlay service
	LTLink<IClientFXOverlay*>	m_OverlayLink;
};

#endif


