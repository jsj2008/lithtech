// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrushTypes.cpp
//
// PURPOSE : VolumeBrushTypes implementation
//
// CREATED : 2/16/98
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "VolumeBrushTypes.h"
#include "Character.h"
#include "SFXMsgIds.h"
#include "SurfaceFunctions.h"
#include "SurfaceFlagsOverrideHelpers.h"

LINKFROM_MODULE( VolumeBrushTypes );

//macro that will override all the properties for the force volumes and hide them
#define HIDE_FORCE_VOLUME_PROPS		\
	PROP_DEFINEGROUP(ForceVolumeProps, PF_HIDDEN | PF_GROUP(3), "") \
		ADD_BOOLPROP_FLAG(ForceVolume, false, PF_HIDDEN | PF_GROUP(3), "") \
		ADD_ROTATIONPROP_FLAG(ForceDir, PF_HIDDEN | PF_GROUP(3) | PF_RELATIVEORIENTATION, "") \
		ADD_REALPROP_FLAG(ForceMag, 0.0f, PF_HIDDEN | PF_GROUP(3), "") \
		ADD_REALPROP_FLAG(ForceWaveFreq, 1.0f, PF_HIDDEN | PF_GROUP(3), "") \
		ADD_REALPROP_FLAG(ForceWaveAmp, 0.0f, PF_HIDDEN | PF_GROUP(3), "") \
		ADD_REALPROP_FLAG(ForceWaveBaseOffset, 1.0f, PF_HIDDEN | PF_GROUP(3), "") \
		ADD_REALPROP_FLAG(Density, 0.0f, PF_HIDDEN | PF_GROUP(3), "") 


BEGIN_CLASS(Water)
	ADD_REALPROP_FLAG(Resistance, 0.08f, 0, "This value affects the resistance of the players movement through the volume.")
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN, "Sets amount of damage dealt by this area for every second the player is in it.")
 	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN, "This is a dropdown menu that allows you to choose the damage type that will be inflicted by the Volume.")
 	ADD_STRINGPROP_FLAG(SoundFilter, "Underwater", PF_STATICLIST, "You could have sound behave differently inside the volume. In the case of liquid, you could mute out high frequencies. In the case of a dimensional gate, you might add a strange echo or other filter.")
	ADD_COLORPROP_FLAG(TintColor, 0.0f, 127.0f, 178.5f, 0, "You can define a tint for the player's view if you want the area to look as if it's filled with light or liquid.")
	ADD_BOOLPROP_FLAG( AllowSwimming, true, 0, "If true, players will be allowed to swim in the volume.  If false, players will not play their swimming animations." )
	ADD_STRINGPROP_FLAG(SurfaceOverride, "Liquid", PF_STATICLIST, "This dropdown menu allows you to choose a specific surface type for the VolumeBrush.  NOTE:  If the Unknown surface type is used, the surface type of the material applied to the brush will be used.")
END_CLASS_FLAGS(Water, VolumeBrush, CF_WORLDMODEL, "Water volumes are used to define a space with movement and physics properties that behave like water." )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Water )
	ADD_MESSAGE( ALLOWSWIMMING, 2, NULL, MSG_HANDLER( Water, HandleAllowSwimmingMsg ), "ALLOWSWIMMING <1 or 0>", "Enables or disables the player swimming animation for the water volume", "msg Water (ALLOWSWIMMING 1)" )
CMDMGR_END_REGISTER_CLASS( Water, VolumeBrush )

void Water::HandleAllowSwimmingMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_0( "0" );

	m_bAllowSwimming = !(crParsedMsg.GetArg( 1 ) == s_cTok_0);

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( m_nSfxMsgId );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits( kVolumeBrush_AllowSwimming, FNumBitsExclusive<kVolumeBrush_NumMsgs>::k_nValue );
	cMsg.Writebool( m_bAllowSwimming );
	g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );

	CreateSpecialFXMsg( );
}

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_CORROSIVEFLUID CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	// JSC this has to be set to a value or else the '|' operator will be 
	//  missing an operand.
	#define CF_HIDDEN_CORROSIVEFLUID 0

