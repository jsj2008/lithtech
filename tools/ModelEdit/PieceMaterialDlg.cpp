// PieceMaterialDlg.cpp : implementation file
//

#include "precompile.h"
#include "stdafx.h"
#include "modeledit.h"
#include "PieceMaterialDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPieceMaterialDlg dialog

CPieceMaterialDlg::CPieceMaterialDlg( const vector<PieceLODInfo>& selection, CWnd* pParent )
	: CDialog(CPieceMaterialDlg::IDD, pParent), m_Selection( selection ), m_LODDistChanged( false ), m_StartDist( -1.0f )
{
	//{{AFX_DATA_INIT(CPieceMaterialDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


// so texture indices can be set in a loop
int textureDlgItems[] =
{
	IDC_TEXTUREINDEX0,
	IDC_TEXTUREINDEX1,
	IDC_TEXTUREINDEX2,
	IDC_TEXTUREINDEX3
};

int textureSpinners[] =
{
	IDC_SPIN5,
	IDC_SPIN6,
	IDC_SPIN7,
	IDC_SPIN8
};


void CPieceMaterialDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPieceMaterialDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	// load the dialog data
	if (!pDX->m_bSaveAndValidate)
	{
		// get the first piece LOD info to compare others against
		float lodDistance = m_Selection[0].m_ModelPiece->GetLODDist( m_Selection[0].m_LODNum );
		int numTextures = m_Selection[0].m_PieceLOD->m_nNumTextures;
		int renderStyle = m_Selection[0].m_PieceLOD->m_iRenderStyle;
		int renderPriority = m_Selection[0].m_PieceLOD->m_nRenderPriority;
		int textureIndices[MAX_PIECE_TEXTURES];

		for( int i = 0; i < MAX_PIECE_TEXTURES; i++ )
		{
			textureIndices[i] = m_Selection[0].m_PieceLOD->m_iTextures[i];
		}

		// if any of the values don't match the first piece, flag them with a negative value
		for( i = 1; i < m_Selection.size(); i++ )
		{
			PieceLOD* curLOD = m_Selection[i].m_PieceLOD;
			ASSERT( curLOD );

			if( m_Selection[i].m_ModelPiece->GetLODDist( m_Selection[i].m_LODNum ) != lodDistance )
				lodDistance = -1.0f;
			if( curLOD->m_nNumTextures != numTextures )
				numTextures = -1;
			if( curLOD->m_iRenderStyle != renderStyle )
				renderStyle = -1;
			if( curLOD->m_nRenderPriority != renderPriority )
				renderPriority = -1;
			
			for( int j = 0; j < MAX_PIECE_TEXTURES; j++ )
			{
				if( curLOD->m_iTextures[j] != textureIndices[j] )
					textureIndices[j] = -1;
			}
		}
		
		CString sVal;

		sVal.Empty();
		if( lodDistance >= 0.0f )
		{
			sVal.Format("%0.2f", lodDistance );
			m_StartDist = (float)atof( sVal );
		}
		GetDlgItem(IDC_LODDISTANCE)->SetWindowText( sVal );

		((CComboBox*)GetDlgItem(IDC_NUMTEXTURES))->SetCurSel( numTextures );

		sVal.Empty();
		if( renderStyle >= 0 )
			sVal.Format("%d", renderStyle );
		GetDlgItem(IDC_RENDERSTYLEINDEX)->SetWindowText( sVal );

		sVal.Empty();
		if( renderPriority >= 0 )
			sVal.Format("%d", renderPriority );
		GetDlgItem(IDC_RENDERPRIORITY)->SetWindowText( sVal );

		for( i = 0; i < MAX_PIECE_TEXTURES; i++ )
		{
			sVal.Empty();
			if( textureIndices[i] >= 0 )
				sVal.Format("%d", textureIndices[i] );
			GetDlgItem( textureDlgItems[i] )->SetWindowText( sVal );
		}

		//read in the NULL checkbox state
		((CButton*)GetDlgItem(IDC_CHECK_NULL_LOD))->SetCheck(m_Selection[0].m_PieceLOD->NumVerts() ? 0 : 1);
	}
	// retrieve the dialog data
	else
	{
		float lodDistance;
		int numTextures;
		int renderStyle;
		int renderPriority;
		int textureIndices[MAX_PIECE_TEXTURES];

		CString str;

		// get values for any controls that aren't empty
		GetDlgItem(IDC_LODDISTANCE)->GetWindowText( str );
		if( str.IsEmpty() )
			lodDistance = -1.0f;
		else
		{
			lodDistance = (float)atof( str );
			if( lodDistance == m_StartDist )
				lodDistance = -1.0f;
		}

		numTextures = ((CComboBox*)GetDlgItem(IDC_NUMTEXTURES))->GetCurSel();
		if( numTextures == CB_ERR )
			numTextures = -1;

		GetDlgItem(IDC_RENDERSTYLEINDEX)->GetWindowText( str );
		if( str.IsEmpty() )
			renderStyle = -1;
		else
			renderStyle = atoi( str );

		GetDlgItem(IDC_RENDERPRIORITY)->GetWindowText( str );
		if( str.IsEmpty() )
			renderPriority = -1;
		else
			renderPriority = min(255, max(0, atoi( str )));

		for( int i = 0; i < MAX_PIECE_TEXTURES; i++ )
		{
			GetDlgItem( textureDlgItems[i] )->GetWindowText( str );
			if( str.IsEmpty() )
				textureIndices[i] = -1;
			else
				textureIndices[i] = atoi( str );
		}

		// update the piece LODs with the changed values
		for( i = 0; i < m_Selection.size(); i++ )
		{
			PieceLOD* curLOD = m_Selection[i].m_PieceLOD;
			ASSERT( curLOD );

			if( lodDistance >= 0.0f )
			{
				m_Selection[i].m_ModelPiece->SetLODDist( m_Selection[i].m_LODNum, lodDistance );
				m_LODDistChanged = true;
			}

			if( numTextures >= 0 )
				curLOD->m_nNumTextures = numTextures;

			if( renderStyle >= 0 )
				curLOD->m_iRenderStyle = renderStyle;

			if( renderPriority >= 0 )
				curLOD->m_nRenderPriority = renderPriority;

			for( int j = 0; j < MAX_PIECE_TEXTURES; j++ )
			{
				if( textureIndices[j] >= 0 )
					curLOD->m_iTextures[j] = textureIndices[j];
			}
		}
	}
}


