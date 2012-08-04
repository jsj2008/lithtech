#include "bdefs.h"
#include "dedit.h"
#include "stdafx.h"
#include "ImportCubeMapDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP( CImportCubeMapDlg, CDialog )
	ON_BN_CLICKED( IDC_POSX_BUTTON, OnPosX )
	ON_BN_CLICKED( IDC_NEGX_BUTTON, OnNegX )
	ON_BN_CLICKED( IDC_POSY_BUTTON, OnPosY )
	ON_BN_CLICKED( IDC_NEGY_BUTTON, OnNegY )
	ON_BN_CLICKED( IDC_POSZ_BUTTON, OnPosZ )
	ON_BN_CLICKED( IDC_NEGZ_BUTTON, OnNegZ )
	ON_BN_CLICKED( IDC_BUTTON_LOAD_GEN_CUBE_MAP, OnImportGeneratedCubeMap)
END_MESSAGE_MAP()

CImportCubeMapDlg::CImportCubeMapDlg( CWnd* parent ) : CDialog( IDD_TEXTURE_CUBEMAP, parent )
{
}

CImportCubeMapDlg::~CImportCubeMapDlg()
{
}

BOOL CImportCubeMapDlg::OnInitDialog( void )
{
	m_InputNameEdit[ePosX] = (CEdit*)GetDlgItem( IDC_POSX );
	m_InputNameEdit[eNegX] = (CEdit*)GetDlgItem( IDC_NEGX );
	m_InputNameEdit[ePosY] = (CEdit*)GetDlgItem( IDC_POSY );
	m_InputNameEdit[eNegY] = (CEdit*)GetDlgItem( IDC_NEGY );
	m_InputNameEdit[ePosZ] = (CEdit*)GetDlgItem( IDC_POSZ );
	m_InputNameEdit[eNegZ] = (CEdit*)GetDlgItem( IDC_NEGZ );
	m_OutputNameEdit = (CEdit*)GetDlgItem( IDC_NAME );

	for( int i = 0; i < 6; i++ )
	{
		m_InputNameEdit[i]->SetWindowText( m_InputName[i] );
	}
	m_OutputNameEdit->SetWindowText( m_OutputName );

	return TRUE;
}

void CImportCubeMapDlg::OnOK( void )
{
	for( int i = 0; i < 6; i++ )
	{
		m_InputNameEdit[i]->GetWindowText( m_InputName[i] );
	}
	m_OutputNameEdit->GetWindowText( m_OutputName );

	CDialog::OnOK();
}

void CImportCubeMapDlg::ChooseFile( CubeMapFace face )
{
	CString fileExt( (LPCSTR)IDS_TGA_EXTENSION );
	CString fileMask( (LPCSTR)IDS_TGA_FILEMASK );

	CFileDialog fileDlg( TRUE, fileExt, m_InputName[face], 0, fileMask, this );
	if( fileDlg.DoModal() == IDOK )
	{
		m_InputName[face] = fileDlg.GetPathName();
		m_InputNameEdit[face]->SetWindowText( m_InputName[face] );
	}
}

void CImportCubeMapDlg::OnImportGeneratedCubeMap()
{
	//have them browse for a file
	CString fileExt( (LPCSTR)IDS_TGA_EXTENSION );
	CString fileMask( (LPCSTR)IDS_TGA_FILEMASK );

	CFileDialog fileDlg( TRUE, fileExt, NULL, 0, fileMask, this );
	if( fileDlg.DoModal() == IDOK )
	{
		//ok, so they found one, let us now grab the file name
		CString sFileName = fileDlg.GetPathName();

		//now we need to strip off the extension
		sFileName = sFileName.Left(sFileName.GetLength() -  fileDlg.GetFileExt().GetLength());

		//the list of available extensions (NOTE: These must match up with the ordering of the face
		//enumeration)
		char pszFaceNames[6][3] = {	"RT", "LF", "UP", "DN", "FR", "BK" };

		//ok, now see if the preceding characters are the type (FW/UP/DW, etc) and a period
		if(sFileName.GetLength() < 3)
			return;

		//strip off the period
		if(sFileName.Right(1) == '.')
			sFileName = sFileName.Left(sFileName.GetLength() - 1);

		//and now check for valid extension
		uint32 nCurrFace;

		bool bMatched = false;
		for(nCurrFace = 0; nCurrFace < 6; nCurrFace++)
		{
			if(sFileName.Right(2).CompareNoCase(pszFaceNames[nCurrFace]) == 0)
			{
				bMatched = true;
				break;
			}
		}

		//make sure we had a hit
		if(!bMatched)
			return;

		//ok, we have a valid name, strip off the extension
		sFileName = sFileName.Left(sFileName.GetLength() - 2);

		//now we can go about loading in each and every file
		for(nCurrFace = 0; nCurrFace < 6; nCurrFace++)
		{
			//build up the face filename
			CString sFaceFile = sFileName + pszFaceNames[nCurrFace];
			sFaceFile += ".tga";

			//make sure that we can open it
			CFileStatus DummyStatus;
			if(CFile::GetStatus(sFaceFile, DummyStatus))
			{
				//we could open it successfully, so make sure to setup the edit control to reflect this
				m_InputName[nCurrFace] = sFaceFile;
			}
			else
			{
				m_InputName[nCurrFace] = "";
			}
			m_InputNameEdit[nCurrFace]->SetWindowText( m_InputName[nCurrFace] );

		}

		//now see if they have a name for the item specified yet...
		CString sCurrOutName;
		m_OutputNameEdit->GetWindowText(sCurrOutName);

		if(sCurrOutName.IsEmpty())
		{
			//they haven't provided an output name, so lets provide one for them based upon the file name
			CString sOutName = sFileName;

			int nPos = sOutName.ReverseFind('\\');
			if(nPos != -1)
				sOutName = sOutName.Mid(nPos + 1);

			m_OutputName = sOutName;
			m_OutputNameEdit->SetWindowText(m_OutputName);
		}
	}
}