#endif

BEGIN_CLASS(CorrosiveFluid)
	ADD_REALPROP_FLAG(Resistance, 0.08f, 0, "This value affects the resistance of the players movement through the volume.")
	ADD_REALPROP_FLAG(Damage, 5.0f, 0, "Sets amount of damage dealt by this area for every second the player is in it.")
 	ADD_STRINGPROP_FLAG(DamageType, "BURN", PF_STATICLIST | PF_HIDDEN, "This is a dropdown menu that allows you to choose the damage type that will be inflicted by the Volume.")
 	ADD_STRINGPROP_FLAG(SoundFilter, "Underwater", PF_STATICLIST, "You could have sound behave differently inside the volume. In the case of liquid, you could mute out high frequencies. In the case of a dimensional gate, you might add a strange echo or other filter.")
	ADD_COLORPROP_FLAG(TintColor, 76.5f, 102.0f, 0.0f, 0, "You can define a tint for the player's view if you want the area to look as if it's filled with light or liquid.")
	ADD_BOOLPROP_FLAG( AllowSwimming, true, 0, "If true, players will be allowed to swim in the volume.  If false, players will not play their swimming animations." )
	ADD_STRINGPROP_FLAG(SurfaceOverride, "Liquid", PF_STATICLIST, "This dropdown menu allows you to choose a specific surface type for the VolumeBrush.  NOTE:  If the Unknown surface type is used, the surface type of the material applied to the brush will be used.")
END_CLASS_FLAGS(CorrosiveFluid, VolumeBrush, CF_HIDDEN_CORROSIVEFLUID|CF_WORLDMODEL, "CorrosiveFluid volumes are used to define a space with movement and physics properties that behave like water and damage the player while they are within the space." )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( CorrosiveFluid )
CMDMGR_END_REGISTER_CLASS( CorrosiveFluid, VolumeBrush )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CorrosiveFluid::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void CorrosiveFluid::ReadProp(const GenericPropList *pProps)
{
	if (!pProps) return;

	VolumeBrush::ReadProp(pProps);
	
	// Make sure we are using the right damage type.
	m_eDamageType = StringToDamageType( "Burn" );
}



BEGIN_CLASS(EndlessFall)
	ADD_REALPROP_FLAG(Damage, 100000000.0f, PF_HIDDEN, "Sets amount of damage dealt by this area for every second the player is in it.")
	ADD_STRINGPROP_FLAG(DamageType, "ENDLESS FALL", PF_STATICLIST | PF_HIDDEN, "This is a dropdown menu that allows you to choose the damage type that will be inflicted by the Volume.")
	HIDE_FORCE_VOLUME_PROPS
END_CLASS_FLAGS(EndlessFall, VolumeBrush, CF_WORLDMODEL, "EndlessFall volumes are used as instant death volumes where the player recieves endless fall damage." )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( EndlessFall )
CMDMGR_END_REGISTER_CLASS( EndlessFall, VolumeBrush )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EndlessFall::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void EndlessFall::ReadProp(const GenericPropList *pProps)
{
	if (!pProps) return;

	VolumeBrush::ReadProp(pProps);
	
	// Make sure we are using the right damage type.
	m_eDamageType = StringToDamageType( "ENDLESS FALL" );
}



BEGIN_CLASS( Burn )
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN, "This is a force that pushes against the player. Use positive or negative values along each axis (x, y, and z) to indicate the direction and strength of the current. In general, you'll need values in the thousands in order to lift a player off the ground. This works well to create moving streams or areas of rushing wind.")
	ADD_REALPROP_FLAG(Damage, 20.0f, 0, "Sets amount of damage dealt by this area for every second the player is in it.")
 	ADD_STRINGPROP_FLAG(DamageType, "BURN", PF_STATICLIST | PF_HIDDEN, "This is a dropdown menu that allows you to choose the damage type that will be inflicted by the Volume.")
	HIDE_FORCE_VOLUME_PROPS
