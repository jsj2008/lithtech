#include "stdafx.h"
#include "OverlayFX.h"
#include "clientfx.h"
#include "resourceextensions.h"

//-------------------------------------------------------------------------------------
// Utility functions
//-------------------------------------------------------------------------------------

//given a value and a range, this will return the parameterized distance in that range. Useful
//for clipping UV's
static float ParameterizeValue(float fValue, float fMin, float fMax)
{
	return (fValue - fMin) / (fMax - fMin);
}

//called to setup the specified edge polygon using the data provided. This will setup the vertices,
//and also setup the texture so that it maps from [0..1] across the render target in each dimension
static void SetupEdgeQuad(	LT_POLYGTTS4& Quad, const LTVector2& vVert1, const LTVector2& vVert2,
							const LTVector2& vVert3, const LTVector2& vVert4, float fRTWidth, 
							float fRTHeight, uint32 nColor)
{
	DrawPrimSetXYZ(Quad, 0, vVert1.x, vVert1.y, 1.0f);
	DrawPrimSetXYZ(Quad, 1, vVert2.x, vVert2.y, 1.0f);
	DrawPrimSetXYZ(Quad, 2, vVert3.x, vVert3.y, 1.0f);
	DrawPrimSetXYZ(Quad, 3, vVert4.x, vVert4.y, 1.0f);

	DrawPrimSetUV(Quad, 0, ParameterizeValue(vVert1.x, 0.0f, fRTWidth), ParameterizeValue(vVert1.y, 0.0f, fRTHeight));
	DrawPrimSetUV(Quad, 1, ParameterizeValue(vVert2.x, 0.0f, fRTWidth), ParameterizeValue(vVert2.y, 0.0f, fRTHeight));
	DrawPrimSetUV(Quad, 2, ParameterizeValue(vVert3.x, 0.0f, fRTWidth), ParameterizeValue(vVert3.y, 0.0f, fRTHeight));
	DrawPrimSetUV(Quad, 3, ParameterizeValue(vVert4.x, 0.0f, fRTWidth), ParameterizeValue(vVert4.y, 0.0f, fRTHeight));

	DrawPrimSetRGBA(Quad, nColor);
	DrawPrimSetTangentSpace(Quad, LTVector(0.0f, 0.0f, -1.0f), LTVector(0.0f, 1.0f, 0.0f), LTVector(-1.0f, 0.0f, 0.0f));
}


//-------------------------------------------------------------------------------------
// COverlayProps
//-------------------------------------------------------------------------------------

COverlayProps::SFloatParam::SFloatParam() :
	m_pszMaterialParam(NULL)
{
}

COverlayProps::COverlayProps() :
	m_nLayer(0),
	m_bAllowHigherLayers(true),
	m_pszOverlayMaterial(NULL),
	m_pszEdgeMaterial(NULL)
{
}

COverlayProps::~COverlayProps()
{
}

//handles loading up a single property from the specified file
bool COverlayProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	//-------------------------
	// Ordering

	if( LTStrIEquals( pszName, "Layer" ) )
	{
		m_nLayer = (uint32)LTMAX(0, CFxProp_Int::Load(pStream));
	}
	else if( LTStrIEquals( pszName, "AllowHigherLayers" ))
	{
		m_bAllowHigherLayers = CFxProp_EnumBool::Load(pStream);
	}
	
	//-------------------------
	// Materials

	else if( LTStrIEquals( pszName, "OverlayMaterial" ))
	{
		m_pszOverlayMaterial = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if( LTStrIEquals( pszName, "EdgeMaterial" ))
	{
		m_pszEdgeMaterial = CFxProp_String::Load(pStream, pszStringTable);
	}
	
	//-------------------------
	// Display

	else if( LTStrIEquals( pszName, "Color" ))
	{
		m_cfcColor.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "AspectWidth" ))
	{
		m_ffcAspectWidth.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "AspectHeight" ))
	{
		m_ffcAspectHeight.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "YHeight" ))
	{
		m_ffcYHeight.Load(pStream, pCurveData);
	}
	else
	{
		//-------------------------
		// Float Material Params

		for(uint32 nCurrFloat = 0; nCurrFloat < COverlayProps::knNumFloatParams; nCurrFloat++)
		{
			char pszPropName[64];

			LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "FloatParam%d", nCurrFloat);
			if(LTStrIEquals(pszPropName, pszName))
			{
				m_FloatParams[nCurrFloat].m_pszMaterialParam = CFxProp_String::Load(pStream, pszStringTable);
				return true;
			}

			LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "FloatValue%d", nCurrFloat);
			if(LTStrIEquals(pszPropName, pszName))
			{
				m_FloatParams[nCurrFloat].m_ffcParamValue.Load(pStream, pCurveData);
				return true;
			}
		}

		//we didn't load up any material parameters, so we need to simply load the base fx props
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void COverlayProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectResource(m_pszOverlayMaterial);
	Collector.CollectResource(m_pszEdgeMaterial);
}


//called to get the appropriate properties for the overlay effect
void fxGetOverlayProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc	fxProp;

	// Add the generic "every effect has these" props
	AddBaseProps( pList );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Ordering");
	pList->AddTail(fxProp);

	fxProp.SetupIntMin("Layer", 0, 0, eCurve_None, "Indicates the layer that this overlay will be a part of. Higher level layers are rendered first, with lower level layers rendered on top of them.");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool("AllowHigherLayers", true, eCurve_None, "Indicates whether or not this overlay should prevent any other overlays of higher layer from being rendered");
	pList->AddTail(fxProp);

	//------------------------------------------------------------
	fxProp.SetupTextLine("Materials");
	pList->AddTail(fxProp);

	fxProp.SetupPath("OverlayMaterial", "", "Material Files (*." RESEXT_MATERIAL ")|*." RESEXT_MATERIAL "|All Files (*.*)|*.*||", eCurve_None, "The material to use when rendering the overlay. This material must use a translucent shader in order to be visible.");
	pList->AddTail(fxProp);

	fxProp.SetupPath("EdgeMaterial", "", "Material Files (*." RESEXT_MATERIAL ")|*." RESEXT_MATERIAL "|All Files (*.*)|*.*||", eCurve_None, "The material to use when rendering the parts of the screen not covered by the overlay. This material must use a translucent shader in order to be visible.");
	pList->AddTail(fxProp);

	//------------------------------------------------------------
	fxProp.SetupTextLine("Display");
	pList->AddTail(fxProp);

	fxProp.SetupColor("Color", 0xFFFFFFFF, eCurve_Linear, "The color of the overlay over its lifetime");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("AspectWidth", 0.0f, 0.0f, eCurve_Linear, "This is the width to use when determining the aspect ratio of the overlay. This should be set to zero to use the aspect ratio of the render target");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("AspectHeight", 0.0f, 0.0f, eCurve_Linear, "This is the height to use when determining the aspect ratio of the overlay. This should be set to zero to use the aspect ratio of the render target");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMinMax("YHeight", 1.0f, 0.0f, 1.0f, eCurve_Linear, "This indicates the height of the render target that should be used when rendering the overlay. This is a percentage from zero to one");
	pList->AddTail(fxProp);

	//------------------------------------------------------------
	fxProp.SetupTextLine("Material Float Parameters");
	pList->AddTail(fxProp);

	for(uint32 nCurrFloat = 0; nCurrFloat < COverlayProps::knNumFloatParams; nCurrFloat++)
	{
		char pszPropName[64];

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "FloatParam%d", nCurrFloat);
		fxProp.SetupString(pszPropName, "", eCurve_None, "Specifies the name of the floating point parameter in the material that should be modified");
		pList->AddTail(fxProp);

		LTSNPrintF(pszPropName, LTARRAYSIZE(pszPropName), "FloatValue%d", nCurrFloat);
		fxProp.SetupFloat(pszPropName, 0.0f, eCurve_Linear, "Specifies the value to use for the associated parameter");
		pList->AddTail(fxProp);
	}
}

//-------------------------------------------------------------------------------------
// COverlayFX
//-------------------------------------------------------------------------------------

COverlayFX::COverlayFX() :
	CBaseFX(eOverlayFX)
{
	m_OverlayLink.SetData(this);
}

COverlayFX::~COverlayFX()
{
	Term();
}

//initializes the effect based upon the passed in data
bool COverlayFX::Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps)
{
	if(!CBaseFX::Init(pData, pProps))
		return false;

	//we need to handle loading up our materials
	m_hOverlayMaterial	= g_pLTClient->GetRenderer()->CreateMaterialInstance(GetProps()->m_pszOverlayMaterial);
	m_hEdgeMaterial		= g_pLTClient->GetRenderer()->CreateMaterialInstance(GetProps()->m_pszEdgeMaterial);

	return true;
}

//terminates the effect
void COverlayFX::Term()
{
	//release our material
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(m_hOverlayMaterial);
	m_hOverlayMaterial = NULL;

	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(m_hEdgeMaterial);
	m_hEdgeMaterial = NULL;

	m_OverlayLink.Remove();
}

