#include "precompile.h"

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

#include "d3d_draw.h"
#include "3d_ops.h"
#include "d3d_texture.h"
#include "tagnodes.h"
#include "renderstruct.h"
#include "drawobjects.h"
#include "d3d_renderworld.h"
#include "de_mainworld.h"
#include "common_draw.h"
#include "relevantlightlist.h"
#include "rendererframestats.h"

#include "LTEffectImpl.h"
#include "lteffectshadermgr.h"
#include "ltshaderdevicestateimp.h"
#include "rendererconsolevars.h"

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IWorldSharedBSP holder
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

//IFindObj holder
#include "findobj.h"
static IFindObj *g_pIFindObj;
define_holder(IFindObj, g_pIFindObj);

// forward declarations
struct DynamicParticleVertex;
void d3d_DrawWireframeBox(const LTVector& Min, const LTVector& Max, uint32 color);
static void CreateLightList( LTObject* obj, CRelevantLightList& lightList );
static void LightDynamicParticlesNonDirectional( DynamicParticleVertex* verts, uint32 numVerts, CRelevantLightList& lightList, bool quads );


const LTVector zero(0.0f,0.0f,0.0f);


//--------------------------------
//--- VolumeEffect vertex info ---
//--------------------------------

// vertex format for dynamic particles
#define DYNAMICPARTICLE_FVF (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

struct DynamicParticleVertex
{
	float x, y, z;
	uint32 color;
	float u, v;
};

const uint32 DPVertSize = sizeof(DynamicParticleVertex);


// information needed to light a particle
struct DynamicParticleLightingData
{
	LTVector pos;			// position to sample lights at
	uint32 alpha;			// alpha is at 0xff000000
	LTVector acc;			// accumulated light color, game code should clear or initialize this (for ambient, etc.)
};


// the number of verts to hold in the vertex buffer (ideally divisible by 3 and 4)
#define DYNAMICPARTICLE_VBSIZE 6000

// the number of indexed quads that will fit in this vertex buffer
#define DYNAMICPARTICLE_NUMQUADS (DYNAMICPARTICLE_VBSIZE/4)

// the size of the index buffer needed to reference these quads
#define DYNAMICPARTICLE_QUADIBSIZE  (DYNAMICPARTICLE_NUMQUADS*6)

// the vertex buffer for dynamic particles
static IDirect3DVertexBuffer9* g_pDynamicParticleVB = NULL;

// the index buffer for dynamic particle quads
static IDirect3DIndexBuffer9* g_pDynamicParticleQuadIB = NULL;

// the lighting sample array for dynamic particles
static DynamicParticleLightingData* g_pDynamicParticleLighting = NULL;

// the next index to write to in the vertex buffer
static uint32 g_nDynamicParticleVBIndex = 0;

// the number of dynamic particles drawn this frame
static uint32 g_nDynamicParticlesDrawn = 0;



//------------------------------
//--- VolumeEffect rendering ---
//------------------------------

