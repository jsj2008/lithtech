// ----------------------------------------------------------------------- //
//
// MODULE  :	LightDirectional.cpp
//
// PURPOSE :	Provides the implementation for the directional light class 
//				which represents an aligned cube of space which has light
//				orthographically projected into it
//
// CREATED :	10/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "LightDirectional.h"
#include "SharedFXStructs.h"

LINKFROM_MODULE( LightDirectional );

BEGIN_CLASS(LightDirectional)
	ADD_STRINGPROP_FLAG(LightLOD, "Low", PF_STATICLIST, "Indicates at which LOD the light will be visible. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")
	ADD_STRINGPROP_FLAG(WorldShadowsLOD, "Low", PF_STATICLIST, "Indicates at which LOD the light will cast shadows from the world. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")
	ADD_STRINGPROP_FLAG(ObjectShadowsLOD, "Low", PF_STATICLIST, "Indicates at which LOD the light will cast shadows from objects. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")
	ADD_VECTORPROP_VAL_FLAG(Dims, 64, 64, 50, PF_ORTHOFRUSTUM, "The dimensions of the area the directional light will project onto")
	ADD_COLORPROP(LightColor, 255.0f, 255.0f, 255.0f, "The color of this light, modulates the texture color")
	ADD_COLORPROP(TranslucentColor, 255.0f, 255.0f, 255.0f, "A value to tint the light color when approximating translucent lighting. This should be approximately the color of the associated texture")
	ADD_COLORPROP(SpecularColor, 255.0f, 255.0f, 255.0f, "A tint to the color value that is applied when rendering specular")
	ADD_REALPROP(IntensityScale, 1.0f, "A scale from zero to one that controls the overall intensity of the light")
	ADD_STRINGPROP_FLAG(Texture, "", PF_FILENAME, "The texture that is projected from this light")
	ADD_STRINGPROP_FLAG(AttenuationTexture, "", PF_FILENAME, "The texture that controls how the light reacts as it moves away from the plane of light emission")
	PROP_DEFINEGROUP(Volumetric, PF_GROUP(1), "Volumetric lighting settings")
		ADD_STRINGPROP_FLAG(VolumetricLOD, "Never", PF_GROUP(1) | PF_STATICLIST, "Indicates the minimum performance level at which the volumetric lighting effect will be visible.")
		ADD_REALPROP_FLAG(VolumetricNoiseIntensity, 0.5f, PF_GROUP(1), "Determines the intensity of the noise effect, from 0 (no noise) to 1 (full noise)")
		ADD_REALPROP_FLAG(VolumetricNoiseScale, 1.0f, PF_GROUP(1), "Determines the size of the noise effect.  Smaller values lead to sharper noise detail.")
		ADD_COLORPROP_FLAG(VolumetricColor, 255.0f, 255.0f, 255.0f, PF_GROUP(1), "Color of the volumetric effect.")
		ADD_STRINGPROP_FLAG(VolumetricTexture, "", PF_GROUP(1) | PF_FILENAME, "Texture to project through the effect.  (aka the Cookie or Gobo texture)")
		ADD_REALPROP_FLAG(VolumetricAttenuation, 1.0f, PF_GROUP(1), "Amount of fading to apply to the volumetric effect based on distance from the light.")
		ADD_BOOLPROP_FLAG(VolumetricAdditive, true, PF_GROUP(1), "When set to True, the volumetric effect will use an additive blend.  Otherwise an alpha blend will be used.")
		ADD_REALPROP_FLAG(VolumetricDepth, 100.0f, PF_GROUP(1), "The approximate depth at which the effect will be at maximum density.")
		ADD_BOOLPROP_FLAG(VolumetricShadow, true, PF_GROUP(1), "When set to True, shadows will be cast in the volume.")
END_CLASS_FLAGS_PLUGIN(LightDirectional, LightBase, 0, LightDirectional_Plugin, "A type of light that projects a texture along the specified direction")

