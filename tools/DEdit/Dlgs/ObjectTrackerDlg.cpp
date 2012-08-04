// PrefabTrackerDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "objecttrackerdlg.h"
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

//allows the filtering out of certain classes
static bool ShouldTrackClass(const char* pszClass)
{
	if(stricmp(pszClass, "Brush") == 0)
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CObjectTrackerDlg dialog


CObjectTrackerDlg::CObjectTrackerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CObjectTrackerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CObjectTrackerDlg)
	//}}AFX_DATA_INIT
}


void CObjectTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectTrackerDlg)
	//}}AFX_DATA_MAP
}

BOOL CObjectTrackerDlg::OnInitDialog()
{
	//change our title text to say texture tracker
	SetWindowText("Object Tracker");

	CDialog::OnInitDialog();

	return TRUE;
}

void CObjectTrackerDlg::ClearText()
{
	uint32 nTexLen = GetObjectEdit()->GetTextLength();
	GetObjectEdit()->SetSel(0, nTexLen);
	GetObjectEdit()->ReplaceSel("");
}

void CObjectTrackerDlg::AddString(const char* pszStr)
{
	uint32 nTexLen = GetObjectEdit()->GetTextLength();
	GetObjectEdit()->SetSel(nTexLen, nTexLen);
	GetObjectEdit()->ReplaceSel(pszStr);
}

void CObjectTrackerDlg::AddWorldObjects(const char* pszWorld, uint32 nScale)
{
	CString sRelativeWorld = RemoveProjectPath(pszWorld);

	//load up the LTA if it is a world
	CLTALoadOnlyAlloc Allocator(512 * 1024);

	//open up the file
	CLTAReader File;
	if(!File.Open(pszWorld, CLTAUtil::IsFileCompressed(pszWorld)))
	{
		return;
	}

	//load up the world node
	CLTANode* pWorld = CLTANodeReader::LoadNode(&File, "nodehierarchy", &Allocator);

	//clean up the file
	File.Close();

	//if there is no world node, then this was not a world
	if(pWorld == NULL)
	{
		Allocator.FreeNode(pWorld);
		Allocator.FreeAllMemory();
		return;
	}

	//now find any matching objects
	CLTANodeIterator Iter(pWorld);
	CLTANode* pProperties;

	while((pProperties = Iter.FindNextList("properties")) != NULL)
	{
		//sanity checks
		if(!pProperties->IsList())
			continue;

		CLTANodeIterator NameIter(pProperties);

		CLTANode* pName = NameIter.FindNextList("name");

		//validate the find
		if(!pName)
			continue;

		if(!pName->IsList() || (pName->GetNumElements() < 2))
			continue;

		//now validate the item
		if(!pName->GetElement(1)->IsAtom())
			continue;

		//grab the name of the object
		const char* pszClassName = pName->GetElement(1)->GetValue();

		//see if this is a class we want to track
		if(ShouldTrackClass(pszClassName))
		{
			for(uint32 nCurrClass = 0; nCurrClass < m_Objects.GetSize(); nCurrClass++)
			{
				if(m_Objects[nCurrClass].m_sClass.CompareNoCase(pszClassName) == 0)
				{
					//we have a match, add this reference
					bool bFoundRef = false;
					for(uint32 nCurrRef = 0; nCurrRef < m_Objects[nCurrClass].m_References.GetSize(); nCurrRef++)
					{
						if(m_Objects[nCurrClass].m_References[nCurrRef].m_sFile.CompareNoCase(sRelativeWorld) == 0)
						{
							//found a reference
							m_Objects[nCurrClass].m_References[nCurrRef].m_nRefCount += nScale;
							bFoundRef = true;
							break;
						}
					}

					//no luck finding an existing one, make a new one
					if(!bFoundRef)
					{
						SObjectInfo::SObjectRef NewRef;
						NewRef.m_sFile = sRelativeWorld;
						NewRef.m_nRefCount = nScale;

						m_Objects[nCurrClass].m_References.Append(NewRef);
					}

					m_Objects[nCurrClass].m_nTotalRefCount += nScale;
				}
			}
		}
	}

	//now clean up the world and the memory
	Allocator.FreeNode(pWorld);
	Allocator.FreeAllMemory();
}


