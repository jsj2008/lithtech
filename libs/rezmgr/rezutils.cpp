#include "ltbasedefs.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#include <direct.h>
#else // LINUX
#include <sys/types.h>
#include <unistd.h>
#endif

#include <string.h>
#include "assert.h"
#define REZMGRDONTUNDEF
#include "rezmgr.h"
#include <time.h>

#define LithTechUserTitle "LithTech Resource File"

// if we are running the special LithRez version
BOOL g_bLithRez = FALSE;

CRezMgr* g_pMgr = NULL;

BOOL g_bVerbose = FALSE;
BOOL g_bCheckZeroLen = FALSE;

long g_nDirCount = 0;
long g_nRezCount = 0;
long g_nErrCount = 0;
long g_nWarnCount = 0;
BOOL g_bLowerCaseUsed = FALSE;
BOOL g_bExitOnDiskError = FALSE;

#define kMaxStr 2048

#ifndef _CONSOLE
#define zprintf printf
#else
#define zprintf printf
#endif


// ZMgrRezMgr Class
class CZMgrRezMgr : public CRezMgr
{
public:
    virtual  BOOL DiskError();
};

BOOL CZMgrRezMgr::DiskError()
{
    zprintf("ERROR! Disk error has occured (possibly drive is full).\n");
    g_nErrCount++;
#ifdef _CONSOLE
	exit(0);
#endif
	return FALSE;
};


BOOL ExtCheck (const char * sExtensions, const char * sExt )
{
#ifdef _LINUX
	notSupportedLinux ();
	return FALSE;
#else
	const unsigned ExtSize = 255;
	char szExtensions[ExtSize + 1];
	szExtensions[0] = '\0';
	strncpy(szExtensions, sExtensions, ExtSize);
	szExtensions[ExtSize] = '\0';

	// extensions must be separated by ;
	char *p = strtok ( szExtensions, ";" );

	do
	{
		if (p)
		{
			// Special just incase *.* was specified
			if ( strcmp( p, "*.*" ) == 0 )
				return TRUE;

			// specified this extension
			if ( stricmp ( p, sExt ) == 0 )
				return TRUE;

			p = strtok( NULL, ";" );
		  }

	  }  while(p);

	// couldn't find this extension
	return FALSE;
#endif

}


//---------------------------------------------------------------------------------------------------
// Extracts a directory full of resources into files
void ExtractDir(CRezDir* pDir, const char* sParamPath) {
  
#ifdef _LINUX
  notSupportedLinux ();
  return;
#else
  ASSERT(pDir != NULL);
  ASSERT(sParamPath != NULL);

  // output directory message to user
  if (g_bVerbose) zprintf("\nExtracting resource directory %s to %s\n",pDir->GetDirName(),sParamPath);

  // increment directories processed counter
  g_nDirCount++;

  // figure out the path to this dir with added backslash
  char sPath[kMaxStr];
  strcpy(sPath,sParamPath);
  if (sPath[strlen(sPath)-1] != '\\') strcat(sPath,"\\");

  // create the directory (just in case it doesn't exist)
  _mkdir(sPath);

  // search through all types in this dir
  CRezTyp* pTyp = pDir->GetFirstType();
  while (pTyp != NULL) {

    // create the string version of this type
    char sType[5];
    g_pMgr->TypeToStr(pTyp->GetType(),sType);

    // search through all resource of this type
    CRezItm* pItm = pDir->GetFirstItem(pTyp);
    while (pItm != NULL) {

      // read in item data from resource
      BYTE* pData = NULL;
      if (pItm->GetSize() > 0) pData = pItm->Load();
      if ((pData != NULL) || (pItm->GetSize() == 0)) {

        // figure out file name and path for data file
        char sFileName[kMaxStr];
        strcpy(sFileName,sPath);
        strcat(sFileName,pItm->GetName());
        strcat(sFileName,".");
        strcat(sFileName,sType);

        // print out message to user
        if (g_bVerbose) zprintf("Extracting: Type = %-4s Name = %-12s Size = %-8i\n",sType,pItm->GetName(),(int)pItm->GetSize());

        // increment resource counter
		g_nRezCount++;

        // open the file
        FILE* pFile = fopen(sFileName,"wb");
        if (pFile != NULL) {

          // write out the file
          if (pItm->GetSize() > 0) {
            if (fwrite(pData,pItm->GetSize(),1,pFile) != 1) {
              zprintf("ERROR! Unable to write file: %s\n",sFileName);
			  g_nErrCount++;
            }
          }

          // close the file
          fclose(pFile);
        }

        // error if unable to create data file
        else
		{
			zprintf("ERROR! Unable to create file: %s\n",sFileName);
			g_nErrCount++;
		}

        // free memory for resource
        pItm->UnLoad();

      // output error if unable to load resource
      }
      else {
        zprintf("ERROR! Unable to load resource. Name = %s\n",pItm->GetName());
  	    g_nErrCount++;
        continue;
      }

      // get next item
      pItm = pDir->GetNextItem(pItm);
    }

    // get next type
    pTyp = pDir->GetNextType(pTyp);
  }

  // search through all directories in this directory and recursivly call ExtractDir
  CRezDir* pLoopDir = pDir->GetFirstSubDir();
  while (pLoopDir != NULL) {

    // figure out the full path for the new directory
    char sDir[kMaxStr];
    strcpy(sDir,sPath);
    strcat(sDir,pLoopDir->GetDirName());
    strcat(sDir,"\\");

    // extract files into the directory
    ExtractDir(pLoopDir,sDir);

    // get next dir
    pLoopDir = pDir->GetNextSubDir(pLoopDir);
  }
#endif
};

