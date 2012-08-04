//------------------------------------------------------------------
//
//   MODULE  : DYNALIGHTFX.H
//
//   PURPOSE : Defines class CDynaLightFX
//
//   CREATED : On 12/14/98 At 5:43:43 PM
//
//------------------------------------------------------------------

#ifndef __DYNALIGHTFX_H__
#define __DYNALIGHTFX_H__

// Includes....

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

#ifndef __CLIENTFXPROP_H__
#	include "ClientFxProp.h"
#endif

#ifndef __CLIENTFXSKYUTILS_H__
#	include "ClientFXSkyUtils.h"
#endif

class CDynaLightProps : public CBaseFXProps
{
public:

	CDynaLightProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//the texture associated with this light
	const char*				m_pszTexture;

	//the level of details associated with this light
	LTEnum<uint8, EEngineLOD>		m_eLightLOD;
	LTEnum<uint8, EEngineLOD>		m_eWorldShadowsLOD;
	LTEnum<uint8, EEngineLOD>		m_eObjectShadowsLOD;

	//whether or not this effect is in the sky
	LTEnum<uint8, EFXSkySetting>	m_eInSky;

	//the transluecent color for this light
	TColor4fFunctionCurveI	m_cfcTranslucentColor;

	//The type of this light
	TEnumFunctionCurve		m_efcType;			

	//The radius of the light	
	TFloatFunctionCurveI	m_ffcRadius;		

	//The color of this light
	TColor4fFunctionCurveI	m_cfcColor;			

	//The intensity of the light
	TFloatFunctionCurveI	m_ffcIntensity;		

	//The amount to flicker the intensity
	TFloatFunctionCurveI	m_ffcFlickerScale;		

	//The specular color
	TColor4fFunctionCurveI	m_cfcSpecularColor;

	//Spotlight field of view
	TFloatFunctionCurveI	m_ffcSpotFovX;
	TFloatFunctionCurveI	m_ffcSpotFovY;
};

class CDynaLightFX : public CBaseFX
{
public :

	// Constuctor
	CDynaLightFX();
   ~CDynaLightFX();

   // Member Functions
	virtual bool Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps);
	virtual void Term();
	virtual bool Update(float tmCur);
	virtual void EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData);

protected :

	//handles updating the properties of the light given the specified unit time value
	void						UpdateDynamicProperties(float fUnitTime);

	const CDynaLightProps*		GetProps()		{ return (const CDynaLightProps*)m_pProps; }

	//our light object
	HOBJECT		m_hLight;
};

#endif