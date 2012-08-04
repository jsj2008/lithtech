//------------------------------------------------------------------
//
//  FILE      : LTVertexShaderMgr.cpp
//
//  PURPOSE   :	Vertex shader manager singleton
//
//  COPYRIGHT : LithTech Inc., 1996-2002
//
//------------------------------------------------------------------

#include "precompile.h"
#include "ltvertexshadermgr.h"
#include "d3d_device.h"
#include "d3d_utils.h"



//-------------------------------------------------------------------------------------------------
// LTVertexShaderImp
//-------------------------------------------------------------------------------------------------


bool LTVertexShaderImp::Init(ILTStream *pStream, const D3DVERTEXELEMENT9 *pVertexElements, uint32 VertexElementsSize, bool bCompileShader)
{
	Term();

	ZeroMemory(m_Constants, sizeof(m_Constants));

	// Allocate memory to read the vertex shader file.
    m_ByteCodeSize = pStream->GetLen();
	LT_MEM_TRACK_ALLOC(m_pByteCode = new uint8[m_ByteCodeSize + 4], LT_MEM_TYPE_RENDER_SHADER);
    if (m_pByteCode == NULL)
	{
		return false;
	}
    ZeroMemory(m_pByteCode, m_ByteCodeSize + 4);

    // Read the pre-compiled vertex shader microcode
	if (pStream->Read(m_pByteCode, m_ByteCodeSize) != LT_OK)
	{
		Term();
		return false;
	}

	// Save this for later.
	m_bCompileShader = bCompileShader;

	if (pVertexElements == NULL)
	{
		Term();
		return false;
	}

	// Copy the vertex elements array so that we can reconstruct the shader if needed.
	unsigned NumElements = VertexElementsSize/sizeof(D3DVERTEXELEMENT9);
	LT_MEM_TRACK_ALLOC(m_pVertexElements = new D3DVERTEXELEMENT9[NumElements], LT_MEM_TYPE_RENDER_SHADER);
	if (m_pVertexElements == NULL)
	{
		Term();
		return false;
	}
	memcpy(m_pVertexElements, pVertexElements, VertexElementsSize);

	// Create the vertex shader handle.
	return Recreate();
}



bool LTVertexShaderImp::Recreate()
{
	FreeDeviceObject();

	if (NULL == m_pByteCode ||
		NULL == m_pVertexElements)
	{
		return false;
	}

	HRESULT hr;

	// Create the declaration interface.
	hr = PD3DDEVICE->CreateVertexDeclaration(m_pVertexElements, &m_pDeclaration);
	if (FAILED(hr))
	{
		OUTPUT_D3D_ERROR(1, hr);
		Term();
		return false;
	}

	// Should we assemble the shader? If this is true, then the byte code must be raw text.
	if (m_bCompileShader)
	{
		// Assemble the shader.
		LPD3DXBUFFER pCode = NULL;
		hr = D3DXAssembleShader((const TCHAR*)m_pByteCode, m_ByteCodeSize, NULL, NULL, 0, &pCode, NULL);
		if (FAILED(hr))
		{
			OUTPUT_D3D_ERROR(1, hr);
			Term();
			return false;
		}

		// Create the vertex shader.
		hr = PD3DDEVICE->CreateVertexShader((const DWORD*)pCode->GetBufferPointer(), &m_pShader);
		if (FAILED(hr))
		{
			OUTPUT_D3D_ERROR(1, hr);
			Term();
			return false;
		}
	}
	else
	{
    	// Create the vertex shader directly from the byte code.
		hr = PD3DDEVICE->CreateVertexShader((const DWORD*)m_pByteCode, &m_pShader);
		if (FAILED(hr))
		{
			OUTPUT_D3D_ERROR(1, hr);
			Term();
			return false;
		}
	}

	return true;
}



void LTVertexShaderImp::Term()
{
	FreeDeviceObject();

	// Free the byte code.
	if (NULL != m_pByteCode)
	{
		delete [] m_pByteCode;
		m_pByteCode = NULL;
	}

	// Free the vertex elements array.
	if (NULL != m_pVertexElements)
	{
		delete [] m_pVertexElements;
		m_pVertexElements = NULL;
	}
}



void LTVertexShaderImp::FreeDeviceObject()
{
	// Release the declaration.
	if (NULL != m_pDeclaration)
	{
		m_pDeclaration->Release();
		m_pDeclaration = NULL;
	}

	// Release the shader.
	if (NULL != m_pShader)
	{
		m_pShader->Release();
		m_pShader = NULL;
	}
}



void LTVertexShaderImp::SetName(const char *pName)
{
	LTStrCpy(m_FileName, pName, MAX_PATH);
}



bool LTVertexShaderImp::GetConstant(unsigned RegisterNum, float *pf0, float *pf1, float *pf2, float *pf3)
{
	if (RegisterNum >= MAX_CONSTANT_REGISTERS)
	{
		return false;
	}

	unsigned i = RegisterNum*4;
	*pf0 = m_Constants[i++];
	*pf1 = m_Constants[i++];
	*pf2 = m_Constants[i++];
	*pf3 = m_Constants[i++];

	return true;
}



