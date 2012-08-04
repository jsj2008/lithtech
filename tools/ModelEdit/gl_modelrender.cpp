//  ----------------------------------------------------------------
//  * setup gl rendering context
//  * render the model using open gl
//  * animation to mesh updates
//  ----------------------------------------------------------------
#include "precompile.h"
#pragma warning (disable:4786)

#include "gl_modelrender.h"
#include "model_ops.h"
#include "pixelformat.h"
#include "geomroutines.h"
#include "transformmaker.h"
#include "model_cleanup.h"
#include <gl/gl.h>
#include <gl/glu.h>
#include <set>
#include "ltascene.h"
#include <winbase.h>


using namespace std;
// ------------------------------------------------------------------------

//  global ambient color ui definable global ambient color
float g_GlobalAmbientColor[3] = { 0.2f , 0.2f , 0.2f };

#define	GLM_TRI_INDEX0	2
#define	GLM_TRI_INDEX1	1
#define	GLM_TRI_INDEX2	0

#define PRIMARY_TEXTURE 1
#define SPECULAR_TEXTURE 0
#define ENVIRONMENTAL_TEXTURE 0

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")




// ------------------------------------------------------------------------
// StateSet
// pushes and pops GL attribs ..
// examp: 
// StateSet zbuf( GL_DEPTH_TEST , FALSE );
// constructor either enables or disables option, destructor does the opposite
// ------------------------------------------------------------------------
class StateSet
{
	int			m_Option;
	int			m_OldVal;
	
	void glChooseEnable(int option, BOOL bEnable)
	{
		if(bEnable)
			glEnable(option);
		else
			glDisable(option);
	}

public:
	StateSet(int option, BOOL bEnable)
	{
		m_Option = option;
		m_OldVal = glIsEnabled(option);
		glChooseEnable(option, bEnable);
	}

	~StateSet()
	{
		glChooseEnable(m_Option, m_OldVal);
	}
};


// ------------------------------------------------------------------------
// vertex blend ( result, a-vec, b-vec, interp )
// ------------------------------------------------------------------------
static inline
void vertexLerp(float *res, float *a, float *b, float &t )
{
    res[0] =  a[0]  + ( (b[0] - a[0]) * t) ;
    res[1] =  a[1]  + ( (b[1] - a[1]) * t) ;
    res[2] =  a[2]  + ( (b[2] - a[2]) * t) ;
}

//  ----------------------------------------------------------------
//  TVert* UpdateVAModelPiece()
// returns pointer to the current position in the vertertex draw buffer.
//  
//  ----------------------------------------------------------------
TVert* UpdateVAModelPiece( Model      *pModel,
                         ModelPiece *pModelPiece,
                         AnimTimeRef*pAnimTimeRef   ,
                         TVert      *pVertexBuffer ,
                         const LTMatrix   *trans)
{
	PieceLOD  *pLOD = pModelPiece->GetLOD(uint32(0));
    ModelAnim *pAnims[2];
    NodeKeyFrame *pKeys[2];
    AnimNode  *pAnimNodes[2] ;

    pAnims[0] = pModel->GetAnim( pAnimTimeRef->m_Prev.m_iAnim );
    pAnims[1] = pModel->GetAnim( pAnimTimeRef->m_Cur.m_iAnim );
    
    pAnimNodes[0] = pAnims[0]->m_AnimNodes[ pModelPiece->m_vaAnimNodeIdx ];
    pAnimNodes[1] = pAnims[1]->m_AnimNodes[ pModelPiece->m_vaAnimNodeIdx ];
    
    pKeys[0] = &pAnimNodes[0]->m_KeyFrames[pAnimTimeRef->m_Prev.m_iFrame];
    pKeys[1] = &pAnimNodes[1]->m_KeyFrames[pAnimTimeRef->m_Cur.m_iFrame];
    
    // get vertex data from key.
    // to do interp between frames ...
    int iVBOffset = pModelPiece->m_VertOffset ;
    int size      = pLOD->NumVerts() + iVBOffset ;
				
    int v_cnt = 0 ;
    CDefVertexLst *pDefVerts[2];
    pDefVerts[0] = pKeys[0]->m_pDefVertexLst ;
    pDefVerts[1] = pKeys[1]->m_pDefVertexLst ;
    float percent = pAnimTimeRef->m_Percent ;

    if( pDefVerts[1] == NULL )
        pDefVerts[1] = pDefVerts[0] ;
    
    // get the vertex animation  multiply it by current transforms
    for( int i = iVBOffset ; i <  size ; i++, v_cnt++ )
    {
        float *prv_val = pDefVerts[0]->getValue( v_cnt ); 
        float *cur_val = pDefVerts[1]->getValue( v_cnt ); 
        float val[3] ;
        vertexLerp( val, prv_val, cur_val, percent );
        
        LTVector vec( val[0], val[1], val[2] ) ;
        LTVector res;
        
     
        MatVMul( &res, trans, &vec );
				
        (*pVertexBuffer).m_vPos = res ;
        (*pVertexBuffer).m_vNormal = pLOD->m_Verts[v_cnt].m_Normal ;
		pVertexBuffer++;
    }	

	return pVertexBuffer ;
}


// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
static CMoArray<DVector> g_TransformedVerts;
static CMoArray<DVector*> g_PseudoIndices;
static float g_tuAdd, g_tvAdd;

GLfloat viewMatrix[16];


// ------------------------------------------------------------ //
// Set up the pixel format for windoze
// ------------------------------------------------------------ //
BOOL SetDCPixelFormat(HDC hDC)
{
	int nPixelFormat;
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1, // Version
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32, // Bit depth
		0, 0, 0, 0, 0, 0,
		0, 0,
		0, 0, 0, 0, 0,
		32, // ZBuffer size
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0 };

	nPixelFormat = ChoosePixelFormat(hDC, &pfd);
	if(nPixelFormat == 0)
		return FALSE;
		
	return SetPixelFormat(hDC, nPixelFormat, &pfd);
}

// ------------------------------------------------------------------------
// SetupViewingParameters( GL-context, draw-structure )
// sets up the camera view for the the gl-context, lights and stuff
// ------------------------------------------------------------------------
void SetupViewingParameters(GLMContext *pContext, DrawStruct * pDrawStruct )
{
	RECT rect;
	float aspect;
	GLfloat ambient[4], diffuse[4], lightPos[4];
	int i;

	wglMakeCurrent(pContext->m_hDC, pContext->m_hglrc);

	GetClientRect(pContext->m_hWnd, &rect);
	aspect = (float)(rect.right - rect.left) / (rect.bottom - rect.top);

	// Set the viewport.
	glViewport(0, 0, rect.right, rect.bottom);

	// Setup the projection matrix.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective((GLdouble)(pDrawStruct->m_FOV), aspect, 0.1f, 5000.0f);

	// Setup the ModelView matrix.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(EXPANDVEC(pDrawStruct->m_ViewerPos),
		EXPANDVEC(pDrawStruct->m_LookAt), 0.0f, 1.0f, 0.0f);

	// This puts it in Lithtech's (left-handed) coordinate system.
	glScalef(1.0f, 1.0f, -1.0f);	


    glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);
    


	// Clear the screen.
	glClearColor(pContext->m_bgColor[0],pContext->m_bgColor[1],
				 pContext->m_bgColor[2],0.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	// Gouraud...
	glShadeModel(GL_SMOOTH);


	if(pDrawStruct->m_bWireframe)
	{
		glDisable(GL_LIGHTING);

		glDisable(GL_CULL_FACE);

		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// No ZBuffering..
		glDisable(GL_DEPTH_TEST);
	}
	else
	{
		// Setup lighting.
		glEnable(GL_LIGHTING);

		// Lights!
		for(i=0; i < MAX_GLM_LIGHTS; i++)
		{
			if(i < pDrawStruct->m_nLights)
			{
				ambient[0] = (pDrawStruct->m_LightColors[i].x / 255.0f) * 0.0f;
				ambient[1] = (pDrawStruct->m_LightColors[i].y / 255.0f) * 0.0f;
				ambient[2] = (pDrawStruct->m_LightColors[i].z / 255.0f) * 0.0f;
				ambient[0] = g_GlobalAmbientColor[0];
				ambient[1] = g_GlobalAmbientColor[1];
				ambient[2] = g_GlobalAmbientColor[2];
			
				ambient[3] = 1.0f;

				diffuse[0] = (pDrawStruct->m_LightColors[i].x / 255.0f) * 0.7f;
				diffuse[1] = (pDrawStruct->m_LightColors[i].y / 255.0f) * 0.7f;
				diffuse[2] = (pDrawStruct->m_LightColors[i].z / 255.0f) * 0.7f;
	
				diffuse[3] = 1.0f;

				lightPos[0] = pDrawStruct->m_LightPositions[i].x;
				lightPos[1] = pDrawStruct->m_LightPositions[i].y;
				lightPos[2] = -pDrawStruct->m_LightPositions[i].z;
				lightPos[3] = 1.0f;

				glLightfv(GL_LIGHT0+i, GL_AMBIENT, ambient);
				glLightfv(GL_LIGHT0+i, GL_DIFFUSE, diffuse);
				glLightfv(GL_LIGHT0+i, GL_POSITION, lightPos);
				glEnable(GL_LIGHT0+i);
			}
			else
			{
				glDisable(GL_LIGHT0+i);
			}
		}


		// Cull faces.
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);

		// Set the polygon mode.
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// ZBuffering on.
		glEnable(GL_DEPTH_TEST);
	}
}

