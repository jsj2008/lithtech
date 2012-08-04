#ifndef RealFileSystem_H
#define RealFileSystem_H

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the REALFILESYSTEM_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// REALFILESYSTEM_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
/*
#ifdef REALFILESYSTEM_EXPORTS
#define REALFILESYSTEM_API __declspec(dllexport)
#else
#define REALFILESYSTEM_API __declspec(dllimport)
#endif

// This class is exported from the RealFileSystem.dll
class REALFILESYSTEM_API CRealFileSystem {
public:
	CRealFileSystem(void);
	// TODO: add your methods here.
};

extern REALFILESYSTEM_API int nRealFileSystem;

REALFILESYSTEM_API int fnRealFileSystem(void);
*/

#endif // RealFileSystem_H