END_CLASS_FLAGS( Burn, VolumeBrush, CF_WORLDMODEL, "Burn volumes define a space where the player recieves Burn damage while within the space." )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Burn )
CMDMGR_END_REGISTER_CLASS( Burn, VolumeBrush )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Burn::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Burn::ReadProp(const GenericPropList *pProps)
{
	if (!pProps) return;

	VolumeBrush::ReadProp(pProps);
	
	// Make sure we are using the right damage type.
	m_eDamageType = StringToDamageType( "BURN" );
}


BEGIN_CLASS( Electricity )
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN, "This is a force that pushes against the player. Use positive or negative values along each axis (x, y, and z) to indicate the direction and strength of the current. In general, you'll need values in the thousands in order to lift a player off the ground. This works well to create moving streams or areas of rushing wind.")
	ADD_REALPROP_FLAG(Damage, 20.0f, 0, "Sets amount of damage dealt by this area for every second the player is in it.")
	ADD_STRINGPROP_FLAG(DamageType, "ELECTRICITY", PF_STATICLIST | PF_HIDDEN, "This is a dropdown menu that allows you to choose the damage type that will be inflicted by the Volume.")
	HIDE_FORCE_VOLUME_PROPS
END_CLASS_FLAGS( Electricity, VolumeBrush, CF_WORLDMODEL, "Electricity volumes define a space where the player recieves Electricity damage while within the space." )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Electricity )
CMDMGR_END_REGISTER_CLASS( Electricity, VolumeBrush )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Burn::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Electricity::ReadProp(const GenericPropList *pProps)
{
	if (!pProps) return;

	VolumeBrush::ReadProp(pProps);

	// Make sure we are using the right damage type.
	m_eDamageType = StringToDamageType( "ELECTRICITY" );
}


BEGIN_CLASS(Filter)
	ADD_REALPROP_FLAG(Visible, 0, PF_HIDDEN, "This flag allows you to make a volume begin hidden in the level. Later you can send a (hidden 0) message to the VolumeBrush object to unhide it.")
    ADD_BOOLPROP_FLAG(Hidden, false, PF_HIDDEN, "This flag allows you to make a volume begin hidden in the level. Later you can send a (hidden 0) message to the VolumeBrush object to unhide it.")
	ADD_REALPROP_FLAG(Resistance, 0.0f, PF_HIDDEN, "This value affects the resistance of the players movement through the volume.")
	ADD_REALPROP_FLAG(Friction, 1.0f, PF_HIDDEN, "This property allows you to alter the friction coefficient of surfaces to create slippery surfaces.")
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN, "This is a force that pushes against the player. Use positive or negative values along each axis (x, y, and z) to indicate the direction and strength of the current. In general, you'll need values in the thousands in order to lift a player off the ground. This works well to create moving streams or areas of rushing wind.")
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN, "Sets amount of damage dealt by this area for every second the player is in it.")
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN, "This is a dropdown menu that allows you to choose the damage type that will be inflicted by the Volume.")
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN, "You can define a tint for the player's view if you want the area to look as if it's filled with light or liquid.")
	ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, PF_HIDDEN, "This allows you to further alter the view from inside a Volume.")
 	ADD_STRINGPROP_FLAG(SoundFilter, "Dynamic", PF_STATICLIST, "You could have sound behave differently inside the volume. In the case of liquid, you could mute out high frequencies. In the case of a dimensional gate, you might add a strange echo or other filter.")
    ADD_BOOLPROP(CanPlayMovementSounds, true, "This flag disables the footstep sounds and head-bob motion in a volume.")

	PROP_DEFINEGROUP(FogStuff, PF_GROUP(2) | PF_HIDDEN, "This is a subset of properties that define the fog effects that will be seen when the player is within the Volume Brush.")
        ADD_BOOLPROP_FLAG(FogEnable, false, PF_GROUP(2) | PF_HIDDEN, "This flag toggles the fog effects on and off.")
		ADD_REALPROP_FLAG(FogFarZ, 300.0f, PF_GROUP(2) | PF_HIDDEN, "This is a value that determines at what distance the fog reaches full saturation.")
		ADD_REALPROP_FLAG(FogNearZ, -100.0f, PF_GROUP(2) | PF_HIDDEN, "This value determines at what distance the fog is at zero saturation.")
		ADD_COLORPROP_FLAG(FogColor, 0.0f, 0.0f, 0.0f, PF_GROUP(2) | PF_HIDDEN, "This color picker determines the color of the fog effect.")

	HIDE_FORCE_VOLUME_PROPS

	ADD_STRINGPROP_FLAG(SURFACE_FLAGS_OVERRIDE, SURFACE_FLAGS_UNKNOWN_STR, PF_STATICLIST | PF_HIDDEN, "This dropdown menu allows you to choose a specific surface flag for the VolumeBrush.  NOTE:  If the Unknown surface flag is used, the surface flag of the material applied to the brush will be used.")
	ADD_STRINGPROP_FLAG(PhysicsModel, "", PF_STATICLIST | PF_HIDDEN, "This dropdown menu allows you to choose what physics model is used with this volume.")
    ADD_BOOLPROP_FLAG(RayHit, false, PF_HIDDEN, "This flag toggles whether or not the object will be rayhit.")

	ADD_BOOLPROP_FLAG(StartOn, true, PF_HIDDEN, "This flag toggles whether or not the volume will be considered ON when the level first loads." )

