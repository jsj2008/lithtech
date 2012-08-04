//------------------------------------------------------------------
//
//   MODULE  : TracerFX.cpp
//
//   PURPOSE : Implements class CTracerFX
//
//   CREATED : On 11/23/98 At 6:21:37 PM
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "ClientFX.h"
#include "TracerFX.h"
#include "iltrenderer.h"
#include "resourceextensions.h"
#include "ClientFXVertexDeclMgr.h"
#include "iperformancemonitor.h"
#include "GameRenderLayers.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXTracer("ClientFX_Tracer", "ClientFX");

//Function to handle filtering of the intersect segment calls needed by the flare sprite
static bool TracerListFilterFn(HOBJECT hTest, void *pUserData)
{
	LTUNREFERENCED_PARAMETER(pUserData);

	//tracers only collide with solid objects
	uint32 nFlags = 0;
	g_pLTClient->Common()->GetObjectFlags(hTest, OFT_Flags, nFlags);

	if(!(nFlags & FLAG_SOLID))
		return false;
	
	//tracers collide with solid models and world models
	uint32 nObjType;
	if(g_pLTClient->Common()->GetObjectType(hTest, &nObjType) != LT_OK)
		return false;

	if((nObjType != OT_WORLDMODEL) && (nObjType != OT_MODEL))
		return false;

    return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTracerProps::CTracerProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CTracerProps::CTracerProps() : 
	m_fLength(5.0f),
	m_fVelocity(1.0f),
	m_pszMaterial(NULL),
	m_fMaxDistance(10000.0f),
	m_bCropTexture(false),
	m_bTranslucentLight(false),
	m_bSolid(false),
	m_bFitLengthToRay(false),
	m_bStartEmitted(false),
	m_eLightLOD(eEngineLOD_Low),
	m_eWorldShadowsLOD(eEngineLOD_Low),
	m_eObjectShadowsLOD(eEngineLOD_Low),
	m_fLightRadius(0.0f),
	m_vLightColor(1.0f, 1.0f, 1.0f, 1.0f),
	m_bUseTargetPos(true),
	m_bBlockedByGeometry(true),
	m_bPlayerView(false)
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTracerProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CTracerProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	//-------------------------------
	//Tracer Properties
	//-------------------------------
	if( LTStrIEquals( pszName, "Material" ))
	{
		m_pszMaterial = CFxProp_String::Load(pStream, pszStringTable );
	}
	else if( LTStrIEquals( pszName, "PlayerView" ) )
	{
		m_bPlayerView = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Length" ))
	{
		m_fLength = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Thickness" ))
	{
		m_ffcThickness.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "Velocity" ))
	{
		m_fVelocity = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "MaxDistance" ))
	{
		m_fMaxDistance = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Color" ))
	{
		m_cfcColor.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "CropTexture" ))
	{
		m_bCropTexture = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Solid" ))
	{
		m_bSolid = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "TranslucentLight" ))
	{
		m_bTranslucentLight = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "FitLengthToRay" ))
	{
		m_bFitLengthToRay = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "StartEmitted" ))
	{
		m_bStartEmitted = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "UseTargetPos" ))
	{
		m_bUseTargetPos = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "BlockedByGeometry" ))
	{
		m_bBlockedByGeometry = CFxProp_EnumBool::Load(pStream);
	}

	//-------------------------------
	//Dynamic light Properties
	//-------------------------------
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
	else if(LTStrIEquals(pszName, "LightColor"))
	{
		m_vLightColor = CFxProp_Color4f::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "LightRadius"))
	{
		m_fLightRadius = CFxProp_Float::Load(pStream);
	}

	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CTracerProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectResource(m_pszMaterial);
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetTracerFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------
void fxGetTracerProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;

	fxProp.SetupTextLine("Common Properties");
	pList->AddTail(fxProp);

	// Add the base props
	AddBaseProps(pList);

	fxProp.SetupTextLine("");
	pList->AddTail(fxProp);
	fxProp.SetupTextLine("Tracer Properties");
	pList->AddTail(fxProp);

	// Add all the props to the list
	fxProp.SetupPath( "Material", "", "Material Files (*." RESEXT_MATERIAL ")|*." RESEXT_MATERIAL "|All Files (*.*)|*.*||", eCurve_None, "Determines the material that will be used when rendering the tracer. This should have a translucent shader with a texture that will be mapped to the dimensions of the tracer");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( "PlayerView", false, eCurve_None, "Determines if the sprite should be rendered in the player view, which means that it should have its Z values adjusted to not be clipped into nearby walls");
	pList->AddTail( fxProp );	

	fxProp.SetupFloatMin( "Thickness", 1.0f, 5.0f, eCurve_Linear, "Specifies how thick the tracer is perpendicular to the direction it is moving" );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "Length", 1.0f, 0.0f, eCurve_None, "Specifies the length of this tracer along the axis it is moving down" );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "Velocity", 1.0f, 0.0f, eCurve_None, "Specifies the speed that the tracer will travel in the forward direction" );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "MaxDistance", 10000.0f, 0.0f, eCurve_None, "Specifies the maximum distance that this tracer can travel. Useful for ensuring that they do not travel the length of the level or other such issues" );
	pList->AddTail( fxProp );

	fxProp.SetupColor("Color", 0xFFFFFFFF, eCurve_Linear, "Specifies the color of the tracer");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "CropTexture", false, eCurve_None, "Determines if the texture on the tracer should be cropped when clipped to the beginning or end of the ray. If it is not, the texture will just be scaled to fit the visible portion." );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "Solid", false, eCurve_None, "Determines if this tracer should be treated as solid or not when rendering" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "TranslucentLight", false, eCurve_None, "For translucent objects, this determines if lighting should be approximated or if it should be fullbright" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "FitLengthToRay", false, eCurve_None, "If set to true, this will ignore the specified length, and will instead use the length of the ray as the length of the tracer" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "StartEmitted", false, eCurve_None, "If set to true, the tracer will start so that it is fully emitted at the beginning instead of emerging out" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "UseTargetPos", true, eCurve_None, "If set to true, the tracer will travel to the target position associated with the effect when emitted instead of shooting down the forward axis" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "BlockedByGeometry", true, eCurve_None, "This determines whether or not geometry between the tracer and the object will block the tracer" );
	pList->AddTail( fxProp );

	fxProp.SetupTextLine("");
	pList->AddTail(fxProp);
	fxProp.SetupTextLine("Light Properties (TEMPORARY FOR DEMO)");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin( "LightRadius", 0.0f, 0.0f, eCurve_None, "Specifies the radius of the point light that will be attached to the tracer (0 indicates no light)" );
	pList->AddTail( fxProp );

	fxProp.SetupColor( "LightColor", 0xFFFFFFFF, eCurve_None, "Specifies the color of the point light that will be attached to the tracer" );
	pList->AddTail( fxProp );

	fxProp.SetupEnum("LightLOD", "Low", "Low,Medium,High,Never", eCurve_None, "Specifies at which LOD levels this light will be rendered");
	pList->AddTail(fxProp);

	fxProp.SetupEnum("WorldShadowsLOD", "Low", "Low,Medium,High,Never", eCurve_None, "Specifies at which LOD levels this light will cast shadows from the world");
	pList->AddTail(fxProp);

	fxProp.SetupEnum("ObjectShadowsLOD", "Low", "Low,Medium,High,Never", eCurve_None, "Specifies at which LOD levels this light will cast shadows from objects it touches");
	pList->AddTail(fxProp);

}