// ------------------------------------------------------------ //
// CreateGLMContext( anonymous-window )
// Creates a GL-context from a window.
// ------------------------------------------------------------ //
GLM_CONTEXT CreateGLMContext(void *phWnd)
{
	GLMContext *pContext;
	HDC hDC;
	HWND hWnd;
	char buffer[2048];

	// Set the pixel format.
	hWnd = (HWND)phWnd;
	hDC = GetDC(hWnd);
	if(!SetDCPixelFormat(hDC))
	{
		ReleaseDC(hWnd, hDC);
		return NULL;
	}	

	pContext = new GLMContext;
	pContext->m_hWnd = hWnd;
	pContext->m_hDC = hDC;
	pContext->m_hglrc = wglCreateContext(pContext->m_hDC);
	pContext->m_nCurTexture = 0;
	wglMakeCurrent(pContext->m_hDC, pContext->m_hglrc);

    sprintf( buffer, "OEM:\t\t%s  \n  Version:\t%s\n  Renderer:\t%s\n", glGetString( GL_VENDOR ), glGetString( GL_VERSION ), glGetString( GL_RENDERER ) );
	return pContext;
}

// ------------------------------------------------------------------------
// Kill gl- context
// ------------------------------------------------------------------------
void DeleteGLMContext(GLM_CONTEXT hContext)
{
	GLMContext *pContext;

	pContext = (GLMContext*)hContext;

	// Delete the texture bindings
	for (DWORD i = 0; i < pContext->m_Textures.GetSize(); i++)
		if (pContext->m_Textures[i])
			glDeleteTextures(1, &(pContext->m_Textures[i]));

	wglMakeCurrent(pContext->m_hDC, NULL);
	wglDeleteContext(pContext->m_hglrc);
	ReleaseDC(pContext->m_hWnd, pContext->m_hDC);

	delete pContext;
}

// ------------------------------------------------------------------------
// Get rid of all textures. Do this before loading a model to start 
// fresh.. .
// ------------------------------------------------------------------------
void ReleaseAllTextures( GLM_CONTEXT hContext )
{
	GLMContext *pContext = (GLMContext*)hContext;

		// Delete the texture bindings
	for (DWORD i = 0; i < pContext->m_Textures.GetSize(); i++)
		if (pContext->m_Textures[i])
			glDeleteTextures(1, &(pContext->m_Textures[i]));
	pContext->m_Textures.Term();
}

// ------------------------------------------------------------------------
// DrawBox( color, min, max );
// ------------------------------------------------------------------------
void DrawBox (DVector *pColor, DVector *pMin, DVector *pMax)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glColor3f (pColor->x, pColor->y, pColor->z);
	glBegin (GL_LINES);
		// top
		glVertex3f (pMin->x, pMin->y, pMin->z);
		glVertex3f (pMax->x, pMin->y, pMin->z);

		glVertex3f (pMax->x, pMin->y, pMin->z);
		glVertex3f (pMax->x, pMin->y, pMax->z);

		glVertex3f (pMax->x, pMin->y, pMax->z);
		glVertex3f (pMin->x, pMin->y, pMax->z);

		glVertex3f (pMin->x, pMin->y, pMax->z);
		glVertex3f (pMin->x, pMin->y, pMin->z);

		// bottom
		glVertex3f (pMax->x, pMax->y, pMax->z);
		glVertex3f (pMin->x, pMax->y, pMax->z);
		
		glVertex3f (pMin->x, pMax->y, pMax->z);
		glVertex3f (pMin->x, pMax->y, pMin->z);

		glVertex3f (pMin->x, pMax->y, pMin->z);
		glVertex3f (pMax->x, pMax->y, pMin->z);

		glVertex3f (pMax->x, pMax->y, pMin->z);
		glVertex3f (pMax->x, pMax->y, pMax->z);

		// sides

		glVertex3f (pMin->x, pMin->y, pMin->z);
		glVertex3f (pMin->x, pMax->y, pMin->z);

		glVertex3f (pMax->x, pMin->y, pMin->z);
		glVertex3f (pMax->x, pMax->y, pMin->z);

		glVertex3f (pMax->x, pMin->y, pMax->z);
		glVertex3f (pMax->x, pMax->y, pMax->z);

		glVertex3f (pMin->x, pMin->y, pMax->z);
		glVertex3f (pMin->x, pMax->y, pMax->z);

	glEnd();
}

// ------------------------------------------------------------------------
// DrawOriginLines()
// draws coordinate system
// ------------------------------------------------------------------------
void DrawOriginLines()
{
	float size;

	size = 150.0f;

	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_LINES);

		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(-size, 0.0f, 0.0f);
		glVertex3f( size, 0.0f, 0.0f);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, -size, 0.0f);
		glVertex3f(0.0f,  size, 0.0f);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, -size);
		glVertex3f(0.0f, 0.0f, size);
	
	glEnd();

	glPointSize(8.0f);
	glBegin(GL_POINTS);
		glVertex3f(size, 0.0f, 0.0f);
		glVertex3f(0.0f, size, 0.0f);
		glVertex3f(0.0f, 0.0f, size);
	glEnd();
}


