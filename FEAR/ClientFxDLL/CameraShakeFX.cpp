#include "stdafx.h"
#include "CameraShakeFX.h"
#include "clientfx.h"


//-------------------------------------------------------------------------------------
// CCameraShakeProps
//-------------------------------------------------------------------------------------

CCameraShakeProps::CCameraShakeProps() :
	m_bUseRadius(true),
	m_eFalloff(eFalloff_Linear)
{
}

CCameraShakeProps::~CCameraShakeProps()
{
}

//handles loading up a single property from the specified file
bool CCameraShakeProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	//-------------------------
	// Effect Intensity

	if( LTStrIEquals( pszName, "Intensity" ) )
	{
		m_ffcIntensity.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "UseRadius" ))
	{
		m_bUseRadius = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Radius" ))
	{
		m_ffcRadius.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "Falloff" ))
	{
		m_eFalloff = (EFalloffCurve)CFxProp_Enum::Load(pStream);
	}

	//-------------------------
	// Keyframe Generation

	else if( LTStrIEquals( pszName, "Frequency" ))
	{
		m_ffcFrequency.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MaxPositionOffset" ))
	{
		m_vfcMaxPositionOffset.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MaxRotationDeg" ))
	{
		m_vfcMaxRotationDeg.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MinFovXScale" ))
	{
		m_ffcMinFovXScale.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MaxFovXScale" ))
	{
		m_ffcMaxFovXScale.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MinFovYScale" ))
	{
		m_ffcMinFovYScale.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "MaxFovYScale" ))
	{
		m_ffcMaxFovYScale.Load(pStream, pCurveData);
	}

	//-------------------------
	// Base Properties

	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}


//called to get the appropriate properties for the camera shake
void fxGetCameraShakeProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc	fxProp;

	// Add the generic "every effect has these" props
	AddBaseProps( pList );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Effect Intensity");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMinMax("Intensity", 1.0f, 0.0f, 1.0f, eCurve_Linear, "Specifies the overall intensity of the effect to be applied. This is blended by the intensity of the distance falloff");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "UseRadius", true, eCurve_None, "Determines whether or not the radius of the effect should be used, or if this should just be applied at full intensity regardless of where the camera is." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("Radius", 100.0f, 0.0f, eCurve_Linear, "Specifies the radius of this effect which is the area that it will shake the camera. This is ignored if UseRadius is false");
	pList->AddTail( fxProp );

	fxProp.SetupEnum("Falloff", "Linear", "Linear,Quartic,Constant", eCurve_None, "Specifies how the intensity should drop off as the camera moves away from the effect center towards the radius");
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Keyframe Generation");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("Frequency", 0.1f, 0.0f, eCurve_Linear, "Indicates the amount of time between generating keyframes in seconds. For example, 0.1 would generate a keyframe every tenth of a second.");
	pList->AddTail( fxProp );

	fxProp.SetupVectorMin("MaxPositionOffset", LTVector::GetIdentity(), 0.0f, eCurve_Linear, "This is the amount that the position can be offset in each direction. These are half dimensions, so for example 3.0 on X would allow the X to be offset between -3.0 and 3.0.");
	pList->AddTail( fxProp );

	fxProp.SetupVectorMinMax("MaxRotationDeg", LTVector::GetIdentity(), 0.0f, 180.0f, eCurve_Linear, "This is the maximum rotation around each axis measured in degrees. This is symmetrical around the axis, so 30 for X will allow rotation around X from -30 to 30");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("MinFovXScale", 1.0f, 0.0f, eCurve_Linear, "The minimum FOV scale for a given time. This is a scale of the FOV so 1.0 means no change, 0.9 means tighter field of view, and 1.1 means larger field of view");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("MaxFovXScale", 1.0f, 0.0f, eCurve_Linear, "The maximum FOV scale for a given time. This is a scale of the FOV so 1.0 means no change, 0.9 means tighter field of view, and 1.1 means larger field of view");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("MinFovYScale", 1.0f, 0.0f, eCurve_Linear, "The minimum FOV scale for a given time. This is a scale of the FOV so 1.0 means no change, 0.9 means tighter field of view, and 1.1 means larger field of view");
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("MaxFovYScale", 1.0f, 0.0f, eCurve_Linear, "The maximum FOV scale for a given time. This is a scale of the FOV so 1.0 means no change, 0.9 means tighter field of view, and 1.1 means larger field of view");
	pList->AddTail( fxProp );


}

//-------------------------------------------------------------------------------------
// CCameraShakeFX
//-------------------------------------------------------------------------------------

CCameraShakeFX::CCameraShakeFX() :
	CBaseFX(eCameraShakeFX)
{
	m_CameraLink.SetData(this);
}

CCameraShakeFX::~CCameraShakeFX()
{
	Term();
}

//initializes the effect based upon the passed in data
bool CCameraShakeFX::Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps)
{
	if(!CBaseFX::Init(pData, pProps))
		return false;

	m_fCurrTime = 0.0f;
	m_fNextTime = 0.0f;

	m_bCurrValid = false;
	m_bNextValid = false;

	return true;
}

//terminates the effect
void CCameraShakeFX::Term()
{
	m_CameraLink.Remove();
}

//called to update the effect
bool CCameraShakeFX::Update(float tmFrameTime)
{
	BaseUpdate(tmFrameTime);

	//since we can have shut down and init on the same frame in rare cases, make sure to not handle
	//the initial frame if we are shutting down
	if(IsShuttingDown())
		m_CameraLink.Remove();
	else if(IsInitialFrame())
		m_pFxMgr->SubscribeCamera(m_CameraLink);

	//determine the key generation frequency
	float fUnitLifetime		= GetUnitLifetime();
	float fKeyGenFrequency	= GetProps()->m_ffcFrequency.GetValue(fUnitLifetime);

	//determine the current time
	float fCurrTime = GetElapsed();

	//we need to properly detect loops. These can be detected by seeing that the current time is less than
	//our current keys time. In such a case, we can just pull the keys back by one loop
	if(fCurrTime < m_fCurrTime)
	{
		m_fCurrTime -= GetLifetime();
		m_fNextTime -= GetLifetime();
	}

	//if we have extended beyond our next keyframe, we need to make it the current one
	if(fCurrTime > m_fNextTime)
	{
		//but see if we can even invalidate the current key (it is past our generation time)
		if(fCurrTime > m_fNextTime + fKeyGenFrequency)
		{
			//we can't even use this frame since time has advanced past when we would use it
			m_fCurrTime = fCurrTime;
			m_bCurrValid = false;

			m_fNextTime = fCurrTime + fKeyGenFrequency;
			m_bNextValid = false;
		}
		else
		{
			//the current key becomes the next key, and the next key becomes a key that we need
			//to generate when needed
			m_CurrKey = m_NextKey;
			m_fCurrTime = m_fNextTime;
			m_bCurrValid = m_bNextValid;

			m_fNextTime = m_fCurrTime + fKeyGenFrequency;
			m_bNextValid = false;
		}
	}

	return true;
}

//This version of update is called while the effect is suspended so that it can do
//things like smooth shutdown depending upon the effect type
bool CCameraShakeFX::SuspendedUpdate(float tmFrameTime)
{
	return CBaseFX::SuspendedUpdate(tmFrameTime);
}

//called in response to a query for a modifier to a camera transform, in order to allow 
//effects to provide a mechanism to influence cameras. This should return true if the 
//output data should be used. tOutTrans is the relative offset to apply to the camera, and
//vOutFov is the relative scale for the FOV
bool CCameraShakeFX::GetCameraModifier(	const LTRigidTransform& tCameraTrans,
										LTRigidTransform& tOutTrans,
										LTVector2& vOutFov)
{
	if(!IsActive() || IsSuspended())
		return false;

    float fUnitLifetime = GetUnitLifetime();

	//determine the intensity of this shake
	float fIntensity = 1.0f;
	if(!GetShakeIntensity(fUnitLifetime, tCameraTrans, fIntensity))
		return false;

	//handle generation of our keyframes if they are invalid
	if(!m_bCurrValid)
	{
		//generate the starting segment of this line
		GenerateKeyFrame(m_fCurrTime, m_CurrKey);
		m_bCurrValid = true;
	}
    if(!m_bNextValid)
	{
		//generate the keyframes that we will interpolate through/to
		GenerateKeyFrame((m_fCurrTime + m_fNextTime) * 0.5f, m_MidKey);
		GenerateKeyFrame(m_fNextTime, m_NextKey);
		m_bNextValid = true;
	}

	//determine the amount we need to interpolate between our keys
    float fInterp = 0.0f;

	float fKeyDelta = m_fNextTime - m_fCurrTime;
	if(fKeyDelta > 0.001f)
        fInterp = (GetElapsed() - m_fCurrTime) / fKeyDelta;

	//allow for easing in and out
	float fInterpBase = (1.0f - fInterp * fInterp);
	fInterp = 1.0f - fInterpBase * fInterpBase;

	//determine our keyframe by interpolating between the keys (optimize for the frequent case
	//where we are very close to one or the other ends)
	static const float kfMinInterp = 0.001f;
	if(fInterp < kfMinInterp)
	{
		tOutTrans = m_CurrKey.m_tTransform;
		vOutFov = m_CurrKey.m_vFovScale;
	}
	else if(fInterp > 1.0f - kfMinInterp)
	{
		tOutTrans = m_NextKey.m_tTransform;
		vOutFov = m_NextKey.m_vFovScale;
	}
	else
	{
		//we will solve this bezier curve using a recursive solving approach, which recursively divides
		//the curve to solve

		//Transform curve
		LTRigidTransform tMid1, tMid2;

		//intermediate segments
		tMid1.Interpolate(m_CurrKey.m_tTransform, m_MidKey.m_tTransform, fInterp);
		tMid2.Interpolate(m_MidKey.m_tTransform, m_NextKey.m_tTransform, fInterp);

		//final segment
		tOutTrans.Interpolate(tMid1, tMid2, fInterp);

		//Fov curve
		LTVector2 vMid1, vMid2;

		//intermediate segments
		vMid1 = m_CurrKey.m_vFovScale.Lerp(m_MidKey.m_vFovScale, fInterp);
		vMid2 = m_MidKey.m_vFovScale.Lerp(m_NextKey.m_vFovScale, fInterp);

		//final segment
		vOutFov = vMid1.Lerp(vMid2, fInterp);
	}

	//and now we need to handle blending between the neutral position and our desired position based
	//upon the intensity
	if(fIntensity <= 1.0f - kfMinInterp)
	{
		tOutTrans.Interpolate(LTRigidTransform::GetIdentity(), tOutTrans, fIntensity);
		vOutFov = LTVector2(1.0f, 1.0f).Lerp(vOutFov, fIntensity);
	}

	return true;
}

//utility function that will calculate the intensity to use for the camera shake. This will return
//false if no shake should be applied at all
bool CCameraShakeFX::GetShakeIntensity(float fUnitLifetime, const LTRigidTransform& tCameraTrans, float& fIntensity)
{
	//get the amount of intensity to apply to this camera
	fIntensity = GetProps()->m_ffcIntensity.GetValue(fUnitLifetime);
	if(fIntensity < 0.001f)
		return false;

	//and now we need to determine the falloff as it moves away from the camera
	if(GetProps()->m_bUseRadius)
	{
		//get the position of the effect
		LTRigidTransform tFxTrans;
		GetCurrentTransform(fUnitLifetime, tFxTrans.m_vPos, tFxTrans.m_rRot);

		//and the radius
		float fRadius = GetProps()->m_ffcRadius.GetValue(fUnitLifetime);
		float fRadiusSqr = fRadius * fRadius;

		//now determine if we are outside of the radius
		float fDistSqr = tCameraTrans.m_vPos.DistSqr(tFxTrans.m_vPos);
		if(fDistSqr >= fRadiusSqr)
			return false;

		//now apply the appropriate falloff curve
		switch(GetProps()->m_eFalloff)
		{
		case CCameraShakeProps::eFalloff_Linear:
			{
				fIntensity *= 1.0f - sqrtf(fDistSqr) / fRadius;
			}
			break;
		case CCameraShakeProps::eFalloff_Quartic:
			{
				float fFalloff = 1.0f - fDistSqr / fRadiusSqr;
				fIntensity *= fFalloff * fFalloff;
			}
			break;
		case CCameraShakeProps::eFalloff_Constant:
			{
			}
			break;
		}

		//handle near zero intensity
		if(fIntensity < 0.001f)
			return false;
	}

	//success
	return true;
}

//called to generate a keyframe given the keyframe, assuming that the keyframe has an appropriate
//time setup to use
void CCameraShakeFX::GenerateKeyFrame(float fKeyTime, SKeyFrame& Keyframe)
{
	float fUnitLifetime = fKeyTime / GetLifetime();
	fUnitLifetime = fmodf(fUnitLifetime, 1.0f);

	//first off, generate the positional offset
	LTVector vPosRange = GetProps()->m_vfcMaxPositionOffset.GetValue(fUnitLifetime);
	Keyframe.m_tTransform.m_vPos.Init(	GetRandom(-vPosRange.x, vPosRange.x),
										GetRandom(-vPosRange.y, vPosRange.y),
										GetRandom(-vPosRange.z, vPosRange.z));

	//and now generate the rotational offset
	LTVector vAngleRange = GetProps()->m_vfcMaxRotationDeg.GetValue(fUnitLifetime);

	LTVector vAngles(	GetRandom(-vAngleRange.x, vAngleRange.x),
						GetRandom(-vAngleRange.y, vAngleRange.y),
						GetRandom(-vAngleRange.z, vAngleRange.z));

	//convert to radians
	vAngles *= MATH_PI / 180.0f;

	//and convert to a quaternion representation
	Keyframe.m_tTransform.m_rRot = LTRotation(VEC_EXPAND(vAngles));

	//and finally generate the field of view for this keyframe
	float fXScale = GetRandom(	GetProps()->m_ffcMinFovXScale.GetValue(fUnitLifetime), 
								GetProps()->m_ffcMaxFovXScale.GetValue(fUnitLifetime));

	float fYScale = GetRandom(	GetProps()->m_ffcMinFovYScale.GetValue(fUnitLifetime), 
								GetProps()->m_ffcMaxFovYScale.GetValue(fUnitLifetime));
	
	Keyframe.m_vFovScale.Init(fXScale, fYScale);
}


