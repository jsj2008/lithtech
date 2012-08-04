// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrushTypes.cpp
//
// PURPOSE : VolumeBrushTypes implementation
//
// CREATED : 2/16/98
//
// ----------------------------------------------------------------------- //

#include "VolumeBrushTypes.h"
#include "BaseCharacter.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LiquidFilterFn()
//
//	PURPOSE:	Filter Liquid volume brushes out of CastRay and/or 
//				IntersectSegment calls (so vectors can go through liquid).
//
//	NOTE:		For now the assumption is if this function is called (via 
//				CastRay or IntersectSegment), FLAG_RAYHIT must be set on the 
//				object (if it is a volume brush).  So, only liquid volume
//				brushes have this flag set on them, so we can conclude that if 
//				this object is of type volume brush that it must be liquid...
//
// ----------------------------------------------------------------------- //

DBOOL LiquidFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pServerDE) return DFALSE;

	HCLASS hVolumeBrush = g_pServerDE->GetClass("VolumeBrush");
	HCLASS hObjClass    = g_pServerDE->GetObjectClass(hObj);

	// Return DTRUE to keep this object (not liquid), or DFALSE to ignore
	// this object (is liquid)...

	return !g_pServerDE->IsKindOf(hObjClass, hVolumeBrush);
}


BEGIN_CLASS(BlueWater)
    ADD_STRINGPROP(SpriteSurfaceName, "Sprites\\VolumeBrushSprites\\Water2.spr")   
    ADD_COLORPROP(SurfaceColor1, 255.0f, 255.0f, 255.0f)   
    ADD_COLORPROP(SurfaceColor2, 255.0f, 255.0f, 255.0f)   
	ADD_REALPROP_FLAG(Viscosity, 0.5f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
END_CLASS_DEFAULT(BlueWater, VolumeBrush, NULL, NULL)

BEGIN_CLASS(DirtyWater)
    ADD_STRINGPROP(SpriteSurfaceName, "Sprites\\VolumeBrushSprites\\Water1.spr")   
    ADD_COLORPROP(SurfaceColor1, 255.0f, 255.0f, 255.0f)   
    ADD_COLORPROP(SurfaceColor2, 255.0f, 255.0f, 255.0f)   
	ADD_REALPROP_FLAG(Viscosity, 0.5f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
END_CLASS_DEFAULT(DirtyWater, VolumeBrush, NULL, NULL)

BEGIN_CLASS(ClearWater)
    ADD_STRINGPROP(SpriteSurfaceName, "Sprites\\VolumeBrushSprites\\Water3.spr")   
    ADD_COLORPROP(SurfaceColor1, 255.0f, 255.0f, 255.0f)   
    ADD_COLORPROP(SurfaceColor2, 255.0f, 255.0f, 255.0f)   
	ADD_REALPROP_FLAG(Viscosity, 0.5f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
END_CLASS_DEFAULT(ClearWater, VolumeBrush, NULL, NULL)

BEGIN_CLASS(CorrosiveFluid)
    ADD_STRINGPROP(SpriteSurfaceName, "Sprites\\VolumeBrushSprites\\Water2.spr")   
    ADD_COLORPROP(SurfaceColor1, 255.0f, 255.0f, 255.0f)   
    ADD_COLORPROP(SurfaceColor2, 255.0f, 255.0f, 255.0f)   
	ADD_REALPROP_FLAG(Viscosity, 0.5f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 5.0f, 0)
	ADD_LONGINTPROP_FLAG(DamageType, DT_BURN, PF_HIDDEN)
END_CLASS_DEFAULT(CorrosiveFluid, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Kato)
    ADD_STRINGPROP(SpriteSurfaceName, "Sprites\\VolumeBrushSprites\\Lava1.spr")   
    ADD_COLORPROP(SurfaceColor1, 255.0f, 255.0f, 255.0f)   
    ADD_COLORPROP(SurfaceColor2, 255.0f, 255.0f, 255.0f)   
	ADD_REALPROP_FLAG(Viscosity, 0.5f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 5.0f, 0)
	ADD_LONGINTPROP_FLAG(DamageType, DT_BURN, PF_HIDDEN)
END_CLASS_DEFAULT(Kato, VolumeBrush, NULL, NULL)

BEGIN_CLASS(LiquidNitrogen)
    ADD_STRINGPROP(SpriteSurfaceName, "Sprites\\VolumeBrushSprites\\Water2.spr")   
    ADD_COLORPROP(SurfaceColor1, 255.0f, 255.0f, 255.0f)   
    ADD_COLORPROP(SurfaceColor2, 255.0f, 255.0f, 255.0f)   
	ADD_REALPROP_FLAG(Viscosity, 0.5f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 10.0f, 0)
	ADD_LONGINTPROP_FLAG(DamageType, DT_FREEZE, PF_HIDDEN)
END_CLASS_DEFAULT(LiquidNitrogen, VolumeBrush, NULL, NULL)

BEGIN_CLASS(PoisonGas)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 5.0f, 0)
	ADD_LONGINTPROP_FLAG(DamageType, DT_CHOKE, PF_HIDDEN)
END_CLASS_DEFAULT(PoisonGas, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Smoke)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.5f, 0)
	ADD_LONGINTPROP_FLAG(DamageType, DT_CHOKE, PF_HIDDEN)
END_CLASS_DEFAULT(Smoke, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Electricity)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 20.0f, 0)
	ADD_LONGINTPROP_FLAG(DamageType, DT_ELECTROCUTE, PF_HIDDEN)
