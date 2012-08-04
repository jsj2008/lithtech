// New DirectEngine file access routines.  These allow you to open
// file trees, look at the contents, and open and read files in them.

// All the routines in here are case-insensitive.

// Most of the routines in here provide access to files with filenames
// relative to the file tree's base.  So if your file tree is based on the
// directory, "e:/dedit/textures", and you ask it to open "tex1.dtx", it'll
// really open "e:/dedit/textures/tex1.dtx".

#ifndef __DE_FILE_H__
#define __DE_FILE_H__


#ifndef __ILTSTREAM_H__
#include "iltstream.h"
#endif


typedef void* HLTFileTree;  // A file tree identifier.


// Returned by GetTreeType.
enum TreeType
{
	RezFileTree,
	DosTree
};


#define DIRECTORY_TYPE	0
#define FILE_TYPE		1

#define DFOPEN_READ	0


struct LTFindInfo
{
	int				m_Type;			// Is this a directory or file?
	char			m_Name[256];
	uint32	m_Date;			// File date/time identifier.
	uint32	m_Size;			// File size.
	void			*m_pInternal;
};



// One-time init and shutdown.
void df_Init();
void df_Term();


// Open and close file trees.  The file trees can come from
// rezfiles or directory trees.
// Fills in pTreePointer if everything goes well.
// Returns 0 if it worked.
// Returns -1 if the file/directory doesn't exist.
// Returns -2 if it exists but can't be opened.
int df_OpenTree(const char *pName, HLTFileTree *&pTreePointer);
void df_CloseTree(HLTFileTree *hTree);


// Find out what kind of tree a tree is.
TreeType df_GetTreeType(HLTFileTree *hTree);

// Returns true if the file exists, false otherwise.
// If it returns true, it fills in pInfo.
bool df_GetFileInfo(HLTFileTree *hTree, const char *pName, LTFindInfo *pInfo);

// Returns 1 if the directory exists, 0 otherwise.
int df_GetDirInfo(HLTFileTree *hTree, char *pName);


// Gets the full filename for the name you specify.  The name you specify
// is relative to the tree (like textures\8bit\tex1.dtx), and the output
// name is the full name (like e:\dedit\textures\8bit\tex1.dtx).
// (The file you specify doesn't have to exist..)
// Returns 0 if the file tree is a rezfile (in which case it doesn't make sense).
int df_GetFullFilename(HLTFileTree *hTree, char *pName, char *pOutName, int maxLen);


// Sets up a ILTStream for you to read from the file.  The ILTStream that it
// sets up will do a em_ThrowError() with ERR_FILEREAD if you try to 
// read or seek past the end of the file.
// The only supported openMode is DFOPEN_READ.
// Throws ERR_MEMORY, and the stream routines throw ERR_FILEREAD.
ILTStream* df_Open(HLTFileTree *hTree, const char *pName, int openMode=DFOPEN_READ);


// Iterate over the files in a directory.
// Initialize pInfo->m_pInternal to NULL before calling.
// Returns 0 when there are no more files.

// info.m_pInternal = NULL;
// while(df_FindNext(hTree, "textures/8bit", &info)) { ... }
int df_FindNext(HLTFileTree *hTree, const char *pDirName, LTFindInfo *pInfo);

// Terminate finding early. 
// Don't call this if df_FindNext() returns 0.
void df_FindClose(LTFindInfo *pInfo);


// Save the contents of a steam to a file
// returns 1 if successful 0 if an error occured
int df_Save(ILTStream *hFile, const char *pName);

// Returns raw file information for the file specified in pName found in the tree hTree
// sFileName returns the full name of the actual file that data is contained in 
//   if sFileName exceeds the size of nMaxFilename then an error is returned and the name is truncated
// sPos returns the position in the file that the data starts at
// nSize returns the size of the data 
int df_GetRawInfo(HLTFileTree *hTree, const char *pName, char* sFileName, unsigned int nMaxFileName, uint32* nPos, uint32* nSize);

#endif  // __DE_FILE_H__




