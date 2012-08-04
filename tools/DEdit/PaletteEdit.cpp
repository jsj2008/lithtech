//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// PaletteEdit.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "paletteedit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPaletteEdit dialog


CPaletteEdit::CPaletteEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CPaletteEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPaletteEdit)
	m_nPaletteIndex = 0;
	m_nPaletteNumberIndex = -1;
	m_sSelectedColor = _T("");
	//}}AFX_DATA_INIT
}


void CPaletteEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPaletteEdit)
	DDX_Text(pDX, IDC_PALETTE_INDEX, m_nPaletteIndex);
	DDX_CBIndex(pDX, IDC_PALETTE_PALNUM, m_nPaletteNumberIndex);
	DDX_Text(pDX, IDC_PALETTE_COLOR, m_sSelectedColor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPaletteEdit, CDialog)
	//{{AFX_MSG_MAP(CPaletteEdit)
	ON_BN_CLICKED(ID_PALETTE_ADD, OnPaletteAdd)
	ON_BN_CLICKED(ID_PALETTE_REMOVE, OnPaletteRemove)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPaletteEdit message handlers

void CPaletteEdit::OnPaletteAdd() 
{
	// TODO: Add your control notification handler code here
	
}

void CPaletteEdit::OnPaletteRemove() 
{
	// TODO: Add your control notification handler code here
	
}

BOOL CPaletteEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();
/*	
	DLink *pCur;
	PaletteDE *pPalette;
	CComboBox *pComboBox;
	CString sNum;
	int nComboIndex;

	pComboBox = ( CComboBox * )GetDlgItem( IDC_PALETTE_PALNUM );
	if( !pComboBox )
		return TRUE;

	for( pCur = g_Palettes.m_pNext; pCur != &g_Palettes; pCur = pCur->m_pNext )
	{	
		pPalette = ( PaletteDE * )pCur->m_pData;
		sNum.Format( "%5d", pPalette->m_Number );
		nComboIndex = pComboBox->AddString( sNum );
		if( nComboIndex != CB_ERR )
		{
			pComboBox->SetItemDataPtr( nComboIndex, ( void * )pPalette );
		}
	}

	pComboBox->SetCurSel( 0 );
*/
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