// ------------------------------------------------------------------------
// Draw an LOD piece 
// 
// ------------------------------------------------------------------------
static void DrawPiece(PieceLOD *pPiece, TVert *pVerts, DrawStruct* pStruct)
{
	DWORD count;
	ModelTri *pCurTri;
	DVector *v[3];
	DVector normal, dir1, dir2;
	int i;

	// Draw the tris.
	pCurTri = pPiece->m_Tris.GetArray();
	count = pPiece->m_Tris.GetSize();

	while(count)
	{
		count--;
        
		v[0] = &pVerts[pCurTri->m_Indices[GLM_TRI_INDEX0]].m_vPos;
		v[1] = &pVerts[pCurTri->m_Indices[GLM_TRI_INDEX1]].m_vPos;
		v[2] = &pVerts[pCurTri->m_Indices[GLM_TRI_INDEX2]].m_vPos;
		
		for(i=0; i < 3; i++){
			glNormal3f(
                EXPANDVEC(pVerts[pCurTri->m_Indices[2-i]].m_vNormal));

#if PRIMARY_TEXTURE                
			glTexCoord2f(
                pCurTri->m_UVs[2-i].tu + g_tuAdd,
                pCurTri->m_UVs[2-i].tv + g_tvAdd);
#elif SPECULAR_TEXTURE

/* This could be interesting - but only if we wanted to play with the
specular texture, which would allow for toon shading, for
instance. If all we're going to do is a linear shade, we might as
well use hardware specular highlights.

As of 10/18/00, this isn't completely working. I suspect another
left-handed coordinate system problem. -DWL
*/              

            // compute specular value
            for (int lightNum=1;
                 lightNum<2;
//                     lightNum<pStruct->m_nLights;
                 ++lightNum){
                
                DVector vertPos=*v[i];
                DVector lightDir=
                    pStruct->m_LightPositions[lightNum]-
                    vertPos;

				lightDir.Normalize();
                DVector normal=
                    pVerts[pCurTri->m_Indices[2-i]].m_vNormal;

                DVector reflect=(2.0f*normal.Dot(lightDir)*normal)-
                    lightDir;

                DVector toViewer=pStruct->m_ViewerPos-vertPos;

                toViewer.Normalize();
                
                float specularValue=
                    _cpp_max(0.005f,reflect.Dot(toViewer));

				specularValue=pow(specularValue,2.0f);
                
                glTexCoord2f(
                    specularValue,
                    0.5f);

				static debugOut=0;

				if (debugOut){
					char msg[80];
					sprintf(msg,"vPos: [%2.2f %2.2f %2.2f]\n",vertPos[0], vertPos[1], vertPos[2]);						
					OutputDebugString(msg);
					sprintf(msg,"lite: [%2.2f %2.2f %2.2f]\n",lightDir[0], lightDir[1], lightDir[2]);
					OutputDebugString(msg);
					sprintf(msg,"norm: [%2.2f %2.2f %2.2f]\n",normal[0], normal[1], normal[2]);
					OutputDebugString(msg);
					sprintf(msg,"refl: [%2.2f %2.2f %2.2f]\n",reflect[0], reflect[1], reflect[2]);
					OutputDebugString(msg);
					sprintf(msg,"view: [%2.2f %2.2f %2.2f]\n\n",toViewer[0], toViewer[1], toViewer[2]);					
					OutputDebugString(msg);
				}
            }

#elif ENVIRONMENTAL_TEXTURE
            
            // compute environmental value

            DVector normal=
                pVerts[pCurTri->m_Indices[2-i]].m_vNormal;

            DVector normalInViewspace;

            for (int j=0;j<4;++j){
                normalInViewspace[j]=
                    normal[0]*viewMatrix[j]+
                    normal[1]*viewMatrix[j+4]+
                    normal[2]*viewMatrix[j+8]+
                    normal[3]*viewMatrix[j+12];
            }
            
			glTexCoord2f(
                .5+normalInViewspace[0]*.5f,
                .5+normalInViewspace[1]*.5f);
#endif
			glVertex3f(EXPANDVEC(*v[i]));
		}
		
		pCurTri++;
	}
}


// ------------------------------------------------------------------------
// Draw an LOD piece 
// 
// ------------------------------------------------------------------------
static void DrawPieceVertexColors(PieceLOD *pPiece, TVert *pVerts, DrawStruct* pStruct)
{
	DWORD count;
	ModelTri *pCurTri;
	DVector *v[3], *c[3];
	DVector normal, dir1, dir2;
	int i;

	// Draw the tris.
	pCurTri = pPiece->m_Tris.GetArray();
	count = pPiece->m_Tris.GetSize();

	while(count)
	{
		count--;
        
		v[0] = &pVerts[pCurTri->m_Indices[GLM_TRI_INDEX0]].m_vPos;
		v[1] = &pVerts[pCurTri->m_Indices[GLM_TRI_INDEX1]].m_vPos;
		v[2] = &pVerts[pCurTri->m_Indices[GLM_TRI_INDEX2]].m_vPos;
		c[0] = &pVerts[pCurTri->m_Indices[GLM_TRI_INDEX0]].m_vColor;
		c[1] = &pVerts[pCurTri->m_Indices[GLM_TRI_INDEX1]].m_vColor;
		c[2] = &pVerts[pCurTri->m_Indices[GLM_TRI_INDEX2]].m_vColor;
		

		for(i=0; i < 3; i++){
			glColor3fv( & (c[i]->x) );
			glNormal3f(
                EXPANDVEC(pVerts[pCurTri->m_Indices[2-i]].m_vNormal));
			glVertex3f(EXPANDVEC(*v[i]));
		}
		
		pCurTri++;
	}
}							

/*
This uses an alternate (mathematically identical) method of setting up the 
transforms that allows it to transform the normals and allows it to only
use one base vertex position:

Current engine method:
NG = node global transform
VG = vertex in global space
PNG = precalculated node global transform
PVL = precalculated vertex in node local space
PVG = precalculated vertex in global space
PVL = ~PNG * PVG

To get a vertex's position.
VG = (NG0 * PVL0) + (NG1 * PVL1)...

// To get away from needing PVL0, PVL1... as input:
VG = NG * (~PNG * PVG) * w1 + NG2 * (~PNG2 * PVG) * w2

So in AlternateGetVertexPositions, we get rid of PVL's and use PVG.

If bNormals is TRUE, then it also transforms each vertex's normal and
places that right after each vertex's position.
*/

