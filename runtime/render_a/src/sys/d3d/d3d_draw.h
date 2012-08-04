
// This defines the d3d drawing structures and routines.

#ifndef __D3D_DRAW_H__
#define __D3D_DRAW_H__

class WorldPoly;
struct RGBColor;
class SharedTexture;
class ViewParams;
class IAggregateShader;

#ifndef __D3D_CONVAR_H__
#include "d3d_convar.h"
#endif

#ifndef __D3D9TYPES_H__
#include <d3d9types.h>
#define __D3D9TYPES_H__
#endif

#ifndef __D3D_DEVICE_H__
#include "d3d_device.h"
#endif

#ifndef __DE_OBJECTS_H__
#include "de_objects.h"
#endif

#ifndef __D3D_UTILS_H__
#include "d3d_utils.h"
#endif

// Definitions.
#ifdef _DEBUG
	void VerifyRenderState(D3DRENDERSTATETYPE state, uint32 val, char *pStateName, char *pValName);
	void VerifyStageState(uint32 stage, D3DTEXTURESTAGESTATETYPE state, uint32 val, char *pStateName, char *pValName);
	void VerifyTranslucentObjectStates();

	#define VERIFY_RENDERSTATE(state, val) VerifyRenderState(state, val, #state, #val);
	#define VERIFY_STAGESTATE(stage, state, val) VerifyStageState(stage, state, val, #state, #val);
	#define VERIFY_TRANSLUCENTOBJECTSTATES() VerifyTranslucentObjectStates();
#else
	#define VERIFY_RENDERSTATE(state, val) 
	#define VERIFY_STAGESTATE(stage, state, val)
	#define VERIFY_TRANSLUCENTOBJECTSTATES()
#endif

// ---------------------------------------------------------------- //
// Externs.
// ---------------------------------------------------------------- //

// This can be used to set a state, and it will automatically restore the state's
// value in its destructor.
class StateSet
{
public:
						StateSet(D3DRENDERSTATETYPE state, uint32 val) {
							m_State = state;
							PD3DDEVICE->GetRenderState(m_State, (unsigned long *)&m_OldVal);
							PD3DDEVICE->SetRenderState(m_State, val); }

						~StateSet() {
							PD3DDEVICE->SetRenderState(m_State, m_OldVal); }

	D3DRENDERSTATETYPE	m_State;
	uint32				m_OldVal;
};


// This can be used to set a state, and it will automatically restore the state's
// value in its destructor.
class StageStateSet
{
public:
						StageStateSet(uint32 stage, D3DTEXTURESTAGESTATETYPE state, uint32 val) 
						{
							m_Stage = stage;
							m_State = state;
							PD3DDEVICE->GetTextureStageState(m_Stage, m_State, (unsigned long *)&m_OldVal);

							if(m_OldVal != val)
								PD3DDEVICE->SetTextureStageState(m_Stage, m_State, val); 
						}

						~StageStateSet() 
						{
							PD3DDEVICE->SetTextureStageState(m_Stage, m_State, m_OldVal); 
						}

	uint32						m_Stage;
	D3DTEXTURESTAGESTATETYPE	m_State;
	uint32						m_OldVal;
};


// This can be used to set a stage sampler state, and it will automatically restore the state's
// value in its destructor.
class SamplerStateSet
{
public:
						SamplerStateSet(uint32 stage, D3DSAMPLERSTATETYPE state, uint32 val) 
						{
							m_Stage = stage;
							m_State = state;
							PD3DDEVICE->GetSamplerState(m_Stage, m_State, (unsigned long *)&m_OldVal);

							if(m_OldVal != val)
								PD3DDEVICE->SetSamplerState(m_Stage, m_State, val); 
						}

						~SamplerStateSet() 
						{
							PD3DDEVICE->SetSamplerState(m_Stage, m_State, m_OldVal); 
						}

	uint32				m_Stage;
	D3DSAMPLERSTATETYPE	m_State;
	uint32				m_OldVal;
};


// ---------------------------------------------------------------- //
// Functions.
// ---------------------------------------------------------------- //

// Get special alpha blend states based on the object's flags2.
inline void d3d_GetBlendStates(LTObject *pObject, uint32 &srcBlend, uint32 &destBlend, uint32 &dwFog, uint32 &dwFogColor)
{
	PD3DDEVICE->GetRenderState(D3DRS_FOGCOLOR, (unsigned long *)&dwFogColor);

	PD3DDEVICE->GetRenderState(D3DRS_FOGENABLE, (unsigned long *)&dwFog);
	if (pObject->m_Flags & FLAG_FOGDISABLE) 
	{
		dwFog = 0; 
	}

	if (pObject->m_Flags2 & FLAG2_ADDITIVE) 
	{
		srcBlend	= D3DBLEND_ONE;
		destBlend	= D3DBLEND_ONE;
		dwFogColor	= 0; 
	}
	else if (pObject->m_Flags2 & FLAG2_MULTIPLY) 
	{
		srcBlend	= D3DBLEND_ZERO;
		destBlend	= D3DBLEND_SRCCOLOR;
		dwFogColor	= 0xFFFFFFFF; 
	}
	else 
	{
		srcBlend	= D3DBLEND_SRCALPHA;
		destBlend	= D3DBLEND_INVSRCALPHA; 
	}
}

void d3d_SetTranslucentObjectStates(bool bAdditive=FALSE);
void d3d_UnsetTranslucentObjectStates(bool bChangeZ);

// Draw a simple wireframe line.
void d3d_DrawLine(const LTVector& src, const LTVector& dest, uint32 color);

// Given the endpoint colors it will render a line that blends between the colors
void d3d_DrawLine(const LTVector& src, const LTVector& dest, uint32 color1, uint32 color2);

// This will render an axis aligned box at the specified position
void d3d_DrawWireframeBox(const LTVector& Min, const LTVector& Max, uint32 nColor);

void d3d_SetD3DMat(D3DTRANSFORMSTATETYPE iTransform, const LTMatrix *pMat);

// Change the gamma ramps
void d3d_SetGamma(const LTVector &vGamma);

//handles clearing the default render target
void d3d_Clear(LTRect *pRect, uint32 flags, LTRGBColor& ClearColor);

//Called to render the world and world models with the specified aggregate shader.
//This should be called after all the object queues have already been filled, and the
//world rendered.
bool d3d_RenderWorldWithAggregate(uint32 nNumFrustumPlanes, const LTPlane* pFrustum,
								 const ViewParams& Params, IAggregateShader* pShader);

//sets up the matrices in the specified view parameter structure
void d3d_SetD3DTransformStates(const ViewParams& Params);


#endif  // __D3D_DRAW_H__