BEGIN_MESSAGE_MAP(CPieceMaterialDlg, CDialog)
	//{{AFX_MSG_MAP(CPieceMaterialDlg)
	ON_CBN_SELCHANGE(IDC_NUMTEXTURES, OnSelNumTextures)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPieceMaterialDlg message handlers

void CPieceMaterialDlg::OnSelNumTextures()
{
	UpdateEnabledStatus();
}


BOOL CPieceMaterialDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CSpinButtonCtrl* pSpin;
	CComboBox* box = (CComboBox*)GetDlgItem(IDC_NUMTEXTURES);
	char curNum[10];
	
	for(int i=0; i <= MAX_PIECE_TEXTURES; i++)
	{
		sprintf( curNum, "%d", i );
		box->AddString( curNum );

	}
	
	for(i = 0; i < MAX_PIECE_TEXTURES; i++)
	{
		pSpin = (CSpinButtonCtrl*)GetDlgItem( textureSpinners[i] );
		pSpin->SetRange32( 0, 32 );
	}

	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN1);
	pSpin->SetRange32( 0, 9 );

	pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPINRENDERPRIORITY);
	pSpin->SetRange32( 0, 255 );

	UpdateData( FALSE );
	UpdateEnabledStatus();

	return TRUE;
}


void CPieceMaterialDlg::UpdateEnabledStatus()
{
	CComboBox* box = (CComboBox*)GetDlgItem(IDC_NUMTEXTURES);
	int numTextures = box->GetCurSel();

	if( numTextures == CB_ERR )
		numTextures = 0;

	// update the texture index controls
	for( int i = 0; i < MAX_PIECE_TEXTURES; i++ )
	{
		GetDlgItem( textureDlgItems[i] )->EnableWindow( numTextures > i );
	}
}