BOOL AlternateGetVertexPositions(	Model *pModel,
 									GVPStruct *pStruct, 
									BOOL bSetupTransforms,
									BOOL bNormals,
									BOOL bDrawOriginalModel,
									BOOL bMovementEncoding,
									void **pNextVert /*= NULL*/ )
{
	DWORD i, j, k;
	ModelPiece *pPiece;
	BYTE *pCurOut;
	DVector *pNormalOut;
	TransformMaker tMaker;
	float vTemp[4];
	ModelVert *pVert;
	const LTMatrix *pTransforms;
	NewVertexWeight *pWeight;
	PieceLOD *pLOD;
	float theVec[4];
	
	int bDoVertexAnim = FALSE ;

	pModel->ResetNodeEvalState();
	pModel->SetupNodeEvalStateFromPieces(pStruct->m_CurrentLODDist);

	// Validate..
	if(!pStruct->m_Vertices)
		return FALSE;

	if(pStruct->m_nAnims > MAX_GVP_ANIMS)
		return FALSE;

	// Copy the vertex positions over.
	pCurOut = (BYTE*)pStruct->m_Vertices;


	if(bSetupTransforms )
	{
		tMaker.SetupFromGVPStruct(pStruct);

		if(bMovementEncoding && !tMaker.SetupMovementEncoding())
			return FALSE;
		if(!tMaker.SetupTransforms())
			return FALSE;
	}

	// Apply the inverse global transform according to the equation above.
	pTransforms = pModel->m_Transforms.GetArray();


	for(i=0; i < pModel->NumPieces(); i++)
	{
		pPiece = pModel->GetPiece(i);
		pLOD = pPiece->GetLOD(pStruct->m_CurrentLODDist );

        // if this piece is vertex animated update the output vertex buffer
		// with new data.
        if( pPiece->m_isVA  )
        {
			// if we're drawing the original model stuff the vertex into the 
			// vertex buffer
			if( bDrawOriginalModel )
			{
				for( j=0 ; j < pLOD->m_Verts ; j++ )
				{
					pVert = &pLOD->m_Verts[j];
					((DVector*)pCurOut)->Init();
					((DVector*)pCurOut)->x = pVert->m_Vec.x ;
					((DVector*)pCurOut)->y = pVert->m_Vec.y ;
					((DVector*)pCurOut)->z = pVert->m_Vec.z ;

					pNormalOut = (LTVector*)(pCurOut + sizeof(LTVector));
					pNormalOut->Init();
					pNormalOut->x=pVert->m_Normal.x;
					pNormalOut->y=pVert->m_Normal.y;
					pNormalOut->z=pVert->m_Normal.z;
					pCurOut += pStruct->m_VertexStride;
				}
			}
			else
			{

                // update the vertex in the model according to animation
                pCurOut = (BYTE*)UpdateVAModelPiece( pModel, pPiece, 
                                                     &pStruct->m_Anims[0], 
                                                     (TVert*)pCurOut, 
                                                     &pTransforms[0]);
                
                
            }
        }
        
        // update draw vertex buffer using skeletal animation
        if(!pPiece->m_isVA)
		for(j=0; j < pLOD->m_Verts; j++)
		{
			pVert = &pLOD->m_Verts[j];

			// Apply all the weights. ( do skeletal deformation )            
			if(pVert->m_nWeights > 0 && (bDrawOriginalModel == FALSE) && !pPiece->m_isVA)
			{
				vTemp[0] = vTemp[1] = vTemp[2] = vTemp[3] = 0.0f;
				for(k=0; k < pVert->m_nWeights; k++)
				{
					pWeight = &pVert->m_Weights[k];
					ASSERT(pWeight->m_iNode < pModel->NumNodes());

					// Regular method..
					MatVMul_Add(vTemp, &pTransforms[pWeight->m_iNode], pWeight->m_Vec);					
					//memcpy(vTemp,pWeight->m_Vec,sizeof(float)*4);
				}

				// Divide by w.
				vTemp[3] = 1.0f / vTemp[3];
				((DVector*)pCurOut)->x = vTemp[0] * vTemp[3];
				((DVector*)pCurOut)->y = vTemp[1] * vTemp[3];
				((DVector*)pCurOut)->z = vTemp[2] * vTemp[3];

				// Calculate normals
				if(bNormals)
				{
					vTemp[0] = vTemp[1] = vTemp[2] = vTemp[3] = 0.0f;

					for(k=0; k < pVert->m_nWeights; k++)
					{
						pWeight = &pVert->m_Weights[k];
						ASSERT(pWeight->m_iNode < pModel->NumNodes());

						theVec[0] = pVert->m_Normal.x * pWeight->m_Vec[3];
						theVec[1] = pVert->m_Normal.y * pWeight->m_Vec[3];
						theVec[2] = pVert->m_Normal.z * pWeight->m_Vec[3];

						theVec[3] = 0 ; //pWeight->m_Vec[3];

						// pTransforms[i] is the appropriate way to transform this from 
						// node-local coords, so we need to transform from world to local
						// which is done by premultiplying by the inverse global transform.

						ModelNode *node=pModel->GetNode(pWeight->m_iNode);
						LTMatrix matrix=node->GetInvGlobalTransform();
						LTMatrix worldToPosed = pTransforms[pWeight->m_iNode] * matrix;

						MatVMul_Add_3x3(vTemp, &worldToPosed, theVec);
					}

					pNormalOut = (DVector*)(pCurOut + sizeof(DVector));

					float oneOverLen = 1.0f/sqrtf( vTemp[0] * vTemp[0] + vTemp[1] * vTemp[1] + vTemp[2] * vTemp[2] );

					vTemp[0] *= oneOverLen ;
					vTemp[1] *= oneOverLen ;
					vTemp[2] *= oneOverLen ;
					pNormalOut->x = vTemp[0] ;
					pNormalOut->y = vTemp[1] ;
					pNormalOut->z = vTemp[2] ;
				}

			}
			else if(bDrawOriginalModel)// draworiginal model
			{
				((DVector*)pCurOut)->Init();
				((DVector*)pCurOut)->x = pVert->m_Vec.x ;
				((DVector*)pCurOut)->y = pVert->m_Vec.y ;
				((DVector*)pCurOut)->z = pVert->m_Vec.z ;

				pNormalOut = (DVector*)(pCurOut + sizeof(DVector));
				pNormalOut->Init();
				pNormalOut->x=pVert->m_Normal.x;
				pNormalOut->y=pVert->m_Normal.y;
				pNormalOut->z=pVert->m_Normal.z;
			}
		

			pCurOut += pStruct->m_VertexStride;
		}// for every vertex
	}

	if( pNextVert )
		*pNextVert = pCurOut;

	return TRUE;
}

// ------------------------------------------------------------------------
// set-vertex-color-based-on-bone-weight( model, render-data, bone-index)
// sets a grey intensity based on the vertex weight for bone index. 
// ------------------------------------------------------------------------
BOOL SetVertexColorBasedOnBoneWeight(Model     *pModel,
 									 GVPStruct *pStruct, 
									 set<int> &requested_bones )
{
	DWORD       i, j, k;
	ModelPiece *pPiece;
	TVert      *pCurOut;
	ModelVert  *pVert;
	PieceLOD   *pLOD;
	NewVertexWeight *pWeight;
	
	// was pstruct set up right?
	if(!pStruct->m_Vertices)
		return FALSE;

	// Copy the vertex positions over.
	pCurOut = (TVert*)pStruct->m_Vertices;


	for(i=0; i < pModel->NumPieces(); i++)
	{
		pPiece = pModel->GetPiece(i);
		pLOD = pPiece->GetLOD(pStruct->m_CurrentLODDist);

		for(j=0; j < pLOD->m_Verts; j++)
		{
			pVert = &pLOD->m_Verts[j];
			// Create Color based on vertex weight and selected bone        
			if(pVert->m_nWeights > 0)
			{
				pCurOut->m_vColor.Init(0.0f,0.0f,0.0f);

				for(k=0; k < pVert->m_nWeights; k++)
				{
					pWeight = &pVert->m_Weights[k];
					ASSERT(pWeight->m_iNode < pModel->NumNodes());

					// if the weight influence is in the requested bone set, create a color 
					// for it on the vertex. 
					if( requested_bones.find( pWeight->m_iNode ) != requested_bones.end())
					{	
						LTVector intensity(pWeight->m_Vec[3],pWeight->m_Vec[3],pWeight->m_Vec[3]);

						// turn the vector red if weight is over one.
						if( pWeight->m_Vec[3] > 1.0 )
							intensity.y = intensity.z = 0.0f ;
						
						pCurOut->m_vColor += intensity;
					}
				}	
			}
			pCurOut ++ ;
		}// for every vertex
	}// for every piece

	return TRUE;
}



