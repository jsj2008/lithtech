//----------------------------------------------------------------------------------
// ResourceManager.h
//----------------------------------------------------------------------------------
#ifndef __ResourceManager_H__
#define __ResourceManager_H__

#include "WONGUI/GUIString.h"
#include "WONGUI/ResourceCollection.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// InfoDlg.
//----------------------------------------------------------------------------------
class ResourceManager
{
protected:
	HGLOBAL            m_hDefaultMem; // Memory Handle to Initialize Default Resources.
	ResourceCollection m_rcDefault;   // Default Resource Collection.
	ResourceCollection m_rcLanguage;  // Language Resource Collection.
	ResourceCollection m_rcCustom;    // Custom Resource Collection.

	//???                     // Key/Value Pairs (Values replace embedded keys in certain strings).

public:
	ResourceManager(void);
	~ResourceManager(void);

	void SetDefaultResourceModule(HMODULE hModule, int nID);
	inline void SetDefaultResourceFile(const std::string& sFile)  { m_rcDefault.Open(sFile); }
	inline ResourceCollection* GetDefaultResources(void)          { return &m_rcDefault; }

	inline void SetLanguageResourceFile(const std::string& sFile) { m_rcLanguage.Open(sFile); }
	inline ResourceCollection* GetLanguageResources(void)         { return &m_rcLanguage; }

	inline void SetCustomResourceFile(const std::string& sFile)   { m_rcCustom.Open(sFile); }
	inline ResourceCollection* GetCustomResources(void)           { return &m_rcCustom; }

	inline void PreLoadResources(void)
	{
		m_rcDefault.DecodeAll();
		m_rcLanguage.DecodeAll();
		m_rcCustom.DecodeAll();
	}
/*
	inline bool SetDefaultResourceDll(const CString& sDllName)
	{
		if (m_hDefResModule)
			FreeLibrary(m_hDefResModule);
		m_hDefResModule = ::LoadLibrary(sDllName);
		return m_hDefResModule != NULL;
	}
	inline HMODULE GetDefResourceDLL(void) const
	{
		return m_hDefResModule;
	}

	inline bool SetCustomResourceDll(const CString& sDllName)
	{
		if (m_hCustResModule)
			FreeLibrary(m_hCustResModule);
		m_hCustResModule = ::LoadLibrary(sDllName);
		return m_hCustResModule != NULL;
	}
	inline HMODULE GetCustomResourceDLL(void) const
	{
		return m_hCustResModule;
	}

	inline bool SetLanguageResourceDll(const CString& sDllName)
	{
		if (m_hLangResModule)
			FreeLibrary(m_hLangResModule);
		m_hLangResModule = ::LoadLibrary(sDllName);
		return m_hLangResModule != NULL;
	}
	inline HMODULE GetLanguageResourceDLL(void) const
	{
		return m_hLangResModule;
	}
*/

	void SetKeyValue(const GUIString& sKey, const GUIString& sValue);

	NativeImagePtr GetImage(int nId);
	NativeImagePtr GetBackgroundImage(int nId);
	NativeImagePtr GetGameLogoImage(int nId);
	NativeImagePtr GetMpsLogoImage(void);

//	HCURSOR   GetCursor(UINT nCursorID);
//	HICON     GetIcon(UINT nIconID);

	GUIString GetString(int nId);
	void AppendInfoString(GUIString& sInfo, UINT nId);
	GUIString BuildInfoString(UINT nFirstId);
};
typedef SmartPtr<ResourceManager> ResourceManagerPtr;

};


#endif
