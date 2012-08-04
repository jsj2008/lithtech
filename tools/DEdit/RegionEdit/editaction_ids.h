//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#ifndef __EDITACTION_IDS_H__
#define __EDITACTION_IDS_H__


	// ------------------------------------------------------------------ //
	// Definitions.
	// ------------------------------------------------------------------ //

	// Action types (other than ACTION_MODIFYALL).
	#define ACTION_MODIFYNODE	2
	#define ACTION_ADDEDNODE	3	// Used after a new node has been added.. the undo removes the node.


	// Edit actions.
	#define EDITACTION_REPLACEALL	50
	
	// A ReplaceNode action can act as 3 different things: remove a node,
	// replace a node, or add a node.
	#define EDITACTION_REPLACENODE	51


#endif

