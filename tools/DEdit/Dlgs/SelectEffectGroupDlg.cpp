#include "bdefs.h"
#include "resource.h"
#include "SelectEffectGroupDlg.h"
#include "EditEffectGroupDlg.h"

BEGIN_MESSAGE_MAP(CSelectEffectGroupDlg, CDialog)
	//{{AFX_MSG_MAP(CSpherePrimitiveDlg)
	ON_BN_CLICKED(ID_BUTTON_NEWEFFECTGROUP, OnButtonNew)
	ON_BN_CLICKED(ID_BUTTON_EDITEFFECTGROUP, OnButtonEdit)
	ON_CBN_SELCHANGE(IDC_COMBO_GROUPLIST, OnSelectionChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CSelectEffectGroupDlg::CSelectEffectGroupDlg(CWnd* pParent) : 
	CDialog(CSelectEffectGroupDlg::IDD, pParent)
{
}

CSelectEffectGroupDlg::~CSelectEffectGroupDlg()
{
}

//message callback
BOOL CSelectEffectGroupDlg::OnInitDialog()
{
	if(!CDialog::OnInitDialog())
		return FALSE;

	//we need to fill in the drop down box with all the groups
	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_COMBO_GROUPLIST);

	FillComboBox();

	//now select the string that they had previously
	int nMatch = pBox->FindStringExact(-1, m_sEffectGroup);

	if(nMatch != CB_ERR)
		pBox->SetCurSel(nMatch);

	UpdateEnabled();

	return TRUE;
}

//fills up the combo box
void CSelectEffectGroupDlg::FillComboBox()
{
	//we need to fill in the drop down box with all the groups
	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_COMBO_GROUPLIST);

	pBox->ResetContent();

	CString sDir = CEditEffectGroupDlg::GetEffectGroupDir() + "*.tfg";

	//add all the files into the list
	pBox->Dir(0, sDir);

	//add in a blank
	int nBlankStr = pBox->AddString("");

	//default the selection to the blank string
	pBox->SetCurSel(nBlankStr);
}

void CSelectEffectGroupDlg::OnOK()
{
	//we need to fill in the drop down box with all the groups
	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_COMBO_GROUPLIST);

	//clear out the string
	m_sEffectGroup = "";

	//get the string from the drop down
	int nSel = pBox->GetCurSel();

	if(nSel != CB_ERR)
		pBox->GetLBText(nSel, m_sEffectGroup);

	CDialog::OnOK();
}

void CSelectEffectGroupDlg::OnButtonNew()
{
	CEditEffectGroupDlg Dlg(this);
	if(Dlg.DoModal() == IDOK)
	{
		//update our list
		FillComboBox();
	}
}

void CSelectEffectGroupDlg::OnButtonEdit()
{
	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_COMBO_GROUPLIST);

	CEditEffectGroupDlg Dlg(this);

	pBox->GetLBText(pBox->GetCurSel(), Dlg.m_sLoadEffect);
	Dlg.DoModal();
}

void CSelectEffectGroupDlg::OnSelectionChanged()
{
	UpdateEnabled();
}

void CSelectEffectGroupDlg::UpdateEnabled()
{
	//get the current selection's text
	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_COMBO_GROUPLIST);
	CString sSel;
	pBox->GetLBText(pBox->GetCurSel(), sSel);

	GetDlgItem(ID_BUTTON_EDITEFFECTGROUP)->EnableWindow(!sSel.IsEmpty());
}

