#include "precompile.h"

#include "renderlight.h"
#include "d3d_convar.h"
#include "d3d_renderstatemgr.h"
#include "rendererconsolevars.h"

//-------------------------------------------------------------------------
// Utility Functions
//-------------------------------------------------------------------------

//given a light structure this will zero out all elements and setup the type field
static void InitLightStruct(D3DLIGHT9& Light, D3DLIGHTTYPE Type)
{
	//clear out the struct
	memset(&Light, 0, sizeof(Light));
	Light.Type = Type;
}

//given a color and a light, it will setup the light's color fields
static void SetupLightColor(D3DLIGHT9& Light, const LTVector& vColor)
{
	Light.Ambient.r = 0.0f;
	Light.Ambient.g = 0.0f;
	Light.Ambient.b = 0.0f;
	Light.Ambient.a = 1.0f;

	Light.Diffuse.r = vColor.x / 255.0f;
	Light.Diffuse.g = vColor.y / 255.0f;
	Light.Diffuse.b = vColor.z / 255.0f;
	Light.Diffuse.a = 1.0f;

	Light.Specular.r = Light.Diffuse.r;
	Light.Specular.g = Light.Diffuse.g;
	Light.Specular.b = Light.Diffuse.b;
	Light.Specular.a = Light.Diffuse.a;
}

//given a light and a slot number, it will install the light into the device
static void InstallLightIntoDevice(uint32 nSlot, D3DLIGHT9& Light)
{
	g_RenderStateMgr.SetLight(nSlot, &Light); 
	g_RenderStateMgr.LightEnable(nSlot, true); 
}

//this will convert a LTVector to a D3DVECTOR
void SetD3DVec(D3DVECTOR& lhs, const LTVector& rhs)
{
	lhs.x = rhs.x;
	lhs.y = rhs.y;
	lhs.z = rhs.z;
}

//-------------------------------------------------------------------------
// CRenderLight
//-------------------------------------------------------------------------
CRenderLight::CRenderLight() :
	m_eLightType(eLight_Invalid),
	m_nFlags(0),
	m_fRadiusSqr(0.0f)
{
}

CRenderLight::~CRenderLight()
{
}

//sets up the light to be a directional light
void CRenderLight::SetupDirLight(const LTVector& vDir, const LTVector& vColor, uint32 nFlags)
{
	m_eLightType	= eLight_Dir;
	m_vLightDir		= vDir;
	m_vColor		= vColor;
	m_nFlags		= nFlags;
}

//sets up the light to be a point light source
void CRenderLight::SetupPointLight(	 const LTVector& vLightPos,
									 const LTVector& vColor, const LTVector& vAttCoeff,
									 float fRadius, ELightAttenuationType eAttenuation, 
									 uint32 nFlags)
{
	m_eLightType	= eLight_Point;
	m_vLightPos		= vLightPos;
	m_vColor		= vColor;
	m_vAttCoeff		= vAttCoeff;
	m_fRadius		= fRadius;
	m_eAttenuation	= eAttenuation;
	m_nFlags		= nFlags;
	m_fRadiusSqr	= fRadius * fRadius;
}

//sets up the light to be a spotlight
void CRenderLight::SetupSpotLight(	 const LTVector& vLightPos,
									 const LTVector& vLightDir, const LTVector& vColor, 
									 const LTVector& vAttCoeff,
									 float fRadius, float fCosFOV,
									 ELightAttenuationType eAttenuation, 
									 uint32 nFlags)
{
	m_eLightType	= eLight_Spot;
	m_vLightPos		= vLightPos;
	m_vLightDir		= vLightDir;
	m_vColor		= vColor;
	m_fRadius		= fRadius;
	m_fCosFOV		= fCosFOV;
	m_nFlags		= nFlags;
	m_fRadiusSqr	= fRadius * fRadius;
	m_vAttCoeff		= vAttCoeff;
	m_eAttenuation	= eAttenuation;
}

//this will install the currently setup light into the device in the specified
//slot
void CRenderLight::InstallLight(uint32 nSlot, const LTVector& vObjectPos, const LTVector& vObjectDims) const
{
	//get the device light
	D3DLIGHT9 Light;
	CreateDeviceLight(vObjectPos, vObjectDims, Light);

	//install it
	InstallLightIntoDevice(nSlot, Light);
}

