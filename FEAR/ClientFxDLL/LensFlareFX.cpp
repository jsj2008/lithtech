//------------------------------------------------------------------
//
//   MODULE  : BASESPRITEFX.CPP
//
//   PURPOSE : Implements class CLensFlareFX
//
//   CREATED : On 11/23/98 At 6:21:37 PM
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "ClientFX.h"
#include "LensFlareFX.h"
#include "iltrenderer.h"
#include "resourceextensions.h"
#include "ClientFXVertexDeclMgr.h"
#include "iperformancemonitor.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXLensFlare("ClientFX_LensFlare", "ClientFX");

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLensFlareProps::CLensFlareProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CLensFlareProps::CLensFlareProps() : 
	m_bDirectional(false),
	m_eInSky(eFXSkySetting_None),
	m_nNumImages(0),
	m_nNumComponents(0),
	m_pszMaterial(NULL),
	m_fVisibleRadius(100.0f)
{
}

CLensFlareProps::SFlareComponent::SFlareComponent() :
	m_nImage(0),
	m_fScale(0.0f),
	m_fOffset(0.0f),
	m_vColor(1.0f, 1.0f, 1.0f, 1.0f)
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLensFlareProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CLensFlareProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	char pszPropName[128];

	//see if it is one of the components, and if so, load it up appropriately
	for(uint32 nCurrComponent = 0; nCurrComponent < knMaxComponents; nCurrComponent++)
	{
		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Image%d", nCurrComponent);
		if(LTStrIEquals(pszPropName, pszName))
		{
			m_Components[nCurrComponent].m_nImage = CFxProp_Int::Load(pStream);
			return true;
		}

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Scale%d", nCurrComponent);
		if(LTStrIEquals(pszPropName, pszName))
		{
			m_Components[nCurrComponent].m_fScale = CFxProp_Float::Load(pStream);
			return true;
		}

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Offset%d", nCurrComponent);
		if(LTStrIEquals(pszPropName, pszName))
		{
			m_Components[nCurrComponent].m_fOffset = CFxProp_Float::Load(pStream);
			return true;
		}

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Color%d", nCurrComponent);
		if(LTStrIEquals(pszPropName, pszName))
		{
			m_Components[nCurrComponent].m_vColor = CFxProp_Color4f::Load(pStream);
			return true;
		}
	}


	if( LTStrIEquals( pszName, "Material" ))
	{
		m_pszMaterial = CFxProp_String::Load(pStream, pszStringTable );
	}
	else if( LTStrIEquals( pszName, "InSky" ))
	{
		m_eInSky = (EFXSkySetting)CFxProp_Enum::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Directional" ))
	{
		m_bDirectional = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "NumImages" ))
	{
		m_nNumImages = CFxProp_Int::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Color" ))
	{
		m_Color.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "VisibleDistance" ))
	{
		m_VisibleDistance.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "VisibleRadius" ))
	{
		m_fVisibleRadius = CFxProp_Float::Load(pStream);
	}
	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