// ------------------------------------------------------------------------
// DrawModelPolies( )
// ------------------------------------------------------------------------
static void DrawModelPolies(GLM_CONTEXT hContext,
							AnimTracker **pTrackers,
                            DWORD nTrackers, 
							BOOL bDrawBright,
                            DMatrix *pRootTransform,
                            BYTE *pSelectedPieces,
							BOOL bNormals,
                            DWORD iLOD,
                            BOOL bDrawOriginalModel,
                            BOOL bTexture,
                            DrawStruct* pStruct,
							bool bCalcRadius = false,
							float* pRadius = NULL,
							bool bCalcAndDraw = false )
{
	int node_cnt ;
	DWORD i, j, nWantedVerts;
	ModelPiece *pPiece;
	GVPStruct gvp;
	DVector v1, v2;
	TVert *pCurVert;
	PieceLOD *pLOD;
	static CMoArray<TVert> tVerts;
	Model *pModel;
	GLMContext *pContext = (GLMContext *)hContext;

	float current_lod_dist = pStruct->m_CurrentLODDist ;

	if(nTrackers == 0 || !pTrackers[0]->m_TimeRef.m_pModel)
		return;

	pModel = pTrackers[0]->m_TimeRef.m_pModel;
	nTrackers = DMIN(nTrackers, MAX_GVP_ANIMS);

	// Size our list.
	nWantedVerts = pModel->GetTotalNumVerts() * 2;
	if(tVerts.GetSize() < nWantedVerts)
	{
		if(!tVerts.SetSize(nWantedVerts))
			return;
	}

	// Use the model code to setup the vertices.
	gvp.m_nAnims = 0;
	for(i=0; i < nTrackers; i++)
	{
		gvp.m_Anims[i] = pTrackers[i]->m_TimeRef;
		gvp.m_nAnims++;
	}

	gvp.m_VertexStride = sizeof(TVert);
	gvp.m_Vertices = tVerts.GetArray();
	gvp.m_BaseTransform = *pRootTransform;
	gvp.m_iLOD = iLOD;
	gvp.m_CurrentLODDist = pStruct->m_CurrentLODDist ;

	TVert* pEndVert;

	if(!AlternateGetVertexPositions(pModel, &gvp, TRUE, TRUE, bDrawOriginalModel, pStruct->m_bMovementEncoding, (void**)&pEndVert))
		return;

	// calculate the radius of this rendered frame of the model
	if( bCalcRadius && pRadius )
	{
		// get the inverse of the base transform
		LTMatrix mInvBase = pRootTransform->MakeInverse();
		// find the maximum magsqr of the vertex locations
		*pRadius = 0.0f;
		for( TVert* pCurVert = tVerts.GetArray(); pCurVert < pEndVert; ++pCurVert )
		{
			// put the vertex in world coords
			LTVector vWorldPos;
			mInvBase.Apply( pCurVert->m_vPos, vWorldPos );
			float fCurMag = vWorldPos.MagSqr();
			if( fCurMag > *pRadius )
				*pRadius = fCurMag;
		}
		// the sqrt of that is what we want
		*pRadius = (float)sqrt( *pRadius );
		if( !bCalcAndDraw )
			return;
	}

	if( pStruct->m_bDrawVertexWeights )
	{
		set<int> selected_nodes ;
		// find first selected node 
		for( node_cnt  = 0 ; node_cnt < (int)pModel->NumNodes() ; node_cnt ++ )
		{
			if( pStruct->m_SelectedNodes[ node_cnt ] )
			{
				selected_nodes.insert( node_cnt );
			}
		}	

		SetVertexColorBasedOnBoneWeight(pModel, &gvp, selected_nodes  );
	}


	if(bDrawBright)
	{
		glDisable(GL_LIGHTING);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
	else
	{
		glColor3f(0.0f, 1.0f, 0.0f);
	}	


// defaults 
	static GLfloat ambient[4] = {0.2f, 0.2f, 0.2f,1.0f} ;
	static GLfloat diffuse[4] = {0.8f, 0.8f, 0.8f,1.0f} ;
	static GLfloat spec[4]    = {0.0f, 0.0f, 0.0f,1.0f};
	static GLfloat emiss[4]   = {0.0f, 0.0f, 0.0f,1.0f}; 
	static GLfloat shine      = 0.0f;

//	glEnable( GL_COLOR_MATERIAL );

	pCurVert  = tVerts.GetArray() ;
	int NumPieces = pModel->NumPieces() ;
	
	// for every piece 
	for( int iPieceCnt = 0 ; iPieceCnt < NumPieces ; iPieceCnt++ )
	{
		pPiece = pModel->GetPiece( iPieceCnt );
 
		// get the current draw lod
		pLOD = pPiece->GetLOD( current_lod_dist );

		// swap in the texture if required
		if( pContext && bTexture && ((uint32)pLOD->m_iTextures[0] < pContext->m_Textures.GetSize()) )
		{
			GLuint pieceLODTexture = pContext->m_Textures[pLOD->m_iTextures[0]];
			if( pieceLODTexture && (pieceLODTexture != pContext->m_nCurTexture) )
			{
				pContext->m_nCurTexture = pieceLODTexture;
				glBindTexture( GL_TEXTURE_2D, pContext->m_nCurTexture );
			}

			// only turn on texturing if there is a valid texture
			if( pieceLODTexture )
				glEnable( GL_TEXTURE_2D );
			else
				glDisable( GL_TEXTURE_2D );
		}
		else
		{
			// no texture, turn off texturing
			glDisable( GL_TEXTURE_2D );
		}

		

		// ------------	------------------------------------------------------------
		// If we want to draw the vertex weights instead of the effect of light on 
		// the model we turn off lights, and material.
		// ------------	------------------------------------------------------------
		if( pStruct->m_bDrawVertexWeights )
		{
			glDisable(GL_LIGHTING);
			glDisable(GL_CULL_FACE);	

			glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
			glBegin( GL_TRIANGLES );
			DrawPieceVertexColors( pLOD, pCurVert, pStruct );
			glEnd();
		}else
		{
			// 
			glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse );
			glMaterialfv( GL_FRONT, GL_AMBIENT, ambient );
			glMaterialfv( GL_FRONT, GL_SPECULAR, spec );
			glMaterialfv( GL_FRONT, GL_SHININESS, &shine );
			glMaterialfv( GL_FRONT, GL_EMISSION, emiss );
			// draw the geometry normally
			glBegin( GL_TRIANGLES );
				DrawPiece( pLOD, pCurVert, pStruct );
			glEnd();
		
		}

		// step on to the next lump
		pCurVert += pLOD->m_Verts.GetSize() ;

	}

	// Draw vertex normals.
	if(bNormals)
	{
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glColor3f(0.0f, 1.0f, 0.0f);

		pCurVert = tVerts.GetArray();
		for(i=0; i < pModel->NumPieces(); i++)
		{
			pPiece = pModel->GetPiece(i);
			pLOD = pPiece->GetLOD( current_lod_dist );
			if( pLOD == NULL ) break ;

			for(j=0; j < pLOD->NumVerts(); j++)
			{
				v1 = pCurVert->m_vPos;
				v2 = v1 + pCurVert->m_vNormal * 5.0f;

				glBegin(GL_LINES);
					glVertex3f(EXPANDVEC(v1));
					glVertex3f(EXPANDVEC(v2));
				glEnd();

				pCurVert++;
			}
		}
	}


	glDisable(GL_TEXTURE_2D);
	if(bDrawBright)
	{
		glColor3f(0.0f, 1.0f, 0.0f);
	}


	DrawOriginLines();


	// If there are any selections, draw those too.
	glDisable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if(pSelectedPieces)
	{
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_TRIANGLES);

			pCurVert = tVerts.GetArray();
			for(i=0; i < pModel->NumPieces(); i++)
			{
				pPiece = pModel->GetPiece(i);
				pLOD = pPiece->GetLOD( current_lod_dist);
				
				if(pSelectedPieces[i])
				{
					DrawPiece(pLOD, pCurVert, pStruct );
				}

				pCurVert += pLOD->m_Verts.GetSize();
			}

		glEnd();
	}
}

// ------------------------------------------------------------------------
// Draw a line 
// ------------------------------------------------------------------------
void DrawArrow(float r, float g, float b, DVector pos, DVector vec, float length)
{
	DVector pos2;

	glColor3f(r, g, b);

	pos2 = pos + vec*length;
	glBegin(GL_LINES);
		glVertex3f(VEC_EXPAND(pos));
		glVertex3f(VEC_EXPAND(pos2));
	glEnd();
}

// ------------------------------------------------------------------------
// DrawCoordSys
// draw the rotation basis at pos, r/g/b = X/Y/Z
// ------------------------------------------------------------------------
void DrawCoordSys(DMatrix *pMat, float arrowLength)
{
	DVector up, right, forward;
	DVector pos;
	StateSet stateTexture(GL_TEXTURE_2D, FALSE);
	StateSet stateDepthTest(GL_DEPTH_TEST, FALSE);
	StateSet Lighting( GL_LIGHTING, FALSE );

	Mat_GetBasisVectors(pMat, &right, &up, &forward);
	pos.x = pMat->m[0][3];
	pos.y = pMat->m[0][7];
	pos.z = pMat->m[0][11];

	DrawArrow(1.0f, 0.0f, 0.0f, pos, right, arrowLength);
	DrawArrow(0.0f, 1.0f, 0.0f, pos, up, arrowLength);
	DrawArrow(0.0f, 0.0f, 1.0f, pos, forward, arrowLength);
}

