//------------------------------------------------------------------
//
//   MODULE  : LTBMODELFX.CPP
//
//   PURPOSE : Implements class CLTBModelFX
//
//   CREATED : On 12/3/98 At 6:34:44 PM
//
//------------------------------------------------------------------
#include "stdafx.h"
#include "LTBModelFX.h"
#include "ClientFX.h"
#include "resourceextensions.h"
#include "iperformancemonitor.h"
#include "GameRenderLayers.h"


//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXModel("ClientFX_Model", "ClientFX");

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLTBModelProps::CLTBModelProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CLTBModelProps::CLTBModelProps() : 
	m_eShadowLOD			( eEngineLOD_Low ),
	m_eInSky				( eFXSkySetting_None ),
	m_bOverrideAniLength	( false ),
	m_fAniLength			( 0.0f ),
	m_bSyncToKey			( false ),
	m_bRotate				( false ),
	m_bForceTranslucent		( false ),
	m_bTranslucentLight		( true ),
	m_bPlayerView			( false ),
	m_pszModelName			( NULL ),
	m_pszAnimName			( NULL ),
	m_pszAttachEffect		( NULL )
{
	for(uint32 nCurrMaterial = 0; nCurrMaterial < knNumModelMaterials; nCurrMaterial++)
	{
		m_pszMaterialName[nCurrMaterial] = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLTBModelProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CLTBModelProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	for(uint32 nCurrMaterial = 0; nCurrMaterial < knNumModelMaterials; nCurrMaterial++)
	{
		char szPropBuffer[32];
		LTSNPrintF(szPropBuffer, LTARRAYSIZE(szPropBuffer), "Material%d", nCurrMaterial);

		if(LTStrIEquals(pszName, szPropBuffer))
		{
			m_pszMaterialName[nCurrMaterial] = CFxProp_String::Load(pStream, pszStringTable);
			return true;
		}
	}

	if( LTStrIEquals( pszName, "Model" ))
	{
		m_pszModelName = CFxProp_String::Load(pStream, pszStringTable );
	}
	else if( LTStrIEquals( pszName, "ShadowLOD" ))
	{
		m_eShadowLOD = (EEngineLOD)CFxProp_Enum::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "PlayerView" ) )
	{
		m_bPlayerView = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "InSky" ))
	{
		m_eInSky = (EFXSkySetting)CFxProp_Enum::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "AttachEffect" ) )
	{
		m_pszAttachEffect = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if( LTStrIEquals( pszName, "OverrideAniLength" ))
	{
		m_bOverrideAniLength = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "AniName" ))
	{
		m_pszAnimName = CFxProp_String::Load(pStream, pszStringTable);
	}
	else if( LTStrIEquals( pszName, "AniLength" ))
	{
		m_fAniLength = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "SyncToKey" ) )
	{
		m_bSyncToKey = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "ForceTranslucent" ) )
	{
		m_bForceTranslucent = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "TranslucentLight" ) )
	{
		m_bTranslucentLight = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "ModelColor" ) )
	{
		m_cfcModelColor.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "ModelScale" ) )
	{
		m_ffcModelScale.Load(pStream, pCurveData);
	}
	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CLTBModelProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectResource(m_pszModelName);
	for(uint32 nCurrMaterial = 0; nCurrMaterial < knNumModelMaterials; nCurrMaterial++)
	{
		Collector.CollectResource(m_pszMaterialName[nCurrMaterial]);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetModelFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetLTBModelProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;
	LTVector vZero(0, 0, 0);

	// Add the base props

	AddBaseProps(pList);

	// Add all the props to the list

	//------------------------------------------------------------
	fxProp.SetupTextLine("Model Files");
	pList->AddTail(fxProp);

	fxProp.SetupPath( "Model", "", "Model Files (*." RESEXT_MODEL_PACKED ")|*." RESEXT_MODEL_PACKED "|All Files (*.*)|*.*||", eCurve_None, "Specifies the compiled model that will be rendered" );
	pList->AddTail(fxProp);

	//add all the material properties
	for(uint32 nCurrMaterial = 0; nCurrMaterial < CLTBModelProps::knNumModelMaterials; nCurrMaterial++)
	{
		char szPropBuffer[32];
		LTSNPrintF(szPropBuffer, LTARRAYSIZE(szPropBuffer), "Material%d", nCurrMaterial);

		fxProp.SetupPath( szPropBuffer, "", "Material Files (*." RESEXT_MATERIAL ")|*." RESEXT_MATERIAL "|All Files (*.*)|*.*||", eCurve_None, "Specifies the material to use for material slot 0" );
		pList->AddTail(fxProp);
	}

	//------------------------------------------------------------
	fxProp.SetupTextLine("Model Appearance");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool( "ForceTranslucent", false, eCurve_None, "If set to true, this will be treated as if it is translucent always, regardless of what the alpha is set to, this should be used with any models using translucent shaders." );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "TranslucentLight", true, eCurve_None, "For translucent objects, this determines if lighting should be approximated or if it should be fullbright" );
	pList->AddTail( fxProp );

	fxProp.SetupEnum( "ShadowLOD", "Low", "Low,Medium,High,Never", eCurve_None, "Determines at which LOD levels this model's shadow should be visible" );
	pList->AddTail( fxProp );

	fxProp.SetupColor( "ModelColor", 0xFFFFFFFF, eCurve_Linear, "Determines the color of the model over time" );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin("ModelScale", 1.0f, 0.0f, eCurve_Linear, "Determines the scale of the model over time");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "PlayerView", false, eCurve_None, "Determines if the sprite should be rendered in the player view, which means that it should have its Z values adjusted to not be clipped into nearby walls");
	pList->AddTail( fxProp );	

	fxProp.SetupEnum( "InSky", SKY_PROP_DEFAULT, SKY_PROP_ENUM, eCurve_None, SKY_PROP_DESCRIPTION);
	pList->AddTail( fxProp );

	fxProp.SetupClientFXRef( "AttachEffect", "", eCurve_None, "This is a semicolon delimited listing of effects that should be created and attached to this model");
	pList->AddTail( fxProp );

	//------------------------------------------------------------
	fxProp.SetupTextLine("Model Animation");
	pList->AddTail(fxProp);

	fxProp.SetupAnimation( "AniName", "", eCurve_None, "Allows the specification of a specific animation to play in the model");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "OverrideAniLength", false, eCurve_None, "If true, this will use the specified animation length and scale the actual animation length to that value" );
	pList->AddTail( fxProp );

	fxProp.SetupFloat( "AniLength", 0, eCurve_None, "If OverrideAniLength is enabled, this specifies how long the animation is to be" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "SyncToKey", false, eCurve_None, "Indicates that the model animation will be synchronized to fit the duration of this key" );
	pList->AddTail( fxProp );

}

