// ModifyDlgBase.h: interface for the CModifyDlgBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODIFYDLGBASE_H__F0643332_184F_11D3_BE29_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYDLGBASE_H__F0643332_184F_11D3_BE29_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CModifyDlgBase
{
public:
	CModifyDlgBase();
	virtual ~CModifyDlgBase();

	// Call this (before DoModal) to enable the ability to modify the name
	void	EnableModifyName(BOOL bEnable=TRUE)		{ m_bNameReadOnly=!bEnable; }
	BOOL	IsEnableModifyName()					{ return !m_bNameReadOnly; }

protected:
	// Call this (after DoModal) to update the states of the common controls
	void	UpdateCommonControlStates(CDialog *pDialog);
	
	// Call this to see if it is okay to close the dialog.  Note that some
	// messages boxes might popup.
	BOOL	IsOkayToClose(CDialog *pDialog, CString sName);

	/************************************************************************/
	// These are called by the IsOkayToClose(...) method
private:
	// Check to see if the key name contains valid characters
	BOOL	IsNameValidCharacters(CString sKey);

	// Call this to see if a key is already defined
	BOOL	IsKeyDefined(CString sKey);
	
	// Display the invalid characters name message
	void	DisplayInvalidNameCharactersMessage(CWnd *pParent);

	// Displays a message box indicating that the key is already defined
	void	DisplayDuplicateKeyMessage(CWnd *pParent);

	// Displays a message box indicating that a name must be entered
	void	DisplayNameEnterMessage(CWnd *pParent);

protected:
	BOOL	m_bNameReadOnly;	// This indicates if the name edit control should be read only

};

#endif // !defined(AFX_MODIFYDLGBASE_H__F0643332_184F_11D3_BE29_0060971BDC6D__INCLUDED_)
