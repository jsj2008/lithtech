#ifndef __LEVELTEXTURESOPTIONSDLG_H__
#define __LEVELTEXTURESOPTIONSDLG_H__

class CLevelTexturesColumn
{
public:

	void	Init(const char* pszName, bool bDefaultEnabled, uint32 nDefaultWidth)
	{
		m_sName				= pszName;
		m_bEnabled			= bDefaultEnabled;
		m_bDefaultEnabled	= bDefaultEnabled;
		m_nDefaultWidth		= nDefaultWidth;
		m_nColID			= 0xFFFFFFFF;
	}

	CString		m_sName;
	uint32		m_nColID;
	uint32		m_nDefaultWidth;
	bool		m_bEnabled;
	bool		m_bDefaultEnabled;
};


class CLevelTexturesOptionsDlg : public CDialog
{
public:

	CLevelTexturesOptionsDlg(CLevelTexturesColumn* pColumns, uint32 nCount);
	~CLevelTexturesOptionsDlg();

	BOOL OnInitDialog();
	void OnOK();
	void OnCancel();

	afx_msg void	OnResetAll();

private:

	CLevelTexturesColumn*	m_pColumns;
	uint32					m_nNumColumns;

	//windows message map
	DECLARE_MESSAGE_MAP()
};

#endif