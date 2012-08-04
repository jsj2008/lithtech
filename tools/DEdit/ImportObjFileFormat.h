#if !defined(AFX_IMPORTOBJFILEFORMAT_H__F0BC2CDA_2A4F_4F91_8173_3565449BCD36__INCLUDED_)
#define AFX_IMPORTOBJFILEFORMAT_H__F0BC2CDA_2A4F_4F91_8173_3565449BCD36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportObjFileFormat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImportObjFileFormat dialog

class CImportObjFileFormat : public CDialog
{
// Construction
public:
	CImportObjFileFormat(CWnd* pParent = NULL);   // standard constructor
	void SetCheckBox(UINT id, BOOL bCheck);

// Dialog Data
	//{{AFX_DATA(CImportObjFileFormat)
	enum { IDD = IDD_IMPORT_OBJ_FILE_DIALOG };
	BOOL	m_reverse_normals;
	BOOL	m_from3DSMax;
	BOOL	m_fromMaya;
	BOOL	m_fromSoftImage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportObjFileFormat)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportObjFileFormat)
	virtual void OnOK();
	afx_msg void OnImportObjReverseCb();
	afx_msg void OnImportObjFrom3dsmaxCheck();
	afx_msg void OnImportObjFromMayaCheck();
	afx_msg void OnImportObjFromSoftimageCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTOBJFILEFORMAT_H__F0BC2CDA_2A4F_4F91_8173_3565449BCD36__INCLUDED_)