bool CLensFlareProps::PostLoadProperties()
{
	//we need to count up the number of lens flare components that are valid (scale > 0 and image in the
	//valid range), move those into a flat list, and count those up
	m_nNumComponents = 0;

	//just find each valid one, and move it to the available spot
	for(uint32 nCurrComponent = 0; nCurrComponent < knMaxComponents; nCurrComponent++)
	{
		//see if this one is valid
		if( (m_Components[nCurrComponent].m_fScale > 0.001f) && 
			(m_Components[nCurrComponent].m_nImage < m_nNumImages))
		{
			//this one is valid, so move it to a valid spot (this works because m_nNumComponents
			//is always <= nCurrComponent, so only invalid components will ever get stomped, which
			//we don't care about anyway)
			m_Components[m_nNumComponents] = m_Components[nCurrComponent];

			//and move onto the next component
			m_nNumComponents++;
		}
	}

	return CBaseFXProps::PostLoadProperties();
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CLensFlareProps::CollectResources(IFXResourceCollector& Collector)
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
void fxGetLensFlareProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;

	// Add the base props
	AddBaseProps(pList);

	// Add all the props to the list
	fxProp.SetupPath( "Material", "", "Material Files (*." RESEXT_MATERIAL ")|*." RESEXT_MATERIAL "|All Files (*.*)|*.*||", eCurve_None, "Determines the material that will be used when rendering the lens flare. This material should use a texture that has a series of lens flares images laid out in a horizontal strip");
	pList->AddTail(fxProp);

	fxProp.SetupEnum( "InSky", SKY_PROP_DEFAULT, SKY_PROP_ENUM, eCurve_None, SKY_PROP_DESCRIPTION);
	pList->AddTail( fxProp );

	fxProp.SetupIntMin( "NumImages", 1, 1, eCurve_None, "The number of images contained in the material textures that can be used by the lens flare");
	pList->AddTail(fxProp);

	fxProp.SetupColor( "Color", 0xFFFFFFFF, eCurve_Linear, "The color of this lens flare over time. This can be used to fade the lens flare in and out and change their color");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin( "VisibleDistance", 1000.0f, 0.0f, eCurve_Linear, "The maximum visible distance of this lens flare. The lens flare will shrink until it hits this distance, at which point it will be invisible.");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin( "VisibleRadius", 100.0f, 0.0f, eCurve_None, "The radius of the visible sphere that will be used for the lens flare. If this sphere is not visible, the lens flare will not be rendered.");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( "Directional", false, eCurve_None, "Determines if this lens flare should be treated directionally, indicating that it is fullest intensity when the viewer is looking directly against the forward direction, and fades as the angle increases.");
	pList->AddTail(fxProp);

	//and now add all of our lens flare components
	char pszPropName[128];
	for(uint32 nCurrComponent = 0; nCurrComponent < CLensFlareProps::knMaxComponents; nCurrComponent++)
	{
		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Component%d", nCurrComponent);
		fxProp.SetupTextLine(pszPropName);
		pList->AddTail(fxProp);

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Image%d", nCurrComponent);
		fxProp.SetupIntMin(pszPropName, 0, 0, eCurve_None, "The index into the image list stored in the material as a horizontal series of images");
		pList->AddTail(fxProp);
		
		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Scale%d", nCurrComponent);
		fxProp.SetupFloatMin(pszPropName, 0.0f, 0.0f, eCurve_None, "The scale of this lens flare component with respect to the source lens flare");
		pList->AddTail(fxProp);		

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Offset%d", nCurrComponent);
		fxProp.SetupFloat(pszPropName, 1.0f, eCurve_None, "The offset along the lens flare vector, that passes through the lens flare center and the center of the camera");
		pList->AddTail(fxProp);		

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "Color%d", nCurrComponent);
		fxProp.SetupColor(pszPropName, 0xFFFFFFFF, eCurve_None, "The color of this lens flare component. This is applied along with the overall color of the lens flare");
		pList->AddTail(fxProp);		
	}	
}

//------------------------------------------------------------------
//
//   FUNCTION : CLensFlareFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CLensFlareFX::CLensFlareFX() :	
	CBaseFX(CBaseFX::eLensFlareFX),
	m_hObject(NULL),
	m_fFlareAlpha(1.0f)
{	
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CLensFlareFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CLensFlareFX::~CLensFlareFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CLensFlareFX
//
//------------------------------------------------------------------

bool CLensFlareFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation
	if( !CBaseFX::Init(pBaseData, pProps))
		return false;
	
	// Combine the direction we would like to face with our parents rotation...
	ObjectCreateStruct ocs;
	
	GetCurrentTransform(0.0f, ocs.m_Pos, ocs.m_Rotation);	

	//create a custom render object with the associated material
	ocs.m_ObjectType	= OT_CUSTOMRENDER;
	ocs.m_Flags2		|= FLAG2_FORCETRANSLUCENT | FLAG_NOLIGHT;

	//setup whether or not it is in the sky
	ocs.m_Flags2 |= GetSkyFlags(GetProps()->m_eInSky);
	
	m_hObject = g_pLTClient->CreateObject( &ocs );
	if( !m_hObject )
		return false;

	//setup the callback on the object so that it will render us
	g_pLTClient->GetCustomRender()->SetRenderingSpace(m_hObject, eRenderSpace_World);
	g_pLTClient->GetCustomRender()->SetRenderCallback(m_hObject, CustomRenderCallback);
	g_pLTClient->GetCustomRender()->SetVisibleCallback(m_hObject, CustomRenderVisibleCallback);
	g_pLTClient->GetCustomRender()->SetCallbackUserData(m_hObject, this);

	//load up the material for this particle system, and assign it to the object
	HMATERIAL hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance(GetProps()->m_pszMaterial);
	g_pLTClient->GetCustomRender()->SetMaterial(m_hObject, hMaterial);
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	//setup a bounding sphere for our lens flare, which just needs to be large enough to determine if the lens
	//flare point is visible or not
	g_pLTClient->GetCustomRender()->SetVisBoundingSphere(m_hObject, LTVector(0, 0, 0), GetProps()->m_fVisibleRadius);

	// Success !!
	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CLensFlareFX
//
//------------------------------------------------------------------

void CLensFlareFX::Term()
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
//   PURPOSE  : Updates class CLensFlareFX
//
//------------------------------------------------------------------

bool CLensFlareFX::Update(float tmFrameTime)
{
	// Base class update first
	BaseUpdate(tmFrameTime);

	//update our position accordingly
	LTRigidTransform tObjTrans;
	GetCurrentTransform(GetUnitLifetime(), tObjTrans.m_vPos, tObjTrans.m_rRot);
	g_pLTClient->SetObjectTransform(m_hObject, tObjTrans);

	return true;
}

//called to enumerate through each of the objects and will call into the provided function for each
void CLensFlareFX::EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData)
{
	if(m_hObject)
	{
		pfnObjectCB(this, m_hObject, pUserData);
	}
}

