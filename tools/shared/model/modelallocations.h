#ifndef __MODELALLOCATIONS_H__
#define __MODELALLOCATIONS_H__

class ModelAllocations;

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
		
		// Tally up everything in the model.
		bool		InitFromModel(Model *pModel);
		
		// Load/save.
		bool		Save(ILTStream &str);

		//Updates a field that had its position saved
		void		UpdateField(ILTStream& str, uint32 nFieldPos, uint32 nVal);

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
		uint32		m_nAnimData;		// Amount of animation data

		uint32		m_nStrings;			// How many strings we're allocating.
		uint32		m_StringLengths;	// Sum of all string lengths (not including null terminator).
		uint32		m_VertAnimDataSize; // byte size of the vertex animation data.


		//file positions for fields that need to be updated after a certain phase has passed
		uint32		m_nAnimDataPos;		// the position of the animation data size in the file
		
	private :
		uint32		m_FileVersion ; 		
		
	};


#endif



