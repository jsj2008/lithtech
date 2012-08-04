// ----------------------------------------------------------------------- //
//
// MODULE  : SnowFX.cpp
//
// PURPOSE : Snow special FX - Implementation
//
// CREATED : 1/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SnowFX.h"
#include "iltclient.h"
#include "GameClientShell.h"


//#define SNOWFX_PROFILE_ACTIVATION			// if defined, airspace activation times will be displayed in the console


extern CGameClientShell* g_pGameClientShell;


// console variables
VarTrack g_cvarSnowEnable;					// false will totally disable snow (deactivating all airspaces)
VarTrack g_cvarSnowDensityScale;			// how much to scale the snow density by
VarTrack g_cvarSnowParticleScale;			// how much to scale each snow particle by


// initialize the static particle manager
CSnowFXParticleMgr CSnowFX::m_ParticleMgr;

bool CSnowFXAirspace::tableInit = false;
float CSnowFXAirspace::sinTable[];
uint32 CSnowFXAirspace::rndTable[];
std::list<HOBJECT> CSnowFX::freeEffects;


struct DynamicParticleVertex
{
	float x, y, z;
	uint32 color;
	float u, v;
};


struct DynamicParticleLightingData
{
	LTVector pos;			// position to sample lights at
	uint32 alpha;			// alpha is at 0xff000000
	LTVector acc;			// accumulated light color, game code should clear or initialize this (for ambient, etc.)
};



CSnowFX::CSnowFX() : CSpecialFX()
{
	VEC_INIT( m_vDims );
	m_fDensity = 512.0f;
	m_fParticleRadius = 1.0f * 2.0f;
	m_fFallRate = 80.0f;
	m_fTumbleRate = 180.0f / 360.0f;
	m_fTumbleRadius = 5.0f;
	m_fMaxDrawDist = 1024.0f;
	m_fMaxDrawDistSq = 1024.0f * 1024.0f;
	m_nAmbientColor = 0x00ffffff;
	m_bUseLighting = false;
	m_bUseSaturate = true;
	m_hstrTextureName = LTNULL;

	m_nInternalTime = 0;

	m_nNumAirspaces = 0;
	m_pAirspaces = NULL;

	m_bEnabled = true;
	m_fDensityScale = 1.0f;
	m_fParticleSizeScale = 1.0f;

	// allocate a few particles now just to initialize the table so we don't get a possible framerate hitch later
	CSnowFXParticle* tmpParticles;
	if( m_ParticleMgr.AllocateParticles( 10, tmpParticles ) )
		m_ParticleMgr.FreeParticles( tmpParticles );
}


CSnowFX::~CSnowFX()
{
	if( m_hstrTextureName && m_pClientDE )
	{
		m_pClientDE->FreeString( m_hstrTextureName );
	}

	debug_deletea( m_pAirspaces );

	for( std::list<HOBJECT>::iterator objIt = freeEffects.begin(); objIt != freeEffects.end(); objIt++ )
	{
		ASSERT( *objIt );
		m_pClientDE->RemoveObject( *objIt );
	}

	freeEffects.clear();
}


