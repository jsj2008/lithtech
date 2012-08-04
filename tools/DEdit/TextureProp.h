#if !defined(AFX_TEXTUREPROP_H__FDB09C81_64C5_11D1_99E4_0060970987C3__INCLUDED_)
#define AFX_TEXTUREPROP_H__FDB09C81_64C5_11D1_99E4_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TextureProp.h : header file
//


#include "dtxmgr.h"


#define TPROP_FLAGS					(1<<0)
#define TPROP_GROUP					(1<<2)
#define TPROP_NUMMIPMAPS			(1<<3)
#define TPROP_FULLBRITES			(1<<4)
#define TPROP_PREFER4444			(1<<5)
#define TPROP_PREFER16BIT			(1<<6)
#define TPROP_COMMANDSTRING			(1<<7)
#define TPROP_DATAFORMAT			(1<<8)
#define TPROP_NONS3TCMIPMAPOFFSET	(1<<9)
#define TPROP_UIMIPMAPOFFSET		(1<<10)
#define TPROP_TEXTUREPRIORITY		(1<<11)
#define TPROP_DETAILTEXTURESCALE	(1<<12)
#define TPROP_DETAILTEXTUREANGLE	(1<<13)
#define TPROP_PREFER5551			(1<<14)
#define TPROP_32BITSYSCOPY			(1<<15)
#define TPROP_NOSYSCACHE			(1<<16)



/////////////////////////////////////////////////////////////////////////////
// CTextureProp dialog

class CTextureProp : public CDialog
{
// Construction
public:
	CTextureProp(CWnd* pParent = NULL);   // standard constructor
	~CTextureProp();

	void	SetCheckBox(UINT id, BOOL bCheck);
	BOOL	GetCheckBox(UINT id);
	void	EnableCheckBox(UINT id, BOOL bEnable);

	void	SetupBPPIdent();
	void	RedrawPreviewWindow();


	BOOL		m_bFullBrights;
	BOOL		m_b32BitSysCopy;
	BOOL		m_bPrefer4444;
	BOOL		m_bPrefer5551;
	BOOL		m_bPrefer16Bit;
	BOOL		m_bNoSysCache;
	DWORD		m_TextureFlags;
	DWORD		m_TextureGroup;
	DWORD		m_nMipmaps;
	DWORD		m_NonS3TCMipmapOffset;
	int			m_AlphaCutoff;
	int			m_AverageAlpha;
	int			m_UIMipmapOffset;
	char		m_CommandString[DTX_COMMANDSTRING_LEN];
	BPPIdent	m_BPPIdent;
	DWORD		m_TexturePriority;
	float		m_DetailTextureScale;
	int16		m_DetailTextureAngle;

	// These will be set in OnOk for whatever the user edited.
	DWORD	m_ChangeFlags;
	BOOL	m_bAllowChange;

	// Texture to use in the preview window.
	// If this is NULL, there won't be a preview window.
	// This texture is deleted automatically when the window is closed.
	TextureData	*m_pPreviewTexture;
	

public:

	DWORD		m_PreviewThreadID;
	HANDLE		m_hPreviewThread;
	HWND		m_hPreviewWnd;
	HANDLE		m_heventPreviewSync;
	
	// Makes sure it doesn't recurse forever when changing texture formats.
	BOOL		m_bTFormatChanges;
	


// Dialog Data
	//{{AFX_DATA(CTextureProp)
	enum { IDD = IDD_TEXTUREPROP };
	CButton	m_Prefer5551;
	CButton	m_Prefer4444;
	CButton	m_Format_DXT5;
	CButton	m_DataFormat;
	CButton	m_Format_DXT3;
	CButton	m_Format_DXT1;
	CButton	m_Format_32Bit;
	CButton m_Format_32P;
	//}}AFX_DATA


	void	SetChange(DWORD id, DWORD flag);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureProp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTextureProp)
	virtual BOOL OnInitDialog();
	afx_msg void OnTexturePropChange();
	afx_msg void OnGroupChange();
	afx_msg void OnNumMipmapsChange();
	afx_msg void OnOk();
	afx_msg void OnFullbrite();
	afx_msg void On32BitSysCopy();
	afx_msg void OnPrefer4444();
	afx_msg void OnPrefer5551();
	afx_msg void OnPrefer16Bit();
	afx_msg void OnNoSysCache();
	afx_msg void OnChangeDtxCommandString();
	afx_msg void OnFormat32bit();
	afx_msg void OnFormatDxt1();
	afx_msg void OnFormatDxt3();
	afx_msg void OnFormatDxt5();
	afx_msg void OnFormat32P();
	afx_msg void OnNonS3TCMipmapOffset();
	afx_msg void OnUIMipmapOffset();
	afx_msg void OnChangeTexturePriority();
	afx_msg void OnChangeDetailTextureScale();
	afx_msg void OnChangeDetailTextureAngle();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTUREPROP_H__FDB09C81_64C5_11D1_99E4_0060970987C3__INCLUDED_)