bool CLensFlareFX::CustomRenderVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera, void* pUser)
{
	return ((CLensFlareFX*)pUser)->HandleVisibleCallback(tCamera, tSkyCamera);
}

//function that handles the visible callback
bool CLensFlareFX::HandleVisibleCallback(const LTRigidTransform& tCamera, const LTRigidTransform& tSkyCamera)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXLensFlare);

	//determine if we are in the sky or not
	bool bInSky = IsInSky(GetProps()->m_eInSky);

	//determine our camera position to use for visibility determination
	LTRigidTransform tLensCamera = (bInSky) ? tSkyCamera : tCamera;

	//and now get the flare information
	LTRigidTransform tFlareTrans;
	g_pLTClient->GetObjectTransform(m_hObject, &tFlareTrans);

	//determine the maximum visible distance of this flare sprite
	float fUnitLifetime = GetUnitLifetime();
	float fMaxDist = GetProps()->m_VisibleDistance.GetValue(fUnitLifetime);

	//determine if we are outside of the distance
	LTVector vFlareToCamera = tLensCamera.m_vPos - tFlareTrans.m_vPos;
	float fDistToCameraSqr = vFlareToCamera.MagSqr();
	if(fDistToCameraSqr > LTSqr(fMaxDist))
		return false;

	//normalize the flare to camera vector
	vFlareToCamera /= LTSqrt(fDistToCameraSqr);

	//also determine the directional component of the viewer
	float fViewerDirectionalScale = -(tLensCamera.m_rRot.Forward().Dot(vFlareToCamera));
	if(fViewerDirectionalScale <= 0.0f)
		return false;

	//if this is a directional sprite, we can perform back face culling
	float fDirectionalScale = 1.0f;
	if(GetProps()->m_bDirectional)
	{
		fDirectionalScale = tFlareTrans.m_rRot.Forward().Dot(vFlareToCamera);

		//bail if we are invisible
		if(fDirectionalScale <= 0.0f)
			return false;
	}

	//if we can't see the point of the lens flare, we don't want to render!
	if(!IsPointVisible(tCamera.m_vPos, tFlareTrans.m_vPos, bInSky)) 
		return false;

	//determine our flare alpha
	m_fFlareAlpha = fViewerDirectionalScale * fDirectionalScale;

	return true;
}

//hook for the custom render object, this will just call into the render function
void CLensFlareFX::CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
{
	((CLensFlareFX*)pUser)->RenderLensFlare(pInterface, tCamera);
}

