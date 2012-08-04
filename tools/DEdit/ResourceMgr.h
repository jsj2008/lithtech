//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ResourceMgr.h: interface for the CResourceMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESOURCEMGR_H__02D326E1_08CF_11D1_99E3_0060970987C3__INCLUDED_)
#define AFX_RESOURCEMGR_H__02D326E1_08CF_11D1_99E3_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


enum resource_type
{
	RESTYPE_PROJECT = 0,
	RESTYPE_WORLD,
	RESTYPE_SPRITE,
	RESTYPE_TEXTURE,
	RESTYPE_SOUND,
	RESTYPE_MODEL,
	RESTYPE_PREFAB,
	RESTYPE_PHYSICS,
	RESTYPE_UNKNOWN
};


class CResourceMgr  
{
public:
	CResourceMgr();
	virtual ~CResourceMgr();

	// All directories are relative to the project directory.
	static BOOL				CreateDir( CString path, resource_type resType );
	static resource_type	GetResourceType( char *ext );
	static resource_type	GetDirType(CString path);
	static BOOL				IsDirType(CString path, resource_type resType );
};

#endif // !defined(AFX_RESOURCEMGR_H__02D326E1_08CF_11D1_99E3_0060970987C3__INCLUDED_)
