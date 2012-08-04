/****************************************************************************
;
;	 MODULE:		DIBMGR (.H)
;
;	PURPOSE:		DIB Manager Class
;
;	HISTORY:		02/15/96  [blg]  This file was created
;
;	COMMENT:		Copyright (c) 1996, Monolith Inc.
;
****************************************************************************/


#ifndef _DIBMGR_H_
#define	_DIBMGR_H_


// Includes...

#include "..\dibmgr\dib.h"
#include "..\dibmgr\dibpal.h"


// Libs...

#ifdef USE_PRAGMA_LIB
	#ifdef _AFXDLL
		#pragma comment (lib, "dibmgr_dll.lib")
	#else
		#pragma comment (lib, "dibmgr.lib")
	#endif
#endif


// Defines...

#define DMPF_NOIDENTITY		0x00000001


// Classes...

class CDibMgr
{
	// Member functions...

public:
	CDibMgr();
	~CDibMgr() { Term(); }

	BOOL					Init(HINSTANCE hInst, HWND hWnd, DWORD flags = 0);
	void					Term();

	BOOL					IsValid() { return(!!m_hInst && !!m_hWnd); }

	int						GetNumDibs() { return(m_collDibs.GetCount()); }
	int						GetNumPals() { return(m_collPals.GetCount()); }
	DWORD					GetFlags() { return(m_dwFlags); }
	CDibPal*				GetCurPal() { return(m_pCurPal); }

	void					SetCurPal(CDibPal* pPal) { m_pCurPal = pPal; }
	void					SetWindowHandle(HWND hWnd);
	void					SetPalette(CDib* pDib, CDibPal* pPal, BOOL bOwner = FALSE);

	CDib*					AddDib(int width, int height, int depth = 8, DWORD flags = 0);
	CDib*					AddDib(BYTE* pBytes, int width, int height, int depth = 8, DWORD flags = 0);
	CDib*					AddDib(BYTE* pBytes, int type, DWORD flags = 0);
	CDib*					AddDib(const char* sFile, DWORD flags = 0);

	CDib*					AddDib( CDib *pOriginalDib, CDibPal *pPal );

	CDibPal*				AddPal(PALETTEENTRY* pPes, DWORD flags = 0);
	CDibPal*				AddPal(BYTE* pRgbs, DWORD flags = 0);
	CDibPal*				AddPal(const char* sFile, DWORD flags = 0);
	CDibPal*				AddPal(BYTE *pData, DWORD dataLen, int type, DWORD flags = 0);

	void					RemoveDib(CDib* pDib);
	void					RemovePal(CDibPal* pPal);

	BOOL					ResizeDib(CDib* pDib, int width, int height, int depth = 8, DWORD flags = 0);

	HDC						GetDC(BOOL bPrep = TRUE, BOOL bBackPal = FALSE);
	BOOL					PrepDC(HDC hDC);
	void					ReleaseDC(HDC hDC);

	void					RemoveAllDibs();
	void					RemoveAllPals();

	static	HINSTANCE		GetGlobalInstanceHandle() { return(s_hInst); }


	// Member variables...

private:
	HINSTANCE				m_hInst;
	HWND					m_hWnd;
	DWORD					m_dwFlags;
	HPALETTE				m_hOldPal;

	CPtrList				m_collDibs;
	CPtrList				m_collPals;

	CDibPal*				m_pCurPal;

	static	HINSTANCE		s_hInst;
};


// Inlines...

inline CDibMgr::CDibMgr()
{
	m_hInst   = NULL;
	m_hWnd    = NULL;
	m_dwFlags = 0;
	m_pCurPal = NULL;
	m_hOldPal = NULL;
}

inline HDC CDibMgr::GetDC(BOOL bPrep, BOOL bBackPal)
{
	HDC hDC = ::GetDC(m_hWnd);
	if (bPrep && hDC && m_pCurPal)
	{
		m_hOldPal = SelectPalette(hDC, m_pCurPal->GetHandle(), bBackPal);
		RealizePalette(hDC);
	}
	return(hDC);
}

inline BOOL CDibMgr::PrepDC(HDC hDC)
{
	if (m_pCurPal)
	{
		m_hOldPal = SelectPalette(hDC, m_pCurPal->GetHandle(), FALSE);
		RealizePalette(hDC);
		return(TRUE);
	}
	return(FALSE);
}

inline void CDibMgr::ReleaseDC(HDC hDC)
{
	if (m_hOldPal)
	{
		SelectPalette(hDC, m_hOldPal, FALSE);
		m_hOldPal = NULL;
	}
	::ReleaseDC(m_hWnd, hDC);
}

inline void CDibMgr::SetWindowHandle(HWND hWnd)
{
	ASSERT(hWnd);
	ASSERT(::IsWindow(hWnd));

	m_hWnd = hWnd;
}


// EOF...

#endif
