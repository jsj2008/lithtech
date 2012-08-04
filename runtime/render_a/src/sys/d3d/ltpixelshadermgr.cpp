//------------------------------------------------------------------
//
//  FILE      : LTPixelShaderMgr.cpp
//
//  PURPOSE   :	Pixel shader manager singleton
//
//  COPYRIGHT : LithTech Inc., 1996-2003
//
//------------------------------------------------------------------

#include "precompile.h"
#include "ltpixelshadermgr.h"
#include "d3d_device.h"
#include "d3d_utils.h"



//-------------------------------------------------------------------------------------------------
// LTPixelShaderImp
//-------------------------------------------------------------------------------------------------


// Init -- load the pixel shader byte code from file.
//
bool LTPixelShaderImp::Init(ILTStream *pStream, bool bCompileShader)
{
	Term();

	ZeroMemory(m_Constants, sizeof(m_Constants));

	// The old LithTech pixel shader format is no longer supported.
	uint32 nTag;
	*pStream >> nTag;
	if (nTag == MAKEFOURCC('L', 'T', 'P', 'S'))
	{
		// not supported
		assert(!"LithTech pixel shader files are no longer supported. You must use DX9 pixel shader files!");
		return false;
	}
	else
	{
		//
		// This is a regular microsoft pixel shader.
		//

		pStream->SeekTo(0);

		// Allocate memory to read the pixel shader file.
    	m_ByteCodeSize = pStream->GetLen();
		LT_MEM_TRACK_ALLOC(m_pByteCode = new uint8[m_ByteCodeSize + 4], LT_MEM_TYPE_RENDER_SHADER);
    	if (m_pByteCode == NULL)
		{
			Term();
			return false;
		}
    	ZeroMemory(m_pByteCode, m_ByteCodeSize + 4);

    	// Read the pre-compiled pixel shader microcode
		if (pStream->Read(m_pByteCode, m_ByteCodeSize) != LT_OK)
		{
			Term();
			return false;
		}
	}

	// Save this for later.
	m_bCompileShader = bCompileShader;

	// Create the pixel shader handle.
	return Recreate();
}



bool LTPixelShaderImp::Recreate()
{
	FreeDeviceObject();

	if (NULL == m_pByteCode)
	{
		return false;
	}

	HRESULT hr;

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

		// Create the pixel shader.
		hr = PD3DDEVICE->CreatePixelShader((const DWORD*)pCode->GetBufferPointer(), &m_pShader);
		if (FAILED(hr))
		{
			OUTPUT_D3D_ERROR(1, hr);
			Term();
			return false;
		}
	}
	else
	{
    	// Create the pixel shader directly from the byte code.
		hr = PD3DDEVICE->CreatePixelShader((const DWORD*)m_pByteCode, &m_pShader);
		if (FAILED(hr))
		{
			OUTPUT_D3D_ERROR(1, hr);
			Term();
			return false;
		}
	}

	// Try to install the pixel shader once for good measure.
	hr = PD3DDEVICE->SetPixelShader(m_pShader);
	if (FAILED(hr))
	{
		// Failed to setup the shader; it isn't supported.
		OUTPUT_D3D_ERROR(1, hr);
		Term();
		return false;
	}

	// We successfully installed it, so clear it out now.
	PD3DDEVICE->SetPixelShader(NULL);

	return true;
}



void LTPixelShaderImp::Term()
{
	FreeDeviceObject();

	// Free the byte code.
	if (NULL != m_pByteCode)
	{
		delete [] m_pByteCode;
		m_pByteCode = NULL;
	}
}



void LTPixelShaderImp::FreeDeviceObject()
{
	// Release the shader.
	if (NULL != m_pShader)
	{
		m_pShader->Release();
		m_pShader = NULL;
	}
}



void LTPixelShaderImp::SetName(const char *pName)
{
	LTStrCpy(m_FileName, pName, MAX_PATH);
}



bool LTPixelShaderImp::GetConstant(unsigned RegisterNum, float *pf0, float *pf1, float *pf2, float *pf3)
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