//this will convert the specified light to a light in the device format
void CRenderLight::CreateDeviceLight(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const
{
	//just dispatch to the appropriate function
	switch(m_eLightType)
	{
	case eLight_Invalid:
		//do nothing
		break;
	case eLight_Dir:
		CreateDirLight(DevLight);
		break;
	case eLight_Point:
		CreatePointLight(vObjectPos, vObjectDims, DevLight);
		break;
	case eLight_Spot:
		CreateSpotLight(vObjectPos, vObjectDims, DevLight);
		break;
	default:
		assert(!"Invalid light type specified.");
		break;
	}
}

void CRenderLight::CreateDirLight(D3DLIGHT9& DevLight) const
{
	//we can represent all dir lights with hardware ones, so just
	//ignore the attenuation model and install the token directional
	//light.
	InitLightStruct(DevLight, D3DLIGHT_DIRECTIONAL);
	SetD3DVec(DevLight.Direction, m_vLightDir);
	SetupLightColor(DevLight, m_vColor);
}

void CRenderLight::CreatePointLight(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const
{
	//InstallPointLightD3D(nSlot);
	//return;

	//look at the lighting types, and install the appropriate light..
	switch(m_eAttenuation)
	{
	case eAttenuation_D3D:		CreatePointLightD3D(vObjectPos, vObjectDims, DevLight);		break;
	case eAttenuation_Quartic:	CreatePointLightQuartic(vObjectPos, vObjectDims, DevLight);	break;
	case eAttenuation_Linear:	CreatePointLightLinear(vObjectPos, vObjectDims, DevLight);	break;
	default:
		assert(!"Invalid attenuation specified");
		break;
	}
}

void CRenderLight::CreateSpotLight(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const
{
	//since we don't do any falloff of spotlights, it really doesn't matter what attenuation we use,
	//so just install the multi-sampled light
	CreateMultiSampledDirLight(vObjectPos, vObjectDims, DevLight);
}

void CRenderLight::CreatePointLightD3D(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const
{
	InitLightStruct(DevLight, D3DLIGHT_POINT);

	DevLight.Range			= m_fRadius;
	DevLight.Attenuation0	= m_vAttCoeff.x;
	DevLight.Attenuation1	= m_vAttCoeff.y;
	DevLight.Attenuation2	= m_vAttCoeff.z;

	SetD3DVec(DevLight.Position, m_vLightPos);
	SetupLightColor(DevLight, m_vColor);
}

void CRenderLight::CreatePointLightQuartic(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const
{
	CreateSampledDirLight(vObjectPos, DevLight);
}

void CRenderLight::CreatePointLightLinear(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const
{
	CreateSampledDirLight(vObjectPos, DevLight);
}

//this form of installing will take a sample at the 
//specified viewer position and will create a dirlight of that color facing
//in the direction from the light to the viewer
void CRenderLight::CreateSampledDirLight(const LTVector& vObjectPos, D3DLIGHT9& DevLight) const
{
	InitLightStruct(DevLight, D3DLIGHT_DIRECTIONAL);
	SetD3DVec(DevLight.Direction, vObjectPos - m_vLightPos);
	SetupLightColor(DevLight, GetLightSample(vObjectPos));
}

//this form of installing will take a sample at the 
//specified viewer position and at each corner of the dimension box, and setup a directional
//light appropriately
void CRenderLight::CreateMultiSampledDirLight(const LTVector& vObjectPos, const LTVector& vObjectDims, D3DLIGHT9& DevLight) const
{
	//the sample positions
	static const float vSampleScales[][3] =		{	{ -1.0f, -1.0f, -1.0f },
													{ -1.0f, -1.0f, 1.0f },
													{ -1.0f, 1.0f, -1.0f },
													{ -1.0f, 1.0f, 1.0f },
													{ 1.0f, -1.0f, -1.0f },
													{ 1.0f, -1.0f, 1.0f },
													{ 1.0f, 1.0f, -1.0f },
													{ 1.0f, 1.0f, 1.0f } };

	static const uint32 nNumSamples = sizeof(vSampleScales) / (sizeof(vSampleScales[0][0]) * 3);

	//we need to sample the center
	LTVector vFinalColor = GetLightSample(vObjectPos);

	//now sample each bounding box corner
	for(uint32 nSample = 0; nSample < nNumSamples; nSample++)
	{
		//find the position by flipping which side we add the dims to
		LTVector vSamplePos;
		vSamplePos.x = vObjectPos.x + vObjectDims.x * vSampleScales[nSample][0];
		vSamplePos.y = vObjectPos.y + vObjectDims.y * vSampleScales[nSample][1];
		vSamplePos.z = vObjectPos.z + vObjectDims.z * vSampleScales[nSample][2];

		LTVector vSampleColor = GetLightSample(vSamplePos);

		//now we need to blend this sample's color with the other sample's color
		if(vSampleColor.MagSqr() > vFinalColor.MagSqr())
			vFinalColor = vSampleColor;
	}

	InitLightStruct(DevLight, D3DLIGHT_DIRECTIONAL);
	SetD3DVec(DevLight.Direction, vObjectPos - m_vLightPos);
	SetupLightColor(DevLight, vFinalColor);
}


//given a point, this will determine the light intensity of the point
LTVector CRenderLight::GetLightSample(const LTVector& vPos) const
{
	//handle the easy cases first

	//for directional lights, it is always the same color
	if(m_eLightType == eLight_Dir)
		return m_vColor;

	//find out distance attenuation
	LTVector vToSample	= vPos - m_vLightPos;
	float fDistSqr		= vToSample.MagSqr();

	//Note that we don't do an invalid light check, this is because the only way a light
	//can be invalid is if it is not initialized, and if it is not initailized it will have a radius
	//of 0 and will therefore always get pulled out in this radius check

	//bail if we are outside
	if(fDistSqr >= m_fRadiusSqr)
		return LTVector(0.0f, 0.0f, 0.0f);	

	float fDistPercent;

	float fDist	= (float)sqrt(fDistSqr);

	//calculate the distance attenuation
	switch (m_eAttenuation)
	{
		case eAttenuation_Quartic : 
		{
			fDistPercent  = 1.0f - (fDistSqr / m_fRadiusSqr);
			fDistPercent *= fDistPercent;
		}
		break;
		case eAttenuation_D3D :
		{
			fDistPercent = 1.0f / (m_vAttCoeff.x + m_vAttCoeff.y * fDist + m_vAttCoeff.z * fDistSqr); 
		}
		break;
		case eAttenuation_Linear : 
		{
			fDistPercent = 1.0f - (fDist / m_fRadius);
		}
		break;
	}

	//alright, handle distance attenuation on point lights
	if(m_eLightType == eLight_Point)
	{
		return m_vColor * fDistPercent;
	}

	//we still have to handle spotlights now...
	float fSampleDotLight	= vToSample.Dot(m_vLightDir) / fDist;
		
	if(fSampleDotLight > m_fCosFOV)
	{
		return m_vColor * (fDistPercent * ((fSampleDotLight - m_fCosFOV) / (1.0f - m_fCosFOV)));
	}
	
	//outside of the dirlight
	return LTVector(0.0f, 0.0f, 0.0f);
}

