// SharedResourceMgr.cpp: implementation of the CSharedResourceMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "basedefs_de.h"
#include "cpp_clientshell_de.h"
#include "SharedResourceMgr.h"
#include <mbstring.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSharedResourceMgr::CSharedResourceMgr()
{
	m_pClientDE=DNULL;
}

CSharedResourceMgr::~CSharedResourceMgr()
{
	Term();
}

// Intialization
DBOOL CSharedResourceMgr::Init(CClientDE *pClientDE)
{
	m_pClientDE=pClientDE;

	if (m_pClientDE == DNULL)
	{
		return DFALSE;
	}

	return DTRUE;
}

// Termination
void CSharedResourceMgr::Term()
{
	if (!m_pClientDE)
	{
		return;
	}

	unsigned int i;
	for (i=0; i < m_sharedSurfaceArray.GetSize(); i++)
	{
		m_pClientDE->DeleteSurface(m_sharedSurfaceArray[i]->m_hSurface);
		delete m_sharedSurfaceArray[i];
	}
	m_sharedSurfaceArray.SetSize(0);
}

// Returns a handle to a surface from its bitmap filename.  This increments the
// reference count by one.
HSURFACE CSharedResourceMgr::GetSurface(char *lpszSurface)
{
	if (!lpszSurface)
	{
		return DNULL;
	}

	// Search for the surface to see if it has been loaded yet
	int nIndex=FindSurfaceIndex(lpszSurface);

	// Return the surface if it is already loaded
	if (nIndex != -1)
	{
		m_sharedSurfaceArray[nIndex]->m_nReferenceCount++;
		return m_sharedSurfaceArray[nIndex]->m_hSurface;
	}

	// Load the surface
	HSURFACE hSurface=m_pClientDE->CreateSurfaceFromBitmap(lpszSurface);
	if (hSurface == DNULL)
	{
		return DNULL;
	}

	// Create the new resource class
	CSharedResourceSurface *pSharedSurface=new CSharedResourceSurface;

	// Copy the filename
	int nBufferSize=_mbstrlen(lpszSurface)+1;
	pSharedSurface->m_lpszPathName=new char[nBufferSize];
	_mbsncpy((unsigned char*)pSharedSurface->m_lpszPathName, (const unsigned char*)lpszSurface, nBufferSize);
	
	pSharedSurface->m_hSurface=hSurface;		// Copy the surface handle
	pSharedSurface->m_nReferenceCount++;		// Setup the reference count
	m_sharedSurfaceArray.Add(pSharedSurface);	// Add the class to the array

	// Return the surface handle
	return pSharedSurface->m_hSurface;
}

// Frees a surface.  It is only removed when its reference count reaches zero
void CSharedResourceMgr::FreeSurface(char *lpszSurface)
{
	int nIndex=FindSurfaceIndex(lpszSurface);

	if (nIndex == -1)
	{
		assert(DFALSE);
		return;
	}

	FreeSurface(nIndex);
}

// Frees a surface.  It is only removed when its reference count reaches zero
void CSharedResourceMgr::FreeSurface(HSURFACE hSurface)
{
	int nIndex=FindSurfaceIndex(hSurface);

	if (nIndex == -1)
	{
		assert(DFALSE);
		return;
	}

	FreeSurface(nIndex);
}

// Frees a surface based on its index into the surface array
void CSharedResourceMgr::FreeSurface(int nIndex)
{
	if (!m_pClientDE)
	{
		return;
	}

	m_sharedSurfaceArray[nIndex]->m_nReferenceCount--;

	if (m_sharedSurfaceArray[nIndex]->m_nReferenceCount == 0)
	{
		m_pClientDE->DeleteSurface(m_sharedSurfaceArray[nIndex]->m_hSurface);			

		delete m_sharedSurfaceArray[nIndex];
		m_sharedSurfaceArray.Remove(nIndex);
	}
}

// Finds a surface index into the surface array
int CSharedResourceMgr::FindSurfaceIndex(char *lpszSurface)
{
	unsigned int i;
	for (i=0; i < m_sharedSurfaceArray.GetSize(); i++)
	{
		if (_mbsicmp((const unsigned char*)m_sharedSurfaceArray[i]->m_lpszPathName, (const unsigned char*)lpszSurface) == 0)
		{
			return i;
		}
	}

	// Surface was not found
	return -1;
}

// Finds a surface index into the surface array
int CSharedResourceMgr::FindSurfaceIndex(HSURFACE hSurface)
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
