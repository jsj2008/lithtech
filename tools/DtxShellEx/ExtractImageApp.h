// ExtractImageApp.h: interface for the CExtractImageApp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXTRACTIMAGEAPP_H__49EA73DA_8B07_4EB8_9378_0662CFE4D5C5__INCLUDED_)
#define AFX_EXTRACTIMAGEAPP_H__49EA73DA_8B07_4EB8_9378_0662CFE4D5C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CExtractImageApp : public CWinApp  
{
protected:
	CDocument*	m_pDoc;		// open doc 

public:
	CExtractImageApp();
	virtual ~CExtractImageApp();

	virtual CDocTemplate* CanOpenDocument(LPCTSTR lpszPath);
	virtual BOOL	LoadDoc(LPCTSTR lpFileName);
	virtual HBITMAP CreateThumbnail(const SIZE bmSize);
	virtual void DeleteOpenDoc();
	virtual int ExitInstance();
	virtual void OnDraw(CDC *pDC) {};
	virtual CSize GetDocSize() { return CSize(100,100); }
};

#endif // !defined(AFX_EXTRACTIMAGEAPP_H__49EA73DA_8B07_4EB8_9378_0662CFE4D5C5__INCLUDED_)