// ------------------------------------------------------------------------
// RecurseAndDrawSkeleton()
// ------------------------------------------------------------------------
void RecurseAndDrawSkeleton(DrawStruct *pStruct, 
							ModelNode  *pNode, 
							LTVector    curNodePos, 
							BOOL       bChainStarted)
{
	DVector myPos;
	DWORD i;
	Model *pModel;
	BOOL bSelected;
	LTMatrix mat ;
	float fEvaledVal;

	pModel = pStruct->GetModel();

	// don't draw the movement node if we are rendering using movement encoding
	if( pStruct->m_bMovementEncoding && (pModel->GetMovementNode() == pNode) )
		return;

	// Transform the position.
	if( !pStruct->m_bDrawOriginalModel )
		mat = pModel->m_Transforms[pNode->GetNodeIndex()];
	else
		mat = pNode->GetGlobalTransform();

	mat.GetTranslation( myPos );

	if(bChainStarted)
	{
		// The chain has already started so draw a line.
		glBegin(GL_LINES);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(VEC_EXPAND(curNodePos));
			glColor3f(0.2f, 0.2f, 0.2f);
			glVertex3f(VEC_EXPAND(myPos));
		glEnd();
	}

	// Draw a point if node is selected
	bSelected = pStruct->m_SelectedNodes[pNode->GetNodeIndex()];
	fEvaledVal= pNode->m_EvalState == ModelNode::kIgnore ? 0.5f : 1.0f ;

	glPointSize(bSelected ? 8.0f : 4.0f);
	glBegin(GL_POINTS);

	if(bSelected)
	{
		// yellow
		glColor3f(fEvaledVal, 1.0f, 0.0f);
	}
	else
	{
		// not yellow
		glColor3f(0.0f, 1.0f, fEvaledVal);
	}

	glVertex3f(VEC_EXPAND(myPos));

	glEnd();

	if(bSelected)
		DrawCoordSys( &mat, 6.0f );

	bChainStarted = TRUE;

	// Go into the children.
	for(i=0; i < pNode->m_Children; i++)
	{
		RecurseAndDrawSkeleton(pStruct, pNode->m_Children[i], myPos, bChainStarted);
	}
}

void RecurseAndDrawOBB(DrawStruct *pStruct, ModelNode  *pNode 	);
	
// ------------------------------------------------------------------------
// DrawSkeleton( draw-struct );
// recursively draws the skeleton, showing selected nodes with a large point and
// a coord-sys representation.
// ------------------------------------------------------------------------
void DrawSkeleton(DrawStruct *pStruct)
{
	LTVector curNodePos;
	
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	curNodePos.Init();
	RecurseAndDrawSkeleton(pStruct, pStruct->GetModel()->GetRootNode(), curNodePos, FALSE);

	if(pStruct->m_bDrawOBB)
		RecurseAndDrawOBB(pStruct, pStruct->GetModel()->GetRootNode());
}

// ------------------------------------------------------------------------
// ltmat_to_glmat( lt-mat in, gl-mat out)
//
// convert a lithtech mat to a gl mat. basically transposes the matrix.
// ------------------------------------------------------------------------
static void ltmat_to_glmat( LTMatrix &mat, float *gl_mat )
{
	int cnt = 0 ;

	for( int row = 0 ; row < 4 ; row ++ )
	for( int col = 0 ; col < 4 ; col++ )
	{
		gl_mat[cnt] = mat.El( col, row );
		cnt ++ ;
	}
}

// -----------------------------------------------------------------------
// draw_box( position, orientation, scale )
//
// draw the bounding box, position, orient and scale it.
// ------------------------------------------------------------------------
static void draw_box( const LTVector &offset, 
					 const LTRotation &orient, 
					 const LTVector & size				 )
{


	LTMatrix mat ;
	float gl_mat[16];
	LTVector vmin(-.5f,-.5f,-.5f), vmax(.5f,.5f,.5f);
	

	mat.Identity();
	orient.ConvertToMatrix(mat);
	
	ltmat_to_glmat(mat,gl_mat);

	// set up the box's local transform
	glPushMatrix();
	// do rotation

	glMultMatrixf( gl_mat );
	glTranslatef( offset.x, offset.y,offset.z );
	//glScalef( size.x, size.y, size.z );
	vmin = vmin * size ; vmax = vmax *size ;
	
	// line drawing of box
	glBegin(GL_LINES);
		// top
		glVertex3f (vmin.x, vmin.y, vmin.z);
		glVertex3f (vmax.x, vmin.y, vmin.z);

		glVertex3f (vmax.x, vmin.y, vmin.z);
		glVertex3f (vmax.x, vmin.y, vmax.z);

		glVertex3f (vmax.x, vmin.y, vmax.z);
		glVertex3f (vmin.x, vmin.y, vmax.z);

		glVertex3f (vmin.x, vmin.y, vmax.z);
		glVertex3f (vmin.x, vmin.y, vmin.z);

		// bottom

		glVertex3f (vmax.x, vmax.y, vmax.z);
		glVertex3f (vmin.x, vmax.y, vmax.z);
		
		glVertex3f (vmin.x, vmax.y, vmax.z);
		glVertex3f (vmin.x, vmax.y, vmin.z);

		glVertex3f (vmin.x, vmax.y, vmin.z);
		glVertex3f (vmax.x, vmax.y, vmin.z);

		glVertex3f (vmax.x, vmax.y, vmin.z);
		glVertex3f (vmax.x, vmax.y, vmax.z);

		// sides

		glVertex3f (vmin.x, vmin.y, vmin.z);
		glVertex3f (vmin.x, vmax.y, vmin.z);

		glVertex3f (vmax.x, vmin.y, vmin.z);
		glVertex3f (vmax.x, vmax.y, vmin.z);

		glVertex3f (vmax.x, vmin.y, vmax.z);
		glVertex3f (vmax.x, vmax.y, vmax.z);

		glVertex3f (vmin.x, vmin.y, vmax.z);
		glVertex3f (vmin.x, vmax.y, vmax.z);

	
	glEnd();

	glPopMatrix();

}

// ------------------------------------------------------------------------
// RecurseAndDrawOBB()
// ------------------------------------------------------------------------
void RecurseAndDrawOBB(DrawStruct *pStruct, 
							ModelNode  *pNode 							)

{
	DVector myPos;
	DWORD i;
	Model *pModel;
	BOOL bSelected;
	LTMatrix mat ;

	pModel = pStruct->GetModel();

	// don't draw the movement node if we are rendering using movement encoding
	if( pStruct->m_bMovementEncoding && (pModel->GetMovementNode() == pNode) )
		return;

	// Transform the position.
	if( pStruct->m_bDrawOriginalModel )
		mat = pNode->GetGlobalTransform();	
	else
		mat = pModel->m_Transforms[pNode->GetNodeIndex()];

	mat.GetTranslation( myPos );

	// Draw a point if node is selected
	bSelected = pStruct->m_SelectedNodes[pNode->GetNodeIndex()];

	glPointSize(bSelected ? 8.0f : 4.0f);
	glBegin(GL_POINTS);

		if(bSelected)
		{
			glColor3f(1.0f, 1.0f, 0.0f);
		}
		else
		{
			glColor3f(0.0f, 1.0f, 1.0f);
		}

		glVertex3f(VEC_EXPAND(myPos));

	glEnd();

	if(bSelected)
	{
		DrawCoordSys( &mat, 6.0f );
	}

	LTRotation default_orient;
	default_orient.Init();
	const SOBB obb = pNode->GetOBB();

	// if node is selected draw one color else draw common color
	if(bSelected) glColor3f(1.0f, 1.0f, 0.0f); else glColor3f(0.0f, 1.0f, 1.0f);

	// apply the current transform into the current matrix stack.
	if( pNode->IsOBBEnabled() )
	{
		float gl_mat[16];
		ltmat_to_glmat( mat, gl_mat );
		// move the box into nodes' coordinate frame

		glPushMatrix();
		glMultMatrixf( gl_mat );
		draw_box( obb.m_Pos , obb.m_Orientation, obb.m_Size);
		glPopMatrix();
	}

	// Recure down tree.
	for(i=0; i < pNode->m_Children; i++)
	{
		RecurseAndDrawOBB(pStruct, pNode->m_Children[i]);
	}
}


void DrawSocket(DrawStruct *pStruct, ModelSocket *pSocket)
{
	LTMatrix socketTransform;
	Model *pModel;

	LTBOOL bSelected = pStruct->m_SelectedNodes[pSocket->m_iNode];

	if(!bSelected)
	{
		glLineStipple(2, 0x1111); // Dotted lines.
		glEnable(GL_LINE_STIPPLE);
	}
	else 
		glLineWidth(2.0f);


	pModel = pStruct->GetModel();

	if(pModel->GetSocketTransform(pSocket, &socketTransform))
	{
		DrawCoordSys(&socketTransform, 10.0f);
	}

	if(!bSelected) {
		glDisable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xFFFF);
	}
	glLineWidth(1.0f);
}