LTBOOL CSnowFX::Init( SFXCREATESTRUCT* psfxCreateStruct )
{
	if( !psfxCreateStruct )
		return LTFALSE;

	CSpecialFX::Init( psfxCreateStruct );

	SNOWCREATESTRUCT* cs = (SNOWCREATESTRUCT*)psfxCreateStruct;

	m_vDims = cs->vDims;
	m_fDensity = cs->fDensity;
	m_fParticleRadius = cs->fParticleRadius * 2.0f;
	m_fFallRate = cs->fFallRate;
	m_fTumbleRate = cs->fTumbleRate / 360.0f;
	m_fTumbleRadius = cs->fTumbleRadius;
	m_fMaxDrawDist = cs->fMaxDrawDist;
	m_fMaxDrawDistSq = m_fMaxDrawDist * m_fMaxDrawDist;
	m_vAmbientColor = cs->vAmbientColor;
	m_nAmbientColor  = (uint32(m_vAmbientColor.x) & 0xff) << 16;
	m_nAmbientColor |= (uint32(m_vAmbientColor.y) & 0xff) << 8;
	m_nAmbientColor |= (uint32(m_vAmbientColor.z) & 0xff);
	m_bUseLighting = cs->bUseLighting;
	m_bUseSaturate = cs->bUseSaturate;
	m_hstrTextureName = cs->hstrTextureName;

	// setup console variables
	if( !g_cvarSnowEnable.IsInitted() )
		g_cvarSnowEnable.Init( g_pLTClient, "SnowEnable", LTNULL, 1.0f );
	if( !g_cvarSnowDensityScale.IsInitted() )
		g_cvarSnowDensityScale.Init( g_pLTClient, "SnowDensityScale", LTNULL, 1.0f );
	if( !g_cvarSnowParticleScale.IsInitted() )
		g_cvarSnowParticleScale.Init( g_pLTClient, "SnowParticleScale", LTNULL, 1.0f );

	return LTTRUE;
}


LTBOOL CSnowFX::CreateObject( ILTClient* pClientDE )
{
	if( !CSpecialFX::CreateObject( pClientDE ) )
		return LTFALSE;

	m_hObject = LTNULL;

	// get the bounds for this snow volume
	LTVector pos;
	g_pLTClient->GetObjectPos( m_hServerObject, &pos );
	LTVector min = pos - m_vDims;
	LTVector size = m_vDims * 2.0f;
	m_vMinBounds = pos - m_vDims;
	m_vMaxBounds = pos + m_vDims;

	// determine how many airspaces it will divide into
	uint32 numXAirspaces = (uint32)((m_vDims.x * 2.0f) / SNOWFX_AIRSPACEDIMS) + 1;
	uint32 numZAirspaces = (uint32)((m_vDims.z * 2.0f) / SNOWFX_AIRSPACEDIMS) + 1;

	// don't add sliver airspaces on boundaries
	if( fmod( (m_vDims.x * 2.0f), SNOWFX_AIRSPACEDIMS ) < 1.0f )
		numXAirspaces--;
	if( fmod( (m_vDims.z * 2.0f), SNOWFX_AIRSPACEDIMS ) < 1.0f )
		numZAirspaces--;

	m_nNumAirspaces = numXAirspaces * numZAirspaces;

	// create the airspaces
	if( m_nNumAirspaces )
		m_pAirspaces = debug_newa( CSnowFXAirspace, m_nNumAirspaces );

	// intialize each airspace with it's position and size
	float remainingZSize = size.z;
	float curZMin = min.z;

	for( uint32 z = 0; z < numZAirspaces; z++ )
	{
		float curZSize = remainingZSize;
		if( curZSize >= SNOWFX_AIRSPACEDIMS )
		{
			curZSize = SNOWFX_AIRSPACEDIMS;
			remainingZSize -= SNOWFX_AIRSPACEDIMS;
		}
		curZSize *= 0.5f;

		float remainingXSize = size.x;
		float curXMin = min.x;

		for( uint32 x = 0; x < numXAirspaces; x++ )
		{
			float curXSize = remainingXSize;
			if( curXSize >= SNOWFX_AIRSPACEDIMS )
			{
				curXSize = SNOWFX_AIRSPACEDIMS;
				remainingXSize -= SNOWFX_AIRSPACEDIMS;
			}
			curXSize *= 0.5f;

			LTVector curPos;
			curPos.x = curXMin + curXSize;
			curPos.y = pos.y;
			curPos.z = curZMin + curZSize;

			LTVector curDims;
			curDims.x = curXSize;
			curDims.y = m_vDims.y;
			curDims.z = curZSize;

			m_pAirspaces[numXAirspaces*z + x].Init( this, curPos, curDims );

			curXMin += SNOWFX_AIRSPACEDIMS;
		}

		curZMin += SNOWFX_AIRSPACEDIMS;
	}

	return LTTRUE;
}