void CObjectTrackerDlg::OnButtonGenerate()
{
	CWaitCursor WaitCursor;

	//first off clear out any existing window text, and any old data
	m_Objects.SetSize(0);

	ClearText();

	CDirDialog WorldDir;
	WorldDir.m_hwndOwner = m_hWnd;
	WorldDir.m_strInitDir = GetProject()->m_BaseProjectDir;
	WorldDir.m_strTitle = "Select World Directory";
	if(!WorldDir.DoBrowse())
		return;

	//our list of prefabs that the worlds reference
	CMoArray<SPrefabInfo>	PrefabList;

	CMoArray<CString>	sWorlds;
	CFileUtils::GetAllWorldFiles(WorldDir.m_strPath, sWorlds);

	//alright, we now need to convert the prefab file list over to structures we can use
	m_Objects.SetSize(GetProject()->m_nClassDefs);

	uint32 nCurrClass;
	for(nCurrClass = 0; nCurrClass < m_Objects.GetSize(); nCurrClass++)
	{
		m_Objects[nCurrClass].m_sClass			= GetProject()->m_ClassDefs[nCurrClass].m_ClassName;
		m_Objects[nCurrClass].m_nTotalRefCount  = 0;

		m_Objects[nCurrClass].m_References.SetSize(0);
	}

	//alright, now we run through every level, open it, and look for prefabs
	for(uint32 nCurrWorld = 0; nCurrWorld < sWorlds.GetSize(); nCurrWorld++)
	{
		CString sWorld = sWorlds[nCurrWorld];

		//add all the world objects
		AddWorldObjects(sWorld, 1);

		//load up the LTA if it is a world
		CLTALoadOnlyAlloc Allocator(512 * 1024);

		//open up the file
		CLTAReader File;

		//and now handle building up the prefabs
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

			//queue this prefab in our lists
			bool bFound = false;
			for(uint32 nCurrPrefab = 0; nCurrPrefab < PrefabList.GetSize(); nCurrPrefab++)
			{
				if(PrefabList[nCurrPrefab].m_sPrefab.CompareNoCase(pValue->GetValue()) == 0)
				{
					PrefabList[nCurrPrefab].m_nNumRefs++;
					bFound = true;
					break;
				}
			}		

			if(!bFound)
			{
				SPrefabInfo NewInfo;
				NewInfo.m_sPrefab = pValue->GetValue();
				NewInfo.m_nNumRefs = 1;
				PrefabList.Add(NewInfo);
			}
		}

		//now clean up the world and the memory
		Allocator.FreeNode(pWorld);
		Allocator.FreeAllMemory();

	}

	//now run through our list of prefabs and add their object counts
	for(uint32 nCurrPrefab = 0; nCurrPrefab < PrefabList.GetSize(); nCurrPrefab++)
	{
		CString sPrefab = GetProject()->m_BaseProjectDir + '\\' + PrefabList[nCurrPrefab].m_sPrefab;
		AddWorldObjects(sPrefab, PrefabList[nCurrPrefab].m_nNumRefs);
	}

	FillEditText();
}

void CObjectTrackerDlg::FillEditText()
{
	CWaitCursor WaitCursor;
	ClearText();

	//now we need to fill in the text into the window
	CString sFinalText;

	bool bShowReferences = IsShowReferences();

	for(uint32 nCurrClass = 0; nCurrClass < m_Objects.GetSize(); nCurrClass++)
	{
		CString sText;
		sText.Format("%s: %d\r\n", m_Objects[nCurrClass].m_sClass, m_Objects[nCurrClass].m_nTotalRefCount);
		sFinalText += sText;

		if(bShowReferences)
		{
			for(uint32 nCurrRef = 0; nCurrRef < m_Objects[nCurrClass].m_References.GetSize(); nCurrRef++)
			{
				sText.Format("%s: %d\r\n", m_Objects[nCurrClass].m_References[nCurrRef].m_sFile, m_Objects[nCurrClass].m_References[nCurrRef].m_nRefCount);
				sFinalText += sText;
			}

			sFinalText += "\r\n";
		}
	}	
	AddString(sFinalText);
}



void CObjectTrackerDlg::OnButtonSave()
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
	for(uint32 nCurrClass = 0; nCurrClass < m_Objects.GetSize(); nCurrClass++)
	{
		CString sText;

#if _MSC_VER >= 1300
		OutFile << m_Objects[nCurrClass].m_sClass << ", " << m_Objects[nCurrClass].m_nTotalRefCount << std::endl;
#else
		OutFile << m_Objects[nCurrClass].m_sClass << ", " << m_Objects[nCurrClass].m_nTotalRefCount << endl;
#endif
		
		if(bShowReferences)
		{
			for(uint32 nCurrRef = 0; nCurrRef < m_Objects[nCurrClass].m_References.GetSize(); nCurrRef++)
			{
#if _MSC_VER >= 1300
				OutFile << m_Objects[nCurrClass].m_References[nCurrRef].m_sFile << ", " << m_Objects[nCurrClass].m_References[nCurrRef].m_nRefCount << std::endl;
#else
				OutFile << m_Objects[nCurrClass].m_References[nCurrRef].m_sFile << ", " << m_Objects[nCurrClass].m_References[nCurrRef].m_nRefCount << endl;
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

void CObjectTrackerDlg::OnCheckShowReferences()
{
	//we just need to refill the edit text
	FillEditText();
}


BEGIN_MESSAGE_MAP(CObjectTrackerDlg, CDialog)
	//{{AFX_MSG_MAP(CObjectTrackerDlg)
		ON_BN_CLICKED(ID_BUTTON_GENERATE, OnButtonGenerate)
		ON_BN_CLICKED(ID_BUTTON_SAVE, OnButtonSave)
		ON_BN_CLICKED(IDC_CHECK_SHOWFILEREFERENCES, OnCheckShowReferences)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

