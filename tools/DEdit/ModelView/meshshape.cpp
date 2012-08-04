#include "bdefs.h"
#include "dedit.h"
#include "meshshape.h"
#include "draw_d3d.h"



CMeshShape::CMeshShape() :
	m_nNumVerts(0),
	m_nNumTris(0),
	m_pNormals(NULL),
	m_pOriginalPos(NULL),
	m_pRenderVerts(NULL),
	m_pTriangleList(NULL),
	m_pszTextureFilename(NULL),
	m_pTextureFile(NULL)
{
}

CMeshShape::~CMeshShape()
{
	FreeTriList();
	FreeVertexList();

	delete [] m_pszTextureFilename;
}


//sets the number of vertices for this mesh
bool CMeshShape::SetNumVertices(uint32 nNumVerts)
{
	//out with the old
	FreeVertexList();

	//in with the new
	m_pOriginalPos	= new LTVector[nNumVerts];
	m_pRenderVerts	= new TLVertex[nNumVerts];
	m_pNormals		= new LTVector[nNumVerts];

	if((m_pOriginalPos == NULL) || (m_pRenderVerts == NULL) || (m_pNormals == NULL))
	{
		FreeVertexList();
		return false;
	}

	m_nNumVerts = nNumVerts;

	//run through and set up the colors
	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		m_pRenderVerts[nCurrVert].color = 0xFFFFFFFF;
		m_pRenderVerts[nCurrVert].tu = 0.0f;
		m_pRenderVerts[nCurrVert].tv = 0.0f;
	}

	return true;
}


//gets the number of vertices for this mesh
uint32 CMeshShape::GetNumVertices() const
{
	return m_nNumVerts;
}

//gets the bounding box for the shape
bool CMeshShape::GetBoundingBox(LTVector& vMin, LTVector& vMax)
{
	if(GetNumVertices() == 0)
	{
		return false;
	}

	//set up with the initial point
	vMin = m_pOriginalPos[0];
	vMax = vMin;

	//now find the bounding box
	for(uint32 nCurrVert = 1; nCurrVert < GetNumVertices(); nCurrVert++)
	{
		VEC_MIN(vMin, vMin, m_pOriginalPos[nCurrVert]); 
		VEC_MAX(vMax, vMax, m_pOriginalPos[nCurrVert]);
	}

	return true;
}


//sets the number of triangles in the mesh
bool CMeshShape::SetNumTriangles(uint32 nNumTris)
{
	FreeTriList();

	m_pTriangleList = new uint16[nNumTris * 3];

	if(m_pTriangleList)
	{
		m_nNumTris = nNumTris;
		return true;
	}

	return false;
}


//gets the number of triangles in the mesh
uint32 CMeshShape::GetNumTriangles() const
{
	return m_nNumTris;
}


//sets the position of a specified vertex
bool CMeshShape::SetVertexPos(uint32 nVertex, const LTVector& vPos)
{
	if(nVertex < GetNumVertices())
	{
		m_pOriginalPos[nVertex] = vPos;
		return true;
	}
	return false;
}

//sets the normal of a specified vertex
bool CMeshShape::SetVertexNormal(uint32 nVertex, LTVector& vNormal)
{
	if(nVertex < GetNumVertices())
	{
		m_pNormals[nVertex] = vNormal;
		return true;
	}
	return false;
}

//sets the UV of a specified vertex
bool CMeshShape::SetVertexUV(uint32 nVertex, float fU, float fV)
{
	if(nVertex < GetNumVertices())
	{
		m_pRenderVerts[nVertex].tu = fU;
		m_pRenderVerts[nVertex].tv = fV;		
		return true;
	}
	return false;
}

//sets up a triangle based upon 3 vertex indices
bool CMeshShape::SetTriangleIndices(uint32 nTri, uint16 nVert1, uint16 nVert2, uint16 nVert3)
{
	if(nTri < GetNumTriangles())
	{
		uint16* pTriIndex = m_pTriangleList + nTri * 3;

		(*pTriIndex++) = nVert1;
		(*pTriIndex++) = nVert2;
		(*pTriIndex) = nVert3;

		return true;
	}
	return false;
}


//gets the list of vertices for rendering
TLVertex* CMeshShape::GetVertexList()
{
	return m_pRenderVerts;
}

//gets the list of the original vertex positions
LTVector* CMeshShape::GetOriginalPosList()
{
	return m_pOriginalPos;
}

//gets the list of triangles for rendering
uint16* CMeshShape::GetTriangleList()
{
	return m_pTriangleList;
}

//gets the normal list
LTVector* CMeshShape::GetNormalList()
{
	return m_pNormals;
}

//gets the filename of the texture for this shape
const char* CMeshShape::GetTextureFilename() const
{
	return m_pszTextureFilename;
}

//sets the filename of the texture for this shape
void CMeshShape::SetTextureFilename(const char* pszFilename)
{
	//free any old file
	delete [] m_pszTextureFilename;

	m_pszTextureFilename = new char[strlen(pszFilename) + 1];

	if(m_pszTextureFilename)
	{
		strcpy(m_pszTextureFilename, pszFilename);
	}
}

//gets the memory footprint for this shape
uint32 CMeshShape::GetMemoryFootprint() const
{
	uint32 nSize = 0;

	nSize += GetNumTriangles() * 3 * sizeof(uint16);
	nSize += GetNumVertices() * (sizeof(LTVector) + sizeof(TLVertex));

	return nSize;
}


void CMeshShape::FreeTriList()
{
	delete [] m_pTriangleList;
	m_pTriangleList = NULL;
	m_nNumTris		= 0;
}

void CMeshShape::FreeVertexList()
{
	delete [] m_pOriginalPos;
	m_pOriginalPos = NULL;

	delete [] m_pRenderVerts;
	m_pRenderVerts = NULL;

	delete [] m_pNormals;
	m_pNormals = NULL;

	m_nNumVerts = 0;
}


