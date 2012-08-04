#if !defined(AFX_EXPORTOBJFILE_H__B9237E02_661E_4F7E_868E_D86806D9114D__INCLUDED_)
#define AFX_EXPORTOBJFILE_H__B9237E02_661E_4F7E_868E_D86806D9114D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportObjFile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportObjFile dialog

class CExportObjFile : public CDialog
{
// Construction
public:
	CExportObjFile(CWnd* pParent = NULL);   // standard constructor
	void SetCheckBox(UINT id, BOOL bCheck);
	BOOL m_isOK;

// Dialog Data
	//{{AFX_DATA(CExportObjFile)
	enum { IDD = IDD_EXPORT_OBJ_FILE_DLG };
	BOOL	m_exportHiddenBrushes;
	BOOL	m_exportFor3DSMax;
	BOOL	m_exportForMaya;
	BOOL	m_exportForSoft;
	BOOL	m_reverseNormals;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportObjFile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportObjFile)
	afx_msg void OnExportHiddenBrushesCheck();
	virtual void OnOK();
	afx_msg void OnExportObjFor3dsmaxCheck();
	afx_msg void OnExportObjForMayaCheck();
	afx_msg void OnExportObjForSoftCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTOBJFILE_H__B9237E02_661E_4F7E_868E_D86806D9114D__INCLUDED_)
