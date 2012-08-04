// ----------------------------------------------------------------------- //
//
// MODULE  : SnowVolume.cpp
//
// PURPOSE : Large-scale procedural snow volume implementation:
//           - snow specific parameters
//
// CREATED : 1/3/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "MsgIDs.h"
#include "SnowVolume.h"
#include "DEditColors.h"
#include "ParsedMsg.h"


LINKFROM_MODULE( SnowVolume );

BEGIN_CLASS(SnowVolume)
	ADD_DEDIT_COLOR( SnowVolume )
	ADD_REALPROP(Density, 512.0f, "The number of particles placed per 256 cubic units of volume.  Snow volumes with different dimensions (vertical and XZ) but with the same density can be seemlessly placed next to each other since the density is relative to the volume and doesn't represent a specific number of particles.")
	ADD_REALPROP(ParticleRadius, 1.0f, "The approximate radius of a particle in WorldEdit units.")
	ADD_REALPROP(FallRate, 80.0f, "The speed at which snow will fall in WorldEdit units per second.  A value of 0 will result in particles that don't fall, but may still spin.  Negative values will not generate desirable results.")
	ADD_REALPROP(TumbleRate, 180.0f, "The number of degrees of rotation per second around a central point.")
	ADD_REALPROP(TumbleRadius, 5.0f, "The distance from the base particle position to rotate at.  0 will result in particles that fall straight down, 10 will result in particles that flutter up to 10 units in either direction as they fall.")
	ADD_REALPROP(MaxDrawDist, 1024.0f, "The maximum distance at which particles are drawn.  Starting at about 80%% of this distance, particles will begin to fade out.")
	ADD_COLORPROP(BaseColor, 255.0f, 255.0f, 255.0f, "The base color of the particles before lighting is added.")
	ADD_BOOLPROP(TranslucentLight, false, "No lights contribute to the color of the particles, only the base color is used.")
	ADD_BOOLPROP(Translucent, true, "Whether or not the particles that this scatter volume creates are translucent or not.")
	ADD_BOOLPROP(BackFaces, false, "This determines if a back face should be rendered for the scatter particles. This should ONLY be set when using a one sided shader as it slows down rendering")
	ADD_STRINGPROP_FLAG(MaterialName, "", PF_FILENAME, "The material to render the particles with.  All of the mip levels need to have a black border at the top and right edges of the material in order to render correctly.  The material needs to use non-premultiplied alpha (for example, entirely white color channel, with the actual shape in the alpha channel.)")
	ADD_BOOLPROP( StartOn, true, "Should the SnowVolume start emitting snow when it is created?" )
END_CLASS(SnowVolume, VolumeEffect, "SnowVolumes are used to define a spatial area where snow particles will be created and fall towards the ground." )


CMDMGR_BEGIN_REGISTER_CLASS( SnowVolume )

	ADD_MESSAGE( ON,	1,	NULL,	MSG_HANDLER( SnowVolume, HandleOnMsg ),		"ON", "This message turns the snow effect on. The effect will turn on through precipitation of particles from the top of the volume", "msg SnowVolume ON" )
	ADD_MESSAGE( OFF,	1,	NULL,	MSG_HANDLER( SnowVolume, HandleOffMsg ),	"OFF", "This message turns the particle system off. The particles disappear immediately", "msg SnowVolume OFF" )

CMDMGR_END_REGISTER_CLASS( SnowVolume, VolumeEffect )

SnowVolume::SnowVolume()
:	VolumeEffect		( ),
	m_fDensity			( 512.0f ),
	m_fParticleRadius	( 1.0f ),
	m_fFallRate			( 80.0f ),
	m_fTumbleRate		( 180.0f ),
	m_fTumbleRadius		( 5.0f ),
	m_fMaxDrawDist		( 1024.0f ),
	m_bTranslucent		( true ),
	m_bTranslucentLight	( false ),
	m_bBackFaces		( false ),
	m_nBaseColor		( SETRGBA(0xFF, 0xFF, 0xFF, 0xFF) ),
	m_sMaterialName		( ),
	m_bOn				( true )
{
	
}


SnowVolume::~SnowVolume()
{

}


uint32 SnowVolume::EngineMessageFn( uint32 messageID, void* pData, float fData )
{
	switch( messageID )
	{
	case MID_PRECREATE:
		{
			ObjectCreateStruct* pOCS = (ObjectCreateStruct*)pData;
			if( !pOCS )
				break;

			if( fData == PRECREATE_WORLDFILE )
			{
				ReadProp( &pOCS->m_cProperties );
				// make sure this object is always in the visible object list
				pOCS->m_Flags |= FLAG_FORCECLIENTUPDATE;
			}
		}
		break;

	case MID_INITIALUPDATE:
		{
			if( fData == INITIALUPDATE_WORLDFILE )			
			{
				InitialUpdate();
			}
		}
		break;
	}

	return VolumeEffect::EngineMessageFn( messageID, pData, fData );
}


