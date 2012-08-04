/****************************************************************************
;
;	 MODULE:		DIBMGR (.CPP)
;
;	PURPOSE:		DIB Manager Class
;
;	HISTORY:		02/15/96  [blg]  This file was created
;
;	COMMENT:		Copyright (c) 1996, Monolith Inc.
;
****************************************************************************/


// Includes...

#include "stdafx.h"
#include "dibmgr.h"


// Statics...

HINSTANCE	CDibMgr::s_hInst = NULL;


// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CDibMgr::Init(HINSTANCE hInst, HWND hWnd, DWORD flags)
{
	// Sanity checks...

	ASSERT(!IsValid());
	ASSERT(hInst);
	ASSERT(hWnd);


	// Set simple member variables...

	m_hInst   = hInst;
	m_hWnd    = hWnd;
	m_dwFlags = flags;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CDibMgr::Term()
{
	RemoveAllDibs();
	RemoveAllPals();

	m_hInst   = NULL;
	m_hWnd    = NULL;
	m_dwFlags = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::RemoveDib
//
//	PURPOSE:	Removes the given dib
//
// ----------------------------------------------------------------------- //

void CDibMgr::RemoveDib(CDib* pDib)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(pDib && pDib->IsValid());
	if (!pDib) return;


	// Remove a palette if necessary...

	CDibPal* pPal = pDib->GetPalette();
	if (pPal && pDib->IsPaletteOwner())
	{
		RemovePal(pPal);
		SetPalette(NULL, FALSE);
	}


	// Remove the dib from our list...

	POSITION pos = pDib->GetPos();
	ASSERT(pos);

	if (pos) m_collDibs.RemoveAt(pos);


	// Destroy the dib...

	delete pDib;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::RemovePal
//
//	PURPOSE:	Removes the given palette
//
// ----------------------------------------------------------------------- //

void CDibMgr::RemovePal(CDibPal* pPal)
{
	// Sanity checks...

	ASSERT(IsValid());
	if (!pPal) return;


	// Remove the palette from our list...

	POSITION pos = pPal->GetPos();
	ASSERT(pos);

	if (pos) m_collPals.RemoveAt(pos);


	// Destroy the palette...

	delete pPal;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::RemoveAllDibs
//
//	PURPOSE:	Removes all dibs that we are managing
//
// ----------------------------------------------------------------------- //

void CDibMgr::RemoveAllDibs()
{
	// Delete all dibs from our collection...

	POSITION pos = m_collDibs.GetHeadPosition();

	while (pos)
	{
		CDib* pDib = (CDib*)m_collDibs.GetNext(pos);
		ASSERT(pDib && pDib->IsValid());

		delete pDib;
	}

	m_collDibs.RemoveAll();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::RemoveAllPals
//
//	PURPOSE:	Removes all pals that we are managing
//
// ----------------------------------------------------------------------- //

void CDibMgr::RemoveAllPals()
{
	// Delete all pals from our collection...

	POSITION pos = m_collPals.GetHeadPosition();

	while (pos)
	{
		CDibPal* pPal = (CDibPal*)m_collPals.GetNext(pos);
		ASSERT(pPal && pPal->IsValid());

		delete pPal;
	}

	m_collPals.RemoveAll();

	m_pCurPal = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::AddDib
//
//	PURPOSE:	Adds a new Dib object
//
// ----------------------------------------------------------------------- //

CDib* CDibMgr::AddDib(int width, int height, int depth, DWORD flags)
{
	// Sanity checks...

	ASSERT(IsValid());


	// Get an hDC to our window...

	HDC hDC = GetDC(FALSE);


	// Create and init a new dib...

	CDib* pDib = new CDib();

	if (!pDib->Init(hDC, width, height, depth, flags))
	{
		ReleaseDC(hDC);
		delete pDib;
		return(NULL);
	}


	// Add the new dib to our collection...

	POSITION pos = m_collDibs.AddTail(pDib);

	pDib->SetPos(pos);


	// Clean up...

	ReleaseDC(hDC);


	// All done...

	return(pDib);
}

CDib* CDibMgr::AddDib(BYTE* pBytes, int width, int height, int depth, DWORD flags)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(pBytes);


	// Get an hDC to our window...

	HDC hDC = GetDC(FALSE);


	// Create and init a new dib...

	CDib* pDib = new CDib();

	if (!pDib->Init(pBytes, hDC, width, height, depth, flags))
	{
		ReleaseDC(hDC);
		delete pDib;
		return(NULL);
	}


	// Add the new dib to our collection...

	POSITION pos = m_collDibs.AddTail(pDib);

	pDib->SetPos(pos);


	// Clean up...

	ReleaseDC(hDC);


	// All done...

	return(pDib);
}

CDib* CDibMgr::AddDib(BYTE* pBytes, int type, DWORD flags)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(pBytes);


	// Get an hDC to our window...

	HDC hDC = GetDC(FALSE);


	// Create and init a new dib...

	CDib* pDib = new CDib();

	if (!pDib->Init(pBytes, type, hDC, flags))
	{
		ReleaseDC(hDC);
		delete pDib;
		return(NULL);
	}


	// Add the new dib to our collection...

	POSITION pos = m_collDibs.AddTail(pDib);

	pDib->SetPos(pos);


	// Clean up...

	ReleaseDC(hDC);


	// All done...

	return(pDib);
}

CDib* CDibMgr::AddDib(const char* sFile, DWORD flags)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(sFile);


	// Get an hDC to our window...

	HDC hDC = GetDC(FALSE);


	// Set the global hInst in case we are trying to load a res...

	s_hInst = m_hInst;


	// Create and init a new dib...

	CDib* pDib = new CDib();

	if (!pDib->Init(sFile, hDC, flags))
	{
		ReleaseDC(hDC);
		delete pDib;
		return(NULL);
	}


	// Add the new dib to our collection...

	POSITION pos = m_collDibs.AddTail(pDib);

	pDib->SetPos(pos);


	// Clean up...

	ReleaseDC(hDC);


	// All done...

	return(pDib);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::AddDib
//
//	PURPOSE:	Copies the dib you pass in, and promotes FROM 8 TO 16 ONLY.
//
// ----------------------------------------------------------------------- //

CDib* CDibMgr::AddDib( CDib *pOriginalDib, CDibPal *pPal )
{
	ASSERT( IsValid() );
	ASSERT( pOriginalDib->GetDepth() == 8 );

	
	HDC		hDC = GetDC( FALSE );
	
	CDib	*pDib = new CDib();
	if( !pDib->Init(hDC, pOriginalDib, pPal) )
	{
		ReleaseDC( hDC );
		delete pDib;
		return NULL;
	}

	POSITION pos = m_collDibs.AddTail(pDib);
	pDib->SetPos(pos);

	ReleaseDC( hDC );

	return pDib;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::AddPal
//
//	PURPOSE:	Adds a new Pal object
//
// ----------------------------------------------------------------------- //

CDibPal* CDibMgr::AddPal(PALETTEENTRY* pPes, DWORD flags)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(pPes);


	// Create and init a new palette...

	CDibPal* pPal = new CDibPal();

	if (!pPal->Init(pPes, flags))
	{
		delete pPal;
		return(NULL);
	}


	// Add the new palette to our collection...

	POSITION pos = m_collPals.AddTail(pPal);

	pPal->SetPos(pos);


	// All done...

	return(pPal);
}

CDibPal* CDibMgr::AddPal(BYTE* pRgbs, DWORD flags)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(pRgbs);


	// Create and init a new palette...

	CDibPal* pPal = new CDibPal();

	if (!pPal->Init(pRgbs, flags))
	{
		delete pPal;
		return(NULL);
	}


	// Add the new palette to our collection...

	POSITION pos = m_collPals.AddTail(pPal);

	pPal->SetPos(pos);


	// All done...

	return(pPal);
}

CDibPal* CDibMgr::AddPal(const char* sFile, DWORD flags)
{
	// Sanity checks...

	ASSERT(IsValid());


	// Set the global hInst in case we are trying to load a res...

	s_hInst = m_hInst;


	// Create and init a new palette...

	CDibPal* pPal = new CDibPal();

	if (!pPal->Init(sFile, flags))
	{
		delete pPal;
		return(NULL);
	}


	// Add the new palette to our collection...

	POSITION pos = m_collPals.AddTail(pPal);

	pPal->SetPos(pos);


	// All done...

	return(pPal);
}


CDibPal* CDibMgr::AddPal(BYTE *pData, DWORD dataLen, int type, DWORD flags)
{
	ASSERT( IsValid() );


	CDibPal	*pPal = new CDibPal();
	if( !pPal->Init(pData, dataLen, type, flags) )
	{
		delete pPal;
		return NULL;
	}

	POSITION pos = m_collPals.AddTail( pPal );
	pPal->SetPos( pos );

	return pPal;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::AddPal
//
//	PURPOSE:	Adds a new Pal object
//
// ----------------------------------------------------------------------- //

BOOL CDibMgr::ResizeDib(CDib* pDib, int width, int height, int depth, DWORD flags)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(pDib && pDib->IsValid());
	if (!pDib) return(FALSE);


	// Get an hDC to our window...

	HDC hDC = GetDC(FALSE);


	// Resize the dib...

	BOOL bRet = pDib->Resize(hDC, width, height, depth, flags);


	// Clean up...

	ReleaseDC(hDC);


	// All done...

	return(bRet);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDibMgr::AddPal
//
//	PURPOSE:	Adds a new Pal object
//
// ----------------------------------------------------------------------- //

void CDibMgr::SetPalette(CDib* pDib, CDibPal* pPal, BOOL bOwner)
{
	// Sanity checks...

	ASSERT(IsValid());
	ASSERT(pDib && pDib->IsValid());
	ASSERT(pPal && pPal->IsValid());


	// Remove an old palette if necessary...

	CDibPal* pOldPal = pDib->GetPalette();

	if (pOldPal && pDib->IsPaletteOwner())
	{
		RemovePal(pOldPal);
		pDib->SetPalette(NULL, FALSE);
	}


	// Set the new palette...

	pDib->SetPalette(pPal, bOwner);
}



