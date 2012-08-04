//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : ProjectDirDlg.cpp
//
//	PURPOSE	  : 
//
//	CREATED	  : December 14 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "projectdirdlg.h"
#include "resource.h"

#include <io.h>



CProjectDirDlg::CProjectDirDlg( CWnd *pParent ) :
	CFileDialog(FALSE, NULL, NULL, OFN_HIDEREADONLY, " |.||", pParent)
{
}


int CProjectDirDlg::DoModal()
{
	CString		str;
	
	str.LoadString( IDS_PROJECTDIR_TITLE );
	m_ofn.lpstrTitle = str;

	return CFileDialog::DoModal();
}