LTBOOL CSnowFX::Update( void )
{
	if( !m_pClientDE || !m_hServerObject )
		return LTFALSE;

	if( m_bWantRemove )
		return LTFALSE;

	// check if snow should be enabled
	m_bEnabled = g_cvarSnowEnable.GetFloat() > 0.0f;

	// update the density scale if needed
	float tmpDensityScale = g_cvarSnowDensityScale.GetFloat();
	if( tmpDensityScale < 0.0f )
		tmpDensityScale = 0.0f;
	if( tmpDensityScale != m_fDensityScale )
	{
		m_fDensityScale = tmpDensityScale;
		for( uint32 i = 0; i < m_nNumAirspaces; i++ )
		{
			m_pAirspaces[i].UpdateDensity();
		}
	}

	// update the particle radius scale
	m_fParticleSizeScale = g_cvarSnowParticleScale.GetFloat();

	// update our internal clock (since floats blow out over time)
	if(!g_pGameClientShell->IsServerPaused())
		m_nInternalTime += (uint32)(10000.0f * g_pGameClientShell->GetFrameTime());

	if( !UpdateAirspaces() )
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


// update which airspaces are visible and setup LOD
bool CSnowFX::UpdateAirspaces( void )
{
	// make sure there are no active airspaces if the snow isn't enabled
	if( !m_bEnabled )
	{
		std::set<CSnowFXAirspace*>::iterator it = activeAirspaces.begin();
		for( ; it != activeAirspaces.end(); it++ )
		{
			(*it)->Deactivate();
		}

		return true;
	}

	// get the camera position
	LTVector camPos;
	HLOCALOBJ camera = g_pPlayerMgr->GetCamera();
	m_pClientDE->GetObjectPos( camera, &camPos );

	// get the distance from the camera to the volume (0 if inside)
	float cameraDistSq = DistSqToAABB( camPos, m_vMinBounds, m_vMaxBounds );

	// check to see if all airspaces should be inactive
	if( cameraDistSq > m_fMaxDrawDistSq )
	{
		// camera is farther away than the max draw distance for this volume, so kill all the airspaces
		std::set<CSnowFXAirspace*>::iterator it = activeAirspaces.begin();
		for( ; it != activeAirspaces.end(); it++ )
		{
			(*it)->Deactivate();
		}

		// don't test any of the airspaces directly
		return true;
	}

	// activate airspaces with positive LOD
	for( uint32 i = 0; i < m_nNumAirspaces; i++ )
	{
		float curLOD = m_pAirspaces[i].CalculateLOD( camPos );

		if( curLOD > 0.0f )
			m_pAirspaces[i].Activate( curLOD );
		else if( m_pAirspaces[i].IsActive() )
			m_pAirspaces[i].Deactivate();
	}

	return true;
}


LTBOOL CSnowFX::OnServerMessage( ILTMessage_Read* pMsg )
{
	return LTTRUE;
}



//--------------------------
//--- CSnowFXParticleMgr ---
//--------------------------

CSnowFXParticleMgr::CSnowFXParticleMgr()
{
	m_NumParticles = 0;
	m_Particles = NULL;
}


CSnowFXParticleMgr::~CSnowFXParticleMgr()
{
	debug_deletea( m_Particles );
}


// get a pointer to a block of particles
bool CSnowFXParticleMgr::AllocateParticles( uint32 numParticles, CSnowFXParticle*& particles )
{
	// if this is the first time particles have been requested, initialize the particle array
	if( !m_Particles )
	{
		m_NumParticles = SNOWFX_PARTICLEPOOLSIZE;

		m_Particles = debug_newa( CSnowFXParticle, m_NumParticles );

		if( !m_Particles )
		{
			ASSERT(0);
			m_NumParticles = 0;
		}

		srand( 101374 );
		for( uint32 i = 0; i < m_NumParticles; i++ )
		{
			m_Particles[i].id = (uint32)(((float)rand() / (float)RAND_MAX) * (SNOWFX_TABLESIZE - 1));
			m_Particles[i].pos.x = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
			m_Particles[i].pos.y = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
			m_Particles[i].pos.z = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		}
	}

	if( !m_Particles || (numParticles > m_NumParticles) )
		return false;

	float offsetAmt = (float)rand() / (float)RAND_MAX;

	uint32 startRange = m_NumParticles - numParticles;
	uint32 start = (uint32)(offsetAmt * startRange);

	particles = &(m_Particles[start]);

	return true;
}


// release the allocated particles
bool CSnowFXParticleMgr::FreeParticles( CSnowFXParticle* particles )
{
	// don't actually do anything, but get the user used to freeing in case we do something in the future
	return true;
}



//-----------------------
//--- CSnowFXAirspace ---
//-----------------------

CSnowFXAirspace::CSnowFXAirspace() :
		m_Parent(NULL), m_Active(false), m_NumParticles(0), m_Particles(NULL), m_MinY(NULL), m_Effect(NULL)
{
	if( !tableInit )
	{
		tableInit = true;

		srand( 1337 );

		float angleStep = (2*PI)/(float)SNOWFX_TABLESIZE;
		float curAngle = 0.0f;

		uint32 i;
		for( i = 0; i < SNOWFX_TABLESIZE; i++ )
		{
			rndTable[i] = (uint32)(((float)rand() / (float)RAND_MAX) * (SNOWFX_TABLESIZE - 1));
			sinTable[i] = (float)sin( curAngle );
			curAngle += angleStep;
		}

		// add overlap to the end of the table for cosine to reference into
		for( ; i < SNOWFX_TABLESIZE + SNOWFX_COSOFFSET; i++ )
		{
			sinTable[i] = (float)sin( curAngle );
			curAngle += angleStep;
		}
	}
}


CSnowFXAirspace::~CSnowFXAirspace()
{
	Deactivate();

	if( m_Effect )
		m_Parent->m_pClientDE->RemoveObject( m_Effect );
}


void CSnowFXAirspace::Init( CSnowFX* parent, const LTVector& pos, const LTVector& dims )
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

	// get the particle blockers for this airspace
	m_Blockers.clear();
	m_Parent->m_pClientDE->GetParticleBlockersInAABB( pos, dims, m_Blockers );

	UpdateDensity();
}


