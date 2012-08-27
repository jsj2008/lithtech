// ----------------------------------------------------------------------- //
//
// MODULE  : ScatterFX.cpp
//
// PURPOSE : Scatter special FX - Implementation
//
// CREATED : 4/3/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScatterFX.h"
#include "iltclient.h"
#include "GameClientShell.h"


extern CGameClientShell* g_pGameClientShell;


// console variables
VarTrack g_cvarScatterEnable;				// false will totally disable scatter (deactivating all subvolumes)


bool CScatterFXSubVolume::tableInit = false;
float CScatterFXSubVolume::sinTable[];
uint32 CScatterFXSubVolume::rndTable[];
std::list<HOBJECT> CScatterFX::freeEffects;


struct DynamicParticleVertex
{
	float x, y, z;
	uint32 color;
	float u, v;
};


CScatterFX::CScatterFX() : CSpecialFX()
{
	VEC_INIT( m_vDims );
	m_nBlindDataIndex = 0xffffffff;
	m_fHeight = 64.0f;
	m_fWidth = 64.0f;
	m_fMaxScale = 1.0f;
	m_fTilt = 30.0f;
	m_fWaveRate = 90.0f;
	m_fWaveDist = 10.0f;
	m_fMaxDrawDist = 1024.0f;
	m_fMaxDrawDistSq = 1024.0f * 1024.0f;
	m_hstrTextureName = LTNULL;
	m_bUseSaturate = true;

	VEC_INIT( m_vCamPos );
	m_nInternalTime = 0;

	m_nNumSubVolumes = 0;
	m_pSubVolumes = NULL;

	m_bEnabled = true;
}


CScatterFX::~CScatterFX()
{
	if( m_hstrTextureName && m_pClientDE )
	{
		m_pClientDE->FreeString( m_hstrTextureName );
	}

	debug_deletea( m_pSubVolumes );

	for( std::list<HOBJECT>::iterator objIt = freeEffects.begin(); objIt != freeEffects.end(); objIt++ )
	{
		ASSERT( *objIt );
		m_pClientDE->RemoveObject( *objIt );
	}

	freeEffects.clear();
}


LTBOOL CScatterFX::Init( SFXCREATESTRUCT* psfxCreateStruct )
{
	if( !psfxCreateStruct )
		return LTFALSE;

	CSpecialFX::Init( psfxCreateStruct );

	SCATTERCREATESTRUCT* cs = (SCATTERCREATESTRUCT*)psfxCreateStruct;

	m_vDims = cs->vDims;
	m_nBlindDataIndex = cs->nBlindDataIndex;
	m_fWidth = cs->fWidth * 0.5f;
	m_fHeight = cs->fHeight;
	m_fMaxScale = cs->fMaxScale;
	m_fTilt = cs->fTilt;
	m_fWaveRate = cs->fWaveRate;
	m_fWaveDist = cs->fWaveDist;
	m_fMaxDrawDist = cs->fMaxDrawDist;
	m_fMaxDrawDistSq = m_fMaxDrawDist * m_fMaxDrawDist;
	m_hstrTextureName = cs->hstrTextureName;
	m_bUseSaturate = cs->bUseSaturate;

	LTVector pos;
	g_pLTClient->GetObjectPos( m_hServerObject, &pos );
	AdjustBounds( pos, m_vDims );

	// setup console variables
	if( !g_cvarScatterEnable.IsInitted() )
		g_cvarScatterEnable.Init( g_pLTClient, "ScatterEnable", LTNULL, 1.0f );

	return LTTRUE;
}


