#if !defined(AFX_SPELLRESOLUTION_H__28504DE5_724F_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_SPELLRESOLUTION_H__28504DE5_724F_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpellResolution.h : header file
//

// Includes....

#include "Spell.h"
//#include "AlterAttributesDlg.h"
//#include "AlterAttributes.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellResolution dialog

class CSpellResolutionDlg : public CDialog
{
// Construction
public:
	CSpellResolutionDlg(CWnd* pParent = NULL);   // standard constructor


	public :

		// Member Functions

		void					ActivateProperties(BOOL bActivate);
		void					ActivatePropertiesDlg(CString sEffect);
		void					MoveDialogs();

		void					SetSpell(CSpell *pSpell) { m_pSpell = pSpell; }

		// Accessors

	private :

		// Member Variables

		CSpell				   *m_pSpell;
//		CAlterAttributesDlg		m_alterAttributesDlg;












// Dialog Data
	//{{AFX_DATA(CSpellResolutionDlg)
	enum { IDD = IDD_SPELLRESOLUTION };
	CEdit	m_rate;
	CComboBox	m_type;
	CEdit	m_initialDelay;
	CComboBox	m_apply;
	CComboBox	m_activationPhase;
	CListBox	m_effects;
	CButton	m_remove;
	CButton	m_add;
	CListBox	m_allEffects;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellResolutionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpellResolutionDlg)
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnSelchangeEffects();
	afx_msg void OnKillfocusEffects();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeApply();
	afx_msg void OnSelchangeActivationphase();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLRESOLUTION_H__28504DE5_724F_11D2_9B4A_0060971BDAD8__INCLUDED_)
