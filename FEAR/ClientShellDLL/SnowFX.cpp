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
#include "iltrenderer.h"
#include "iltcustomrender.h"
#include "PlayerCamera.h"


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
CSnowFX::TFreeEffectList CSnowFX::freeEffects;


struct DynamicParticleVertex
{
	void Init(	float x, float y, float z, uint32 nColor, float u, float v, 
				const LTVector& vNormal, const LTVector& vTangent, const LTVector& vBinormal)
	{
		m_fX = x;
		m_fY = y;
		m_fZ = z;
		m_nColor = nColor;
		m_fU = u;
		m_fV = v;
		m_vNormal = vNormal;
		m_vTangent = vTangent;
		m_vBinormal = vBinormal;
	}

	float m_fX, m_fY, m_fZ;
	uint32 m_nColor;
	float m_fU, m_fV;
	LTVector m_vNormal, m_vTangent, m_vBinormal;
};

CSnowFX::CSnowFX() : CSpecialFX()
{
	m_vDims.Init();
	m_fDensity = 512.0f;
	m_fParticleRadius = 1.0f * 2.0f;
	m_fFallRate = 80.0f;
	m_fTumbleRate = 180.0f / 360.0f;
	m_fTumbleRadius = 5.0f;
	m_fMaxDrawDist = 1024.0f;
	m_fMaxDrawDistSq = 1024.0f * 1024.0f;
	m_bTranslucent = true;
	m_bTranslucentLight = true;
	m_bBackFaces = false;
	m_nBaseColor = 0xFFFFFFFF;
	m_hVertexDecl = NULL;
	m_bOn = true;
	m_bUseCycling = false;

	m_nInternalTime = 0;

	m_nNumAirspaces = 0;
	m_pAirspaces = NULL;

	m_bEnabled = true;
	m_fDensityScale = 1.0f;
	m_fParticleSizeScale = 1.0f;

	m_fLowestParticleY = 0.0f;

	// allocate a few particles now just to initialize the table so we don't get a possible framerate hitch later
	CSnowFXParticle* tmpParticles;
	if( m_ParticleMgr.AllocateParticles( 10, tmpParticles ) )
		m_ParticleMgr.FreeParticles( tmpParticles );
}


CSnowFX::~CSnowFX()
{
	debug_deletea( m_pAirspaces );

	for( TFreeEffectList::iterator objIt = freeEffects.begin(); objIt != freeEffects.end(); objIt++ )
	{
		ASSERT( *objIt );
		m_pClientDE->RemoveObject( *objIt );
	}

	g_pLTClient->GetCustomRender()->ReleaseVertexDeclaration(m_hVertexDecl);
	m_hVertexDecl = NULL;

	freeEffects.clear();
}


bool CSnowFX::Init( SFXCREATESTRUCT* psfxCreateStruct )
{
	if( !psfxCreateStruct )
		return false;

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
	m_bTranslucent = cs->bTranslucent;
	m_bTranslucentLight = cs->bTranslucentLight;
	m_bBackFaces = cs->bBackFaces;
	m_nBaseColor = cs->nBaseColor;
	m_sMaterialName = cs->sMaterialName;
	m_bOn = cs->bOn;

	LTVector pos;
	g_pLTClient->GetObjectPos( m_hServerObject, &pos );

	m_fLowestParticleY = pos.y + m_vDims.y;

	// setup console variables
	if( !g_cvarSnowEnable.IsInitted() )
		g_cvarSnowEnable.Init( g_pLTClient, "SnowEnable", NULL, 1.0f );
	if( !g_cvarSnowDensityScale.IsInitted() )
		g_cvarSnowDensityScale.Init( g_pLTClient, "SnowDensityScale", NULL, 1.0f );
	if( !g_cvarSnowParticleScale.IsInitted() )
		g_cvarSnowParticleScale.Init( g_pLTClient, "SnowParticleScale", NULL, 1.0f );

	//create our vertex declaration
	SVertexDeclElement VertexDecl[] =
	{
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Position, 0 },
		{ 0, eVertexDeclDataType_PackedColor, eVertexDeclUsage_Color, 0 },
		{ 0, eVertexDeclDataType_Float2, eVertexDeclUsage_TexCoord, 0 },
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Normal, 0 },
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Tangent, 0 },
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Binormal, 0 }
	};
	
	g_pLTClient->GetCustomRender()->CreateVertexDeclaration(LTARRAYSIZE(VertexDecl), VertexDecl, m_hVertexDecl);

	return true;
}


