#ifndef PIECEMERGE_H
#define PIECEMERGE_H

#pragma once

#include <vector>
#include <list>


class CPieceMerge
{
public:
	CPieceMerge( Model* model, std::vector<ModelPiece*>& pieces );
	~CPieceMerge();

	// merge the pieces and return the number of resulting pieces
	uint32 Merge();

private:
	Model* m_Model;								// pointer to the model
	std::list<ModelPiece*> m_Pieces;			// list of pieces yet to be merged
	std::vector<ModelPiece*> m_OriginalPieces;	// original pieces in order

	// get a list of pieces that can be merged with the first piece in the list and remove them from the list of pieces waiting to be merged
	// returns false if no pieces were mergeable with the first piece in the list
	bool GetMergeablePieces( std::list<ModelPiece*>& pieces );

	// returns true if two model pieces are mergeable
	bool ArePiecesMergeable( ModelPiece* a, ModelPiece* b );

	// merge these pieces into one and delete the original pieces (must be mergeable pieces)
	bool MergePieces( std::list<ModelPiece*>& pieces );

	// fix vertex animation piece mappings that were trampled in the merge
	bool RemapVertexAnimations();
	void RemapVertexAnimationsRecurse( AnimNode* curNode, uint32 newOriginal, uint32 newIndex );
};


#endif // PIECEMERGE_H
