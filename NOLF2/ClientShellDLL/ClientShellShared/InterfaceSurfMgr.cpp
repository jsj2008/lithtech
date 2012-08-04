// InterfaceSurfMgr.cpp: implementation of the CInterfaceSurfMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ltbasedefs.h"
#include "iclientshell.h"
#include "InterfaceSurfMgr.h"
#ifndef __PSX2
#include <mbstring.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInterfaceSurfMgr::CInterfaceSurfMgr()
{
    m_pClientDE=LTNULL;
}

CInterfaceSurfMgr::~CInterfaceSurfMgr()
{
	Term();
}

// Intialization
LTBOOL CInterfaceSurfMgr::Init(ILTClient *pClientDE)
{
	m_pClientDE=pClientDE;

    if (m_pClientDE == LTNULL)
	{
        return LTFALSE;
	}

    return LTTRUE;
}

// Termination
void CInterfaceSurfMgr::Term()
{
	if (!m_pClientDE)
	{
		return;
	}
	FreeAllSurfaces();
	return;
}


void CInterfaceSurfMgr::FreeAllSurfaces()
{
	unsigned int i;
	for (i=0; i < m_sharedSurfaceArray.GetSize(); i++)
	{
		m_pClientDE->DeleteSurface(m_sharedSurfaceArray[i]->m_hSurface);
		debug_delete(m_sharedSurfaceArray[i]);
	}
	m_sharedSurfaceArray.SetSize(0);
}

// Returns a handle to a surface from its bitmap filename.
HSURFACE CInterfaceSurfMgr::GetSurface(char *lpszSurface)
{
	if (!lpszSurface)
	{
        return LTNULL;
	}

	// Search for the surface to see if it has been loaded yet
	int nIndex=FindSurfaceIndex(lpszSurface);

	// Return the surface if it is already loaded
	if (nIndex != -1)
	{
		return m_sharedSurfaceArray[nIndex]->m_hSurface;
	}

	// Load the surface
	HSURFACE hSurface=m_pClientDE->CreateSurfaceFromBitmap(lpszSurface);
    if (hSurface == LTNULL)
	{
        return LTNULL;
	}

	// Create the new resource class
	CSharedSurface *pSharedSurface=debug_new(CSharedSurface);

	// Copy the filename
#ifdef __PSX2
    int nBufferSize=strlen(lpszSurface)+1;
#else
    int nBufferSize=_mbstrlen(lpszSurface)+1;
#endif
	pSharedSurface->m_lpszPathName=debug_newa(char, nBufferSize);
#ifdef __PSX2
    strncpy(pSharedSurface->m_lpszPathName, lpszSurface, nBufferSize);
#else
    _mbsncpy((unsigned char*)pSharedSurface->m_lpszPathName, (const unsigned char*)lpszSurface, nBufferSize);
#endif

	pSharedSurface->m_hSurface=hSurface;		// Copy the surface handle
	m_sharedSurfaceArray.Add(pSharedSurface);	// Add the class to the array

	// Return the surface handle
	return pSharedSurface->m_hSurface;
}

// Frees a surface.  It is only removed when its reference count reaches zero
void CInterfaceSurfMgr::FreeSurface(char *lpszSurface)
{
	int nIndex=FindSurfaceIndex(lpszSurface);

	if (nIndex == -1)
	{
		return;
	}

	FreeSurface(nIndex);
}

// Frees a surface.  It is only removed when its reference count reaches zero
void CInterfaceSurfMgr::FreeSurface(HSURFACE hSurface)
{
	int nIndex=FindSurfaceIndex(hSurface);

	if (nIndex == -1)
	{
		return;
	}

	FreeSurface(nIndex);
}

// Frees a surface based on its index into the surface array
void CInterfaceSurfMgr::FreeSurface(int nIndex)
{
	if (!m_pClientDE)
	{
		return;
	}

	m_pClientDE->DeleteSurface(m_sharedSurfaceArray[nIndex]->m_hSurface);

	debug_delete(m_sharedSurfaceArray[nIndex]);
	m_sharedSurfaceArray.Remove(nIndex);
}

// Finds a surface index into the surface array
int CInterfaceSurfMgr::FindSurfaceIndex(char *lpszSurface)
{
	unsigned int i;
	for (i=0; i < m_sharedSurfaceArray.GetSize(); i++)
	{
#ifdef __PSX2
        if (stricmp(m_sharedSurfaceArray[i]->m_lpszPathName, lpszSurface) == 0)
#else
        if (_mbsicmp((const unsigned char*)m_sharedSurfaceArray[i]->m_lpszPathName, (const unsigned char*)lpszSurface) == 0)
#endif
        {
			return i;
		}
	}

	// Surface was not found
	return -1;
}

// Finds a surface index into the surface array
int CInterfaceSurfMgr::FindSurfaceIndex(HSURFACE hSurface)
{
	unsigned int i;
	for (i=0; i < m_sharedSurfaceArray.GetSize(); i++)
	{
		if (m_sharedSurfaceArray[i]->m_hSurface == hSurface)
		{
			return i;
		}
	}

	// Surface was not found
	return -1;
}