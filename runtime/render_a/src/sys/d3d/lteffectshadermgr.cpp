//------------------------------------------------------------------
//
//  FILE      : LTEffectShaderMgr.cpp
//
//  PURPOSE   :	Effect shader manager singleton
//
//  COPYRIGHT : LithTech Inc., 1996-2005
//
//------------------------------------------------------------------

#include "precompile.h"
#include "lteffectshadermgr.h"
#include "d3d_device.h"
#include "d3d_utils.h"

//-------------------------------------------------------------------------------------------------
// LTPixelShaderMgr
//-------------------------------------------------------------------------------------------------


LTEffectShaderMgr::LTEffectShaderMgr()
{
}



LTEffectShaderMgr::~LTEffectShaderMgr()
{
	Term();
}



void LTEffectShaderMgr::Term()
{
	// Delete the pixel shaders.
	m_EffectShaders.Term();

	LTEffectPools::iterator iter = m_EffectPools.begin();
	while(iter != m_EffectPools.end())
	{
		if(iter->second)
		{
			iter->second->Release();
		}
		++iter;
	}
	m_EffectPools.clear();
}



LTEffectShaderMgr& LTEffectShaderMgr::GetSingleton()
{
	static LTEffectShaderMgr s_Singleton;
	return s_Singleton;
}



bool LTEffectShaderMgr::AddEffectShader(ILTStream *pStream, 
										const char *ShaderName,
										int ShaderID, 
										const uint32 *pVertexElements,
										uint32 VertexElementsSize,
										HEFFECTPOOL EffectPoolID)
{
	// Check to see if this pixel shader has already been added.
	if (m_EffectShaders.Get(ShaderID) != LTNULL)
	{
		return false;
	}

	// Create and add the pixel shader to the list.
	LTEffectImpl *pShader = m_EffectShaders.Add(ShaderName, ShaderID);
	if (pShader == LTNULL)
	{
		return false;
	}

	// Initialize the shader.
	return pShader->Init(pStream, pVertexElements, VertexElementsSize, EffectPoolID);
}



void LTEffectShaderMgr::RemoveEffectShader(int ShaderID)
{
	LTEffectImpl *pShader = m_EffectShaders.Get(ShaderID);
	if (pShader != LTNULL)
	{
		// Now remove the pixel shader.
		m_EffectShaders.Remove(ShaderID);
	}
}



void LTEffectShaderMgr::RemoveAllEffectShaders()
{
	m_EffectShaders.Term();
}



LTEffectShader*	LTEffectShaderMgr::GetEffectShader(int ShaderID)
{
	LTEffectImpl *pShader = m_EffectShaders.Get(ShaderID);
	return pShader;
}

bool LTEffectShaderMgr::AddEffectPool(HEFFECTPOOL EffectPoolID)
{
	if(EffectPoolID == INVALID_EFFECT_POOL)
	{
		dsi_ConsolePrint("Error: %d is an invalid EffectPool ID!");
		return false;
	}

	LPD3DXEFFECTPOOL pEffectPool;
	HRESULT hr = D3DXCreateEffectPool(&pEffectPool);
	if(!FAILED( hr ))
	{
		m_EffectPools[EffectPoolID] = pEffectPool;
		return true;
	}

	return false;
}

bool LTEffectShaderMgr::RemoveEffectPool(HEFFECTPOOL EffectPoolID)
{
	LTEffectPools::iterator iter = m_EffectPools.find(EffectPoolID);
	if(iter != m_EffectPools.end())
	{
		iter->second->Release();
		m_EffectPools.erase(iter);
		return true;
	}

	return false;
}

ID3DXEffectPool* LTEffectShaderMgr::GetEffectPool(HEFFECTPOOL EffectPoolID)
{
	if(EffectPoolID == INVALID_EFFECT_POOL)
	{
		return NULL;
	}

	LTEffectPools::iterator iter = m_EffectPools.find(EffectPoolID);
	if(iter != m_EffectPools.end())
	{
		return iter->second;
	}

	return NULL;
}

void LTEffectShaderMgr::FreeDeviceObjects()
{
	LTEffectImpl *pShader = (LTEffectImpl*)(m_EffectShaders.GetFront());
	while (pShader != NULL)
	{
		pShader->FreeDeviceObject();
		pShader = (LTEffectImpl*)(pShader->GetNext());
	}
}

void LTEffectShaderMgr::RecreateEffectShaders()
{
	LTEffectImpl *pShader = (LTEffectImpl*)(m_EffectShaders.GetFront());
	while (pShader != NULL)
	{
		pShader->Recreate();
		pShader = (LTEffectImpl*)(pShader->GetNext());
	}
}
