// ----------------------------------------------------------------------- //
//
// MODULE  : ScatterVolume.cpp
//
// PURPOSE : Scattered surface particles volume implementation:
//           - scatter specific parameters
//
// CREATED : 4/3/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "MsgIDs.h"
#include "ScatterVolume.h"

LINKFROM_MODULE( ScatterVolume );

BEGIN_CLASS(ScatterVolume)
	ADD_VECTORPROP_VAL_FLAG(Dims, 128.0f, 128.0f, 128.0f, PF_LOCALDIMS, "The Dims define the volume an effect takes place in. It is an area defined in WorldEdit units as a box around the center of the effect object.")
	ADD_REALPROP_FLAG(BlindDataIndex, -1.0f, PF_HIDDEN, "The index into the blind object data")
	ADD_REALPROP(Density, 128.0f, "The number of particles placed per 256 square units of surface area.  The density can be reduced by placement material blending and clumpiness.")
	ADD_REALPROP(Clumpiness, 0.0f, "A number between 0 and 1 that specifies how much particles will 'clump' together.  0 distributes particles evenly over the surface, while 1 will result in very clumpy placement.  This works about the same as a 'Noise' material in Max or Maya.")
	ADD_REALPROP(ClumpSize, 256.0f, "The approximate dimensions of a clump before it has been modified with the clump cutoff value.  Also the approximate distance between clumps.")
	ADD_REALPROP(ClumpCutoff, 0.5f, "A number between 0 and 1 that specifies how sharp the edges of clumps should be.  0 will result in a smooth falloff in density towards the edges of the clump, while higher numbers will result in sharper edges and fewer stragglers.  Numbers above about 0.7 will result in VERY sparse placement.")
	ADD_VECTORPROP_VAL(ClumpOffset, 0.0f, 0.0f, 0.0f, "By default, different scatter volumes will have clumps in the same locations in world space.  This can be useful for having multiple materials within the same clump.  If you really want the clumps to be independent of each other, change the clump offset in one of the volumes.  A good value might be 50 50 50 for example.")
	ADD_REALPROP(Width, 64.0f, "The base width of the quad that is rendered for each particle.  Note that this is independent of material aspect ratio, so if you material is 256x64, you would probably enter a width that is 4 times larger than the height.")
	ADD_REALPROP(Height, 64.0f, "The base height of the quad that is rendered for each particle.  Note that this is independent of material aspect ratio, so if you material is 256x64, you would probably enter a width that is 4 times larger than the height.")
	ADD_REALPROP(PlantDepth, 0.0f, "A value that specifies how far the particle should be embedded in the surface.  If this is negative, then the particles will float above the surface.")
	ADD_REALPROP(Tilt, 30.0f, "The range of tilt that particles may be placed at.  This value is in degrees.")
	ADD_REALPROP(ScaleRange, 0.2f, "The +/- range of scales that different particles will have.  For example, value of 0.2 will result in particles that are between 80%% and 120%% of the base width and height.")
	ADD_REALPROP(WaveRate, 90.0f, "The speed of the wave motion of the particles.  This is in degrees per second, so 360 will result in a complete wave cycle every second, while 90 will result in a complete wave cycle every 4 seconds.")
	ADD_REALPROP(WaveDist, 10.0f, "The distance in WorldEdit units to move the top of the particles.  For example, value of 10 will result in a total movement range of 10 units in either direction.")
	ADD_REALPROP(WaveSize, 128.0f, "The approximate dimensions of an area of common movement.  Over this area, particles will wave approximately in unison, smoothly blending to other nearby motions.  This works about the same as a 'Noise' texture in Max or Maya.")
	ADD_REALPROP(WaveDirSize, 128.0f, "The size of areas of similar wind direction.")
	ADD_REALPROP(MaxDrawDist, 1024.0f, "The maximum distance at which particles in a subvolume of this volume will be visible.")
	ADD_STRINGPROP_FLAG(MaterialName, "", PF_FILENAME, "The material that will be placed on the rendered quad.  Particles are placed on the surface based on the bottom center of the quad, so it is usually best for materials to have an anchor point at the bottom center as well.")
	ADD_STRINGPROP_FLAG(PlacementMaterial, "", PF_FILENAME, "This is an optional material that is only used for placing materials.  If this is not empty, then particles will only be placed on polygons that have this material applied to them.  If multi-texturing is in use, then the amount of this material will be used to change the density based on vertex alpha.")
	ADD_REALPROP(PlacementCutoff, 0.5f, "If multi-texturing is in use on a polygon, then points on the surface with a vertex alpha value less than this value will not have any particles placed on them.")
	ADD_COLORPROP(BaseColor, 255.0f, 255.0f, 255.0f, "The base color of the particles before lighting is added.")
	ADD_BOOLPROP(TranslucentLight, FALSE, "No lights contribute to the color of the particles, only the base color is used.")
	ADD_BOOLPROP(Translucent, TRUE, "Whether or not the particles that this scatter volume creates are translucent or not.")
	ADD_BOOLPROP(BackFaces, FALSE, "This determines if a back face should be rendered for the scatter particles. This should ONLY be set when using a one sided shader as it slows down rendering")
	ADD_REALPROP(NumImages, 1, "This indicates the number of images stored in the material. These should be layed out like an image strip along the X axis, so if there are two images, image one sould go from 0-.5, and image two should go from .5-1 horizontally.")
END_CLASS(ScatterVolume, VolumeEffect, "A scatter volume will generate a collection of particles on the geometry contained within the dimensions of the object and will handle rendering these particles at runtime.")

CMDMGR_BEGIN_REGISTER_CLASS( ScatterVolume )
CMDMGR_END_REGISTER_CLASS( ScatterVolume, VolumeEffect )

