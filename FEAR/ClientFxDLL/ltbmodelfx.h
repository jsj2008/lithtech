//------------------------------------------------------------------
//
//   MODULE  : LTBMODELFX.H
//
//   PURPOSE : Defines class CLTBModelFX
//
//   CREATED : On 12/3/98 At 6:34:45 PM
//
//------------------------------------------------------------------

#ifndef __LTBMODELFX_H__
#define __LTBMODELFX_H__

// Includes....

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

#ifndef __CLIENTFXSKYUTILS_H__
#	include "ClientFXSkyUtils.h"
#endif

class CLTBModelProps : public CBaseFXProps
{
public:

	enum	{ knNumModelMaterials = 5 };

	CLTBModelProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	TColor4fFunctionCurveI			m_cfcModelColor;	// The color of the model over its lifetime
	TFloatFunctionCurveI			m_ffcModelScale;	// The scale of the model over its lifetime

	const char*						m_pszModelName;
	const char*						m_pszMaterialName[knNumModelMaterials];
	const char*						m_pszAnimName;
	const char*						m_pszAttachEffect;
	LTEnum<uint8, EEngineLOD>		m_eShadowLOD;
	LTEnum<uint8, EFXSkySetting>	m_eInSky;
	bool							m_bOverrideAniLength;
	bool							m_bSyncToKey;
	bool							m_bForceTranslucent;
	bool							m_bTranslucentLight;
	bool							m_bRotate;

	//should we have this object render in the player rendering layer?
	bool							m_bPlayerView;

	float							m_fAniLength;
};

class CLTBModelFX : public CBaseFX
{
public :

	// Constuctor
	CLTBModelFX();
	~CLTBModelFX();

	// Member Functions
	virtual bool Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
	virtual bool Update(float tmCur);
	virtual void Term();
	virtual void EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData);

	virtual bool IsFinishedShuttingDown(); 

protected :

	const CLTBModelProps*			GetProps() { return (const CLTBModelProps*)m_pProps; }

	// Member Variables
	float		m_fAniRate;

	//our model object
	HOBJECT		m_hObject;
};

#endif