// draw dynamic particles effect
static void DrawDynamicParticles(const ViewParams& Params, LTVolumeEffect* pEffect )
{
	bool lighting = (pEffect->m_DPLighting != VolumeEffectInfo::kNone );
	bool renderQuads = (pEffect->m_DPPrimitive == VolumeEffectInfo::kQuadlist);
	bool saturate = (g_CV_Saturate && pEffect->m_DPSaturate);

	// initialize the vertex buffer if it doesn't exist
	if( !g_pDynamicParticleVB )
	{
		if( FAILED( PD3DDEVICE->CreateVertexBuffer( DYNAMICPARTICLE_VBSIZE*DPVertSize, D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, DYNAMICPARTICLE_FVF, D3DPOOL_DEFAULT, &g_pDynamicParticleVB ) ) )
		{
			g_pDynamicParticleVB = NULL;
			return;
		}
	}

	// initialize the index buffer if it's needed and doesn't exist
	if( renderQuads && !g_pDynamicParticleQuadIB )
	{
		if( FAILED( PD3DDEVICE->CreateIndexBuffer( DYNAMICPARTICLE_QUADIBSIZE*2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pDynamicParticleQuadIB ) ) )
		{
			g_pDynamicParticleQuadIB = NULL;
			return;
		}

		uint8* ibData;
		uint16* curIndex;
		uint16 baseVert;

		if( FAILED( g_pDynamicParticleQuadIB->Lock( 0, 0, (void **)&ibData, 0 ) ) )
		{
			g_pDynamicParticleQuadIB->Release();
			g_pDynamicParticleQuadIB = NULL;
			return;
		}

		curIndex = (uint16*)ibData;

		// fill it with quad references
		for( uint32 i = 0; i < DYNAMICPARTICLE_NUMQUADS; ++i )
		{
			baseVert = i*4;
			*(curIndex++) = baseVert;
			*(curIndex++) = baseVert+1;
			*(curIndex++) = baseVert+2;
			*(curIndex++) = baseVert+2;
			*(curIndex++) = baseVert+1;
			*(curIndex++) = baseVert+3;
		}

		g_pDynamicParticleQuadIB->Unlock();
	}

	// initialize the light sample array if it's needed and doesn't exist
	if( lighting && !g_pDynamicParticleLighting )
	{
		uint32 numLightingData = DYNAMICPARTICLE_VBSIZE / 3;
		if( !(g_pDynamicParticleLighting = new DynamicParticleLightingData[numLightingData]) )
			return;
	}

	uint8* vbData;				// pointer into the locked vertex buffer
	uint32 totalFilled = 0;		// number of tris added so far
	bool first = true;			// true if this is the first time through the loop
	bool done = false;			// true if the object is done sending us tris

	// don't render alpha 0 fragments (Saves >50% fill (and read) rate for triangle particles)
	StateSet ssAlphaRef( D3DRS_ALPHAREF, 0x00 );
	StateSet ssAlphaFunc( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
	StateSet ssAlphaTest( D3DRS_ALPHATESTENABLE, true );

	// some things change the culling mode, so make sure nothing is culled
	StateSet ssCull( D3DRS_CULLMODE, D3DCULL_NONE );

	// clamp the texture instead of wrapping
	SamplerStateSet ssWrapModeU0(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	SamplerStateSet ssWrapModeV0(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// the following are used for color saturation
	StageStateSet tss0( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	StageStateSet tss1( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	StageStateSet tss2( 0, D3DTSS_COLOROP, saturate ? D3DTOP_MODULATE2X : D3DTOP_MODULATE );
	StageStateSet tss3( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	StageStateSet tss4( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	StageStateSet tss5( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	StageStateSet tss6( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	StageStateSet tss7( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

	// grab light information for this volume if needed
	CRelevantLightList lightList;
	if( lighting )
		CreateLightList( pEffect, lightList );

	// keep filling the vertex buffer until this there are no more verts in this effect
	while( !done )
	{
		// the number of verts remaining in the vertex buffer
		uint32 vbRemainingVerts = DYNAMICPARTICLE_VBSIZE - g_nDynamicParticleVBIndex;

		if( (vbRemainingVerts < 100) || !g_nDynamicParticlesDrawn )
		{
			// if there are only a few verts left unfilled, or this is the first lock of a frame, start up a new vertex buffer
			if( FAILED( g_pDynamicParticleVB->Lock( 0, 0, (void **)&vbData, D3DLOCK_DISCARD ) ) )
				return;
			g_nDynamicParticleVBIndex = 0;
			vbRemainingVerts = DYNAMICPARTICLE_VBSIZE;
		}
		else
		{
			// otherwise grab the unused portion of the current vertex buffer
			if( FAILED( g_pDynamicParticleVB->Lock( g_nDynamicParticleVBIndex*DPVertSize, vbRemainingVerts*DPVertSize, (void **)&vbData, D3DLOCK_NOOVERWRITE ) ) )
				return;
		}

		// ask the object to fill in the triangle data
		uint32 curFilled;
		done = pEffect->m_DPUpdateFn( pEffect->m_DPUserData, vbData, g_pDynamicParticleLighting, vbRemainingVerts, totalFilled, curFilled );

		ASSERT( (!renderQuads && (curFilled % 3 == 0)) || (renderQuads && (curFilled % 4 == 0)) );

		// light the verts if needed
		if( lighting )
		{
			LightDynamicParticlesNonDirectional( (DynamicParticleVertex*)vbData, curFilled, lightList, renderQuads );
		}

		// unlock the vertex buffer
		g_pDynamicParticleVB->Unlock();

		// setup initial state if this is the first time through
		if( first )
		{
			first = false;
			if( FAILED( PD3DDEVICE->SetStreamSource( 0, g_pDynamicParticleVB, 0, DPVertSize ) ) )
				return;
			if( FAILED( PD3DDEVICE->SetVertexShader(NULL) ) )
				return;
			if( FAILED( PD3DDEVICE->SetFVF( DYNAMICPARTICLE_FVF ) ) )
				return;
			if( renderQuads )
				if( FAILED( PD3DDEVICE->SetIndices( g_pDynamicParticleQuadIB) ) )
					return;
			if( !pEffect->m_DPTexture || !d3d_SetTexture( pEffect->m_DPTexture, 0, eFS_VolumeEffectTexMemory ) )
			{
				d3d_DisableTexture( 0 );
			}
			else
			{
				//if it passed and we have an Associated Effect, we need to upload the texture to the effect
				LTEffectImpl* pEffectShader = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pEffect->m_nEffectShaderID);
				if(pEffectShader)
				{
					ID3DXEffect* pD3DEffect = pEffectShader->GetEffect();
					if(pD3DEffect)
					{
						if(pEffect->m_DPTexture)
						{				
							RTexture* pRTexture = (RTexture*)pEffect->m_DPTexture->m_pRenderData;
							pD3DEffect->SetTexture("texture0", pRTexture->m_pD3DTexture);
						}
					}
				}
			}
		}

		// render trilist
		if( !renderQuads )
		{
			uint32 numTris = curFilled/3;

			// draw the triangles

			if(!numTris)
			{
				return;
			}

			LTEffectImpl* pEffectShader = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pEffect->m_nEffectShaderID);
			if(pEffectShader)
			{
				ID3DXEffect* pD3DEffect = pEffectShader->GetEffect();
				if(pD3DEffect)
				{
					bool bFailed = false;

					i_client_shell->OnEffectShaderSetParams(pEffectShader, NULL, NULL, LTShaderDeviceStateImp::GetSingleton());

					UINT nPasses = 0;
					pD3DEffect->Begin(&nPasses, 0);

					for(UINT i = 0; i < nPasses; ++i)
					{
						pD3DEffect->BeginPass(i);

						if(FAILED( PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLELIST, g_nDynamicParticleVBIndex, numTris ) ))
						{
							bFailed = true;
						}

						pD3DEffect->EndPass();
					}

					pD3DEffect->End();

					if(bFailed)
					{
						return;
					}
				}
			}	
			else
			{	if(FAILED( PD3DDEVICE->DrawPrimitive( D3DPT_TRIANGLELIST, g_nDynamicParticleVBIndex, numTris ) ))
				{
					return;
				}
			}


			IncFrameStat(eFS_VolumeEffectTriangles, numTris);
		}
		else
		{
			uint32 numTris = curFilled/2;

			// draw the quads (doubled up tris)
			if( !numTris)
			{
				return;
			}
			
			LTEffectImpl* pEffectShader = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pEffect->m_nEffectShaderID);
			if(pEffectShader)			
			{
				ID3DXEffect* pD3DEffect = pEffectShader->GetEffect();
				if(pD3DEffect)
				{
					bool bFailed = false;

					i_client_shell->OnEffectShaderSetParams(pEffectShader, NULL, NULL, LTShaderDeviceStateImp::GetSingleton());

					UINT nPasses = 0;
					pD3DEffect->Begin(&nPasses, 0);

					for(UINT i = 0; i < nPasses; ++i)
					{
						pD3DEffect->BeginPass(i);

						if(FAILED( PD3DDEVICE->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, g_nDynamicParticleVBIndex, curFilled, (g_nDynamicParticleVBIndex/4)*6, numTris ) ) )
						{
							bFailed = true;
						}

						pD3DEffect->EndPass();
					}

					pD3DEffect->End();

					if(bFailed)
					{
						return;
					}
				}
			}
			else
			{			
				if(FAILED( PD3DDEVICE->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, g_nDynamicParticleVBIndex, curFilled, (g_nDynamicParticleVBIndex/4)*6, numTris ) ) )
				{
					return;
				}
			}

			IncFrameStat(eFS_VolumeEffectTriangles, numTris);
		}

		// increment the index into the vertex buffer
		g_nDynamicParticleVBIndex += curFilled;
		totalFilled += curFilled;
	}

	// don't leak the texture
	d3d_DisableTexture(0);

	// Don't leak the VB or IB either
	PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
	PD3DDEVICE->SetIndices(0);
}


static inline LTVector CalcPointQuarticSample( const CRenderLight& light, const LTVector& pos )
{
	// get light vector
	LTVector vecToSample = pos - light.GetPos();
	float distSqr = vecToSample.MagSqr();

	// bail if outside
	if( distSqr >= light.GetRadiusSqr() )
		return zero;

	// calculate attenuation and return
	float distPercent = 1.0f - (distSqr/ light.GetRadiusSqr());
	return distPercent * distPercent * light.GetColor();
}


static inline LTVector CalcPointD3DSample( const CRenderLight& light, const LTVector& pos )
{
	// get light vector
	LTVector vecToSample = pos - light.GetPos();
	float distSqr = vecToSample.MagSqr();

	// bail if outside
	if( distSqr >= light.GetRadiusSqr() )
		return zero;

	const LTVector& coef = light.GetAttCoeff();
	return light.GetColor() / (coef.x + coef.y * (float)sqrt( distSqr ) + coef.z * distSqr);
}


static inline LTVector CalcSpotlightSample( const CRenderLight& light, const LTVector& pos )
{
	// get light vector
	LTVector vecToSample = pos - light.GetPos();
	float distSqr = vecToSample.MagSqr();

	// bail if outside
	if( distSqr >= light.GetRadiusSqr() )
		return zero;

	float dist = (float)sqrt( distSqr );
	float sampleDotLight = vecToSample.Dot( light.GetDir() ) / dist;

	if( sampleDotLight > light.GetCosFOV() )
	{
		float cosFOV = light.GetCosFOV();
		return light.GetColor() * ((sampleDotLight - cosFOV) / (1.0f - cosFOV));
	}

	// outside of cone
	return zero;
}


static void LightDynamicParticlesNonDirectional( DynamicParticleVertex* verts, uint32 numVerts, CRelevantLightList& lightList, bool quads )
{
	DynamicParticleLightingData* lightData = g_pDynamicParticleLighting;
	uint32 numLights = lightList.GetNumLights();

	uint32 numSamples = numVerts / (quads ? 4 : 3);

	uint32 i;
	LTVector color;
	uint32 argb;

	// sample the lights at the lighting points
	for( uint32 curLightNum = 0; curLightNum < numLights; ++curLightNum )
	{
		i = 0;
		const CRenderLight& curLight = lightList.GetLight( curLightNum );
		CRenderLight::ELightType type = curLight.GetType();

		if( type == CRenderLight::eLight_Point )
		{
			if( curLight.GetAttType() == eAttenuation_Quartic )
			{
				// quartic point light
				for( ; i < numSamples; ++i )
					lightData[i].acc += CalcPointQuarticSample( curLight, lightData[i].pos );
			}
			else
			{
				// assume d3d point light
				for( ; i < numSamples; ++i )
					lightData[i].acc += CalcPointD3DSample( curLight, lightData[i].pos );
			}
		}
		else if( type == CRenderLight::eLight_Spot )
		{
			const LTVector& lightPos = curLight.GetPos();
			const LTVector& col = curLight.GetColor();
			const LTVector& dir = curLight.GetDir();
			float radSqr = curLight.GetRadiusSqr();
			float cosFov = curLight.GetCosFOV();
			float invCosFov = 1.0f / (1.0f - cosFov);
			// spotlight
			for( ; i < numSamples; ++i )
				lightData[i].acc += CalcSpotlightSample( curLight, lightData[i].pos);
		}
		else
		{
			// assume dir light, just give all points the lights color
			const LTVector& color = curLight.GetColor();
			for( ; i < numSamples; ++i )
				lightData[i].acc += color;
		}
	}

	if( quads )
	{
		for( i = 0; i < numSamples; ++i, ++lightData )
		{
			color = lightData->acc;
			if( color.x > 255.0f ) color.x = 255.0f;
			if( color.y > 255.0f ) color.y = 255.0f;
			if( color.z > 255.0f ) color.z = 255.0f;

			argb = lightData->alpha | ((uint32)(color.x)<<16) | ((uint32)(color.y)<<8) | (uint32)(color.z);

			verts->color = argb;
			++verts;
			verts->color = argb;
			++verts;
			verts->color = argb;
			++verts;
			verts->color = argb;
			++verts;
		}
	}
	else
	{
		for( i = 0; i < numSamples; ++i, ++lightData )
		{
			color = lightData->acc;
			if( color.x > 255.0f ) color.x = 255.0f;
			if( color.y > 255.0f ) color.y = 255.0f;
			if( color.z > 255.0f ) color.z = 255.0f;

			argb = lightData->alpha | ((uint32)(color.x)<<16) | ((uint32)(color.y)<<8) | (uint32)(color.z);

			verts->color = argb;
			++verts;
			verts->color = argb;
			++verts;
			verts->color = argb;
			++verts;
		}
	}
}


// static light setup callback
static void StaticLightCallback( WorldTreeObj* obj, void* user )
{
	ASSERT( obj->GetObjType() == WTObj_Light );

	CRelevantLightList* lightList = (CRelevantLightList*)user;
	StaticLight* light = (StaticLight*)obj;

	// determine light color
	LTVector col( light->m_Color );
	if( light->m_pLightGroupColor )
		col = col * *(light->m_pLightGroupColor);

	// early out if light isn't going to contribute anything of value
	if( (col.x < 1.0f) && (col.y < 1.0f) && (col.z < 1.0f) )
		return;

	CRenderLight renderLight;

	if( light->m_FOV > -0.99f )
		renderLight.SetupSpotLight( light->m_Pos, light->m_Dir, col, light->m_AttCoefs, light->m_Radius, light->m_FOV, light->m_eAttenuation, light->m_Flags );
	else
		renderLight.SetupPointLight( light->m_Pos, col, light->m_AttCoefs, light->m_Radius, light->m_eAttenuation, light->m_Flags );

	lightList->InsertLight( renderLight, light->m_fConvertToAmbient );
}


// setup the relevant light list
static void CreateLightList( LTObject* obj, CRelevantLightList& lightList )
{
	// TODO: console variable
	uint32 maxLights = LTMIN( 100, MAX_LIGHTS_SUPPORTED_BY_D3D );

	LTVector objPos = obj->GetPos();
	LTVector objDims = obj->GetDims();

	lightList.ClearList();
	lightList.SetMaxLights( maxLights );
	lightList.SetObjectPos( objPos );
	lightList.SetObjectDims( objDims );

	CRenderLight renderLight;

	// ambient lighting
	if( g_have_world )
	{
		LTRGBColor col;
		w_DoLightLookup( world_bsp_shared->LightTable(), &objPos, &col );

		LTVector ambientLight( col.rgb.r, col.rgb.g, col.rgb.b );

		lightList.AddAmbient( ambientLight );
	}

	// TODO: static sunlight

	// dynamic lights
	for( uint32 i = 0; i < g_nNumObjectDynamicLights; i++ )
	{
		DynamicLight* srcLight = g_ObjectDynamicLights[i];

		if( srcLight->m_Flags & FLAG_ONLYLIGHTWORLD )
			continue;

		LTVector color( (float)srcLight->m_ColorR, (float)srcLight->m_ColorG, (float)srcLight->m_ColorB );
		LTVector attCoef( 1.0f, 0.0f, 19.0f / (srcLight->m_LightRadius * srcLight->m_LightRadius) );
		renderLight.SetupPointLight( srcLight->GetPos(), color, attCoef, srcLight->m_LightRadius, eAttenuation_D3D, 0 );
		lightList.InsertLight( renderLight, 0.0f );
	}

	// static lights
	if( g_have_world )
	{
		FindObjInfo findInfo;
		findInfo.m_iObjArray = NOA_Lights;
		findInfo.m_Min = objPos - objDims;
		findInfo.m_Max = objPos + objDims;
		findInfo.m_CB = &StaticLightCallback;
		findInfo.m_pCBUser = &lightList;

		world_bsp_client->ClientTree()->FindObjectsInBox2( &findInfo );
	}
}


// actually render a volume effect
static void TestAndDrawVolumeEffect(const ViewParams& Params, LTObject* pObject )
{
	LTVolumeEffect* pEffect = (LTVolumeEffect*)pObject;

	// draw debug info
	if( g_CV_DrawVolumeEffectVolumes )
	{
		d3d_DisableTexture(0);
		d3d_DrawWireframeBox(pObject->GetBBoxMin(), pObject->GetBBoxMax(), 0xFFFFFFFF );
	}

	// draw the specific volume effect type
	switch( pEffect->m_EffectType )
	{
	case VolumeEffectInfo::kDynamicParticles:
		DrawDynamicParticles( Params, pEffect );
		break;
	}
}


//------------------------------------------------------
//--- VolumeEffect functions called from drawobjects ---
//------------------------------------------------------

// the renderer is starting up
void d3d_InitVolumeEffectDraw( void)
{
}


// the renderer is shutting down (device lost, etc.)
void d3d_TermVolumeEffectDraw( void )
{
	// free the dynamic particle vertex buffer
	if( g_pDynamicParticleVB )
	{
		g_pDynamicParticleVB->Release();
		g_pDynamicParticleVB = NULL;
		g_nDynamicParticleVBIndex = 0;
	}

	if( g_pDynamicParticleQuadIB )
	{
		g_pDynamicParticleQuadIB->Release();
		g_pDynamicParticleQuadIB = NULL;
	}

	if( g_pDynamicParticleLighting )
	{
		delete [] g_pDynamicParticleLighting;
		g_pDynamicParticleLighting = NULL;
	}
}


// called before each frame
void d3d_VolumeEffectPreFrame( void )
{
	// point to the start of the vertex buffer
	g_nDynamicParticleVBIndex = 0;
	g_nDynamicParticlesDrawn = 0;
}


// queue the volume effect for rendering
void d3d_ProcessVolumeEffect( LTObject* pObject )
{
	if( !g_CV_DrawVolumeEffects )
		return;

	if(pObject->m_Flags2 & FLAG2_FORCETRANSLUCENT)
	{
		d3d_GetVisibleSet()->m_TranslucentVolumeEffects.Add( pObject );
	}
	else
	{
		d3d_GetVisibleSet()->m_SolidVolumeEffects.Add( pObject );
	}
}


void d3d_QueueTranslucentVolumeEffects(const ViewParams& Params, ObjectDrawList& DrawList)
{
	VERIFY_TRANSLUCENTOBJECTSTATES();

	d3d_GetVisibleSet()->m_TranslucentVolumeEffects.Queue( DrawList, Params, TestAndDrawVolumeEffect );
}


void d3d_DrawSolidVolumeEffects(const ViewParams& Params)
{
	d3d_GetVisibleSet()->m_SolidVolumeEffects.Draw(Params, TestAndDrawVolumeEffect);
}
