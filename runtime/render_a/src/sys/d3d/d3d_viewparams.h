#ifndef __D3D_VIEWPARAMS_H__
#define __D3D_VIEWPARAMS_H__

#ifndef __LTPLANE_H__
#include "ltplane.h"
#endif

#ifndef __AABB_H__
#include "aabb.h"
#endif


#define NUM_CLIPPLANES	6


// This structure holds the info that is used to precalculate a bunch
// of view frustum stuff.  All its member are specified in camera
// space (translated and rotated so the camera is looking down Z).
struct ViewBoxDef
{
	LTVector	m_COP;				// Center of projection.
	float		m_WindowSize[2];	// Half-width and half-height of view window.
	float		m_NearZ, m_FarZ;	// Near and far Z (in relation to m_COP.z).
};


// The global viewing parameters.
class ViewParams
{
public:

	bool		ViewAABBIntersect(const LTVector &vBoxMin, const LTVector &vBoxMax) const;

public:

	ViewBoxDef	m_ViewBox; // Just copied from the input to d3d_Initfrustum.
	
	LTRect		m_Rect;

	// Screen/projection info.  
	
	float		m_fScreenWidth, m_fScreenHeight;

	float		m_NearZ;	// Near clip Z.
	float		m_FarZ;		// Maximum far Z.

	// Viewer matrix (passed into d3d_Initfrustum), this is the view to world transform
	LTMatrix	m_mInvView;
	
	// World space to camera space (puts camera at origin and aligns to XYZ axes).
	// (D3DTRANSFORMSTATE_VIEW)
	LTMatrix	m_mView;

	// These are the same as the d3d matrices.		
	// (D3DTRANSFORMSTATE_PROJECTION)
	LTMatrix	m_mProjection;

	//This is the matrix that should be used for the world environment map transform
	//using so that it is not view orientation dependant
	LTMatrix	m_mWorldEnvMap;

	// m_DeviceTransform * mProjectionTransform.
	LTMatrix	m_DeviceTimesProjection;

	// All the above matrices put together.. use this when drawing unclipped things.
	LTMatrix	m_FullTransform;

	// Identity matrix.
	LTMatrix	m_mIdentity;

	// Current inverse world transform
	LTMatrix	m_mInvWorld;

	// World space view frustum points.
	LTVector	m_ViewPoints[8];

	// World space AABB of the view frustum
	LTVector	m_ViewAABBMin, m_ViewAABBMax;

	// Planes defining the view frustum in world space.	
	LTPlane		m_ClipPlanes[NUM_CLIPPLANES];

	// Corner of an AABB that should be checked for intersection on each of the planes
	EAABBCorner m_AABBPlaneCorner[NUM_CLIPPLANES];
	
	// Viewer orientation and vectors.
	LTVector	m_Up, m_Right, m_Forward;

	// Viewer position.
	LTVector	m_Pos;

	// Sky viewer position.
	LTVector	m_SkyViewPos;

	// What rendering mode we are in
	enum	ERenderMode	{	eRenderMode_Normal,
							eRenderMode_Glow
						};

	ERenderMode	m_eRenderMode;
};





#endif