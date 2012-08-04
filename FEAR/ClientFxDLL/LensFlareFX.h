//------------------------------------------------------------------
//
//   MODULE  : LensFlareFX.H
//
//   PURPOSE : Provides a base class for the different sprite effects
//				to derive from and get common functionality
//
//   CREATED : On 11/23/98 At 6:21:38 PM
//
//------------------------------------------------------------------

#ifndef __LENSFLAREFX_H__
#define __LENSFLAREFX_H__

// Includes....

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

#ifndef __ILTCUSTOMRENDER_H__
#	include "iltcustomrender.h"
#endif

#ifndef __CLIENTFXSKYUTILS_H__
#	include "ClientFXSkyUtils.h"
#endif

class CLensFlareProps : public CBaseFXProps
{
public:

	//the maximum number of lens flare components that are allowed
	enum {	knMaxComponents	= 16 };

	CLensFlareProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//called after all the properties have been loaded to perform any custom initialization
	virtual bool PostLoadProperties();

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//the material that handles the rendering of this lens flare, also associates the image
	//texture, which is a horizontal strip of frames
	const char*				m_pszMaterial;

	//should this effect be placed in the sky
	EFXSkySetting			m_eInSky;

	//determine if this is a directional lens flare (strongest intensity when looking directly at it)
	bool					m_bDirectional;

	//the visible radius of the lens flare. If this sphere is not visible, the lens flare will not be
	//rendered
	float					m_fVisibleRadius;

	//the color of the lens flare
	TColor4fFunctionCurveI	m_Color;

	//the maximum distance that the lens flare is visible from
	TFloatFunctionCurveI	m_VisibleDistance;

	//the number of images within the texture, laid out in a horizontal strip
	uint32					m_nNumImages;

	//the number of components used
	uint32					m_nNumComponents;

	//the definition of each lens flare component that we have
	struct SFlareComponent
	{
		SFlareComponent();

		//the index into the image of the texture
		uint32					m_nImage;

		//the scale of this flare component
		float					m_fScale;

		//the offset along the vector that goes from the screen center to the flare center (can be negative)
		float					m_fOffset;

		//the color of this component (full RGBA)
		LTVector4				m_vColor;
	};

	//the individual lens flare components
	SFlareComponent			m_Components[knMaxComponents];
};

class CLensFlareFX : public CBaseFX
{
public :

	CLensFlareFX();
	virtual ~CLensFlareFX();

	// Member Functions

	virtual bool Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
	virtual bool Update(float tmCur);
	virtual void Term();
	virtual void EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData);

private:

	//hook for the custom render object, this will just call into the render function
	static void				CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser);
	static bool				CustomRenderVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera, void* pUser);

	//function that handles the visible callback
	bool					HandleVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera);

	//function that handles the actual custom rendering
	void					RenderLensFlare(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera);

	const CLensFlareProps*	GetProps() { return (const CLensFlareProps*)m_pProps; }

	//the current alpha to use when rendering the lens flare, calculated in the visible callback
	//prior to rendering
	float					m_fFlareAlpha;

	//our custom render object
	HOBJECT					m_hObject;

};

//function that will add all the base sprite properties
void fxGetLensFlareProps(CFastList<CEffectPropertyDesc> *pList);

#endif