//------------------------------------------------------------------
//
//   FUNCTION : CLTBModelFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CLTBModelFX::CLTBModelFX() :
	CBaseFX(CBaseFX::eLTBModelFX),
	m_fAniRate(0.0f),
	m_hObject(NULL)
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CLTBModelFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CLTBModelFX::~CLTBModelFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CLTBModelFX
//
//------------------------------------------------------------------

bool CLTBModelFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXModel);

	// Perform base class initialisation

	if (!CBaseFX::Init(pBaseData, pProps)) 
		return false;

	ObjectCreateStruct ocs;
	ocs.m_ObjectType		= OT_MODEL;

	GetCurrentTransform(0.0f, ocs.m_Pos, ocs.m_Rotation);

	//determine the flags for this object
	if(GetProps()->m_bForceTranslucent)
		ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	//setup the translucent lighting
	if(!GetProps()->m_bTranslucentLight)
		ocs.m_Flags |= FLAG_NOLIGHT;

	//setup whether or not it is in the sky
	ocs.m_Flags2 |= GetSkyFlags(GetProps()->m_eInSky);

	ocs.SetFileName(GetProps()->m_pszModelName);

	for(uint32 nCurrMaterial = 0; nCurrMaterial < CLTBModelProps::knNumModelMaterials; nCurrMaterial++)
	{
		ocs.SetMaterial(nCurrMaterial, GetProps()->m_pszMaterialName[nCurrMaterial]);
	}

	m_hObject = g_pLTClient->CreateObject(&ocs);
	if( !m_hObject ) 
		return false;

	//setup our rendering layer
	if(GetProps()->m_bPlayerView)
		g_pLTClient->GetRenderer()->SetObjectDepthBiasTableIndex(m_hObject, eRenderLayer_Player);

	//setup the timer for this object so it matches that of our FX manager
	g_pLTClient->Timer()->SetObjectTimer(m_hObject, m_pFxMgr->GetTimer());

	//setup the shadow LOD for this model as was specified by the artist
	g_pLTClient->SetObjectShadowLOD(m_hObject, GetProps()->m_eShadowLOD);

	ILTModel *pLTModel = g_pLTClient->GetModelLT();
	
	//setup the animation if the user specified one
	if(!LTStrEmpty(GetProps()->m_pszAnimName))
	{
		//ok, we need to set this
		HMODELANIM hAnim = g_pLTClient->GetAnimIndex(m_hObject, GetProps()->m_pszAnimName);

		if(hAnim != INVALID_MODEL_ANIM)
		{
			//ok, lets set this animation
			pLTModel->SetCurAnim(m_hObject, MAIN_TRACKER, hAnim, false);
		}
	}
	//disable looping on this animation (so we can actually stop!)
	pLTModel->SetLooping(m_hObject, MAIN_TRACKER, false);

	// Setup the initial data needed to override the models animation length...
	if( GetProps()->m_bOverrideAniLength )
	{
		uint32 nAnimLength;

		pLTModel->GetCurAnimLength( m_hObject, MAIN_TRACKER, nAnimLength );
		pLTModel->SetCurAnimTime( m_hObject, MAIN_TRACKER, 0 );

		float fAniLength = (GetProps()->m_fAniLength < MATH_EPSILON) ? GetLifetime() : GetProps()->m_fAniLength;
		
		if( fAniLength >= MATH_EPSILON || fAniLength <= -MATH_EPSILON )
			m_fAniRate = (nAnimLength * 0.001f) / fAniLength;

		pLTModel->SetAnimRate( m_hObject, MAIN_TRACKER, m_fAniRate );
	}

	//create our attached effects
	if(!LTStrEmpty(GetProps()->m_pszAttachEffect))
	{
		CLIENTFX_CREATESTRUCT CreateStruct("", pBaseData->m_dwFlags, m_hObject, LTRigidTransform::GetIdentity());
		CreateNewFX(m_pFxMgr, GetProps()->m_pszAttachEffect, CreateStruct, true);
	}

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CLTBModelFX
//
//------------------------------------------------------------------