bool SnowVolume::ReadProp( const GenericPropList *pProps )
{
	if( !pProps )
		return false;

	m_fDensity			= pProps->GetReal( "Density", m_fDensity );
	m_fParticleRadius	= pProps->GetReal( "ParticleRadius", m_fParticleRadius );
	m_fFallRate			= pProps->GetReal( "FallRate", m_fFallRate );
	m_fTumbleRate		= pProps->GetReal( "TumbleRate", m_fTumbleRate );
	m_fTumbleRadius		= pProps->GetReal( "TumbleRadius", m_fTumbleRadius );
	m_fMaxDrawDist		= pProps->GetReal( "MaxDrawDist", m_fMaxDrawDist );
	m_bTranslucent		= pProps->GetBool( "Translucent", m_bTranslucent );
	m_bTranslucentLight	= pProps->GetBool( "TranslucentLight", m_bTranslucentLight );
	m_bBackFaces		= pProps->GetBool( "BackFaces", m_bBackFaces );
	m_sMaterialName		= pProps->GetString( "MaterialName", "" );
	m_bOn				= pProps->GetBool( "StartOn", m_bOn );

	LTVector vColor	= pProps->GetColor( "BaseColor", LTVector(255.0f, 255.0f, 255.0f) );
	m_nBaseColor = SETRGBA((uint8)vColor.x, (uint8)vColor.y, (uint8)vColor.z, 0xFF);
	
	return true;
}


void SnowVolume::InitialUpdate( void )
{
	CreateSpecialFX();
}


void SnowVolume::WriteSnowInfo( ILTMessage_Write& cMsg )
{
	cMsg.WriteLTVector( m_vDims );
	cMsg.Writefloat( m_fDensity );
	cMsg.Writefloat( m_fParticleRadius );
	cMsg.Writefloat( m_fFallRate );
	cMsg.Writefloat( m_fTumbleRate );
	cMsg.Writefloat( m_fTumbleRadius );
	cMsg.Writefloat( m_fMaxDrawDist );
	cMsg.Writebool( m_bTranslucent );
	cMsg.Writebool( m_bTranslucentLight );
	cMsg.Writebool( m_bBackFaces );
	cMsg.Writeuint32( m_nBaseColor );
	cMsg.WriteString( m_sMaterialName.c_str() );
	cMsg.Writebool( m_bOn );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::CreateSpecialFX
//
//  PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void SnowVolume::CreateSpecialFX( bool bUpdateClients /* = false  */ )
{
	CAutoMessage cMsg;

	cMsg.Writeuint8( SFX_SNOW_ID );
	WriteSnowInfo( cMsg );
	g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );

	if( bUpdateClients )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_SNOW_ID );
		cMsg.WriteObject( m_hObject );
		WriteSnowInfo( cMsg );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::HandleOnMsg
//
//  PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void SnowVolume::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
	{
		TurnOn( true );
	}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::HandleOffMsg
//
//  PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void SnowVolume::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
	{
		TurnOn( false );
	}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::TurnOn
//
//  PURPOSE:	Turn the snow volume on and off... 
//
// ----------------------------------------------------------------------- //

void SnowVolume::TurnOn( bool bOn )
{
	if( m_bOn == bOn )
		return;

	m_bOn = bOn;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_SNOW_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( SVFX_TURNON );
	cMsg.Writebool( m_bOn );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::Save
//
//  PURPOSE:	Save the object... 
//
// ----------------------------------------------------------------------- //

void SnowVolume::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	VolumeEffect::Save( pMsg );

	SAVE_FLOAT( m_fDensity );
	SAVE_FLOAT( m_fParticleRadius );
	SAVE_FLOAT( m_fFallRate );
	SAVE_FLOAT( m_fTumbleRate );
	SAVE_FLOAT( m_fTumbleRadius );
	SAVE_FLOAT( m_fMaxDrawDist );
	SAVE_BOOL( m_bTranslucent );
	SAVE_BOOL( m_bTranslucentLight );
	SAVE_BOOL( m_bBackFaces );
	SAVE_DWORD( m_nBaseColor );
	SAVE_STDSTRING( m_sMaterialName );
	SAVE_bool( m_bOn );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::Load
//
//  PURPOSE:	Load the object...
//
// ----------------------------------------------------------------------- //

void SnowVolume::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	VolumeEffect::Load( pMsg );

	LOAD_FLOAT( m_fDensity );
	LOAD_FLOAT( m_fParticleRadius );
	LOAD_FLOAT( m_fFallRate );
	LOAD_FLOAT( m_fTumbleRate );
	LOAD_FLOAT( m_fTumbleRadius );
	LOAD_FLOAT( m_fMaxDrawDist );
	LOAD_BOOL( m_bTranslucent );
	LOAD_BOOL( m_bTranslucentLight );
	LOAD_BOOL( m_bBackFaces );
	LOAD_DWORD( m_nBaseColor );
	LOAD_STDSTRING( m_sMaterialName );
	LOAD_bool( m_bOn );

	CreateSpecialFX( true );
}
