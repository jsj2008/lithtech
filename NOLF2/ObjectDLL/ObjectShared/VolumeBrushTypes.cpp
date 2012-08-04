// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrushTypes.cpp
//
// PURPOSE : VolumeBrushTypes implementation
//
// CREATED : 2/16/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VolumeBrushTypes.h"
#include "Character.h"
#include "SFXMsgIds.h"
#include "SurfaceFunctions.h"

LINKFROM_MODULE( VolumeBrushTypes );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LiquidFilterFn()
//
//	PURPOSE:	Filter Liquid volume brushes out of CastRay and/or
//				IntersectSegment calls (so vectors can go through liquid).
//
// ----------------------------------------------------------------------- //

bool LiquidFilterFn(HOBJECT hObj, void *pUserData)
{
 	return (GetSurfaceType(hObj) != ST_LIQUID);
}

#pragma force_active on
BEGIN_CLASS(Water)
	ADD_REALPROP_FLAG(Viscosity, 0.035f, 0)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
 	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
 	ADD_STRINGPROP_FLAG(SoundFilter, "Underwater", PF_STATICLIST)
	ADD_COLORPROP_FLAG(TintColor, 0.0f, 127.0f, 178.5f, 0)
END_CLASS_DEFAULT_FLAGS(Water, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Water )
CMDMGR_END_REGISTER_CLASS( Water, VolumeBrush )


BEGIN_CLASS(Ice)
	ADD_REALPROP_FLAG(Friction, 0.0f, 0)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
 	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS(Ice, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Ice )
CMDMGR_END_REGISTER_CLASS( Ice, VolumeBrush )


BEGIN_CLASS(CorrosiveFluid)
	ADD_REALPROP_FLAG(Viscosity, 0.035f, 0)
	ADD_REALPROP_FLAG(Damage, 5.0f, 0)
 	ADD_STRINGPROP_FLAG(DamageType, "BURN", PF_STATICLIST | PF_HIDDEN)
 	ADD_STRINGPROP_FLAG(SoundFilter, "Underwater", PF_STATICLIST)
	ADD_COLORPROP_FLAG(TintColor, 76.5f, 102.0f, 0.0f, 0)
END_CLASS_DEFAULT_FLAGS(CorrosiveFluid, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( CorrosiveFluid )
CMDMGR_END_REGISTER_CLASS( CorrosiveFluid, VolumeBrush )


BEGIN_CLASS(FreezingWater)
	ADD_REALPROP_FLAG(Viscosity, 0.035f, 0)
	ADD_REALPROP_FLAG(Damage, 10.0f, 0)
 	ADD_STRINGPROP_FLAG(DamageType, "FREEZE", PF_STATICLIST | PF_HIDDEN)
 	ADD_STRINGPROP_FLAG(SoundFilter, "Underwater", PF_STATICLIST)
	ADD_COLORPROP_FLAG(TintColor, 0.0f, 127.0f, 178.5f, 0)
    ADD_COLORPROP_FLAG(LightAdd, 25.5f, 25.5f, 25.5f, 0)
END_CLASS_DEFAULT_FLAGS(FreezingWater, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( FreezingWater )
CMDMGR_END_REGISTER_CLASS( FreezingWater, VolumeBrush )



BEGIN_CLASS(PoisonGas)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 5.0f, 0)
 	ADD_STRINGPROP_FLAG(DamageType, "POISON", PF_STATICLIST | PF_HIDDEN)
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 76.7f, 0)
END_CLASS_DEFAULT_FLAGS(PoisonGas, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( PoisonGas )
CMDMGR_END_REGISTER_CLASS( PoisonGas, VolumeBrush )


BEGIN_CLASS(Electricity)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 20.0f, 0)
 	ADD_STRINGPROP_FLAG(DamageType, "ELECTROCUTE", PF_STATICLIST | PF_HIDDEN)
    ADD_COLORPROP_FLAG(LightAdd, 25.5f, 25.5f, 25.5f, 0)
END_CLASS_DEFAULT_FLAGS(Electricity, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Electricity )
CMDMGR_END_REGISTER_CLASS( Electricity, VolumeBrush )


