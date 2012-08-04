#ifndef __TEXTURESEARCHREPLACEDLG_H__
#define __TEXTURESEARCHREPLACEDLG_H__

class CTextureSearchReplaceDlg : public CDialog
{
public:

	CTextureSearchReplaceDlg();
	~CTextureSearchReplaceDlg();

	BOOL OnInitDialog();
	void OnOK();
	void OnCancel();
	afx_msg void OnButtonBrowse();

private:

	//windows message map
	DECLARE_MESSAGE_MAP()
};

#endif