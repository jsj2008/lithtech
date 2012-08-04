
#ifndef MATRIXCONFIGDLG_H
#define MATRIXCONFIGDLG_H

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file
#endif

#include "resource.h"
#include "renderstyle.h"
#include "RenderStylesEditor.h"
#include <string>

using namespace std;

struct MtxCfDlg_Data {
	float*				pMatrix;
	uint32*				TexCoordCount;
	bool*				ProjectUVEnable; 
};

class CMatrixConfigDlg : public CDialog
{
public:
    CMatrixConfigDlg() : CDialog(IDD_CONFIGMATRIX)					{ m_bReady = false; }

	// Set by the parent dialog (tell us how to set our init conditions and where changes go)...
	void SetMatrixData(MtxCfDlg_Data* pData)						{ m_pData = pData; }

private:
	// Dialog controls...
	CEdit*				m_pEB_MatrixParam[16];
	CButton*			m_pBn_ProjectEnable;
	CComboBox*			m_pCB_TexCoordCount;

	bool				m_bReady;									// Need this to ignore messages until we can set all of our data...
	MtxCfDlg_Data*		m_pData;									// Passed in data (we set need to set this structure with any changes so the main dlg gets our changes)...

	void				InitializeDialogControls();					// Grab the Dialog's controls...
	void				SetDialogControls_From_RenderStyleData();	// Setup the controls with the passed in data...

public:
	// CDialog Overrides...
	virtual BOOL		OnInitDialog();

	//{{AFX_MSG(CAppForm)
	afx_msg void OnMatrixParamChanged()								{ if (!m_bReady) return; for (uint32 i = 0; i < 16; ++i) { CString WindowText; m_pEB_MatrixParam[i]->GetWindowText(WindowText); m_pData->pMatrix[i] = (float)atof(LPCSTR(WindowText)); } g_AppFormView->RenderStyleDataChanged(); } 
	afx_msg void OnProjectEnable()									{ if (!m_bReady) return; *m_pData->ProjectUVEnable = m_pBn_ProjectEnable->GetCheck() ? true : false; g_AppFormView->RenderStyleDataChanged(); } 
	afx_msg void OnTexCoordCountChanged();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif


