// ----------------------------------------------------------------------- //
//
// MODULE  :	LightPoint.cpp
//
// PURPOSE :	Provides the implementation for the point light class which
//				represents a single color point emitter.
//
// CREATED :	2/21/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //
#include "Stdafx.h"
#include "LightPoint.h"

LINKFROM_MODULE( LightPoint );

BEGIN_CLASS(LightPoint)
	ADD_STRINGPROP_FLAG(LightLOD, "Low", PF_STATICLIST, "Indicates at which LOD the light will be visible. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")
	ADD_STRINGPROP_FLAG(WorldShadowsLOD, "Low", PF_STATICLIST, "Indicates at which LOD the light will cast shadows from the world. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")
	ADD_STRINGPROP_FLAG(ObjectShadowsLOD, "Low", PF_STATICLIST, "Indicates at which LOD the light will cast shadows from objects. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_RADIUS | PF_DISTANCE, "The radius of this light that controls the falloff")
	ADD_COLORPROP(LightColor, 255.0f, 255.0f, 255.0f, "The color of the light")
	ADD_COLORPROP(TranslucentColor, 255.0f, 255.0f, 255.0f, "A color used to modulate the light color for translucent light approximation")
	ADD_COLORPROP(SpecularColor, 255.0f, 255.0f, 255.0f, "A color to modulate the light color when rendering specular")
 	ADD_REALPROP(IntensityScale, 1.0f, "A value that controls the overall intensity of this light")
END_CLASS_FLAGS_PLUGIN(LightPoint, LightBase, 0, LightBase_Plugin, "A point light source that emits light equally in all directions")

CMDMGR_BEGIN_REGISTER_CLASS( LightPoint )
CMDMGR_END_REGISTER_CLASS( LightPoint, LightBase )

LightPoint::LightPoint()
{
	m_eLightType = eEngineLight_Point;
}

LightPoint::~LightPoint()
{
}

//virtual function that derived classes must override to handle loading in of
//property data
void LightPoint::ReadLightProperties(const GenericPropList *pProps)
{
	m_fLightRadius = pProps->GetReal("LightRadius", m_fLightRadius);
	m_fIntensityScale = pProps->GetReal("IntensityScale", m_fIntensityScale);

	//read in the LOD's
	m_eLightLOD			= CEngineLODPropUtil::StringToLOD(pProps->GetString("LightLOD", ""));
	m_eWorldShadowsLOD	= CEngineLODPropUtil::StringToLOD(pProps->GetString("WorldShadowsLOD", ""));
	m_eObjectShadowsLOD = CEngineLODPropUtil::StringToLOD(pProps->GetString("ObjectShadowsLOD", ""));

	//read in colors and normalize them to be 0..1
	m_vColor			= pProps->GetColor("LightColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
	m_vTranslucentColor = pProps->GetColor("TranslucentColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
	m_vSpecularColor	= pProps->GetColor("SpecularColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
}

