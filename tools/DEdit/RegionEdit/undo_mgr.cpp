//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#include "bdefs.h"
#include "regiondoc.h"
#include "editpoly.h"
#include "editregion.h"
#include "edit_actions.h"
#include "undowarningdlg.h"
#include "dedit.h"
#include "optionsmisc.h"


extern BOOL g_bGlobalUndoDisable;


// Called when they do an action.  Setup an edit action which will undo the pre action.
typedef CEditAction* (*PreReverseFn)(CUndoMgr *pMgr, CPreAction *pPreAction);

// Called when they undo an action.  Setup a redo for it.
typedef CEditAction* (*EditReverseFn)(CUndoMgr *pMgr, CEditAction *pAction);

// Actually do an edit action.
typedef void (*ExecuteActionFn)(CUndoMgr *pMgr, CEditAction *pAction);



// ------------------------------------------------------------------ //
// Tables for each function type.
// ------------------------------------------------------------------ //

#define NUM_PREREVERSE_FNS() (sizeof(g_PreReverseFns) / sizeof(g_PreReverseFns[0]))
#define NUM_EDITREVERSE_FNS() (sizeof(g_EditReverseFns) / sizeof(g_EditReverseFns[0]))
#define NUM_EXECUTE_FNS() (sizeof(g_ExecuteFns) / sizeof(g_ExecuteFns[0]))

static struct {
	int m_Type;
	PreReverseFn m_Fn;
} g_PreReverseFns[] =
{
	ACTION_MODIFYNODE, ModifyNode_CreatePreReverse,
	ACTION_ADDEDNODE, AddedNode_CreatePreReverse,
};

static struct {
	int m_Type;
	EditReverseFn m_Fn;
} g_EditReverseFns[] =
{
	EDITACTION_REPLACENODE, ReplaceNode_CreateEditReverse,
};

static struct {
	int m_Type;
	ExecuteActionFn m_Fn;
} g_ExecuteFns[] =
{
	EDITACTION_REPLACENODE, ReplaceNode_Execute,
};


// ------------------------------------------------------------------ //
// Helper functions to find each function type.
// ------------------------------------------------------------------ //

static PreReverseFn FindPreReverseFn(int type)
{
	for(int i=0; i < NUM_PREREVERSE_FNS(); i++)
		if(g_PreReverseFns[i].m_Type == type)
			return g_PreReverseFns[i].m_Fn;

	ASSERT(false);
	return NULL;
}

static EditReverseFn FindEditReverseFn(int type)
{
	for(int i=0; i < NUM_EDITREVERSE_FNS(); i++)
		if(g_EditReverseFns[i].m_Type == type)
			return g_EditReverseFns[i].m_Fn;

	ASSERT(false);
	return NULL;
}

static ExecuteActionFn FindExecuteFn(int type)
{
	for(int i=0; i < NUM_EXECUTE_FNS(); i++)
		if(g_ExecuteFns[i].m_Type == type)
			return g_ExecuteFns[i].m_Fn;

	ASSERT(false);
	return NULL;
}




CUndoMgr::CUndoMgr()
{
	m_nUndos = 0;
	m_nRedos = 0;
	m_nWorldNodeCount = 0;
	m_nNextNodeCountWarning = 1024;
	m_bShowingWarningDialog = false;
	m_bShowNodeCountWarning = GetApp()->GetOptions().GetMiscOptions()->GetShowUndoWarnings();
	m_bAlwaysClearUndo		= false;
	m_bCanAddActions = true;
}


CUndoMgr::~CUndoMgr()
{
	Term();
}


bool CUndoMgr::Init(CRegionDoc *pDoc, uint32 nUndos, uint32 nRedos)
{
	uint32 i;

	m_nUndos = 0;
	m_Undos.SetSize(nUndos);
	for(i=0; i < nUndos; i++)
	{
		m_Undos[i] = new EditActionList;
	}

	m_nRedos = 0;
	m_Redos.SetSize(nRedos);
	for(i=0; i < nRedos; i++)
	{
		m_Redos[i] = new EditActionList;
	}

	m_nWorldNodeCount = 0;
	m_pDoc = pDoc;
	return true;
}


void CUndoMgr::Term()
{
	uint32 i;

	for(i=0; i < m_Undos; i++)
		GDeleteAndRemoveElements(*m_Undos[i]);

	for(i=0; i < m_Redos; i++)
		GDeleteAndRemoveElements(*m_Redos[i]);

	DeleteAndClearArray(m_Undos);
	DeleteAndClearArray(m_Redos);

	m_nWorldNodeCount = 0;
}


void CUndoMgr::PreAction(CPreAction *pAction)
{
	PreActionList theList;
	
	theList.AddHead(pAction);
	PreActions(&theList, false);
}


void CUndoMgr::PreActions(PreActionList *pList, bool bClearList)
{
	EditActionList *pUndo;

	// No undos if they REALLY don't want them.
	if(g_bGlobalUndoDisable)
		return;

	if(m_bCanAddActions)
	{
		m_nRedos = 0;

		// Setup a slot for a new undo action.
		pUndo = SetupNewUndoSlot();

		// Setup its reversal action.
		SetupReversePreActions(pList, pUndo);

		// Count number of World Nodes added
		m_nWorldNodeCount += pUndo->GetSize();

		if (m_nWorldNodeCount >= m_nNextNodeCountWarning)  DoNodeCountWarning();
	}

	if(bClearList)
		GDeleteAndRemoveElements(*pList);
}


