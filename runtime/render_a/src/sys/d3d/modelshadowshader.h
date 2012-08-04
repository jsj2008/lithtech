#ifndef __MODELSHADOWSHADER_H__
#define __MODELSHADOWSHADER_H__

#ifndef __IAGGREGATESHADER_H__
#	include "iaggregateshader.h"
#endif

// ------------------------------------------------------------------------
// ShadowLightInfo
// This class contains the info that tells how the shadow is projected
// onto the world
// ------------------------------------------------------------------------
class ShadowLightInfo
{
public:

	enum { eNumShadowLightFrustumPlanes = 6 };

	LTVector	m_vLightOrigin;
	LTVector	m_Vecs[3];		// Coordinate system for texture.
	LTPlane		m_ProjectionPlane;
	LTVector	m_vProjectionCenter;
	LTVector	m_vWindowTopLeft;
	float		m_fSizeX;
	float		m_fSizeY;
	
	// Shadowed polies are clipped into here.
	LTPlane		m_FrustumPlanes[eNumShadowLightFrustumPlanes];
};

class CModelShadowShader :
	public IAggregateShader
{
public:

	//access for the singleton object
	static CModelShadowShader*		GetSingleton();



	CModelShadowShader();
	virtual ~CModelShadowShader();

	//Initialization call. This is called before any calls are made to this shader, this
	//allows it to perform operations such as initialization of vertex shaders, and other
	//states that are constant. If it returns false, nothing should call render...
	virtual bool	BeginRendering();

	//called when rendering is completed. This should return false if any errors occurred
	//while rendering
	virtual bool	EndRendering();

	//called to bind any device specific data to this object
	virtual bool	Bind();

	//called to free any device specific data to this object
	virtual bool	Release();

	//determines if this device can support perspective projection for shadows
	bool			CanSupportPerspective();

	//Called when the world transform changes. This will be called whenever the
	//rendering switches spaces and gives the shader a chance to transform any
	//data it is using
	virtual bool	SetWorldTransform(const LTMatrix& mInvWorldTrans);

	//called when beginning a shader. This can be used to do shader specific tasks such
	//as setting up the vertex shader, the vertex buffers, etc.
	virtual bool	BeginShader(		uint32 nVertSize,				//the size of the vertices
										uint32 nVertType,				//the type of vertex as passed to D3D
										IDirect3DVertexShader9 *pShader, // for DX9 FVF is separate
										IDirect3DVertexBuffer9 *pVB,	//pointer to the vertex buffer
										IDirect3DIndexBuffer9 *pIB,		//the index buffer
										ERenderShader eShaderID			//the ID of the shader for custom setup
									 );

	//called to render a shader block. It takes in all data needed to render the underlying
	//geometry as well as the ID of the shader type so that it can perform custom rendering
	//operations upon it
	virtual bool	RenderSection(		D3DPRIMITIVETYPE nPrimType,		//primitive type being rendered, D3DPT_, must be indexed
										const uint16 *pIndices,			//the original index lists
										const SRBVertex *pVertices,		//the original vertex lists
										uint32 nTotalIndexCount,		//number of primitives being rendered
										uint32 nStartVert,				//index into the vertex buffer to begin on
										uint32 nTotalVertCount,			//number of vertices
										int32 nVertOffset,				//offset of the vertex in the original vertices list
										uint32 nStartIndex				//index into the index buffer to begin on
					);

	//called to set up the shadow information
	void			SetShadowInfo(	const ViewParams& Params,			//the view params for rendering
									IDirect3DBaseTexture9* pTexture,	//the texture for the shadow
									const ShadowLightInfo* pInfo,		//the struct containing info for the shadow projection
									float fXTexScale, float fYTexScale,	//scale of the texture to the area
									bool bPerspective,					//ortho or perspective
									float fFarClip,						//the maximum distance that this shadow can project
									float fFadeOffset					//offset of the fade beginning to compensate for model dimensions
					);


private:

	//restores the render states that were modified
	bool		RestoreRenderStates();

	//determines the Z bias given the specified shader
	uint32		GetZBias(ERenderShader eShaderID) const;

	//determines if the specified shader should be excluded from being shadowed
	bool		IsExcluded(ERenderShader eShaderID) const;

	//sets up all render states
	bool		SetRenderStates();

	//determine if we are rendering or not (used for sanity checks)
	bool		m_bRendering;

	//determines if an error occurred during rendering
	bool		m_bErrorOccurred;

	//the texture that we are using for the actual shadow texture
	IDirect3DBaseTexture9*	m_pShadowTexture;

	//the matrix to convert the world points to the projector space
	D3DXMATRIX				m_mWorldToProjector;

	//the matrix that handles the texture transform for the fade texture
	D3DXMATRIX				m_mFadeTex;

	//the position of the projector for culling
	LTVector				m_vProjectorPos;

	//the vector used for the plane of the points. Note that the normal is scaled
	//so that a dot past 1 is beyond the far plane
	LTPlane					m_ProjectorPlane;

	//flag indicating whether or not this has been bound to a device yet
	bool					m_bBound;

	// The temporary index buffer used for backface culling
	IDirect3DIndexBuffer9*	m_pTempIB;
	uint32					m_nTempIBIndex;

	//whether or not we are doing perspective or ortho projection
	bool					m_bPerspective;

	//the maximum distance that this can project
	float					m_fProjDist;

	//texture used for fading out the shadow at the vertices
	IDirect3DTexture9		*m_pFadeTexture;

	//the length of the fade texture
	uint32					m_nFadeTextureX;

	//the bounding sphere that encompasses the area the shadow projects onto
	LTVector				m_vBSphereCenter;
	float					m_fBSphereRad;

	//the bounding sphere center transformed into the new world space
	LTVector				m_vTransBSphereCenter;

	// Number of triangles the temporary index buffer can handle
	enum { k_nTempIBTris = 4096 };
	enum { k_nTempIBSize = k_nTempIBTris * 3 };

	//flags to hold information on whether or not we have validated the device for
	//supporting perspective projection
	bool					m_bValidatedPerspective;
	bool					m_bFailedValidation;

	//---------------------------------------------
	// Render state backups for restoring. Only to be used to reset states
	DWORD					m_nSBFogEnable;

};


#endif

