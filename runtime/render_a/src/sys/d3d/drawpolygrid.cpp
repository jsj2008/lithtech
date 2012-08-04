
//
//
//
// PolyGrid drawing.  PolyGrids are height-mapped grids of bytes that are converted
// into polygons.  A PolyGrid's dimensions are based on its sample grid dimensions,
// and its Y extents are always -127 to 127.  You can make it any size by modifying
// its scale value.  For example, if you had a grid with sample data width 10 and
// sample data height 20, the grid's extents (when its m_Scale is (1,1,1,)) would be:
// (-5, -127, -10) to (5, 127, 10).  The drawing code goes to some pains to stretch
// it out to actually fit correctly since its sample points normally extend
// from 0 to (n-1) instead of 0 to n.
//
//
//
//

#include "precompile.h"

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

#include "3d_ops.h"
#include "de_objects.h"
#include "d3d_draw.h"
#include "common_draw.h"
#include "d3d_texture.h"
#include "tagnodes.h"
#include "drawobjects.h"
#include "d3d_renderworld.h"
#include "setuptouchinglights.h"
#include "iltclient.h"
#include "ltpixelshadermgr.h"
#include "rendererframestats.h"

#include "LTEffectImpl.h"
#include "lteffectshadermgr.h"
#include "ltshaderdevicestateimp.h"
#include "rendererconsolevars.h"

//Interface for the client file manager
#include "client_filemgr.h"
static IClientFileMgr* g_pIClientFileMgr;
define_holder(IClientFileMgr, g_pIClientFileMgr);


//----------------------------------------------------------------------------
// Polygrid Effect Vertex format
//   Uses: HW TnL, 1 texture channel, diffuse color, normal, 
//----------------------------------------------------------------------------
class CPolyGridEffectVertex
{
public:

	//position
	LTVector	m_Vec;

	//normal
	LTVector	m_Normal;

	//colors
	uint32		m_nColor;

	//texture coordinates
	float		m_fU0;
	float		m_fV0;

	LTVector	m_Tangent;
	LTVector	m_Binormal;
};


//----------------------------------------------------------------------------
// Polygrid Vertex format
//   Uses: HW TnL, 1 texture channel, diffuse color, normal
//----------------------------------------------------------------------------
#define POLYGRIDVERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)

class CPolyGridVertex
{
public:

	//position
	LTVector	m_Vec;

	//normal
	LTVector	m_Normal;

	//colors
	uint32		m_nColor;

	//texture coordinates
	float		m_fU0;
	float		m_fV0;
};

//The vertex buffer we can fill in with vertex information
static void*	g_TriVertList		= NULL;
static uint32	g_TriVertListSize	= 0;

//----------------------------------------------------------------------------
// Polygrid bumpmapped Vertex format
//   Uses: HW TnL, 1 texture, 3 basis vectors, color
//----------------------------------------------------------------------------
#define POLYGRIDBUMPVERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX4 | D3DFVF_TEXCOORDSIZE4(1) | D3DFVF_TEXCOORDSIZE4(2) | D3DFVF_TEXCOORDSIZE4(3))

class CPolyGridBumpVertex
{
public:

	//position
	LTVector	m_Vec;

	//colors
	uint32		m_nColor;

	//texture coordinates
	float		m_fU0;
	float		m_fV0;

	LTVector	m_vBasisRight;
	float		m_fEyeX;

	LTVector	m_vBasisUp;
	float		m_fEyeY;

	LTVector	m_vBasisForward;
	float		m_fEyeZ;

};

//----------------------------------------------------------------------------
// CFresnelTable
//   A utility class that provides a quick indexing scheme for fresnel terms
//----------------------------------------------------------------------------
class CFresnelTable
{
public:

	//generates a table based upon the volume index of refraction.
	//For good values just play around but for reference,
	//Air IOR = 1.0003
	//Water IOR = 1.333
	CFresnelTable() : m_fVolumeIOR(0.0f), m_fBaseReflection(0.0f)	{}

	//generates a table based upon the volume index of refraction.
	//For good values just play around but for reference,
	//Air IOR = 1.0003
	//Water IOR = 1.333
	void	GenerateTable(float fVolumeIOR, float fBaseReflection);

	//given a dot procuct of a normal and the viewing vector, it will return the
	//appropriate fresnel term, already loaded into the alpha component of a color
	uint32	GetValue(float fDot) const;

	//returns the IOR of the volume used to generate the table
	float	GetVolumeIOR() const		{ return m_fVolumeIOR; }

	//returns the base reflection of the volume used to generate the table
	float	GetBaseReflection() const	{ return m_fBaseReflection; }

private:

	enum	{ TABLE_SIZE	= 1024 };

	float	m_fVolumeIOR;
	float	m_fBaseReflection;

	uint32	m_nTable[TABLE_SIZE];
};

//utility function to allow easier inlining of squaring
template<class T> T Sqr(const T Val)	{ return Val * Val; }

void CFresnelTable::GenerateTable(float fVolumeIOR, float fBaseReflection)
{
	//just always assume that the player is in a standard air volume
	static const float kfViewerIOR = 1.0003f;

	//calculate the ratio of volume to viewer
	float fIORRatioSqr = Sqr(fVolumeIOR / kfViewerIOR);

	float fCos		= 0.0f;
	float fCosInc	= 1.0f / TABLE_SIZE;

	//run through and calculate our values
	for(uint32 nCurrEntry = 0; nCurrEntry < TABLE_SIZE; nCurrEntry++)
	{
		//now precalculate some values
		float fG = fIORRatioSqr + Sqr(fCos) - 1.0f;

		//figure out the final fresnel term
		float fVal = (Sqr(fG - fCos) / (2.0f * Sqr(fG + fCos))) * (1.0f + Sqr(fCos * (fG + fCos) - 1.0f) / Sqr(fCos * (fG - fCos) + 1.0f));

		//this should always be (0..1)
		assert(fVal >= 0.0f);
		assert(fVal <= 1.0f);

		//add our base reflection onto it
		fVal += fBaseReflection;

		//and now clamp it to be in range
		fVal = LTCLAMP(fVal, 0.0f, 1.0f);

		//now convert it to the appropriate format
		m_nTable[nCurrEntry] = ((uint32)(fVal * 255.0f)) << 24;

		if(nCurrEntry && m_nTable[nCurrEntry] < m_nTable[nCurrEntry - 1])
			int nBreakme = 1;

		fCos += fCosInc;
	}

	m_fVolumeIOR		= fVolumeIOR;
	m_fBaseReflection	= fBaseReflection;
}

