#include "precompile.h"

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

#include "3d_ops.h"
#include "common_stuff.h"
#include "fixedpoint.h"
#include "de_objects.h"
#include "d3d_texture.h"
#include "d3d_draw.h"
#include "iltclient.h"
#include "d3d_renderstatemgr.h"
#include "setuptouchinglights.h"
#include "rendererframestats.h"

#include "renderstruct.h"
#include "counter.h"

#include "LTEffectImpl.h"
#include "lteffectshadermgr.h"
#include "ltshaderdevicestateimp.h"
#include "rendererconsolevars.h"

// ---------------------------------------------------------------- //
// Definitions.
// ---------------------------------------------------------------- //

#define PARTICLE_BATCH_SIZE		64
#define NUM_PARTICLE_INDICES	(PARTICLE_BATCH_SIZE * 6)
#define PI						3.141592653f

//number of rotations to precompute
#define ROT_TABLE_SIZE			256



// ---------------------------------------------------------------- //
// Globals.
// ---------------------------------------------------------------- //

namespace
{

	//indices into each of the different offset vectors
	enum EOffsetCorner
	{
		UpperLeft	= 0,
		UpperRight	= 1,
		BottomRight = 2,
		BottomLeft  = 3,
		NumOffsets
	};

	//These are essentially the same vectors as below, however, they are each rotated in
	//a table for faster rotation of sprites

	static LTVector g_vRotOffset[ROT_TABLE_SIZE][NumOffsets];

	static LTVector g_vOffset[NumOffsets];

	//the normal of the particles
	static LTVector g_vParticleNormal;

	//these are the prior camera up and forward vectors. These are stored in order to allow
	//caching of rotation vectors between particle systems if they are close enough. Note that these
	//will be 0 vectors at init since they are in the global space, and the caching relies upon this.
	static LTVector g_vCacheTestRotUp;
	static LTVector g_vCacheTestRotRight;


} // unnamed namespace




//----------------------------------------------------------------------------
// Particle Vertex format
//   Uses: HW TnL, 1 texture channel, diffuse color, specular color (for fog)
//----------------------------------------------------------------------------
#define PARTICLEVERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)



class CParticleVertex
{
public:

	CParticleVertex()
		: m_Normal(g_vParticleNormal)
	{
	}

	//Called to setup the fields of the vertex
	inline void 		SetupVert(const LTVector& vPos, const LTVector& vOffset, float fSize, uint32 nColor)
	{
		m_Vec.x		= vPos.x + vOffset.x * fSize;
		m_Vec.y		= vPos.y + vOffset.y * fSize;
		m_Vec.z		= vPos.z + vOffset.z * fSize;
		m_nColor	= nColor;
	}

	static void 		SetupBaseVerts()
	{
		DWORD numVerts = PARTICLE_BATCH_SIZE * 4;

		CParticleVertex *pVerts = m_BatchVerts;

	    //Stamp the vertices with data that won't change anymore this frame.
	    for (DWORD j=0; j < (numVerts >> 2); ++j)
	    {
	        pVerts[0].m_fU = 0.0f;
	        pVerts[0].m_fV = 0.0f;

	        pVerts[1].m_fU = 1.0f;
	        pVerts[1].m_fV = 0.0f;

			pVerts[2].m_fU = 1.0f;
			pVerts[2].m_fV = 1.0f;

			pVerts[3].m_fU = 0.0f;
			pVerts[3].m_fV = 1.0f;

			pVerts += 4;
	    }
	}

public:

	LTVector				m_Vec;
	LTVector				m_Normal;
	uint32					m_nColor;
	float					m_fU;
	float					m_fV;

	static CParticleVertex 	m_BatchVerts[PARTICLE_BATCH_SIZE * 4];
};


// static batch vertices
CParticleVertex CParticleVertex::m_BatchVerts[PARTICLE_BATCH_SIZE * 4];



// ---------------------------------------------------------------- //
// Internal functions.
// ---------------------------------------------------------------- //

//This function will return an index list of size NUM_PARTICLE_INDICES which
//is guaranteed to be initialized with the appropriate index list for the particles
static uint16* GetParticleIndices()
{
	static uint16 sParticleIndices[NUM_PARTICLE_INDICES];
	static bool   sbParticleIndicesInitted = false;


	// Init our particle indices table.
	if(!sbParticleIndicesInitted)
	{
		sbParticleIndicesInitted = true;

		// (6 indices per particle).
		uint16 curIndex = 0;
		uint16 *pCurIndex = sParticleIndices;

		for(uint32 i = 0; i < PARTICLE_BATCH_SIZE; i++)
		{
			*pCurIndex++ = curIndex++;
			*pCurIndex++ = curIndex++;
			*pCurIndex++ = curIndex++;

			*pCurIndex++ = curIndex - 3;
			*pCurIndex++ = curIndex - 1;
			*pCurIndex++ = curIndex;
			++curIndex;
		}
	}

	return sParticleIndices;
}