LTBOOL CScatterFX::CreateObject( ILTClient* pClientDE )
{
	if( !CSpecialFX::CreateObject( pClientDE ) )
		return LTFALSE;

	m_hObject = LTNULL;

	LTVector pos;
	g_pLTClient->GetObjectPos( m_hServerObject, &pos );
	m_vMinBounds = pos - m_vDims;
	m_vMaxBounds = pos + m_vDims;

	// grab the subvolumes and particles from the blind object data
	uint32 numVolumes = 0;

	if( m_nBlindDataIndex != 0xffffffff )
	{
		// grab the blind data
		uint8* blindData = NULL;
		uint32 blindDataSize = 0;
		if( m_pClientDE->GetBlindObjectData( m_nBlindDataIndex, SCATTER_BLINDOBJECTID, blindData, blindDataSize ) != LT_OK )
			return LTFALSE;
		uint8* curBlindData = blindData;

		uint32 fltSz = sizeof(float);

		// get the number of subvolumes
		numVolumes = *((uint32*)curBlindData);
		curBlindData += 4;

		if( numVolumes )
			m_pSubVolumes = debug_newa( CScatterFXSubVolume, numVolumes );
		m_nNumSubVolumes = numVolumes;

		for( uint32 i = 0; i < numVolumes; i++ )
		{
			// get the position of this subvolume
			LTVector pos;
			pos.x = *((float*)curBlindData);
			curBlindData += fltSz;
			pos.y = *((float*)curBlindData);
			curBlindData += fltSz;
			pos.z = *((float*)curBlindData);
			curBlindData += fltSz;

			// get the dimensions of this subvolume
			LTVector dims;
			dims.x = *((float*)curBlindData);
			curBlindData += fltSz;
			dims.y = *((float*)curBlindData);
			curBlindData += fltSz;
			dims.z = *((float*)curBlindData);
			curBlindData += fltSz;

			// get the number of particles in this volume
			uint32 numParticles = *((uint32*)curBlindData);
			curBlindData += 4;

			ASSERT( numParticles );

			// this will be dealloced by the subvolume it is passed down to
			CScatterFXParticle* particles = debug_newa( CScatterFXParticle, numParticles );

			for( uint32 j = 0; j < numParticles; j++ )
			{
				particles[j].pos.x = *((float*)curBlindData);
				curBlindData += fltSz;
				particles[j].pos.y = *((float*)curBlindData);
				curBlindData += fltSz;
				particles[j].pos.z = *((float*)curBlindData);
				curBlindData += fltSz;

				particles[j].color = *((uint32*)curBlindData) & 0x00ffffff;
				curBlindData += 4;

				particles[j].scale = *((float*)curBlindData);
				curBlindData += fltSz;

				particles[j].waveRot = *curBlindData;
				curBlindData++;

				particles[j].waveStart = *curBlindData;
				curBlindData++;

				//-0----
				float tmp = (float)rand() / (float)RAND_MAX;
				tmp *= 2.0f * PI;

				particles[j].xoffset = (float)sin( tmp ) * m_fWidth * particles[j].scale;
				particles[j].yoffset = (float)cos( tmp ) * m_fWidth * particles[j].scale;
				//------
			}

			AdjustBounds( pos, dims );

			m_pSubVolumes[i].Init( this, pos, dims, numParticles, particles );
		}

		// we're done with the blind data for this object, free it
		m_pClientDE->FreeBlindObjectData( m_nBlindDataIndex, SCATTER_BLINDOBJECTID );
	}

	return LTTRUE;
}


LTBOOL CScatterFX::Update( void )
{
	if( !m_pClientDE || !m_hServerObject )
		return LTFALSE;

	if( m_bWantRemove )
		return LTFALSE;

	// check if scatter should be enabled
	m_bEnabled = g_cvarScatterEnable.GetFloat() > 0.0f;

	// update our internal time counter
	if(!g_pGameClientShell->IsServerPaused())
		m_nInternalTime += (uint32)(10000.0f * g_pGameClientShell->GetFrameTime());

	// get the camera position
	HLOCALOBJ camera = g_pPlayerMgr->GetCamera();
	m_pClientDE->GetObjectPos( camera, &m_vCamPos );

	// update the sub volumes
	if( !UpdateSubVolumes() )
		return LTFALSE;

	return LTTRUE;
}