//given a dot procuct of a normal and the viewing vector, it will return the
//appropriate fresnel term
uint32 CFresnelTable::GetValue(float fDot) const
{
	//make sure that the value is within range
	assert(fDot <= 1.0f);
	assert(fDot >= -1.0f);

	//now look into our table
	return m_nTable[(int32)(fabsf(fDot) * (TABLE_SIZE - 1))];
}

//----------------------------------------------------------------------------
// CFresnelTableCache
//   A utility class that manages several fresnel tables so that they don't
//	have to be regenerated so frequently
//----------------------------------------------------------------------------
class CFresnelTableCache
{
public:

	enum {	CACHE_SIZE	= 5 };

	CFresnelTableCache()
	{
		//init the idle counts all to 0
		for(uint32 nCurrElem = 0; nCurrElem < CACHE_SIZE; nCurrElem++)
		{
			m_nIdleCount[nCurrElem] = 0;
		}
	}

	const CFresnelTable* GetTable(float fVolumeIOR, float fBaseReflection)
	{
		//see if we have any that match
		uint32 nCurrElem;
		for(nCurrElem = 0; nCurrElem < CACHE_SIZE; nCurrElem++)
		{
			if( (fabs(m_Tables[nCurrElem].GetVolumeIOR() - fVolumeIOR) < 0.01) &&
				(fabs(m_Tables[nCurrElem].GetBaseReflection() - fBaseReflection) < 0.01))
			{
				//we have a match, clear out its idle count
				m_nIdleCount[nCurrElem] = 0;

				return &m_Tables[nCurrElem];
			}
		}

		//unable to find one, go through and find the longest idle one
		uint32 nIdle = 0;
		uint32 nIdleTime = m_nIdleCount[0];

		for(nCurrElem = 1; nCurrElem < CACHE_SIZE; nCurrElem++)
		{
			if(m_nIdleCount[nCurrElem] > nIdleTime)
			{
				nIdle		= nCurrElem;
				nIdleTime	= m_nIdleCount[nCurrElem];
			}
			else
			{
				//increment our idle time, since we weren't picked, so we will be next time
				m_nIdleCount[nCurrElem]++;
			}
		}

		//now setup and return that item
		m_nIdleCount[nIdle] = 0;
		m_Tables[nIdle].GenerateTable(fVolumeIOR, fBaseReflection);
		return &m_Tables[nIdle];
	}

private:

	CFresnelTable	m_Tables[CACHE_SIZE];
	uint32			m_nIdleCount[CACHE_SIZE];
};

//the global cache of tables
static CFresnelTableCache g_FresnelCache;

// ---------------------------------------------------------------- //
// Internal functions.
// ---------------------------------------------------------------- //

template <class VertType>
static inline void d3d_SetupVertexPos(VertType *pVertex, float fX, float fY, float fZ, uint32 nColor, float fU, float fV)
{
	pVertex->m_Vec.x		= fX;
	pVertex->m_Vec.y		= fY;
	pVertex->m_Vec.z		= fZ;
	pVertex->m_nColor		= nColor;
	pVertex->m_fU0			= fU;
	pVertex->m_fV0			= fV;
}


static void d3d_SetDefaultBlendStates()
{
	//disable the fancier approach
	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_TEXTUREFACTOR, D3DRGBA_255(255, 255, 255, 255)));

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
}


static void d3d_SetEnvMapTextureStates(const ViewParams& Params, uint32 envMapType, LTPolyGrid* pGrid, bool bCubic)
{
	//use the T-Factor for the overall translucency of the polygrid
	D3D_CALL(PD3DDEVICE->SetRenderState(D3DRS_TEXTUREFACTOR, D3DRGBA_255(255, 255, 255, pGrid->m_ColorA)));

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	PD3DDEVICE->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);

	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	PD3DDEVICE->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	PD3DDEVICE->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	//now we need to set up the environment map transform

	//get the basis vectors for world space
	LTVector vRight, vUp, vForward;
	Params.m_mView.GetBasisVectors(&vRight, &vUp, &vForward);

	//now setup the transpose, thus converting the normals into worldspace
	D3DXMATRIX mCamToWorld( vRight.x, vUp.x, vForward.x, 0.0f,
							vRight.y, vUp.y, vForward.y, 0.0f,
							vRight.z, vUp.z, vForward.z, 0.0f,
							0.0f,	  0.0f,	 0.0f,		 1.0f);

	if(!bCubic)
	{
		//setup our texture channel 1 to generate environment maps
		D3DXMATRIX mTex1Trans(	0.5f,	0.0f,	0.0f,	0.0f,
								0.0f,	0.0f,	0.0f,	0.0f,
								0.0f,	0.5f,	1.0f,	0.0f,
								0.5f,	0.5f,	0.0f,	1.0f );

		mCamToWorld = mCamToWorld * mTex1Trans;
	}

	//setup our input parameters for the texture coordinates
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, bCubic ? D3DTTFF_COUNT3 : D3DTTFF_COUNT2);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR | 1);

	//setup our texture transform
	PD3DDEVICE->SetTransform(D3DTS_TEXTURE1, &mCamToWorld);
}

static void d3d_UnsetEnvMapTextureStates()
{
	//disable the environment map transform
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,  D3DTTFF_DISABLE);
	PD3DDEVICE->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 1);

	d3d_SetDefaultBlendStates();
}

//Given an index for a vertex to calculate a normal for, as well as index offsets to
//form two basis vectors for a plane, it will calculate the normal and store it in
//the vertex of the specified index
static inline void GenerateNormal(char* pData, CPolyGridVertex* pVert, int32 nXOff1, int32 nXOff2, int32 nYOff1, int32 nYOff2,
								  float fWidth, float fHeight, float fWidthTimesHeight, float fYScale)
{
	//sanity checks!
	assert(nXOff1 != nXOff2);
	assert(nYOff1 != nYOff2);

	pVert->m_Normal.x = ((int32)pData[nXOff1] - pData[nXOff2]) * fYScale * fHeight;
	pVert->m_Normal.y = fWidthTimesHeight;
	pVert->m_Normal.z = ((int32)pData[nYOff1] - pData[nYOff2]) * fYScale * fWidth;

	//normalize our normal
	pVert->m_Normal.Normalize();

	//just a quick check to make sure that the normal is in the right hemisphere
	assert(pVert->m_Normal.Dot(LTVector(0.0f, 1.0f, 0.0f)) > 0.0f);
}