bool LTPixelShaderImp::SetConstant(unsigned RegisterNum, float f0, float f1, float f2, float f3)
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



bool LTPixelShaderImp::SetConstant(unsigned RegisterNum, const LTMatrix &Matrix)
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
// LTPixelShaderMgr
//-------------------------------------------------------------------------------------------------


LTPixelShaderMgr::LTPixelShaderMgr()
{
}



LTPixelShaderMgr::~LTPixelShaderMgr()
{
	Term();
}



void LTPixelShaderMgr::Term()
{
	// Delete the pixel shaders.
	m_PixelShaders.Term();
}



LTPixelShaderMgr& LTPixelShaderMgr::GetSingleton()
{
	static LTPixelShaderMgr s_Singleton;
	return s_Singleton;
}



bool LTPixelShaderMgr::AddPixelShader(ILTStream *pStream, const char *ShaderName, int ShaderID, bool bCompileShader)
{
	// Check to see if this pixel shader has already been added.
	if (m_PixelShaders.Get(ShaderID) != LTNULL)
	{
		return false;
	}

	// Create and add the pixel shader to the list.
	LTPixelShaderImp *pShader = m_PixelShaders.Add(ShaderName, ShaderID);
	if (pShader == LTNULL)
	{
		return false;
	}

	// Initialize the shader.
	return pShader->Init(pStream, bCompileShader);
}



void LTPixelShaderMgr::RemovePixelShader(int ShaderID)
{
	LTPixelShaderImp *pShader = m_PixelShaders.Get(ShaderID);
	if (pShader != LTNULL)
	{
		// Now remove the pixel shader.
		m_PixelShaders.Remove(ShaderID);
	}
}



void LTPixelShaderMgr::RemoveAllPixelShaders()
{
	m_PixelShaders.Term();
}



LTPixelShader*	LTPixelShaderMgr::GetPixelShader(int ShaderID)
{
	LTPixelShaderImp *pShader = m_PixelShaders.Get(ShaderID);
	return pShader;
}



void LTPixelShaderMgr::FreeDeviceObjects()
{
	LTPixelShaderImp *pShader = (LTPixelShaderImp*)(m_PixelShaders.GetFront());
	while (pShader != NULL)
	{
		pShader->FreeDeviceObject();
		pShader = (LTPixelShaderImp*)(pShader->GetNext());
	}
}



void LTPixelShaderMgr::RecreatePixelShaders()
{
	LTPixelShaderImp *pShader = (LTPixelShaderImp*)(m_PixelShaders.GetFront());
	while (pShader != NULL)
	{
		pShader->Recreate();
		pShader = (LTPixelShaderImp*)(pShader->GetNext());
	}
}



bool LTPixelShaderMgr::SetPixelShaderConstants(LTPixelShader *pShader)
{
	LTPixelShaderImp *pShaderImp = (LTPixelShaderImp*)pShader;

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

	// Set the user-defined constants.
	HRESULT hr = PD3DDEVICE->SetPixelShaderConstantF(0, pConstants, LTPixelShader::MAX_CONSTANT_REGISTERS);
	if (FAILED(hr))
	{
		OUTPUT_D3D_ERROR(1, hr);
		return false;
	}

	return true;
}



bool LTPixelShaderMgr::InstallPixelShader(LTPixelShader *pShader) const
{
	LTPixelShaderImp *pShaderImp = (LTPixelShaderImp*)pShader;
	if (pShaderImp == NULL ||
	    !pShaderImp->IsValidShader())
	{
		return false;
	}

	// Setup the pixel shader.
	HRESULT hr = PD3DDEVICE->SetPixelShader(pShaderImp->GetShader());
	if (FAILED(hr))
	{
		OUTPUT_D3D_ERROR(1, hr);
		return false;
	}

	return true;
}



void LTPixelShaderMgr::UninstallPixelShader() const
{
	PD3DDEVICE->SetPixelShader(NULL);
}
