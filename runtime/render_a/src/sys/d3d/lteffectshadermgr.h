//------------------------------------------------------------------
//
//  FILE      : LTEffectShaderMgr.h
//
//  PURPOSE   :	Effect shader manager singleton
//
//  COPYRIGHT : LithTech Inc., 1996-2005
//
//------------------------------------------------------------------

#ifndef __LTEFFECTSHADERMGR_H__
#define __LTEFFECTSHADERMGR_H__


#include "ltbasedefs.h"
#include "ltidtoobjecttable.h"
#include "LTEffectImpl.h"

#include <d3dx9.h>

// maps effect shader ID to effect shader pointer
typedef std::map<int, LTEffectImpl*> LTEffectShaderMap;

// lookup table for effect shaders
typedef LTIDToObjectTable<LTEffectImpl, LTEffectShaderMap>	LTEffectShaders;

typedef std::map<HEFFECTPOOL, ID3DXEffectPool*> LTEffectPools;

// LTPixelShaderMgr -- the actual manager for the effect shaders.
class LTEffectShaderMgr
{
public:
	~LTEffectShaderMgr();

	void						Term();

	// singleton access
	static LTEffectShaderMgr&	GetSingleton();

	// add an effect shader from file
	bool						AddEffectShader(ILTStream *pStream, 
												const char *ShaderName,
												int ShaderID,
												const uint32 *pVertexElements,
												uint32 VertexElementsSize,
												HEFFECTPOOL EffectPoolID);

	// remove an effect shader
	void						RemoveEffectShader(int ShaderID);

	// remove all effect shaders
	void						RemoveAllEffectShaders();

	// effect shader access
	LTEffectShader*					GetEffectShader(int ShaderID);

	bool						AddEffectPool(HEFFECTPOOL EffectPoolID);
	bool						RemoveEffectPool(HEFFECTPOOL EffectPoolID);
	ID3DXEffectPool*			GetEffectPool(HEFFECTPOOL EffectPoolID);

	// frees all the device shader handles
	void						FreeDeviceObjects();

	// recreates all the shaders. This is necessary when the device changes.
	void						RecreateEffectShaders();

private:

	LTEffectShaderMgr();

private:

	LTEffectShaders				m_EffectShaders;			// list of pixel shaders loaded from file
	LTEffectPools				m_EffectPools;
};



#endif // __LTEFFECTSHADERMGR_H__
