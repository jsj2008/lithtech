//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

// This file handles all the undo/redo operations for DEdit.

#ifndef __UNDO_MGR_H__
#define __UNDO_MGR_H__

	#include "bdefs.h"
	#include "worldnode.h"


	class CRegionDoc;
	class CNodeReplacement;
	class CEditAction;
	class CPreAction;


	#define EditActionList CGLinkedList<CEditAction*>
	#define PreActionList CGLinkedList<CPreAction*>

	
	class CUndoMgr
	{
		public:
	
						CUndoMgr();
						~CUndoMgr();

			bool		Init(CRegionDoc *pDoc, uint32 nUndos, uint32 nRedos);
			void		Term();

			// Call before you do an action.. this will add an undo for the action.
			void		PreAction(CPreAction *pAction);

			// You can consider one undo/redo as a list of actions.  They will be 
			// executed in the reverse order that they're in the list.
			void		PreActions(PreActionList *pActions, bool bClearList=FALSE);
			
			void		Undo();
			void		Redo();

			EditActionList* SetupNewUndoSlot();

			void		SetupReversePreActions(PreActionList *pMain, EditActionList *pReverse);
			void		SetupReverseEditActions(EditActionList *pMain, EditActionList *pReverse);
			void		ExecuteActions(EditActionList *pList);

			void		SetUndoLimit(uint32 nLimit) { Term(); Init(m_pDoc, nLimit, nLimit); }

		public:

			CRegionDoc					*m_pDoc;
			
			CMoArray<EditActionList*>	m_Undos;
			uint32						m_nUndos;

			CMoArray<EditActionList*>	m_Redos;
			uint32						m_nRedos;

			bool						m_bCanAddActions;

			uint32						m_nWorldNodeCount; // number of World Nodes currently in the Undo buffer
			bool						m_bShowNodeCountWarning;  // do we want to show warning dialogs?

		private:

			void						DoNodeCountWarning();

			bool						m_bAlwaysClearUndo;		  // do we always clear the undo list when the limit is reached?
			uint32						m_nNextNodeCountWarning;  // how many nodes we need before the next warning
			bool						m_bShowingWarningDialog;  // necessary for odd thread sync issues
	};


#endif  // __UNDO_MGR_H__