//Given an index for a vertex to calculate a basis space for, as well as index offsets to
//form two basis vectors for a plane, it will calculate the space and store it in
//the vertex of the specified index
static inline void GenerateEffectBasisSpace(char* pData, CPolyGridEffectVertex* pVert, int32 nXOff1, int32 nXOff2, int32 nYOff1, int32 nYOff2,
									  float fWidth, float fHeight, float fWidthTimesHeight, float fYScale)
{
	//sanity checks!
	assert(nXOff1 != nXOff2);
	assert(nYOff1 != nYOff2);

	pVert->m_Tangent.x = -fWidth;
	pVert->m_Tangent.y = ((int32)pData[nXOff1] - pData[nXOff2]) * fYScale;
	pVert->m_Tangent.z = 0.0f;

	pVert->m_Normal.x = 0.0f;
	pVert->m_Normal.y = ((int32)pData[nYOff1] - pData[nYOff2]) * fYScale;
	pVert->m_Normal.z = fHeight;

	pVert->m_Binormal.x = pVert->m_Tangent.y * fHeight;
	pVert->m_Binormal.y = fWidthTimesHeight;
	pVert->m_Binormal.z = pVert->m_Normal.y * fWidth;

	//normalize our normals
	pVert->m_Binormal.Normalize();
	pVert->m_Normal.Normalize();
	pVert->m_Tangent.Normalize();

	//just a quick check to make sure that the normal is in the right hemisphere
	//assert(pVert->m_vBinormal.Dot(LTVector(0.0f, 1.0f, 0.0f)) > 0.0f);
}

//Given an index for a vertex to calculate a basis space for, as well as index offsets to
//form two basis vectors for a plane, it will calculate the space and store it in
//the vertex of the specified index
static inline void GenerateBasisSpace(char* pData, CPolyGridBumpVertex* pVert, int32 nXOff1, int32 nXOff2, int32 nYOff1, int32 nYOff2,
								  float fWidth, float fHeight, float fWidthTimesHeight, float fYScale)
{
	//sanity checks!
	assert(nXOff1 != nXOff2);
	assert(nYOff1 != nYOff2);

	pVert->m_vBasisRight.x = -fWidth;
	pVert->m_vBasisRight.y = ((int32)pData[nXOff1] - pData[nXOff2]) * fYScale;
	pVert->m_vBasisRight.z = 0.0f;

	pVert->m_vBasisForward.x = 0.0f;
	pVert->m_vBasisForward.y = ((int32)pData[nYOff1] - pData[nYOff2]) * fYScale;
	pVert->m_vBasisForward.z = fHeight;

	pVert->m_vBasisUp.x = pVert->m_vBasisRight.y * fHeight;
	pVert->m_vBasisUp.y = fWidthTimesHeight;
	pVert->m_vBasisUp.z = pVert->m_vBasisForward.y * fWidth;

	//normalize our normals
	pVert->m_vBasisUp.Normalize();
	pVert->m_vBasisForward.Normalize();
	pVert->m_vBasisRight.Normalize();

	//just a quick check to make sure that the normal is in the right hemisphere
	assert(pVert->m_vBasisUp.Dot(LTVector(0.0f, 1.0f, 0.0f)) > 0.0f);
}

template <class Function, class VertType>
static void GeneratePolyGridVectors(LTPolyGrid* pGrid, VertType* pVert, Function GenFunction)
{
	//get the dims of ths polygrid
	int32 nWidth	= pGrid->m_Width;
	int32 nHeight	= pGrid->m_Height;

	//some helpful variables

	//total number of elements
	int32 nTotal = nWidth * nHeight;
	int32 nBottomRow = nWidth * (nHeight - 1);

	char* pData				= pGrid->m_Data;
	float fYScale			= pGrid->GetDims().y * 2.0f / 255.0f;
	float fWidth			= 2.0f * pGrid->m_xScale;
	float fHeight			= 2.0f * pGrid->m_yScale;
	float fWidthTimesHeight	= fWidth * fHeight;
	float fTileXScale		= fWidth / (nWidth - 1);
	float fTileZScale		= fHeight / (nHeight - 1);

	//first generate the corners
	//UL
	GenFunction(pData + 0, pVert + 0, 1, 0, nWidth, 0, fWidth, fHeight, fWidthTimesHeight, fYScale);
	//UR
	GenFunction(pData + nWidth - 1, pVert + nWidth - 1, -1, 0, 0, nWidth, fWidth, fHeight, fWidthTimesHeight, fYScale);
	//LL
	GenFunction(pData + nBottomRow, pVert + nBottomRow, 1, 0, 0, -nWidth, fWidth, fHeight, fWidthTimesHeight, fYScale);
	//LR
	GenFunction(pData + nTotal - 1, pVert + nTotal - 1, 0, -1, 0, -nWidth, fWidth, fHeight, fWidthTimesHeight, fYScale);

	//now generate each edge
	int32 nCurrX, nCurrY;

	for(nCurrX = 1; nCurrX < nWidth - 1; nCurrX++)
	{
		//top
		GenFunction(pData + nCurrX, pVert + nCurrX, -1, 1, 0, nWidth, fWidth, fHeight, fWidthTimesHeight, fYScale);
		//bottom
		GenFunction(pData + nBottomRow + nCurrX, pVert + nBottomRow + nCurrX, -1, 1, -nWidth, 0, fWidth, fHeight, fWidthTimesHeight, fYScale);
	}

	for(nCurrY = nWidth; nCurrY < nTotal - nWidth; nCurrY += nWidth)
	{
		//left
		GenFunction(pData + nCurrY, pVert + nCurrY, 0, 1, -nWidth, nWidth, fWidth, fHeight, fWidthTimesHeight, fYScale);
		//right
		GenFunction(pData + nCurrY + nWidth - 1, pVert + nCurrY + nWidth - 1, 0, -1, nWidth, -nWidth, fWidth, fHeight, fWidthTimesHeight, fYScale);
	}

	//now generate the internals of the polygrid
	for(nCurrY = 1; nCurrY < nHeight - 1; nCurrY++)
	{
		uint32 nPos = nCurrY * nWidth + 1;

		for(nCurrX = 1; nCurrX < nWidth - 1; nCurrX++)
		{
			GenFunction(pData + nPos, pVert + nPos, -1, 1, -nWidth, nWidth, fWidth, fHeight, fWidthTimesHeight, fYScale);
			nPos++;
		}
	}
}