ScatterVolume::ScatterVolume()
:	VolumeEffect		( ),
	m_nBlindDataIndex	( 0xffffffff ),
	m_fHeight			( 64.0f ),
	m_fWidth			( 64.0f ),
	m_fMaxScale			( 1.0f ),
	m_fWaveRate			( 90.0f ),
	m_fWaveDist			( 10.0f ),
	m_fMaxDrawDist		( 1024.0f ),
	m_sMaterialName		( ),
	m_bTranslucent		( true ),
	m_bTranslucentLight	( false ),
	m_bBackFaces		( false ),
	m_nBaseColor		( SETRGBA(0xFF, 0xFF, 0xFF, 0xFF) ),
	m_nNumImages		( 1 )
{
}

ScatterVolume::~ScatterVolume()
{
}


uint32 ScatterVolume::EngineMessageFn( uint32 messageID, void* pData, float fData )
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


bool ScatterVolume::ReadProp( const GenericPropList *pProps )
{
	if( !pProps )
		return false;

	float fBlindDataIndex = pProps->GetReal( "BlindDataIndex", -1.0f );
	if( fBlindDataIndex > 0.0f )
		m_nBlindDataIndex = (uint32)fBlindDataIndex;
	else
		m_nBlindDataIndex = 0xffffffff;

	m_fHeight		= pProps->GetReal( "Height", m_fHeight );
	m_fWidth		= pProps->GetReal( "Width", m_fWidth );
	m_fWaveRate		= pProps->GetReal( "WaveRate", m_fWaveRate );
	m_fWaveDist		= pProps->GetReal( "WaveDist", m_fWaveDist );
	m_fMaxDrawDist	= pProps->GetReal( "MaxDrawDist", m_fMaxDrawDist );
	m_bTranslucent	= pProps->GetBool( "Translucent", m_bTranslucent );
	m_bTranslucentLight	= pProps->GetBool( "TranslucentLight", m_bTranslucentLight );
	m_bBackFaces	= pProps->GetBool( "BackFaces", m_bBackFaces );

	LTVector vColor	= pProps->GetColor( "BaseColor", LTVector(255.0f, 255.0f, 255.0f) );
	m_nBaseColor = SETRGBA((uint8)vColor.x, (uint8)vColor.y, (uint8)vColor.z, 0xFF);

	m_nNumImages = (uint32)LTCLAMP((int32)(pProps->GetReal("NumImages", 1.0f) + 0.5f), 1, 255);

	m_fMaxScale = 1.0f + pProps->GetReal( "ScaleRange", 0.2f );

	m_sMaterialName = pProps->GetString( "MaterialName", "" );
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SnowVolume::CreateSpecialFX
//
//  PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void ScatterVolume::CreateSpecialFX( bool bUpdateClients /* = false  */ )
{
	CAutoMessage cMsg;

	cMsg.Writeuint8( SFX_SCATTER_ID );
	WriteScatterInfo( cMsg );
	g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );

	if( bUpdateClients )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_SCATTER_ID );
		cMsg.WriteObject( m_hObject );
		WriteScatterInfo( cMsg );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
	}
}

void ScatterVolume::InitialUpdate( void )
{
	CreateSpecialFX( );
}


void ScatterVolume::WriteScatterInfo( ILTMessage_Write& cMsg )
{
	cMsg.WriteLTVector( m_vDims );
	cMsg.Writeuint32( m_nBlindDataIndex );
	cMsg.Writefloat( m_fHeight );
	cMsg.Writefloat( m_fWidth );
	cMsg.Writefloat( m_fMaxScale );
	cMsg.Writefloat( m_fWaveRate );
	cMsg.Writefloat( m_fWaveDist );
	cMsg.Writefloat( m_fMaxDrawDist );
	cMsg.WriteString( m_sMaterialName.c_str() );
	cMsg.Writebool( m_bTranslucent );
	cMsg.Writebool( m_bTranslucentLight );
	cMsg.Writebool( m_bBackFaces );
	cMsg.Writeuint32( m_nBaseColor );
	cMsg.Writeuint8( m_nNumImages );
}

void ScatterVolume::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	VolumeEffect::Save( pMsg );

	SAVE_VECTOR( m_vDims );
	SAVE_DWORD( m_nBlindDataIndex );
	SAVE_FLOAT( m_fHeight );
	SAVE_FLOAT( m_fWidth );
	SAVE_FLOAT( m_fMaxScale );
	SAVE_FLOAT( m_fWaveRate );
	SAVE_FLOAT( m_fWaveDist );
	SAVE_FLOAT( m_fMaxDrawDist );
	SAVE_STDSTRING( m_sMaterialName );
	SAVE_BOOL( m_bTranslucent );
	SAVE_BOOL( m_bTranslucentLight );
	SAVE_BOOL( m_bBackFaces );
	SAVE_DWORD( m_nBaseColor );
	SAVE_DWORD( m_nNumImages );
}

void ScatterVolume::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	VolumeEffect::Load( pMsg );

	LOAD_VECTOR( m_vDims );
	LOAD_DWORD( m_nBlindDataIndex );
	LOAD_FLOAT( m_fHeight );
	LOAD_FLOAT( m_fWidth );
	LOAD_FLOAT( m_fMaxScale );
	LOAD_FLOAT( m_fWaveRate );
	LOAD_FLOAT( m_fWaveDist );
	LOAD_FLOAT( m_fMaxDrawDist );
	LOAD_STDSTRING( m_sMaterialName );
	LOAD_BOOL( m_bTranslucent );
	LOAD_BOOL( m_bTranslucentLight );
	LOAD_BOOL( m_bBackFaces );
	LOAD_DWORD( m_nBaseColor );
	LOAD_DWORD( m_nNumImages );

	CreateSpecialFX( true );
}
