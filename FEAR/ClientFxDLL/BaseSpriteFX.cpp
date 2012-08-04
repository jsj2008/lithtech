//------------------------------------------------------------------
//
//   MODULE  : BASESPRITEFX.CPP
//
//   PURPOSE : Implements class CBaseSpriteFX
//
//   CREATED : On 11/23/98 At 6:21:37 PM
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "ClientFX.h"
#include "BaseSpriteFX.h"
#include "iltrenderer.h"
#include "resourceextensions.h"
#include "ClientFXVertexDeclMgr.h"
#include "GameRenderLayers.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CBaseSpriteProps::CBaseSpriteProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CBaseSpriteProps::CBaseSpriteProps() : 
	m_fAspectWidth (1.0f),
	m_fAspectHeight(1.0f),
	m_bAlignToCamera(true),
	m_bAlignAroundZ(false),
	m_eInSky(eFXSkySetting_None),
	m_bSolid(false),
	m_bTranslucentLight(true),
	m_bTwoSided(false),
	m_pszMaterial(NULL),
	m_bPlayerView(false),
	m_fToCameraOffset(0.0f),
	m_fRotationVelRad(0.0f),
	m_bRandomRotation(false)
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CBaseSpriteProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CBaseSpriteProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	if( LTStrIEquals( pszName, "Material" ))
	{
		m_pszMaterial = CFxProp_String::Load(pStream, pszStringTable );
	}
	else if( LTStrIEquals( pszName, "AspectWidth" ))
	{
		m_fAspectWidth = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "AspectHeight" ))
	{
		m_fAspectHeight = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "AlignToCamera" ))
	{
		m_bAlignToCamera = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "AlignAroundZ" ))
	{
		m_bAlignAroundZ = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "InSky" ))
	{
		m_eInSky = (EFXSkySetting)CFxProp_Enum::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Solid" ))
	{
		m_bSolid = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "TranslucentLight" ) )
	{
		m_bTranslucentLight = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "TwoSided" ) )
	{
		m_bTwoSided = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "PlayerView" ) )
	{
		m_bPlayerView = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "RandomRotation" ) )
	{
		m_bRandomRotation = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "RotationalVel" ) )
	{
		m_fRotationVelRad = MATH_DEGREES_TO_RADIANS(CFxProp_Float::Load(pStream));
	}
	else if( LTStrIEquals( pszName, "ToCameraOffset" ) )
	{
		m_fToCameraOffset = CFxProp_Float::Load(pStream);
	}
	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

bool CBaseSpriteProps::PostLoadProperties()
{
	m_fAspectRatio = m_fAspectHeight / m_fAspectWidth;

	return CBaseFXProps::PostLoadProperties();
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CBaseSpriteProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectResource(m_pszMaterial);
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetSpriteFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------
void fxGetBaseSpriteProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;

	// Add the base props
	AddBaseProps(pList);

	// Add all the props to the list

	//-------------------------------------------
	fxProp.SetupTextLine("Sprite Alignment");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( "AlignToCamera", true, eCurve_None, "Determines if the sprite should always face the camera");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "AlignAroundZ", false, eCurve_None, "Determines if the sprite should always be oriented around the forward axis of the object. By doing so it will also be anchored on the middle right hand side. This is ignored if AlignToCamera is enabled.");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "RandomRotation", false, eCurve_None, "Determines if the starting rotation for the sprite should be randomly selected. This only applies to camera aligned sprites.");
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "RotationalVel", 0.0f, eCurve_None, "Specifies the rotational speed of the sprite in degrees per second. This only applies to camera aligned sprites.");
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "ToCameraOffset", 0.0f, eCurve_None, "This specifies a distance that the sprite will be moved towards the camera, often useful for biasing sprites to avoid clipping. This only applies to camera aligned sprites.");
	pList->AddTail( fxProp );

	//-------------------------------------------
	fxProp.SetupTextLine("Sprite Rendering");
	pList->AddTail(fxProp);

	fxProp.SetupPath( "Material", "", "Material Files (*." RESEXT_MATERIAL ")|*." RESEXT_MATERIAL "|All Files (*.*)|*.*||", eCurve_None, "Determines the material that will be used when rendering the sprite");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin( "AspectWidth", 1.0f, 0.0f, eCurve_None, "Specifies the width of the texture to use for aspect ratio determination" );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "AspectHeight", 1.0f, 0.0f, eCurve_None, "Specifies the height of the texture to use for aspect ratio determination" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "Solid", false, eCurve_None, "Determines if the sprite should be considered solid or translucent");
	pList->AddTail( fxProp );	

	fxProp.SetupEnumBool( "TranslucentLight", true, eCurve_None, "For translucent objects, this determines if lighting should be approximated or if it should be fullbright" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "TwoSided", false, eCurve_None, "Determines if the sprite should have both a front and a back side. Should only be used when the back side of the sprite can be seen.");
	pList->AddTail( fxProp );	

	fxProp.SetupEnum( "InSky", SKY_PROP_DEFAULT, SKY_PROP_ENUM, eCurve_None, SKY_PROP_DESCRIPTION);
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "PlayerView", false, eCurve_None, "Determines if the sprite should be rendered in the player view, which means that it should have its Z values adjusted to not be clipped into nearby walls");
	pList->AddTail( fxProp );	
}