bool CSnowFX::CreateObject( ILTClient* pClientDE )
{
	if( !CSpecialFX::CreateObject( pClientDE ) )
		return false;

	m_hObject = NULL;

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

	return true;
}


bool CSnowFX::Update( void )
{
	if( !m_pClientDE || !m_hServerObject )
		return false;

	if( m_bWantRemove )
		return false;

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
	m_nInternalTime += (uint32)(10000.0f * SimulationTimer::Instance().GetTimerElapsedS( ));

	if( !UpdateAirspaces() )
		return false;

	return true;
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
	if( !m_bEnabled || !m_bOn )
	{
		TSubVolumeSet::iterator it = activeAirspaces.begin();
		while( it != activeAirspaces.end())
		{
			// Deactivate will remove the item from the set, thus invalidating the
			// iterator.  Get the next item before calling deactivate.
			TSubVolumeSet::iterator next = it;
			next++;
			(*it)->Deactivate();
			it = next;
		}


		return true;
	}

	// get the camera position
	LTVector camPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );

	// get the distance from the camera to the volume (0 if inside)
	float cameraDistSq = DistSqToAABB( camPos, m_vMinBounds, m_vMaxBounds );

	// check to see if all airspaces should be inactive
	if( cameraDistSq > m_fMaxDrawDistSq )
	{
		// camera is farther away than the max draw distance for this volume, so kill all the airspaces
		TSubVolumeSet::iterator it = activeAirspaces.begin();
		while( it != activeAirspaces.end())
		{
			// Get the snowfx and iterate to the next one.  Deactive will invalidate
			// our iterator.
			CSnowFXAirspace* pSnowFXAirspace = *it;
			it++;

			pSnowFXAirspace->Deactivate();
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

		m_bUseCycling &= !m_pAirspaces[i].m_bCycledOnce;
	}

	LTVector pos;
	g_pLTClient->GetObjectPos( m_hServerObject, &pos );

	// No need to use cycling if the particles can fill the entire volume...

	if( m_fLowestParticleY <= pos.y - m_vDims.y )
		m_bUseCycling = false;

	return true;
}