bool CSnowFXAirspace::UpdateDensity( void )
{
	float relVolume = (m_Dims.x / 256.0f) * (m_Dims.y / 256.0f) * (m_Dims.z / 256.0f);
	m_NumParticles = (uint32)(m_Parent->m_fDensity * m_Parent->m_fDensityScale * relVolume);

	if( m_NumParticles >= SNOWFX_PARTICLEPOOLSIZE )
		m_NumParticles = SNOWFX_PARTICLEPOOLSIZE - 1;

	// reactivate active airspaces to get the right number of particles
	if( m_Active )
	{
		if( !Deactivate() )
			return false;

		if( !Activate( m_LOD ) )
			return false;
	}

	return true;
}


// the engine calls this with a pointer to an airspace in pUser
// fill in the specified vertex buffer with generated particles
bool SnowFXFillVertexBuffer( void* pUser, void* pVertexBufferData, void* pLightingData, uint32 nVBNumVerts, uint32 nPrevFilledVerts, uint32& nNumFilledVerts )
{
	CSnowFXAirspace* airspace = (CSnowFXAirspace*)pUser;

	return airspace->FillVertexBuffer( pVertexBufferData, pLightingData, nVBNumVerts, nPrevFilledVerts, nNumFilledVerts );
}


// returns the LOD this airspace should be at
float CSnowFXAirspace::CalculateLOD( const LTVector& camPos )
{
	float distance = DistSqToAABB( camPos, m_MinBounds, m_MaxBounds );

	// the airspace is too far away, give it a 0 LOD
	if( distance > m_Parent->m_fMaxDrawDistSq )
		return 0.0f;

	float lodBegin = SNOWFX_LODPERCENT * m_Parent->m_fMaxDrawDist;
	distance = (float)sqrt( distance );

	// the airspace is closer than the fadeout start point, give it a 1 LOD
	if( distance < lodBegin )
		return 1.0f;

	// evaluate falloff using 3x^2 - 2x^3
	distance = (distance - lodBegin) / (m_Parent->m_fMaxDrawDist - lodBegin);
	return 1.0f - (distance * distance * (3.0f - 2.0f * distance));
}


