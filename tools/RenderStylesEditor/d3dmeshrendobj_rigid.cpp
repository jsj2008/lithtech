// d3dmeshrendobj_rigid.cpp

#include "stdafx.h"

#include "d3dmeshrendobj_rigid.h"
#include "d3d_device.h"
#include "d3d_renderstatemgr.h"
#include "d3d_effectmgr.h"
#include "ltb.h"

CD3DRigidMesh::CD3DRigidMesh()
{
	Reset();
}

CD3DRigidMesh::~CD3DRigidMesh()
{
	FreeAll();
}

void CD3DRigidMesh::Reset()
{
	m_VBController.Reset();
	m_iMaxBonesPerVert	= 0;
	m_iMaxBonesPerTri	= 0;
	m_iVertCount		= 0;
	m_iPolyCount		= 0;
}

void CD3DRigidMesh::FreeAll()
{
	m_VBController.FreeAll();

	Reset();
}

bool CD3DRigidMesh::Load(uint8* pSrcData)
{
	// Note: This is a renderobject file...
	LTB_Header Header; uint32 iSizeOf; uint32 iPieceType;
	memcpy(&Header,pSrcData,sizeof(Header));											pSrcData += sizeof(Header);
	memcpy(&iPieceType,pSrcData,sizeof(iPieceType));									pSrcData += sizeof(iPieceType);
	memcpy(&iSizeOf,pSrcData,sizeof(iSizeOf));											pSrcData += sizeof(iSizeOf);

	// Read in the basics...
	memcpy(&m_iVertCount,pSrcData,sizeof(m_iVertCount));								pSrcData += sizeof(m_iVertCount);
	memcpy(&m_iPolyCount,pSrcData,sizeof(m_iPolyCount));								pSrcData += sizeof(m_iPolyCount);
	memcpy(&m_iMaxBonesPerTri,pSrcData,sizeof(m_iMaxBonesPerTri));						pSrcData += sizeof(m_iMaxBonesPerTri);
	memcpy(&m_iMaxBonesPerVert,pSrcData,sizeof(m_iMaxBonesPerVert));					pSrcData += sizeof(m_iMaxBonesPerVert);
	memcpy(&m_VertStreamFlags[0],pSrcData,sizeof(uint32)*4);							pSrcData += sizeof(uint32)*4;
	memcpy(&m_iBoneEffector,pSrcData,sizeof(m_iBoneEffector));							pSrcData += sizeof(m_iBoneEffector);

	// Create our VB...
	for (uint32 i=0;i<4;++i) 
	{
		if (!m_VertStreamFlags[i]) continue;

		if (!m_VBController.CreateStream(i,m_iVertCount,m_VertStreamFlags[i],VertexBufferController::eNO_WORLD_BLENDS,false,true)) 
		{ 
			FreeAll(); 
			return false; 
		} 
	}

	if (!m_VBController.CreateIndexBuffer(m_iPolyCount*3)) { FreeAll(); return false; }

	// Read in our Verts...
	for (i=0;i<4;++i) 
	{
		if (!m_VertStreamFlags[i]) continue;
		m_VBController.Lock((VertexBufferController::VB_TYPE)(VertexBufferController::eVERTSTREAM0 + i),false); //true);
		uint8* pVertData = (uint8*)m_VBController.getVertexData(i);
		memcpy(pVertData,pSrcData,m_VBController.getVertexSize(i) * m_iVertCount);		pSrcData += m_VBController.getVertexSize(i) * m_iVertCount;
		m_VBController.UnLock((VertexBufferController::VB_TYPE)(VertexBufferController::eVERTSTREAM0 + i)); 
	}

	// Read in pIndexList...
	m_VBController.Lock(VertexBufferController::eINDEX, false); //true);
	memcpy(m_VBController.getIndexData(),pSrcData,sizeof(uint16) * m_iPolyCount * 3);	pSrcData += sizeof(uint16) * m_iPolyCount * 3;
	m_VBController.UnLock(VertexBufferController::eINDEX);

	return true; 
}