static void GeneratePolyGridFresnelAlpha(const LTVector& vViewPos, CPolyGridVertex* pVerts, LTPolyGrid* pGrid, uint32 nNumVerts)
{
	//we need to transform the camera position into our view space
	LTMatrix mInvWorldTrans;

	mInvWorldTrans.Identity();
	mInvWorldTrans.SetTranslation(-pGrid->GetPos());

	LTMatrix mOrientation;
	pGrid->m_Rotation.ConvertToMatrix(mOrientation);

	mInvWorldTrans = mOrientation * mInvWorldTrans;

	LTVector vCameraPos = mInvWorldTrans * vViewPos;

	//now generate the internals of the polygrid
	CPolyGridVertex* pCurrVert	= pVerts;
	CPolyGridVertex* pEnd		= pCurrVert + nNumVerts;

	//determine the fresnel table that we are going to be using
	const CFresnelTable* pTable = g_FresnelCache.GetTable(LTMAX(1.0003f, pGrid->m_fFresnelVolumeIOR), pGrid->m_fBaseReflection);

	//use a vector from the camera to the center of the grid to base our approximations off of. The further
	//we get to the edges the more likely this error will be, but it is better than another sqrt per vert
	LTVector vToPGPt;

	while(pCurrVert < pEnd)
	{
		//the correct but slow way, so only do it every once in a while
		//if((pCurrVert - g_TriVertList) % 4 == 0)
		{
			vToPGPt = vCameraPos - pCurrVert->m_Vec;
		}

		pCurrVert->m_nColor |= pTable->GetValue(vToPGPt.Dot(pCurrVert->m_Normal) / vToPGPt.Mag());
		++pCurrVert;
	}
}

static void GeneratePolyGridFresnelAlphaAndCamera(const LTVector& vViewPos, CPolyGridBumpVertex* pVerts, LTPolyGrid* pGrid, uint32 nNumVerts)
{
	//we need to transform the camera position into our view space
	LTMatrix mInvWorldTrans;

	mInvWorldTrans.Identity();
	mInvWorldTrans.SetTranslation(-pGrid->GetPos());

	LTMatrix mOrientation;
	pGrid->m_Rotation.ConvertToMatrix(mOrientation);

	mInvWorldTrans = mOrientation * mInvWorldTrans;

	LTVector vCameraPos = mInvWorldTrans * vViewPos;

	//now generate the internals of the polygrid
	CPolyGridBumpVertex* pCurrVert	= pVerts;
	CPolyGridBumpVertex* pEnd		= pCurrVert + nNumVerts;

	//determine the fresnel table that we are going to be using
	const CFresnelTable* pTable = g_FresnelCache.GetTable(LTMAX(1.0003f, pGrid->m_fFresnelVolumeIOR), pGrid->m_fBaseReflection);

	//use a vector from the camera to the center of the grid to base our approximations off of. The further
	//we get to the edges the more likely this error will be, but it is better than another sqrt per vert
	LTVector vToPGPt;

	while(pCurrVert < pEnd)
	{
		//the correct but slow way, so only do it every once in a while
		//if((pCurrVert - g_TriVertList) % 4 == 0)
		{
			vToPGPt = vCameraPos - pCurrVert->m_Vec;
			vToPGPt.Normalize();
		}

		pCurrVert->m_fEyeX = vToPGPt.x;
		pCurrVert->m_fEyeY = vToPGPt.y;
		pCurrVert->m_fEyeZ = vToPGPt.z;

		pCurrVert->m_nColor |= pTable->GetValue(vToPGPt.Dot(pCurrVert->m_vBasisUp));
		++pCurrVert;
	}
}