//---------------------------------------------------------------------------------------------------
// Display the contents of a resource file
void ViewDir(CRezDir* pDir, const char* sParamPath) {
  ASSERT(pDir != NULL);
  ASSERT(sParamPath != NULL);

  // output directory message to user
  zprintf("\nDirectory %s :\n",pDir->GetDirName());

  // increment directories processed counter
  g_nDirCount++;

  // figure out the path to this dir with added backslash
  char sPath[kMaxStr];
  strcpy(sPath,sParamPath);
  if (sPath[strlen(sPath)-1] != '\\') strcat(sPath,"\\");

  // search through all types in this dir
  CRezTyp* pTyp = pDir->GetFirstType();
  while (pTyp != NULL) {

    // create the string version of this type
    char sType[5];
    g_pMgr->TypeToStr(pTyp->GetType(),sType);

    // search through all resource of this type
    CRezItm* pItm = pDir->GetFirstItem(pTyp);
    while (pItm != NULL) {

      // print out message to user
      zprintf("  Type = %-4s Name = %-12s Size = %-8i\n",sType,pItm->GetName(),(int)pItm->GetSize());

      // increment resource counter
	  g_nRezCount++;

      // get next item
      pItm = pDir->GetNextItem(pItm);
    }

    // get next type
    pTyp = pDir->GetNextType(pTyp);
  }

  // search through all directories in this directory and recursivly call ViewDir
  CRezDir* pLoopDir = pDir->GetFirstSubDir();
  while (pLoopDir != NULL) {

    // figure out the full path for the new directory
    char sDir[kMaxStr];
    strcpy(sDir,sPath);
    strcat(sDir,pLoopDir->GetDirName());
    strcat(sDir,"\\");

    // View files in the directory
    ViewDir(pLoopDir,sDir);

    // get next dir
    pLoopDir = pDir->GetNextSubDir(pLoopDir);
  }
};

