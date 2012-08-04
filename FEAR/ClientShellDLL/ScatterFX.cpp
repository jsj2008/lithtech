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
#include "iltrenderer.h"
#include "iltcustomrender.h"
#include "PlayerCamera.h"

//the version information for our blind object data
#define SCATTER_BLINDOBJECTVERSION		1

//the scatter particle data found in the blind object data
class CBlindObjectScatterParticle
{
public:
	LTVector pos;
	float scale;
	uint8 waveRot;
	uint8 waveStart;
	uint8 offsetIndex;
	uint8 imageIndex;
};

#if defined(PLATFORM_XENON)
// XENON: Necessary code for implementing runtime swapping
#include "endianswitch.h"

// Type descriptions for the endian swapper
DATATYPE_TO_ENDIANFORMAT(CBlindObjectScatterParticle, "4f4c");

#endif // PLATFORM_XENON

extern CGameClientShell* g_pGameClientShell;


// console variables
VarTrack g_cvarScatterEnable;				// false will totally disable scatter (deactivating all subvolumes)


bool CScatterFXSubVolume::tableInit = false;
float CScatterFXSubVolume::sinTable[];
float* CScatterFXSubVolume::cosTable;
uint32 CScatterFXSubVolume::rndTable[];
CScatterFX::TFreeEffectList CScatterFX::freeEffects;


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


CScatterFX::CScatterFX() : CSpecialFX()
{
	m_vDims.Init();
	m_nBlindDataIndex = 0xffffffff;
	m_fHeight = 64.0f;
	m_fWidth = 64.0f;
	m_fMaxScale = 1.0f;
	m_fWaveRate = 90.0f;
	m_fWaveDist = 10.0f;
	m_fMaxDrawDist = 1024.0f;
	m_fMaxDrawDistSq = 1024.0f * 1024.0f;
	m_hVertexDecl = NULL;
	m_bTranslucent = true;
	m_bTranslucentLight = true;
	m_bBackFaces = false;
	m_nBaseColor = 0xFFFFFFFF;
	m_nNumImages = 1;

	m_nInternalTime = 0;

	m_nNumSubVolumes = 0;
	m_pSubVolumes = NULL;

	m_bEnabled = true;
}


CScatterFX::~CScatterFX()
{
	debug_deletea( m_pSubVolumes );

	for( TFreeEffectList::iterator objIt = freeEffects.begin(); objIt != freeEffects.end(); objIt++ )
	{
		ASSERT( *objIt );
		m_pClientDE->RemoveObject( *objIt );
	}

	g_pLTClient->GetCustomRender()->ReleaseVertexDeclaration(m_hVertexDecl);
	m_hVertexDecl = NULL;

	freeEffects.clear();
}


