// d3dmeshrendobj_rigid.cpp

#include "precompile.h"

#include "d3dmeshrendobj_rigid.h"
#include "ltb.h"
#include "d3d_device.h"
#include "d3d_renderstatemgr.h"
#include "ltvertexshadermgr.h"
#include "ltpixelshadermgr.h"
#include "d3d_renderstyle.h"
#include "de_objects.h"
#include "ltshaderdevicestateimp.h"
#include "rendererconsolevars.h"


//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

CD3DRigidMesh::CD3DRigidMesh()
{
	Reset();
}

CD3DRigidMesh::~CD3DRigidMesh()
{
	FreeAll();
}

void CD3DRigidMesh::CalcUsedNodes( Model *pModel )
{
	CreateUsedNodeList(1);
	m_pUsedNodeList[0] = m_iBoneEffector;

}
void CD3DRigidMesh::Reset()
{
	m_VBController.Reset();
	m_iVertCount		= 0;
	m_iPolyCount		= 0;
	m_pIndexData		= NULL;

	m_bSWVertProcessing	=  ((g_Device.GetDeviceCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0) ? true : false;

	m_bSWVSBuffers = false;

	// Use software processing for shaders ? 
	if ( g_Device.GetDeviceCaps()->VertexShaderVersion < D3DVS_VERSION(1,1) )
		m_bSWVSBuffers = true;

	// in case software has been forced 
	if ( g_CV_ForceSWVertProcess )
		m_bSWVertProcessing = true;

	m_bNonFixPipeData	= false;
	for (uint32 i = 0; i < 4; ++i) m_pVertData[i] = NULL;
}

void CD3DRigidMesh::FreeAll()
{
	m_VBController.FreeAll();
	if (m_pIndexData)
	{
		delete[] m_pIndexData; m_pIndexData = NULL;
	}
	for (uint32 i = 0; i < 4; ++i)
	{
		if (m_pVertData[i])
		{
			delete[] m_pVertData[i];  m_pVertData[i] = NULL;
		}
	}

	Reset();
}

bool CD3DRigidMesh::Load(ILTStream& File, LTB_Header& LTBHeader)
{
	if (LTBHeader.m_iFileType != LTB_D3D_MODEL_FILE)
	{
		OutputDebugString("Error: Wrong file type in CD3DSkelMesh::Load\n");
		return false;
	}
	if (LTBHeader.m_iVersion  != CD3D_LTB_LOAD_VERSION)
	{
		OutputDebugString("Error: Wrong file version in CD3DSkelMesh::Load\n");
		return false;
	}

	// Read in the basics...
	uint32 iObjSize; File.Read(&iObjSize,sizeof(iObjSize));
	File.Read(&m_iVertCount,sizeof(m_iVertCount));
	File.Read(&m_iPolyCount,sizeof(m_iPolyCount)); uint32 iMaxBonesPerVert, iMaxBonesPerTri;
	File.Read(&iMaxBonesPerTri,sizeof(iMaxBonesPerTri)); assert(iMaxBonesPerTri == 1);
	File.Read(&iMaxBonesPerVert,sizeof(iMaxBonesPerVert)); assert(iMaxBonesPerVert == 1);
	File.Read(&m_VertStreamFlags[0],sizeof(uint32)*4);
	File.Read(&m_iBoneEffector,sizeof(m_iBoneEffector));

	// Read in our Verts...
	for (uint32 i=0;i<4;++i)
	{
		if (!m_VertStreamFlags[i]) continue;

		uint32 iVertexSize	= 0;									// Figure out the vertex size...
		uint32 iVertFlags	= 0;
		uint32 iUVSets		= 0;
		GetVertexFlags_and_Size(eNO_WORLD_BLENDS,m_VertStreamFlags[i],iVertFlags,iVertexSize,iUVSets,m_bNonFixPipeData);

		uint32 iSize = iVertexSize * m_iVertCount;					// Alloc the VertData...
		LT_MEM_TRACK_ALLOC(m_pVertData[i] = new uint8[iSize],LT_MEM_TYPE_RENDERER);
		File.Read(m_pVertData[i],iSize);
	}

	// Read in pIndexList...
	LT_MEM_TRACK_ALLOC(m_pIndexData = new uint8[sizeof(uint16) * m_iPolyCount * 3],LT_MEM_TYPE_RENDERER);
	File.Read(m_pIndexData,sizeof(uint16) * m_iPolyCount * 3);

	// Create the VBs and stuff...
	ReCreateObject();

	return true;
}

// Create the VBs and stuff from our sys mem copies...
void CD3DRigidMesh::ReCreateObject()
{

	// Create our VB...
	for (uint32 i=0;i<4;++i)
	{
		if (!m_VertStreamFlags[i])
			continue;
		if (!m_VBController.CreateStream(i,m_iVertCount,m_VertStreamFlags[i],eNO_WORLD_BLENDS,false,true, m_bSWVertProcessing))
		{
			FreeAll();
			return;
		}
	}
	if (!m_VBController.CreateIndexBuffer(m_iPolyCount*3,false,true, m_bSWVertProcessing))
	{
		FreeAll();
		return;
	}

	// Read in our Verts...
	for (int i = 0; i < 4; ++i)
	{
		if (!m_VertStreamFlags[i])
			continue;
		m_VBController.Lock((VertexBufferController::VB_TYPE)(VertexBufferController::eVERTSTREAM0 + i),false);
		uint8* pVertData = (uint8*)m_VBController.getVertexData(i);
		uint32 iSize = m_VBController.getVertexSize(i) * m_iVertCount;
		memcpy(pVertData,m_pVertData[i],iSize);
		m_VBController.UnLock((VertexBufferController::VB_TYPE)(VertexBufferController::eVERTSTREAM0 + i));
	}

	// Read in pIndexList...
	m_VBController.Lock(VertexBufferController::eINDEX,false);
	memcpy(m_VBController.getIndexData(),m_pIndexData,sizeof(uint16) * m_iPolyCount * 3);
	m_VBController.UnLock(VertexBufferController::eINDEX);
}

// We're loosing focus, free the stuff...
void CD3DRigidMesh::FreeDeviceObjects()
{
	m_VBController.FreeAll();									// Free our VB...
}

// NOTE: The texture list needs to change to be device independent...
void CD3DRigidMesh::Render(ModelInstance *pInstance, D3DMATRIX& WorldTransform,CD3DRenderStyle* pRenderStyle, uint32 iRenderPass)
{
	//setup our transform for this model
	g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0), &WorldTransform);

	// Set the vertex shader constants.
	if (m_pVertexShader != NULL)
	{
		// Let the client set some constants.
		if (NULL != i_client_shell)
		{
			i_client_shell->OnVertexShaderSetConstants(m_pVertexShader, iRenderPass, pRenderStyle, pInstance,
													   LTShaderDeviceStateImp::GetSingleton());
		}

		// Send the constants to the video card.
		LTVertexShaderMgr::GetSingleton().SetVertexShaderConstants(m_pVertexShader);
	}

	// Set the pixel shader constants.
	if (m_pPixelShader != NULL)
	{
		// Let the client set some constants.
		if (NULL != i_client_shell)
		{
			i_client_shell->OnPixelShaderSetConstants(m_pPixelShader, iRenderPass, pRenderStyle, pInstance,
													  LTShaderDeviceStateImp::GetSingleton());
		}

		// Send the constants to the video card.
		LTPixelShaderMgr::GetSingleton().SetPixelShaderConstants(m_pPixelShader);
	}

	//render away
	m_VBController.Render(0, 0, m_iVertCount, m_iPolyCount);
}

