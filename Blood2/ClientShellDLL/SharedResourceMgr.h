// SharedResourceMgr.h: interface for the CSharedResourceMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHAREDRESOURCEMGR_H__8B896D01_6918_11D2_BDAE_0060971BDC6D__INCLUDED_)
#define AFX_SHAREDRESOURCEMGR_H__8B896D01_6918_11D2_BDAE_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "stdlith.h"
class CSharedResourceSurface
{
public:
	// Constructor
	CSharedResourceSurface()
	{
		m_lpszPathName=DNULL;
		m_hSurface=DNULL;
		m_nReferenceCount=0;
	}

	// Destructor
	~CSharedResourceSurface()
	{
		if (m_lpszPathName)
		{
			delete []m_lpszPathName;
			m_lpszPathName=DNULL;
		}		
	}

public:
	char		*m_lpszPathName;
	HSURFACE	m_hSurface;
	int			m_nReferenceCount;
};

class CSharedResourceMgr  
{
public:
	CSharedResourceMgr();
	virtual ~CSharedResourceMgr();

	// Intialization/Termination
	DBOOL		Init(CClientDE *pClientDE);
	void		Term();

	// Returns a handle to a surface from its bitmap filename.  This increments the
	// reference count by one.
	HSURFACE	GetSurface(char *lpszSurface);

	// Frees a surface.  It is only removed when its reference count reaches zero
	void		FreeSurface(char *lpszSurface);
	void		FreeSurface(HSURFACE hSurface);

protected:
	// Frees a surface based on its index into the surface array
	void		FreeSurface(int nIndex);

	// Finds a surface index into the surface array.  -1 is returned if it cannot be found
	int			FindSurfaceIndex(char *lpszSurface);
	int			FindSurfaceIndex(HSURFACE hSurface);

protected:
	CClientDE		*m_pClientDE;		// Pointer to client interface

	// Array of shared surfaces
	CMoArray<CSharedResourceSurface *>	m_sharedSurfaceArray;
};

#endif // !defined(AFX_SHAREDRESOURCEMGR_H__8B896D01_6918_11D2_BDAE_0060971BDC6D__INCLUDED_)