// see if a particle is within the blocker polygon
static inline bool TestParticleBlocker( const LTVector& pos, uint32 numEdges, const LTPlane*& edgeNormals )
{
	float px = pos.x;
	float pz = pos.z;

	const LTPlane* curPlane = edgeNormals;

	// check to see if the point is outside any of the edge planes
	for( uint32 i = 0; i < numEdges; i++, curPlane++ )
	{
		if( curPlane->m_Normal.x * px + curPlane->m_Normal.z * pz > curPlane->m_Dist )
			return false;
	}

	// point is inside all edge planes
	return true;
}


bool CSnowFXAirspace::Activate( float detail )
{
	// update the LOD
	ASSERT( detail > 0.0f && detail <= 1.0f );
	m_LOD = detail;

	// don't do anything else if already active
	if( m_Active )
		return true;

#ifdef SNOWFX_PROFILE_ACTIVATION
	LARGE_INTEGER pcStart, pcEnd, pcFreq;
	QueryPerformanceFrequency( &pcFreq );
	QueryPerformanceCounter( &pcStart );
#endif

	if( !m_Parent->m_ParticleMgr.AllocateParticles( m_NumParticles, m_Particles ) )
		return false;

	m_Active = true;

	// allocate space for the particle minimum elevation array
	m_MinY = debug_newa( float, m_NumParticles );

	// initialize the minimum elevation array
	for( uint32 i = 0; i < m_NumParticles; i++ )
	{
		m_MinY[i] = m_Pos.y - m_Dims.y;
	}

	// adjust minimum elevation array by blockers
	for( std::vector<uint32>::iterator it = m_Blockers.begin(); it != m_Blockers.end(); it++ )
	{
		float t;
		LTPlane blockerPlane;
		uint32 numEdges;
		LTPlane* edgePlanes;

		// get the current particle blocker
		m_Parent->m_pClientDE->GetParticleBlockerEdgesXZ( *it, blockerPlane, numEdges, edgePlanes );

		// don't consider nearly vertical blockers
		if( fabs(blockerPlane.m_Normal.y) < 0.00001 )
			continue;

		// precalculate some stuff for plane intersection calculations
		float dc = -1.0f / blockerPlane.m_Normal.y;
		float xc = blockerPlane.m_Normal.x * dc;
		float zc = blockerPlane.m_Normal.z * dc;
		dc *= blockerPlane.m_Dist;

		LTVector curPos;
		curPos.y = 0.0f;

		// check all particles against this blocker
		for( uint32 i = 0; i < m_NumParticles; i++ )
		{
			curPos.x = m_Pos.x + m_Particles[i].pos.x * m_Dims.x;
			curPos.z = m_Pos.z + m_Particles[i].pos.z * m_Dims.z;

			// check if the xz of the particle is within the projected xz polygon
			if( TestParticleBlocker( curPos, numEdges, const_cast<const LTPlane *&>(edgePlanes) ) )
			{
				// find where the particle hits the blocker
				t = dc + curPos.x * xc + curPos.z * zc;
				if( t > m_MinY[i] )
					m_MinY[i] = t;
			}
		}
	}

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
		ocs.m_Flags2 = FLAG2_FORCETRANSLUCENT;
		ocs.m_Pos = m_Pos;
		ocs.m_Rotation.Init();

		m_Effect = m_Parent->m_pClientDE->CreateObject( &ocs );
	}

	VolumeEffectInfo vei;
	vei.m_EffectType = VolumeEffectInfo::kDynamicParticles;
	vei.m_Dims = m_Dims;
	vei.m_DPPrimitive = VolumeEffectInfo::kTrilist;
	vei.m_DPLighting = m_Parent->m_bUseLighting ? VolumeEffectInfo::kSinglePointNonDirectional : VolumeEffectInfo::kNone;
	vei.m_DPLightConstant = 0xffffffff;
	vei.m_DPSaturate = m_Parent->m_bUseSaturate;
	vei.m_DPTextureName = m_Parent->m_pClientDE->GetStringData( m_Parent->m_hstrTextureName );
	vei.m_DPUserData = this;
	vei.m_DPUpdateFn = &SnowFXFillVertexBuffer;

	m_Parent->m_pClientDE->SetupVolumeEffect( m_Effect, vei );

	m_Parent->activeAirspaces.insert( this );