//---------------------------------------------------------------------------------------------------
// Transfers a directory full of files into the resource file
void TransferDir(CRezDir* pDir, const char* sParamPath, const char * sExts ) {

#ifdef _LINUX
	notSupportedLinux ();
	return;
#else

  ASSERT(pDir != NULL);
  ASSERT(sParamPath != NULL);
  _finddata_t fileinfo;
  REZID nMiscID = 10000000;

  // output directory message to user
  if (g_bVerbose) zprintf("\nCreating resource directory %s from %s\n",pDir->GetDirName(),sParamPath);

  // increment directories processed counter
  g_nDirCount++;

  // figure out the path to this dir with added backslash
  char sPath[kMaxStr];
  strcpy(sPath,sParamPath);
  if (sPath[strlen(sPath)-1] != '\\') strcat(sPath,"\\");

  // figure out the find search string by adding *.* to search for everything
  char sFindPath[kMaxStr];
  strcpy(sFindPath,sPath);
  strcat(sFindPath,"*.*" );

  // being search for everything in this directory using findfirst and findnext
  long nFindHandle = _findfirst( sFindPath, &fileinfo );
  if (nFindHandle >= 0) {

    // loop through all entries in this directory
    do {

      // skip the files . and ..
      if (strcmp(fileinfo.name,".") == 0) continue;
      if (strcmp(fileinfo.name,"..") == 0) continue;

	  // if file name is empty put out warning and go to next file
	  if (strlen(fileinfo.name) == 0)
	  {
          zprintf("WARNING! Skipping file encountered with no base name.\n");
		  g_nWarnCount++;
		  continue;
	  }

      // if this is a subdirectory
      if ((fileinfo.attrib & _A_SUBDIR) == _A_SUBDIR) {

        // figure out the base name
        char sBaseName[kMaxStr];
        strcpy(sBaseName,fileinfo.name);
        if (!g_bLowerCaseUsed) strupr(sBaseName);

        // figure out the path name we are working on
        char sPathName[kMaxStr];
        strcpy(sPathName,sPath);
        strcat(sPathName,sBaseName);
        strcat(sPathName,"\\");

        // create new directory entry in resource file
        CRezDir* pNewDir = pDir->CreateDir(sBaseName);

        // error if directory not created
        if (pNewDir == NULL) {
          zprintf("ERROR! Unable to create directory.  Name = %s\n",sBaseName);
		  g_nErrCount++;
          continue;
        }

        // call TransferDir on the new directory
        TransferDir(pNewDir,sPathName, sExts );
      }

      // if this is a file add it to the resource file
      else {

        // figure out the file name we are working on
        char sFileName[kMaxStr];
        strcpy(sFileName,sPath);
        strcat(sFileName,fileinfo.name);

        // skip if file size is 0
//        if (fileinfo.size <= 0) continue;

        // check for zero len files
		if (g_bCheckZeroLen) {
			if (fileinfo.size <= 0) {
				zprintf("WARNING! Zero length file %s\n",sFileName);
				g_nWarnCount++;
			}
		}

        // split the file name up into its parts
        char drive[_MAX_DRIVE+1];
        char dir[_MAX_DIR+1];
        char fname[_MAX_FNAME+1];
        char ext[_MAX_EXT+1];
         _splitpath(sFileName, drive, dir, fname, ext );


		 char extCheck[_MAX_EXT+2] = "*";

		 strcat ( extCheck, ext );

		// make sure it is a valid extension
		if (!ExtCheck( sExts, extCheck ) )
			continue;


        // figure out the Name for this file
        char sName[kMaxStr];
		ASSERT((_MAX_FNAME+_MAX_EXT) <= kMaxStr);
        strcpy(sName,fname);

		// check if the extension is too long
		BOOL bExtensionTooLong = FALSE;
		if (strlen(ext) > 5)
		{
			bExtensionTooLong = TRUE;

			strcat(sName,ext);

			zprintf("WARNING! Filename %s extension too long.  Extension will be contained in name.\n",sFileName);
			g_nWarnCount++;
		}

        if (!g_bLowerCaseUsed) strupr(sName);

        // figure out the ID for this file (if name is all digits use it as ID number, otherwise assign a number)
        REZID nID;
        {
          int nNameLen = strlen(sName);
		  int i;
          for (i = 0; i < nNameLen; i++) {
            if ((sName[i] < '0') || (sName[i] > '9')) break;
          }
          if (i < nNameLen) {
            nID = nMiscID;
            nMiscID++;
          }
          else {
            nID = atol(sName);
          }
        }

        // figure out the Type for this file
        char sExt[5];
        REZTYPE nType;
		if (!bExtensionTooLong)
		{
			if (strlen(ext) > 0) {
				strcpy(sExt,&ext[1]);
				strupr(sExt);
				nType = g_pMgr->StrToType(sExt);
			}
			else nType = 0;
		}
		else
		{
			nType = 0;
		}

        // convert type back to string
        char sType[5];
        g_pMgr->TypeToStr(nType,sType);

        // print out message to user
        if (g_bVerbose) zprintf("Adding: Type = %-4s Name = %-12s Size = %-8i ID = %-8i\n",sType,sName,(int)fileinfo.size,(int)nID);

        // create new resource
        CRezItm* pItm = pDir->CreateRez(nID,sName,nType);

        // make sure resource was created
        if (pItm == NULL) {
          zprintf("ERROR! Unable to create resource from file %s  Rez Name = %s ID = %i\n",sFileName,sName,(int)nID);
		  g_nErrCount++;
          continue;
        }

        // increment resource counter
		g_nRezCount++;

    	// store the file time in the resource
		pItm->SetTime((REZTIME)fileinfo.time_write);

        // allocate memory for resource
        BYTE* pData = pItm->Create(fileinfo.size);

        // open the file
        FILE* pFile = fopen(sFileName,"rb");
        if (pFile != NULL) {

          // if the file has data
          if (fileinfo.size > 0) {

            // read in the file
            if (fread(pData,fileinfo.size,1,pFile) == 1) {

              // write resource out to resource file
              pItm->Save();
            }

            // error if unable to read data file
            else
			{
				zprintf("ERROR! Unable to read file: %s\n",sFileName);
				g_nErrCount++;
			}
          }

          // close the file
          fclose(pFile);
        }

        // error if unable to open data file
        else
		{
			zprintf("ERROR! Unable to open file: %s\n",sFileName);
		    g_nErrCount++;
		}

        // free memory for resource
        pItm->UnLoad();
      }

    // get the next entry in this directory
    } while (_findnext(nFindHandle, &fileinfo) == 0);

    // close out the directory findfirst and findnext
    _findclose(nFindHandle);
  }
#endif

};

