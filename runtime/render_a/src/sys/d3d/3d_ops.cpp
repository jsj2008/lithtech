
#include "precompile.h"
#include "3d_ops.h"
#include "d3d_draw.h"
#include "common_draw.h"
#include "common_init.h"
#include "fixedpoint.h"
#include "d3d_texture.h"

// Make sure the right intel compiler is being used.
#ifdef _USE_INTEL_COMPILER
	#if __ICL < 400
		#error This build requires the Intel Compiler version 4.0 or higher
	#endif
#endif

void d3d_CalcLightAdd(LTObject *pObject, LTVector *pLightAdd)
{
	uint32 i;
    float distSquared, percent;
	DynamicLight *pLight;


	pLightAdd->Init();

	for(i=0; i < g_nNumObjectDynamicLights; i++)
	{
		pLight = g_ObjectDynamicLights[i];

		if ((pLight->m_Flags & FLAG_ONLYLIGHTWORLD) != 0)
			continue;

		distSquared = pLight->GetPos().DistSqr(pObject->GetPos());

		if(distSquared < (pLight->m_LightRadius*pLight->m_LightRadius))
		{
			// Add some light.
			percent = 1.0f - ((float)sqrt(distSquared) / pLight->m_LightRadius);
			percent *= 0.7f; // Tone down a little..

			pLightAdd->x += (float)((long)pLight->m_ColorR - (255 - (long)pLight->m_ColorR)) * percent;
			pLightAdd->y += (float)((long)pLight->m_ColorG - (255 - (long)pLight->m_ColorG)) * percent;
			pLightAdd->z += (float)((long)pLight->m_ColorB - (255 - (long)pLight->m_ColorB)) * percent;
		}
	}
}

// Setup the renderer for really close drawing
void d3d_SetReallyClose(CReallyCloseData* pData)
{
	assert(pData);

	D3DMATRIX mIdentity = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	//preserve our old viewport
	PD3DDEVICE->GetViewport(&pData->m_OldViewport);

	// view port min-max Z change for pv model.
	D3DVIEWPORT9 ViewportData;
	ViewportData.X		= pData->m_OldViewport.X;
	ViewportData.Y		= pData->m_OldViewport.Y;
	ViewportData.Width	= pData->m_OldViewport.Width;
	ViewportData.Height = pData->m_OldViewport.Height;
	ViewportData.MinZ	= 0;
	ViewportData.MaxZ	= 0.1f;


	PD3DDEVICE->SetViewport(&ViewportData);

	float aspect = g_ViewParams.m_fScreenWidth / g_ViewParams.m_fScreenHeight;

	// Setup the projection transform by using the power of D3D.
	D3DXMATRIX NewProj;

	D3DXMatrixPerspectiveFovLH(&NewProj,
							   g_CV_PVModelFOV.m_Val * 0.01745329251994f, // convert degrees to rad on the fly.
							   aspect,
							   g_CV_ModelNear.m_Val,
							   g_CV_ModelFar.m_Val);

	//save the old transforms
	PD3DDEVICE->GetTransform(D3DTS_PROJECTION, &pData->m_OldProj);
	PD3DDEVICE->GetTransform(D3DTS_VIEW, &pData->m_OldView);

	//setup the new matrices
	PD3DDEVICE->SetTransform(D3DTS_PROJECTION, &NewProj);
	PD3DDEVICE->SetTransform(D3DTS_VIEW, &mIdentity);
}

//unsets the renderer from really close rendering
void d3d_UnsetReallyClose(CReallyCloseData* pData)
{
	assert(pData);

	PD3DDEVICE->SetViewport(&pData->m_OldViewport);

	PD3DDEVICE->SetTransform(D3DTS_PROJECTION, &pData->m_OldProj);
	PD3DDEVICE->SetTransform(D3DTS_VIEW, &pData->m_OldView );
}

