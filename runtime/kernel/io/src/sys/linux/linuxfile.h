// New DirectEngine file access routines.  These allow you to open
// file trees, look at the contents, and open and read files in them.

// All the routines in here are case-insensitive.

// Most of the routines in here provide access to files with filenames
// relative to the file tree's base.  So if your file tree is based on the
// directory, "e:/dedit/textures", and you ask it to open "tex1.dtx", it'll
// really open "e:/dedit/textures/tex1.dtx".

#ifndef __DE_FILE_ACCESS_H__
#define __DE_FILE_ACCESS_H__


#ifndef __ILTSTREAM_H__
#include "iltstream.h"
#endif

	// g++ doesn't like the way that we were using
	// HLTFileTree without every defining it fully,
	// so use void* instead.
	typedef void* HLTFileTree; //class HLTFileTree;


	// Returned by GetTreeType.
	typedef enum
	{
		RezFileTree,
		DosTree,
		UnixTree
	} TreeType;


	#define DIRECTORY_TYPE	0
	#define FILE_TYPE		1

	#define DFOPEN_READ		0


	typedef struct LTFindInfo
	{
		int				m_Type;			// Is this a directory or file?
		char			m_Name[256];
		unsigned long	m_Date;			// File date/time identifier.
		unsigned long	m_Size;			// File size.
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
	void df_CloseTree(HLTFileTree* hTree);

	
	// Find out what kind of tree a tree is.
	TreeType df_GetTreeType(HLTFileTree* hTree);

	// Builds a path out of the two names (so if you pass in "bitmaps" and "bitmap1.pcx",
	// it'll build "bitmaps\bitmap1.pcx").
	void df_BuildName(char *pPart1, char *pPart2, char *pOut);


	// Returns 1 if the file exists, 0 otherwise.
	// If it returns 1, it fills in pInfo.
	int df_GetFileInfo(HLTFileTree* hTree, const char *pName, LTFindInfo *pInfo);

	// Returns 1 if the directory exists, 0 otherwise.
	int df_GetDirInfo(HLTFileTree hTree, char *pName);


	// Gets the full filename for the name you specify.  The name you specify
	// is relative to the tree (like textures\8bit\tex1.dtx), and the output
	// name is the full name (like e:\dedit\textures\8bit\tex1.dtx).
	// (The file you specify doesn't have to exist..)
	// Returns 0 if the file tree is a rezfile (in which case it doesn't make sense).
	int df_GetFullFilename(HLTFileTree hTree, char *pName, char *pOutName, int maxLen);


	// Sets up a DStream for you to read from the file.  The DStream that it
	// sets up will do a em_ThrowError() with ERR_FILEREAD if you try to 
	// read or seek past the end of the file.
	// The only supported openMode is DFOPEN_READ.
	// Throws ERR_MEMORY, and the stream routines throw ERR_FILEREAD.
	ILTStream* df_Open(HLTFileTree* hTree, const char *pName, int openMode=DFOPEN_READ);


	// Iterate over the files in a directory.
	// Initialize pInfo->m_pInternal to NULL before calling.
	// Returns 0 when there are no more files.
	
	// info.m_pInternal = NULL;
	// while(df_FindNext(hTree, "textures/8bit", &info)) { ... }
	int df_FindNext(HLTFileTree* hTree, const char *pDirName, LTFindInfo *pInfo);
	
	// Terminate finding early. 
	// Don't call this if df_FindNext() returns 0.
	void df_FindClose(LTFindInfo *pInfo);


	// Save the contents of a steam to a file
	// returns 1 if successful 0 if an error occured
	int df_Save(ILTStream *hFile, const char* pName);

#endif  // __DE_FILE_ACCESS_H__




