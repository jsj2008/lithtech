#ifndef IMPORTLODDLG_H
#define IMPORTLODDLG_H

#pragma once

#include "ModelEditDlg.h"


// dialog used to import custom piece LODs
class CImportLODDlg : public CDialog
{
public:
	CImportLODDlg( CWnd* parent = NULL );

	// template for this dialog
	enum { IDD = IDD_IMPORT_LODS };

	// dialog data set by the caller
	Model* m_ImportModel;					// the model that LODs are being imported from
	Model* m_CurrentModel;					// the model currently being edited in ModelEdit
	CModelEditDlg* m_ModelEditDlg;			// the main ModelEdit dialog

	// dialog data passed back to the caller
	std::vector<PieceLODInfo> m_Selection;	// the selected LODs

protected:
	CLTWinTreeMgr m_PieceList;

	virtual void DoDataExchange( CDataExchange* pDX );
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();

	// context menu stuff for the piece list
	void OnContextMenu( CWnd* pWnd, CPoint point );
	void OnPieceExpandAll();
	void OnPieceCollapseAll();
};


#endif // IMPORTLODDLG_H