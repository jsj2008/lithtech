
//disable STL warnings...
#pragma warning(disable : 4786)

#include "precompile.h"
#include "d3d_device.h"
#include "modelshadowshader.h"
#include "common_draw.h"
#include "setupmodel.h"
#include "d3d_texture.h"
#include "rendererframestats.h"
#include "..\shadows\d3dshadowtexture.h"			//for the optional ALPHAREF_SHADOWS compile define

//----------------------------------------------------------
// Singleton implementation
//----------------------------------------------------------

static CModelShadowShader g_ModelShadowSingleton;

//access for the singleton object
CModelShadowShader* CModelShadowShader::GetSingleton()
{
	return &g_ModelShadowSingleton;
}

//----------------------------------------------------------
// CModelShadowShader
//----------------------------------------------------------


CModelShadowShader::CModelShadowShader() :
	m_bRendering(false),
	m_bErrorOccurred(false),
	m_pShadowTexture(NULL),
	m_bBound(false),
	m_pTempIB(NULL),
	m_nTempIBIndex(0),
	m_bPerspective(false),
	m_fProjDist(0.0f),
	m_pFadeTexture(NULL),
	m_nFadeTextureX(0),
	m_bValidatedPerspective(false),
	m_bFailedValidation(false)
{
}

CModelShadowShader::~CModelShadowShader()
{
}

//Initialization call. This is called before any calls are made to this shader, this
//allows it to perform operations such as initialization of vertex shaders, and other
//states that are constant. If it returns false, nothing should call render...
bool CModelShadowShader::BeginRendering()
{
	//setup our render states
	if(!SetRenderStates())
		return false;
	
	//used for sanity checks
	m_bRendering		= true;
	//clear our error state
	m_bErrorOccurred	= false;

	return true;
}

//called when rendering is completed. This should return false if any errors occurred
//while rendering
bool CModelShadowShader::EndRendering()
{
	RestoreRenderStates();

	return m_bErrorOccurred;
}


//Called when the world transform changes. This will be called whenever the
//rendering switches spaces and gives the shader a chance to transform any
//data it is using
bool CModelShadowShader::SetWorldTransform(const LTMatrix& mInvWorldTrans)
{
	//transform all of positions that we use for testing into the new world space
	m_vTransBSphereCenter = mInvWorldTrans * m_vBSphereCenter;

	//now we need to apply this to our internal texture transforms

	//first make the transpose version so D3D can use it
	D3DXMATRIX mD3DInvTrans ( mInvWorldTrans.m[0][0], mInvWorldTrans.m[1][0], mInvWorldTrans.m[2][0], mInvWorldTrans.m[3][0],
							  mInvWorldTrans.m[0][1], mInvWorldTrans.m[1][1], mInvWorldTrans.m[2][1], mInvWorldTrans.m[3][1],
							  mInvWorldTrans.m[0][2], mInvWorldTrans.m[1][2], mInvWorldTrans.m[2][2], mInvWorldTrans.m[3][2],
							  mInvWorldTrans.m[0][3], mInvWorldTrans.m[1][3], mInvWorldTrans.m[2][3], mInvWorldTrans.m[3][3]);

	//now apply it to the two channels
	D3DXMATRIX mResult = mD3DInvTrans * m_mWorldToProjector;
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE0, &mResult);

	mResult = mD3DInvTrans * m_mFadeTex;
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, &mResult);

	//success
	return true;
}

//called when beginning a shader. This can be used to do shader specific tasks such
//as setting up the vertex shader, the vertex buffers, etc.
bool CModelShadowShader::BeginShader(	uint32 nVertSize,				//the size of the vertices
										uint32 nVertType,				//the type of vertex as passed to D3D
										IDirect3DVertexShader9 *pShader, // for DX9 FVF is separate
										IDirect3DVertexBuffer9 *pVB,	//pointer to the vertex buffer
										IDirect3DIndexBuffer9 *pIB,		//the index buffer
										ERenderShader eShaderID			//the ID of the shader for custom setup
									 )
{	
	//don't even bother rendering if this is excluded
	if(IsExcluded(eShaderID))
		return false;

	//setup our Z bias to match the underlying shader
	// For DX9 there are two biases, which one is correct?
	// setup our Z bias to match the underlying shader
/*	if (GetZBias(eShaderID) != 0)
	{
		PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(1.0f));
	}
	else
	{
		PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(0.0f));
	}

	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(-(float)GetZBias(eShaderID)));*/

	//setup our input streams for D3D
	PD3DDEVICE->SetVertexShader(NULL);
	PD3DDEVICE->SetFVF(nVertType);

	PD3DDEVICE->SetStreamSource(0, pVB, 0, nVertSize);

	if(!g_CV_ModelShadow_Proj_BackFaceCull.m_Val)
	{
		PD3DDEVICE->SetIndices(pIB);
	}

	return true;
}




//called to render a shader block. It takes in all data needed to render the underlying
//geometry as well as the ID of the shader type so that it can perform custom rendering
//operations upon it
bool CModelShadowShader::RenderSection(	D3DPRIMITIVETYPE nPrimType,		//primitive type being rendered, D3DPT_, must be indexed
										const uint16 *pIndices,			//the original index lists
										const SRBVertex *pVertices,		//the original vertex lists
										uint32 nTotalIndexCount,		//number of primitives being rendered
										uint32 nStartVert,				//index into the vertex buffer to begin on
										uint32 nTotalVertCount,			//number of vertices
										int32 nVertOffset,				//offset of the vertex in the original vertices list
										uint32 nStartIndex				//index into the index buffer to begin on
									)
{
	//make sure that we are rendering and that there are valid inputs
	if(!m_bRendering || !pIndices || !pVertices)
	{
		m_bErrorOccurred = true;
		return false;
	}

	assert((nTotalIndexCount % 3) == 0);

	//see if we can take the quicker route and not do any backface culling
	if(!g_CV_ModelShadow_Proj_BackFaceCull.m_Val)
	{
		//we don't have to cull, just blit our block and bail
		PD3DDEVICE->DrawIndexedPrimitive(nPrimType, 0, nStartVert, nTotalVertCount, nStartIndex, nTotalIndexCount / 3);

		//update our model shadow tri count
		IncFrameStat(eFS_ModelShadowTriangles, nTotalIndexCount / 3);	

		//success on this one
		return true;
	}

	//now actually render the polygons by building up our culled index buffer list, and 
	//rendering the polygons
	uint16 *pOutIndices;

	HRESULT hr = m_pTempIB->Lock(m_nTempIBIndex * sizeof(uint16), (k_nTempIBSize - m_nTempIBIndex) * sizeof(uint16), 
					(void **)&pOutIndices, m_nTempIBIndex ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);
	
	//sanity check
	if (FAILED(hr))
	{
		ASSERT(!"Failed on model shadow lighting index buffer lock!");
		m_bErrorOccurred = true;
		return false;
	}

	//setup info to keep track of our running index list that we will fill up
	uint32 nBaseIndex = 0;

	const uint16 *pEndIndex = &pIndices[nTotalIndexCount];

	//---------------------------------
	//variables for finding backfacing

	//the polygon normal
	LTVector vPolyNormal;

	for (; pIndices < pEndIndex; pIndices += 3)
	{
		// Get the vertex positions
		const SRBVertex &v0 = pVertices[ pIndices[0] ];
		const SRBVertex &v1 = pVertices[ pIndices[1] ];
		const SRBVertex &v2 = pVertices[ pIndices[2] ];

		//find the polygon normal
		vPolyNormal = (v2.m_vPos - v0.m_vPos).Cross(v1.m_vPos - v0.m_vPos);

		float fMag = vPolyNormal.Mag();
		//vPolyNormal.Normalize();

		//bounding sphere culling
		if(fabs(vPolyNormal.Dot(m_vTransBSphereCenter - v0.m_vPos)) > m_fBSphereRad * fMag)
			continue;

		if (m_ProjectorPlane.Normal().Dot(vPolyNormal) > 0.01f)
			continue;

		// Do we need to make space, if so we can just render this block and reset
		if (m_nTempIBIndex >= k_nTempIBSize)
		{
			//allow D3D to have the index buffer again
			m_pTempIB->Unlock();

			//draw our polys
			PD3DDEVICE->SetIndices(m_pTempIB);
			PD3DDEVICE->DrawIndexedPrimitive(nPrimType, 0, nStartVert, nTotalVertCount, 0, nBaseIndex / 3);

			//update our model shadow tri count
			IncFrameStat(eFS_ModelShadowTriangles, nBaseIndex / 3);	

			//reset and reobtain our handle to the indices
			m_nTempIBIndex	= 0;
			nBaseIndex		= 0;

			m_pTempIB->Lock(0, 0, (void **)&pOutIndices, D3DLOCK_DISCARD);
		}

		// Write out the indices
		pOutIndices[0] = (uint16)(pIndices[0] + nVertOffset);
		pOutIndices[1] = (uint16)(pIndices[1] + nVertOffset);
		pOutIndices[2] = (uint16)(pIndices[2] + nVertOffset);

		// Move ahead in the output
		pOutIndices		+= 3;
		nBaseIndex		+= 3;
	}

	//release our hold on the index buffer
	m_pTempIB->Unlock();
	
	//see if we had vertices still waiting around in the list
	if (nBaseIndex != m_nTempIBIndex)
	{
		//we did, so we need to render them
		PD3DDEVICE->SetIndices(m_pTempIB);
		PD3DDEVICE->DrawIndexedPrimitive(nPrimType, 0, nStartVert, nTotalVertCount, 0, nBaseIndex / 3);

		//update our model shadow tri count
		IncFrameStat(eFS_ModelShadowTriangles, nBaseIndex / 3);	
		
		if (m_nTempIBIndex == k_nTempIBSize)
			m_nTempIBIndex = 0;
	}

	return true;
}

//determines if this device supports perspective shadows
bool CModelShadowShader::CanSupportPerspective()
{
	//see if we have already validated
	if(m_bValidatedPerspective)
	{
		return !m_bFailedValidation;
	}

	//alright, we haven't tested, so just say that we can
	return true;
}

//sets up all render states
bool CModelShadowShader::SetRenderStates()
{
	//save the appropriate states
	PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, &m_nSBFogEnable);

	//setup our blend to modulate with the frame buffer
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,  TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,			D3DBLEND_ZERO);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND,			D3DBLEND_SRCCOLOR);

	PD3DDEVICE->SetRenderState(D3DRS_ZENABLE,			D3DZB_TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE,		FALSE);
	PD3DDEVICE->SetRenderState(D3DRS_ZFUNC,				D3DCMP_EQUAL);

	PD3DDEVICE->SetRenderState(D3DRS_LIGHTING,			FALSE);
	PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE,			FALSE);
	PD3DDEVICE->SetRenderState(D3DRS_CULLMODE,			D3DCULL_CCW);
	
	//-----------------------------
	// Texture stage 0
	//-----------------------------

	//setup the texture so that it will clamp UV coordinates on our shadow texture
	PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	//setup our input parameters for the texture coordinates
	if(m_bPerspective)
	{
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 | D3DTTFF_PROJECTED);
	}
	else
	{
		PD3DDEVICE->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	}

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 0); 

	//setup our texture transform
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE0, &m_mWorldToProjector);

	//setup our base shadow texture
	d3d_SetTextureDirect(m_pShadowTexture, 0);

	//-----------------------------
	// Texture stage 1
	//-----------------------------

	//setup the texture so that it will clamp UV coordinates on our shadow texture
	PD3DDEVICE->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	PD3DDEVICE->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	//setup our input parameters for the texture coordinates
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 1); 

	//setup our texture transform
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, &m_mFadeTex);

	//setup our base shadow texture
	d3d_SetTextureDirect(m_pFadeTexture, 1);


	//--------------------
	// Texture pipe
	//--------------------
	
	//for this we just want to basically pick texture 1 modulated with the diffuse, non PMA
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);


	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	if(g_CV_ModelShadow_Proj_Fade.m_Val)
	{
		//for this we just want to add the colors together
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);


		PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		//end of the line
		PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}
	else
	{
		//end of the line
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}


#ifdef ALPHATEST_SHADOWS
	//setup the alpha ref to reject anything that isn't in the shadow, thus helping fillrate
	PD3DDEVICE->SetRenderState(D3DRS_ALPHAREF,			0);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC,			D3DCMP_NOTEQUAL);