//function that handles the custom rendering
void CLensFlareFX::RenderLensFlare(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXLensFlare);

	//setup our vertex declaration
	if(pInterface->SetVertexDeclaration(g_ClientFXVertexDecl.GetTexTangentSpaceDecl()) != LT_OK)
		return;

	//bind a quad index stream
	if(pInterface->BindQuadIndexStream() != LT_OK)
		return;

	//determine how many indices we are going to need
	uint32 nNumIndices  = 6 * GetProps()->m_nNumComponents;
	uint32 nNumVertices = 4 * GetProps()->m_nNumComponents;

	//sanity check to ensure that we can at least render a sprite
	LTASSERT(QUAD_RENDER_INDEX_STREAM_SIZE >= nNumIndices, "Error: Quad index list is too small to render a lens flare");
	LTASSERT(DYNAMIC_RENDER_VERTEX_STREAM_SIZE / sizeof(STexTangentSpaceVert) >= nNumVertices, "Error: Dynamic vertex buffer size is too small to render a lens flare");

	//determine the orientation of the sprite based upon its facing
	//we want to orient the sprite based upon the alignment of the camera

	LTVector vCameraRight	= tCamera.m_rRot.Right();
	LTVector vCameraUp		= tCamera.m_rRot.Up();
	LTVector vCameraForward	= tCamera.m_rRot.Forward();

	//determine the tangent space of these vertices
	LTVector vTangent	= vCameraRight;
	LTVector vBinormal	= -vCameraUp;
	LTVector vNormal	= -vCameraForward;

	//lock down our buffer for rendering
	SDynamicVertexBufferLockRequest LockRequest;
	if(pInterface->LockDynamicVertexBuffer(nNumVertices, LockRequest) != LT_OK)
		return;

	//determine our unit lifetime
	float fUnitLifetime = GetUnitLifetime();

	//get our object position
	LTVector vFlarePos;
	g_pLTClient->GetObjectPos(m_hObject, &vFlarePos);

	//fill in our sprite vertices
	STexTangentSpaceVert* pCurrOut = (STexTangentSpaceVert*)LockRequest.m_pData;

	//determine the color of this lens flare
	LTVector4 vColor = GetProps()->m_Color.GetValue(fUnitLifetime);

	//determine the directional vector of the lens flare (the vector that goes from the center of the
	//camera to the center of the lens flare)

	//get the camera if it were moved forward along the forward vector so the screen plane would contain 
	//the lens flare
	LTVector vCameraPlanePos = tCamera.m_vPos + vCameraForward * vCameraForward.Dot(vFlarePos - tCamera.m_vPos);

	//the flare vector, note this MUST not be normalized since the magnitude is important to us
	LTVector vFlareVec = vFlarePos - vCameraPlanePos;

	//determine the width and height of each image in the texture
	float fImageWidth  = 1.0f / (float)GetProps()->m_nNumImages;

	//the starting index we will be filling in
	uint32 nStartingIndex = 0;

	//scale the overall alpha of the flare based upon the view and directional attenuation
	vColor.w *= m_fFlareAlpha;

	//note that the flare scale is HALF of the flare, not the full thing since we are offsetting from the component
	//center
	float fFlareScale = 0.5f;

	//fill in the vertices
	for(uint32 nCurrComponent = 0; nCurrComponent < GetProps()->m_nNumComponents; nCurrComponent++)
	{
		//determine the component we will be rendering
		const CLensFlareProps::SFlareComponent& Component = GetProps()->m_Components[nCurrComponent];

		//determine the color of this lens flare component
		LTVector4 vCombinedColor = (Component.m_vColor * vColor) * 255.0f;
		uint32 nColor = SETRGBA((uint8)(vCombinedColor.x), (uint8)(vCombinedColor.y), (uint8)(vCombinedColor.z), (uint8)(vCombinedColor.w));  

		//determine the center of this flare component
		LTVector vComponentCenter = vCameraPlanePos + vFlareVec * Component.m_fOffset;

		//and the scale of this component
		float fComponentScale = fFlareScale * Component.m_fScale;
		LTVector vScaledRight = vTangent * -fComponentScale;
		LTVector vScaledDown  = vBinormal * fComponentScale;

		//and the UV's of this image
		uint32 nImageIndex = Component.m_nImage;
		float fULeft  = (float)nImageIndex * fImageWidth;
		float fURight = (float)(nImageIndex + 1) * fImageWidth;

		pCurrOut[nStartingIndex + 0].m_vPos = vComponentCenter + vScaledRight - vScaledDown;
		pCurrOut[nStartingIndex + 0].m_vUV.Init(fULeft, 0.0f);
		
		pCurrOut[nStartingIndex + 1].m_vPos = vComponentCenter - vScaledRight - vScaledDown;
		pCurrOut[nStartingIndex + 1].m_vUV.Init(fURight, 0.0f);
		
		pCurrOut[nStartingIndex + 2].m_vPos = vComponentCenter - vScaledRight + vScaledDown;
		pCurrOut[nStartingIndex + 2].m_vUV.Init(fURight, 1.0f);

		pCurrOut[nStartingIndex + 3].m_vPos = vComponentCenter + vScaledRight + vScaledDown;
		pCurrOut[nStartingIndex + 3].m_vUV.Init(fULeft, 1.0f);

		//setup the remaining vertex components
		for(uint32 nCurrVert = 0; nCurrVert < 4; nCurrVert++)
		{
			pCurrOut[nStartingIndex + nCurrVert].m_nPackedColor = nColor;
			pCurrOut[nStartingIndex + nCurrVert].m_vNormal		= vNormal;
			pCurrOut[nStartingIndex + nCurrVert].m_vTangent		= vTangent;
			pCurrOut[nStartingIndex + nCurrVert].m_vBinormal	= vBinormal;
		}

		//move onto the next set of four verts to fill in
		nStartingIndex += 4;
	}

	//unlock and render the batch
	pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
	pInterface->RenderIndexed(	eCustomRenderPrimType_TriangleList, 
								0, nNumIndices, LockRequest.m_nStartIndex, 
								0, nNumIndices);
}
