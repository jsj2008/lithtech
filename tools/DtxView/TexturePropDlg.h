//------------------------------------------------------------------
//
//  FILE      : TexturePropDlg.h
//
//  PURPOSE   :	Texture properties dialog
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#ifndef __TEXTUREPROPDLG_H__
#define __TEXTUREPROPDLG_H__

#pragma once

#include "TextureProp.h"





class CTexturePropDlg : public CDialog
{
public:

	CTexturePropDlg(CWnd* pParent = NULL);
	~CTexturePropDlg();

	void			SetCheckBox(UINT id, BOOL bCheck);
	BOOL			GetCheckBox(UINT id);
	void			EnableCheckBox(UINT id, BOOL bEnable);

	void			SetupBPPIdent();
	void			RedrawPreviewWindow();

	void			SetChange(uint32 id, uint32 flag);

protected:

	virtual void 	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:

	// texture data and settings
	TextureProp		m_TextureProp;

	// These will be set in OnOk for whatever the user edited.
	DWORD			m_ChangeFlags;
	BOOL			m_bAllowChange;

	DWORD			m_PreviewThreadID;
	HANDLE			m_hPreviewThread;
	HWND			m_hPreviewWnd;
	HANDLE			m_heventPreviewSync;

	// Makes sure it doesn't recurse forever when changing texture formats.
	BOOL			m_bTFormatChanges;

	enum { IDD = IDD_DTXVIEW_DIALOG };
	CButton			m_Prefer5551;
	CButton			m_Prefer4444;
	CButton			m_Format_DXT5;
	CButton			m_DataFormat;
	CButton			m_Format_DXT3;
	CButton			m_Format_DXT1;
	CButton			m_Format_32Bit;
	CButton 		m_Format_32P;

protected:

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

	DECLARE_MESSAGE_MAP()
};


// Displays a message box with the app's name in it.
int		AppMessageBox( UINT idString, UINT nType );
int		AppMessageBox( const char *pStr, UINT nType );


#endif // __TEXTUREPROPDLG_H__