#endif

	//we now need to handle validation of the device, since the GeForce 1 & 2 don't properly
	//handle the projected flag on the first texture transformation. So see if we can validate this,
	//if so, great, we can continue using perspective, otherwise use ortho
	if(m_bPerspective && !m_bValidatedPerspective)
	{
		DWORD nPasses;
		if(FAILED(PD3DDEVICE->ValidateDevice(&nPasses)))
		{
			m_bFailedValidation = true;
			m_bValidatedPerspective = true;
	
			//since our values will be incorrect anyway, don't render
			return false;
		}

		m_bFailedValidation = false;
		m_bValidatedPerspective = true;
	}

	return true;
}

//restores the render states that were modified
bool CModelShadowShader::RestoreRenderStates()
{

#ifdef ALPHATEST_SHADOWS
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE,  FALSE);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHAREF,			ALPHAREF_NONE);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHAFUNC,			D3DCMP_GREATEREQUAL);
#endif

	PD3DDEVICE->SetRenderState(D3DRS_CULLMODE,			D3DCULL_NONE);
	PD3DDEVICE->SetRenderState(D3DRS_ZENABLE,			D3DZB_TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_ZWRITEENABLE,		TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_ZFUNC,				D3DCMP_LESSEQUAL);
	PD3DDEVICE->SetRenderState(D3DRS_FOGENABLE,			m_nSBFogEnable);

	//restore the texture wrapping
	PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	PD3DDEVICE->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	PD3DDEVICE->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	PD3DDEVICE->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	//turn off the texture transformation
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS,  D3DTTFF_DISABLE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0); 
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,  D3DTTFF_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1); 

	//turn off Z biasing
//	PD3DDEVICE->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, F2DW(0.0f));
//	PD3DDEVICE->SetRenderState(D3DRS_DEPTHBIAS, F2DW(0.0f));

	//clear out the textures
	d3d_DisableTexture(0);
	d3d_DisableTexture(1);

	return true;
}

//determines the Z bias given the specified shader
uint32 CModelShadowShader::GetZBias(ERenderShader eShaderID) const
{
	switch(eShaderID)
	{
	case eShader_Lightmap_Texture:
	case eShader_Lightmap_Texture_Detail:
	case eShader_Lightmap_Texture_EnvMap:
	case eShader_Lightmap_Texture_EnvBumpMap:
	case eShader_Lightmap_DualTexture:
		return 1;
		break;
	default:
		//by default assume it is not alpha blended
		return 0;
		break;
	}
}

//determines if the specified shader should be excluded from being shadowed
bool CModelShadowShader::IsExcluded(ERenderShader eShaderID) const
{
	switch(eShaderID)
	{
	case eShader_Invalid:
	case eShader_Lightmap:
	case eShader_Gouraud_Effect: // Since projected shadows use the fixed function pipeline, they will not properly cast
								 // on to programmable pipeline transformed surfaces.
		return true;
		break;
	
	default:
		//by default assume it is not alpha blended
		return false;
		break;
	}

	return false;
}

