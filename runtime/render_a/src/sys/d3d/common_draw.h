
#ifndef __COMMON_DRAW_H__
#define __COMMON_DRAW_H__




#define MAX_VISIBLE_LIGHTS	64


struct RenderContext;
struct SceneDesc;
class  DynamicLight;

#ifndef __3D_OPS_H__
#include "3d_ops.h"
#endif


// ---------------------------------------------------------------- //
// Externs.
// ---------------------------------------------------------------- //

extern uint16 g_CurFrameCode;
extern uint32 g_CurObjectFrameCode;

extern SceneDesc *g_pSceneDesc;

extern bool g_have_world;

// The list of lights that will be used to light the world
extern DynamicLight *g_WorldDynamicLights[MAX_VISIBLE_LIGHTS];
extern uint32 g_nNumWorldDynamicLights;

// The list of dynamic lights that will be used to light objects
extern DynamicLight *g_ObjectDynamicLights[MAX_VISIBLE_LIGHTS];
extern uint32 g_nNumObjectDynamicLights;

// This is so we can inline some things.
#include "d3d_draw.h"


// ---------------------------------------------------------------- //
// Functions.
// ---------------------------------------------------------------- //

bool d3d_InitFrame(ViewParams* pParams, SceneDesc *pDesc);

// Init the view box.
void d3d_InitViewBox(ViewBoxDef *pDef,
	float nearZ, float  farZ, float xFov, float yFov);

// This can be used for recursive rendering.  Pass in the screen extents of the box
// you want it to render into and it sets up the view box accordingly.
void d3d_InitViewBox2(ViewBoxDef *pDef, 
	float nearZ, float farZ,
	const ViewParams& PrevParams,
	float screenMinX, float screenMinY, 
	float screenMaxX, float screenMaxY);

bool d3d_InitFrustum(ViewParams *pParams, 
	float xFov, float yFov, 
	float nearZ, float farZ, 
	float screenMinX, float screenMinY, float screenMaxX, float screenMaxY,
	const LTVector *pPos, const LTRotation *pRotation, ViewParams::ERenderMode eMode);

bool d3d_InitFrustum2(ViewParams *pParams, 
	ViewBoxDef *pViewBox, 
	float screenMinX, float screenMinY, float screenMaxX, float screenMaxY,
	const LTMatrix *pMat, const LTVector& vScale, ViewParams::ERenderMode eMode);

void d3d_IncrementFrameCode(RenderContext *pContext);

// ---------------------------------------------------------------- //
// Inlines.
// ---------------------------------------------------------------- //

inline void d3d_SetFPState()
{
#ifdef _M_IX86
	short control;

	_asm {	
		fstcw	control		 // Get FPU control word
		and	control, 0xfcff  // PC field = 00 for single precision	
		fldcw   control	
	}
#endif
}

#endif  // __COMMON_DRAW_H__