//Given a stage to install it on, it will grab the global world envmap transform
//and apply it into the texture transform on that stage
void d3d_SetEnvMapTransform(RTexture* pEnvMap, uint32 nStage)
{
	//determine how many times the texture will be tiling.

	float fScale = g_CV_EnvScale;

	//now see if it is okay to divide by it, since that will provide proper scaling
	if(fabs(fScale) > 0.001f)
		fScale = -0.5f / fScale;

	//now apply our custom scale
	LTMatrix mScale;

	if(pEnvMap->IsCubeMap())
	{
		PD3DDEVICE->SetTextureStageState(nStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
		mScale = g_ViewParams.m_mWorldEnvMap;
	}
	else
	{
		PD3DDEVICE->SetTextureStageState(nStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		mScale.Init(fScale,	0.0f,	0.0f, 0.5f,
					0.0f,   fScale, 0.0f, 0.5f,
					0.0f,	0.0f,   1.0f, 0.0f,
					0.0f,   0.0f,   0.0f, 1.0f);

		//now multiply the two together to get our result
		mScale = mScale * g_ViewParams.m_mWorldEnvMap;
	}

	LTMatrix& m = mScale;

	//now setup the D3D version of our matrix
	D3DMATRIX mat;
	mat._11 = m.m[0][0]; mat._12 = m.m[1][0]; mat._13 = m.m[2][0]; mat._14 = m.m[3][0];
	mat._21 = m.m[0][1]; mat._22 = m.m[1][1]; mat._23 = m.m[2][1]; mat._24 = m.m[3][1];
	mat._31 = m.m[0][2]; mat._32 = m.m[1][2]; mat._33 = m.m[2][2]; mat._34 = m.m[3][2];
	mat._41 = m.m[0][3]; mat._42 = m.m[1][3]; mat._43 = m.m[2][3]; mat._44 = m.m[3][3];

	//and install the transform
	PD3DDEVICE->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + nStage), &mat);
	PD3DDEVICE->SetTextureStageState(nStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR | nStage);

}

//Given a stage, this will restore the parameters appropriately so as not to mess up any other sections
void d3d_UnsetEnvMapTransform(uint32 nStage)
{
	PD3DDEVICE->SetTextureStageState(nStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | nStage);
	PD3DDEVICE->SetTextureStageState(nStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
}



//given a stage, it will set up the bump transform appropriately
void d3d_SetBumpMapTransform(uint32 nStage, float fTextureScale)
{
	//setup the bump map transform
	float fScale	= g_CV_EnvBumpMapBumpScale * fTextureScale;

	float fOne		= 1.0f;
	float fZero		= 0.0f;

	PD3DDEVICE->SetTextureStageState( nStage, D3DTSS_BUMPENVMAT00, *((DWORD*)&fScale));
	PD3DDEVICE->SetTextureStageState( nStage, D3DTSS_BUMPENVMAT01, *((DWORD*)&fZero));
	PD3DDEVICE->SetTextureStageState( nStage, D3DTSS_BUMPENVMAT10, *((DWORD*)&fZero));
	PD3DDEVICE->SetTextureStageState( nStage, D3DTSS_BUMPENVMAT11, *((DWORD*)&fScale));

	PD3DDEVICE->SetTextureStageState( nStage, D3DTSS_BUMPENVLSCALE, *((DWORD*)&fOne));
	PD3DDEVICE->SetTextureStageState( nStage, D3DTSS_BUMPENVLOFFSET, *((DWORD*)&fZero));
}


//  Convert each vector member (-1.0 to 1.0) to (0 to 255) and saves the result in a DWORD. 
DWORD d3d_VectorToRGB(LTVector * vector)
{
	DWORD dwR = (DWORD)(127 * vector->x + 128);
	DWORD dwG = (DWORD)(127 * vector->y + 128);
	DWORD dwB = (DWORD)(127 * vector->z + 128);

return (DWORD)(0xff000000 + (dwR << 16) + (dwG << 8) + dwB);
}