// returns the minimum squared distance to an AABB (0 if pos is inside the AABB)
static inline float DistSqToAABB( const LTVector& pos, const LTVector& min, const LTVector& max )
{
	float distance = 0.0f;

	if( pos.x < min.x )
	{
		float tmp = pos.x - min.x;
		distance = tmp * tmp;
	}
	else if( pos.x > max.x )
	{
		float tmp = pos.x - max.x;
		distance = tmp * tmp;
	}

	if( pos.y < min.y )
	{
		float tmp = pos.y - min.y;
		distance += tmp * tmp;
	}
	else if( pos.y > max.y )
	{
		float tmp = pos.y - max.y;
		distance += tmp * tmp;
	}

	if( pos.z < min.z )
	{
		float tmp = pos.z - min.z;
		distance += tmp * tmp;
	}
	else if( pos.z > max.z )
	{
		float tmp = pos.z - max.z;
		distance += tmp * tmp;
	}

	return distance;
}


// update which subvolumes are visible and setup LOD
bool CScatterFX::UpdateSubVolumes( void )
{
	// make sure there are no active subvolumes if scatter isn't enabled
	if( !m_bEnabled )
	{
		std::set<CScatterFXSubVolume*>::iterator it = activeSubVolumes.begin();
		while( it != activeSubVolumes.end() )
		{
			std::set<CScatterFXSubVolume*>::iterator next = it;
			next++;
			(*it)->Deactivate();
			it = next;
		}

		return true;
	}

	// get the distance from the camera to the volume (0 if inside)
	float cameraDistSq = DistSqToAABB( m_vCamPos, m_vMinBounds, m_vMaxBounds );

	//check to see if all subvolumes should be inactive
	if( cameraDistSq > m_fMaxDrawDistSq )
	{
		// camera is farther away than the max draw distance for this volume, so kill all the subvolumes
		std::set<CScatterFXSubVolume*>::iterator it = activeSubVolumes.begin();
		while( it != activeSubVolumes.end() )
		{
			std::set<CScatterFXSubVolume*>::iterator next = it;
			next++;
			(*it)->Deactivate();
			it = next;
		}

		// don't test any of the subvolumes directly
		return true;
	}

	// activate subvolumes with positive LOD
	for( uint32 i = 0; i < m_nNumSubVolumes; i++ )
	{
		float curLOD = m_pSubVolumes[i].CalculateLOD( m_vCamPos );

		if( curLOD > 0.0f )
			m_pSubVolumes[i].Activate( curLOD );
		else if( m_pSubVolumes[i].IsActive() )
			m_pSubVolumes[i].Deactivate();
	}

	return true;
}


// take bounds that assume point sized particles, and adjust for particle size, wave, etc.
void CScatterFX::AdjustBounds( LTVector& pos, LTVector& dims )
{
	// start off with no offset (all particles are considered to be static points)
	float maxXZOffset = 0.0f;
	float maxYOffset = 0.0f;

	// adjust for height of particles
	maxYOffset += m_fHeight * m_fMaxScale;

	// adjust for width of particles;
	maxXZOffset += m_fWidth * m_fMaxScale;

	// adjust for wind offsets
	maxXZOffset += m_fWaveDist;

	LTVector min = pos - dims;
	LTVector max = pos + dims;

	min.x -= maxXZOffset;
	min.z -= maxXZOffset;
	max.x += maxXZOffset;
	max.z += maxXZOffset;
	max.y += maxYOffset;

	dims = (max - min) * 0.5f;
	pos = min + dims;
}


LTBOOL CScatterFX::OnServerMessage( ILTMessage_Read* pMsg )
{
	return LTTRUE;
}



//---------------------------
//--- CScatterFXSubVolume ---
//---------------------------

CScatterFXSubVolume::CScatterFXSubVolume() :
	m_Parent(NULL), m_Active(false), m_NumParticles(0), m_Particles(NULL), m_Effect(NULL)
{
	if( !tableInit )
	{
		tableInit = true;

		srand( 1337 );

		float angleStep = (2*PI)/(float)SCATTERFX_TABLESIZE;
		float curAngle = 0.0f;

		uint32 i;
		for( i = 0; i < SCATTERFX_TABLESIZE; ++i )
		{
			rndTable[i] = (uint32)(((float)rand() / (float)RAND_MAX) * (SCATTERFX_TABLESIZE - 1));
			sinTable[i] = (float)sin( curAngle );
			curAngle += angleStep;
		}

		// add overlap to the end of the table for cosine to reference into
		for( ; i < SCATTERFX_TABLESIZE + SCATTERFX_COSOFFSET; ++i )
		{
			sinTable[i] = (float)sin( curAngle );
			curAngle += angleStep;
		}
	}
}