void CUndoMgr::Undo()
{
	EditActionList *pRedo, *pUndo;


	// It does this stuff here because someone added bad code that caused it do
	// do an undo in SynchronizePropertyLists() when LoadDataIntoRegion was called! 
	m_bCanAddActions = false;

	// Add a redo.
	ASSERT(m_nUndos > 0);
	pUndo = m_Undos[m_nUndos - 1];
	m_nWorldNodeCount -= pUndo->GetSize(); // subtract the number of items that are being undone

	pRedo = m_Redos[m_nRedos];
	m_nRedos++;
		
	// Setup its reverse so we can redo it.
	SetupReverseEditActions(pUndo, pRedo);
		
	// Do the action!
	ExecuteActions(pUndo);
	m_nUndos--;

	m_bCanAddActions = true;
}


void CUndoMgr::Redo()
{
	EditActionList *pRedo, *pUndo;

	m_bCanAddActions = false;

	ASSERT(m_nRedos > 0);
	pRedo = m_Redos[m_nRedos-1];
	m_nRedos--;
	m_nWorldNodeCount += pRedo->GetSize(); // add the number of items that are being redone

		
	// Add its reverse so we can undo it.
	pUndo = SetupNewUndoSlot();
	SetupReverseEditActions(pRedo, pUndo);

	// Do the redo.
	ExecuteActions(pRedo);

	m_bCanAddActions = true;
}


EditActionList* CUndoMgr::SetupNewUndoSlot()
{
	EditActionList *pUndo;

	if(m_nUndos < m_Undos)
	{
		pUndo = m_Undos[m_nUndos];

		m_nUndos++;
	}
	else
	{
		pUndo = m_Undos[0];
		m_nWorldNodeCount -=  pUndo->GetSize(); // subtract the number of items that are being wiped out

		m_Undos.Remove(0);
		m_Undos.Append(pUndo);
	}

	GDeleteAndRemoveElements(*pUndo);
	return pUndo;
}


void CUndoMgr::SetupReversePreActions(PreActionList *pMain, EditActionList *pReverse)
{
	GPOS pos;
	CPreAction *pAction;
	CEditAction *pReverseAction;

	GDeleteAndRemoveElements(*pReverse);

	for(pos=pMain->GetHeadPosition(); pos; )
	{
		pAction = pMain->GetNext(pos);
		
		pReverseAction = FindPreReverseFn(pAction->m_ActionType)(this, pAction);
		if(pReverseAction)
		{
			pReverse->AddHead(pReverseAction);
		}
	}
}


void CUndoMgr::SetupReverseEditActions(EditActionList *pMain, EditActionList *pReverse)
{
	GPOS pos;
	CEditAction *pAction, *pReverseAction;

	GDeleteAndRemoveElements(*pReverse);

	for(pos=pMain->GetHeadPosition(); pos; )
	{
		pAction = pMain->GetNext(pos);
		
		pReverseAction = FindEditReverseFn(pAction->m_ActionType)(this, pAction);
		pReverse->AddHead(pReverseAction);
	}
}


void CUndoMgr::ExecuteActions(EditActionList *pList)
{
	GPOS pos;
	CEditAction *pAction;

	for(pos=pList->GetHeadPosition(); pos; )
	{
		pAction = pList->GetNext(pos);
	
		FindExecuteFn(pAction->m_ActionType)(this, pAction);
	}

	m_pDoc->NotifySelectionChange();
}


void CUndoMgr::DoNodeCountWarning()
{
	if (!m_bShowNodeCountWarning)
	{
		if (m_bAlwaysClearUndo) // remove first item from undo stack since we're over the size limit
		{
			while (m_nWorldNodeCount >= m_nNextNodeCountWarning) 
			{
				m_nWorldNodeCount -=  m_Undos[0]->GetSize(); // subtract the number of items that are being wiped out
				GDeleteAndRemoveElements(*m_Undos[0]);
				m_Undos.Remove(0);
				m_Undos.Append(new EditActionList);
				m_nUndos--;
			}
		}

		return;
	}


	if(!m_bShowingWarningDialog)
	{
		m_bShowingWarningDialog = true; // necessary for odd thread sync issues

		CUndoWarningDlg Dlg;
		Dlg.m_nWorldNodeCount = m_nWorldNodeCount;

		switch (Dlg.DoModal())
		{
		case IDC_UW_ALWAYS:
			m_bShowNodeCountWarning = false;
			m_bAlwaysClearUndo		= true;

		case IDC_UW_YES:
			while (m_nWorldNodeCount >= m_nNextNodeCountWarning) 
			{
				m_nWorldNodeCount -=  m_Undos[0]->GetSize(); // subtract the number of items that are being wiped out
				GDeleteAndRemoveElements(*m_Undos[0]);
				m_Undos.Remove(0);
				m_Undos.Append(new EditActionList);
				m_nUndos--;
			}
			break;

		case IDC_UW_NEVER:
			m_bShowNodeCountWarning = false;

		case IDC_UW_NO:
			m_nNextNodeCountWarning += m_nWorldNodeCount; // double the threshhold for the next warning
			break;
		}

		m_bShowingWarningDialog = false;
	}
}
		

