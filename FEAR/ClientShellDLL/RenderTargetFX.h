 // ----------------------------------------------------------------------- //
//
// MODULE  : RenderTargetFX.h
//
// PURPOSE : Client side implementation of the RenderTarget object which is
//			 used to render scenes to a texture and display them on a material
//
// CREATED : 8/4/03
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __RENDERTARGETFX_H__
#define __RENDERTARGETFX_H__

#ifndef __SPECIAL_FX_H__
#	include "SpecialFX.h"
#endif

#ifndef __ILTRENDERER_H__
#	include "iltrenderer.h"
#endif

class CRenderTargetGroupFX;

//the maximum number of render targets that can be created within a level
#define MAX_RENDERTARGETFX_OBJECTS	30

//------------------------------------------------------------------------------
//the creation structure that is read from the server side object and used for
//the creation of the client side object
struct RENDERTARGETCREATESTRUCT : 
	public SFXCREATESTRUCT
{
    RENDERTARGETCREATESTRUCT()
	{
		m_nRenderTargetGroupID		= 0;
		m_nRenderTargetLOD			= 0;
		m_bMirror					= false;
		m_bRefraction				= false;
		m_fRefractionClipPlaneBias	= 0.0f;
		m_vFOV.Init(MATH_DEGREES_TO_RADIANS(90.0f), MATH_DEGREES_TO_RADIANS(90.0f));
	}

    //the number of frames to wait before rendering the render target again
	uint32		m_nRenderTargetGroupID;

	//the render target LOD
	uint32		m_nRenderTargetLOD;

	//the update frequency of this render target (in frames, 0 means skip no frames, so render each frame)
	uint32		m_nUpdateFrequency;

	//the update offset of this render target to allow for staggering (in frames)
	uint32		m_nUpdateOffset;

	//the field of view for the camera
	LTVector2	m_vFOV;

	//flag indicating whether or not this is a mirror render target
	bool		m_bMirror;

	//flag indicating whether or not this is a refraction render target
	bool		m_bRefraction;

	//biasing distance for the clip plane when rendering a refraction target
	float		m_fRefractionClipPlaneBias;

	//the material that this applies to
	std::string	m_sMaterial;

	//the parameter within the material that this applies to
	std::string m_sParam;
};

//------------------------------------------------------------------------------
// The actual game side object that handles the render target and updating it
class CRenderTargetFX : 
	public CSpecialFX
{
public :

	CRenderTargetFX();
	~CRenderTargetFX();

    virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
    virtual bool Update();
    virtual bool CreateObject(ILTClient* pClientDE);

	//get the ID of this special effect object, used for identifying object types dynamically
	virtual uint32 GetSFXID() { return SFX_RENDERTARGET_ID; }

	//this function should be called to force a render target update, for example if the device
	//is lost with an Alt+Tab
	void	DirtyRenderTarget()			{ m_bDirtyRenderTarget = true; }

	//this function will determine if the render target was visible, and if so will dirty it
	void	DirtyIfVisible();

	//this call will update the render target by rendering a scene. This call MUST be made within
	//a begin3d/end3d block otherwise it will fail
	bool	UpdateRenderTarget(const LTRigidTransform& tCamera, const LTVector2& vCameraFOV);

	//called to increment the current frame code used by the render targets for synchronization
	static void	IncrementCurrentFrame()	{ s_nCurrentFrame++; }

private :

	//this will render the scene to the render target using the camera object
	bool		UpdateRenderTargetCamera(HRENDERTARGET hRenderTarget, bool bCubeMap);

	//this will update the scene to render a mirror render target
	bool		UpdateRenderTargetMirror(HRENDERTARGET hRenderTarget, const LTRigidTransform& tCamera, const LTVector2& vCameraFOV);

	//this will update the scene to render a refraction render target
	bool		UpdateRenderTargetRefraction(HRENDERTARGET hRenderTarget, const LTRigidTransform& tCamera, const LTVector2& vCameraFOV);

	//called to bind the render target to the specified material parameter
	bool		BindToMaterialParam();

	//called to unbind from the material parameter that it is currently bound to
	bool		UnbindMaterialParam();

	//called to verify that the render target group is valid. This will attempt to set it up if it is
	//invalid, and will return whether or not the render target group can be used
	bool		SetupRenderTargetGroup();

	//called internally to handle either enabling or disabling the render target based upon
	//the provided parameter
	void		EnableRenderTarget(bool bEnable);

	//the render target visibility query that we can use to determine when our render target is 
	//visible
	HRENDERTARGETVIS		m_hRenderTargetVis;

	//the render target group we are linked to. Note that this can be NULL if the render target
	//group could not be found (common due to the indeterminate object construction order due
	//to networking)
	CRenderTargetGroupFX*	m_pRenderTargetGroup;

	//the current LOD level of this render target. 0 = low, 1 = medium, 2 = high
	uint32		m_nRenderTargetLOD;

	//flag indicating whether or not this render target is currently bound to a material param
	bool		m_bBoundToMaterialParam;

	//a dirty flag that if set indicates that the render target needs to be re-rendered
	bool		m_bDirtyRenderTarget;

	//the ID of the render target group that will provide our surface
	uint32		m_nRenderTargetGroupID;

	//the update frequency of this render target (in frames, 0 means skip no frames, so render each frame)
	uint32		m_nUpdateFrequency;

	//the update offset of this render target to allow for staggering (in frames)
	uint32		m_nUpdateOffset;

	//the field of view for the camera
	LTVector2	m_vFOV;

	//flag indicating whether or not this is a mirror render target
	bool		m_bMirror;

	//flag indicating whether or not this is a refraction render target
	bool		m_bRefraction;

	//biasing distance for the clip plane when rendering a refraction target
	float		m_fRefractionClipPlaneBias;

	//the material that this applies to
	std::string	m_sMaterial;

	//the parameter within the material that this applies to
	std::string m_sParam;

	//a system global frame code used by the render targets to handle synchronization of when they
	//should be dirtied so that render targets can be reliably staggered
	static uint32	s_nCurrentFrame;
};

#endif // __RENDERTARGETFX_H__