//------------------------------------------------------------------
//
//   FUNCTION : CTracerFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CTracerFX::CTracerFX( ) :	
	CBaseFX( CBaseFX::eTracerFX ),
	m_fRayPosition(0.0f),
	m_fRayLength(1.0f),
	m_hTracerLight(NULL),
	m_hObject(NULL)
{	
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CTracerFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CTracerFX::~CTracerFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CTracerFX
//
//------------------------------------------------------------------

bool CTracerFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXTracer);

	//cleanup any old data
	Term();

	// Perform base class initialisation
	if( !CBaseFX::Init(pBaseData, pProps))
		return false;

	//determine our starting position and orientation
	LTVector vPos;
	LTRotation rRot;

	GetCurrentTransform(0.0f, vPos, rRot);	
	m_vStartPos = vPos;

	//determine what target position we want to use
	LTVector vTargetPos;

	//if we are using target data, just get the transform from this
	if(GetProps()->m_bUseTargetPos && pBaseData->m_bUseTargetData)
	{
		LTRigidTransform tTargetOffset(pBaseData->m_vTargetOffset, LTRotation::GetIdentity());
		LTRigidTransform tWS = GetWorldSpaceTransform(NULL, pBaseData->m_hTargetObject, INVALID_MODEL_NODE, INVALID_MODEL_SOCKET, tTargetOffset);

		LTVector vToTarget = tWS.m_vPos - vPos;
		m_fRayLength	= vToTarget.Mag();
		m_vDirection	= vToTarget / m_fRayLength;
		vTargetPos		= tWS.m_vPos;
	}
	else
	{
		//determine the end point where we want to stop testing
		m_vDirection	= rRot.Forward();
		m_fRayLength	= GetProps()->m_fMaxDistance;
		vTargetPos		= vPos + m_vDirection * m_fRayLength;
	}

	//handle intersection if we need to
	if(GetProps()->m_bBlockedByGeometry)
	{
		//we need to perform a ray cast and determine if we actually hit anything in the world. If not,
		//we just want to use the maximum tracer length
		IntersectQuery	iQuery;
		IntersectInfo	iInfo;

		iQuery.m_Flags		= INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
		iQuery.m_FilterFn	= TracerListFilterFn;
		iQuery.m_pUserData	= NULL;
		iQuery.m_From		= m_vStartPos;
		iQuery.m_To			= vTargetPos;

		//now find the point of intersection
		if( g_pLTClient->IntersectSegment( iQuery, &iInfo ) )
		{
			//we hit something, so use that as our ending position
			m_fRayLength = (iInfo.m_Point - m_vStartPos).Mag();
		}
	}

	//now determine our starting position (this is zero unless we start emitted, in which case
	//it is the length of the tracer)
	if(GetProps()->m_bStartEmitted)
	{
		//determine the length of this tracer
		m_fRayPosition = GetTracerLength();
	}
	
	// Combine the direction we would like to face with our parents rotation...
	ObjectCreateStruct ocs;
	
	//create a custom render object with the associated material
	ocs.m_Pos				= vPos;
	ocs.m_Rotation			= rRot;
	ocs.m_ObjectType		= OT_CUSTOMRENDER;
	
	if(!GetProps()->m_bSolid)
		ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	if(!GetProps()->m_bTranslucentLight)
		ocs.m_Flags |= FLAG_NOLIGHT;
	
	m_hObject = g_pLTClient->CreateObject( &ocs );
	if( !m_hObject )
		return false;

	//setup our rendering layer
	if(GetProps()->m_bPlayerView)
		g_pLTClient->GetRenderer()->SetObjectDepthBiasTableIndex(m_hObject, eRenderLayer_Player);

	//setup the callback on the object so that it will render us
	g_pLTClient->GetCustomRender()->SetRenderingSpace(m_hObject, eRenderSpace_World);
	g_pLTClient->GetCustomRender()->SetRenderCallback(m_hObject, CustomRenderCallback);
	g_pLTClient->GetCustomRender()->SetCallbackUserData(m_hObject, this);

	//load up the material for this particle system, and assign it to the object
	HMATERIAL hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance(GetProps()->m_pszMaterial);
	g_pLTClient->GetCustomRender()->SetMaterial(m_hObject, hMaterial);
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	//we don't setup the visibility for this object yet since when it starts out it is not visible

	//setup the dynamic light object if the user specified a radius
	if(GetProps()->m_fLightRadius > 0.1f)
	{
		ObjectCreateStruct LightOCS;
		LightOCS.m_Pos			= vPos;
		LightOCS.m_Rotation		= rRot;
		LightOCS.m_ObjectType	= OT_LIGHT;

		m_hTracerLight = g_pLTClient->CreateObject( &LightOCS );

		//setup the properties of the light
		g_pLTClient->SetLightRadius(m_hTracerLight, GetProps()->m_fLightRadius);
		g_pLTClient->SetObjectColor(m_hTracerLight, VEC4_EXPAND(GetProps()->m_vLightColor));
		g_pLTClient->SetLightType(m_hTracerLight, eEngineLight_Point);
		g_pLTClient->SetLightDetailSettings(m_hTracerLight, GetProps()->m_eLightLOD, GetProps()->m_eWorldShadowsLOD, GetProps()->m_eObjectShadowsLOD);
	}

	// Success !!
	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CTracerFX
