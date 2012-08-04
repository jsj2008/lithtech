#ifndef EXPORTD3D_DLG
#define EXPORTD3D_DLG

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ExportD3D_Dlg.h : header file

/////////////////////////////////////////////////////////////////////////////
// ExportD3D_Dlg dialog
class CExportD3D_Dlg : public CFileDialog
{
	DECLARE_DYNAMIC(CExportD3D_Dlg)

public:

	enum EAnimCompressionType { NONE = 0 , RELEVANT = 1, RELEVANT_16bit = 2 , REL_PV16 = 3};

	CExportD3D_Dlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

	bool	m_bDoneWithUpdate;
	CString	m_szFileName;
	bool	m_bUseMatrixPalettes;
	bool	m_bMaxBonesPerVert;
	bool	m_bMaxBonesPerTri;
	bool	m_bExcludeGeometry;
	bool	m_bPauseAfterCompile;
	uint32	m_iMaxBonesPerVert;
	uint32	m_iMaxBonesPerTri;
	float	m_fMinBoneWeight;
	EAnimCompressionType m_CompressionType;

	bool	m_bStreamOptionsPosition;
	bool	m_bStreamOptionsNormal;
	bool	m_bStreamOptionsColor;
	bool	m_bStreamOptionsTangent;
	bool	m_bStreamOptionsUV1;
	bool	m_bStreamOptionsUV2;
	bool	m_bStreamOptionsUV3;
	bool	m_bStreamOptionsUV4;
	
	virtual void OnInitDone( );	

protected:
	//{{AFX_MSG(ExportD3D_Dlg)
	afx_msg void OnDataChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