BEGIN_CLASS(EndlessFall)
	ADD_REALPROP_FLAG(Damage, 100000000.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DamageType, "ENDLESS FALL", PF_STATICLIST | PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS(EndlessFall, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( EndlessFall )
CMDMGR_END_REGISTER_CLASS( EndlessFall, VolumeBrush )



BEGIN_CLASS(Wind)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
	ADD_REALPROP_FLAG(Friction, 0.0f, 0)
END_CLASS_DEFAULT_FLAGS(Wind, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Wind )
CMDMGR_END_REGISTER_CLASS( Wind, VolumeBrush )



BEGIN_CLASS(ColdAir)
	ADD_REALPROP_FLAG(Damage, 5.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DamageType, "FREEZE", PF_STATICLIST | PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS(ColdAir, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( ColdAir )
CMDMGR_END_REGISTER_CLASS( ColdAir, VolumeBrush )


BEGIN_CLASS( Burn )
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 20.0f, 0)
 	ADD_STRINGPROP_FLAG(DamageType, "BURN", PF_STATICLIST | PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS( Burn, VolumeBrush, NULL, NULL, CF_WORLDMODEL )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Burn )
CMDMGR_END_REGISTER_CLASS( Burn, VolumeBrush )


BEGIN_CLASS(Filter)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
	ADD_REALPROP_FLAG(Friction, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Visible, 0, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	PROP_DEFINEGROUP(FogStuff, PF_GROUP(2) | PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS(Filter, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Filter )
CMDMGR_END_REGISTER_CLASS( Filter, VolumeBrush )



BEGIN_CLASS(SafteyNet)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
	ADD_REALPROP_FLAG(Friction, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Visible, 0, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	PROP_DEFINEGROUP(FogStuff, PF_GROUP(2) | PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS(SafteyNet, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( SafteyNet )
CMDMGR_END_REGISTER_CLASS( SafteyNet, VolumeBrush )



BEGIN_CLASS(Ladder)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.4f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS(Ladder, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Ladder )
CMDMGR_END_REGISTER_CLASS( Ladder, VolumeBrush )


#pragma force_active off


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Ladder::UpdatePhysics()
//
//	PURPOSE:	Update the physics of the passed in object
//
// ----------------------------------------------------------------------- //

void Ladder::UpdatePhysics(ContainerPhysics* pCPStruct)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (IsCharacter(pCPStruct->m_hObject))
	{
		CCharacter* pCharacter = (CCharacter*)pServerDE->HandleToObject(pCPStruct->m_hObject);
		if (pCharacter)
		{
			pCharacter->UpdateOnLadder(this, pCPStruct);
		}
	}
}

#pragma force_active on
BEGIN_CLASS(Weather)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Friction, 1.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
	ADD_REALPROP_FLAG(FogFarZ, 1500.0f, PF_GROUP(2))
	ADD_REALPROP_FLAG(FogNearZ, -500.0f, PF_GROUP(2))
	ADD_COLORPROP_FLAG(FogColor, 192.0f, 192.0f, 192.0f, PF_GROUP(2))
	ADD_REALPROP_FLAG(MaxViewDistance, 1000.0f, 0)
	PROP_DEFINEGROUP(RainProperties, PF_GROUP(3))
        ADD_BOOLPROP_FLAG(LightRain, LTFALSE, PF_GROUP(3))
        ADD_BOOLPROP_FLAG(NormalRain, LTTRUE, PF_GROUP(3))
        ADD_BOOLPROP_FLAG(HeavyRain, LTFALSE, PF_GROUP(3))
	PROP_DEFINEGROUP(SnowProperties, PF_GROUP(4))
        ADD_BOOLPROP_FLAG(LightSnow, LTFALSE, PF_GROUP(4))
        ADD_BOOLPROP_FLAG(NormalSnow, LTTRUE, PF_GROUP(4))
        ADD_BOOLPROP_FLAG(HeavySnow, LTFALSE, PF_GROUP(4))
END_CLASS_DEFAULT_FLAGS(Weather, VolumeBrush, NULL, NULL, CF_WORLDMODEL)

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Weather )
CMDMGR_END_REGISTER_CLASS( Weather, VolumeBrush )

#pragma force_active off

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Weather::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Weather::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	VolumeBrush::ReadProp(pStruct);

	m_dwWeatherFlags = 0;

    bool bVal = LTFALSE;
    g_pLTServer->GetPropBool("LightRain", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_LIGHT_RAIN;

    g_pLTServer->GetPropBool("NormalRain", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_NORMAL_RAIN;

    g_pLTServer->GetPropBool("HeavyRain", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_HEAVY_RAIN;

    g_pLTServer->GetPropBool("LightSnow", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_LIGHT_SNOW;

    g_pLTServer->GetPropBool("NormalSnow", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_NORMAL_SNOW;

    g_pLTServer->GetPropBool("HeavySnow", &bVal);
	if (bVal) m_dwWeatherFlags |= WFLAG_HEAVY_SNOW;

    g_pLTServer->GetPropReal("MaxViewDistance", &m_fViewDist);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Weather::CreateSpecialFXMsg()
//
//	PURPOSE:	Add Weather specific data to the special fx message
//
// ----------------------------------------------------------------------- //

void Weather::WriteSFXMsg(ILTMessage_Write *pMsg)
{
	m_nSfxMsgId = SFX_WEATHER_ID;

	VolumeBrush::WriteSFXMsg(pMsg);

	// Add weather specific data to the special fx message...

    pMsg->Writeuint32(m_dwWeatherFlags);
    pMsg->Writefloat(m_fViewDist);
}


BEGIN_CLASS( Gravity )

	ADD_BOOLPROP(Hidden, LTFALSE)
	ADD_REALPROP_FLAG(Viscosity, 0.0f, 0)
	ADD_REALPROP_FLAG(Friction, 1.0f, 0)
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN)
 	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN)
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN)
    ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
 	ADD_STRINGPROP_FLAG(SoundFilter, "Dynamic", PF_STATICLIST | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanPlayMovementSounds, LTTRUE, PF_HIDDEN)

	PROP_DEFINEGROUP(FogStuff, PF_GROUP(2) | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PhysicsModel, "", PF_STATICLIST)

	// New prop

	ADD_REALPROP_FLAG( PlayerGravity, -2000.0f, 0 )

END_CLASS_DEFAULT_FLAGS( Gravity, VolumeBrush, NULL, NULL, CF_WORLDMODEL )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Gravity )
CMDMGR_END_REGISTER_CLASS( Gravity, VolumeBrush )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Gravity::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Gravity::ReadProp( ObjectCreateStruct *pStruct )
{
	if( !pStruct ) return;

	VolumeBrush::ReadProp( pStruct );

	GenericProp gProp;
	if( g_pLTServer->GetPropGeneric( "PlayerGravity", &gProp ) == LT_OK )
	{
		m_fGravity = gProp.m_Float;
	}
}