//
//------------------------------------------------------------------

void CTracerFX::Term()
{
	if (m_hObject) 
	{
		g_pLTClient->RemoveObject(m_hObject);
		m_hObject = NULL;
	}

	if(m_hTracerLight)
	{
		g_pLTClient->RemoveObject(m_hTracerLight);
		m_hTracerLight = NULL;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : IsFinishedShuttingDown()
//
//   PURPOSE  : Determines if the tracer is done or not
//
//------------------------------------------------------------------

bool CTracerFX::IsFinishedShuttingDown()
{
	//we are only done if the tracer is past the end of the ray
	if(m_fRayPosition - GetTracerLength() >= m_fRayLength)
		return true;

	//we are still visible, so bail
	return false;
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CTracerFX
//
//------------------------------------------------------------------

bool CTracerFX::Update(float tmFrameTime)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXTracer);

	// Base class update first
	BaseUpdate(tmFrameTime);

	//determine the length of this tracer
	float fTracerLen = GetTracerLength();

	//update the position along the ray
	m_fRayPosition += GetProps()->m_fVelocity * tmFrameTime;

	float fTracerHead = m_fRayPosition;
	float fTracerTail  = m_fRayPosition - fTracerLen;

	//now we need to find the extents of the segment
	float fSegmentStart = LTCLAMP(fTracerHead, 0.0f, m_fRayLength);
	float fSegmentEnd   = LTCLAMP(fTracerTail, 0.0f, m_fRayLength);

	//now we generate a bounding box around these points
	LTVector vCenter = m_vStartPos + m_vDirection * ((fSegmentStart + fSegmentEnd) * 0.5f);

	//and now the half dimensions that encompass the tracer
	LTVector vHalfDims = m_vDirection * ((fSegmentStart - fSegmentEnd) * 0.5f);

	//make sure that this half dims represents the maximum extents
	vHalfDims.Max(-vHalfDims);

	//extend the half dims to include the thickness of the tracer
	float fUnitLifetime = GetUnitLifetime();
	float fHalfThickness = GetProps()->m_ffcThickness.GetValue(fUnitLifetime) * 0.5f;
	vHalfDims += LTVector(fHalfThickness, fHalfThickness, fHalfThickness);

	//and now setup the object position and visibility
	g_pLTClient->SetObjectPos(m_hObject, vCenter);
	g_pLTClient->GetCustomRender()->SetVisBoundingBox(m_hObject, -vHalfDims, vHalfDims);

	//handle updating the light if we have one
	if(m_hTracerLight)
	{
		//the light needs to be moved to the center of the tracer
		g_pLTClient->SetObjectPos(m_hTracerLight, vCenter);

		//and have the intensity controlled by how much of the tracer is visible
		float fVisible = (fSegmentStart - fSegmentEnd) / fTracerLen;

		if(fVisible > 0.01f)
		{
			g_pLTClient->Common()->SetObjectFlags(m_hTracerLight, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			g_pLTClient->SetLightIntensityScale(m_hTracerLight, fVisible);
		}
		else
		{
			g_pLTClient->Common()->SetObjectFlags(m_hTracerLight, OFT_Flags, 0, FLAG_VISIBLE);
		}
	}

	//we are done if the tracer is past the end of the ray
	return (m_fRayPosition - GetTracerLength() < m_fRayLength);
}

//called to enumerate through each of the objects and will call into the provided function for each
void CTracerFX::EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData)
{
	if(m_hObject)
	{
		pfnObjectCB(this, m_hObject, pUserData);
	}
}

//hook for the custom render object, this will just call into the render function
void CTracerFX::CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
{
	((CTracerFX*)pUser)->RenderTracer(pInterface, tCamera);
}

//function that handles the custom rendering
void CTracerFX::RenderTracer(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXTracer);

	//first determine the length, position, and U range for this tracer (this allows for some
	//early outs)
	float fTracerLen	= GetTracerLength();
	float fTracerStart	= m_fRayPosition;
	float fTracerEnd	= fTracerStart - fTracerLen;

	if((fTracerStart <= 0.0f) || (fTracerEnd >= m_fRayLength))
	{
		//the tracer has fully gone through the ray, don't render
		return;
	}

	//now we need to clip the extents to the range [0..ray length], and handle cropping of the texture
	float fUMin = 0.0f;
	float fUMax = 1.0f;

	if(fTracerEnd < 0.0f)
	{
		//adjust the U max if we are cropping
		if(GetProps()->m_bCropTexture)
		{
			fUMax += fTracerEnd / fTracerLen;
		}
		fTracerEnd = 0.0f;
	}
	if(fTracerStart >= m_fRayLength)
	{
		//adjust the U min if we are cropping
		if(GetProps()->m_bCropTexture)
		{
			fUMin += (fTracerStart - m_fRayLength) / fTracerLen;
		}
		fTracerStart = m_fRayLength;
	}

	//setup our vertex declaration
	if(pInterface->SetVertexDeclaration(g_ClientFXVertexDecl.GetTexTangentSpaceDecl()) != LT_OK)
		return;

	//bind a quad index stream
	if(pInterface->BindQuadIndexStream() != LT_OK)
		return;

	//sanity check to ensure that we can at least render a sprite
	LTASSERT(QUAD_RENDER_INDEX_STREAM_SIZE >= 6, "Error: Quad index list is too small to render a tracer");
	LTASSERT(DYNAMIC_RENDER_VERTEX_STREAM_SIZE / sizeof(STexTangentSpaceVert) > 4, "Error: Dynamic vertex buffer size is too small to render a tracer");

	//we need to determine the facing of this tracer. This is formed by creating a plane from the points
	//Camera, Start, and another point on the ray. The plane normal is then the up, the right is the ray
	//direction, and the normal is ray cross plane normal. 
	LTVector vStartToCamera = m_vStartPos - tCamera.m_vPos;
	float fMag = vStartToCamera.Mag( );
	if( fMag < 0.00001f )
	{
		vStartToCamera = tCamera.m_rRot.Forward();
	}
	else
	{
		vStartToCamera /= fMag;
	}

	//determine the up vector
	LTVector vUp = vStartToCamera.Cross(m_vDirection);
	vUp.Normalize();

	//now determine the actual normal (doesn't need to be normalized since the vectors are
	//unit length and orthogonal)
	LTVector vNormal = vUp.Cross(m_vDirection);

	//lock down our buffer for rendering
	SDynamicVertexBufferLockRequest LockRequest;
	if(pInterface->LockDynamicVertexBuffer(4, LockRequest) != LT_OK)
		return;

	//fill in our sprite vertices
	STexTangentSpaceVert* pCurrOut = (STexTangentSpaceVert*)LockRequest.m_pData;

	//determine the color of this tracer
	float fUnitLifetime = GetUnitLifetime();
	uint32 nColor = CFxProp_Color4f::ToColor(GetProps()->m_cfcColor.GetValue(fUnitLifetime));

	//calculate the front of the tracer in world space
	LTVector vFront = m_vStartPos + m_vDirection * fTracerStart;
	LTVector vBack	= m_vStartPos + m_vDirection * fTracerEnd;

	//and the thickness vector
	float fThickness = GetProps()->m_ffcThickness.GetValue(fUnitLifetime);
	LTVector vThickness = vUp * (fThickness * 0.5f);

	//fill in the particle vertices
	pCurrOut[0].m_vPos = vFront + vThickness;
	pCurrOut[0].m_vUV.Init(fUMin, 0.0f);
	
	pCurrOut[1].m_vPos = vBack + vThickness;
	pCurrOut[1].m_vUV.Init(fUMax, 0.0f);
	
	pCurrOut[2].m_vPos = vBack - vThickness;
	pCurrOut[2].m_vUV.Init(fUMax, 1.0f);

	pCurrOut[3].m_vPos = vFront - vThickness;
	pCurrOut[3].m_vUV.Init(fUMin, 1.0f);

	//setup the remaining vertex components
	for(uint32 nCurrVert = 0; nCurrVert < 4; nCurrVert++)
	{
		pCurrOut[nCurrVert].m_nPackedColor	= nColor;
		pCurrOut[nCurrVert].m_vNormal		= vNormal;
		pCurrOut[nCurrVert].m_vTangent		= vUp;
		pCurrOut[nCurrVert].m_vBinormal		= m_vDirection;
	}

	//unlock and render the batch
	pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
	pInterface->RenderIndexed(	eCustomRenderPrimType_TriangleList, 
								0, 6, LockRequest.m_nStartIndex, 
								0, 4);
}
