#include "StdAfx.h"
#include "LTMultiWnd.h"

#if defined(_DEBUG)
	#define new DEBUG_NEW
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMultiWnd::InitFromBitmap
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
BOOL CLTMultiWnd::InitFromBitmap(int nControlID, char* szWndName, CLTWnd* pParentWnd,
						CStringArray *pcollBitmaps, int xPos, int yPos, DWORD dwFlags,
						DWORD dwState)
{
	// Sanity Check
	if(m_bInitialized)
		return FALSE;

	// We will be deleting the surfaces
	m_bDeleteSurfOnTerm = TRUE;

	CLTSurfaceArray collSurfs;
	if(!CreateSurfaces(pcollBitmaps,&collSurfs))
		return FALSE;

	// Call the base class
	if(!Init(nControlID,szWndName,pParentWnd,&collSurfs,xPos,yPos,dwFlags,dwState))
	{
		FreeAllSurfaces(&collSurfs);
		return FALSE;
	}

	// All done!
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMultiWnd::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
BOOL CLTMultiWnd::Init(int nControlID, char* szWndName, CLTWnd* pParentWnd,
						CLTSurfaceArray *pcollSurfs, int xPos, int yPos, DWORD dwFlags, DWORD dwState)
{
	// Sanity Check
	if(m_bInitialized)
		return FALSE;

	if(!pcollSurfs)
		return FALSE;

	int nSize = pcollSurfs->GetSize();
	if(nSize == 0)
		return FALSE;

	for (int i = 0; i < nSize; i ++)
	{
		m_collSurfaces.Add(pcollSurfs->GetAt(i));
	}

	HSURFACE hSurf = m_collSurfaces.GetAt(0);

	// Call the base class
	if(!CLTMaskedWnd::Init(nControlID,szWndName,pParentWnd,hSurf,xPos,yPos,dwFlags,dwState))
		return FALSE;

	// All done!
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMultiWnd::FreeAllSurfaces
//
//	PURPOSE:	Frees all surfaces
//
// ----------------------------------------------------------------------- //
void CLTMultiWnd::FreeAllSurfaces(CLTSurfaceArray* pcollSurfs)
{
	HSURFACE hSurf;
	int i;
	if(pcollSurfs)
	{
		for (i = 0; i < pcollSurfs->GetSize(); i ++)
		{
			hSurf = pcollSurfs->GetAt(i);
            if (hSurf)
            {
                g_pLTClient->DeleteSurface(hSurf);
                hSurf = NULL;
            }
		}
	}

	for (i = 0; i < m_collSurfaces.GetSize(); i ++)
	{
		hSurf = m_collSurfaces.GetAt(i);
        if (hSurf)
        {
            g_pLTClient->DeleteSurface(hSurf);
            hSurf = NULL;
        }
	}
	m_collSurfaces.RemoveAll();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMultiWnd::CreateSurfaces
//
//	PURPOSE:	Creates all the surfaces
//
// ----------------------------------------------------------------------- //
BOOL CLTMultiWnd::CreateSurfaces(CStringArray *pcollBitmaps, CLTSurfaceArray* pcollSurfs)
{
	if(!pcollBitmaps || !pcollSurfs)
		return FALSE;

	// Create the surfaces
	HSURFACE hSurf;
	int nSize = pcollBitmaps->GetSize();
	if(nSize == 0)
		return FALSE;

	for (int i = 0; i < pcollBitmaps->GetSize(); i ++)
	{
        hSurf = g_pLTClient->CreateSurfaceFromBitmap((char *)(LPCSTR)pcollBitmaps->GetAt(i));
		if(!hSurf)
		{
			TRACE("CLTMultiWnd::InitFromBitmap - ERROR - Could not create the surface: ""%s""\n",pcollBitmaps->GetAt(i));
			FreeAllSurfaces(pcollSurfs);
			return FALSE;
		}

		pcollSurfs->Add(hSurf);
	}
	return TRUE;
}