void CD3DRigidMesh::BeginRender(D3DMATRIX& pD3DTransforms, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass)
{
	// If this pass has a vertex shader, use it.
	RSD3DRenderPass *pPass = pRenderStyle->GetRenderPass_D3DOptions(iRenderPass);

	if (NULL != pPass &&
	    pPass->bUseVertexShader &&
		 pPass->VertexShaderID != LTVertexShader::VERTEXSHADER_INVALID)
	{
		if ( m_bSWVSBuffers && !m_bSWVertProcessing )
		{
			m_bSWVertProcessing = true;
			FreeDeviceObjects();
			ReCreateObject();
		}

		// Store the pointer to the actual shader during rendering.
		m_pVertexShader = LTVertexShaderMgr::GetSingleton().GetVertexShader(pPass->VertexShaderID);
		if (m_pVertexShader != NULL)
		{
			// Install the shader.
			if (!LTVertexShaderMgr::GetSingleton().InstallVertexShader(m_pVertexShader))
			{
				m_pVertexShader = NULL;
				return;
			}
		}
		else
		{
			return;
		}
	}
	else if (!m_VBController.getVertexFormat(0) || m_bNonFixPipeData)
	{
					  // This is a non fixed function pipe VB - bail out...
		return;
	}
	else if (FAILED(g_RenderStateMgr.SetVertexShader(m_VBController.getVertexFormat(0))))
	{
		return;
	}

	// If this pass has a pixel shader, use it.
	if (NULL != pPass &&
	    pPass->bUsePixelShader &&
		pPass->PixelShaderID != LTPixelShader::PIXELSHADER_INVALID)
	{
		// Store the pointer to the actual shader during rendering.
		m_pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(pPass->PixelShaderID);
		if (m_pPixelShader != NULL)
		{
			// Install the shader.
			if (!LTPixelShaderMgr::GetSingleton().InstallPixelShader(m_pPixelShader))
			{
				m_pPixelShader = NULL;
				return;
			}
		}
	}


	// We need software processing 
	if (m_bSWVertProcessing)
	{
		PD3DDEVICE->SetSoftwareVertexProcessing(TRUE);
	}


	m_VBController.SetStreamSources();
}