bool CLTBModelFX::IsFinishedShuttingDown()
{
	//if we are syncing to the model key, we are always done
	if(GetProps()->m_bSyncToKey)
	{
		return true;
	}

	//otherwise just ask the animation if it has completed
	uint32 dwState = 0;
	g_pLTClient->GetModelLT()->GetPlaybackState(m_hObject, MAIN_TRACKER, dwState);

	return (bool)(!!(dwState & MS_PLAYDONE));
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CLTBModelFX
//
//------------------------------------------------------------------

void CLTBModelFX::Term()
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
//   PURPOSE  : Updates class CLTBModelFX
//
//------------------------------------------------------------------

bool CLTBModelFX::Update(float tmFrameTime)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXModel);

	// Base class update first	
	BaseUpdate(tmFrameTime);

	//update our model position
	LTRigidTransform tObjTrans;
	GetCurrentTransform(GetUnitLifetime(), tObjTrans.m_vPos, tObjTrans.m_rRot);
	g_pLTClient->SetObjectTransform(m_hObject, tObjTrans);

	if(GetProps()->m_bSyncToKey)
	{
		//we need to find out where in the key we currently are
		ILTModel		*pLTModel = g_pLTClient->GetModelLT();
		
		//we have the main tracker, see where in its timeline it is
		uint32 nAnimLength;
		g_pLTClient->GetModelLT()->ResetAnim( m_hObject, MAIN_TRACKER );
		pLTModel->GetCurAnimLength(m_hObject, MAIN_TRACKER, nAnimLength);

		if(nAnimLength > 0)
			nAnimLength--;

		float tmWrappedTime = fmodf(GetUnitLifetime(), 1.0f);
		uint32 nAnimTime = (uint32)(tmWrappedTime * nAnimLength);
		pLTModel->SetCurAnimTime(m_hObject, MAIN_TRACKER, nAnimTime);
	}

	//see if we should reset our model animation
	if(!GetProps()->m_bSyncToKey && !IsShuttingDown() && IsFinishedShuttingDown())
	{
		//Reset the animation
		g_pLTClient->GetModelLT()->ResetAnim( m_hObject, MAIN_TRACKER );

		if(GetProps()->m_bOverrideAniLength)
			g_pLTClient->GetModelLT()->SetAnimRate( m_hObject, MAIN_TRACKER, m_fAniRate);
	}

	//update the object color and scale
	float fUnitLifetime = GetUnitLifetime();

	//update the color
	LTVector4 vColor = GetProps()->m_cfcModelColor.GetValue(fUnitLifetime);
	g_pLTClient->SetObjectColor(m_hObject, vColor.x, vColor.y, vColor.z, vColor.w);

	//update the scale
	float fScale = GetProps()->m_ffcModelScale.GetValue(fUnitLifetime);
	g_pLTClient->SetObjectScale(m_hObject, fScale);

	return true;
}

//called to enumerate through each of the objects and will call into the provided function for each
void CLTBModelFX::EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData)
{
	if(m_hObject)
	{
		pfnObjectCB(this, m_hObject, pUserData);
	}
}