//---------------------------------------------------------------------------------------------------
// Freshen a directory full of files into the resource file
void FreshenDir(CRezDir* pDir, const char* sParamPath) {
#ifdef _LINUX
	notSupportedLinux ();
	return;
#else
  ASSERT(pDir != NULL);
  ASSERT(sParamPath != NULL);
  _finddata_t fileinfo;

  // get the current time (THIS IS NOT NEEDED!)
//  REZTIME nCurTimeVal;
//  time_t nTimeVal;
//  nTimeVal = time(&nTimeVal);
//  nCurTimeVal = (REZTIME)nTimeVal;

  // output directory message to user
  if (g_bVerbose) zprintf("\nFreshening resource directory %s from %s\n",pDir->GetDirName(),sParamPath);

  // increment directories processed counter
  g_nDirCount++;

  // figure out the path to this dir with added backslash
  char sPath[kMaxStr];
  strcpy(sPath,sParamPath);
  if (sPath[strlen(sPath)-1] != '\\') strcat(sPath,"\\");

  // figure out the find search string by adding *.* to search for everything
  char sFindPath[kMaxStr];
  strcpy(sFindPath,sPath);
  strcat(sFindPath,"*.*");

  // being search for everything in this directory using findfirst and findnext
  long nFindHandle = _findfirst( sFindPath, &fileinfo );
  if (nFindHandle >= 0) {

    // loop through all entries in this directory
    do {

      // skip the files . and ..
      if (strcmp(fileinfo.name,".") == 0) continue;
      if (strcmp(fileinfo.name,"..") == 0) continue;

	  // if file name is empty put out warning and go to next file
	  if (strlen(fileinfo.name) == 0)
	  {
          zprintf("WARNING! Skipping file encountered with no base name.\n");
		  g_nWarnCount++;
		  continue;
	  }

      // if this is a subdirectory
      if ((fileinfo.attrib & _A_SUBDIR) == _A_SUBDIR) {

        // figure out the base name
        char sBaseName[kMaxStr];
        strcpy(sBaseName,fileinfo.name);
        if (!g_bLowerCaseUsed) strupr(sBaseName);

        // figure out the path name we are working on
        char sPathName[kMaxStr];
        strcpy(sPathName,sPath);
        strcat(sPathName,sBaseName);
        strcat(sPathName,"\\");

		CRezDir* pNewDir;

		// check if this dir exists in the rez file already
        pNewDir = pDir->GetDir(sBaseName);
        if (pNewDir == NULL) {

          // create new directory entry in resource file
          pNewDir = pDir->CreateDir(sBaseName);

          // error if directory not created
          if (pNewDir == NULL) {
            zprintf("ERROR! Unable to create directory.  Name = %s\n",sBaseName);
		    g_nErrCount++;
            continue;
          }
		}

        // call TransferDir on the new directory
        FreshenDir(pNewDir,sPathName);
      }

      // if this is a file add it to the resource file
      else {

        // figure out the file name we are working on
        char sFileName[kMaxStr];
        strcpy(sFileName,sPath);
        strcat(sFileName,fileinfo.name);              
        
        // skip if file size is 0
//        if (fileinfo.size <= 0) continue;
        
        // check for zero len files
		if (g_bCheckZeroLen) {
			if (fileinfo.size <= 0) {
				zprintf("WARNING! Zero length file %s\n",sFileName);
				g_nWarnCount++;
			}
		}
        // split the file name up into its parts
        char drive[_MAX_DRIVE+1];
        char dir[_MAX_DIR+1];
        char fname[_MAX_FNAME+1];
        char ext[_MAX_EXT+1];
         _splitpath(sFileName, drive, dir, fname, ext );
        
        // figure out the Name for this file
        char sName[kMaxStr];
        ASSERT(strlen(fname) < kMaxStr);
        strcpy(sName,fname);

		// check if the extension is too long
		BOOL bExtensionTooLong = FALSE;
		if (strlen(ext) > 5)
		{
			bExtensionTooLong = TRUE;

			strcat(sName,ext);

			zprintf("WARNING! Filename %s extension too long.  Extension will be contained in name.\n",sFileName);
			g_nWarnCount++;
		}

        if (!g_bLowerCaseUsed) strupr(sName);
        
        // figure out the Type for this file
        char sExt[5];
        REZTYPE nType;
		if (!bExtensionTooLong)
		{
			if (strlen(ext) > 0) {
				strcpy(sExt,&ext[1]);
				strupr(sExt);
				nType = g_pMgr->StrToType(sExt);
			}
			else nType = 0;
		}
		else
		{
			nType = 0;
		}

        // figure out the ID for this file (if name is all digits use it as ID number, otherwise assign a number)
        REZID nID;
        {
          int nNameLen = strlen(sName);
          int i;
          for (i = 0; i < nNameLen; i++) {
            if ((sName[i] < '0') || (sName[i] > '9')) break;
          }
          if (i < nNameLen) {
            nID = g_pMgr->GetNextIDNumToUse( );
            g_pMgr->SetNextIDNumToUse( nID + 1 );
          }
          else {
            nID = atol(sName);
          }
        }
        
        // convert type back to string 
        char sType[5];
        g_pMgr->TypeToStr(nType,sType);
       
		BYTE* pData;

		// see if this resource exists already, and if so does it need to be updated
		CRezItm* pItm = pDir->GetRez(sName,nType);
	    REZTIME nFileTime = (REZTIME)fileinfo.time_write;
		if (pItm != NULL) {

		  // check timestamp to see if we need to update this resource (if not on to next file!)
          if (nFileTime <= pItm->GetTime()) continue;

          if (g_bVerbose) zprintf("Update: Type = %-4s Name = %-12s Size = %-8i ID = %-8i\n",sType,sName,(int)fileinfo.size,(int)nID);

        }
        else {
          // print out message to user
          if (g_bVerbose) zprintf("Adding: Type = %-4s Name = %-12s Size = %-8i ID = %-8i\n",sType,sName,(int)fileinfo.size,(int)nID);
        
          // create new resource
          pItm = pDir->CreateRez(nID,sName,nType);
        
          // make sure resource was created
          if (pItm == NULL) {
            zprintf("ERROR! Unable to create resource from file %s.  Rez Name = %s ID = %i\n",sFileName,sName,(int)nID);
		    g_nErrCount++;
            continue;
          }
        
        }

        // increment resource counter
		g_nRezCount++;

        // allocate memory for resource
        pData = pItm->Create(fileinfo.size);

        // set the date and time for this new file
    	pItm->SetTime(nFileTime);

        // open the file
        FILE* pFile = fopen(sFileName,"rb");
        if (pFile != NULL) {
        
          // if the file has data
          if (fileinfo.size > 0) {

            // read in the file
            if (fread(pData,fileinfo.size,1,pFile) == 1) {
        
              // write resource out to resource file
              pItm->Save();
            }
        
            // error if unable to read data file
            else 
			{
				zprintf("ERROR! Unable to read file: %s\n",sFileName);
			    g_nErrCount++;
			}
          }
        
          // close the file
          fclose(pFile);
        }
        
        // error if unable to open data file
        else 
		{
			zprintf("ERROR! Unable to open file: %s\n",sFileName);
		    g_nErrCount++;
		}
        
        // free memory for resource
        pItm->UnLoad();
      }
        
    // get the next entry in this directory
    } while (_findnext(nFindHandle, &fileinfo) == 0);

    // close out the directory findfirst and findnext
    _findclose(nFindHandle);
  }
#endif

};