void d3d_DrawPolyGrid(const ViewParams &Params, LTObject *pObj)
{
	//the global pixel shader to be used for the water bumpmapping and a flag indicating if
	//creation failed
	bool				s_bPixelShaderFailed = false;

	//make sure that the polygrid is valid
	assert(pObj);

	//get our polygrid
    LTPolyGrid *pGrid = (LTPolyGrid*)pObj;

	// Make sure it's initialized.
	if(!pGrid->m_Data)
		return;

	//must have an index buffer
	if(!pGrid->m_Indices || (pGrid->m_nIndices == 0))
		return;

	//and of course, make sure that the size is reasonable
	if((pGrid->m_Width < 2) || (pGrid->m_Height < 2))
		return;

	IncFrameStat(eFS_PolyGridTriangles, pGrid->m_nTris);

	//cache the half dimensions
	float fHalfGridWidth  = ((float)pGrid->m_Width - 1) * 0.5f;
	float fHalfGridHeight = ((float)pGrid->m_Height - 1) * 0.5f;

	//now we need to build our color lookup table for this polygrid, this is faster for any
	//polygrid that is larger than 8x8, and even then we can get a nice tight loop that
	//should still be rather optimal

	// Set the blending mode based on the flags.
	uint32 nSrcBlend, nDestBlend, nFog, nFogColor;
	d3d_GetBlendStates(pGrid, nSrcBlend, nDestBlend, nFog, nFogColor);
	StateSet ssSrcBlend(D3DRS_SRCBLEND, nSrcBlend);
	StateSet ssDestBlend(D3DRS_DESTBLEND, nDestBlend);
	StateSet ssFog(D3DRS_FOGENABLE, nFog);
	StateSet ssFogColor(D3DRS_FOGCOLOR, nFogColor);

	//flag indicating whether or not
	bool bEnvMap		= false;
	bool bCubicEnvMap	= false;
	bool bBumpMap		= false;

	//vertex information
	uint32 nVertexSize  = sizeof(CPolyGridVertex);
	uint32 nVertexFVF	= POLYGRIDVERTEX_FORMAT;

	//UV scales to adjust the texture by (defaults to 1, but if we are bump mapping, this will
	//be set to the detail texture scale)
	float	fPGUScale	= 1.0f;
	float	fPGVScale	= 1.0f;

	// Set the texture if necessary.
	SpriteTracker* pTracker;

	LTPixelShader *pPixelShader = NULL;

	if(pGrid->m_pSprite)
	{
		pTracker = &pGrid->m_SpriteTracker;
		if(pTracker->m_pCurFrame)
		{
			SharedTexture* pTex = pTracker->m_pCurFrame->m_pTex;
			if(pTex)
			{
				//get the type of this texture
				ESharedTexType eTexType = pTex->m_eTexType;

				//the base texture
				SharedTexture* pBaseTex		= NULL;
				//the environment map
				SharedTexture* pEnvMapTex	= pGrid->m_pEnvMap;
				//the bumpmap
				SharedTexture* pBumpMapTex	= NULL;

				//determine our texture data
				if(pGrid->m_nPGFlags & PG_NORMALMAPSPRITE)
				{
					//only use a bump map texture if we actually have an environment map
					if(pEnvMapTex)
						pBumpMapTex	= pTex;
				}
				else
				{
					//we have a normal texture
					pBaseTex	= pTex;
				}

				// Set up the environment mapping texture if applicable
				if(	pEnvMapTex && g_CV_EnvMapPolyGrids.m_Val)
				{
					//see if the is a cubic environment map
					RTexture *pRenderTexture = (RTexture*)pEnvMapTex->m_pRenderData;
					if(pRenderTexture && pRenderTexture->IsCubeMap())
					{
						bCubicEnvMap = true;
					}

					bEnvMap = true;
				}

				if(	pBumpMapTex && g_CV_BumpMapPolyGrids.m_Val)
				{
					//we now need to make sure that the bump map shader is valid

					//see if we need to load it
					if (!s_bPixelShaderFailed)
					{
						// Get the pixel shader.
						pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(LTPixelShader::PIXELSHADER_ENVBUMPMAP);
						if (NULL == pPixelShader)
						{
							FileRef ref;
							ref.m_FileType 	= FILE_ANYFILE;
							ref.m_pFilename = "ps\\envbumpmap.psh";

							// Try to load it.
							ILTStream *pStream = g_pIClientFileMgr->OpenFile(&ref);
							if (NULL != pStream)
							{
								if (LTPixelShaderMgr::GetSingleton().AddPixelShader(pStream, ref.m_pFilename,
																					LTPixelShader::PIXELSHADER_ENVBUMPMAP, true))
								{
									pPixelShader = LTPixelShaderMgr::GetSingleton().GetPixelShader(LTPixelShader::PIXELSHADER_ENVBUMPMAP);
								}

								// Close the file.
								pStream->Release();
							}
						}
					}

					// See if we can continue.
					if (NULL != pPixelShader && pPixelShader->IsValidShader())
					{
						bBumpMap	= true;
						nVertexSize = sizeof(CPolyGridBumpVertex);
						nVertexFVF	= POLYGRIDBUMPVERTEX_FORMAT;
						fPGUScale	= ((RTexture*)pBumpMapTex->m_pRenderData)->m_DetailTextureScale;
						fPGVScale	= fPGUScale;
					}
					else
					{
						//we failed to create the shader, the device can't support it
						s_bPixelShaderFailed = true;
					}
				}

				//setup the textures based upon the configuration
				if(bBumpMap)
				{
					d3d_SetTexture(pBumpMapTex, 0, eFS_PolyGridBumpMapTexMemory);
					d3d_SetTexture(pEnvMapTex, 3, eFS_PolyGridEnvMapTexMemory);
				}
				else
				{
					LTEffectImpl* pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pGrid->m_nEffectShaderID);
					if(pEffect)
					{
						ID3DXEffect* pD3DEffect = pEffect->GetEffect();
						if(pD3DEffect)
						{
							nVertexSize = sizeof(CPolyGridEffectVertex); 

							RTexture* pRTexture = (RTexture*)pBaseTex->m_pRenderData;
							pD3DEffect->SetTexture("texture0", pRTexture->m_pD3DTexture);

							if(bEnvMap)
							{
								pRTexture = (RTexture*)pEnvMapTex->m_pRenderData;
								pD3DEffect->SetTexture("texture1", pRTexture->m_pD3DTexture);
							}

						}
					}else
					{
						d3d_SetTexture(pBaseTex, 0, eFS_PolyGridBaseTexMemory);

						if(bEnvMap)
						{
							d3d_SetTexture(pEnvMapTex, 1, eFS_PolyGridEnvMapTexMemory);
						}
					}
				}
			}
			else
			{
				//if we didn't set any texture, we need to make sure and clear out the texture
				//channel
				d3d_DisableTexture(0);
			}
		}
	}

	//specify that we were visible
	pGrid->m_Flags |= FLAG_INTERNAL1;

	// Build the vertex list.
	uint32 nBufferSize = pGrid->m_Width * pGrid->m_Height * nVertexSize;
	if(nBufferSize > g_TriVertListSize)
	{
		//we need to reallocate our list of vertices
		dfree(g_TriVertList);
		LT_MEM_TRACK_ALLOC(g_TriVertList = dalloc(nBufferSize),LT_MEM_TYPE_RENDERER);

		//check the allocation
		if(!g_TriVertList)
		{
			g_TriVertListSize = 0;
			return;
		}

		g_TriVertListSize = nBufferSize;
	}

	//determine if this polygrid should be lit or not
	bool bFresnel = (pGrid->m_nPGFlags & PG_FRESNEL) && (g_CV_FresnelPolyGrids.m_Val) ? true : false;

	//determine what alpha value we should use
	uint8 nColorAlpha = 0;
	if(!bEnvMap)
		nColorAlpha = 255;
	else if(!bFresnel)
		nColorAlpha = 128;

	float fScaledR = pGrid->m_ColorR * MATH_ONE_OVER_255;
	float fScaledG = pGrid->m_ColorG * MATH_ONE_OVER_255;
	float fScaledB = pGrid->m_ColorB * MATH_ONE_OVER_255;

	uint32 nColorTable[256];
	for(uint32 nCurrColor = 0; nCurrColor < 256; nCurrColor++)
	{
		//Note that the color table is 0..255, object color is 0..255 as well
		nColorTable[nCurrColor] = D3DRGBA_255(	pGrid->m_ColorTable[nCurrColor].x * fScaledR,
												pGrid->m_ColorTable[nCurrColor].y * fScaledG,
												pGrid->m_ColorTable[nCurrColor].z * fScaledB,
												nColorAlpha);
	}

	//setup our world matrix to represent a space that holds the orientation of the polygrid
	//as well as the center position. This allows all operations to take place
	//as if performed on the XZ plane (note that we don't do the scale as that tends
	//to mess up normals)
	LTMatrix mWorldTrans;
	LTVector vUnitScale(1.0f, 1.0f, 1.0f);
	d3d_SetupTransformation(&pGrid->GetPos(), (float*)&pGrid->m_Rotation, &vUnitScale, &mWorldTrans);
	d3d_SetD3DMat(D3DTS_WORLD, &mWorldTrans);

	//calculate our position increments
	float fXInc			= pGrid->GetDims().x * 2.0f / (pGrid->m_Width - 1);
	float fZInc			= pGrid->GetDims().z * 2.0f / (pGrid->m_Height - 1);
	float fYScale		= pGrid->GetDims().y / 127.0f;

	float fXStart		= -fHalfGridWidth * fXInc;
	float fCurrX		= fXStart;
	float fCurrZ		= -fHalfGridHeight * fZInc;


	int8* pDataPos		= (int8*)pGrid->m_Data;
	int8* pDataEnd		= pDataPos + pGrid->m_Width * pGrid->m_Height;
	int8* pLineDataEnd	= pDataPos + pGrid->m_Width;

	uint32* pColor		= nColorTable + 128;

	float fXScale		= pGrid->m_xScale / ((pGrid->m_Width - 1) * fXInc);
	float fZScale		= pGrid->m_yScale / ((pGrid->m_Height - 1) * fZInc);

	float fStartU		= (float)fmod(pGrid->m_xPan * fPGUScale, 1.0f);
	float fStartV		= (float)fmod(pGrid->m_yPan * fPGVScale, 1.0f);

	float fCurrU		= fStartU;
	float fCurrV		= fStartV;

	float fUInc			= fXInc * fXScale * fPGUScale;
	float fVInc			= fZInc * fZScale * fPGVScale;

	int32 nWidth		= pGrid->m_Width;
	float fSpacingX		= fXInc * 2.0f;
	float fSpacingZ		= fZInc * 2.0f;

	uint32 nNumVerts;

	bool bEffect = false;
	LTEffectImpl* pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pGrid->m_nEffectShaderID);
	if(pEffect)
	{
		ID3DXEffect* pD3DEffect = pEffect->GetEffect();
		if(pD3DEffect)
		{
			bEffect = true;
		}
	}

	if(bBumpMap)
	{
		CPolyGridBumpVertex* pVertexPos = (CPolyGridBumpVertex*)g_TriVertList;

		if(pGrid->m_pValidMask)
		{
			//this polygrid has a valid mask, meaning that we need to skip over vertices
			//as needed
			uint32* pCurrMask = pGrid->m_pValidMask;

			//amount to adjust the mask at the end of a line
			uint32 nMaskLineAdjust = (pGrid->m_Width % 32) ? 1 : 0;

			uint32 nShift;
			uint32 nNormalY1 = 0;
			uint32 nNormalY2 = nWidth;

			float fWidthTimesHeight = fSpacingX * fSpacingZ;

			while(pDataPos < pDataEnd)
			{
				nShift = 0x1;

				while(pDataPos < pLineDataEnd)
				{
					if(*pCurrMask & nShift)
					{
						//this is valid, add this vertex
						d3d_SetupVertexPos(pVertexPos, fCurrX, *pDataPos * fYScale, fCurrZ, pColor[*pDataPos], fCurrU, fCurrV);

						//generate a normal for it
						GenerateBasisSpace(pDataPos, pVertexPos, -1, 1, nNormalY1, nNormalY2, fSpacingX, fSpacingZ, fWidthTimesHeight, fYScale);

						//move along to the next vertex
						pVertexPos++;
					}

					pDataPos++;
					fCurrX += fXInc;
					fCurrU += fUInc;

					if(nShift == 0x80000000)
					{
						pCurrMask++;
						nShift = 1;
					}
					else
					{
						nShift <<= 1;
					}
				}

				//reset the line
				fCurrX = fXStart;

				//update our threshold for when to move onto the next line
				pLineDataEnd += pGrid->m_Width;
				pCurrMask += nMaskLineAdjust;

				//update our position
				fCurrZ += fZInc;
				fCurrU  = fStartU;
				fCurrV += fVInc;

				//update the normal offsets to ensure we don't go outside of our buffer
				nNormalY1 = -nWidth;
				if(pLineDataEnd >= pDataEnd)
					nNormalY2 = 0;
			}

			nNumVerts = pVertexPos - (CPolyGridBumpVertex*)g_TriVertList;
		}
		else
		{
			nNumVerts = pGrid->m_Width * pGrid->m_Height;

			while(pDataPos < pDataEnd)
			{
				while(pDataPos < pLineDataEnd)
				{
					d3d_SetupVertexPos(pVertexPos, fCurrX, *pDataPos * fYScale, fCurrZ, pColor[*pDataPos], fCurrU, fCurrV);

					pDataPos++;
					pVertexPos++;
					fCurrX += fXInc;
					fCurrU += fUInc;
				}

				//reset the line
				fCurrX = fXStart;

				//update our threshold for when to move onto the next line
				pLineDataEnd += pGrid->m_Width;

				//update our position
				fCurrZ += fZInc;
				fCurrU  = fStartU;
				fCurrV += fVInc;
			}

			//now we need to generate the normals for the polygrid
			GeneratePolyGridVectors(pGrid, (CPolyGridBumpVertex*)g_TriVertList, GenerateBasisSpace);
		}
	}
	else if(bEffect)
	{
		CPolyGridEffectVertex* pVertexPos = (CPolyGridEffectVertex*)g_TriVertList;

		if(pGrid->m_pValidMask)
		{
			//this polygrid has a valid mask, meaning that we need to skip over vertices
			//as needed
			uint32* pCurrMask = pGrid->m_pValidMask;

			//amount to adjust the mask at the end of a line
			uint32 nMaskLineAdjust = (pGrid->m_Width % 32) ? 1 : 0;

			uint32 nShift;
			uint32 nNormalY1 = 0;
			uint32 nNormalY2 = nWidth;

			float fWidthTimesHeight = fSpacingX * fSpacingZ;

			while(pDataPos < pDataEnd)
			{
				nShift = 0x1;

				while(pDataPos < pLineDataEnd)
				{
					if(*pCurrMask & nShift)
					{
						//this is valid, add this vertex
						d3d_SetupVertexPos(pVertexPos, fCurrX, *pDataPos * fYScale, fCurrZ, pColor[*pDataPos], fCurrU, fCurrV);

						//generate a normal for it
						GenerateEffectBasisSpace(pDataPos, pVertexPos, -1, 1, nNormalY1, nNormalY2, fSpacingX, fSpacingZ, fWidthTimesHeight, fYScale);

						//move along to the next vertex
						pVertexPos++;
					}

					pDataPos++;
					fCurrX += fXInc;
					fCurrU += fUInc;

					if(nShift == 0x80000000)
					{
						pCurrMask++;
						nShift = 1;
					}
					else
					{
						nShift <<= 1;
					}
				}

				//reset the line
				fCurrX = fXStart;

				//update our threshold for when to move onto the next line
				pLineDataEnd += pGrid->m_Width;
				pCurrMask += nMaskLineAdjust;

				//update our position
				fCurrZ += fZInc;
				fCurrU  = fStartU;
				fCurrV += fVInc;

				//update the normal offsets to ensure we don't go outside of our buffer
				nNormalY1 = -nWidth;
				if(pLineDataEnd >= pDataEnd)
					nNormalY2 = 0;
			}

			nNumVerts = pVertexPos - (CPolyGridEffectVertex*)g_TriVertList;
		}
		else
		{
			nNumVerts = pGrid->m_Width * pGrid->m_Height;

			while(pDataPos < pDataEnd)
			{
				while(pDataPos < pLineDataEnd)
				{
					d3d_SetupVertexPos(pVertexPos, fCurrX, *pDataPos * fYScale, fCurrZ, pColor[*pDataPos], fCurrU, fCurrV);

					pDataPos++;
					pVertexPos++;
					fCurrX += fXInc;
					fCurrU += fUInc;
				}

				//reset the line
				fCurrX = fXStart;

				//update our threshold for when to move onto the next line
				pLineDataEnd += pGrid->m_Width;

				//update our position
				fCurrZ += fZInc;
				fCurrU  = fStartU;
				fCurrV += fVInc;
			}

			//now we need to generate the normals for the polygrid
			LTEffectImpl* pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pGrid->m_nEffectShaderID);
			if(pEffect)
			{
				ID3DXEffect* pD3DEffect = pEffect->GetEffect();
				if(pD3DEffect)
				{
					GeneratePolyGridVectors(pGrid, (CPolyGridEffectVertex*)g_TriVertList, GenerateEffectBasisSpace);
				}
			}
			else
			{
				GeneratePolyGridVectors(pGrid, (CPolyGridVertex*)g_TriVertList, GenerateNormal);
			}
		}
	}
	else //fixed function
	{
		CPolyGridVertex* pVertexPos = (CPolyGridVertex*)g_TriVertList;

		if(pGrid->m_pValidMask)
		{
			//this polygrid has a valid mask, meaning that we need to skip over vertices
			//as needed
			uint32* pCurrMask = pGrid->m_pValidMask;

			//amount to adjust the mask at the end of a line
			uint32 nMaskLineAdjust = (pGrid->m_Width % 32) ? 1 : 0;

			uint32 nShift;
			uint32 nNormalY1 = 0;
			uint32 nNormalY2 = nWidth;

			float fWidthTimesHeight = fSpacingX * fSpacingZ;

			while(pDataPos < pDataEnd)
			{
				nShift = 0x1;

				while(pDataPos < pLineDataEnd)
				{
					if(*pCurrMask & nShift)
					{
						//this is valid, add this vertex
						d3d_SetupVertexPos(pVertexPos, fCurrX, *pDataPos * fYScale, fCurrZ, pColor[*pDataPos], fCurrU, fCurrV);

						//generate a normal for it
						GenerateNormal(pDataPos, pVertexPos, -1, 1, nNormalY1, nNormalY2, fSpacingX, fSpacingZ, fWidthTimesHeight, fYScale);

						//move along to the next vertex
						pVertexPos++;
					}

					pDataPos++;
					fCurrX += fXInc;
					fCurrU += fUInc;

					if(nShift == 0x80000000)
					{
						pCurrMask++;
						nShift = 1;
					}
					else
					{
						nShift <<= 1;
					}
				}

				//reset the line
				fCurrX = fXStart;

				//update our threshold for when to move onto the next line
				pLineDataEnd += pGrid->m_Width;
				pCurrMask += nMaskLineAdjust;

				//update our position
				fCurrZ += fZInc;
				fCurrU  = fStartU;
				fCurrV += fVInc;

				//update the normal offsets to ensure we don't go outside of our buffer
				nNormalY1 = -nWidth;
				if(pLineDataEnd >= pDataEnd)
					nNormalY2 = 0;
			}

			nNumVerts = pVertexPos - (CPolyGridVertex*)g_TriVertList;
		}
		else
		{
			nNumVerts = pGrid->m_Width * pGrid->m_Height;

			while(pDataPos < pDataEnd)
			{
				while(pDataPos < pLineDataEnd)
				{
					d3d_SetupVertexPos(pVertexPos, fCurrX, *pDataPos * fYScale, fCurrZ, pColor[*pDataPos], fCurrU, fCurrV);

					pDataPos++;
					pVertexPos++;
					fCurrX += fXInc;
					fCurrU += fUInc;
				}

				//reset the line
				fCurrX = fXStart;

				//update our threshold for when to move onto the next line
				pLineDataEnd += pGrid->m_Width;

				//update our position
				fCurrZ += fZInc;
				fCurrU  = fStartU;
				fCurrV += fVInc;
			}

			//now we need to generate the normals for the polygrid
			LTEffectImpl* pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pGrid->m_nEffectShaderID);
			if(pEffect)
			{
				ID3DXEffect* pD3DEffect = pEffect->GetEffect();
				if(pD3DEffect)
				{
					GeneratePolyGridVectors(pGrid, (CPolyGridEffectVertex*)g_TriVertList, GenerateEffectBasisSpace);
				}
			}
			else
			{
				GeneratePolyGridVectors(pGrid, (CPolyGridVertex*)g_TriVertList, GenerateNormal);
			}
		}
	}

	// Set environment map texture coordinates.
	if(bEnvMap && !bBumpMap)
	{
		d3d_SetEnvMapTextureStates(Params, pTracker->m_pCurFrame->m_pTex->m_eTexType, pGrid, bCubicEnvMap);
	}

	//see if we are just doing a base texture
	if(!bEnvMap && !bBumpMap)
	{
		d3d_SetDefaultBlendStates();
	}

	//generate the alpha if we can use it
	if(bBumpMap)
	{
		GeneratePolyGridFresnelAlphaAndCamera(Params.m_Pos, (CPolyGridBumpVertex*)g_TriVertList, pGrid, nNumVerts);
	}
	else if(bFresnel)
	{
		GeneratePolyGridFresnelAlpha(Params.m_Pos, (CPolyGridVertex*)g_TriVertList, pGrid, nNumVerts);
	}

	//make the backfacing polygons cull
	StateSet ssCullMode(D3DRS_CULLMODE, (pGrid->m_nPGFlags & PG_NOBACKFACECULL) ? D3DCULL_NONE : D3DCULL_CCW);

	//setup the pixel shader if we are bumpmapping
	if(bBumpMap)
	{
		assert(NULL != pPixelShader && pPixelShader->IsValidShader());

		// Set the pixel shader constants.
		float *pConstants = pPixelShader->GetConstants();
		pConstants[0] = 0.0f;
		pConstants[1] = 0.0f;
		pConstants[2] = 0.0f;
		pConstants[3] = pGrid->m_ColorA / 255.0f;
		LTPixelShaderMgr::GetSingleton().SetPixelShaderConstants(pPixelShader);

		// Install the pixel shader.
		LTPixelShaderMgr::GetSingleton().InstallPixelShader(pPixelShader);

		//now actually draw the polygrid
		D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
		D3D_CALL(PD3DDEVICE->SetFVF(nVertexFVF));
		
		int nNumPolies = (pGrid->m_nIndices/3);

		// Is this polygrid larger than our buffer? If so, break it into smaller patches.
		if(nNumPolies > g_CV_PolyGridBufferSize)
		{
			int32 nRemainingPolies = nNumPolies;
			uint32 nCurrentVertPosition = 0;

			while(nRemainingPolies > 0)
			{
				uint32 nPoliesThisFrame = (nRemainingPolies > g_CV_PolyGridBufferSize) ? g_CV_PolyGridBufferSize: nRemainingPolies;
				D3D_CALL(PD3DDEVICE->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,nNumVerts,nPoliesThisFrame,&pGrid->m_Indices[nCurrentVertPosition],D3DFMT_INDEX16,g_TriVertList, nVertexSize));
				nCurrentVertPosition += nPoliesThisFrame*3;
				nRemainingPolies -= nPoliesThisFrame;
			}
		}
		else
		{
			D3D_CALL(PD3DDEVICE->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,nNumVerts,(pGrid->m_nIndices)/3,pGrid->m_Indices,D3DFMT_INDEX16,g_TriVertList, nVertexSize));
		}

		// Uninstall the pixel shader.
		LTPixelShaderMgr::GetSingleton().UninstallPixelShader();

		d3d_DisableTexture(0);
		d3d_DisableTexture(3);
	}
	else
	{
		//now actually draw the polygrid
		D3D_CALL(PD3DDEVICE->SetVertexShader(NULL));
		D3D_CALL(PD3DDEVICE->SetFVF(nVertexFVF));

		int nNumPolies = (pGrid->m_nIndices/3);

		// Is this polygrid larger than our buffer? If so, break it into smaller patches.
		if(nNumPolies > g_CV_PolyGridBufferSize)
		{
			int32 nRemainingPolies = nNumPolies;
			uint32 nCurrentVertPosition = 0;

			while(nRemainingPolies > 0)
			{
				uint32 nPoliesThisFrame = (nRemainingPolies > g_CV_PolyGridBufferSize) ? g_CV_PolyGridBufferSize: nRemainingPolies;
				
				LTEffectImpl* pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pGrid->m_nEffectShaderID);
				if(pEffect)
				{
					pEffect->UploadVertexDeclaration();

					ID3DXEffect* pD3DEffect = pEffect->GetEffect();
					if(pD3DEffect)
					{
						i_client_shell->OnEffectShaderSetParams(pEffect, NULL, NULL, LTShaderDeviceStateImp::GetSingleton());

						UINT nPasses = 0;
						pD3DEffect->Begin(&nPasses, 0);

						for(int i = 0; i < nPasses; ++i)
						{
							pD3DEffect->BeginPass(i);
							D3D_CALL(PD3DDEVICE->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,nNumVerts,nPoliesThisFrame,&pGrid->m_Indices[nCurrentVertPosition],D3DFMT_INDEX16,g_TriVertList, nVertexSize));
							pD3DEffect->EndPass();
						}

						pD3DEffect->End();
					}

				}
				else
				{
					D3D_CALL(PD3DDEVICE->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,nNumVerts,nPoliesThisFrame,&pGrid->m_Indices[nCurrentVertPosition],D3DFMT_INDEX16,g_TriVertList, nVertexSize));
				}

				nCurrentVertPosition += nPoliesThisFrame*3;
				nRemainingPolies -= nPoliesThisFrame;
			}
		}
		else
		{
			LTEffectImpl* pEffect = (LTEffectImpl*)LTEffectShaderMgr::GetSingleton().GetEffectShader(pGrid->m_nEffectShaderID);
			if(pEffect)
			{
				pEffect->UploadVertexDeclaration();

				ID3DXEffect* pD3DEffect = pEffect->GetEffect();
				if(pD3DEffect)
				{
					i_client_shell->OnEffectShaderSetParams(pEffect, NULL, NULL, LTShaderDeviceStateImp::GetSingleton());

					UINT nPasses = 0;
					pD3DEffect->Begin(&nPasses, 0);
					
					for(int i = 0; i < nPasses; ++i)
					{
						pD3DEffect->BeginPass(i);
						D3D_CALL(PD3DDEVICE->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,nNumVerts,(pGrid->m_nIndices)/3,pGrid->m_Indices,D3DFMT_INDEX16,g_TriVertList, nVertexSize));
						pD3DEffect->EndPass();
					}

					pD3DEffect->End();
				}

			}
			else
			{
				// No Effect Shader, just fixed function.
				D3D_CALL(PD3DDEVICE->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,nNumVerts,(pGrid->m_nIndices)/3,pGrid->m_Indices,D3DFMT_INDEX16,g_TriVertList, nVertexSize));
			}
			
		}

		d3d_DisableTexture(0);
		d3d_DisableTexture(1);
	}

	if (bEnvMap)
		d3d_UnsetEnvMapTextureStates();

	//reset our world transform so that it won't mess up the rendering of other objects
	d3d_SetD3DMat(D3DTS_WORLD, &Params.m_mIdentity);
}


