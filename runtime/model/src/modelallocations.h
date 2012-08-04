#ifndef __MODELALLOCATIONS_H__
#define __MODELALLOCATIONS_H__

#ifndef __MODEL_H__
#include "model.h"
#endif

// --------------------------------------------------------------------------
// ModelAllocations
// Utility class that counts the size of a model.
// (The engine needs to know how big a model is going to be.)
// --------------------------------------------------------------------------
class ModelAllocations
{
public:

				ModelAllocations();

	// Clear everything.
	void		Clear();
	
	// Load/save.
	bool		Load(ILTStream &str, uint32 FileVersion = 0);

	// Returns the number of bytes that need to be allocated for a single
	// block of memory for the model to load into.
	uint32		CalcAllocationSize();


public:

	// Note that the AnimInfos aren't handled in here because it can't pre-store
	// how many AnimInfos it will need to allocate.
	
	uint32		m_nKeyFrames;		// Number of keyframes.
	uint32		m_nParentAnims;		// Number of animations (that come from us).
	uint32		m_nNodes;			// Number of nodes.
	uint32		m_nPieces;			// Number of pieces.
	uint32		m_nChildModels;		// Number of child models (including the self child model).
	uint32		m_nTris;			// Number of triangles.
	uint32		m_nVerts;			// Number of vertices.
	uint32		m_nVertexWeights;	// Number of vertex weights.
	uint32		m_nLODs;			// Number of LODs.
	uint32		m_nSockets;			// Number of sockets.
	uint32		m_nWeightSets;		// Number of weight sets.

	uint32		m_nStrings;			// How many strings we're allocating.
	uint32		m_StringLengths;	// Sum of all string lengths (not including null terminator).
	uint32		m_VertAnimDataSize; // byte size of the vertex animation data.
	uint32		m_nAnimData;		// byte size of the animation data
};

#endif



