#if !defined(AFX_SPELLBASEPARMDLG_H__28504DE4_724F_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_SPELLBASEPARMDLG_H__28504DE4_724F_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpellBaseParmDlg.h : header file
//

// Includes....

#include "Spell.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellBaseParmDlg dialog

class CSpellBaseParmDlg : public CDialog
{
// Construction
public:
	CSpellBaseParmDlg(CWnd* pParent = NULL);   // standard constructor

	public :

		void							SetSpell(CSpell *pSpell) { m_pSpell = pSpell; }

	private :

		// Member Variables

		CSpell							*m_pSpell;












// Dialog Data
	//{{AFX_DATA(CSpellBaseParmDlg)
	enum { IDD = IDD_SPELLBASEPARAMETERS };
	CEdit	m_cost;
	CEdit	m_desc;
	CComboBox	m_castHowOften;
	CComboBox	m_castPtType;
	CComboBox	m_targetType;
	CComboBox	m_type;
	CEdit	m_radius;
	CEdit	m_name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellBaseParmDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpellBaseParmDlg)
	afx_msg void OnAddTotem();
	afx_msg void OnRemoveTotem();
	afx_msg void OnSelChangeCastHowOften();
	afx_msg void OnSelChangeCastPtType();
	afx_msg void OnSelChangeTargetType();
	afx_msg void OnDestroy();
	afx_msg void OnChangeDescription();
	afx_msg void OnSelchangeAnimspeed();
	afx_msg void OnSelChangeSpellType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLBASEPARMDLG_H__28504DE4_724F_11D2_9B4A_0060971BDAD8__INCLUDED_)