//---------------------------------------------------------------------------------------------------
// Notify of errors and warnings
void NotifyErrWarn() {
  if (g_nErrCount > 0) {
	  if (g_nErrCount == 1) zprintf("\n%i ERROR HAS OCCURED!!!!!!!\n",g_nErrCount);
	  else zprintf("\n%i ERRORS HAVE OCCURED!!!!!!!\n",g_nErrCount);
  }
  if (g_nWarnCount > 0) {
	  if (g_nWarnCount == 1) zprintf("\n%i WARNING HAS OCCURED!!!!!!!\n",g_nWarnCount);
	  else  zprintf("\n%i WARNINGS HAVE OCCURED!!!!!!!\n",g_nWarnCount);
  }
}


BOOL CheckLithHeader(CRezMgr* pMgr)
{
	// if the LithRez flag is not set then don't even check we are OK
	if (!g_bLithRez) return TRUE;

	// check the header
	char* sCheckTitle = pMgr->GetUserTitle();
	if (sCheckTitle != NULL) 
	{
		if (strcmp(sCheckTitle, LithTechUserTitle) == 0)
		{
			return TRUE;
		};
	}

    // get the root directory of the resource manager
    CRezDir* pDir = pMgr->GetRootDir();

	// if the header check failed then check for special shogo & blood2 files
	if (pDir->GetRezFromDosPath("cshell.dll") != NULL) return TRUE;
	if (pDir->GetRezFromDosPath("cres.dll") != NULL) return TRUE;
	if (pDir->GetRezFromDosPath("sres.dll") != NULL) return TRUE;
	if (pDir->GetRezFromDosPath("object.lto") != NULL) return TRUE;
	if (pDir->GetRezFromDosPath("patch.txt") != NULL) return TRUE;
	if (pDir->GetRezFromDosPath("sounds\\dirtypesounds.") != NULL) return TRUE;
	if (pDir->GetRezFromDosPath("blood2.dep") != NULL) return TRUE;
	if (pDir->GetRezFromDosPath("riot.dep") != NULL) return TRUE;

	// output error message
    zprintf("ERROR! Not a LithTech resource file!\n");

	return FALSE;
};

