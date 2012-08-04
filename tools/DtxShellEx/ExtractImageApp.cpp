// ExtractImageApp.cpp: implementation of the CExtractImageApp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DtxShellExApp.h"
#include "ExtractImageApp.h"
#include "DtxShellExDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExtractImageApp::CExtractImageApp()
{
	m_pDoc = NULL;
}

CExtractImageApp::~CExtractImageApp()
{
	/*
	if(m_pDoc != NULL) 
	{
		m_pDoc->OnCloseDocument(); //will delete m_pDoc 
		m_pDoc = NULL;
	}
	*/
}

//////////////////
// This helper fn determines if I can open a document. It searches the doc
// templates for one whose file name extension matches the requested file.
// (copied from an article of MSJ magazine)
CDocTemplate* CExtractImageApp::CanOpenDocument(LPCTSTR lpszPath)
{
	CDocument* pDoc=NULL;
	POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
	while (pos != NULL) {
		CDocTemplate* pdt = m_pDocManager->GetNextDocTemplate(pos);
		if (pdt->MatchDocType(lpszPath,pDoc) >= CDocTemplate::yesAttemptNative)
			return pdt;
	}
	if (!pDoc) {
		// If you got here, you may have forgotten to set the string(s)
		// describing your document types
		//CString sErr; sErr.Format("***Can't find doc template for %s\n", lpszPath);
		//AfxMessageBox(sErr);
	}
	return NULL;
}

//////////////////
// Load document. This function is responsible for opening the document
// and setting m_pOpenDoc to the document loaded.
BOOL CExtractImageApp::LoadDoc(LPCTSTR lpFileName)
{
	ASSERT(lpFileName!=NULL);
	CString sFileName = lpFileName;
	CDocTemplate* pDocTemplate = CanOpenDocument(sFileName);
	if (pDocTemplate) 
	{
		if(!m_pDoc) {
			m_pDoc = pDocTemplate->CreateNewDocument();
			m_pDoc->m_bAutoDelete = TRUE;
		}
		if (m_pDoc)
		{		
			// Init our CDocument
			m_pDoc->DeleteContents();

			// Load the DTX and create buffers.
			((CDtxShellExDoc*)m_pDoc)->LoadDTX(sFileName);

			return TRUE;
			//delete pDoc;
		}
	}
	return FALSE;
}

HBITMAP CExtractImageApp::CreateThumbnail(const SIZE bmSize)
{
	HBITMAP hThumb; CBitmap bmpThumb; 
	if(!m_pDoc) return NULL;

	CSize bmDocSize = GetDocSize(); // derived class knows it
	// Create memory DC, create a color bitmap, and draw on it
	CDC memdc;
	memdc.CreateCompatibleDC(NULL);
	bmpThumb.CreateBitmap(bmSize.cx,bmSize.cy,
		memdc.GetDeviceCaps(PLANES), memdc.GetDeviceCaps(BITSPIXEL), NULL); 
	CBitmap* pOldBm = memdc.SelectObject(&bmpThumb);
	memdc.PatBlt(0,0,bmSize.cx,bmSize.cy,WHITENESS);
	memdc.SetMapMode(MM_ANISOTROPIC);
	memdc.SetViewportExt(bmSize.cx, bmSize.cy);
	memdc.SetWindowExt(bmDocSize.cx,bmDocSize.cy);
	OnDraw(&memdc); //let the derived class to handle it
	memdc.SelectObject(pOldBm); 
	hThumb = (HBITMAP)bmpThumb.Detach(); 
	return hThumb; 
}
 
void CExtractImageApp::DeleteOpenDoc()
{
	if(m_pDoc != NULL) 
	{
		m_pDoc->OnCloseDocument(); //will delete m_pDoc 
		m_pDoc = NULL;
	}
}

int CExtractImageApp::ExitInstance()
{
	DeleteOpenDoc();
	return CWinApp::ExitInstance();
}