CScatterFXSubVolume::~CScatterFXSubVolume()
{
	Deactivate();

	debug_deletea( m_Particles );

	if( m_Effect )
		m_Parent->m_pClientDE->RemoveObject( m_Effect );
}


void CScatterFXSubVolume::Init( CScatterFX* parent, const LTVector& pos, const LTVector& dims, uint32 numParticles, CScatterFXParticle* particles )
{
	ASSERT( !m_Active );
	ASSERT( parent );

	m_Active = false;
	m_Parent = parent;
	m_LOD = 0.0f;
	m_Pos = pos;
	m_Dims = dims;
	m_MinBounds = pos - dims;
	m_MaxBounds = pos + dims;
	m_NumParticles = numParticles;
	m_Particles = particles;
}


// the engine calls this with a pointer to an subvolume in pUser
// fill in the specified vertex buffer with generated particles
bool ScatterFXFillVertexBuffer( void* pUser, void* pVertexBufferData, void* pLightingData, uint32 nVBNumVerts, uint32 nPrevFilledVerts, uint32& nNumFilledVerts )
{
	CScatterFXSubVolume* subVolume = (CScatterFXSubVolume*)pUser;

	return subVolume->FillVertexBuffer( pVertexBufferData, pLightingData, nVBNumVerts, nPrevFilledVerts, nNumFilledVerts );
}


// returns the LOD this subvolume should be at
float CScatterFXSubVolume::CalculateLOD( const LTVector& camPos )
{
	float distance = DistSqToAABB( camPos, m_MinBounds, m_MaxBounds );

	// the subvolume is too far away, give it a 0 LOD
	if( distance > m_Parent->m_fMaxDrawDistSq )
		return 0.0f;

	float lodBegin = SCATTERFX_LODPERCENT * m_Parent->m_fMaxDrawDist;
	distance = (float)sqrt( distance );

	// the subvolume is closer than the fadeout start point, give it a 1 LOD
	if( distance < lodBegin )
		return 1.0f;

	// evaluate falloff using 3x^2 - 2x^3
	distance = (distance - lodBegin) / (m_Parent->m_fMaxDrawDist - lodBegin);
	return 1.0f - (distance * distance * (3.0f - 2.0f * distance));
}