bool CSnowFX::OnServerMessage( ILTMessage_Read* pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return false;

	uint8 nMsgId = pMsg->Readuint8();

	switch(nMsgId)
	{
		case SVFX_TURNON :
		{
			m_bOn = pMsg->Readbool();
			
			// AirSpaces care about cycling when they activate...

			m_bUseCycling = true;

			// Reset the airspaces cycled flags..

			for( uint32 i = 0; i < m_nNumAirspaces; i++ )
			{
				m_pAirspaces[i].m_bCycledOnce = false;
			}

			LTVector pos;
			g_pLTClient->GetObjectPos( m_hServerObject, &pos );

			m_fLowestParticleY = pos.y + m_vDims.y;
		}
		break;

		default:
		break;
	}

	return true;
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
	}

		if( !m_Particles )
		{
			ASSERT(0);
			m_NumParticles = 0;
		}

		srand( 101374 );
		for( uint32 i = 0; i < m_NumParticles; i++ )
		{
		m_Particles[i].bCycled = false;
			m_Particles[i].id = (uint32)(((float)rand() / (float)RAND_MAX) * (SNOWFX_TABLESIZE - 1));
			m_Particles[i].pos.x = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
			m_Particles[i].pos.y = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
			m_Particles[i].pos.z = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
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
		m_Parent			( NULL ),
		m_Active			( false ),
		m_NumParticles		( 0 ),
		m_Particles			( NULL ),
		m_MinY				( NULL ),
		m_Effect			( NULL ),
		m_bUseCycling		( false ),
		m_bCycledOnce		( false )
{
	if( !tableInit )
	{
		tableInit = true;

		srand( 1337 );

		float angleStep = (2.0f * MATH_PI)/(float)SNOWFX_TABLESIZE;
		float curAngle = 0.0f;

		for( uint32 i = 0; i < SNOWFX_TABLESIZE; i++ )
		{
			rndTable[i] = (uint32)(((float)rand() / (float)RAND_MAX) * (SNOWFX_TABLESIZE - 1));
			sinTable[i] = LTSin( curAngle );
			curAngle += angleStep;
		}

		// add overlap to the end of the table for cosine to reference into
		for( ; i < SNOWFX_TABLESIZE + SNOWFX_COSOFFSET; i++ )
		{
			sinTable[i] = LTSin( curAngle );
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
	uint32 nSize = 0;
	// First get the number of blockers.
	m_Parent->m_pClientDE->GetParticleBlockersInAABB( pos, dims, NULL, nSize );
	m_Blockers.resize( nSize );
	m_Parent->m_pClientDE->GetParticleBlockersInAABB( pos, dims, &m_Blockers[0], nSize );

	m_bUseCycling = false;

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
static void SnowFXCustomRenderCallback( ILTCustomRenderCallback* pICustomRender, const LTRigidTransform& tCamera, void* pUser)
{
	CSnowFXAirspace* airspace = (CSnowFXAirspace*)pUser;
	airspace->CustomRenderCallback( pICustomRender );
}


// returns the LOD this airspace should be at
float CSnowFXAirspace::CalculateLOD( const LTVector& camPos )
{
	float distance = DistSqToAABB( camPos, m_MinBounds, m_MaxBounds );

	// the airspace is too far away, give it a 0 LOD
	if( distance > m_Parent->m_fMaxDrawDistSq )
		return 0.0f;

	float lodBegin = SNOWFX_LODPERCENT * m_Parent->m_fMaxDrawDist;
	distance = LTSqrt( distance );

	// the airspace is closer than the fadeout start point, give it a 1 LOD
	if( distance < lodBegin )
		return 1.0f;

	// evaluate falloff using 3x^2 - 2x^3
	distance = (distance - lodBegin) / (m_Parent->m_fMaxDrawDist - lodBegin);
	return 1.0f - (distance * distance * (3.0f - 2.0f * distance));
}


// see if a particle is within the blocker polygon
static inline bool TestParticleBlocker( const LTVector& pos, uint32 numEdges, const LTPlane* edgeNormals )
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
		m_MinY[i] = -m_Dims.y;
	}

	// adjust minimum elevation array by blockers
	for( TBlockerList::iterator it = m_Blockers.begin(); it != m_Blockers.end(); it++ )
	{
		float t;
		LTPlane blockerPlane;
		uint32 numEdges;
		LTPlane* edgePlanes;

		// get the current particle blocker
		m_Parent->m_pClientDE->GetParticleBlockerEdgesXZ( *it, blockerPlane, numEdges, edgePlanes );

		// don't consider nearly vertical blockers
		if( LTAbs(blockerPlane.m_Normal.y) < 0.00001 )
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
			if( TestParticleBlocker( curPos, numEdges, edgePlanes ) )
			{
				// find where the particle hits the blocker
				t = dc + curPos.x * xc + curPos.z * zc;
				if( t > m_MinY[i] )
					m_MinY[i] = t - m_Pos.y;
			}
		}
	}

	// create the volume effect (either reusing an old one, or creating a new one)
	if( !m_Parent->freeEffects.empty() )
	{
		m_Effect = HOBJECT(*m_Parent->freeEffects.begin());
		m_Parent->freeEffects.pop_front();

		m_Parent->m_pClientDE->SetObjectPos( m_Effect, m_Pos );
		g_pCommonLT->SetObjectFlags( m_Effect, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
	}
	else
	{
		ObjectCreateStruct ocs;
		ocs.m_ObjectType = OT_CUSTOMRENDER;
		ocs.m_Flags = FLAG_VISIBLE;
		ocs.m_Pos = m_Pos;
		ocs.m_Rotation.Init();

		if(m_Parent->m_bTranslucent)
			ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;
		if(!m_Parent->m_bTranslucentLight)
			ocs.m_Flags |= FLAG_NOLIGHT;

		m_Effect = m_Parent->m_pClientDE->CreateObject( &ocs );
	}

	//setup our vertex declaration


	//setup our material on our object
	HMATERIAL hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance( m_Parent->m_sMaterialName.c_str() );
	g_pLTClient->GetCustomRender()->SetMaterial(m_Effect, hMaterial);
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	//now setup the visibility of this object
	g_pLTClient->GetCustomRender()->SetVisBoundingBox(m_Effect, -m_Dims, m_Dims);

	//and finally, setup our callback function
	g_pLTClient->GetCustomRender()->SetRenderCallback(m_Effect, SnowFXCustomRenderCallback);
	g_pLTClient->GetCustomRender()->SetCallbackUserData(m_Effect, this);

	g_pLTClient->GetCustomRender()->SetRenderingSpace(m_Effect, eRenderSpace_Object);

	m_bUseCycling = m_Parent->m_bUseCycling;
	m_bCycledOnce = false;

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


void CSnowFXAirspace::CustomRenderCallback( ILTCustomRenderCallback* pCustomRender)
{
	//determine if we need to generate back faces
	bool bBackFaces = m_Parent->m_bBackFaces;
	uint32 nVertsPerQuad = (bBackFaces) ? 8 : 4;
	uint32 nIndicesPerQuad = (bBackFaces) ? 12 : 6;

	//determine the largest number of quads that we can fill in during a given batch
	uint32 nVertsPerBatch	= DYNAMIC_RENDER_VERTEX_STREAM_SIZE / sizeof(DynamicParticleVertex);
	uint32 nIndicesPerBatch = QUAD_RENDER_INDEX_STREAM_SIZE;
	uint32 nQuadsPerBatch	= LTMIN(nVertsPerBatch / nVertsPerQuad, nIndicesPerBatch / nIndicesPerQuad);

	//setup our vertex declaration for rendering
	if(pCustomRender->SetVertexDeclaration(m_Parent->m_hVertexDecl) != LT_OK)
		return;

	//also setup our quad indices
	if(pCustomRender->BindQuadIndexStream() != LT_OK)
		return;

	//precalculate some data that will be used when generating
	LTVector pos;
	float alpha, curY, xRotOffset, zRotOffset, xUnitOffset, zUnitOffset;
	uint32 tmpInt, color;

	double seconds = m_Parent->m_nInternalTime / 10000.0;
	double fallRate = (m_Parent->m_fFallRate * seconds / m_Dims.y);
	uint32 tumbleOffset = (uint32)(m_Parent->m_fTumbleRate * seconds * SNOWFX_TABLESIZE) % SNOWFX_TABLESIZE;

	float particleSize = m_Parent->m_fParticleRadius * m_Parent->m_fParticleSizeScale;
	float tumbleRadius = m_Parent->m_fTumbleRadius;
	float yOffset = (float)fmod( fallRate, 2.0 );

	uint8 nInitialAlpha = (m_Parent->m_bTranslucent) ? 0 : 0xFF;
	uint8 nBaseAlpha = (m_Parent->m_bTranslucent) ? 0xFF : (uint8)(m_LOD * 0xFF);
	uint32 baseAlphaShifted = SETRGBA(0, 0, 0, nBaseAlpha);
	uint32 initialColor = SETRGBA(GETR(m_Parent->m_nBaseColor), GETG(m_Parent->m_nBaseColor), GETB(m_Parent->m_nBaseColor), nInitialAlpha);

	//the current triangle we are rendering
	uint32 nCurrQuad = 0;
	m_bCycledOnce = true;

	LTVector vNormal, vTangent, vBinormal;

	//render all the triangles
	while(nCurrQuad < m_NumParticles)
	{
		//determine how many triangles we want to lock
		uint32 nNumQuadsForBatch = LTMIN(m_NumParticles - nCurrQuad, nQuadsPerBatch);

		//request the lock from the dynamic vertex buffer
		SDynamicVertexBufferLockRequest LockRequest;
		LTRESULT hr = pCustomRender->LockDynamicVertexBuffer(nQuadsPerBatch * nVertsPerQuad, LockRequest);
		if((hr != LT_OK) || (LockRequest.m_nStride != sizeof(DynamicParticleVertex)))
		{
			//failed to lock the buffer, just fail
			return;
		}

		DynamicParticleVertex* pCurrVert = (DynamicParticleVertex*)LockRequest.m_pData;

		//determine the end of when we want to generate
		uint32 nBatchEnd = nCurrQuad + nNumQuadsForBatch;
			
		// procedurally generate particles for this batch
		while(nCurrQuad < nBatchEnd)
		{
			// update the vertical position
			curY = m_Particles[nCurrQuad].pos.y - yOffset;
			if( curY  < -1.0f )
				curY += 2.0f;
			pos.y = curY * m_Dims.y;

			// get the initial position of the particle
			pos.x = m_Particles[nCurrQuad].pos.x * m_Dims.x;
			pos.z = m_Particles[nCurrQuad].pos.z * m_Dims.z;

			// orient the particle around the y axis
			tmpInt = m_Particles[nCurrQuad].id;
			zUnitOffset = sinTable[tmpInt];
			xUnitOffset = sinTable[tmpInt + SNOWFX_COSOFFSET];
			zRotOffset = particleSize * zUnitOffset;
			xRotOffset = particleSize * xUnitOffset;


			// fade in the first 20% of the particles life
			alpha = (1.0f - curY) * 2.5f;
			if( alpha < 1.0f )
			{
				color = initialColor | ((uint32)(alpha*nBaseAlpha)<<24);

				m_Particles[nCurrQuad].bCycled = true;
			}
			else if( m_bUseCycling )
			{
				if( m_Particles[nCurrQuad].bCycled )
				{
					// This particle has been cycled so we should be able to see it...

					color = (initialColor | baseAlphaShifted);

					if( pos.y < m_Parent->m_fLowestParticleY )
					{
						// Set this cycled particles Y to the parents lowest Y...
						// All particles above the parents Y should be visible, cycled or not...

						m_Parent->m_fLowestParticleY = pos.y;
					}
				}
				else
				{
					if( pos.y >= m_Parent->m_fLowestParticleY )
					{
						// All particles above the parents Y should be visible and considered cycled...

						color = color = (initialColor | baseAlphaShifted);
						m_Particles[nCurrQuad].bCycled = true;
					}
					else
					{
						// Not cycled and below the parents Y so we shouldn't see it...

						color = initialColor;
					}
				}

				// See if all the particles have been cycled...

				m_bCycledOnce &= m_Particles[nCurrQuad].bCycled;
			}
			else
			{
				color = initialColor | baseAlphaShifted;
			} 

			//if particle is out of volume, shrink it to zero to hide it
			if( pos.y < m_MinY[nCurrQuad] )
			{
				zRotOffset = 0.0f;
				xRotOffset = 0.0f;
			}

			// move the particle back and forth as it falls
			tmpInt = (rndTable[tmpInt] + tumbleOffset) % SNOWFX_TABLESIZE;
			pos.x += tumbleRadius * sinTable[tmpInt + SNOWFX_COSOFFSET];
			pos.z += tumbleRadius * sinTable[tmpInt];

			vNormal.Init(zUnitOffset, 0.0f, -xUnitOffset);
			vTangent.Init(-xUnitOffset, 0.0f, -zUnitOffset);
			vBinormal.Init(0.0f, 1.0f, 0.0f);

			// fill in the triangle information
			pCurrVert->Init(pos.x - xRotOffset, pos.y + particleSize, pos.z - zRotOffset,
							color, 0.0f, 0.0f, vNormal, vTangent, vBinormal);
			++pCurrVert;

			pCurrVert->Init(pos.x + xRotOffset, pos.y + particleSize, pos.z + zRotOffset,
							color, 1.0f, 0.0f, vNormal, vTangent, vBinormal);
			++pCurrVert;

			pCurrVert->Init(pos.x + xRotOffset, pos.y - particleSize, pos.z + zRotOffset,
							color, 1.0f, 1.0f, vNormal, vTangent, vBinormal);
			++pCurrVert;

			pCurrVert->Init(pos.x - xRotOffset, pos.y - particleSize, pos.z - zRotOffset,
							color, 0.0f, 1.0f, vNormal, vTangent, vBinormal);
			++pCurrVert;

			if(bBackFaces)
			{
				vNormal = -vNormal;

				pCurrVert->Init(pos.x - xRotOffset, pos.y - particleSize, pos.z - zRotOffset,
								color, 0.0f, 1.0f, vNormal, vTangent, vBinormal);
				++pCurrVert;

				pCurrVert->Init(pos.x + xRotOffset, pos.y - particleSize, pos.z + zRotOffset,
								color, 1.0f, 1.0f, vNormal, vTangent, vBinormal);
				++pCurrVert;

				pCurrVert->Init(pos.x + xRotOffset, pos.y + particleSize, pos.z + zRotOffset,
								color, 1.0f, 0.0f, vNormal, vTangent, vBinormal);
				++pCurrVert;

				pCurrVert->Init(pos.x - xRotOffset, pos.y + particleSize, pos.z - zRotOffset,
								color, 0.0f, 0.0f, vNormal, vTangent, vBinormal);
				++pCurrVert;
			}

			++nCurrQuad;
		}

		//unlock and bind the buffer, then render
		pCustomRender->UnlockAndBindDynamicVertexBuffer(LockRequest);
		pCustomRender->RenderIndexed(	eCustomRenderPrimType_TriangleList, 
										0, nNumQuadsForBatch * nIndicesPerQuad, LockRequest.m_nStartIndex, 
										0, nNumQuadsForBatch * nVertsPerQuad);
	}
}