bool LTVertexShaderImp::SetConstant(unsigned RegisterNum, float f0, float f1, float f2, float f3)
{
	if (RegisterNum >= MAX_CONSTANT_REGISTERS)
	{
		return false;
	}

	unsigned i = RegisterNum*4;
	m_Constants[i++] = f0;
	m_Constants[i++] = f1;
	m_Constants[i++] = f2;
	m_Constants[i++] = f3;

	return true;
}



bool LTVertexShaderImp::SetConstant(unsigned RegisterNum, const LTMatrix &Matrix)
{
	if (RegisterNum + 3 >= MAX_CONSTANT_REGISTERS)
	{
		return false;
	}

	unsigned iReg = RegisterNum*4;
	for (unsigned i = 0; i < 4; ++i)
	{
		for (unsigned j = 0; j < 4; ++j)
		{
			m_Constants[iReg++] = Matrix.m[i][j];
		}
	}

	return true;
}



//-------------------------------------------------------------------------------------------------
// LTVertexShaderMgr
//-------------------------------------------------------------------------------------------------


LTVertexShaderMgr::LTVertexShaderMgr()
{
}



LTVertexShaderMgr::~LTVertexShaderMgr()
{
	Term();
}



void LTVertexShaderMgr::Term()
{
	// Delete the vertex shaders.
	m_VertexShaders.Term();
}



LTVertexShaderMgr& LTVertexShaderMgr::GetSingleton()
{
	static LTVertexShaderMgr s_Singleton;
	return s_Singleton;
}



bool LTVertexShaderMgr::AddVertexShader(ILTStream *pStream, const char *ShaderName, int ShaderID,
										const D3DVERTEXELEMENT9 *pVertexElements, uint32 VertexElementsSize, bool bCompileShader)
{
	// Check to see if this vertex shader has already been added.
	if (m_VertexShaders.Get(ShaderID) != LTNULL)
	{
		return false;
	}

	// Create and add the vertex shader to the list.
	LTVertexShaderImp *pShader = m_VertexShaders.Add(ShaderName, ShaderID);
	if (pShader == LTNULL)
	{
		return false;
	}

	// Initialize the shader.
	return pShader->Init(pStream, pVertexElements, VertexElementsSize, bCompileShader);
}



void LTVertexShaderMgr::RemoveVertexShader(int ShaderID)
{
	LTVertexShaderImp *pShader = m_VertexShaders.Get(ShaderID);
	if (pShader != LTNULL)
	{
		// Now remove the vertex shader.
		m_VertexShaders.Remove(ShaderID);
	}
}



void LTVertexShaderMgr::RemoveAllVertexShaders()
{
	m_VertexShaders.Term();
}



LTVertexShader*	LTVertexShaderMgr::GetVertexShader(int ShaderID)
{
	LTVertexShaderImp *pShader = m_VertexShaders.Get(ShaderID);
	return pShader;
}



void LTVertexShaderMgr::FreeDeviceObjects()
{
	LTVertexShaderImp *pShader = (LTVertexShaderImp*)(m_VertexShaders.GetFront());
	while (pShader != NULL)
	{
		pShader->FreeDeviceObject();
		pShader = (LTVertexShaderImp*)(pShader->GetNext());
	}
}



void LTVertexShaderMgr::RecreateVertexShaders()
{
	LTVertexShaderImp *pShader = (LTVertexShaderImp*)(m_VertexShaders.GetFront());
	while (pShader != NULL)
	{
		pShader->Recreate();

		pShader = (LTVertexShaderImp*)(pShader->GetNext());
	}
}



bool LTVertexShaderMgr::SetVertexShaderConstants(LTVertexShader *pShader)
{
	LTVertexShaderImp *pShaderImp = (LTVertexShaderImp*)pShader;

	// Make sure we have valid data.
	if (pShaderImp == NULL ||
		!pShaderImp->IsValidShader())
	{
		return false;
	}

	// Get the constants.
	float *pConstants = pShaderImp->GetConstants();
	if (pConstants == NULL)
	{
		return false;
	}

	// Set the constants.
	HRESULT hr = PD3DDEVICE->SetVertexShaderConstantF(0, pConstants, LTVertexShader::MAX_CONSTANT_REGISTERS);
	if (FAILED(hr))
	{
		OUTPUT_D3D_ERROR(1, hr);
		return false;
	}

	return true;
}



bool LTVertexShaderMgr::InstallVertexShader(LTVertexShader *pShader) const
{
	LTVertexShaderImp *pShaderImp = (LTVertexShaderImp*)pShader;
	if (pShaderImp == NULL ||
	    !pShaderImp->IsValidShader())
	{
		return false;
	}

	HRESULT hr;

	// Setup the vertex shader.
	hr = PD3DDEVICE->SetVertexShader(pShaderImp->GetShader());
	if (FAILED(hr))
	{
		OUTPUT_D3D_ERROR(1, hr);
		return false;
	}

	// Setup the vertex shader declaration.
	hr = PD3DDEVICE->SetVertexDeclaration(pShaderImp->GetDeclaration());
	if (FAILED(hr))
	{
		OUTPUT_D3D_ERROR(1, hr);
		return false;
	}

	return true;
}



void LTVertexShaderMgr::UninstallVertexShader() const
{
	PD3DDEVICE->SetVertexShader(NULL);
}