END_CLASS_FLAGS_PLUGIN(Filter, VolumeBrush, CF_WORLDMODEL, CVolumePlugin, "Sound filter volume.")

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Filter )
CMDMGR_END_REGISTER_CLASS( Filter, VolumeBrush )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Filter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Filter::ReadProp(const GenericPropList *pProps)
{
	if (!pProps) return;

	VolumeBrush::ReadProp(pProps);

	// Make sure we are using the right friction.
	// The default value of "Friction" was changed on 8-3-05, this code
	// needs to remain until all world levels and pre-fabs are
	// younger than this date.
	m_fFriction = 1.0f;
}


BEGIN_CLASS(SafteyNet)
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN, "Sets amount of damage dealt by this area for every second the player is in it.")
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN, "This is a dropdown menu that allows you to choose the damage type that will be inflicted by the Volume.")
	ADD_REALPROP_FLAG(Friction, 1.0f, PF_HIDDEN, "This property allows you to alter the friction coefficient of surfaces to create slippery surfaces.")
	ADD_REALPROP_FLAG(Resistance, 0.0f, PF_HIDDEN, "This value affects the resistance of the players movement through the volume.")
	ADD_REALPROP_FLAG(Visible, 0, PF_HIDDEN, "This flag sets whether the brush that makes up the volume is visible or not.")
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN, "This is a force that pushes against the player. Use positive or negative values along each axis (x, y, and z) to indicate the direction and strength of the current. In general, you'll need values in the thousands in order to lift a player off the ground. This works well to create moving streams or areas of rushing wind.")
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN, "You can define a tint for the player's view if you want the area to look as if it's filled with light or liquid.")
    ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, PF_HIDDEN, "This allows you to further alter the view from inside a Volume.")
	PROP_DEFINEGROUP(FogStuff, PF_GROUP(2) | PF_HIDDEN, "This is a subset of properties that define the fog effects that will be seen when the player is within the Volume Brush.")
	HIDE_FORCE_VOLUME_PROPS
END_CLASS_FLAGS(SafteyNet, VolumeBrush, CF_WORLDMODEL, "SafteyNet volumes define a space that protect players from fall damage.  Player will not recieve fall damage while within a SafteyNet volume." )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( SafteyNet )
CMDMGR_END_REGISTER_CLASS( SafteyNet, VolumeBrush )

