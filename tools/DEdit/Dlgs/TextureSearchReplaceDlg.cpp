#include "bdefs.h"
#include "texturesearchreplacedlg.h"
#include "resource.h"
#include "streamsim.h"
#include "dtxmgr.h"
#include "fileutils.h"
#include "edithelpers.h"
#include "projectbar.h"
#include "dirdialog.h"
#include <io.h>
#include <direct.h>

BEGIN_MESSAGE_MAP(CTextureSearchReplaceDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_DIR, OnButtonBrowse)
END_MESSAGE_MAP()


CTextureSearchReplaceDlg::CTextureSearchReplaceDlg() :
	CDialog(IDD_TEXTURE_SEARCH_REPLACE)
{
}

CTextureSearchReplaceDlg::~CTextureSearchReplaceDlg()
{
}

BOOL CTextureSearchReplaceDlg::OnInitDialog()
{
	if(!CDialog::OnInitDialog())
		return FALSE;

	return TRUE;
}

//This function handles the actual updating of the texture
static uint32 ReplaceInTexture(const char* pszFilename, const CString& sFind, const CString& sReplace, bool bFlags, bool bGroup, bool bCommand)
{
	//we first need to try and open up this texture
	ILTStream* pStream = streamsim_Open(pszFilename, "rb");
	if(!pStream)
	{
		return 0;
	}

	TextureData* pDTX;
	if(dtx_Create(pStream, &pDTX, TRUE, FALSE) != LT_OK)
	{
		pStream->Release();		
		return 0;
	}

	pStream->Release();		

	bool bSave = false;

	//alright, now handle replace
	if(bFlags)
	{
		uint32 nFind = atoi(sFind);

		if(nFind == pDTX->m_Header.m_UserFlags)
		{
			pDTX->m_Header.m_UserFlags = atoi(sReplace);
			bSave = true;
		}
	}

	if(bGroup)
	{
		uint32 nFind = atoi(sFind);

		if(nFind == pDTX->m_Header.GetTextureGroup())
		{
			pDTX->m_Header.m_Extra[0] = atoi(sReplace);
			bSave = true;
		}
	}

	if(bCommand)
	{
		int nPos = 0;
		CString sCommand = pDTX->m_Header.m_CommandString;

		CString sUpperCommand = sCommand;
		sUpperCommand.MakeUpper();

		while(1)
		{
			nPos = sUpperCommand.Find(sFind, nPos);

			if(nPos == -1)
			{
				//no match
				break;
			}

			//we have a match
			CString sRight = sCommand.Mid(nPos + sFind.GetLength());
			sCommand = sCommand.Left(nPos);
			sCommand += sReplace;
			sCommand += sRight;

			sUpperCommand = sCommand;
			sUpperCommand.MakeUpper();

			nPos += sReplace.GetLength();

			bSave = true;
		}

		//copy the string back, but make sure we don't go past the array and ruin the texture
		strncpy(pDTX->m_Header.m_CommandString, sCommand, DTX_COMMANDSTRING_LEN);

		//make sure it terminates properly
		pDTX->m_Header.m_CommandString[DTX_COMMANDSTRING_LEN - 1] = '\0';
	}

	if(bSave)
	{
		//we first need to try and open up this texture
		ILTStream* pOutStream = streamsim_Open(pszFilename, "wb");
		if(!pOutStream)
		{
			dtx_Destroy(pDTX);
			return 0;
		}

		dtx_Save(pDTX, pOutStream);
		pOutStream->Release();
	}

	dtx_Destroy(pDTX);

	//return 1 if we saved, otherwise 0
	if(bSave)
		return 1;

	return 0;
}


