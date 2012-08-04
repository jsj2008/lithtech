#include "stdafx.h"
#include "d3d_device.h"
#include "d3d_effectmgr.h"
#include "LTEffectInclude.h"

// GLOBALS
CD3DEffectMgr g_EffectMgr;

//Do the vertex decls
// Rigid Model (-stream0 position normal uv1 basisvectors)
D3DVERTEXELEMENT9 decl_Model[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }, //size 12
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 }, //size 12
	{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 }, //size 8
	{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 }, // size 12
	{ 0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0 }, // size 12 (52 bytes total)
	D3DDECL_END()
};

CD3DEffectMgr::CD3DEffectMgr():
m_pEffect(NULL),
m_pVertexDeclaration(NULL),
m_bEnabled(false)
{
	m_pszLastError[0] = '\0';
	m_vPosition.x = m_vPosition.y = m_vPosition.z = 0.0f;
}

CD3DEffectMgr::~CD3DEffectMgr()
{
	Term();
}

bool CD3DEffectMgr::Init()
{
	if(m_pVertexDeclaration)
	{
		return true;
	}


	HRESULT hr = PD3DDEVICE->CreateVertexDeclaration(decl_Model, &m_pVertexDeclaration);
	if (FAILED(hr))
	{
		Term();
		return false;
	}

	return true;
}

void CD3DEffectMgr::Term()
{
	if(m_pEffect)
	{
		m_pEffect->Release();
		m_pEffect = NULL;
	}

	if(m_pVertexDeclaration)
	{
		m_pVertexDeclaration->Release();
		m_pVertexDeclaration = NULL;
	}
}

bool CD3DEffectMgr::Load(const char* szFilename, bool bSoftware)
{
	// make sure we're term'd first
	Term();
	Init();

	if(szFilename == NULL)
	{
		return false;
	}

	LPD3DXBUFFER pErrorBuffer = NULL;

	DWORD dwShaderFlags = 0;

	if(bSoftware)
	{
		dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
		dwShaderFlags |= D3DXSHADER_NO_PRESHADER;
		dwShaderFlags |= D3DXSHADER_DEBUG;
	}

	LTEffectInclude includeSolver;
	//includeSolver.SetParentFilename(szFilename);
	HRESULT hr = D3DXCreateEffectFromFile(PD3DDEVICE,
		szFilename,
		NULL,
		&includeSolver,
		dwShaderFlags,
		NULL,
		&m_pEffect,
		&pErrorBuffer);

	if(FAILED(hr))
	{
		if(pErrorBuffer)
		{
			const char* pszError = (pErrorBuffer != NULL) ? (const char*)pErrorBuffer->GetBufferPointer() : "<Unknown Error>";

			// Save the error to our last error buffer.
			int nLen = strlen(pszError);
			strncpy(m_pszLastError, pszError, (nLen < 1023) ? nLen : 1023);

			pErrorBuffer->Release();
		}

		m_pEffect = NULL;
		return false;
	}

	if(pErrorBuffer)
	{
		pErrorBuffer->Release();
	}


	return true;
}

void CD3DEffectMgr::UploadVertexDecl()
{
	if(m_pVertexDeclaration)
	{
		PD3DDEVICE->SetVertexDeclaration(m_pVertexDeclaration);
	}
}