//Given a floating point angle, it will map it into the table of rotated vectors
//and return the index
static inline int MapAngleToRotTable(float fAngle)
{
	//0 maps to 0, 2*PI maps to ROT_TABLE_SIZE, so lets do that caculation
	int nIndex = RoundFloatToInt(fAngle * (ROT_TABLE_SIZE / (2 * PI)));

	//wrap it around
	if(nIndex >= 0)
		return nIndex % ROT_TABLE_SIZE;

	return ROT_TABLE_SIZE - ((-nIndex) % ROT_TABLE_SIZE);
}



//Given a particle as well as a vector with appropriate color scales for RGB in XYZ
//it will create a color packed into a 32 bit value
static inline uint32 CalcParticleColor(const PSParticle* pParticle, const LTVector& vColorScale, float fAlphaScale)
{
	return  (uint32)(RoundFloatToInt(pParticle->m_Alpha   * fAlphaScale)   << 24) |
			(uint32)(RoundFloatToInt(pParticle->m_Color.x * vColorScale.x) << 16) |
			(uint32)(RoundFloatToInt(pParticle->m_Color.y * vColorScale.y) << 8) |
			(uint32)(RoundFloatToInt(pParticle->m_Color.z * vColorScale.z));
}



//Given a particle as well as a vector with appropriate color scales for RGB in XYZ
//it will create a color packed into a 32 bit value
static inline uint32 CalcParticleColor(const PSParticle* pParticle)
{
	return  (uint32)(RoundFloatToInt(pParticle->m_Alpha  ) << 24) |
			(uint32)(RoundFloatToInt(pParticle->m_Color.x) << 16) |
			(uint32)(RoundFloatToInt(pParticle->m_Color.y) << 8) |
			(uint32)(RoundFloatToInt(pParticle->m_Color.z));
}



static PSParticle* d3d_DrawParticleBatch(LTParticleSystem *pSystem, PSParticle *pParticles, int nParticles)
{
	//make sure that the number of particles we are drawing is less than a batch,
	//and more than 0
	assert(nParticles > 0);
	assert(nParticles <= PARTICLE_BATCH_SIZE);

	float fScale = 1.0f / 255.0f;
	LTVector vColorScale;
	vColorScale.x = fScale * (float)pSystem->m_ColorR;
	vColorScale.y = fScale * (float)pSystem->m_ColorG;
	vColorScale.z = fScale * (float)pSystem->m_ColorB;

	float alphaScale = pSystem->m_ColorA;

	PSParticle		*pCurIn = pParticles;
	CParticleVertex *pCurVert = CParticleVertex::m_BatchVerts;
	uint32			nShade;

	//see if we need to do rotations on these particles
	if(pSystem->m_psFlags & PS_USEROTATION)
	{
		//we need to consider the rotation of the particles (a bit slower)
		int nMapIndex;

		do
		{
			//Cache some stuff to avoid dereferences
			LTVector&	vPos = pCurIn->m_Pos;
			float&		fSize = pCurIn->m_Size;
			nMapIndex	= MapAngleToRotTable(pCurIn->m_fAngle);

			//Figure out the shade of this particle.
            nShade = CalcParticleColor(pCurIn, vColorScale, alphaScale);

			// Top-left.
			pCurVert[0].SetupVert(vPos, g_vRotOffset[nMapIndex][UpperLeft], fSize, nShade);

			// Top-right.
			pCurVert[1].SetupVert(vPos, g_vRotOffset[nMapIndex][UpperRight], fSize, nShade);

			// Bottom-right.
			pCurVert[2].SetupVert(vPos, g_vRotOffset[nMapIndex][BottomRight], fSize, nShade);

			// Bottom-left.
			pCurVert[3].SetupVert(vPos, g_vRotOffset[nMapIndex][BottomLeft], fSize, nShade);

			pCurVert += 4;

			--nParticles;
			pCurIn = pCurIn->m_pNext;
		}
		while(nParticles);
	}
	else
	{
		//we can ignore the rotation of the particles and take the slightly
		//faster approach
		do
		{
			//Cache some stuff to avoid dereferences
			LTVector&	vPos = pCurIn->m_Pos;
			float&		fSize = pCurIn->m_Size;

			//Figure out the shade of this particle.
			nShade = CalcParticleColor(pCurIn, vColorScale, alphaScale);

			// Top-left.
			pCurVert[0].SetupVert(vPos, g_vOffset[UpperLeft], fSize, nShade);

			// Top-right.
			pCurVert[1].SetupVert(vPos, g_vOffset[UpperRight], fSize, nShade);

			// Bottom-right.
			pCurVert[2].SetupVert(vPos, g_vOffset[BottomRight], fSize, nShade);

			// Bottom-left.
			pCurVert[3].SetupVert(vPos, g_vOffset[BottomLeft], fSize, nShade);

			pCurVert += 4;

			--nParticles;
			pCurIn = pCurIn->m_pNext;
		}
		while(nParticles);
	}

	if (pCurVert != CParticleVertex::m_BatchVerts)
	{
		LTEffectImpl* pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pSystem->m_nEffectShaderID);
		if(pEffect)
		{
			ID3DXEffect* pD3DEffect = pEffect->GetEffect();
			if(pD3DEffect)
			{
				if(pSystem->m_pCurTexture)
				{				
					RTexture* pRTexture = (RTexture*)pSystem->m_pCurTexture->m_pRenderData;
					pD3DEffect->SetTexture("texture0", pRTexture->m_pD3DTexture);
				}
				//batch it!
				int nParticlesDrawn = (pCurVert - CParticleVertex::m_BatchVerts) >> 2;

				i_client_shell->OnEffectShaderSetParams(pEffect, NULL, NULL, LTShaderDeviceStateImp::GetSingleton());

				UINT nPasses = 0;
				pD3DEffect->Begin(&nPasses, 0);

				for(UINT i = 0; i < nPasses; ++i)
				{
					pD3DEffect->BeginPass(i);
					

					D3D_CALL(PD3DDEVICE->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,
						0,
						pCurVert - CParticleVertex::m_BatchVerts,
						nParticlesDrawn * 2,
						GetParticleIndices(),
						D3DFMT_INDEX16,
						CParticleVertex::m_BatchVerts,
						sizeof(CParticleVertex)));

					pD3DEffect->EndPass();
				}

				pD3DEffect->End();
			}
		}
		else
		{
			int nParticlesDrawn = (pCurVert - CParticleVertex::m_BatchVerts) >> 2;
			D3D_CALL(PD3DDEVICE->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,
				0,
				pCurVert - CParticleVertex::m_BatchVerts,
				nParticlesDrawn * 2,
				GetParticleIndices(),
				D3DFMT_INDEX16,
				CParticleVertex::m_BatchVerts,
				sizeof(CParticleVertex)));
		}

	}

	return pCurIn;
}



