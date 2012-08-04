//------------------------------------------------------------------
//
//   MODULE  : DYNALIGHTFX.CPP
//
//   PURPOSE : Implements class CDynaLightFX
//
//   CREATED : On 12/14/98 At 5:43:43 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "DynaLightFX.h"
#include "ClientFX.h"
#include "resourceextensions.h"
#include "iperformancemonitor.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXLight("ClientFX_Light", "ClientFX");

//----------------------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------------------

//converts an enumeration from the property type to the engine type
static EEngineLightType GetEngineLightType(uint32 nType)
{
	switch(nType)
	{
	case 1:
		return eEngineLight_PointFill;
	case 2:
		return eEngineLight_SpotProjector;
	case 3:
		return eEngineLight_CubeProjector;
	case 4:
		return eEngineLight_BlackLight;
	default:
		return eEngineLight_Point;
	}
}

//converts a 4 color vector to a 3 color vector by multiplying the components by the alpha
static LTVector ToColor3(const LTVector4& vColor)
{
	return LTVector(	vColor.x * vColor.w,
						vColor.y * vColor.w,
						vColor.z * vColor.w);
}

//----------------------------------------------------------------------------------------
// CDynaLightProps
//----------------------------------------------------------------------------------------

CDynaLightProps::CDynaLightProps() :
	m_eLightLOD(eEngineLOD_Low),
	m_eWorldShadowsLOD(eEngineLOD_Low),
	m_eObjectShadowsLOD(eEngineLOD_Low),
	m_eInSky(eFXSkySetting_None),
	m_pszTexture(NULL)
{
}

//handles loading of properties
bool CDynaLightProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	if(LTStrIEquals(pszName, "Type"))
	{
		m_efcType.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "LightLOD"))
	{
		m_eLightLOD = (EEngineLOD)CFxProp_Enum::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "WorldShadowsLOD"))
	{
		m_eWorldShadowsLOD = (EEngineLOD)CFxProp_Enum::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "ObjectShadowsLOD"))
	{
		m_eObjectShadowsLOD = (EEngineLOD)CFxProp_Enum::Load(pStream);
	}
	else if(LTStrIEquals( pszName, "InSky"))
	{
		m_eInSky = (EFXSkySetting)CFxProp_Enum::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "Color"))
	{
		m_cfcColor.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "Intensity"))
	{
		m_ffcIntensity.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "FlickerScale"))
	{
		m_ffcFlickerScale.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "Radius"))
	{
		m_ffcRadius.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "Texture"))
	{
		m_pszTexture = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if(LTStrIEquals(pszName, "TranslucentColor"))
	{
		m_cfcTranslucentColor.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "SpecularColor"))
	{
		m_cfcSpecularColor.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "FovX"))
	{
		m_ffcSpotFovX.Load(pStream, pCurveData);
	}
	else if(LTStrIEquals(pszName, "FovY"))
	{
		m_ffcSpotFovY.Load(pStream, pCurveData);
	}
	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CDynaLightProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectResource(m_pszTexture);
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetDynaLightFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetDynaLightProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;

	// Add the base props

	AddBaseProps(pList);

	fxProp.SetupTextLine("Light Parameters");
	pList->AddTail(fxProp);

	fxProp.SetupEnum("LightLOD", "Low", "Low,Medium,High,Never", eCurve_None, "Specifies at which LOD levels this light will be rendered");
	pList->AddTail(fxProp);

	fxProp.SetupEnum("WorldShadowsLOD", "Low", "Low,Medium,High,Never", eCurve_None, "Specifies at which LOD levels this light will cast shadows from the world");
	pList->AddTail(fxProp);

	fxProp.SetupEnum("ObjectShadowsLOD", "Low", "Low,Medium,High,Never", eCurve_None, "Specifies at which LOD levels this light will cast shadows from objects it touches");
	pList->AddTail(fxProp);

	fxProp.SetupEnum( "InSky", SKY_PROP_DEFAULT, SKY_PROP_ENUM, eCurve_None, SKY_PROP_DESCRIPTION);
	pList->AddTail( fxProp );

	fxProp.SetupEnum("Type", "Point", "Point,PointFill,Spot,Cubic,BlackLight", eCurve_Step, "The type of light this light will be");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("Radius", 100.0f, 0.0f, eCurve_Linear, "The radius of the light (Point,Spot,Cubic)");
	pList->AddTail(fxProp);

	fxProp.SetupPath("Texture", "", "Texture Files (*.dds)|*.dds|Texture Animation Files (*."RESEXT_TEXTUREANIM")|*."RESEXT_TEXTUREANIM"|All Files (*.*)|*.*||", eCurve_None, "The texture associated with this light (Ambient,Spot,Cubic)");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMinMax("Intensity", 1.0f, 0.0f, 2.0f, eCurve_Linear, "The intensity of this light 0-1 (All)");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMinMax("FlickerScale", 1.0f, 0.0f, 1.0f, eCurve_Linear, "The amount to randomly flicker the intensity by (All)");
	pList->AddTail(fxProp);

	fxProp.SetupTextLine("Color Parameters");
	pList->AddTail(fxProp);

	fxProp.SetupColor("Color", SETRGBA(0xFF, 0xFF, 0xFF, 0xFF), eCurve_Linear, "The color of the light (All)");
	pList->AddTail(fxProp);

	fxProp.SetupColor("TranslucentColor", SETRGBA(0xFF, 0xFF, 0xFF, 0xFF), eCurve_Linear, "The color that best fits the associated texture, modulates light color for translucent approximation (All)");
	pList->AddTail(fxProp);

	fxProp.SetupColor("SpecularColor", SETRGBA(0xFF, 0xFF, 0xFF, 0xFF), eCurve_Linear, "The specular color of the light. Modulates the base color (All)");
	pList->AddTail(fxProp);

	fxProp.SetupTextLine("Spotlight Parameters");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMinMax("FovX", 45.0f, 0.0f, 170.0f, eCurve_Linear, "The field of view for a spotlight (Spot)");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMinMax("FovY", 45.0f, 0.0f, 170.0f, eCurve_Linear, "The field of view for a spotlight (Spot)");
	pList->AddTail(fxProp);
}

