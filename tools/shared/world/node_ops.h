//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//


// The NodeOps modules provides all the functions that are used to create,
// modify, and delete world nodes.

#ifndef __NODEOPS_H__
#define __NODEOPS_H__

	
	
	// Attach the node to a parent node.
	void no_AttachNode(CEditRegion *pRegion, CWorldNode *pNode, CWorldNode *pParent=NULL);

	// Detach the node.
	void no_DetachNode(CEditRegion *pRegion, CWorldNode *pNode, BOOL bAttachToRoot);

	// Initializes a node you've allocated (adds it to the region's lists and
	// to the node tree).
	void no_InitializeNewNode(CEditRegion *pRegion, CWorldNode *pNode, CWorldNode *pParent);

	// Create a brush and parent it up to the given parent node (the
	// root node if no parent is specified).
	CEditBrush* no_CreateNewBrush(CEditRegion *pRegion, CWorldNode *pParent=NULL);

	// Removes all traces of the node's existance and burns its papers.
	// If bDeleteChildren is TRUE, then it calls no_DestroyNode for each child.
	// Otherwise, it attaches the children to the parent node.
	void no_DestroyNode(CEditRegion *pRegion, CWorldNode *pNode, BOOL bDeleteChildren);


	// Sets up brush properties
	void SetupBrushProperties(CEditBrush *pBrush, CEditRegion *pRegion);

#endif  // __NODEOPS_H__