void SafteyNet::ReadProp(const GenericPropList *pProps)
{
	if (!pProps) return;

	VolumeBrush::ReadProp(pProps);

	// Make sure we are using the right friction.
	// The default value of "Friction" was changed on 8-3-05, this code
	// needs to remain until all world levels and pre-fabs are
	// younger than this date.
	m_fFriction = 1.0f;
}



BEGIN_CLASS( Gravity )

	ADD_BOOLPROP(Hidden, false, "This flag allows you to make a volume begin hidden in the level. Later you can send a (hidden 0) message to the VolumeBrush object to unhide it.")
	ADD_REALPROP_FLAG(Resistance, 0.0f, 0, "This value affects the resistance of the players movement through the volume.")
	ADD_REALPROP_FLAG(Friction, 1.0f, 0, "This property allows you to alter the friction coefficient of surfaces to create slippery surfaces.")
	ADD_VECTORPROP_VAL_FLAG(Current, 0.0f, 0.0f, 0.0f, PF_HIDDEN, "This is a force that pushes against the player. Use positive or negative values along each axis (x, y, and z) to indicate the direction and strength of the current. In general, you'll need values in the thousands in order to lift a player off the ground. This works well to create moving streams or areas of rushing wind.")
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN, "Sets amount of damage dealt by this area for every second the player is in it.")
 	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN, "This is a dropdown menu that allows you to choose the damage type that will be inflicted by the Volume.")
	ADD_COLORPROP_FLAG(TintColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN, "You can define a tint for the player's view if you want the area to look as if it's filled with light or liquid.")
    ADD_COLORPROP_FLAG(LightAdd, 0.0f, 0.0f, 0.0f, PF_HIDDEN, "This allows you to further alter the view from inside a Volume.")
 	ADD_STRINGPROP_FLAG(SoundFilter, "Dynamic", PF_STATICLIST | PF_HIDDEN, "You could have sound behave differently inside the volume. In the case of liquid, you could mute out high frequencies. In the case of a dimensional gate, you might add a strange echo or other filter.")
    ADD_BOOLPROP_FLAG(CanPlayMovementSounds, true, PF_HIDDEN, "This flag disables the footstep sounds and head-bob motion in a volume.")

	PROP_DEFINEGROUP(FogStuff, PF_GROUP(2) | PF_HIDDEN, "This is a subset of properties that define the fog effects that will be seen when the player is within the Volume Brush.")
	ADD_STRINGPROP_FLAG(PhysicsModel, "", PF_STATICLIST, "This dropdown menu allows you to choose what physics model is used with this volume." )

	// New prop

	ADD_REALPROP_FLAG( PlayerGravity, DEFAULT_WORLD_GRAVITY, 0, "This allows you to override the default gravity of the player while within this volume." )

END_CLASS_FLAGS( Gravity, VolumeBrush, CF_WORLDMODEL, "Gravity volumes define a space that can modify the players default gravity while the player is within the volume." )

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

void Gravity::ReadProp( const GenericPropList *pProps )
{
	if( !pProps )
		return;

	VolumeBrush::ReadProp( pProps );

	m_fGravity = pProps->GetReal( "PlayerGravity", m_fGravity );
}

BEGIN_CLASS( Wind )
	ADD_REALPROP_FLAG(Damage, 0.0f, PF_HIDDEN, "Sets amount of damage dealt by this area for every second the player is in it.")
	ADD_STRINGPROP_FLAG(DamageType, "UNSPECIFIED", PF_STATICLIST | PF_HIDDEN, "This is a dropdown menu that allows you to choose the damage type that will be inflicted by the Volume.")
	ADD_REALPROP_FLAG(Friction, 1.0f, 0, "This property allows you to alter the friction coefficient of surfaces to create slippery surfaces.")
END_CLASS_FLAGS( Wind, VolumeBrush, CF_WORLDMODEL, "Wind volumes define a space where the player will be affected by a wind current." )

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Wind )
CMDMGR_END_REGISTER_CLASS( Wind, VolumeBrush )

// EOF
