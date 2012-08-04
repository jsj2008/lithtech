
#include "precompile.h"
#include "LTEffectImpl.h"
#include "rendererconsolevars.h"
#include "render.h"
#include "d3d_device.h"
#include "d3d_texture.h"
#include "d3d_utils.h"
#include "LTEffectInclude.h"
#include "lteffectshadermgr.h"
#include "rendertargetmgr.h"
#include "rendertarget.h"
#include "de_world.h"

LTEffectImpl::LTEffectImpl():
m_pEffect(NULL),
m_pVertexElements(NULL),
m_pVertexDeclaration(NULL),
m_bCompileShader(true),
m_pByteCode(NULL),
m_ByteCodeSize(0),
m_EffectPoolID(INVALID_EFFECT_POOL)
{
}

LTEffectImpl::~LTEffectImpl()
{
	Term();
}

bool LTEffectImpl::Init(ILTStream *pStream, const uint32 *pVertexElements, uint32 VertexElementsSize, HEFFECTPOOL EffectPoolID)
{
	uint32 nLen = pStream->GetLen();

	LT_MEM_TRACK_ALLOC(m_pByteCode = new uint8[nLen] ,LT_MEM_TYPE_RENDER_SHADER);
	pStream->Read(m_pByteCode, nLen);
	m_ByteCodeSize = (unsigned)nLen;

	// Copy the vertex elements array so that we can reconstruct the shader if needed.
	if(pVertexElements)
	{
		unsigned NumElements = VertexElementsSize/sizeof(D3DVERTEXELEMENT9);
		LT_MEM_TRACK_ALLOC(m_pVertexElements = new D3DVERTEXELEMENT9[NumElements], LT_MEM_TYPE_RENDER_SHADER);
		if (m_pVertexElements == NULL)
		{
			Term();
			return false;
		}
		memcpy(m_pVertexElements, pVertexElements, VertexElementsSize);
	}

	m_EffectPoolID = EffectPoolID;

	return Recreate();
}

void LTEffectImpl::Term()
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

	if(m_pByteCode)
	{
		delete [] m_pByteCode;
		m_pByteCode = NULL;
		m_ByteCodeSize = 0;
	}
}

void  LTEffectImpl::FreeDeviceObject()
{
	if(m_pVertexDeclaration)
	{
		m_pVertexDeclaration->Release();
		m_pVertexDeclaration = NULL;
	}

	if(m_pEffect)
	{
		m_pEffect->Release();
		m_pEffect = NULL;
	}
}