//------------------------------------------------------------------
//
//   FUNCTION : CBaseSpriteFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CBaseSpriteFX::CBaseSpriteFX( CBaseFX::FXType nType ) :	
	CBaseFX( nType ),
	m_hObject(NULL),
	m_fWidth(0.0f),
	m_fCurrRotationRad(0.0f)
{	
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CBaseSpriteFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CBaseSpriteFX::~CBaseSpriteFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CBaseSpriteFX
//
//------------------------------------------------------------------

bool CBaseSpriteFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation
	if( !CBaseFX::Init(pBaseData, pProps))
		return false;
	
	// Combine the direction we would like to face with our parents rotation...
	ObjectCreateStruct ocs;
	
	GetCurrentTransform(0.0f, ocs.m_Pos, ocs.m_Rotation);	

	//create a custom render object with the associated material
	ocs.m_ObjectType		= OT_CUSTOMRENDER;
	
	if(!GetProps()->m_bSolid)
		ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	if(!GetProps()->m_bTranslucentLight)
		ocs.m_Flags |= FLAG_NOLIGHT;

	//setup whether or not it is in the sky
	ocs.m_Flags2 |= GetSkyFlags(GetProps()->m_eInSky);
	
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

	//handle random rotation
	if(GetProps()->m_bRandomRotation)
		m_fCurrRotationRad = GetRandom(0.0f, MATH_TWOPI);

	// Success !!
	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CBaseSpriteFX
//
//------------------------------------------------------------------

void CBaseSpriteFX::Term()
{
	if (m_hObject) 
	{
		g_pLTClient->RemoveObject(m_hObject);
		m_hObject = NULL;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CBaseSpriteFX
//
//------------------------------------------------------------------

bool CBaseSpriteFX::Update(float tmFrameTime)
{
	// Base class update first
	BaseUpdate(tmFrameTime);

	//update our object position
	LTRigidTransform tObjTrans;
	GetCurrentTransform(GetUnitLifetime(), tObjTrans.m_vPos, tObjTrans.m_rRot);
	g_pLTClient->SetObjectTransform(m_hObject, tObjTrans);

	//update our rotation and keep it in a reasonable numerical range
	m_fCurrRotationRad += GetProps()->m_fRotationVelRad * tmFrameTime;
	m_fCurrRotationRad = fmodf(m_fCurrRotationRad, MATH_TWOPI);

	return true;
}

//called to enumerate through each of the objects and will call into the provided function for each
void CBaseSpriteFX::EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData)
{
	if(m_hObject)
	{
		pfnObjectCB(this, m_hObject, pUserData);
	}
}

//called to set the width used by the visibility, this will update the visibility primitive
//of the sprite based upon its rendering style. 
void CBaseSpriteFX::SetVisScale(float fScale)
{
	//determine the half dimensions of the sprite
	float fWidth = fScale / 2.0f;
	float fHeight = fWidth * GetProps()->m_fAspectRatio;
	float fDiag;

	if(GetProps()->m_bAlignToCamera)
	{
		fDiag = LTSqrt(LTSqr(fWidth) + LTSqr(fHeight) + LTSqr(GetProps()->m_fToCameraOffset));
	}
	else
	{
		fDiag = LTSqrt(LTSqr(fWidth) + LTSqr(fHeight));

		if(GetProps()->m_bAlignAroundZ)
			fDiag *= 2.0f;
	}
	
	g_pLTClient->GetCustomRender()->SetVisBoundingSphere(m_hObject, LTVector(0.0f, 0.0f, 0.0f), fDiag);	
}

//called to change the scale of the sprite. This will update the visibile bounding area
void CBaseSpriteFX::SetScale(float fScale)
{
	//and of course, save the width for future use
	m_fWidth = fScale;
}

//hook for the custom render object, this will just call into the render function
void CBaseSpriteFX::CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
{
	((CBaseSpriteFX*)pUser)->RenderSprite(pInterface, tCamera);
}

//function that handles the custom rendering
void CBaseSpriteFX::RenderSprite(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera)
{
	//setup our vertex declaration
	if(pInterface->SetVertexDeclaration(g_ClientFXVertexDecl.GetTexTangentSpaceDecl()) != LT_OK)
		return;

	//bind a quad index stream
	if(pInterface->BindQuadIndexStream() != LT_OK)
		return;

	//determine how many indices we are going to need
	uint32 nNumIndices  = (GetProps()->m_bTwoSided) ? 12 : 6;
	uint32 nNumVertices = (GetProps()->m_bTwoSided) ? 8 : 4;

	//sanity check to ensure that we can at least render a sprite
	LTASSERT(QUAD_RENDER_INDEX_STREAM_SIZE >= nNumIndices, "Error: Quad index list is too small to render a sprite");
	LTASSERT(DYNAMIC_RENDER_VERTEX_STREAM_SIZE / sizeof(STexTangentSpaceVert) >= nNumVertices, "Error: Dynamic vertex buffer size is too small to render a sprite");

	//determine the up and right vectors for the sprite
	LTVector vTangent, vBinormal;

	//get the position of this sprite
	LTRigidTransform tObjTransform;
	g_pLTClient->GetObjectTransform(m_hObject, &tObjTransform);

	//determine the center of this sprite
	LTVector vCenter = tObjTransform.m_vPos;

	//determine the orientation of the sprite based upon its facing
	if(GetProps()->m_bAlignToCamera)
	{
		//apply the to camera offset
		LTVector vToCamera = tCamera.m_vPos - vCenter;
		float fScale = GetProps()->m_fToCameraOffset / vToCamera.Mag();
		vCenter += vToCamera * fScale;

		//perform the rotation
		float fCosAng = LTCos(m_fCurrRotationRad);
		float fSinAng = LTSin(m_fCurrRotationRad);

		LTVector vRight = tCamera.m_rRot.Right();
		LTVector vUp = -tCamera.m_rRot.Up();

		vTangent	= fCosAng * vRight + fSinAng * vUp;
		vBinormal	= fCosAng * vUp - fSinAng * vRight;
	}
	else if(GetProps()->m_bAlignAroundZ)
	{
		//we want to orient around the Z axis and align to the camera

		//we need to determine our U vector, which is always our forward
		LTVector vU = tObjTransform.m_rRot.Forward();

		//and now we want to offset our center so that we are anchored on the right hand
		//side of the sprite to the point
		vCenter += vU * (m_fWidth * 0.5f);

		//determine the axis from our camera to our object
		LTVector vToCamera = tCamera.m_vPos - vCenter;

		//and now derive our V vector from the forward and the direction to the camera
		LTVector vV = vToCamera.Cross(vU);

		//detect degenerate cases
		if(vV == LTVector::GetIdentity())
		{
			//degenerate case, any orientation will work fine since the sprite won't
			//be visible anyway
			vV.Init(0.0f, 1.0f, 0.0f);
		}

		//and normalize our vector
		vV.Normalize();

		//now we can determine our tangent and binormal vectors
		vTangent	= -vU;
		vBinormal	= vV;
	}
	else
	{
		vTangent	= -tObjTransform.m_rRot.Right();
		vBinormal	= -tObjTransform.m_rRot.Up();
	}
	
	LTVector vNormal = vBinormal.Cross(vTangent);

	//scale the right and down values to be the appropriate size
	LTVector vRight	= vTangent * m_fWidth * -0.5f;
	LTVector vDown	= vBinormal * m_fWidth * GetProps()->m_fAspectRatio * 0.5f;


	//lock down our buffer for rendering
	SDynamicVertexBufferLockRequest LockRequest;
	if(pInterface->LockDynamicVertexBuffer(nNumVertices, LockRequest) != LT_OK)
		return;

	//fill in our sprite vertices
	STexTangentSpaceVert* pCurrOut = (STexTangentSpaceVert*)LockRequest.m_pData;

	uint32 nColor = SETRGBA(	(uint8)(m_vColor.x * 255.0f), 
								(uint8)(m_vColor.y * 255.0f), 
								(uint8)(m_vColor.z * 255.0f), 
								(uint8)(m_vColor.w * 255.0f));

	//fill in the particle vertices
	pCurrOut[0].m_vPos = vCenter + vRight - vDown;
	pCurrOut[0].m_vUV.Init(0.0f, 0.0f);
	
	pCurrOut[1].m_vPos = vCenter - vRight - vDown;
	pCurrOut[1].m_vUV.Init(1.0f, 0.0f);
	
	pCurrOut[2].m_vPos = vCenter - vRight + vDown;
	pCurrOut[2].m_vUV.Init(1.0f, 1.0f);

	pCurrOut[3].m_vPos = vCenter + vRight + vDown;
	pCurrOut[3].m_vUV.Init(0.0f, 1.0f);

	//setup the remaining vertex components
	for(uint32 nCurrVert = 0; nCurrVert < 4; nCurrVert++)
	{
		pCurrOut[nCurrVert].m_nPackedColor = nColor;
		pCurrOut[nCurrVert].m_vNormal = vNormal;
		pCurrOut[nCurrVert].m_vTangent = vTangent;
		pCurrOut[nCurrVert].m_vBinormal = vBinormal;
	}

	//and fill in the back side if appropriate
	if(GetProps()->m_bTwoSided)
	{
		pCurrOut[4].m_vPos = vCenter - vRight - vDown;
		pCurrOut[4].m_vUV.Init(1.0f, 0.0f);
		
		pCurrOut[5].m_vPos = vCenter + vRight - vDown;
		pCurrOut[5].m_vUV.Init(0.0f, 0.0f);
		
		pCurrOut[6].m_vPos = vCenter + vRight + vDown;
		pCurrOut[6].m_vUV.Init(0.0f, 1.0f);

		pCurrOut[7].m_vPos = vCenter - vRight + vDown;
		pCurrOut[7].m_vUV.Init(1.0f, 1.0f);

		//setup the remaining vertex components
		for(uint32 nCurrVert = 4; nCurrVert < 8; nCurrVert++)
		{
			pCurrOut[nCurrVert].m_nPackedColor = nColor;
			pCurrOut[nCurrVert].m_vNormal = -vNormal;
			pCurrOut[nCurrVert].m_vTangent = -vTangent;
			pCurrOut[nCurrVert].m_vBinormal = vBinormal;
		}
	}

	//unlock and render the batch
	pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
	pInterface->RenderIndexed(	eCustomRenderPrimType_TriangleList, 
								0, nNumIndices, LockRequest.m_nStartIndex, 
								0, nNumVertices);
}
