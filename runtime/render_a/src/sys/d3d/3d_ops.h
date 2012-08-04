
// This module defines lots of the main 3d operations like transformation,
// clipping, projection, rejection tests, etc..

#ifndef __3D_OPS_H__
#define __3D_OPS_H__

#ifndef __RENDERERCONSOLEVARS_H__
#	include "rendererconsolevars.h"
#endif

#ifndef __D3D_VIEWPARAMS_H__
#include "d3d_viewparams.h"
#endif

// DEFINES
#define ROTATION_MAX	100000.0f

#define CLIP_EPSILON	0.00001f

#define CPLANE_NEAR_INDEX		0
#define CPLANE_FAR_INDEX		1
#define CPLANE_LEFT_INDEX		2
#define CPLANE_TOP_INDEX		3
#define CPLANE_RIGHT_INDEX		4
#define CPLANE_BOTTOM_INDEX		5

#define NUM_CLIPPLANES			6

// Near Z is always this.
#define NEARZ			7.0f
#define MAX_FARZ		500000.0f

// STRUCTURES
struct TLRGB
{
	uint8 b;
	uint8 g;
	uint8 r;
	uint8 a;
};

struct RGBColor
{
	union
	{
		TLRGB rgb;
		uint32 color;
	};
};

// The formats for the vertices.
#define TLVERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

// Our internal structure for a D3DTLVERTEX which makes it easier to
// access the color values.
class TLVertex
{
public:

	void SetTCoords(float inTU, float inTV)
	{
		tu = inTU;
		tv = inTV;
	}

	LTVector m_Vec;
	float rhw;

	union
	{
		TLRGB rgb;
		uint32 color;
	};

	float tu, tv;
};

//class to hold information about the really close state change
class CReallyCloseData
{
public:
	D3DVIEWPORT9	m_OldViewport;
	D3DMATRIX		m_OldProj;
	D3DMATRIX		m_OldView;
};

// ---------------------------------------------------------------- //
// Externs.
// ---------------------------------------------------------------- //

extern ViewParams g_ViewParams;
class RTexture;


// ---------------------------------------------------------------- //
// Functions.
// ---------------------------------------------------------------- //

// Setup the transformation for an object.
void d3d_SetupTransformation(const LTVector *pPos, float *pRotation, LTVector *pScale, LTMatrix *pMat);

// Calculate the sum of the lights touching this leaf.
void d3d_CalcLightAdd(LTObject *pObject, LTVector *pLightAdd);

// Setup the renderer for really close drawing
void d3d_SetReallyClose(CReallyCloseData* pData);

//unsets the renderer from really close rendering
void d3d_UnsetReallyClose(CReallyCloseData* pData);

//Given a stage to install it on, it will grab the global world envmap transform
//and apply it into the texture transform on that stage
void d3d_SetEnvMapTransform(RTexture* pEnvMap, uint32 nStage);

//Given a stage, this will restore the parameters appropriately so as not to mess up any other sections
void d3d_UnsetEnvMapTransform(uint32 nStage);

//given a stage, it will set up the bump transform appropriately
void d3d_SetBumpMapTransform(uint32 nStage, float fTextureScale);


DWORD d3d_VectorToRGB(LTVector * vector);


inline void d3d_SetupTransformation(const LTVector *pPos, float *pRotation, LTVector *pScale, LTMatrix *pMat)
{
	if(pRotation[0] > ROTATION_MAX || pRotation[0] < -ROTATION_MAX)
		pRotation[0] = 0.0f;

	if(pRotation[1] > ROTATION_MAX || pRotation[1] < -ROTATION_MAX)
		pRotation[1] = 0.0f;

	if(pRotation[2] > ROTATION_MAX || pRotation[2] < -ROTATION_MAX)
		pRotation[2] = 0.0f;

	if(pRotation[3] > ROTATION_MAX || pRotation[3] < -ROTATION_MAX)
		pRotation[3] = 1.0f;

	quat_ConvertToMatrix(pRotation, pMat->m);

	pMat->m[0][0] *= pScale->x;
	pMat->m[1][0] *= pScale->x;
	pMat->m[2][0] *= pScale->x;

	pMat->m[0][1] *= pScale->y;
	pMat->m[1][1] *= pScale->y;
	pMat->m[2][1] *= pScale->y;

	pMat->m[0][2] *= pScale->z;
	pMat->m[1][2] *= pScale->z;
	pMat->m[2][2] *= pScale->z;

	pMat->m[0][3] = pPos->x;
	pMat->m[1][3] = pPos->y;
	pMat->m[2][3] = pPos->z;
}

#endif