//called to update the effect
bool COverlayFX::Update(float tmFrameTime)
{
	BaseUpdate(tmFrameTime);

	//since we can have shut down and init on the same frame in rare cases, make sure to not handle
	//the initial frame if we are shutting down
	if(IsShuttingDown())
		m_OverlayLink.Remove();
	else if(IsInitialFrame())
		m_pFxMgr->SubscribeOverlay(m_OverlayLink);
	
	return true;
}

//called to determine if this effect supports overlays. If it doesn't, it should return false. If it
//does, then it should provide the layer value and whether or not it allows higher layers to be displayed
bool COverlayFX::GetOverlayLayer(uint32& nLayer, bool& bAllowHigher)
{
	if(!IsActive() || IsSuspended())
		return false;

	nLayer			= GetProps()->m_nLayer;
	bAllowHigher	= GetProps()->m_bAllowHigherLayers;

	return true;
}

//called to render the overlay. This should return false if the effect does not support overlays
bool COverlayFX::RenderOverlay()
{
	float fUnitLifetime = GetUnitLifetime();

	//update our material parameters
	UpdateMaterialParams(fUnitLifetime);

	//and now actually render our overlay
	RenderOverlay(fUnitLifetime);

	//success
	return true;
}

//called to render the overlay to the current render target
void COverlayFX::RenderOverlay(float fUnitLifetime)
{
	//bail if we have no materials
	if(!m_hOverlayMaterial && !m_hEdgeMaterial)
		return;

	//get the dimensions of our current render target
	uint32 nRTWidth, nRTHeight;
	g_pLTClient->GetRenderer()->GetCurrentRenderTargetDims(nRTWidth, nRTHeight);

	//convert to float for easier comparison and compute some derived information
	float fRTWidth  = (float)nRTWidth;
	float fRTHeight = (float)nRTHeight;
	float fRTAspect = fRTWidth / fRTHeight;

	LTVector2 vRTCenter(fRTWidth * 0.5f, fRTHeight * 0.5f);
	LTRect2f rRTRect(-0.5f, -0.5f, fRTWidth, fRTHeight);

	//we need to render so extract out our dimension information
	float fAspectWidth	= GetProps()->m_ffcAspectWidth.GetValue(fUnitLifetime);
	float fAspectHeight = GetProps()->m_ffcAspectHeight.GetValue(fUnitLifetime);
	float fYHeight		= GetProps()->m_ffcYHeight.GetValue(fUnitLifetime);

	//determine the overlay aspect ratio
	float fAspectRatio = fRTAspect;

	if((fAspectWidth != 0.0f) && (fAspectHeight != 0.0f))
		fAspectRatio = fAspectWidth / fAspectHeight;

	//extract the color of our overlay
	uint32 nColor = CFxProp_Color4f::ToColor(GetProps()->m_cfcColor.GetValue(fUnitLifetime));

	//we now have all the position information we need in order to setup and render our 
	//overlay. Create our actual overlay rectangle
	float fHalfOverlayHeight	= fRTHeight * fYHeight * 0.5f;
	float fHalfOverlayWidth		= fHalfOverlayHeight * fAspectRatio;
	LTVector2 vOverlayHalfDims(fHalfOverlayWidth, fHalfOverlayHeight);

	LTRect2f rOverlay((vRTCenter - vOverlayHalfDims) - 0.5f, vRTCenter + vOverlayHalfDims);

	//we now need to handle clipping the rectangle to the overlay 
	LTRect2f rClippedOverlay = rRTRect.GetIntersection(rOverlay);

	//and derive our UV coordinates for the clipped version
	LTRect2f rOverlayUVs(	ParameterizeValue(rClippedOverlay.Left(), rOverlay.Left(), rOverlay.Right()),
							ParameterizeValue(rClippedOverlay.Top(), rOverlay.Top(), rOverlay.Bottom()),
							ParameterizeValue(rClippedOverlay.Right(), rOverlay.Left(), rOverlay.Right()),
							ParameterizeValue(rClippedOverlay.Bottom(), rOverlay.Top(), rOverlay.Bottom()));

	//adjust for the camera viewport
	HOBJECT hCamera = m_pFxMgr->GetCamera();
	HOBJECT hOldDrawprimCamera;
	LTRect2f rOldCameraRect(0.0f,0.0f, 1.0f,1.0f);
	if (hCamera != NULL)
	{
		LTRect2f rCamera;
		g_pLTClient->GetCameraRect(hCamera, rOldCameraRect);
		g_pLTClient->SetCameraRect(hCamera, LTRect2f(0.0f,0.0f, 1.0f,1.0f));
		hOldDrawprimCamera = g_pLTClient->GetDrawPrim()->GetCamera();
		g_pLTClient->GetDrawPrim()->SetCamera(hCamera);
	}
	
	//now apply bilinear filtering offsets

	//we can now setup our draw prim for rendering
	LT_POLYGTTS4 OverlayQuad;
	DrawPrimSetXYWH(OverlayQuad, rClippedOverlay.Left(), rClippedOverlay.Top(), rClippedOverlay.GetWidth(), rClippedOverlay.GetHeight());
	DrawPrimSetUVWH(OverlayQuad, rOverlayUVs.Left(), rOverlayUVs.Top(), rOverlayUVs.GetWidth(), rOverlayUVs.GetHeight());
	DrawPrimSetRGBA(OverlayQuad, nColor);
	DrawPrimSetTangentSpace(OverlayQuad, LTVector(0.0f, 0.0f, -1.0f), LTVector(0.0f, 1.0f, 0.0f), LTVector(-1.0f, 0.0f, 0.0f));

	//and render our overlay
	g_pLTClient->GetDrawPrim()->DrawPrimMaterial(&OverlayQuad, 1, m_hOverlayMaterial);

	//we now need to render the edge quads as appropriate
	LT_POLYGTTS4 EdgeQuads[4];
	uint32 nNumEdgeQuads = 0;

	//---------------
	//left 
	if(rClippedOverlay.Left() > 0.0f)
	{
		SetupEdgeQuad(EdgeQuads[nNumEdgeQuads], rRTRect.GetBottomLeft(), rRTRect.GetTopLeft(), rClippedOverlay.GetTopLeft(), rClippedOverlay.GetBottomLeft(), fRTWidth, fRTHeight, nColor);
		nNumEdgeQuads++;
	}

	//---------------
	//top
	if(rClippedOverlay.Top() > 0.0f)
	{
		SetupEdgeQuad(EdgeQuads[nNumEdgeQuads], rRTRect.GetTopLeft(), rRTRect.GetTopRight(), rClippedOverlay.GetTopRight(), rClippedOverlay.GetTopLeft(), fRTWidth, fRTHeight, nColor);
		nNumEdgeQuads++;
	}

	//---------------
	//right
	if(rClippedOverlay.Right() < fRTWidth)
	{
		SetupEdgeQuad(EdgeQuads[nNumEdgeQuads], rRTRect.GetTopRight(), rRTRect.GetBottomRight(), rClippedOverlay.GetBottomRight(), rClippedOverlay.GetTopRight(), fRTWidth, fRTHeight, nColor);
		nNumEdgeQuads++;
	}

	//---------------
	//bottom
	if(rClippedOverlay.Bottom() < fRTHeight)
	{
		SetupEdgeQuad(EdgeQuads[nNumEdgeQuads], rRTRect.GetBottomRight(), rRTRect.GetBottomLeft(), rClippedOverlay.GetBottomLeft(), rClippedOverlay.GetBottomRight(), fRTWidth, fRTHeight, nColor);
		nNumEdgeQuads++;
	}

	//and render our edges
	g_pLTClient->GetDrawPrim()->DrawPrimMaterial(EdgeQuads, nNumEdgeQuads, m_hEdgeMaterial);

	//restore the camera rectangle & drawprim camera
	if (hCamera != NULL)
	{
		g_pLTClient->SetCameraRect(hCamera, rOldCameraRect);
		g_pLTClient->GetDrawPrim()->SetCamera(hOldDrawprimCamera);
	}
}

//called to update the parameters assocaited with the materials
void COverlayFX::UpdateMaterialParams(float fUnitLifetime)
{
	//run through and install our floating point material parameters
	for(uint32 nCurrFloat = 0; nCurrFloat < COverlayProps::knNumFloatParams; nCurrFloat++)
	{
		//determine the name of this param
		const char* pszParamName = GetProps()->m_FloatParams[nCurrFloat].m_pszMaterialParam;

		//determine if this is a valid parameter
		if(LTStrEmpty(pszParamName))
			continue;

		//calculate the value
		float fValue = GetProps()->m_FloatParams[nCurrFloat].m_ffcParamValue.GetValue(fUnitLifetime);

		//and install it into the material
		if(m_hOverlayMaterial)
			g_pLTClient->GetRenderer()->SetInstanceParamFloat(m_hOverlayMaterial, pszParamName, fValue);
	}
}





