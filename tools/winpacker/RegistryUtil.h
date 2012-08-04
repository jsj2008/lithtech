#ifndef __REGISTRYUTIL_H__
#define __REGISTRYUTIL_H__

// Gets the value in a registry key, or returns the default if it cannot be found
// The current key must be one of the HKEY_CLASSES_ROOT, etc. For the path, it should
// be like "dir\dir\keyname"
CString		GetRegistryKey(HKEY hCurrentKey, const char* pszKeyPath, const char* pszDefault);

// This will write out the specified key to the registry. The current key must be one of
// the predefined keys, and the path should be in the format mentioned above
BOOL		SetRegistryKey(HKEY hCurrentKey, const char* pszKeyPath, const char* pszValue);

//deletes the specified key from the registry
BOOL		DeleteRegistryKey(HKEY hCurrentKey, const char* pszKeyPath);

#endif
