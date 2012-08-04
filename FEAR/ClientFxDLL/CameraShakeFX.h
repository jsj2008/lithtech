//--------------------------------------------------------------------------------
// CameraShakeFX.h
//
// Provides the definition for the camera shake effect which allows for generation
// of random keyframes based upon a specified frequency that it will interpolate
// the camera through to create a chaotic shaking effect that is framerate
// independant and highly artist controlable. For more information please refer
// to the ClientFX Camera Effects documentation.
//
//--------------------------------------------------------------------------------
#ifndef __CAMERASHAKEFX_H__
#define __CAMERASHAKEFX_H__

#ifndef __BASEFX_H__
#	include "basefx.h"
#endif

//----------------------------------------------------------------
// CCameraShakeProps
// The properties for the camera shake effect
//----------------------------------------------------------------
class CCameraShakeProps :
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

	CCameraShakeProps();
	virtual ~CCameraShakeProps();

	//handles loading up a single property from the specified file
	virtual bool LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData);

	//indicates whether or not the radius should be used, or if this should
	//just be a global effect
	bool					m_bUseRadius;

	//the falloff curve to use as the viewer approaches the radius
	EFalloffCurve			m_eFalloff;

	//the radius to use for this effect
	TFloatFunctionCurveI	m_ffcRadius;

	//the frequency at which to generate new random keyframes
	TFloatFunctionCurveI	m_ffcFrequency;

	//the overall intensity of the effect so that artists can more easily blend it in and out
	TFloatFunctionCurveI	m_ffcIntensity;

	//the range for the position offset of the camera, these are in half dims
	TVectorFunctionCurveI	m_vfcMaxPositionOffset;

	//the range in euler degrees to allow for each angle relative to the base transform. These
	//are half angles and are symmetrical around the appropriate axis
	TVectorFunctionCurveI	m_vfcMaxRotationDeg;

	//the field of view scale
	TFloatFunctionCurveI	m_ffcMinFovXScale;
	TFloatFunctionCurveI	m_ffcMaxFovXScale;
	TFloatFunctionCurveI	m_ffcMinFovYScale;
	TFloatFunctionCurveI	m_ffcMaxFovYScale;
};

//called to get the appropriate properties for the camera shake
void fxGetCameraShakeProps(CFastList<CEffectPropertyDesc> *pList);

//----------------------------------------------------------------
class CCameraShakeFX :
	public CBaseFX,
	public IClientFXCamera
{
public:

	CCameraShakeFX();
	virtual ~CCameraShakeFX();

		//initializes the effect based upon the passed in data
	virtual bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps);

	//terminates the effect
	virtual void	Term();

	//called to update the effect
	virtual bool	Update(float tmFrameTime);

	//This version of update is called while the effect is suspended so that it can do
	//things like smooth shutdown depending upon the effect type
	virtual bool	SuspendedUpdate(float tmFrameTime);

	//called in response to a query for a modifier to a camera transform, in order to allow 
	//effects to provide a mechanism to influence cameras. This should return true if the 
	//output data should be used. tOutTrans is the relative offset to apply to the camera, and
	//vOutFov is the relative scale for the FOV
	virtual bool	GetCameraModifier(	const LTRigidTransform& tCameraTrans,
										LTRigidTransform& tOutTrans,
										LTVector2& vOutFov);

private:

	//a structure to represent a single evaluated key frame
	struct SKeyFrame
	{
		//the relative transform of this keyframe
		LTRigidTransform	m_tTransform;

		//the field of view scale of this keyframe
		LTVector2			m_vFovScale;
	};

	//utility function that will calculate the intensity to use for the camera shake. This will return
	//false if no shake should be applied at all
	bool GetShakeIntensity(float fUnitLifetime, const LTRigidTransform& tCameraTrans, float& fIntensity);

	//called to generate a keyframe given the keyframe, assuming that the keyframe has an appropriate
	//time setup to use
	void GenerateKeyFrame(float fKeyTime, SKeyFrame& Keyframe);

	//provide type safe access to our properties
	const CCameraShakeProps* GetProps() const	{ return (CCameraShakeProps*)m_pProps; }

	//our two keyframes that we are interpolating between. There are three keys, the current key
	//which is the start of the segment, the next key, which is the end of the segment, and the middle
	//key which is another random key that is used to create a fake curve to hide the linear nature
	SKeyFrame	m_CurrKey;
	SKeyFrame	m_MidKey;
	SKeyFrame	m_NextKey;

	//the time that the the start and end keys are positioned on (the mid key is between these)
	float m_fCurrTime;
	float m_fNextTime;

	//whether or not each keyframe is valid (invalid keyframes will be regenerated, and the
	//middle key is dirtied with the next key)
	bool m_bCurrValid;
	bool m_bNextValid;

	//our link for our camera service
	LTLink<IClientFXCamera*>	m_CameraLink;
};

#endif