//This function will recurse through all subdirectories of the specified directory
//and perform the search and replace on textures
static uint32 FindTexturesToReplaceIn(const char* pszStartDir, const CString& sFind, const CString& sReplace, bool bFlags, bool bGroup, bool bCommand)
{
	uint32 nNumChanged = 0;

	//setup the root directory string
	CString sRootDir(pszStartDir);
	CFileUtils::EnsureValidFileName(sRootDir);
	sRootDir.TrimRight("\\");
	sRootDir += '\\';

	//see if this directory contains files we want to update
	bool bCheckFiles =	CFileUtils::DoesFileExist(sRootDir + "DirTypeTextures");

	CString sSearch = sRootDir + "*";

	//get the file list
	_finddata_t FileData;

	long SearchHandle = _findfirst(sSearch, &FileData);

	if(SearchHandle != -1)
	{
		do
		{
			//see if this file is a directory
			if(FileData.attrib & _A_SUBDIR)
			{
				//make sure that this isn't the . or .. directories
				if( (strcmp(FileData.name, ".") != 0) &&
					(strcmp(FileData.name, "..") != 0))
				{
					//we need to recurse
					nNumChanged += FindTexturesToReplaceIn(sRootDir + FileData.name, sFind, sReplace, bFlags, bGroup, bCommand);
				}
			}
			else if(bCheckFiles)
			{
				//see if this file is a level
				CString sFile(FileData.name);
				CString sExtension = CFileUtils::GetExtension(sFile);

				if(	(sExtension.CompareNoCase("dtx") == 0))
				{
					nNumChanged += ReplaceInTexture(sRootDir + sFile, sFind, sReplace, bFlags, bGroup, bCommand);
				}
			}
		}
		while(_findnext(SearchHandle, &FileData) != -1);
	}

	return nNumChanged;
}

void CTextureSearchReplaceDlg::OnOK()
{
	//determine the fields we need to edit
	bool bFlags = ((CButton*)GetDlgItem(IDC_CHECK_SEARCH_FLAGS))->GetCheck() != 0;
	bool bGroup = ((CButton*)GetDlgItem(IDC_CHECK_SEARCH_GROUP))->GetCheck() != 0;
	bool bCommand = ((CButton*)GetDlgItem(IDC_CHECK_SEARCH_COMMAND))->GetCheck() != 0;

	//get the search text
	CString sFind;
	CString sReplace;
	GetDlgItem(IDC_EDIT_FIND)->GetWindowText(sFind);
	GetDlgItem(IDC_EDIT_REPLACEWITH)->GetWindowText(sReplace);

	CString sStartDir;
	GetDlgItem(IDC_EDIT_START_DIR)->GetWindowText(sStartDir);

	CString sProjPath = GetProject()->m_BaseProjectDir ;

	//add the project path if applicable
	if(	(sStartDir.Left(sProjPath.GetLength()).CompareNoCase(sProjPath) != 0) &&
		(sStartDir.Find(':') == -1))
	{
		sStartDir.TrimLeft("\\/");
		sStartDir = sProjPath + '\\' + sStartDir;
	}

	//if one is selected, go through all the textures
	if((!bFlags && !bGroup && !bCommand) || sFind.IsEmpty())
	{
		MessageBox("Invalid Search Criteria Specified", "Error Performing Search", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	//we only do upper compares to remove case sensitivity
	sFind.MakeUpper();

	//handle the actual texture updating
	uint32 nNumReplaced = FindTexturesToReplaceIn(sStartDir, sFind, sReplace, bFlags, bGroup, bCommand);

	CString sFormat;
	sFormat.Format("Search and replace completed. %d textures were modified successfully", nNumReplaced);
	MessageBox(sFormat, "Complete", MB_OK);
	
	CDialog::OnOK();
}

void CTextureSearchReplaceDlg::OnCancel()
{
	//do nothing
	CDialog::OnCancel();
}

void CTextureSearchReplaceDlg::OnButtonBrowse()
{
	CDirDialog Dlg;
	Dlg.m_strInitDir	= GetProject()->m_BaseProjectDir;
	Dlg.m_strTitle		= "Select Starting Directory";
	Dlg.m_hwndOwner		= m_hWnd;

	if(Dlg.DoBrowse())
	{
		CString sPath = Dlg.m_strPath;
		CString sProjPath = GetProject()->m_BaseProjectDir ;

		//add the project path if applicable
		if(sPath.Left(sProjPath.GetLength()).CompareNoCase(sProjPath) == 0)
		{
			sPath = sPath.Mid(sProjPath.GetLength());
			sPath.TrimLeft("\\/");
		}

		GetDlgItem(IDC_EDIT_START_DIR)->SetWindowText(sPath);
	}
}