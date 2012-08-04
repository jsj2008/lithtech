
#ifndef __GL_MODELRENDER_H__
#define __GL_MODELRENDER_H__

#include "model.h"
#include "dtxmgr.h"
#include "ltaModel.h"
#include <GL/gl.h>
#include <GL/glu.h>

typedef void* GLM_CONTEXT;

#define MAX_GLM_LIGHTS		8
#define NUM_ANIM_INFOS		4	// Max number of animations that can be tracked at once.


// ------------------------------------------------------------------------
// GLMContext
// current GL rendering context. 
// ------------------------------------------------------------------------
class GLMContext
{
public:
	HWND	m_hWnd;
	HDC		m_hDC;
	HGLRC	m_hglrc;
	CMoArray<GLuint> m_Textures;
	GLuint m_nCurTexture;

	float m_bgColor[3]; // gl background clear color

} ;

// ------------------------------------------------------------------------
// DrawStruct
// Model-Scene parameters.
// ------------------------------------------------------------------------
struct DrawStruct
{
	
public:
	
	BOOL		SetupTransformMaker(TransformMaker *pMaker);

	// Returns how many (consecutive) AnimTrackers in m_Times are valid.
	DWORD		NumValidTimes();


public:

	Model*		GetModel()	{return m_Times[0]->m_TimeRef.m_pModel;}

	// The exact place to draw from.
	AnimTracker	*m_Times[NUM_ANIM_INFOS];
	
	DWORD	m_iLOD;
	float	m_CurrentLODDist ;

	// VIEW PARAMS
	float	m_FOV;
	CVector	m_ViewerPos;
	CVector	m_LookAt;

	
	// LIGHT PARAMS
	CVector	m_LightPositions[MAX_GLM_LIGHTS];
	// RGB 0-255
	CVector	m_LightColors[MAX_GLM_LIGHTS];
	int		m_nLights;
	

	BOOL	m_bDims;
	CVector m_DimsColor;
	CVector m_ModelBoxColor;
	BOOL	m_bAnimBox;
	CVector m_AnimBoxColor;
	
	// --------------------------------------
	// DRAW FLAGS 
	// --------------------------------------
	BOOL	m_bWireframe;
	BOOL	m_bNormals;
	// Draw the model bright (without lighting).
	BOOL	m_bDrawBright;
    BOOL	m_bDrawVertexWeights ;

	// Draw the skeleton.
	BOOL	m_bDrawSkeleton;
	BOOL    m_bDrawOBB ;
	// draw the original mesh
	BOOL	m_bDrawOriginalModel;

	// draw the model using movement encoding
	bool	m_bMovementEncoding;

	// show textures
	BOOL	m_bDrawTextures;

	// Show sockets and/or attachments.
	BOOL	m_bDrawSockets;
	BOOL	m_bDrawAttachments;
	BOOL	m_bSolidAttachments;
	
	// calculate the internal radius of the model while rendering
	bool	m_bCalcRadius;
	// filled in with the model radius if m_bCalcRadius is set
	float	m_fModelRadius;
	// if set this will cause it to calculate radius AND draw the animation
	bool	m_bCalcAndDraw;

	// Show profile info.
	BOOL	m_bProfile;

	BOOL	m_bShowNormalRef;

	BYTE	*m_SelectedPieces; // Should be as big as Model::NumPieces().
	BYTE	*m_SelectedNodes; // Should be as big as Model::NumNodes().
	
	
};

						   
// Represents a transformed vertex.
class TVert
{
public:
	inline TVert&	operator=(TVert other)
	{
		m_vPos = other.m_vPos;
		m_vNormal = other.m_vNormal;
		m_vColor = other.m_vColor ;
		return *this;
	}
	
	LTVector		m_vPos;
	LTVector		m_vNormal;
	float			m_UV[2];
	LTVector		m_vColor ;
};

// ------------------------------------------------------------------------
// Free Floatting functions 
// ------------------------------------------------------------------------

// Init and term.
// Note: your window must have WS_CLIPCHILDREN and WS_CLIPSIBLINGS set.
GLM_CONTEXT CreateGLMContext(void *hWnd);
void DeleteGLMContext(GLM_CONTEXT hContext);

// The drawing function.
// Draws the model centered at the origin.
// pEyePoint is the position of the viewer.
// pLookAt is where it looks at (it should probably be (0,0,0)).
void DrawModel(GLM_CONTEXT hContext, DrawStruct *pStruct);

// ------------------------------------------------------------------------
// DrawMetaModel test the raw lta file
void DrawMetaModel( GLM_CONTEXT, DrawStruct * );


void DrawCoordSys(DMatrix *pMat, float arrowLength);
void DrawWorldCoordSys();

// ------------------------------------------------------------------------
// setup viewing parameters, sets up lookat, lights, wireframe params etc
// for gl
void SetupViewingParameters( GLMContext *pContext, DrawStruct *);

// ------------------------------------------------------------------------
// Set the texture to draw with.
BOOL SetGLMTexture(GLM_CONTEXT hContext, TextureData *pTexture, DWORD nTextureNum = 0);
// get rid of animated textures.
void ReleaseAllTextures( GLM_CONTEXT hContext );

// set the background color for the gl-window.
void SetGLMBackgroundColor( GLM_CONTEXT , COLORREF );
void SetGLMBackgroundColor( GLM_CONTEXT hContext, float *color );

/* The coordinate system this uses...

				 +y
					  -z
				 ^	 /
				 |	/
				 | /
				 |/
	-x ----------|----------> +x
				/|
			   / |
			  +z |
*/

// updates the draw-buffer vertex and animation .
BOOL AlternateGetVertexPositions(	Model *pModel,
 									GVPStruct *pStruct, 
									BOOL bSetupTransforms,
									BOOL bNormals,
									BOOL bDrawOriginalModel,
									BOOL bMovementEncoding,
									void **pNextVert = NULL );

#endif  // __GL_MODELRENDER_H__