#ifdef SNOWFX_PROFILE_ACTIVATION
	QueryPerformanceCounter( &pcEnd );
	double pcTime = (double)(pcEnd.QuadPart - pcStart.QuadPart) * (double)1000.0 / (double)pcFreq.QuadPart;
	g_pLTClient->CPrint( " Snow Activation: %8.6f ms", pcTime );
#endif
	
	return true;
}


bool CSnowFXAirspace::Deactivate( void )
{
	if( !m_Active )
		return true;

	g_pCommonLT->SetObjectFlags( m_Effect, OFT_Flags, 0, FLAG_VISIBLE );
	m_Parent->freeEffects.push_front( m_Effect );
	m_Effect = NULL;

	m_Parent->m_ParticleMgr.FreeParticles( m_Particles );
	m_Particles = NULL;

	debug_deletea( m_MinY );
	m_MinY = NULL;

	m_Active = false;

	m_Parent->activeAirspaces.erase( this );

	return true;
}


bool CSnowFXAirspace::FillVertexBuffer( void* pVertexBufferData, void* pLightingData, uint32 nVBNumVerts, uint32 nPrevFilledVerts, uint32& nNumFilledVerts )
{
	DynamicParticleVertex* curVert = (DynamicParticleVertex*)pVertexBufferData;

	uint32 numTris = nVBNumVerts / 3;
	uint32 prevTris = nPrevFilledVerts / 3;

	if( numTris > m_NumParticles - prevTris )
		numTris = m_NumParticles - prevTris;

	uint32 maxTri = prevTris + numTris;

	LTVector pos;
	float alpha, curY, xRotOffset, zRotOffset;
	uint32 tmpInt, color;

	double seconds = m_Parent->m_nInternalTime / 10000.0;
	double fallRate = (m_Parent->m_fFallRate * seconds / m_Dims.y);
	uint32 tumbleOffset = (uint32)(m_Parent->m_fTumbleRate * seconds * SNOWFX_TABLESIZE) % SNOWFX_TABLESIZE;

	float particleSize = m_Parent->m_fParticleRadius * m_Parent->m_fParticleSizeScale;
	float tumbleRadius = m_Parent->m_fTumbleRadius;
	float yOffset = (float)fmod( fallRate, 2.0 );
	uint32 initialColor = m_Parent->m_nAmbientColor;
	uint8 baseAlpha = (uint8)(m_LOD * 0xff);
	uint32 baseAlphaShifted = baseAlpha << 24;

	uint32 curTri = 0;

	// two different loops, first for lit particles, second for unlit (be sure to update both when changing behavior!)
	if( m_Parent->m_bUseLighting )
	{
		LTVector ambient = m_Parent->m_vAmbientColor;
		DynamicParticleLightingData* curLight = (DynamicParticleLightingData*)pLightingData;

		// procedurally generate particles for this frame
		for( uint32 i = prevTris; i < maxTri; ++i )
		{
			// update the vertical position
			curY = m_Particles[i].pos.y - yOffset;
			if( curY  < -1.0f )
				curY += 2.0f;
			pos.y = m_Pos.y + curY * m_Dims.y;

			// fade in the first 20% of the particles life
			alpha = (1.0f - curY) * 2.5f;
			if( alpha < 1.0f )
				color = initialColor | ((uint32)(alpha*baseAlpha)<<24);
			else
				color = initialColor | baseAlphaShifted;

			//TEMP: need to kill particle rather than alpha it out
			if( pos.y < m_MinY[i] )
				color = initialColor;

			// get the initial position of the particle
			pos.x = m_Pos.x + m_Particles[i].pos.x * m_Dims.x;
			pos.z = m_Pos.z + m_Particles[i].pos.z * m_Dims.z;

			// orient the particle around the y axis
			tmpInt = m_Particles[i].id;
			zRotOffset = particleSize * sinTable[tmpInt];
			xRotOffset = particleSize * sinTable[tmpInt + SNOWFX_COSOFFSET];

			// move the particle back and forth as it falls
			tmpInt = (rndTable[tmpInt] + tumbleOffset) % SNOWFX_TABLESIZE;
			pos.x += tumbleRadius * sinTable[tmpInt + SNOWFX_COSOFFSET];
			pos.z += tumbleRadius * sinTable[tmpInt];

			// fill in the triangle information
			curVert->x = pos.x - xRotOffset;
			curVert->y = pos.y + particleSize;
			curVert->z = pos.z - zRotOffset;
			curVert->u = 0.0f;
			curVert->v = -1.0f;
			++curVert;
			curVert->x = pos.x - xRotOffset;
			curVert->y = pos.y - particleSize;
			curVert->z = pos.z - zRotOffset;
			curVert->u = 0.0f;
			curVert->v = 1.0f;
			++curVert;
			curVert->x = pos.x + xRotOffset;
			curVert->y = pos.y - particleSize;
			curVert->z = pos.z + zRotOffset;
			curVert->u = 2.0f;
			curVert->v = 1.0f;
			++curVert;

			// lighting information
			curLight->pos = pos;
			curLight->alpha = color;
			curLight->acc = ambient;
			++curLight;
			
			++curTri;
		}
	}
	else
	{
		// procedurally generate particles for this frame
		for( uint32 i = prevTris; i < maxTri; ++i )
		{
			// update the vertical position
			curY = m_Particles[i].pos.y - yOffset;
			if( curY  < -1.0f )
				curY += 2.0f;
			pos.y = m_Pos.y + curY * m_Dims.y;

			// fade in the first 20% of the particles life
			alpha = (1.0f - curY) * 2.5f;
			if( alpha < 1.0f )
				color = initialColor | ((uint32)(alpha*baseAlpha)<<24);
			else
				color = initialColor | baseAlphaShifted;

			//TEMP: need to kill particle rather than alpha it out
			if( pos.y < m_MinY[i] )
				color = initialColor;

			// get the initial position of the particle
			pos.x = m_Pos.x + m_Particles[i].pos.x * m_Dims.x;
			pos.z = m_Pos.z + m_Particles[i].pos.z * m_Dims.z;

			// orient the particle around the y axis
			tmpInt = m_Particles[i].id;
			zRotOffset = particleSize * sinTable[tmpInt];
			xRotOffset = particleSize * sinTable[tmpInt + SNOWFX_COSOFFSET];

			// move the particle back and forth as it falls
			tmpInt = (rndTable[tmpInt] + tumbleOffset) % SNOWFX_TABLESIZE;
			pos.x += tumbleRadius * sinTable[tmpInt + SNOWFX_COSOFFSET];
			pos.z += tumbleRadius * sinTable[tmpInt];

			// fill in the triangle information
			curVert->x = pos.x - xRotOffset;
			curVert->y = pos.y + particleSize;
			curVert->z = pos.z - zRotOffset;
			curVert->color = color;
			curVert->u = 0.0f;
			curVert->v = -1.0f;
			++curVert;
			curVert->x = pos.x - xRotOffset;
			curVert->y = pos.y - particleSize;
			curVert->z = pos.z - zRotOffset;
			curVert->color = color;
			curVert->u = 0.0f;
			curVert->v = 1.0f;
			++curVert;
			curVert->x = pos.x + xRotOffset;
			curVert->y = pos.y - particleSize;
			curVert->z = pos.z + zRotOffset;
			curVert->color = color;
			curVert->u = 2.0f;
			curVert->v = 1.0f;
			++curVert;

			++curTri;
		}
	}

	nNumFilledVerts = curTri * 3;

	return maxTri == m_NumParticles;
}