// ------------------------------------------------------------------------
// DrawAttachement( model, socket-transform, lod-level, solid, draw-context )
// Draw the model attached to a socket.
// ------------------------------------------------------------------------
static void DrawAttachment(Model *pAttachment,
                           DMatrix *pTransform,
                           DWORD iLOD,
                           BOOL bSolid,
                           DrawStruct* pStruct)
{
	AnimTracker tracker, *pTracker;
	StateSet stateLighting(GL_LIGHTING, bSolid);
	StateSet stateCull(GL_CULL_FACE, bSolid);
	StateSet stateDepth(GL_DEPTH_TEST, bSolid);
	GLenum eOldPolyMode;
	glGetIntegerv(GL_POLYGON_MODE, (int *)&eOldPolyMode);
	glPolygonMode(GL_FRONT_AND_BACK, (bSolid) ? GL_FILL : GL_LINE);

	tracker.m_TimeRef.Init(pAttachment, 0, 0, 0, 0, 0.0f);
	pTracker = &tracker;

	DrawModelPolies(NULL,
                    &pTracker,
                    1,
                    FALSE,
                    pTransform,
                    NULL,
                    FALSE,
                    iLOD,
                    FALSE,
                    pStruct->m_bDrawTextures,
                    pStruct);

	glPolygonMode(GL_FRONT_AND_BACK, eOldPolyMode);
}


static void DrawSocketsAndAttachments(DrawStruct *pStruct)
{
	uint32 i;
	ModelSocket *pSocket;
	Model *pModel;
	TransformMaker tMaker;
	LTMatrix mSocket;

	//if( !pStruct->m_bDrawAttachments || !pStruct->m_bDrawSockets )
		//return ;

	pModel = pStruct->GetModel();
	pModel->ResetNodeEvalState();

	// tag the nodes to eval.
	for(i=0; i < pModel->NumSockets(); i++)
	{
		pSocket = pModel->GetSocket(i);

		pModel->SetupNodeEvalStateFromNode( pSocket->m_iNode );
	}

	// Setup transforms.  
	pStruct->SetupTransformMaker(&tMaker);
	
	if(pStruct->m_bMovementEncoding && !tMaker.SetupMovementEncoding())
		return;
	if(!tMaker.SetupTransforms())
		return;

	
	for(i=0; i < pModel->NumSockets(); i++)
	{
		pSocket = pModel->GetSocket(i);
		
		if(pStruct->m_bDrawAttachments && pSocket->m_pAttachment)
		{
			if(pModel->GetSocketTransform(pSocket, &mSocket))
			{
				DrawAttachment(
					pSocket->m_pAttachment, 
					&mSocket, 
					pStruct->m_iLOD, 
					pStruct->m_bSolidAttachments,
                    pStruct);
			}
		}

		if(pStruct->m_bDrawSockets)
		{
			DrawSocket(
				pStruct, 
				pSocket);
		}
	}
}



// ------------------------------------------------------------------------
// DrawModel
// Entry point for rendering the model.
// ------------------------------------------------------------------------
void DrawModel(GLM_CONTEXT hContext, DrawStruct *pStruct)
{
	GLMContext *pContext;
	DMatrix baseTransform;
	DVector vMin, vMax;
	Model *pModel;
	uint32 renderTicksTGT = 0;
	LARGE_INTEGER Ticks;
	RECT rect;
	char str[256];
	uint64 startTime;
	uint64 stopTime;
	double renderTicks;

	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	double m_fPrecisionScaleMS = (Frequency.QuadPart == 0) ? 1.0 : 1000.0 / (double)(Frequency.QuadPart);

	QueryPerformanceCounter(&Ticks);

	startTime = Ticks.QuadPart;

	
	// Must have at least one valid AnimTracker (the first one is used for things
	// like dims).
	if(!pStruct->m_Times[0]->m_TimeRef.m_pModel)
		return;

	pModel = pStruct->GetModel();
	
	pContext = (GLMContext*)hContext;


	// Draw the model.
	renderTicks = 0;

	Mat_Identity(&baseTransform);
    
    DrawModelPolies(hContext,
                    pStruct->m_Times,
                    pStruct->NumValidTimes(),
                    pStruct->m_bDrawBright,
                    &baseTransform,
                    pStruct->m_SelectedPieces,
                    pStruct->m_bNormals,
                    pStruct->m_iLOD,
                    pStruct->m_bDrawOriginalModel,
                    pStruct->m_bDrawTextures,
                    pStruct,
					pStruct->m_bCalcRadius,
					&(pStruct->m_fModelRadius),
					pStruct->m_bCalcAndDraw);

	// jump out if we're not actually drawing anything
	if( pStruct->m_bCalcRadius && !pStruct->m_bCalcAndDraw )
		return;
	

	if(pStruct->m_bDrawSkeleton)
	{
		DrawSkeleton(pStruct);
	}

	if (pStruct->m_bDims)
	{
		vMax = pModel->GetAnimInfo(pStruct->m_Times[0]->m_TimeRef.m_Prev.m_iAnim)->m_vDims;
		vMin = -vMax;
		DrawBox ((DVector*)&pStruct->m_DimsColor, &vMin, &vMax);
	}


	// Draw sockets.
	if(pStruct->m_bDrawAttachments || pStruct->m_bDrawSockets)
	{
		DrawSocketsAndAttachments(pStruct);
	}


	if (pStruct->m_bAnimBox)
	{
		vMax.x = vMax.y = vMax.z = pModel->m_GlobalRadius;
		vMin = -vMax;
		DrawBox ((DVector*)&pStruct->m_AnimBoxColor, &vMin, &vMax);
	}

	// Draw some status stuff..
	if(pStruct->m_bProfile)
	{
		QueryPerformanceCounter(&Ticks);

		stopTime = Ticks.QuadPart;
	
		renderTicks = ( ( stopTime-startTime) * m_fPrecisionScaleMS);

		rect.left = rect.top = 0;
		rect.right = 1000;
		rect.bottom = 100;
		sprintf(str, "Tris: %d, Ticks: %f", pModel->CalcNumTris(pStruct->m_CurrentLODDist), renderTicks);
		DrawText(pContext->m_hDC, str, strlen(str), &rect, DT_LEFT);
	}
}


BOOL SetGLMTexture(GLM_CONTEXT hContext, TextureData *pTexture, DWORD nTextureNum)
{
	BYTE *p32BitTexture;
	FormatMgr formatMgr;
	PFormat srcFormat, destFormat;
	ConvertRequest request;
	DRESULT dResult;
	TextureMipData *pMip;
	DWORD err;
	GLMContext *pContext;


	if(!(pContext = (GLMContext*)hContext))
		return FALSE;

	wglMakeCurrent(pContext->m_hDC, pContext->m_hglrc);
	pMip = &pTexture->m_Mips[0];

	// Convert to 32-bit.
	p32BitTexture = new BYTE[pMip->m_Width * pMip->m_Height * 4];
	if(!p32BitTexture)
		return FALSE;

	dtx_SetupDTXFormat(pTexture, &srcFormat);
	destFormat.Init(BPP_32, 0xFF000000, 0x000000FF, 0x0000FF00, 0x00FF0000);

	request.m_pSrcFormat = &srcFormat;
	request.m_pSrc = pTexture->m_Mips[0].m_Data;
	request.m_SrcPitch = pMip->m_Pitch;
	request.m_pDestFormat = &destFormat;
	request.m_pDest = p32BitTexture;
	request.m_DestPitch = pMip->m_Width * 4;
	request.m_Width = pMip->m_Width;
	request.m_Height = pMip->m_Height;
	request.m_Flags = 0;

	//! we need to set the pallete for BPP_32P
	if (pTexture->m_Header.GetBPPIdent() == BPP_32P)
	{
		// find the pallete section
		DtxSection* pSection = dtx_FindSection(pTexture, "PALLETE32");

		if (pSection)
		{
			request.m_pSrcPalette = (RPaletteColor*)pSection->m_Data;  
		}
	}

	dResult = formatMgr.ConvertPixels(&request);
	if(dResult != LT_OK)
		return FALSE;

	// Clear the error state
	err = glGetError();

	while (pContext->m_Textures.GetSize() <= nTextureNum)
		pContext->m_Textures.Append(0);

	// Free the texture channel if it's already allocated
	if (pContext->m_Textures[nTextureNum])
	{
		glDeleteLists(pContext->m_Textures[nTextureNum], 1);
		err = glGetError();
	}

	// Create a texture binding for the texture
	GLuint nTextureID;
	glGenTextures(1, &nTextureID);
	pContext->m_Textures[nTextureNum] = nTextureID;
	glBindTexture(GL_TEXTURE_2D, nTextureID);
	err = glGetError();

	glDisable(GL_TEXTURE_1D);
	err = glGetError();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, pMip->m_Width, pMip->m_Height, 0, 
		GL_RGBA, GL_UNSIGNED_BYTE, p32BitTexture);
	err = glGetError();

	delete p32BitTexture;

	// Adjust for bilinear filtering.
	g_tuAdd = 0.5f / (float)pMip->m_Width;
	g_tvAdd = 0.5f / (float)pMip->m_Height;
	return TRUE;
}

