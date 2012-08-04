//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#ifndef __EDIT_ACTIONS_H__
#define __EDIT_ACTIONS_H__

	
	#include "worldnode.h"
	#include "editaction_ids.h"
	#include "undo_mgr.h"



	// ------------------------------------------------------------------ //
	// Structures used by the actions.
	// ------------------------------------------------------------------ //

	class CEditAction : public CGLLNode
	{
		public:

						CEditAction() {}
			virtual		~CEditAction() {}
	

			int					m_ActionType;

	};


	class CPreAction : public CGLLNode
	{
		public:
				
						CPreAction(int type, CWorldNode *pNode)
						{
							m_ActionType = type;
							m_pNode = pNode;
						}

			int			m_ActionType;
			CWorldNode	*m_pNode;

	};


	// ------------------------------------------------------------------ //
	// Helpers..
	// ------------------------------------------------------------------ //

	void AddToActionListIfNew(PreActionList *pList, CPreAction *pAction, BOOL bDeleteIfNot=FALSE);


	// ------------------------------------------------------------------ //
	// Action functions.
	// ------------------------------------------------------------------ //

	CEditAction* ModifyNode_CreatePreReverse(CUndoMgr *pMgr, CPreAction *pPreAction);
	CEditAction* ReplaceNode_CreateEditReverse(CUndoMgr *pMgr, CEditAction *pAction);
	void ReplaceNode_Execute(CUndoMgr *pMgr, CEditAction *pAction);
		
	CEditAction* AddedNode_CreatePreReverse(CUndoMgr *pMgr, CPreAction *pPreAction);


#endif  // __EDIT_ACTIONS_H__