bool  LTEffectImpl::Recreate()
{
	// if it's not termed, then term it
	FreeDeviceObject();

	if(!m_pByteCode)
	{
		return false;
	}

	HRESULT hr;

	// Create the declaration interface.
	if(m_pVertexElements)
	{
		hr = PD3DDEVICE->CreateVertexDeclaration(m_pVertexElements, &m_pVertexDeclaration);
		if (FAILED(hr))
		{
			OUTPUT_D3D_ERROR(1, hr);
			Term();
			return false;
		}
	}

	LPD3DXBUFFER pErrorBuffer = NULL;

	DWORD dwShaderFlags = 0;
	
	if(g_CV_Effect_ForceSoftwareShaders)
	{
		dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
		dwShaderFlags |= D3DXSHADER_NO_PRESHADER;
		dwShaderFlags |= D3DXSHADER_DEBUG;
	}

	//m_EffectPoolID
	LPD3DXEFFECTPOOL pEffectPool = NULL;
	
	if(m_EffectPoolID != INVALID_EFFECT_POOL)
	{
		pEffectPool = LTEffectShaderMgr::GetSingleton().GetEffectPool(m_EffectPoolID);
	}
	
	LTEffectInclude includeHandler;
	includeHandler.SetParentFilename(m_FileName);
	hr = D3DXCreateEffect(r_GetRenderStruct()->GetD3DDevice(),
		m_pByteCode,
		m_ByteCodeSize,
		NULL,
		&includeHandler,
		dwShaderFlags,
		pEffectPool,
		&m_pEffect,
		&pErrorBuffer);

	if(FAILED(hr))
	{
		const char* pszError = (pErrorBuffer != NULL) ? (const char*)pErrorBuffer->GetBufferPointer() : "<Unknown Error>";
		dsi_ConsolePrint("EffectShader: failed to load! (Errorcode: %d)", hr);
		dsi_ConsolePrint("D3DError: (%s)", pszError);

		if(pErrorBuffer)
		{
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

void LTEffectImpl::SetName(const char *pName)
{
	LTStrCpy(m_FileName, pName, MAX_PATH);
}

LTRESULT LTEffectImpl::SetBool(const char* szParam, LTBOOL bBool)  const
{
	HRESULT hr = m_pEffect->SetBool(szParam, bBool);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}

	return LT_OK;
}


LTRESULT LTEffectImpl::SetBoolArray(const char* szParam, LTBOOL *bBool, int nCount) const
{	
	HRESULT hr = m_pEffect->SetBoolArray(szParam, (const BOOL*)bBool, nCount);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetFloat(const char* szParam, float fFloat) const
{
	HRESULT hr = m_pEffect->SetFloat(szParam, fFloat);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetFloatArray(const char* szParam, float *fFloat, int nCount) const
{
	HRESULT hr = m_pEffect->SetFloatArray(szParam, (const float*)fFloat, nCount);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetInt(const char* szParam, int nInt) const
{
	HRESULT hr = m_pEffect->SetInt(szParam, nInt);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetIntArray(const char* szParam, int *nInt, int nCount) const
{
	HRESULT hr = m_pEffect->SetIntArray(szParam, (const int*)nInt, nCount);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}


	return LT_OK;
}

LTRESULT LTEffectImpl::SetMatrix(const char* szParam, LTMatrix &mMatrix) const
{
	D3DXMATRIX matD3DX( *(const D3DMATRIX*)&mMatrix );
	HRESULT hr = m_pEffect->SetMatrix(szParam, &matD3DX);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetMatrixArray(const char* szParam, LTMatrix *mMatrix, int nCount) const
{
	D3DXMATRIX matD3DX( *(const D3DMATRIX*)&mMatrix );
	HRESULT hr = m_pEffect->SetMatrixArray(szParam, &matD3DX, nCount);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetMatrixTranspose(const char* szParam, LTMatrix &mMatrix) const
{
	D3DXMATRIX matD3DX( *(const D3DMATRIX*)&mMatrix );
	HRESULT hr = m_pEffect->SetMatrixTranspose(szParam, &matD3DX);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetMatrixTransposeArray(const char* szParam, LTMatrix *mMatrix, int nCount) const
{
	D3DXMATRIX matD3DX( *(const D3DMATRIX*)&mMatrix );
	HRESULT hr = m_pEffect->SetMatrixTransposeArray(szParam, &matD3DX, nCount);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetString(const char* szParam, const char* szString) const
{
	if(FAILED(m_pEffect->SetString(szParam, szString)))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetTechnique(const char* szTechnique) const
{
	if(szTechnique)
	{
		D3DXHANDLE hTechnique = m_pEffect->GetTechniqueByName(szTechnique);
		if(NULL == hTechnique)
		{
			return LT_ERROR;
		}
		
		if( FAILED(m_pEffect->SetTechnique(hTechnique)) )
		{
			return LT_ERROR;
		}

		return LT_OK;
	}

	return LT_ERROR;
}

LTRESULT LTEffectImpl::ValidateTechnique(const char* szTechnique) const
{
	if(szTechnique)
	{
		if(FAILED(m_pEffect->ValidateTechnique(szTechnique)))
		{
			// It's bad!
			return LT_ERROR;
		}

		// It's good!
		return LT_OK;
	}

	// Null string!
	return LT_ERROR;
}

LTRESULT LTEffectImpl::FindFirstValidTechnique(LTTechniqueInfo* pInfo) const
{

	if(pInfo == NULL)
	{
		return LT_ERROR;
	}

	D3DXHANDLE hHandle = NULL;
	if(FAILED(m_pEffect->FindNextValidTechnique(NULL, &hHandle)))
	{
		return LT_ERROR;
	}

	D3DXTECHNIQUE_DESC desc;
	if( FAILED(m_pEffect->GetTechniqueDesc(hHandle, &desc)) )
	{
		return LT_ERROR;
	}

	strncpy(pInfo->szName, desc.Name, 127);
	pInfo->nPasses = desc.Passes;

	return LT_OK;
}

LTRESULT LTEffectImpl::SetTexture(const char* szParam, HTEXTURE hTexture) const
{
	if(hTexture == NULL)
	{
		return LT_ERROR;
	}

	SharedTexture* pSharedTexture = (SharedTexture*)hTexture;
	RTexture *pRenderTexture = (RTexture*)pSharedTexture->m_pRenderData;
	
	if(FAILED(m_pEffect->SetTexture(szParam, pRenderTexture->m_pD3DTexture)))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetTextureRT(const char* szParam, HRENDERTARGET hRenderTarget) const
{
	CRenderTarget* pRenderTarget = CRenderTargetMgr::GetSingleton().GetRenderTarget(hRenderTarget);
	if(pRenderTarget)
	{
		IDirect3DTexture9* pTexture = pRenderTarget->GetRenderTargetTexture();
		if(pTexture)
		{
			if(FAILED(m_pEffect->SetTexture(szParam, pTexture)))
			{
				return LT_ERROR;
			}

			return LT_OK;
		}
	}

	return LT_ERROR;
}

LTRESULT LTEffectImpl::SetVector(const char* szParam, float *fFloat) const
{
	D3DXVECTOR4 vecFour;
	vecFour.x = fFloat[0];
	vecFour.y = fFloat[1];
	vecFour.z = fFloat[2];
	vecFour.w = fFloat[3];

	HRESULT hr = m_pEffect->SetVector(szParam, &vecFour);

	if(FAILED(hr))
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT LTEffectImpl::SetVectorArray(const char* szParam, float *fFloat, int nCount) const
{
	dsi_ConsolePrint("Error: SetVectorArray() is not implemented!");
	return LT_OK;
}

LTRESULT LTEffectImpl::UploadVertexDeclaration()
{
	if(m_pVertexDeclaration)
	{
		if( FAILED(PD3DDEVICE->SetVertexDeclaration(m_pVertexDeclaration)) )
		{
			return LT_ERROR;
		}
	}

	return LT_OK;
}