bool CScatterFXSubVolume::Activate( float detail )
{
	// update the LOD
	ASSERT( detail > 0.0f && detail <= 1.0f );
	m_LOD = detail;

	// adjust translucent vs. solid based on LOD
	if( m_LOD < 1.0f )
		g_pCommonLT->SetObjectFlags( m_Effect, OFT_Flags2, FLAG2_FORCETRANSLUCENT, FLAG2_FORCETRANSLUCENT );
	else
		g_pCommonLT->SetObjectFlags( m_Effect, OFT_Flags2, 0, FLAG2_FORCETRANSLUCENT );

	// don't do anything else if already active
	if( m_Active )
		return true;

	m_Active = true;

	// create the volume effect (either reusing an old one, or creating a new one)
	if( !m_Parent->freeEffects.empty() )
	{
		m_Effect = HOBJECT(*m_Parent->freeEffects.begin());
		m_Parent->freeEffects.pop_front();

		m_Parent->m_pClientDE->SetObjectPos( m_Effect, &m_Pos );
		g_pCommonLT->SetObjectFlags( m_Effect, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
	}
	else
	{
		ObjectCreateStruct ocs;
		INIT_OBJECTCREATESTRUCT( ocs );
		ocs.m_ObjectType = OT_VOLUMEEFFECT;
		ocs.m_Flags = FLAG_VISIBLE;
		ocs.m_Pos = m_Pos;
		ocs.m_Rotation.Init();

		m_Effect = m_Parent->m_pClientDE->CreateObject( &ocs );
	}

	VolumeEffectInfo vei;
	vei.m_EffectType = VolumeEffectInfo::kDynamicParticles;
	vei.m_Dims = m_Dims;
	vei.m_DPPrimitive = VolumeEffectInfo::kQuadlist;
	vei.m_DPLighting = VolumeEffectInfo::kNone;
	vei.m_DPLightConstant = 0xffffffff;
	vei.m_DPSaturate = m_Parent->m_bUseSaturate;
	vei.m_DPTextureName = m_Parent->m_pClientDE->GetStringData( m_Parent->m_hstrTextureName );
	vei.m_DPUserData = this;
	vei.m_DPUpdateFn = &ScatterFXFillVertexBuffer;

	m_Parent->m_pClientDE->SetupVolumeEffect( m_Effect, vei );

	m_Parent->activeSubVolumes.insert( this );
	
	return true;
}


bool CScatterFXSubVolume::Deactivate( void )
{
	if( !m_Active )
		return true;

	g_pCommonLT->SetObjectFlags( m_Effect, OFT_Flags, 0, FLAG_VISIBLE );
	m_Parent->freeEffects.push_front( m_Effect );
	m_Effect = NULL;

	m_Active = false;

	m_Parent->activeSubVolumes.erase( this );

	return true;
}


bool CScatterFXSubVolume::FillVertexBuffer( void* pVertexBufferData, void* pLightingData, uint32 nVBNumVerts, uint32 nPrevFilledVerts, uint32& nNumFilledVerts )
{
	DynamicParticleVertex* curVert = (DynamicParticleVertex*)pVertexBufferData;

	uint32 numQuads = nVBNumVerts / 4;
	uint32 prevQuads = nPrevFilledVerts / 4;

	if( numQuads > m_NumParticles - prevQuads )
		numQuads = m_NumParticles - prevQuads;

	uint32 maxQuad = prevQuads + numQuads;

	double seconds = m_Parent->m_nInternalTime / 10000.0;
	uint32 curWave = (uint32)((fmod( (m_Parent->m_fWaveRate * seconds), 360.0 ) / 360.0) * SCATTERFX_TABLESIZE);

	float width = m_Parent->m_fWidth;
	float height = m_Parent->m_fHeight;

	float waveAmt, waveXOffset, waveZOffset;

	LTVector pos;
	float xoff, yoff, scale;
	uint32 color, alpha;

	// setup the alpha information
	if( m_LOD < 1.0f )
		alpha = ((uint8)(m_LOD * 0xff)) << 24;
	else
		alpha = 0xff000000;

	uint32 curQuad = 0;

	for( uint32 i = prevQuads; i < maxQuad; ++i )
	{
		pos = m_Particles[i].pos;
		scale = m_Particles[i].scale * height;
		xoff = m_Particles[i].xoffset;
		yoff = m_Particles[i].yoffset;
		color = m_Particles[i].color | alpha;

		waveAmt = sinTable[(curWave + m_Particles[i].waveStart) % SCATTERFX_TABLESIZE] * m_Parent->m_fWaveDist;
		waveXOffset = sinTable[m_Particles[i].waveRot] * waveAmt;
		waveZOffset = sinTable[m_Particles[i].waveRot + SCATTERFX_COSOFFSET] * waveAmt;

		curVert->x = pos.x - xoff + waveXOffset;
		curVert->y = pos.y + scale;
		curVert->z = pos.z - yoff + waveZOffset;
		curVert->color = color;
		curVert->u = 0.0f;
		curVert->v = 0.0f;
		++curVert;
		curVert->x = pos.x - xoff;
		curVert->y = pos.y;
		curVert->z = pos.z - yoff;
		curVert->color = color;
		curVert->u = 0.0f;
		curVert->v = 1.0f;
		++curVert;
		curVert->x = pos.x + xoff + waveXOffset;
		curVert->y = pos.y + scale;
		curVert->z = pos.z + yoff + waveZOffset;
		curVert->color = color;
		curVert->u = 1.0f;
		curVert->v = 0.0f;
		++curVert;
		curVert->x = pos.x + xoff;
		curVert->y = pos.y;
		curVert->z = pos.z + yoff;
		curVert->color = color;
		curVert->u = 1.0f;
		curVert->v = 1.0f;
		++curVert;

		++curQuad;
	}

	nNumFilledVerts = curQuad * 4;

	return maxQuad == m_NumParticles;
}