END_CLASS_DEFAULT(Electricity, VolumeBrush, NULL, NULL)

BEGIN_CLASS(EndlessFall)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 100000000.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_ENDLESS_FALL, PF_HIDDEN)
END_CLASS_DEFAULT(EndlessFall, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Wind)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
END_CLASS_DEFAULT(Wind, VolumeBrush, NULL, NULL)

BEGIN_CLASS(ZeroGravity)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
END_CLASS_DEFAULT(ZeroGravity, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Vacuum)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 10.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_EXPLODE, PF_HIDDEN)
END_CLASS_DEFAULT(Vacuum, VolumeBrush, NULL, NULL)

BEGIN_CLASS(Ladder)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
	ADD_BOOLPROP_FLAG(FogEnable, DFALSE, PF_HIDDEN)
    ADD_REALPROP_FLAG(FogFarZ, 300.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(FogNearZ,-100.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(FogColor, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
END_CLASS_DEFAULT(Ladder, VolumeBrush, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ladder::UpdatePhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

void Ladder::UpdatePhysics(ContainerPhysics* pCPStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (pServerDE->IsKindOf(pServerDE->GetObjectClass(pCPStruct->m_hObject), m_hPlayerClass))
	{
		CBaseCharacter* pCharacter = (CBaseCharacter*)pServerDE->HandleToObject(pCPStruct->m_hObject);
		if (pCharacter)
		{
			pCharacter->UpdateOnLadder(this, pCPStruct);
		}
	}
}

BEGIN_CLASS(TotalRed)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
END_CLASS_DEFAULT(TotalRed, VolumeBrush, NULL, NULL)

BEGIN_CLASS(TintScreen)
	ADD_BOOLPROP_FLAG(ShowSurface, DFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceHeight, 0.0f, PF_HIDDEN)
    ADD_STRINGPROP_FLAG(SpriteSurfaceName, "", PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor1, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_COLORPROP_FLAG(SurfaceColor2, 0.0f, 0.0f, 0.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMin, 15.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleMax, 25.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(XScaleDuration, 10.0f, PF_HIDDEN)   
    ADD_REALPROP_FLAG(YScaleDuration, 10.0f, PF_HIDDEN)   
	ADD_REALPROP_FLAG(SurfaceHeight, 5.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(SurfaceAlpha, 0.7f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumSurfacePolies, 160, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(DamageType, DT_UNSPECIFIED, PF_HIDDEN)
    ADD_COLORPROP_FLAG(Color, 255.0f, 255.0f, 255.0f, 0)   
END_CLASS_DEFAULT(TintScreen, VolumeBrush, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TintScreen::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD TintScreen::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if ( fData == PRECREATE_WORLDFILE )
			{
				pServerDE->GetPropVector("Color", &m_vColor);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				// Pack our color into the upper 3 bytes of the user data...

				DBYTE r = (DBYTE)m_vColor.x;
				DBYTE g = (DBYTE)m_vColor.y;
				DBYTE b = (DBYTE)m_vColor.z;

				DDWORD dwData = pServerDE->GetObjectUserFlags(m_hObject);
				dwData = ((r<<24) | (g<<16) | (b<<8) | (dwData & 0x000F));
				pServerDE->SetObjectUserFlags(m_hObject, dwData);
			}
			break;
		}

		default : break;
	}

	return VolumeBrush::EngineMessageFn(messageID, pData, fData);
}