//called to set up the shadow information
void CModelShadowShader::SetShadowInfo(	const ViewParams& Params,			//the view params for rendering
									    IDirect3DBaseTexture9* pTexture,	//the texture for the shadow
										const ShadowLightInfo* pInfo,		//info for the projection
										float fTexScaleX, float fTexScaleY,	//scale of the texture to the area
										bool bPerspective,					//ortho or perspective
										float fProjDist,					//the maximum distance that this shadow can project
										float fFadeOffset
									)
{
	//check the params
	assert(pTexture);
	assert(pInfo);

	//copy over our information
	m_vProjectorPos		= pInfo->m_vLightOrigin;
	m_pShadowTexture	= pTexture;
	m_fProjDist			= fProjDist;
	m_bPerspective		= bPerspective;

	//now copy over and adjust the plane
	m_ProjectorPlane	= pInfo->m_ProjectionPlane;

	// Setup the texture coordinate transform.  We get the tcoords by projecting
	// onto the plane, subtracting pInfo->m_vWindowTopLeft, and doing a dot 
	// product with the orientation vectors.

	//find our vectors in camera space since that is where the vertex positions will be located
	LTVector vCamUL			= Params.m_mView * pInfo->m_vWindowTopLeft;

	LTVector vCamRight;
	LTVector vCamUp;
	LTVector vCamForward;
	Params.m_mView.Apply3x3(pInfo->m_Vecs[0], vCamRight);
	Params.m_mView.Apply3x3(pInfo->m_Vecs[1], vCamUp);

	vCamForward = vCamUp.Cross(vCamRight);

	//create our matrix that will find the dot product along each axis
	LTVector vCamLightOrg	= Params.m_mView * pInfo->m_vLightOrigin;

	//the distance to the near clip plane
	float fNearClip = (pInfo->m_vProjectionCenter - pInfo->m_vLightOrigin).Mag();
	float fFarClip  = (fNearClip + m_fProjDist);

	//create our matrix that will scale the dot products to the appropriate UV coordinates
	float fSizeX = pInfo->m_fSizeX;
	float fSizeY = pInfo->m_fSizeY;

	//the position of the far upper right corner
	LTVector vFarURCorner;

	//build our final transformation matrix, along with the far upper left point (used for culling)
	if (m_bPerspective) 
	{
		D3DXMATRIX mDot(	vCamRight.x,					vCamUp.x,				vCamForward.x, 0.0f,
							vCamRight.y,					vCamUp.y,				vCamForward.y, 0.0f,
							vCamRight.z,					vCamUp.z,				vCamForward.z, 0.0f, 
							-vCamRight.Dot(vCamLightOrg),	-vCamUp.Dot(vCamLightOrg), -vCamForward.Dot(vCamLightOrg), 1.0f
						);


		D3DXMATRIX mProj(	fNearClip / fSizeX,		0.0f,					0.0f,		0.0f,
							0.0f,					-fNearClip / fSizeY,	0.0f,		0.0f,
							0.5f,					0.5f,					1.0f,		0.0f,
							0.0f,					0.0f,					0.0f,		1.0f
						);

		m_mWorldToProjector = mDot * mProj;
		
		//now we need to calculate the far upper right corner

		//find the near UR corner
		LTVector vNearCenter = m_ProjectorPlane.Normal() * m_ProjectorPlane.Dist();
		LTVector vNearURCorner = vNearCenter + pInfo->m_Vecs[0] * (fSizeX * 0.5f) + pInfo->m_Vecs[1] * (fSizeY * 0.5f);

		//now find the normalized direction vector to the corner
		LTVector vToCorner = vNearURCorner - pInfo->m_vLightOrigin;
		vToCorner.Normalize();

		//now find the rate of change with respect to the forward direction
		float fScale = vToCorner.Dot(m_ProjectorPlane.Normal());

		//now advance the corner along the appropriate length
		float fDist = (vNearCenter - pInfo->m_vLightOrigin).Mag() + m_fProjDist;

		vFarURCorner = vToCorner * fDist / fScale;
	}
	else
	{
		D3DXMATRIX mDot(	vCamRight.x,					vCamUp.x,				vCamForward.x, 0.0f,
							vCamRight.y,					vCamUp.y,				vCamForward.y, 0.0f,
							vCamRight.z,					vCamUp.z,				vCamForward.z, 0.0f, 
							-vCamRight.Dot(vCamUL),			-vCamUp.Dot(vCamUL),	-vCamForward.Dot(vCamLightOrg), 1.0f
						);


		D3DXMATRIX mScale(	fTexScaleX / fSizeX,				0.0f,								0.0f, 0.0f,
							0.0f,								fTexScaleY / fSizeY,				0.0f, 0.0f,
							0.0f,								0.0f,								1.0f, 0.0f, 
							( 1.0f - fTexScaleX ) * 0.5f,		( 1.0f - fTexScaleY ) * 0.5f,		0.0f, 1.0f
						);

		// Our transform is to simply do a dot product on each plane axis, and then scale it accordingly
		m_mWorldToProjector = mDot * mScale;

		//figure out the far upper left point

		//first move it to the center of the far clip
		vFarURCorner = m_ProjectorPlane.Normal() * (m_ProjectorPlane.Dist() + m_fProjDist);

		//now offset it to the upper right
		vFarURCorner += pInfo->m_Vecs[0] * (fSizeX * 0.5f) + pInfo->m_Vecs[1] * (fSizeY * 0.5f);
	}
	
	//create a bounding sphere that encompasses the shadow area for quick culling
	m_vBSphereCenter	= m_vProjectorPos + m_ProjectorPlane.Normal() * 
							(m_ProjectorPlane.DistTo(m_vProjectorPos) + m_fProjDist * 0.5f);
	m_fBSphereRad		= (m_vBSphereCenter - vFarURCorner).Mag();	

	//our transformed bounding sphere center is just our standard center
	m_vTransBSphereCenter = m_vBSphereCenter;


	//now setup our fade matrix, which is a dot product with the distance from the shadow
	//plane so that at near it is 1/TexX, and at far it is 1
	LTVector vScaledNormal = vCamForward;
	vScaledNormal.Normalize();
	vScaledNormal /= (m_fProjDist + fFadeOffset);

	//find out what percentage of the texture needs to be moved back for the offset
	float fTexelFadeOffset = fFadeOffset / (m_fProjDist + fFadeOffset);

	//offset the distance by the size amount
	LTVector vPlanePos = Params.m_mView * (m_ProjectorPlane.Normal() * m_ProjectorPlane.Dist());
	float fOffset = -vScaledNormal.Dot(vPlanePos) + 1.0f / m_nFadeTextureX + fTexelFadeOffset;

	D3DXMATRIX mFade(	vScaledNormal.x,	vCamRight.x, 0.0f, 0.0f,
						vScaledNormal.y,	vCamRight.y, 0.0f, 0.0f,
						vScaledNormal.z,	vCamRight.z, 1.0f, 0.0f, 
						fOffset,			0.0f,		 0.0f, 1.0f
					);

	m_mFadeTex = mFade;

}

