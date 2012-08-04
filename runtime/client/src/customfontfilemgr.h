//--------------------------------------------------------------------------------------
// CustomFontFileMgr.h
//
// This provides the definitions for the custom font file system. This system facilitates
// loading custom font files from the game resources and registering them with the
// operating system. Care must be taken so that it will properly work with the rez file
// formats since windows cannot load up font files that are not actual files, meaning
// that they must be temporarily extracted from the resource file.
//
//--------------------------------------------------------------------------------------

#ifndef __CUSTOMFONTFILEMGR_H__
#define __CUSTOMFONTFILEMGR_H__

#include <vector>

//this object represents a custom font file that has been registered. This exists primarily
//to serve as a locator to the file, and track whether or not it was an original file, or
//one that had to be extracted.
class CCustomFontFile
{
public:
	
	const char*		GetFilename() const			{ return m_pszFilename; }
	bool			IsExtracted() const			{ return m_bExtracted; }

private:

    //only allow the custom font manager to create and destroy these
	friend class CCustomFontFileMgr;

	CCustomFontFile();
	~CCustomFontFile();

	//the filename of this font file
	char	m_pszFilename[MAX_PATH + 1];

	//boolean indicating if the file was extracted
	bool	m_bExtracted;
};

//the manager of the custom font files. This primarily handles all the registration code, and also
//providing means of tracking the allocated custom fonts and providing warnings for leaked resources.
class CCustomFontFileMgr
{
public:

	~CCustomFontFileMgr();

	//provides access to the custom font file manager singleton
	static CCustomFontFileMgr&			GetSingleton();

	//called to register a custom font file. This will return NULL on an error
	CCustomFontFile*	RegisterCustomFontFile(const char* pszRelResource);

	//called to unregister a custom font file. This will return false if the custom font file
	//is not valid. Note that this will delete the font file so the pointer must not be used
	//after this call.
	bool				UnregisterCustomFontFile(CCustomFontFile* pFontFile);

private:

	//prevent external construction since this is intended to be used as a singleton
	CCustomFontFileMgr();

	//the list of the font files currently registered
	typedef std::vector<CCustomFontFile*>	TCustomFontFileList;
	TCustomFontFileList		m_FileList;
};


#endif
