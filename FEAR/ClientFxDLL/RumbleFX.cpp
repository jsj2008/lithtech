#include "stdafx.h"
#include "RumbleFX.h"
#include "clientfx.h"


//-------------------------------------------------------------------------------------
// CRumbleProps
//-------------------------------------------------------------------------------------

CRumbleProps::CRumbleProps() :
	m_bUseRadius(true),
	m_eFalloff(eFalloff_Linear)
{
}

CRumbleProps::~CRumbleProps()
{
}

//handles loading up a single property from the specified file
bool CRumbleProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	//-------------------------
	// Effect Intensity

	if( LTStrIEquals( pszName, "UseRadius" ))
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
	else
	{
		//see if it is an intensity curve
		for(uint32 nCurrMotor = 0; nCurrMotor < NUM_CLIENTFX_CONTROLLER_MOTORS; nCurrMotor++)
		{
			char pszPropName[32];
			LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Intensity%d", nCurrMotor);
			if(LTStrIEquals(pszName, pszPropName))
			{
				m_ffcIntensity[nCurrMotor].Load(pStream, pCurveData);
				return true;
			}
		}        
		
		//the property wasn't handled, so forward it on to our base
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}


//called to get the appropriate properties for the camera shake
void fxGetRumbleProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc	fxProp;

	// Add the generic "every effect has these" props
	AddBaseProps( pList );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Effect Intensity");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( "UseRadius", true, eCurve_None, "Determines whether or not the radius of the effect should be used, or if this should just be applied at full intensity regardless of where the camera is." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("Radius", 100.0f, 0.0f, eCurve_Linear, "Specifies the radius of this effect which is the area that it will shake the camera. This is ignored if UseRadius is false");
	pList->AddTail( fxProp );

	fxProp.SetupEnum("Falloff", "Linear", "Linear,Quartic,Constant", eCurve_None, "Specifies how the intensity should drop off as the camera moves away from the effect center towards the radius");
	pList->AddTail( fxProp );

	//see if it is an intensity curve
	for(uint32 nCurrMotor = 0; nCurrMotor < NUM_CLIENTFX_CONTROLLER_MOTORS; nCurrMotor++)
	{
		char pszPropName[32];
		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Intensity%d", nCurrMotor);

		fxProp.SetupFloatMinMax(pszPropName, 0.0f, 0.0f, 1.0f, eCurve_Linear, "Specifies the intensity of the vibration for the associated motors. Larger and lower frequency motors are the smaller values with higher frequency motors being larger indices. These values range from 0 (motor being off) to 1 (motor running at full intensity)");
		pList->AddTail( fxProp );
	}     
}

//-------------------------------------------------------------------------------------
// CRumbleFX
//-------------------------------------------------------------------------------------

CRumbleFX::CRumbleFX() :
	CBaseFX(eRumbleFX)
{
	m_ControllerLink.SetData(this);
}

CRumbleFX::~CRumbleFX()
{
	Term();
}

//initializes the effect based upon the passed in data
bool CRumbleFX::Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps)
{
	if(!CBaseFX::Init(pData, pProps))
		return false;

	return true;
}

//terminates the effect
void CRumbleFX::Term()
{
	m_ControllerLink.Remove();
}

//called to update the effect
bool CRumbleFX::Update(float tmFrameTime)
{
	BaseUpdate(tmFrameTime);

	//since we can have shut down and init on the same frame in rare cases, make sure to not handle
	//the initial frame if we are shutting down
	if(IsShuttingDown())
		m_ControllerLink.Remove();
	else if(IsInitialFrame())
		m_pFxMgr->SubscribeController(m_ControllerLink);

	return true;
}

//This version of update is called while the effect is suspended so that it can do
//things like smooth shutdown depending upon the effect type
bool CRumbleFX::SuspendedUpdate(float tmFrameTime)
{
	return CBaseFX::SuspendedUpdate(tmFrameTime);
}

//called in response to a query for the intensity that should be used for the controller motors
//and allows this effect to influence the motors appropriately. Motor intensities range from [0..1]
//but this function should only accumulate and should not clamp in order to allow for interactions.
void CRumbleFX::GetControllerModifier(const LTRigidTransform& tCameraTrans,
									  float fMotorIntensity[NUM_CLIENTFX_CONTROLLER_MOTORS])
{
	if(!IsActive() || IsSuspended())
		return;

    float fUnitLifetime = GetUnitLifetime();

	//precalcualte the intensity scale for the motors
	float fDistIntensity = 1.0f;

	//now compute this data if we are using the radius, otherwise it doesn't matter
	if(GetProps()->m_bUseRadius)
	{
		//and the radius
		float fRadius = GetProps()->m_ffcRadius.GetValue(fUnitLifetime);
		float fRadiusSqr = fRadius * fRadius;

		//get the position of the effect
		LTRigidTransform tFxTrans;
		GetCurrentTransform(fUnitLifetime, tFxTrans.m_vPos, tFxTrans.m_rRot);

		//now determine if we are outside of the radius
		float fDistSqr = tCameraTrans.m_vPos.DistSqr(tFxTrans.m_vPos);
		if(fDistSqr >= fRadiusSqr)
			return;

		//now apply the appropriate falloff curve
		switch(GetProps()->m_eFalloff)
		{
		case CRumbleProps::eFalloff_Linear:
			{
				fDistIntensity = 1.0f - sqrtf(fDistSqr) / fRadius;
			}
			break;
		case CRumbleProps::eFalloff_Quartic:
			{
				float fFalloff = 1.0f - fDistSqr / fRadiusSqr;
				fDistIntensity = fFalloff * fFalloff;
			}
			break;
		case CRumbleProps::eFalloff_Constant:
			{
			}
			break;
		}
	}

	for(uint32 nCurrMotor = 0; nCurrMotor < NUM_CLIENTFX_CONTROLLER_MOTORS; nCurrMotor++)
	{
		//get the amount of intensity to apply to this camera
		float fIntensity = GetProps()->m_ffcIntensity[nCurrMotor].GetValue(fUnitLifetime) * fDistIntensity;

		//now accumulate that into the motor intensity
		fMotorIntensity[nCurrMotor] += fIntensity;		
	}
}



