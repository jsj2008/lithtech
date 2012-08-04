//--------------------------------------------------------------------------------
// RumbleFX.h
//
// Provides the definition for the camera shake effect which allows for generation
// of random keyframes based upon a specified frequency that it will interpolate
// the camera through to create a chaotic shaking effect that is framerate
// independant and highly artist controlable. For more information please refer
// to the ClientFX Camera Effects documentation.
//
//--------------------------------------------------------------------------------
#ifndef __RUMBLEFX_H__
#define __RUMBLEFX_H__

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

//----------------------------------------------------------------
// CRumbleProps
// The properties for the camera shake effect
//----------------------------------------------------------------
class CRumbleProps :
	public CBaseFXProps
{
public:

	//an enumeration that represents the different falloff methods
	enum EFalloffCurve
	{
		eFalloff_Linear,
		eFalloff_Quartic,
		eFalloff_Constant
	};

	CRumbleProps();
	virtual ~CRumbleProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//indicates whether or not the radius should be used, or if this should
	//just be a global effect
	bool					m_bUseRadius;

	//the falloff curve to use as the viewer approaches the radius
	EFalloffCurve			m_eFalloff;

	//the radius to use for this effect
	TFloatFunctionCurveI	m_ffcRadius;

	//the overall intensity of the effect so that artists can more easily blend it in and out
	TFloatFunctionCurveI	m_ffcIntensity[NUM_CLIENTFX_CONTROLLER_MOTORS];
};

//called to get the appropriate properties for the camera shake
void fxGetRumbleProps(CFastList<CEffectPropertyDesc> *pList);

//----------------------------------------------------------------
class CRumbleFX :
	public CBaseFX,
	public IClientFXController
{
public:

	CRumbleFX();
	virtual ~CRumbleFX();

		//initializes the effect based upon the passed in data
	virtual bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps);

	//terminates the effect
	virtual void	Term();

	//called to update the effect
	virtual bool	Update(float tmFrameTime);

	//This version of update is called while the effect is suspended so that it can do
	//things like smooth shutdown depending upon the effect type
	virtual bool	SuspendedUpdate(float tmFrameTime);

	//called in response to a query for the intensity that should be used for the controller motors
	//and allows this effect to influence the motors appropriately. Motor intensities range from [0..1]
	//but this function should only accumulate and should not clamp in order to allow for interactions.
	virtual void	GetControllerModifier(	const LTRigidTransform& tCameraTrans,
											float fMotorIntensity[NUM_CLIENTFX_CONTROLLER_MOTORS]);

private:

	//provide type safe access to our properties
	const CRumbleProps* GetProps() const	{ return (CRumbleProps*)m_pProps; }

	//our link for the controller service
	LTLink<IClientFXController*>	m_ControllerLink;
};

#endif


