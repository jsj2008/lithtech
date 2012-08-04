// ----------------------------------------------------------------------- //
//
// MODULE  : ServerAssetMgr.cpp
//
// PURPOSE : ServerAssetMgr - Implementation
//
// CREATED : 9/21/01 (based on PS2 ServerAssetMgr)
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerAssetMgr.h"

// define this to print out all of the files that assetmgr loads
//#define DEBUGASSETLOADING

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerAssetMgr::Init()
//
//	PURPOSE:	Initialize the asset mgr and cache all the files in
//				the specified attribute file
//
// ----------------------------------------------------------------------- //

LTBOOL CServerAssetMgr::Init(const char *szAttributeFile)
{
	int i = 0;

    if (!szAttributeFile) return(LTFALSE);

    // Load the asset file
    if (!Parse(szAttributeFile)) return(LTFALSE);

    char keyName[128];
	const int cMaxFiles = 1000;

    
	// Load Models...

	CString str;
	char* fileName;

    for (i = 0; i < cMaxFiles; i++) 
	{
        sprintf(keyName, "Model%03d", i);

        // End of list, stop
        if (!m_buteMgr.Exist("Models", keyName)) break;

		str = m_buteMgr.GetString("Models", keyName);

		if (!str.IsEmpty())
		{
			fileName = (char*)(LPCSTR)str;
			g_pLTServer->CacheFile(FT_MODEL, fileName);

#ifdef DEBUGASSETLOADING
			g_pLTServer->CPrint("CServerAssetMgr caching model %s\n", fileName);
#endif
		}
    }

    
	// Load Textures...

    for (i = 0; i < cMaxFiles; i++) 
	{
        sprintf(keyName, "Texture%03d", i);

        // End of list, stop
        if (!m_buteMgr.Exist("Textures", keyName)) break;
 
		str = m_buteMgr.GetString("Textures", keyName);

		if (!str.IsEmpty())
		{
			fileName = (char*)(LPCSTR)str;
			g_pLTServer->CacheFile(FT_TEXTURE, fileName);

#ifdef DEBUGASSETLOADING       
			g_pLTServer->CPrint("CServerAssetMgr caching texture %s\n", fileName);
#endif
		}
    }

    
	// Load Sprites...

    for (i = 0; i < cMaxFiles; i++) 
	{
        sprintf(keyName, "Sprite%03d", i);

        // End of list, stop
        if (!m_buteMgr.Exist("Sprites", keyName)) break;

		str = m_buteMgr.GetString("Sprites", keyName);

		if (!str.IsEmpty())
		{
			fileName = (char*)(LPCSTR)str;
			g_pLTServer->CacheFile(FT_SPRITE, fileName);

#ifdef DEBUGASSETLOADING       
			g_pLTServer->CPrint("CServerAssetMgr caching sprite %s\n", fileName);
#endif
		}
    }


    // Load Sounds...

    for (i = 0; i < cMaxFiles; i++) 
	{
        sprintf(keyName, "Sound%03d", i);

        // End of list, stop
        if (!m_buteMgr.Exist("Sounds", keyName)) break;

 		str = m_buteMgr.GetString("Sounds", keyName);

		if (!str.IsEmpty())
		{
			fileName = (char*)(LPCSTR)str;
			g_pLTServer->CacheFile(FT_SOUND, fileName);

#ifdef DEBUGASSETLOADING       
			g_pLTServer->CPrint("CServerAssetMgr caching sound %s\n", fileName);
#endif
		}
    }

    
    m_buteMgr.Term();
    
    return(LTTRUE);
}