// ---------------------------------------------------------------- //
// External functions.
// ---------------------------------------------------------------- //
void d3d_ProcessPolyGrid(LTObject *pObject)
{
	if(!g_CV_DrawPolyGrids)
		return;

	if (pObject->IsTranslucent())
	{
		//see if it needs to be rendered before other translucent objects though
		if(pObject->ToPolyGrid()->m_nPGFlags & PG_RENDERBEFORETRANSLUCENTS)
		{
			d3d_GetVisibleSet()->m_EarlyTranslucentPolyGrids.Add(pObject);
		}
		else
		{
			d3d_GetVisibleSet()->m_TranslucentPolyGrids.Add(pObject);
		}
	}
	else
	{
		d3d_GetVisibleSet()->m_SolidPolyGrids.Add(pObject);
	}
}


void d3d_DrawSolidPolyGrids(const ViewParams& Params)
{
	d3d_GetVisibleSet()->m_SolidPolyGrids.Draw(Params, d3d_DrawPolyGrid);
}

void d3d_DrawEarlyTranslucentPolyGrids(const ViewParams& Params)
{
	d3d_GetVisibleSet()->m_EarlyTranslucentPolyGrids.Draw(Params, d3d_DrawPolyGrid);
}

// Translucent polygrid queueing hook function for sorting
void d3d_QueueTranslucentPolyGrids(const ViewParams& Params, ObjectDrawList& DrawList)
{
	d3d_GetVisibleSet()->m_TranslucentPolyGrids.Queue(DrawList, Params, d3d_DrawPolyGrid);
}

void d3d_TermPolyGridDraw()
{
	dfree(g_TriVertList);

	g_TriVertList = LTNULL;
	g_TriVertListSize = 0;
}