// NOTE: The texture list needs to change to be device independent...
void CD3DRigidMesh::RenderWithEffect(ModelInstance *pInstance, D3DMATRIX& WorldTransform,CD3DRenderStyle* pRenderStyle, uint32 iRenderPass)
{
	/*
	//setup our transform for this model
	g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0), &WorldTransform);

	if (FAILED(PD3DDEVICE->SetFVF(m_VBController.getVertexFormat(0))))
	{
		return;
	}

	//Set our sources
	m_VBController.SetStreamSources();
	*/

	//render away
	m_VBController.Render(0, 0, m_iVertCount, m_iPolyCount);
}




void CD3DRigidMesh::BeginRenderWithEffect(D3DMATRIX& pD3DTransforms, CD3DRenderStyle* pRenderStyle, uint32 iRenderPass)
{

	//setup our transform for this model
	g_RenderStateMgr.SetTransform(D3DTS_WORLDMATRIX(0), &pD3DTransforms);

	/*
	if(g_pVertexDeclaration)
	{
		PD3DDEVICE->SetVertexDeclaration(g_pVertexDeclaration);
	}
	else
	if (!m_VBController.getVertexFormat(0) || m_bNonFixPipeData)
	{
		// This is a non fixed function pipe VB - bail out...
		return;
	}
	else if (FAILED(g_RenderStateMgr.SetVertexShader(m_VBController.getVertexFormat(0))))
	{
		return;
	}
	*/

	m_VBController.SetStreamSources();

	// We need software processing 
	if (m_bSWVertProcessing)
	{
		PD3DDEVICE->SetSoftwareVertexProcessing(TRUE);
	}
}

void CD3DRigidMesh::EndRender()
{
	if ( m_bSWVertProcessing )
	{
		// If we are running with hardware then turn back on hardware processing
		if ( (g_Device.GetDeviceCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) )
		{
		
			PD3DDEVICE->SetSoftwareVertexProcessing(FALSE);
		}
	}


	PD3DDEVICE->SetStreamSource(0, 0, 0, 0);
	PD3DDEVICE->SetIndices(0);

	// Uninstall the vertex shader.
	if (NULL != m_pVertexShader)
	{
		LTVertexShaderMgr::GetSingleton().UninstallVertexShader();
		m_pVertexShader = NULL;
	}

	// Uninstall the pixel shader.
	if (NULL != m_pPixelShader)
	{
		LTPixelShaderMgr::GetSingleton().UninstallPixelShader();
		m_pPixelShader = NULL;
	}
}