// ----------------------------------------------------------
// SetGLMBackgroundColor( gl-context, COLORREF )
// ----------------------------------------------------------
void SetGLMBackgroundColor( GLM_CONTEXT hContext, COLORREF win_color )
{
	GLMContext *pContext ;
	unsigned char *pColDecomp = (unsigned char*)&win_color;

	if(!(pContext = (GLMContext*)hContext))
	{
		ASSERT( pContext != NULL ); // let windows deal with this error.
		return ;
	}

	// unitize
	pContext->m_bgColor[0] = pColDecomp[0]/255.0f;
	pContext->m_bgColor[1] = pColDecomp[1]/255.0f;
	pContext->m_bgColor[2] = pColDecomp[2]/255.0f;

}


// ----------------------------------------------------------
// SetGLMBackgroundColor( gl-context, COLORREF )
// ----------------------------------------------------------
void SetGLMBackgroundColor( GLM_CONTEXT hContext, float *color )
{
	GLMContext *pContext ;
	if(!(pContext = (GLMContext*)hContext))
	{
		ASSERT( pContext != NULL ); // let windows deal with this error.
		return ;
	}

	pContext->m_bgColor[0] = color[0];
	pContext->m_bgColor[1] = color[1];
	pContext->m_bgColor[2] = color[2];

}




BOOL DrawStruct::SetupTransformMaker(TransformMaker *pMaker)
{
	DWORD i;

	pMaker->m_nAnims = 0;
	for(i=0; i < NUM_ANIM_INFOS && pMaker->m_nAnims < MAX_GVP_ANIMS; i++)
	{
		if(m_Times[i]->m_TimeRef.m_pModel)
		{
			pMaker->m_Anims[pMaker->m_nAnims] = m_Times[i]->m_TimeRef;
			pMaker->m_nAnims++;
		}
	}

	return pMaker->m_nAnims > 0;
}


DWORD DrawStruct::NumValidTimes()
{
	DWORD i, nTimes;

	nTimes = 0;
	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		if(!m_Times[i]->m_TimeRef.m_pModel)
			break;

		++nTimes;
	}

	return nTimes;
}

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
void DrawWorldCoordSys() 
{
	static GLuint mGrid = 0;
	static float mGridSize = 120;
	static float d;

    if(mGrid)
      glCallList( mGrid );
    else {
     mGrid = glGenLists( 1 );
      if( mGrid == 0 )
      {
			return ;    
      }
	  
      glNewList(mGrid, GL_COMPILE);
      glPushAttrib(GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT );
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);
      
      glColor3f(.3f,.3f,.3f);
	  glLineWidth( 5.0f );

      glBegin(GL_LINES);
	  
	  // draw axis 
      glColor3f( .3f, 0.0f,0.0f);
      glVertex3f(-mGridSize, 0.0f, 0.0f);
      glVertex3f(      0.0f, 0.0f, 0.0f);
      glColor3f( 1.0f, 0.0f,0.0f);
      glVertex3f( mGridSize, 0.0f, 0.0f);
      glVertex3f(      0.0f, 0.0f, 0.0f);
      
      d = 0.0f ;
      glColor3f( 0.0f, 0.0f,.3f);
      glVertex3f(d, 0.0f, -mGridSize);
      glVertex3f(d, 0.0f, -0.0f);
      glColor3f( 0.0f, 0.0f,1.0f);
      glVertex3f(d, 0.0f,  mGridSize);
      glVertex3f(d, 0.0f, -0.0f);
     // glEnd();

	   d = 0.0f ;
      glColor3f( 0.0f, 0.2f,0.0f);
      glVertex3f(d,  -mGridSize,0.0f);
      glVertex3f(d, 0.0f, -0.0f);
      glColor3f( 0.0f, 1.0f,0.0f);
      glVertex3f(d, mGridSize,0.0f);
      glVertex3f(d, 0.0f, -0.0f);


      glEnd();

	  glLineWidth(1.0f);

      glPopAttrib();
      
      glEndList();
      
    }
  }

#if(0)
// ray obb collision 
// returns false on ray rejection.
bool ray_obb( const LTVector &o, const LTVector &d, const SOBB &obb, float &t )
{
	float tmin = -HUGE;
	float tmax = HUGE;

	LTVector p = obb.m_Pos - o ;
	float e ;
	float f;
	float hi;
	float t1,t2;
	LTVector X = obb.m_Orientation.Right();
	LTVector Y = obb.m_Orientation.Up();
	LTVector Z = obb.m_Orientation.Forward();
	// x
	e = X.Dot(p);
	f = X.Dot(d);
	hi= obb.m_Size.x /2.0f ;
	if( fabs(f) > 0.0015 )
	{
		t1 = ( e + hi ) / f ;
		t2 = ( e - hi ) / f ;
		if( t1 > t2 ) {float v = t2 ; t2 = t1 ; t1 = v ; }
		if(t1 > tmin ) tmin = t1 ;
		if(t2 > tmin ) tmin = t2 ;
		if(tmin > tmax ) { t = 0 ; return false ; }
		if(tmax < 0  ) { t = 0 ; return false ; }
		if( ((-e - hi) > 0 ) || ( (-e + hi) < 0 )) { t = 0 ; return false ; }

	}

	e = Y.Dot(p);
	f = Y.Dot(d);
	hi= obb.m_Size.y /2.0f ;
	if( fabs(f) > 0.0015 )
	{
		t1 = ( e + hi ) / f ;
		t2 = ( e - hi ) / f ;
		if( t1 > t2 ) {float v = t2 ; t2 = t1 ; t1 = v ; }
		if(t1 > tmin ) tmin = t1 ;
		if(t2 > tmin ) tmin = t2 ;
		if(tmin > tmax ) { t = 0 ; return false ; }
		if(tmax < 0    ) { t = 0 ; return false ; }
		if( ((-e - hi) > 0 ) || ( (-e + hi) < 0 )) { t = 0 ; return false ; }

	}

	e = Z.Dot(p);
	f = Z.Dot(d);
	hi= obb.m_Size.z /2.0f ;
	if( fabs(f) > 0.0015 )
	{
		t1 = ( e + hi ) / f ;
		t2 = ( e - hi ) / f ;
		if( t1 > t2 ) {float v = t2 ; t2 = t1 ; t1 = v ; }
		if(t1 > tmin ) tmin = t1 ;
		if(t2 > tmin ) tmin = t2 ;
		if(tmin > tmax ) { t = 0 ; return false ; }
		if(tmax < 0    ) { t = 0 ; return false ; }
		if( ((-e - hi) > 0 ) || ( (-e + hi) < 0 )) { t = 0 ; return false ; }

	}
	
	if( tmin > 0 ) { t = tmin ; return true ;}
	if( tmax > 0 ) { t = tmax ; return true ;}
}
#endif

//bool obb_obb( const SOBB &A, const SOBB &B , float &t)

