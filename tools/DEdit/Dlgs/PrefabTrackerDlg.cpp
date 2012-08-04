// PrefabTrackerDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "prefabtrackerdlg.h"
#include "dirdialog.h"
#include "fileutils.h"
#include "ltamgr.h"
#include "editprojectmgr.h"

#if _MSC_VER >= 1300
#include <fstream>
#else
#include <fstream.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//given a full filename, it will return the file name without the project path
//with no leading \'s
static CString RemoveProjectPath(const CString& sFile)
{
	//get the project path
	CString sProjPath = GetProject()->m_BaseProjectDir;

	//make sure that the other file is longer than this
	if(sFile.GetLength() <= sProjPath.GetLength())
		return sFile;

	//see if we have a matching front end
	if(sFile.Left(sProjPath.GetLength()).CompareNoCase(sProjPath) == 0)
	{
		//it matches
		return sFile.Mid(sProjPath.GetLength() + 1);
	}

	return sFile;
}

/////////////////////////////////////////////////////////////////////////////
// CPrefabTrackerDlg dialog


CPrefabTrackerDlg::CPrefabTrackerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrefabTrackerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrefabTrackerDlg)
	//}}AFX_DATA_INIT
}


void CPrefabTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefabTrackerDlg)
	//}}AFX_DATA_MAP
}

BOOL CPrefabTrackerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;
}

void CPrefabTrackerDlg::ClearText()
{
	uint32 nTexLen = GetPrefabEdit()->GetTextLength();
	GetPrefabEdit()->SetSel(0, nTexLen);
	GetPrefabEdit()->ReplaceSel("");
}

void CPrefabTrackerDlg::AddString(const char* pszStr)
{
	uint32 nTexLen = GetPrefabEdit()->GetTextLength();
	GetPrefabEdit()->SetSel(nTexLen, nTexLen);
	GetPrefabEdit()->ReplaceSel(pszStr);
}


void CPrefabTrackerDlg::OnButtonGenerate()
{
	CWaitCursor WaitCursor;

	//first off clear out any existing window text, and any old data
	m_Prefabs.SetSize(0);
	ClearText();

	//alright, now we need to get the prefab directory and the world directory
	CDirDialog PrefabDir;
	PrefabDir.m_hwndOwner = m_hWnd;
	PrefabDir.m_strInitDir = GetProject()->m_BaseProjectDir;
	PrefabDir.m_strTitle = "Select Prefab Directory";
	if(!PrefabDir.DoBrowse())
		return;

	CDirDialog WorldDir;
	WorldDir.m_hwndOwner = m_hWnd;
	WorldDir.m_strInitDir = GetProject()->m_BaseProjectDir;
	WorldDir.m_strTitle = "Select World Directory";
	if(!WorldDir.DoBrowse())
		return;

	//alright, we now have both directories. Now build up a list of all the prefabs and worlds under
	//that directory
	CMoArray<CString>	sPrefabs;
	CFileUtils::GetAllWorldFiles(PrefabDir.m_strPath, sPrefabs);

	CMoArray<CString>	sWorlds;
	CFileUtils::GetAllWorldFiles(WorldDir.m_strPath, sWorlds);

	//alright, we now need to convert the prefab file list over to structures we can use
	m_Prefabs.SetSize(sPrefabs.GetSize());

	uint32 nCurrPrefab;
	for(nCurrPrefab = 0; nCurrPrefab < sPrefabs.GetSize(); nCurrPrefab++)
	{
		m_Prefabs[nCurrPrefab].m_sFilename		= RemoveProjectPath(sPrefabs[nCurrPrefab]);
		m_Prefabs[nCurrPrefab].m_nTotalRefCount = 0;

		m_Prefabs[nCurrPrefab].m_References.SetSize(0);
	}

	//alright, now we run through every level, open it, and look for prefabs
	for(uint32 nCurrWorld = 0; nCurrWorld < sWorlds.GetSize(); nCurrWorld++)
	{
		CString sWorld = sWorlds[nCurrWorld];
		CString sRelativeWorld = RemoveProjectPath(sWorld);

		//load up the LTA if it is a world
		CLTALoadOnlyAlloc Allocator(512 * 1024);

		//open up the file
		CLTAReader File;
		if(!File.Open(sWorld, CLTAUtil::IsFileCompressed(sWorld)))
		{
			continue;
		}

		//load up the world node
		CLTANode* pWorld = CLTANodeReader::LoadNode(&File, "world", &Allocator);

		//clean up the file
		File.Close();

		//if there is no world node, then this was not a world
		if(pWorld == NULL)
		{
			Allocator.FreeNode(pWorld);
			Allocator.FreeAllMemory();
			continue;
		}

		//now find any matching prefabs
		CLTANodeIterator Iter(pWorld);
		CLTANode* pPrefabName;

		while((pPrefabName = Iter.FindNextList("prefabfile")) != NULL)
		{
			//sanity checks
			ASSERT(pPrefabName->IsList());
			
			if(pPrefabName->GetNumElements() < 2)
				continue;

			CLTANode* pValue = pPrefabName->GetElement(1);

			if(pValue->IsList())
				continue;

			//valid, now see if we have a match
			for(uint32 nCurrPrefab = 0; nCurrPrefab < m_Prefabs.GetSize(); nCurrPrefab++)
			{
				if(m_Prefabs[nCurrPrefab].m_sFilename.CompareNoCase(pValue->GetValue()) == 0)
				{
					//we have a match, add this reference
					bool bFoundRef = false;
					for(uint32 nCurrRef = 0; nCurrRef < m_Prefabs[nCurrPrefab].m_References.GetSize(); nCurrRef++)
					{
						if(m_Prefabs[nCurrPrefab].m_References[nCurrRef].m_sFile.CompareNoCase(sRelativeWorld) == 0)
						{
							//found a reference
							m_Prefabs[nCurrPrefab].m_References[nCurrRef].m_nRefCount++;
							bFoundRef = true;
							break;
						}
					}

					//no luck finding an existing one, make a new one
					if(!bFoundRef)
					{
						SPrefabInfo::SPrefabRef NewRef;
						NewRef.m_sFile = sRelativeWorld;
						NewRef.m_nRefCount = 1;

						m_Prefabs[nCurrPrefab].m_References.Append(NewRef);
					}

					m_Prefabs[nCurrPrefab].m_nTotalRefCount++;
				}
			}		
		}

		//now clean up the world and the memory
		Allocator.FreeNode(pWorld);
		Allocator.FreeAllMemory();
	}

	FillEditText();
}

