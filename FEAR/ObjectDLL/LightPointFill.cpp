// ----------------------------------------------------------------------- //
//
// MODULE  :	LightPointFill.cpp
//
// PURPOSE :	Provides the implementation for the point fill light class which
//				represents a single color point emitter that cannot cast shadows
//				or have specular.
//
// CREATED :	2/21/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //
#include "Stdafx.h"
#include "LightPointFill.h"

LINKFROM_MODULE( LightPointFill );

BEGIN_CLASS(LightPointFill)
	ADD_STRINGPROP_FLAG(LightLOD, "Low", PF_STATICLIST, "Indicates at which LOD the light will be visible. For low, it will always be visible, for medium, it will only be visible in medium or higher and so on.")
	ADD_REALPROP_FLAG(LightRadius, 300.0f, PF_RADIUS | PF_DISTANCE, "The radius of this light that controls the falloff")
	ADD_COLORPROP(LightColor, 255.0f, 255.0f, 255.0f, "The color of the light")
	ADD_COLORPROP(TranslucentColor, 255.0f, 255.0f, 255.0f, "A color used to modulate the light color for translucent light approximation")
	ADD_REALPROP(IntensityScale, 1.0f, "A value that controls the overall intensity of this light")
END_CLASS_FLAGS_PLUGIN(LightPointFill, LightBase, 0, LightBase_Plugin, "A fast point light source that emits light equally in all directions but does not allow for shadows or specular")

CMDMGR_BEGIN_REGISTER_CLASS( LightPointFill )
CMDMGR_END_REGISTER_CLASS( LightPointFill, LightBase )

LightPointFill::LightPointFill()
{
	m_eLightType = eEngineLight_PointFill;
}

LightPointFill::~LightPointFill()
{
}

//virtual function that derived classes must override to handle loading in of
//property data
void LightPointFill::ReadLightProperties(const GenericPropList *pProps)
{
	m_fLightRadius		= pProps->GetReal("LightRadius", m_fLightRadius);
	m_fIntensityScale	= pProps->GetReal("IntensityScale", m_fIntensityScale);

	//read in the LOD's
	m_eLightLOD = CEngineLODPropUtil::StringToLOD(pProps->GetString("LightLOD", ""));

	//read in colors and normalize them to be 0..1
	m_vColor			= pProps->GetColor("LightColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
	m_vTranslucentColor = pProps->GetColor("TranslucentColor", LTVector(255.0f, 255.0f, 255.0f)) / 255.0f;
}