static void d3d_DrawParticles(LTParticleSystem *pSystem)
{
	int nBatches	= pSystem->m_nParticles / PARTICLE_BATCH_SIZE;
	int nExtra		= pSystem->m_nParticles % PARTICLE_BATCH_SIZE;

	//setup our vertex shader for this particle system
	D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
	D3D_CALL(PD3DDEVICE->SetFVF(PARTICLEVERTEX_FORMAT));

	// No need to set the normal and the UVs for each vertex.
    // They are the same for all vertices this frame.
    CParticleVertex::SetupBaseVerts();

	// Draw all the batches and extra particles..
	PSParticle *pCurPos = pSystem->m_ParticleHead.m_pNext;
	for(int i=0; i < nBatches; i++)
	{
		pCurPos = d3d_DrawParticleBatch(pSystem, pCurPos, PARTICLE_BATCH_SIZE);
	}

	if(nExtra)
	{
		d3d_DrawParticleBatch(pSystem, pCurPos, nExtra);
	}
}



void d3d_DrawParticleSystem(const ViewParams& Params, LTParticleSystem *pParticleSystem)
{
	CountAdder cTicks_ParticleSystem(g_pSceneDesc->m_pTicks_Render_ParticleSystems);

	//add our triangles drawn
	IncFrameStat(eFS_ParticleTriangles, pParticleSystem->m_nParticles * 2);

	bool bObjectSpace = !(pParticleSystem->m_psFlags & PS_WORLDSPACE);

	// Set the texture.
	SharedTexture *pTexture = pParticleSystem->m_pCurTexture;
	if(!pTexture || !d3d_SetTexture(pTexture, 0, eFS_ParticleTexMemory))
	{
		d3d_DisableTexture(0);
	}

	//find the basis vectors the particles should use (this ensures that they are
	//perpindicular to the camera)
	LTVector vParticleUp;
	LTVector vParticleRight;

	if(pParticleSystem->m_Flags & FLAG_REALLYCLOSE)
	{
		//if we are really close, we are in camera space, so the vectors are simply
		//the basis vectors of a normal untransformed space
		vParticleUp.Init(0, 1.0f, 0);
		vParticleRight.Init(1.0f, 0, 0);
		g_vParticleNormal.Init(0, 0, -1.0f);
	}
	else
	{
		//if we aren't really close, we need to use the camera's orientation as the basis
		//for our space
		vParticleUp			= Params.m_Up;
		vParticleRight		= Params.m_Right;
		g_vParticleNormal	= -Params.m_Forward;
	}

	// Setup the main transform.
	LTMatrix systemTransform;

	if(bObjectSpace)
	{
		d3d_SetupTransformation(&pParticleSystem->GetPos(), (float*)&pParticleSystem->m_Rotation, &pParticleSystem->m_Scale, &systemTransform);

		//initialize the camera plane, but make sure it is in object space
		LTMatrix mInverse = systemTransform;
		mInverse.Inverse();

		//transform these vectors into object space
		mInverse.Apply3x3(vParticleUp);
		mInverse.Apply3x3(vParticleRight);
		mInverse.Apply3x3(g_vParticleNormal);
	}

	//now scale these by two to prevent needing to do this in many other locations
	//These are multiplied by 2 to compensate for old code...
	vParticleUp		*= 2.0f;
	vParticleRight	*= 2.0f;

	//create custom vectors for the particles to render.
	g_vOffset[UpperLeft]   =  vParticleUp - vParticleRight;
	g_vOffset[UpperRight]  =  vParticleUp + vParticleRight;
	g_vOffset[BottomLeft]  = -vParticleUp - vParticleRight;
	g_vOffset[BottomRight] = -vParticleUp + vParticleRight;

	//now if we are rotating, we need to calculate the rotated versions and store them in
	//the table
	if(pParticleSystem->m_psFlags & PS_USEROTATION)
	{
		//see if we can bail on caculating the vectors (can be done if the
		//previous up and right vectors were close enough to the old ones)
		//Note the 3.91, that is because the vectors are each of magnitude
		//2, so if aligned the dot product is 4.
		if(	(g_vCacheTestRotUp.Dot(vParticleUp) < 3.91f) ||
			(g_vCacheTestRotRight.Dot(vParticleRight) < 3.91f))
		{
			//they were too far apart....guess we need to calculate them.

			//update our cache tests
			g_vCacheTestRotUp = vParticleUp;
			g_vCacheTestRotRight = vParticleRight;

			//figure out the stepping we need for the angle
			float fAngleStep = (2 * PI) / (float)ROT_TABLE_SIZE;

			LTVector vBasisUp;
			LTVector vBasisRight;

			float fAngle = 0.0f;
			for(uint32 nCurrAngle = 0; nCurrAngle < ROT_TABLE_SIZE; nCurrAngle++)
			{
				//we need to calculate the vectors for each
				vBasisUp	= (float)cos(fAngle) * vParticleUp + (float)sin(fAngle) * vParticleRight;

				//The right can be computed faster through a cross product. Be careful of the size
				//though, since if that changes, this will need to be scaled
				vBasisRight = vBasisUp.Cross(g_vParticleNormal);

				//now form the four vectors as a combination of these
				g_vRotOffset[nCurrAngle][UpperLeft]   =  vBasisUp - vBasisRight;
				g_vRotOffset[nCurrAngle][UpperRight]  =  vBasisUp + vBasisRight;
				g_vRotOffset[nCurrAngle][BottomLeft]  = -vBasisUp - vBasisRight;
				g_vRotOffset[nCurrAngle][BottomRight] = -vBasisUp + vBasisRight;

				//increment our angle
				fAngle += fAngleStep;
			}
		}
	}

	//setup the lighting if applicable
	if(pParticleSystem->m_psFlags & PS_LIGHT)
	{
		PD3DDEVICE->SetRenderState(D3DRS_LIGHTING, TRUE);
		d3d_SetupTouchingLights(pParticleSystem->m_SystemCenter, pParticleSystem->m_SystemRadius);
	}

	//setup the transform for the vertices...either by setting up the world to camera
	//or the object to world
	CReallyCloseData RCData;

	if(pParticleSystem->m_Flags & FLAG_REALLYCLOSE)
	{
		//if we are really close, we are in the camera space, so we need to disable
		//the world to view transform
		d3d_SetReallyClose(&RCData);
	}

	//setup our world transform
	if(bObjectSpace)
		d3d_SetD3DMat(D3DTS_WORLD, &systemTransform);

	//don't let the texture wrap, this prevents pixels from appearring on opposite sides
	//due to filtering
	SamplerStateSet ssWrapModeU0(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	SamplerStateSet ssWrapModeV0(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	d3d_DrawParticles(pParticleSystem);

	//restore the transformation
	if(bObjectSpace)
		d3d_SetD3DMat(D3DTS_WORLD, &Params.m_mIdentity);

	if(pParticleSystem->m_psFlags & PS_LIGHT)
	{
		PD3DDEVICE->SetRenderState(D3DRS_LIGHTING, FALSE);
	}

	//restore the world to view matrix
	if(pParticleSystem->m_Flags & FLAG_REALLYCLOSE)
	{
		//if we are really close, we are in the camera space, so we need to disable
		//the world to view transform
		d3d_UnsetReallyClose(&RCData);
	}

	//try to not leak memory
	d3d_DisableTexture(0);
}



