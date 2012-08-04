// InterfaceSurfMgr.h: interface for the CInterfaceSurfMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERFACESURFMGR_H__8B896D01_6918_11D2_BDAE_0060971BDC6D__INCLUDED_)
#define AFX_INTERFACESURFMGR_H__8B896D01_6918_11D2_BDAE_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "stdlith.h"
class CSharedSurface
{
public:
	// Constructor
	CSharedSurface()
	{
        m_lpszPathName=LTNULL;
        m_hSurface=LTNULL;
	}

	// Destructor
	~CSharedSurface()
	{
		if (m_lpszPathName)
		{
			debug_deletea(m_lpszPathName);
            m_lpszPathName=LTNULL;
		}
	}

public:
	char		*m_lpszPathName;
	HSURFACE	m_hSurface;
};

class CInterfaceSurfMgr
{
public:
	CInterfaceSurfMgr();
	virtual ~CInterfaceSurfMgr();

	// Intialization/Termination
    LTBOOL       Init(ILTClient *pClientDE);
	void		Term();

	// Returns a handle to a surface from its bitmap filename.
	HSURFACE	GetSurface(char *lpszSurface);

	// Frees a surface.
	void		FreeSurface(char *lpszSurface);
	void		FreeSurface(HSURFACE hSurface);

	void		FreeAllSurfaces();

protected:
	// Frees a surface based on its index into the surface array
	void		FreeSurface(int nIndex);

	// Finds a surface index into the surface array.  -1 is returned if it cannot be found
	int			FindSurfaceIndex(char *lpszSurface);
	int			FindSurfaceIndex(HSURFACE hSurface);

protected:
    ILTClient       *m_pClientDE;       // Pointer to client interface

	// Array of shared surfaces
	CMoArray<CSharedSurface *>	m_sharedSurfaceArray;
};

#endif // !defined(AFX_INTERFACESURFMGR_H__8B896D01_6918_11D2_BDAE_0060971BDC6D__INCLUDED_)