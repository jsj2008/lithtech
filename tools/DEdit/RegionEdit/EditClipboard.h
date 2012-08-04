//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//



#ifndef __EDITCLIPBOARD_H__
#define __EDITCLIPBOARD_H__


// Includes....
#include "editpoly.h"
#include "editregion.h"
#include "worldnode.h"
#include "edit_actions.h"



class CEditClipboard
{
// Member functions...

public:

								CEditClipboard();
								~CEditClipboard();
	
	void						Term();

	// Copy the region's selected nodes into the clipboard
	void						Copy(CEditRegion *pRegion);

	// Paste the clipboards nodes into the world
	void						Paste( CEditRegion *pRegion, PreActionList &undoList, BOOL bPasteAtMarker);

	// This will paste an array of source nodes into the edit region
	void						PasteNodes(CEditRegion *pRegion, CMoArray<CWorldNode*> &sourceArray, PreActionList &undoList, BOOL bPasteAtMarker);

	BOOL						IsFull( ) { return( m_Nodes.GetSize( ) != 0 ); };
	
protected:									
	void						ClipboardDuplicate( CWorldNode *pSrc, CWorldNode *pDest, CEditRegion *pAddBrushesTo=NULL );
	void						CopyParenthood( CMoArray< CWorldNode * > &src, CMoArray< CWorldNode * > &dest );

	
// Member data...

protected:

	void						DeleteLocalNode( CWorldNode *pNode );
	CMoArray<CWorldNode*>		m_Nodes;		// Array of nodes in the clipboard
	CStringHolder				m_stringHolder;	// The string holder for the poly attributes
};



#endif  // __EDITCLIPBOARD_H__