void CPrefabTrackerDlg::FillEditText()
{
	CWaitCursor WaitCursor;
	ClearText();

	//now we need to fill in the text into the window
	CString sFinalText;

	bool bShowReferences = IsShowReferences();

	for(uint32 nCurrPrefab = 0; nCurrPrefab < m_Prefabs.GetSize(); nCurrPrefab++)
	{
		CString sText;
		sText.Format("%s: %d\r\n", m_Prefabs[nCurrPrefab].m_sFilename, m_Prefabs[nCurrPrefab].m_nTotalRefCount);
		sFinalText += sText;

		if(bShowReferences)
		{
			for(uint32 nCurrRef = 0; nCurrRef < m_Prefabs[nCurrPrefab].m_References.GetSize(); nCurrRef++)
			{
				sText.Format("%s: %d\r\n", m_Prefabs[nCurrPrefab].m_References[nCurrRef].m_sFile, m_Prefabs[nCurrPrefab].m_References[nCurrRef].m_nRefCount);
				sFinalText += sText;
			}

			sFinalText += "\r\n";
		}
	}	
	AddString(sFinalText);
}



void CPrefabTrackerDlg::OnButtonSave()
{
	CFileDialog Dlg(FALSE, ".csv", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "CSV Files(*.csv)|*.csv|All Files(*.*)|*.*||");

	if(Dlg.DoModal() != IDOK)
		return;

#if _MSC_VER >= 1300
	std::ofstream OutFile(Dlg.GetPathName());
#else
	ofstream OutFile(Dlg.GetPathName());
#endif

	if(!OutFile.good())
	{
		CString sError;
		sError.Format("Error opening file %s for writing", Dlg.GetPathName());
		MessageBox(sError, "Error opening file", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	bool bShowReferences = IsShowReferences();

	//now we need to fill in the text into the window
	for(uint32 nCurrPrefab = 0; nCurrPrefab < m_Prefabs.GetSize(); nCurrPrefab++)
	{
		CString sText;

#if _MSC_VER >= 1300
		OutFile << m_Prefabs[nCurrPrefab].m_sFilename << ", " << m_Prefabs[nCurrPrefab].m_nTotalRefCount << std::endl;
#else
		OutFile << m_Prefabs[nCurrPrefab].m_sFilename << ", " << m_Prefabs[nCurrPrefab].m_nTotalRefCount << endl;
#endif
		
		if(bShowReferences)
		{
			for(uint32 nCurrRef = 0; nCurrRef < m_Prefabs[nCurrPrefab].m_References.GetSize(); nCurrRef++)
			{
#if _MSC_VER >= 1300
				OutFile << m_Prefabs[nCurrPrefab].m_References[nCurrRef].m_sFile << ", " << m_Prefabs[nCurrPrefab].m_References[nCurrRef].m_nRefCount << std::endl;
#else
				OutFile << m_Prefabs[nCurrPrefab].m_References[nCurrRef].m_sFile << ", " << m_Prefabs[nCurrPrefab].m_References[nCurrRef].m_nRefCount << endl;
#endif
			}

#if _MSC_VER >= 1300
			OutFile << std::endl;
#else
			OutFile << endl;
#endif
		}
	}	
}

void CPrefabTrackerDlg::OnCheckShowReferences()
{
	//we just need to refill the edit text
	FillEditText();
}


BEGIN_MESSAGE_MAP(CPrefabTrackerDlg, CDialog)
	//{{AFX_MSG_MAP(CPrefabTrackerDlg)
		ON_BN_CLICKED(ID_BUTTON_GENERATE, OnButtonGenerate)
		ON_BN_CLICKED(ID_BUTTON_SAVE, OnButtonSave)
		ON_BN_CLICKED(IDC_CHECK_SHOWFILEREFERENCES, OnCheckShowReferences)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