// NOTE: The texture list needs to change to be device independent...
void CD3DRigidMesh::Render(D3DMATRIX& WorldTransform,CD3DRenderStyle* pRenderStyle,vector<LPDXTexture>& TextureList)
{
	if (pRenderStyle == NULL)  return;

	g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0), &WorldTransform);
	PD3DDEVICE->SetRenderState(D3DRS_VERTEXBLEND,D3DVBF_DISABLE);
	PD3DDEVICE->SetRenderState(D3DRS_LIGHTING,TRUE);
	PD3DDEVICE->SetRenderState(D3DRS_NORMALIZENORMALS,FALSE);

	//m_VBController.SetStreamSources();

	if( (g_EffectMgr.GetEnabled()) && (g_EffectMgr.GetEffect() != NULL) )
	{
		ID3DXEffect *pEffect = g_EffectMgr.GetEffect();

		g_EffectMgr.UploadVertexDecl();
		m_VBController.SetStreamSources();

		// This is a temp HACK
		//g_RenderStateMgr.SetRenderStyleStates(pRenderStyle,0,TextureList);

		pEffect->SetTechnique(g_EffectMgr.GetTechnique());

		D3DXMATRIX matD3DX( *g_RenderStateMgr.GetTransform(D3DTS_WORLD));
		D3DXMATRIX MatWorld;
		D3DXMatrixTranspose(&MatWorld, &matD3DX);
		pEffect->SetMatrix("worldMatrix", &MatWorld);

		//view proj
		D3DXMATRIX MatView;
		PD3DDEVICE->GetTransform(D3DTS_VIEW, &MatView);

		pEffect->SetMatrix("viewMatrix", &MatView);

		D3DXMATRIX MatProj;
		PD3DDEVICE->GetTransform(D3DTS_PROJECTION, &MatProj);

		pEffect->SetMatrix("projMatrix", &MatProj);

		D3DXMATRIX MatViewProj;
		D3DXMatrixMultiply(&MatViewProj, &MatView, &MatProj);
		D3DXMatrixTranspose(&MatViewProj, &MatViewProj);

		D3DXMATRIX matD3DX2( MatViewProj);
		pEffect->SetMatrix("viewProjMatrix", &matD3DX2);

		float fTemp[4];
		fTemp[0] = g_EffectMgr.GetPosition()->x;
		fTemp[1] = g_EffectMgr.GetPosition()->y;
		fTemp[2] = g_EffectMgr.GetPosition()->z;
		fTemp[3] = 0.0f;
		pEffect->SetFloatArray("CameraPosition", fTemp, 4);

		char szBuf[32];
		ZeroMemory(szBuf, 32);

		for (uint32 iTexStage = 0; iTexStage < 4; ++iTexStage)
		{
			if(TextureList[iTexStage])
			{			
				PD3DDEVICE->SetTexture(iTexStage, TextureList[iTexStage]->GetTexture());

				sprintf(szBuf, "texture%d", iTexStage);
				D3DXHANDLE hHandle = pEffect->GetParameterByName(NULL, szBuf);

				HRESULT hrTex = pEffect->SetTexture(hHandle, TextureList[iTexStage]->GetTexture());
				if(FAILED(hrTex))
				{
					OutputDebugString("Error sending texture to Effect!");
				}
			}
		}

		UINT nPasses;
		pEffect->Begin(&nPasses, 0);

		for(int n = 0; n < (int)nPasses; ++n)
		{
			pEffect->BeginPass(n);
			m_VBController.Render(0,0,m_iVertCount,m_iPolyCount); 
			pEffect->EndPass();
		}

		pEffect->End();

		PD3DDEVICE->SetTexture(0, NULL);
		PD3DDEVICE->SetTexture(1, NULL);
		PD3DDEVICE->SetTexture(2, NULL);
		PD3DDEVICE->SetTexture(3, NULL);
	}
	else
	{
		g_EffectMgr.UploadVertexDecl();
		m_VBController.SetStreamSources();
	
		for (uint32 iRenderPass = 0; iRenderPass < pRenderStyle->GetRenderPassCount(); ++iRenderPass) 
		{
			LPDIRECT3DVERTEXSHADER9 VertexShader = pRenderStyle->GetVertexShader(iRenderPass,0);
			
			RSD3DRenderPass D3DRenderPass;

			if (VertexShader) 
			{																		// Using a custom vertex shader...
				if (FAILED(g_RenderStateMgr.SetVertexShader(VertexShader)))							{ assert(0); return; }
				if (!pRenderStyle->GetRenderPass_D3DOptions(iRenderPass,&D3DRenderPass))			{ assert(0); return; }
	//			if (!g_RenderStateMgr.SetVertexShaderConstants(pRenderStyle, &D3DRenderPass,0))		{ assert(0); return; } 
			}
			else 
			{	
				
				// Using the standand pipe...
				/*
				if (!m_VBController.getVertexFormat(0))	return;										// This is a non fixed function pipe VB - bail out...
				if (FAILED(g_RenderStateMgr.SetFVF(m_VBController.getVertexFormat(0))))	
				{ 
					assert(0); 
					return; 
				} 
				*/
				
			}

			g_RenderStateMgr.SetRenderStyleStates(pRenderStyle,iRenderPass,TextureList);			// Set your render states with the render state mgr...

			//g_EffectMgr.UploadVertexDecl();
			m_VBController.Render(0,0,m_iVertCount,m_iPolyCount); 
		}
	}

	D3DXMATRIX mIdentity; D3DXMatrixIdentity(&mIdentity);										// Reset our World Matrix...
	g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0),&mIdentity);
	PD3DDEVICE->SetRenderState(D3DRS_LIGHTING,FALSE);
	PD3DDEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	PD3DDEVICE->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
	PD3DDEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); 
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
}
