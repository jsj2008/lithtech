//------------------------------------------------------------------
//
//   MODULE  : TracerFX.H
//
//   PURPOSE : Provides a tracer effect intended for guns. This is essentially
//				a camera aligned sprite that will travel down a ray defined by
//				the forward of the orientation and create a quad that travels
//				the length.
//
//   CREATED : On 11/23/98 At 6:21:38 PM
//
//------------------------------------------------------------------

#ifndef __TRACERFX_H__
#define __TRACERFX_H__

// Includes....

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

#ifndef __ILTCUSTOMRENDER_H__
#	include "iltcustomrender.h"
#endif


class CTracerProps : public CBaseFXProps
{
public:

	CTracerProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//this is called to collect the resources associated with these properties. For more information
	//see the IFXResourceCollector interface.
	virtual void CollectResources(IFXResourceCollector& Collector);

	//Tracer Properties

	//the material for the tracer
	const char*		m_pszMaterial;

	//the length of the actual tracer in world units
	float			m_fLength;

	//the thickness of the tracer in world units over the duration of the effet
	TFloatFunctionCurveI	m_ffcThickness;

	//the color of the tracer over the duration of the effet
	TColor4fFunctionCurveI	m_cfcColor;

	//the speed that the tracer moves in the direction in world units per second
	float			m_fVelocity;

	//the maximum distance that this tracer can be shot
	float			m_fMaxDistance;

	//should the tracer crop the texture or scale it when clipped to the ray?
	bool			m_bCropTexture;

	//solid or not?
	bool			m_bSolid;

	//should this be lit when translcuent
	bool			m_bTranslucentLight;

	//should we set the length of this tracer to be that of the distance it will travel?
	bool			m_bFitLengthToRay;

	//should we start out with our tracer fully extended
	bool			m_bStartEmitted;

	//when this is set, the tracer will go to the target position instead of along the forward axis
	bool			m_bUseTargetPos;

	//when this is set the tracer will be blocked by geometry between the target
	bool			m_bBlockedByGeometry;

	//should we have this object render in the player rendering layer?
	bool			m_bPlayerView;

	//Dynamic Light Properties

	//the LOD settings for the light
	LTEnum<uint8, EEngineLOD>	m_eLightLOD;
	LTEnum<uint8, EEngineLOD>	m_eWorldShadowsLOD;
	LTEnum<uint8, EEngineLOD>	m_eObjectShadowsLOD;

	//the radius of the dynamic light
	float			m_fLightRadius;

	//the color of the dynamic light
	LTVector4		m_vLightColor;
};

class CTracerFX : public CBaseFX
{
public :

	CTracerFX();
	virtual ~CTracerFX();

	// Member Functions

	virtual bool	Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps);
	virtual bool	Update(float tmCur);
	virtual void	Term();
	virtual void	EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData);

	virtual bool	IsFinishedShuttingDown();

private:

	//called to determine the length of the tracer. This assumes that the ray has already been cast prior
	//to this call
	float		GetTracerLength() const	{ return GetProps()->m_bFitLengthToRay ? m_fRayLength : GetProps()->m_fLength; }

	//hook for the custom render object, this will just call into the render function
	static void CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser);

	//function that handles the actual custom rendering
	void RenderTracer(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera);

	const CTracerProps*	GetProps() const { return (const CTracerProps*)m_pProps; }

	//our custom render object
	HOBJECT			m_hObject;

	//the position of the tracer along the ray in world units
	float			m_fRayPosition;

	//the total distance that this tracer can travel
	float			m_fRayLength;

	//the starting position of the tracer
	LTVector		m_vStartPos;

	//the direction of the ray
	LTVector		m_vDirection;

	//the dynamic light attached to this tracer
	HOBJECT			m_hTracerLight;
};

//function that will add all the base sprite properties
void fxGetTracerProps(CFastList<CEffectPropertyDesc> *pList);

#endif