//----------------------------------------------------------------------------------------
// CDynaLightFX
//----------------------------------------------------------------------------------------

//------------------------------------------------------------------
//
//   FUNCTION : CDynaLightFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CDynaLightFX::CDynaLightFX() :	
	CBaseFX(CBaseFX::eDynaLightFX),
	m_hLight(NULL)
{

}

//------------------------------------------------------------------
//
//   FUNCTION : ~CDynaLightFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CDynaLightFX::~CDynaLightFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CDynaLightFX
//
//------------------------------------------------------------------

bool CDynaLightFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pBaseData, pProps)) 
		return false;

	ObjectCreateStruct ocs;

	ocs.m_ObjectType		= OT_LIGHT;
	ocs.m_Scale				= 1.0f;

	GetCurrentTransform(0.0f, ocs.m_Pos, ocs.m_Rotation);

	//setup whether or not it is in the sky
	ocs.m_Flags2 |= GetSkyFlags(GetProps()->m_eInSky);

	// Create the light
	m_hLight = g_pLTClient->CreateObject(&ocs);

	//setup static light properties

	//setup the LOD's associated with the light
	g_pLTClient->SetLightDetailSettings(m_hLight, GetProps()->m_eLightLOD, GetProps()->m_eWorldShadowsLOD, GetProps()->m_eObjectShadowsLOD);

	//setup the texture associated with the light
	g_pLTClient->SetLightTexture(m_hLight, GetProps()->m_pszTexture);

	//setup any extra properties
	UpdateDynamicProperties(0.0f);

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CDynaLightFX
//
//------------------------------------------------------------------

void CDynaLightFX::Term()
{
	if(m_hLight)
	{
		g_pLTClient->RemoveObject(m_hLight);
		m_hLight = NULL;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CDynaLightFX
//
//------------------------------------------------------------------

bool CDynaLightFX::Update(float tmFrameTime)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXLight);

	// Base class update first	
	BaseUpdate(tmFrameTime);

	//update our light position
	LTRigidTransform tObjTrans;
	GetCurrentTransform(GetUnitLifetime(), tObjTrans.m_vPos, tObjTrans.m_rRot);
	g_pLTClient->SetObjectTransform(m_hLight, tObjTrans);

	//if we are shutting down, clear the radius of this light
	if (IsShuttingDown())
	{
		g_pLTClient->SetLightRadius(m_hLight, 0.0f);		
		return true;
	}

	float fUnitLifetime = GetUnitLifetime();

	UpdateDynamicProperties(fUnitLifetime);
	return true;
}

//called to enumerate through each of the objects and will call into the provided function for each
void CDynaLightFX::EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData)
{
	if(m_hLight)
	{
		pfnObjectCB(this, m_hLight, pUserData);
	}
}

//handles updating the properties of the light given the specified unit time value
void CDynaLightFX::UpdateDynamicProperties(float fUnitTime)
{
	//update the type of this light
	g_pLTClient->SetLightType(m_hLight, GetEngineLightType(GetProps()->m_efcType.GetValue(fUnitTime)));

	//the intensity of the light
	float fIntensity	= GetProps()->m_ffcIntensity.GetValue(fUnitTime);
	float fFlickerScale	= GetProps()->m_ffcFlickerScale.GetValue(fUnitTime);
	g_pLTClient->SetLightIntensityScale(m_hLight, LTMAX(0.0f, fIntensity * GetRandom(fFlickerScale, 1.0f)) );

	//get the color of this light
	LTVector vColor = ToColor3(GetProps()->m_cfcColor.GetValue(fUnitTime));
	g_pLTClient->SetObjectColor(m_hLight, vColor.x, vColor.y, vColor.z, 1.0f);

	//and now the radius of the light
	float fRadius = GetProps()->m_ffcRadius.GetValue(fUnitTime);
	g_pLTClient->SetLightRadius(m_hLight, fRadius);

	//the specular color
	LTVector4 vSpecular = GetProps()->m_cfcSpecularColor.GetValue(fUnitTime);
	g_pLTClient->SetLightSpecularColor(m_hLight, ToColor3(vSpecular));

	//the translucent color
	LTVector4 vTranslucent = GetProps()->m_cfcTranslucentColor.GetValue(fUnitTime);
	g_pLTClient->SetLightTranslucentColor(m_hLight, ToColor3(vTranslucent));

	//the spot light information
	float fFovX = MATH_DEGREES_TO_RADIANS(GetProps()->m_ffcSpotFovX.GetValue(fUnitTime));
	float fFovY = MATH_DEGREES_TO_RADIANS(GetProps()->m_ffcSpotFovY.GetValue(fUnitTime));

	g_pLTClient->SetLightSpotInfo(m_hLight, fFovX, fFovY, 0.0f);
}

