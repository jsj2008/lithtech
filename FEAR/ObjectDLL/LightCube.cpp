// ----------------------------------------------------------------------- //
//
// MODULE  :	LightCube.cpp
//
// PURPOSE :	Provides the implementation for the cube projector light class 
//				which represents cubic environment map point emitter.
//
// CREATED :	2/21/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //
#include "Stdafx.h"
#include "LightCube.h"

LINKFROM_MODULE( LightCube );

BEGIN_CLASS(LightCube)
	ADD_STRINGPROP_FLAG(LightLOD, "Low", PF_STATICLIST, "Indicates at which LOD the light will be visible. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")
	ADD_STRINGPROP_FLAG(WorldShadowsLOD, "Low", PF_STATICLIST, "Indicates at which LOD the light will cast shadows from the world. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")
	ADD_STRINGPROP_FLAG(ObjectShadowsLOD, "Low", PF_STATICLIST, "Indicates at which LOD the light will cast shadows from objects. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_RADIUS | PF_DISTANCE, "Specifies the radius of this light where the intensity falls to zero")
	ADD_COLORPROP(LightColor, 255.0f, 255.0f, 255.0f, "Specifies the color of this light. This is modulated by the texture.")
	ADD_COLORPROP(TranslucentColor, 255.0f, 255.0f, 255.0f, "Specifies the translucent color that modulates the light color when calculating translucent lighting. This should approximate the texture's color")
	ADD_COLORPROP(SpecularColor, 255.0f, 255.0f, 255.0f, "Specifies a specular color that modulates the main light color when rendering specular")
	ADD_REALPROP(IntensityScale, 1.0f, "Specifies an overall intensity of the light ranging from zero to one")
	ADD_STRINGPROP_FLAG(Texture, "", PF_FILENAME, "Specifies a texture that will be projected outwards in all directions")
END_CLASS_FLAGS_PLUGIN(LightCube, LightBase, 0, LightBase_Plugin, "A light that projects a cubic environment map out in all directions")

CMDMGR_BEGIN_REGISTER_CLASS( LightCube )
CMDMGR_END_REGISTER_CLASS( LightCube, LightBase )

LightCube::LightCube()
{
	m_eLightType = eEngineLight_CubeProjector;
}

LightCube::~LightCube()
{
}

//virtual function that derived classes must override to handle loading in of
//property data
void LightCube::ReadLightProperties(const GenericPropList *pProps)
{
	m_fLightRadius = pProps->GetReal("LightRadius", m_fLightRadius);
	m_fIntensityScale = pProps->GetReal("IntensityScale", m_fIntensityScale);

	//read in the texture associated with this light
	m_sLightTexture = pProps->GetString("Texture", "");

	//read in the LOD's
	m_eLightLOD			= CEngineLODPropUtil::StringToLOD(pProps->GetString("LightLOD", ""));
	m_eWorldShadowsLOD	= CEngineLODPropUtil::StringToLOD(pProps->GetString("WorldShadowsLOD", ""));
	m_eObjectShadowsLOD = CEngineLODPropUtil::StringToLOD(pProps->GetString("ObjectShadowsLOD", ""));

	//read in colors and normalize them to be 0..1
	m_vColor			= pProps->GetColor("LightColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
	m_vTranslucentColor = pProps->GetColor("TranslucentColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
	m_vSpecularColor	= pProps->GetColor("SpecularColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
}