bool CScatterFX::Init( SFXCREATESTRUCT* psfxCreateStruct )
{
	if( !psfxCreateStruct )
		return false;

	CSpecialFX::Init( psfxCreateStruct );

	SCATTERCREATESTRUCT* cs = (SCATTERCREATESTRUCT*)psfxCreateStruct;

	m_vDims = cs->vDims;
	m_nBlindDataIndex = cs->nBlindDataIndex;
	m_fWidth = cs->fWidth * 0.5f;
	m_fHeight = cs->fHeight;
	m_fMaxScale = cs->fMaxScale;
	m_fWaveRate = cs->fWaveRate;
	m_fWaveDist = cs->fWaveDist;
	m_fMaxDrawDist = cs->fMaxDrawDist;
	m_fMaxDrawDistSq = m_fMaxDrawDist * m_fMaxDrawDist;
	m_sMaterialName = cs->sMaterialName;
	m_bTranslucent = cs->bTranslucent;
	m_bTranslucentLight = cs->bTranslucentLight;
	m_bBackFaces = cs->bBackFaces;
	m_nBaseColor = cs->nBaseColor;
	m_nNumImages = cs->nNumImages;

	g_pLTClient->GetObjectTransform( m_hServerObject, &m_tTransform );
	m_tTransform.GetInverse().ToMatrix(m_mInvTransform);		

	m_vMinBounds = -m_vDims;
	m_vMaxBounds = m_vDims;
	AdjustBounds( m_vMinBounds, m_vMaxBounds );

	// setup console variables
	if( !g_cvarScatterEnable.IsInitted() )
		g_cvarScatterEnable.Init( g_pLTClient, "ScatterEnable", NULL, 1.0f );

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


bool CScatterFX::CreateObject( ILTClient* pClientDE )
{
	if( !CSpecialFX::CreateObject( pClientDE ) )
		return false;

	m_hObject = NULL;

	// grab the subvolumes and particles from the blind object data
	uint32 numVolumes = 0;

	if( m_nBlindDataIndex != 0xffffffff )
	{
		// grab the blind data
		uint8* blindData = NULL;
		uint32 blindDataSize = 0;
		if( m_pClientDE->GetBlindObjectData( m_nBlindDataIndex, SCATTER_BLINDOBJECTID, blindData, blindDataSize ) != LT_OK )
			return false;
		uint8* curBlindData = blindData;

		uint32 fltSz = sizeof(float);

		//get our version information
		uint32 nVersion = *((uint32*)curBlindData);
		curBlindData += 4;

		if(nVersion != SCATTER_BLINDOBJECTVERSION)
			return false;

		// get the number of subvolumes
		numVolumes = *((uint32*)curBlindData);
		curBlindData += 4;

#if defined(PLATFORM_XENON)
		// XENON: Swap data at runtime
		LittleEndianToNative(&numVolumes);
#endif // PLATFORM_XENON

		if( numVolumes )
			m_pSubVolumes = debug_newa( CScatterFXSubVolume, numVolumes );
		m_nNumSubVolumes = numVolumes;

		for( uint32 i = 0; i < numVolumes; i++ )
		{
#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			// Note: The next values are two LTVectors
			LittleEndianToNative((LTVector*)curBlindData, 2);
#endif // PLATFORM_XENON

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
#if defined(PLATFORM_XENON)
			// XENON: Swap data at runtime
			LittleEndianToNative(&numParticles);
			// Following the particles is an array of structures, so swap them all at the same time
			LittleEndianToNative((CBlindObjectScatterParticle*)curBlindData, numParticles);
#endif // PLATFORM_XENON

			ASSERT( numParticles );

			//point to our blind object data directly
			const CBlindObjectScatterParticle* pBlindObject = (const CBlindObjectScatterParticle*)curBlindData;
			curBlindData += sizeof(CBlindObjectScatterParticle) * numParticles;

			LTVector vMin = pos - dims;
			LTVector vMax = pos + dims;
			AdjustBounds( vMin, vMax );

			m_pSubVolumes[i].Init( this, vMin, vMax, numParticles, pBlindObject );
		}
	}

	return true;
}


bool CScatterFX::Update( void )
{
	if( !m_pClientDE || !m_hServerObject )
		return false;

	if( m_bWantRemove )
		return false;

	// check if scatter should be enabled
	m_bEnabled = g_cvarScatterEnable.GetFloat() > 0.0f;

	// update our internal time counter
	m_nInternalTime += (uint32)(10000.0f * SimulationTimer::Instance().GetTimerElapsedS( ));

	// get the camera position
	LTVector vCamPos = m_mInvTransform * g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );

	// update the sub volumes
	if( !UpdateSubVolumes(vCamPos) )
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


// update which subvolumes are visible and setup LOD
bool CScatterFX::UpdateSubVolumes( const LTVector& vCamPos )
{
	// make sure there are no active subvolumes if scatter isn't enabled
	if( !m_bEnabled )
	{
		TSubVolumeSet::iterator it = activeSubVolumes.begin();
		while( it != activeSubVolumes.end())
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

	// get the distance from the camera to the volume (0 if inside)
	float cameraDistSq = DistSqToAABB( vCamPos, m_vMinBounds, m_vMaxBounds );

	//check to see if all subvolumes should be inactive
	if( cameraDistSq > m_fMaxDrawDistSq )
	{
		// camera is farther away than the max draw distance for this volume, so kill all the subvolumes
		TSubVolumeSet::iterator it = activeSubVolumes.begin();
		while( it != activeSubVolumes.end())
		{
			// Deactivate will remove the item from the set, thus invalidating the
			// iterator.  Get the next item before calling deactivate.
			TSubVolumeSet::iterator next = it;
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
		float curLOD = m_pSubVolumes[i].CalculateLOD( vCamPos );

		if( curLOD > 0.0f )
			m_pSubVolumes[i].Activate( curLOD );
		else if( m_pSubVolumes[i].IsActive() )
			m_pSubVolumes[i].Deactivate();
	}

	return true;
}


// take bounds that assume point sized particles, and adjust for particle size, wave, etc.
void CScatterFX::AdjustBounds( LTVector& vMin, LTVector& vMax )
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

	vMin.x -= maxXZOffset;
	vMin.z -= maxXZOffset;
	vMax.x += maxXZOffset;
	vMax.z += maxXZOffset;
	vMax.y += maxYOffset;
}


bool CScatterFX::OnServerMessage( ILTMessage_Read* /*pMsg*/ )
{
	return true;
}



//---------------------------
//--- CScatterFXSubVolume ---
//---------------------------

CScatterFXSubVolume::CScatterFXSubVolume() :
	m_Parent(NULL), m_Active(false), m_NumParticles(0), m_pBlindObject(NULL), m_Effect(NULL)
{
	if( !tableInit )
	{
		tableInit = true;

		srand( 1337 );

		float angleStep = (2.0f * MATH_PI)/(float)SCATTERFX_TABLESIZE;
		float curAngle = 0.0f;

		for( uint32 i = 0; i < SCATTERFX_TABLESIZE; ++i )
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

		cosTable = sinTable + SCATTERFX_COSOFFSET;
	}
}


CScatterFXSubVolume::~CScatterFXSubVolume()
{
	Deactivate();

	if( m_Effect )
		m_Parent->m_pClientDE->RemoveObject( m_Effect );
}


void CScatterFXSubVolume::Init( CScatterFX* parent, const LTVector& vMin, const LTVector& vMax, uint32 numParticles, 
								const CBlindObjectScatterParticle* pBlindObject )
{
	ASSERT( !m_Active );
	ASSERT( parent );

	m_Active = false;
	m_Parent = parent;
	m_LOD = 0.0f;
	m_MinBounds = vMin;
	m_MaxBounds = vMax;
	m_NumParticles = numParticles;
	m_pBlindObject = pBlindObject;
}


// the engine calls this with a pointer to an subvolume in pUser
// fill in the specified vertex buffer with generated particles
static void ScatterFXCustomRenderCallback( ILTCustomRenderCallback* pICustomRender, const LTRigidTransform& tCamera, void* pUser)
{
	CScatterFXSubVolume* subVolume = (CScatterFXSubVolume*)pUser;
	subVolume->CustomRenderCallback( pICustomRender );
}


// returns the LOD this subvolume should be at
float CScatterFXSubVolume::CalculateLOD( const LTVector& camPos ) const
{
	float distance = DistSqToAABB( camPos, m_MinBounds, m_MaxBounds );

	// the subvolume is too far away, give it a 0 LOD
	if( distance > m_Parent->m_fMaxDrawDistSq )
		return 0.0f;

	float lodBegin = SCATTERFX_LODPERCENT * m_Parent->m_fMaxDrawDist;
	distance = LTSqrt( distance );

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

	// don't do anything else if already active
	if( m_Active )
		return true;

	m_Active = true;

	// create the volume effect (either reusing an old one, or creating a new one)
	if( !m_Parent->freeEffects.empty() )
	{
		m_Effect = HOBJECT(*m_Parent->freeEffects.begin());
		m_Parent->freeEffects.pop_front();

		m_Parent->m_pClientDE->SetObjectTransform( m_Effect, m_Parent->m_tTransform );
		g_pCommonLT->SetObjectFlags( m_Effect, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
	}
	else
	{
		ObjectCreateStruct ocs;
		ocs.m_ObjectType = OT_CUSTOMRENDER;
		ocs.m_Flags = FLAG_VISIBLE;
		ocs.m_Pos = m_Parent->m_tTransform.m_vPos;
		ocs.m_Rotation = m_Parent->m_tTransform.m_rRot;

		if(m_Parent->m_bTranslucent)
			ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;
		if(!m_Parent->m_bTranslucentLight)
			ocs.m_Flags |= FLAG_NOLIGHT;

		m_Effect = m_Parent->m_pClientDE->CreateObject( &ocs );
	}

	//setup our material on our object
	HMATERIAL hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance( m_Parent->m_sMaterialName.c_str() );
	g_pLTClient->GetCustomRender()->SetMaterial(m_Effect, hMaterial);
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	//now setup the visibility of this object (but need to make it in world space)
	LTMatrix3x4 mObjSpace;
	m_Parent->m_tTransform.ToMatrix(mObjSpace);

	LTVector vCenter = mObjSpace * ((m_MinBounds + m_MaxBounds) * 0.5f) - m_Parent->m_tTransform.m_vPos;
	LTVector vDims = (m_MaxBounds - m_MinBounds) * 0.5f;

	LTVector vRight, vUp, vForward;
	mObjSpace.GetBasisVectors(vRight, vUp, vForward);
	
	LTVector vAABBDims;
	vAABBDims.x = fabsf(vRight.x) * vDims.x + fabsf(vUp.x) * vDims.y + fabsf(vForward.x) * vDims.z;
	vAABBDims.y = fabsf(vRight.y) * vDims.x + fabsf(vUp.y) * vDims.y + fabsf(vForward.y) * vDims.z;
	vAABBDims.z = fabsf(vRight.z) * vDims.x + fabsf(vUp.z) * vDims.y + fabsf(vForward.z) * vDims.z;

	g_pLTClient->GetCustomRender()->SetVisBoundingBox(m_Effect, vCenter - vAABBDims, vCenter + vAABBDims);

	//and finally, setup our callback function
	g_pLTClient->GetCustomRender()->SetRenderCallback(m_Effect, ScatterFXCustomRenderCallback);
	g_pLTClient->GetCustomRender()->SetCallbackUserData(m_Effect, this);

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


void CScatterFXSubVolume::CustomRenderCallback( ILTCustomRenderCallback* pCustomRender )
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

	double seconds = m_Parent->m_nInternalTime / 10000.0;
	uint32 curWave = (uint32)((fmod( (m_Parent->m_fWaveRate * seconds), 360.0 ) / 360.0) * SCATTERFX_TABLESIZE);

	float height = m_Parent->m_fHeight;
	float width = m_Parent->m_fWidth;

	float waveAmt, waveXOffset, waveZOffset;

	LTVector pos;
	float xoff, yoff, scale, widthscale, unitxoff, unityoff;

	//determine our color based upon what our parent is set to
	uint32 nColor = m_Parent->m_nBaseColor;

	// setup the alpha information so it can fade out at a distance
	uint8 nAlpha = 0xFF;
    if(m_Parent->m_bTranslucent && (m_LOD < 1.0f) )
		nAlpha = ((uint8)(m_LOD * 0xff));

	nColor = SETRGBA(GETR(nColor), GETG(nColor), GETB(nColor), nAlpha);

	//setup our U/V scales for the texture
	float fTexWidth = 1.0f / (float)m_Parent->m_nNumImages;

	//the current quad we are rendering
	uint32 nCurrQuad = 0;

    //render all the triangles
	while(nCurrQuad < m_NumParticles)
	{
		//determine how many quads we want to lock
		uint32 nNumQuadsForBatch = LTMIN(m_NumParticles - nCurrQuad, nQuadsPerBatch);

		//request the lock from the dynamic vertex buffer
		SDynamicVertexBufferLockRequest LockRequest;
		LTRESULT hr = pCustomRender->LockDynamicVertexBuffer(nNumQuadsForBatch * nVertsPerQuad, LockRequest);

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
			const CBlindObjectScatterParticle& Particle = m_pBlindObject[nCurrQuad];

			widthscale = Particle.scale * width;
			unitxoff = sinTable[Particle.offsetIndex];
			unityoff = cosTable[Particle.offsetIndex];
			pos = Particle.pos;
			scale = Particle.scale * height;
			xoff = unitxoff * widthscale;
			yoff = unityoff * widthscale;

			//determine our U/V coords
			float fU0 = Particle.imageIndex * fTexWidth;
			float fU1 = fU0 + fTexWidth;

			LTVector vNormal(unityoff, 0.0f, -unitxoff);
			LTVector vTangent(-unitxoff, 0.0f, -unityoff);
			LTVector vBinormal(0.0f, 1.0f, 0.0f);

			waveAmt = sinTable[(curWave + m_pBlindObject[nCurrQuad].waveStart) % SCATTERFX_TABLESIZE] * m_Parent->m_fWaveDist;
			waveXOffset = sinTable[m_pBlindObject[nCurrQuad].waveRot] * waveAmt;
			waveZOffset = cosTable[m_pBlindObject[nCurrQuad].waveRot] * waveAmt;

			pCurrVert->Init(pos.x - xoff + waveXOffset, pos.y + scale, pos.z - yoff + waveZOffset,
							nColor, fU0, 0.0f, vNormal, vTangent, vBinormal);
			++pCurrVert;

			pCurrVert->Init(pos.x + xoff + waveXOffset, pos.y + scale, pos.z + yoff + waveZOffset,
							nColor, fU1, 0.0f, vNormal, vTangent, vBinormal);
			++pCurrVert;

			pCurrVert->Init(pos.x + xoff, pos.y, pos.z + yoff,
							nColor, fU1, 1.0f, vNormal, vTangent, vBinormal);
			++pCurrVert;

			pCurrVert->Init(pos.x - xoff, pos.y, pos.z - yoff,
							nColor, fU0, 1.0f, vNormal, vTangent, vBinormal);
			++pCurrVert;

			if(bBackFaces)
			{
				vNormal = -vNormal;

				pCurrVert->Init(pos.x - xoff, pos.y, pos.z - yoff,
								nColor, fU0, 1.0f, vNormal, vTangent, vBinormal);
				++pCurrVert;

				pCurrVert->Init(pos.x + xoff, pos.y, pos.z + yoff,
								nColor, fU1, 1.0f, vNormal, vTangent, vBinormal);
				++pCurrVert;

				pCurrVert->Init(pos.x + xoff + waveXOffset, pos.y + scale, pos.z + yoff + waveZOffset,
								nColor, fU1, 0.0f, vNormal, vTangent, vBinormal);
				++pCurrVert;

				pCurrVert->Init(pos.x - xoff + waveXOffset, pos.y + scale, pos.z - yoff + waveZOffset,
								nColor, fU0, 0.0f, vNormal, vTangent, vBinormal);
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