//called to bind any device specific data to this object
bool CModelShadowShader::Bind()
{
	if (m_bBound)
		return true;

	// Don't try to bind again, even if we fail
	m_bBound = true;

	// Create the index buffer
	HRESULT hr = PD3DDEVICE->CreateIndexBuffer(	k_nTempIBSize * sizeof(uint16), 
												D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, 
												D3DPOOL_DEFAULT, &m_pTempIB);

	//sanity check
	if (FAILED(hr))
	{
		dsi_ConsolePrint("Unable to create model shadow index buffer.");
		return false;
	}

	ASSERT(!m_pFadeTexture);
	m_nFadeTextureX = LTMAX(8, LTMIN(256, 128));
	hr = PD3DDEVICE->CreateTexture(m_nFadeTextureX, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pFadeTexture);

	if (FAILED(hr))
	{
		dsi_ConsolePrint("Unable to create dynamic lighting Z texture.");
		Release();
		return false;
	}

	//now we need to actually fill in the fade texture
	D3DLOCKED_RECT sRect;

	m_pFadeTexture->LockRect(0, &sRect, 0, 0);

	ASSERT(SUCCEEDED(hr));
	ASSERT(sRect.pBits);
	ASSERT(sRect.Pitch);

	uint32* pCurTexel = (uint32*)sRect.pBits;

	//note that the first color MUST be white. This will cause the shadow not
	//to appear between the model and the light
	pCurTexel[0] = D3DRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	for (uint32 nZLoop = 1; nZLoop < m_nFadeTextureX; ++nZLoop)
	{
		float fValue = (float)(nZLoop - 1) / (m_nFadeTextureX - 2);
		float fFinalValue = LTMAX(0.0f, fValue * 1.2f - 0.2f);
		pCurTexel[nZLoop] = D3DRGBA(fFinalValue, fFinalValue, fFinalValue, 1.0f);
	}
	m_pFadeTexture->UnlockRect(0);

	m_nTempIBIndex = 0;

	return true;
}

//called to free any device specific data to this object
bool CModelShadowShader::Release()
{
	if (!m_bBound)
		return true;

	//free our index buffer
	if (m_pTempIB)
	{
		m_pTempIB->Release();
		m_pTempIB = NULL;
	}

	if(m_pFadeTexture)
	{
		m_pFadeTexture->Release();
		m_pFadeTexture = NULL;
	}

	m_bBound = false;

	return true;
}