CMDMGR_BEGIN_REGISTER_CLASS( LightDirectional )
CMDMGR_END_REGISTER_CLASS( LightDirectional, LightBase )

LightDirectional::LightDirectional()
{
	m_eLightType = eEngineLight_Directional;
}

LightDirectional::~LightDirectional()
{
}

//virtual function that derived classes must override to handle loading in of
//property data
void LightDirectional::ReadLightProperties(const GenericPropList *pProps)
{
	m_vDirectionalDims = pProps->GetVector("Dims", LTVector(0.0f, 0.0f, 0.0f));
	m_fIntensityScale = pProps->GetReal("IntensityScale", 1.0f);

	//read in the texture associated with this light
	m_sLightTexture = pProps->GetString("Texture", "");
	m_sLightAttenuationTexture = pProps->GetString("AttenuationTexture", "");

	//read in the LOD's
	m_eLightLOD			= CEngineLODPropUtil::StringToLOD(pProps->GetString("LightLOD", ""));
	m_eWorldShadowsLOD	= CEngineLODPropUtil::StringToLOD(pProps->GetString("WorldShadowsLOD", ""));
	m_eObjectShadowsLOD = CEngineLODPropUtil::StringToLOD(pProps->GetString("ObjectShadowsLOD", ""));

	//read in colors and normalize them to be 0..1
	m_vColor			= pProps->GetColor("LightColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
	m_vTranslucentColor = pProps->GetColor("TranslucentColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
	m_vSpecularColor	= pProps->GetColor("SpecularColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
}

//virtual function that derived classes may override to change the creation struct before the object is created
void LightDirectional::PostReadProp(ObjectCreateStruct *pStruct)
{
	//directional lights really need to have their full orientation sent down to the clients
	//to prevent issues with the rotation being quantized and causing overlap issues
	pStruct->m_Flags |= FLAG_FULLPOSITIONRES;
}

uint32 LightDirectional::OnObjectCreated(const GenericPropList* pProps, float fCreationReason)
{
	//let our base class handle the message first
	uint32 nResult = LightBase::OnObjectCreated(pProps, fCreationReason);
	if (nResult == 0)
		return nResult;

	EEngineLOD eLOD	= CEngineLODPropUtil::StringToLOD(pProps->GetString("VolumetricLOD", ""), eEngineLOD_Never);

	//we don't need to continue if we're not going to have a volumetric lighting object
	if (eLOD == eEngineLOD_Never)
		return LT_OK;

	//read in the volumetric lighting options
	VOLUMETRICLIGHTCREATESTRUCT vlcs;
	vlcs.m_eLOD				= eLOD;
	vlcs.m_fNoiseIntensity	= pProps->GetReal("VolumetricNoiseIntensity", vlcs.m_fNoiseIntensity);
	vlcs.m_fNoiseScale		= pProps->GetReal("VolumetricNoiseScale", vlcs.m_fNoiseScale);
	vlcs.m_vColor			= pProps->GetColor("VolumetricColor", vlcs.m_vColor * 255.0f) / 255.0f;
	vlcs.m_sTexture			= pProps->GetString("VolumetricTexture", vlcs.m_sTexture.c_str());
	// Use the light's texture if they don't specify a texture
	if (vlcs.m_sTexture.empty())
		vlcs.m_sTexture = m_sLightTexture;
	vlcs.m_fAttenuation		= pProps->GetReal("VolumetricAttenuation", vlcs.m_fAttenuation);
	vlcs.m_bAdditive		= pProps->GetBool("VolumetricAdditive", vlcs.m_bAdditive);
	vlcs.m_fDepth			= pProps->GetReal("VolumetricDepth", vlcs.m_fDepth);
	vlcs.m_bShadow			= pProps->GetBool("VolumetricShadow", vlcs.m_bShadow);

	//update the sfx message
	CAutoMessage cMsg((uint8)SFX_VOLUMETRICLIGHT_ID);
	vlcs.Write(cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	
	//don't create/destroy the object based on visibility
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE);

	return LT_OK;
}
