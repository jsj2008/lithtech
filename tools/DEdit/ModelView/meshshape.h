#ifndef __MESHSHAPE_H__
#define __MESHSHAPE_H__

class TLVertex;

#include <ltbasedefs.h>

class CMeshShape
{
public:

	CMeshShape();
	~CMeshShape();

	//sets the number of vertices for this mesh
	bool			SetNumVertices(uint32 nNumVerts);

	//gets the number of vertices for this mesh
	uint32			GetNumVertices() const;

	//sets the number of triangles in the mesh
	bool			SetNumTriangles(uint32 nNumTris);

	//gets the number of triangles in the mesh
	uint32			GetNumTriangles() const;

	//sets the position of a specified vertex
	bool			SetVertexPos(uint32 nVertex, const LTVector& vPos);

	//sets the UV of a specified vertex
	bool			SetVertexUV(uint32 nVertex, float fU, float fV);

	//sets the normal of a specified vertex
	bool			SetVertexNormal(uint32 nVertex, LTVector& vNormal);

	//sets up a triangle based upon 3 vertex indices
	bool			SetTriangleIndices(	uint32 nTri, uint16 nVert1, 
										uint16 nVert2, uint16 nVert3);

	//gets the bounding box for the shape. Returns false if there
	//are no points, and the dims should be ignored
	bool			GetBoundingBox(LTVector& vMin, LTVector& vMax);

	//gets the list of vertices for rendering
	TLVertex*		GetVertexList();

	//gets the list of the original vertex positions
	LTVector*		GetOriginalPosList();

	//gets the list of the normals
	LTVector*		GetNormalList();

	//gets the list of triangles for rendering
	uint16*			GetTriangleList();

	//gets the memory footprint for this shape
	uint32			GetMemoryFootprint() const;

	//gets the filename of the texture for this shape
	const char*		GetTextureFilename() const;

	//sets the filename of the texture for this shape
	void			SetTextureFilename(const char* pszFilename);

	//the internal file representation
	struct DFileIdent_t*	m_pTextureFile;
	
private:

	void			FreeTriList();
	void			FreeVertexList();

	//the filename for this shape's texture
	char*			m_pszTextureFilename;


	//the count of the number of vertices
	uint32			m_nNumVerts;

	//the count of the number of tris
	uint32			m_nNumTris;

	//the list of the origninal vector positions
	LTVector*		m_pOriginalPos;

	//the list of vertices used for the actual rendering
	TLVertex*		m_pRenderVerts;

	//the list of normals for each vertex
	LTVector*		m_pNormals;

	//the indexed face list
	uint16*			m_pTriangleList;


};

#endif