//---------------------------------------------------------------------------------------
// IsCommandSet
//
// Determines if a flag is set in the passed in command string
//
// -JohnO
BOOL IsCommandSet(char cFlag, const char* pszCommand)
{
	//get the length of the command
	int nCommandLen = strlen(pszCommand);

	//get the uppercase version of the flag
	char cUpperFlag = toupper(cFlag);

	//go through each character and do a case insensitive test for the flag
	for(int nCurrChar = 0; nCurrChar < nCommandLen; nCurrChar++)
	{
		if( cUpperFlag == toupper(pszCommand[nCurrChar]) )
		{
			//found it
			return TRUE;
		}
	}

	//didn't find it
	return FALSE;
}





//---------------------------------------------------------------------------------------------------
int RezCompiler(const char* sCmd, const char* sRezFile, const char* sTargetDir, BOOL bLithRez, const char * sFilespec ) {
  ASSERT(sCmd != NULL);
  ASSERT(sRezFile != NULL);

  // initialize global variables
  g_pMgr			= NULL;
  g_bVerbose		= FALSE;
  g_nDirCount		= 0;
  g_nRezCount		= 0;
  g_nErrCount		= 0;
  g_nWarnCount		= 0;
  g_bLowerCaseUsed	= FALSE;
  g_bLithRez		= bLithRez;

 
  //why does it do this uppercasing? This has been removed - JohnO
  //strupr(sRezFile);
  //if (sTargetDir != NULL) strupr(sTargetDir);
  
  // check for the verbose option
  g_bVerbose = IsCommandSet('V', sCmd);
  
  // check for the zero len check option
  g_bCheckZeroLen = IsCommandSet('Z', sCmd);

  // check for the zero len check option
  g_bLowerCaseUsed = IsCommandSet('L', sCmd);


  // get input parameters

  //default the command to information, but check for others, to make sure it
  //is exclusive
  char Command = 'I';

  if(IsCommandSet('V', sCmd))
	  Command = 'V';
  if(IsCommandSet('X', sCmd))
	  Command = 'X';
  if(IsCommandSet('C', sCmd))
	  Command = 'C';
  if(IsCommandSet('F', sCmd))
	  Command = 'F';
  if(IsCommandSet('S', sCmd))
	  Command = 'S';

  // execute command
  switch (Command) {

    // extract command
    case 'X': {
	  if (sTargetDir == NULL)
	  {
		zprintf("ERROR! Target directory missing.\n");
		g_nErrCount++;
		return 0;
	  }

      // open resource manager
      CZMgrRezMgr Mgr;
	  Mgr.SetItemByIDUsed(TRUE);
      Mgr.Open(sRezFile);
      g_pMgr = &Mgr;

	  // check for LithTech header
	  if (!CheckLithHeader(g_pMgr)) break;

      // get the root directory of the resource manager
      CRezDir* pDir = Mgr.GetRootDir();

	  // output initial message to user
      zprintf("\nExtracting rez file %s to directory %s\n",sRezFile,sTargetDir);
      
      // copy data from directory to resource file
      ExtractDir(pDir,sTargetDir);

	  // output stats to user
      if (g_bVerbose) zprintf("\n");
      zprintf("Finished extracting %i directories %i resources\n",g_nDirCount,g_nRezCount);

	  // if any errors or warnings have occured notify user
	  NotifyErrWarn();

      // close resource file
      Mgr.Close();

      break;
    }

    // view command
    case 'V': {
      // open resource manager
      CZMgrRezMgr Mgr;
	  Mgr.SetItemByIDUsed(TRUE);
      Mgr.Open(sRezFile);
      g_pMgr = &Mgr;

	  // check for LithTech header
	  if (!CheckLithHeader(g_pMgr)) break;

      // get the root directory of the resource manager
      CRezDir* pDir = Mgr.GetRootDir();

	  // output initial message to user
      zprintf("\nView resource file %s\n",sRezFile);
      
      // view rez file
      ViewDir(pDir,"");

	  // output stats to user
      zprintf("\n");
      zprintf("File contains %i directories %i resources\n",g_nDirCount,g_nRezCount);

	  // if any errors or warnings have occured notify user
	  NotifyErrWarn();

      // close resource file
      Mgr.Close();

      break;
    }

    // create command
    case 'C': {
	  if (sTargetDir == NULL)
	  {
		zprintf("ERROR! Target directory missing.\n");
		g_nErrCount++;
		return 0;
	  }

      // open resource manager
      CZMgrRezMgr Mgr;
	  Mgr.SetItemByIDUsed(TRUE);
      if (!Mgr.Open(sRezFile,FALSE,TRUE))
	  {
		  return(0);
	  }

      g_pMgr = &Mgr;

	  // set the LithTech header if necessary
	  if (g_bLithRez)
	  {
		  g_pMgr->SetUserTitle(LithTechUserTitle);
	  }

      // get the root directory of the resource manager
      CRezDir* pDir = Mgr.GetRootDir();

	  // output initial message to user
      zprintf("\nCreating rez file %s from directory %s\n",sRezFile,sTargetDir);
      
      // copy data from directory to resource file
      TransferDir(pDir,sTargetDir, sFilespec);

	  // output stats to user
      if (g_bVerbose) zprintf("\n");
      zprintf("Finished creating %i directories %i resources\n",g_nDirCount,g_nRezCount);

      // force the is sorted flag to true because this was all done in order
      Mgr.ForceIsSortedFlag(TRUE);

	  // if any errors or warnings have occured notify user
	  NotifyErrWarn();

      // close resource file
      Mgr.Close();

      break;
    }

    // freshen command
    case 'F': {
	  if (g_bLithRez) break;

	  if (sTargetDir == NULL)
	  {
		zprintf("ERROR! Target directory missing.\n");
		g_nErrCount++;
		return 0;
	  }

      // open resource manager
      CZMgrRezMgr Mgr;
	  Mgr.SetItemByIDUsed(TRUE);
      Mgr.Open(sRezFile,FALSE);
      g_pMgr = &Mgr;

	  // check for LithTech header
	  if (!CheckLithHeader(g_pMgr)) break;

      // get the root directory of the resource manager
      CRezDir* pDir = Mgr.GetRootDir();

	  // output initial message to user
      zprintf("\nFreshening rez file %s from directory %s\n",sRezFile,sTargetDir);
      
      // copy data from directory to resource file
      FreshenDir(pDir,sTargetDir);

	  // output stats to user
      if (g_bVerbose) zprintf("\n");
      zprintf("Finished freshening %i directories %i resources\n",g_nDirCount,g_nRezCount);

	  // if any errors or warnings have occured notify user
	  NotifyErrWarn();

      // close resource file
      Mgr.Close();

      break;
    }

    // sort command
    case 'S': {
	  if (g_bLithRez) break;

      // open resource manager
      CZMgrRezMgr Mgr;
	  Mgr.SetItemByIDUsed(TRUE);
      Mgr.Open(sRezFile);

	  // check for LithTech header
	  if (!CheckLithHeader(g_pMgr)) break;

	  // output initial message to user
      zprintf("\nSorting %s\n",sRezFile);

      // close resource file compacting it
      Mgr.Close(TRUE);

      break;
	};

	// information about rez file
	case 'I': {
	  if (g_bLithRez) break;
	  
      // open resource manager
      CZMgrRezMgr Mgr;
	  Mgr.SetItemByIDUsed(TRUE);
      if (!Mgr.Open(sRezFile)) {
	    zprintf("\nFailed to open file %s\n",sRezFile);
		break;
	  }

	  // output resource file information
	  zprintf("\n Rez File = %s\n",sRezFile);
	  if (Mgr.IsSorted()) zprintf("Is Sorted = TRUE\n");
	  else zprintf("Is Sorted = FALSE\n");
	  zprintf("\n");

	  // if any errors or warnings have occured notify user
	  NotifyErrWarn();

      // close resource file
      Mgr.Close();

	  break;
	};

    // invalid command
    default : {
      return 0;
      break;
    }
  }

  return g_nRezCount;
}
