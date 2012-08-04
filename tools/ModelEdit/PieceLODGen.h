#ifndef PIECELODGEN_H
#define PIECELODGEN_H

#pragma once


// description of an edge collapse
struct EdgeCollapse
{
	unsigned m_Tris[2];		// triangles being collapsed
	unsigned m_Verts[2];	// edge being collapsed
};


// builds a new LOD given some parameters and a source piece LOD
class CBuildPieceLOD
{
public:
	// lod generation settings
	float m_Distance;				// distance at which this lod kicks in
	float m_Percent;				// percentage of tris this lod will ideally have
	float m_MaxEdgeLen;				// maximum length of an edge in the lod
	unsigned m_MinNumTris;			// minimum number of tris in the lod

	Model* m_Model;					// pointer to the model
	PieceLOD* m_PieceLOD;			// pointer to the source piece LOD
	ModelPiece* m_ModelPiece;		// pointer to the piece containing the source piece LOD
	int m_LODNum;					// which LOD the source piece LOD is within the piece

	// default constructor
	CBuildPieceLOD() : m_TriActive(20000), m_Tris(20000), m_Verts(10000) {}

	// generate the LOD from the source LOD
	bool BuildLOD();

private:
	CMoArray<bool> m_TriActive;			// is the specified triangle active
	CMoArray<ModelTri> m_Tris;			// new triangles
	CMoArray<ModelVert> m_Verts;		// new vertices
	CMoArray<EdgeCollapse> m_Collapses;	// array of edge collapses to get new LOD

	// initialize internal structures
	void SetupGenLODModel();

	// find the best edge collapse candidate
	bool FindShortestEdge( EdgeCollapse& edgeCollapse );

	// returns true if 2 and only 2 triangles share the edge
	// fills in tris with the tris that share the edge
	bool TestEdgeCollapse( unsigned vert0, unsigned vert1, unsigned* tris );

	// count the number of uncollapsed triangles
	unsigned CalcNumActiveTris();

	// updates structures with an edge collapse
	bool ProcessEdgeCollapse( const EdgeCollapse& edgeCollapse );

	// given a triangle and a vert index, get which vert on the tri has that index
	bool GetTriVert( unsigned tri, unsigned vertIndex, unsigned& vert );

	// merge two verts into a new vert
	void MergeVerts( const EdgeCollapse& edgeCollapse );

	// check to see if a vert already has a weight for a node
	NewVertexWeight* FindWeight( ModelVert* vert, unsigned node );

	// update any m_iReplacements referencing vertIndex to newVertIndex
	void MarkReplacements( unsigned vertIndex, unsigned newVertIndex );

	// any verts that reference only the removed triangles need to have any replacements
	// referencing them updated to reference the new vertex instead
	void UpdateDeadVertexReferences( const EdgeCollapse& edgeCollapse, unsigned newIndex );

	// add a new LOD to the model using the current set of edge collapses
	bool AddLOD();
};


// used for efficient initialization of an array of arrays
class ModelTriPtrArray
{
	enum { SIZE=20 };

public:
	ModelTriPtrArray() : m_MaskedData(SIZE) {}
	int GetSize() { return m_MaskedData.GetSize(); }
	void Append( ModelTri* type ) { m_MaskedData.Append( type ); }
	ModelTri* operator[]( int val ) { return m_MaskedData[val]; }
	
private:
	CMoArray<ModelTri*> m_MaskedData;
};


#endif // PIECELODGEN_H