// ButeEditDoc.cpp : implementation of the CButeEditDoc class
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "tdguard.h"

#include "ButeEditDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CButeEditDoc

IMPLEMENT_DYNCREATE(CButeEditDoc, CDocument)

BEGIN_MESSAGE_MAP(CButeEditDoc, CDocument)
	//{{AFX_MSG_MAP(CButeEditDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CButeEditDoc construction/destruction

CButeEditDoc::CButeEditDoc()
{
	m_pButeMgr=NULL;
	m_sSelectedTag="";
}

CButeEditDoc::~CButeEditDoc()
{
	if (m_pButeMgr)
	{
		m_pButeMgr->Term();
		delete m_pButeMgr;
		m_pButeMgr=NULL;
	}
}

BOOL CButeEditDoc::OnNewDocument()
{

	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}

	if (!CDocument::OnNewDocument())
		return FALSE;

	// Term and delete the current bute mgr
	if (m_pButeMgr)
	{
		m_pButeMgr->Term();
		delete m_pButeMgr;
		m_pButeMgr=NULL;
	}

	// Create the new Bute Manager
	m_pButeMgr=new CButeMgr;
	m_pButeMgr->Init();

	// Remove the current array of tags
	m_tagArray.RemoveAll();

	// Clear the currently selected tag
	m_sSelectedTag="";

	// Clear the modified flag
	SetModifiedFlag(FALSE);

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CButeEditDoc serialization

void CButeEditDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{	
	}
	else
	{		
	}
}

/************************************************************************/
// The callback function to retrieve the tags in the bute file
bool CButeEditDoc::GetTagsCallback(const char* szTag, void* pAux)
{
	// Get the string array
	CStringArray *pStringArray=(CStringArray *)pAux;
	ASSERT(pStringArray);

	// Add the string
	if (pStringArray)
	{
		pStringArray->Add(szTag);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CButeEditDoc diagnostics

#ifdef _DEBUG
void CButeEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CButeEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CButeEditDoc commands

BOOL CButeEditDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	ASSERT(m_pButeMgr);
	if (m_pButeMgr)
	{
		// Save
		if (m_pButeMgr->Save(lpszPathName))
		{
			// Clear the modified flag
			SetModifiedFlag(FALSE);

			// Reparse the file
			if (!m_pButeMgr->Parse(lpszPathName))
			{
				// Display the error string
				MessageBox(AfxGetMainWnd()->GetSafeHwnd(), "Parsing error: "+m_pButeMgr->GetErrorString(), "Parsing Error", MB_OK | MB_ICONERROR);
				return FALSE;
			}

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

BOOL CButeEditDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!m_pButeMgr)
	{
		m_pButeMgr=new CButeMgr;
	}
	
	// Parse the bute file
	if (!m_pButeMgr->Parse(lpszPathName))
	{
		// Display the error string
		MessageBox(AfxGetMainWnd()->GetSafeHwnd(), "Parsing error: "+m_pButeMgr->GetErrorString(), "Parsing Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	else
	{
		// Remove the tags
		m_tagArray.RemoveAll();

		// Clear the selected tag
		m_sSelectedTag="";

		// Get the tags
		m_pButeMgr->GetTags(GetTagsCallback, (void *)&m_tagArray);

		// Clear the modified flag
		SetModifiedFlag(FALSE);

		return TRUE;
	}	
}
	
/************************************************************************/
// Call this function to modify a specific key/value in every tag
void CButeEditDoc::ReplaceAllKeyValues(CString sKey, CButeMgr::SymTypes type, void *pValueData)
{
	int i;
	for (i=0; i < GetNumTags(); i++)
	{
		// Get the tag
		CString sTag=GetTag(i);

		// Check to see if the key exists
		if (!m_pButeMgr->Exist(sTag, sKey))
		{
			continue;
		}

		// Check to see if the key is the correct type
		if (m_pButeMgr->GetType(sTag, sKey) == type)
		{
			// Set the new value
			switch (type)
			{
			case CButeMgr::IntType:			
				{
					m_pButeMgr->SetInt(sTag, sKey, *(int *)pValueData);
					break;
				}
			case CButeMgr::DwordType:
				{
					m_pButeMgr->SetDword(sTag, sKey, *(DWORD *)pValueData);
					break;
				}
			case CButeMgr::ByteType:
				{
					m_pButeMgr->SetByte(sTag, sKey, *(BYTE *)pValueData);
					break;
				}
			case CButeMgr::BoolType:
				{
					m_pButeMgr->SetBool(sTag, sKey, *(bool *)pValueData);
					break;
				}
			case CButeMgr::DoubleType:
				{
					m_pButeMgr->SetDouble(sTag, sKey, *(double *)pValueData);
					break;
				}
			case CButeMgr::FloatType:
				{
					m_pButeMgr->SetFloat(sTag, sKey, *(float *)pValueData);
					break;
				}
			case CButeMgr::StringType:
				{
					m_pButeMgr->SetString(sTag, sKey, *(CString *)pValueData);
					break;
				}
			case CButeMgr::RectType:
				{
					m_pButeMgr->SetRect(sTag, sKey, *(CRect *)pValueData);
					break;
				}
			case CButeMgr::PointType:
				{
					m_pButeMgr->SetPoint(sTag, sKey, *(CPoint *)pValueData);
					break;
				}
			case CButeMgr::VectorType:
				{
					m_pButeMgr->SetVector(sTag, sKey, *(CAVector *)pValueData);
					break;
				}
			case CButeMgr::RangeType:
				{
					m_pButeMgr->SetRange(sTag, sKey, *(CARange *)pValueData);
					break;
				}
			default:
				{
					ASSERT(FALSE);
				}
			}
		}
	}
}

/************************************************************************/
// Adds a tag to the bute file
BOOL CButeEditDoc::AddTag(CString sTag)
{
	if (m_pButeMgr)
	{
		// Add the tag to ButeMgr
		if (m_pButeMgr->AddTag(sTag))
		{
			// Add the tag to the tag array
			m_tagArray.Add(sTag);

			return TRUE;
		}
	}

	return FALSE;
}