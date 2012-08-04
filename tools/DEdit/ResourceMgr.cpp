//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ResourceMgr.cpp: implementation of the CResourceMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "resourcemgr.h"
#include "editprojectmgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResourceMgr::CResourceMgr()
{

}

CResourceMgr::~CResourceMgr()
{

}


//------------------------------------------------------------------------------------------
//
//  CResourceMgr::CreateDir
//
//  Purpose:	Creates a directory
//
//------------------------------------------------------------------------------------------
BOOL CResourceMgr::CreateDir( CString path, resource_type resType )
{
	CString fullPath, fullDirPath, fullName;	
	FILE *fp;

	CStringArray fileNameArray;

	switch( resType )
	{
		case RESTYPE_MODEL:
			fileNameArray.Add("DirTypeModels");
			break;

		case RESTYPE_SOUND:
			fileNameArray.Add("DirTypeSounds");
			break;

		case RESTYPE_SPRITE:
			fileNameArray.Add("DirTypeSprites");
			break;

		case RESTYPE_TEXTURE:
			fileNameArray.Add("DirTypeTextures");
			break;

		case RESTYPE_WORLD:
			fileNameArray.Add("DirTypeWorlds");
			break;

		case RESTYPE_PROJECT:
			fileNameArray.Add("DirTypeProject");
			break;

		case RESTYPE_PREFAB:
			fileNameArray.Add("DirTypeWorlds");
			fileNameArray.Add("DirTypePrefabs");
			break;

		case RESTYPE_PHYSICS:
			fileNameArray.Add("DirTypePhysics");
			break;

		default:			
			break;
	}

	fullDirPath = dfm_GetFullFilename(GetFileMgr(), path);
	if( !CreateDirectory(fullDirPath, NULL) )
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			CString str;
			str.FormatMessage( IDS_UNABLETOCREATEDIR, fullDirPath );
			AfxGetMainWnd()->MessageBox( str, AfxGetAppName(), MB_OK );
			return FALSE;
		}
	}

	if( fileNameArray.GetSize() > 0 )
	{
		int i;
		for (i=0; i < fileNameArray.GetSize(); i++)
		{
			fullName = dfm_BuildName( path, fileNameArray[i] );
			fullPath = dfm_GetFullFilename( GetFileMgr(), fullName );

			if( fp = fopen( (LPCTSTR)fullPath, "wb") )
				fclose(fp);
			else
				return FALSE;
		}
	}

	return TRUE;
}


//------------------------------------------------------------------------------------------
//
//  CResourceMgr::GetResourceType
//
//  Purpose:	Calculates the image index to use, given a file extention.
//
//------------------------------------------------------------------------------------------
resource_type CResourceMgr::GetResourceType( char *ext )
{
	CString s;

	s.LoadString( IDS_BARE_ED_EXTENSION );
	if( !stricmp( ext, s ))
		return RESTYPE_WORLD;
	s.LoadString( IDS_BARE_SPRITE_EXTENSION );
	if( !stricmp( ext, s ))
		return RESTYPE_SPRITE;
	s.LoadString( IDS_BARE_TEX_EXTENSION );
	if( !stricmp( ext, s ))
		return RESTYPE_TEXTURE;
	s.LoadString( IDS_BARE_SOUND_EXTENSION );
	if( !stricmp( ext, s ))
		return RESTYPE_SOUND;
	s.LoadString( IDS_BARE_MODEL_EXTENSION );
	if( !stricmp( ext, s ))
		return RESTYPE_MODEL;

	return RESTYPE_UNKNOWN;
}

//------------------------------------------------------------------------------------------
//
//  CResourceMgr::GetDirType
//
//  Purpose:	Calculates the image index to use, given a resource directory.
//
//------------------------------------------------------------------------------------------

resource_type CResourceMgr::GetDirType(CString dirName)
{
	if(IsDirType(dirName, RESTYPE_MODEL))
		return RESTYPE_MODEL;

	if(IsDirType(dirName, RESTYPE_SPRITE))
		return RESTYPE_SPRITE;

	if(IsDirType(dirName, RESTYPE_SOUND))
		return RESTYPE_SOUND;

	if(IsDirType(dirName, RESTYPE_TEXTURE))
		return RESTYPE_TEXTURE;

	if(IsDirType(dirName, RESTYPE_WORLD))
		return RESTYPE_WORLD;

	if(IsDirType(dirName, RESTYPE_PROJECT))
		return RESTYPE_PROJECT;

	if(IsDirType(dirName, RESTYPE_PREFAB))
		return RESTYPE_PREFAB;

	if(IsDirType(dirName, RESTYPE_PHYSICS))
		return RESTYPE_PHYSICS;

	return RESTYPE_UNKNOWN;
}


BOOL DoesDirHaveFile(CString path, char *pName)
{
	CString testDir;

	testDir = dfm_BuildName(path, pName);
	return dfm_DoesFileExist(GetFileMgr(), testDir);
}


BOOL CResourceMgr::IsDirType(CString path, resource_type resType)
{
	switch( resType )
	{
		case RESTYPE_MODEL:
			return DoesDirHaveFile(path, "DirTypeModels");

		case RESTYPE_SOUND:
			return DoesDirHaveFile(path, "DirTypeSounds");

		case RESTYPE_SPRITE:
			return DoesDirHaveFile(path, "DirTypeSprites");

		case RESTYPE_TEXTURE:
			return DoesDirHaveFile(path, "DirTypeTextures");

		case RESTYPE_WORLD:
			return DoesDirHaveFile(path, "DirTypeWorlds");

		case RESTYPE_PROJECT:
			return DoesDirHaveFile(path, "DirTypeProject");

		case RESTYPE_PREFAB:
			return DoesDirHaveFile(path, "DirTypePrefabs");

		case RESTYPE_PHYSICS:
			return DoesDirHaveFile(path, "DirTypePhysics");
	}

	return FALSE;
}


