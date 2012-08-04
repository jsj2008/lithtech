
//#ifndef _CONSOLE
//#include "windows.h"
//#endif
#include "ControlFileMgr.h"
#include "ltmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CONTROLFILEMAXSTRINGSIZE 2048

//**************************************************************************
// class CControlFileWord 

//--------------------------------------------------------------------------
BOOL CControlFileWord::GetVal(char* sVal, int nMaxStrLen)
{
	if (m_nStrLen > nMaxStrLen)
	{
		memcpy(sVal,m_sVal,nMaxStrLen);
		m_sVal[nMaxStrLen-1] = '\0';
	}
	else strcpy(sVal,m_sVal);
	return TRUE;
}


//--------------------------------------------------------------------------
int CControlFileWord::GetStrLen()
{
	return m_nStrLen;
}


//--------------------------------------------------------------------------
BOOL CControlFileWord::GetVal(int& nVal)
{
	nVal = atoi(m_sVal);
	return TRUE;
}


#pragma warning( push )
#pragma warning( disable : 4244 )
// Some code
//--------------------------------------------------------------------------
BOOL CControlFileWord::GetVal(float& nVal)
{
	nVal = atof(m_sVal);
	return TRUE;
}
#pragma warning( pop ) 


//--------------------------------------------------------------------------
BOOL CControlFileWord::GetVal(double& nVal)
{
	nVal = atof(m_sVal);
	return TRUE;
}


//**************************************************************************
// class CControlFileWordList 


//**************************************************************************
// class CControlFileKey

//--------------------------------------------------------------------------
CControlFileWord* CControlFileKey::GetFirstWord()
{
	return m_lstWords.GetFirst();
}


//--------------------------------------------------------------------------
CControlFileKey* CControlFileKey::NextWithSameName()
{
	CControlFileKey* pKey = this->Next();
	while (pKey != NULL)
	{
		if (stricmp(pKey->m_sKeyName,m_sKeyName) == 0) break;
		pKey = pKey->Next();
	}
	return pKey;
	
}


//**************************************************************************
//class CControlFileKeyList 

//--------------------------------------------------------------------------
CControlFileKey* CControlFileKeyList::Find(const char* sKeyName)
{
	CControlFileKey* pKey = GetFirst();
	while (pKey != NULL)
	{
		if (stricmp(pKey->m_sKeyName,sKeyName) == 0) break;
		pKey = pKey->Next();
	}
	return pKey;
}


//**************************************************************************
//class CControlFileSection 

//--------------------------------------------------------------------------
CControlFileKey* CControlFileSection::GetKey(const char* sKeyName)
{
	if (sKeyName == NULL) return m_lstKeys.GetFirst();
	else return m_lstKeys.Find(sKeyName);
}


//**************************************************************************
//class CControlFileSectionList : public CLithBaseList<CControlFileSection> 

//--------------------------------------------------------------------------
CControlFileSection* CControlFileSectionList::Find(const char* sSectionName)
{
	CControlFileSection* pSection = GetFirst();
	while (pSection != NULL)
	{
		if (stricmp(pSection->m_sSectionName,sSectionName) == 0) break;
		pSection = pSection->Next();
	}
	return pSection;
}


//**************************************************************************
// class CControlFileMgr

//--------------------------------------------------------------------------
CControlFileMgr::CControlFileMgr()
{
	// class was just created so we are not yet initialized
	m_bInitialized = FALSE;
}


//--------------------------------------------------------------------------
CControlFileMgr::CControlFileMgr(const char* sFileName, const char* sWhiteSpace)
{
	// class was just created so we are not yet initialized
	m_bInitialized = FALSE;

	// initialize the mgr and read in and process the specified control file
	Init(sFileName,sWhiteSpace);
}


//--------------------------------------------------------------------------
CControlFileMgr::~CControlFileMgr()
{
	// terminate the Mgr
	Term();

	// clear all of the defines that have been made with AddDefine
	ClearDefines();
}


//--------------------------------------------------------------------------
BOOL CControlFileMgr::Init(const char* sFileName, const char* sWhiteSpace)
{
	ASSERT(sFileName != NULL);
	LT_MEM_TRACK_ALLOC(m_sFileName.Alloc(strlen(sFileName)+1), LT_MEM_TYPE_MISC);
	m_sFileName.Copy(sFileName,strlen(sFileName)+1);

	if (sWhiteSpace == NULL)
	{
		LT_MEM_TRACK_ALLOC(m_sWhiteSpace.Alloc(6), LT_MEM_TYPE_MISC);
		m_sWhiteSpace.Copy(" =,\t\15",6);
	}
	else
	{
		LT_MEM_TRACK_ALLOC(m_sWhiteSpace.Alloc(strlen(sWhiteSpace)), LT_MEM_TYPE_MISC);
		m_sWhiteSpace.Copy(sWhiteSpace,strlen(sWhiteSpace)+1);
	}

	m_cEndOfLine = '\n';
	m_cSectionStart = '[';
	m_cSectionEnd = ']';
	m_cComment = ';';
	m_cStringMarker = '\"';
	m_cSpecialCommand = '#';

	BOOL bRetVal = ProcessFile(sFileName);

	m_bInitialized = TRUE;

	if (bRetVal)
	{
		return TRUE;
	}
	else
	{
		Term();
		return FALSE;
	}
}


//--------------------------------------------------------------------------
void CControlFileMgr::Term()
{
	if (m_bInitialized)
	{
		m_sFileName.Free();
		m_sWhiteSpace.Free();

		// delete all sections in this control file
		CControlFileSection* pSection;
		while ((pSection = m_lstSections.GetFirst()) != NULL)
		{

			// delete all keys in this section
			CControlFileKey* pKey;
			while ((pKey = pSection->m_lstKeys.GetFirst()) != NULL)
			{

				// delete all words in this key
				CControlFileWord* pWord;
				while ((pWord = pKey->m_lstWords.GetFirst()) != NULL)
				{
					pKey->m_lstWords.Delete(pWord);
					delete pWord;
				}

				pSection->m_lstKeys.Delete(pKey);
				delete pKey;
			}

			m_lstSections.Delete(pSection);
			delete pSection;
		}

		m_bInitialized = FALSE;
	}
}


//--------------------------------------------------------------------------
CControlFileSection* CControlFileMgr::GetSection(const char* sSectionName)
{
	if (sSectionName == NULL) return m_lstSections.GetFirst();
	else return m_lstSections.Find(sSectionName);
}


//--------------------------------------------------------------------------
CControlFileKey* CControlFileMgr::GetKey(const char* sSectionName, const char* sKeyName)
{
	CControlFileSection* pSection = GetSection(sSectionName);
	if (pSection != NULL)
	{
		return pSection->GetKey(sKeyName);
	}
	else return NULL;
}


//--------------------------------------------------------------------------
BOOL CControlFileMgr::GetKeyVal(const char* sSectionName, const char* sKeyName, int& nVal)
{
	CControlFileKey* pKey = GetKey(sSectionName,sKeyName);
	if (pKey != NULL)
	{
		CControlFileWord* pWord = pKey->GetFirstWord();
		if (pWord != NULL)
		{
			return pWord->GetVal(nVal);
		}
		else return FALSE;
	}
	else return FALSE;
}


//--------------------------------------------------------------------------
BOOL CControlFileMgr::GetKeyVal(const char* sSectionName, const char* sKeyName, float& nVal)
{
	CControlFileKey* pKey = GetKey(sSectionName,sKeyName);
	if (pKey != NULL)
	{
		CControlFileWord* pWord = pKey->GetFirstWord();
		if (pWord != NULL)
		{
			return pWord->GetVal(nVal);
		}
		else return FALSE;
	}
	else return FALSE;
}


//--------------------------------------------------------------------------
BOOL CControlFileMgr::GetKeyVal(const char* sSectionName, const char* sKeyName, double& nVal)
{
	CControlFileKey* pKey = GetKey(sSectionName,sKeyName);
	if (pKey != NULL)
	{
		CControlFileWord* pWord = pKey->GetFirstWord();
		if (pWord != NULL)
		{
			return pWord->GetVal(nVal);
		}
		else return FALSE;
	}
	else return FALSE;
}


//--------------------------------------------------------------------------
BOOL CControlFileMgr::GetKeyVal(const char* sSectionName, const char* sKeyName, char* sVal, int nMaxStrLen)
{
	CControlFileKey* pKey = GetKey(sSectionName,sKeyName);
	if (pKey != NULL)
	{
		CControlFileWord* pWord = pKey->GetFirstWord();
		if (pWord != NULL)
		{
			return pWord->GetVal(sVal,nMaxStrLen);
		}
		else return FALSE;
	}
	else return FALSE;
}


//--------------------------------------------------------------------------
int CControlFileMgr::GetKeyValStrLen(const char* sSectionName, const char* sKeyName)
{
	CControlFileKey* pKey = GetKey(sSectionName,sKeyName);
	if (pKey != NULL)
	{
		CControlFileWord* pWord = pKey->GetFirstWord();
		if (pWord != NULL)
		{
			return pWord->GetStrLen();
		}
		else return FALSE;
	}
	else return 0;
}


//--------------------------------------------------------------------------
// add a define that can be checked with #ifdef or #ifndef special commands
// this must be done before init is called!
//--------------------------------------------------------------------------
BOOL CControlFileMgr::AddDefine(const char* sDefine)
{
	// can't make a NULL define
	if (sDefine == NULL) return FALSE;

	// figure out the lenght of the new define
	int nDefineLenCount = strlen(sDefine);

	// can't make a zero length define
	if (nDefineLenCount < 1) return FALSE;

	// allocate the new define object
	CControlFileDefine* pDefine;
	LT_MEM_TRACK_ALLOC(pDefine = new CControlFileDefine,LT_MEM_TYPE_MISC);

	// make sure allocation succeeded
	if (pDefine == NULL) return FALSE;

	// allocate the string contaned in the define object
	pDefine->m_sVal.Alloc(nDefineLenCount+1);

	// copy the new define value
	pDefine->m_sVal.Copy(sDefine, nDefineLenCount+1);

	// we have succeeded
	return TRUE;
}


//--------------------------------------------------------------------------
// clears all defines that have been made with AddDefine
//--------------------------------------------------------------------------
void CControlFileMgr::ClearDefines()
{
	// pointer to use to iterate the define list
	CControlFileDefine* pDefine;

	// loop through all defines 
	while ((pDefine = m_lstDefines.GetFirst()) != NULL)
	{
		// remove define from define list
		m_lstDefines.Delete(pDefine);

		// delete the define
		delete pDefine;
	}
}


//--------------------------------------------------------------------------
// returns true if the given string has been passed into AddDefine
//--------------------------------------------------------------------------
BOOL CControlFileMgr::IsDefined(const char* sDefine)
{
	// NULL does not match anything
	if (sDefine == NULL) return FALSE;

	// empty string does not match anything
	if (sDefine[0] == '\0') return FALSE;

	// pointer to use to iterate the define list
	CControlFileDefine* pDefine = m_lstDefines.GetFirst();

	// loop through all defines
	while (pDefine != NULL)
	{
		// does this define match
		if (stricmp(pDefine->GetVal(), sDefine) == 0) return TRUE;

		// go to next item
		pDefine = pDefine->Next();
	}

	// we found nothing that matches
	return FALSE;
}

// information about ifdef stack
#define IFDEFSTACKSIZE 1024
#define STACKCMD_NONE 0
#define STACKCMD_IFDEF_USED 1
#define STACKCMD_IFNDEF_USED 2
#define STACKCMD_ELSE_USED 3
#define STACKCMD_UNUSED 1000
#define STACKCMD_IFDEF_UNUSED 1001
#define STACKCMD_IFNDEF_UNUSED 1002
#define STACKCMD_ELSE_UNUSED 1003


//--------------------------------------------------------------------------
BOOL CControlFileMgr::ProcessFile(const char* sFileName)
{
	ASSERT(sFileName != NULL);

	// if the skip level is greater than zero then we are skipping commands
	int nIfDefSkipLevel = 0;

	// stack used to store ifdef information
	int aryIfDefStack[IFDEFSTACKSIZE];

	// current position in IfDef stack
	int nIfDefStackPos = -1;

	// open file for text mode read, exit with error if open fails
	if (!FileOpen(sFileName)) return FALSE;

	// set up default empty section as current section
	CControlFileSection* pCurSection;
	LT_MEM_TRACK_ALLOC(pCurSection = new CControlFileSection,LT_MEM_TYPE_MISC);
	LT_MEM_TRACK_ALLOC(pCurSection->m_sSectionName.Alloc(1), LT_MEM_TYPE_MISC);
	pCurSection->m_sSectionName[0] = '\0';
	m_lstSections.InsertLast(pCurSection);

	// loop to read in file
	char cVal;
	while (!FileEOF())
	{
		// get char from file
		cVal = FileGetChar();	

		// check for end of file
		if (FileEOF()) break;

		// check if this char is whitespace (if so go to next char)
		if (strchr(m_sWhiteSpace,cVal) != NULL) continue;

		// chck if this is an end of line (if so go to next char)
		if (cVal == m_cEndOfLine) continue;

		// check for a comment line
		if (cVal == m_cComment)
		{
			while (!FileEOF())
			{
				if (cVal == m_cEndOfLine) break;
				cVal = FileGetChar();
			}
			continue;
		}

		// check for a special command
		if (cVal == m_cSpecialCommand)
		{
			CLithSimpAryStat<char,CONTROLFILEMAXSTRINGSIZE> sSpecialCommand;
			CLithSimpAryStat<char,CONTROLFILEMAXSTRINGSIZE> sSpecialParameter;

			// read in the special command
			int nCount = 0;
			while (!FileEOF())
			{
				cVal = FileGetChar();
				if (strchr(m_sWhiteSpace,cVal) != NULL) break;
				if (cVal == m_cEndOfLine) break;
				if (FileEOF()) break;
				if (nCount < (CONTROLFILEMAXSTRINGSIZE-1))
				{
					sSpecialCommand[nCount] = cVal;
					nCount++;
				}
			}
			sSpecialCommand[nCount] = '\0';

			// skip whitespace
			while (!FileEOF())
			{
				cVal = FileGetChar();
				if (strchr(m_sWhiteSpace, cVal) == NULL) break;
				if (cVal == m_cEndOfLine) break;
			}

			// read in the special parameter (if present)
			nCount = 0;
			while (!FileEOF())
			{
				cVal = FileGetChar();
				if (strchr(m_sWhiteSpace,cVal) != NULL) break;
				if (cVal == m_cEndOfLine) break;
				if (FileEOF()) break;
				if (nCount < (CONTROLFILEMAXSTRINGSIZE-1))
				{
					sSpecialParameter[nCount] = cVal;
					nCount++;
				}
			}
			sSpecialParameter[nCount] = '\0';

			// is this an ifdef special command
			if (stricmp(sSpecialCommand,"ifdef") == 0)
			{
				// make sure the stack is not full
				if (nIfDefStackPos < (IFDEFSTACKSIZE-1))
				{
					// increment the stack position
					nIfDefStackPos++;

					// check if this define has been defined
					if (IsDefined(sSpecialParameter)) 
					{
						// set stack item to a used ifdef
						aryIfDefStack[nIfDefStackPos] = STACKCMD_IFDEF_USED;
					}
					else
					{
						// set stack item to an unused ifdef
						aryIfDefStack[nIfDefStackPos] = STACKCMD_IFDEF_UNUSED;

						// we are to skip this section so increment the skip level
						nIfDefSkipLevel++;
					}
				}
			}

			// is this an ifndef special command
			if (stricmp(sSpecialCommand,"ifndef") == 0)
			{
				// make sure the stack is not full
				if (nIfDefStackPos < (IFDEFSTACKSIZE-1))
				{
					// increment the stack position
					nIfDefStackPos++;

					// check if this define has not been defined
					if (!IsDefined(sSpecialParameter)) 
					{
						// set stack item to a used ifndef
						aryIfDefStack[nIfDefStackPos] = STACKCMD_IFNDEF_USED;
					}
					else
					{
						// set stack item to an unused ifndef
						aryIfDefStack[nIfDefStackPos] = STACKCMD_IFNDEF_UNUSED;

						// we are to skip this section so increment the skip level
						nIfDefSkipLevel++;
					}
				}
			}

			// is this an else special command
			if (stricmp(sSpecialCommand,"else") == 0)
			{
				// make sure the stack has something in it
				if (nIfDefStackPos >= 0)
				{
					// if we are currently in an unused section
					if (aryIfDefStack[nIfDefStackPos] > STACKCMD_UNUSED)
					{
						// then this else is to switch to a used section replace if with else in stack
						aryIfDefStack[nIfDefStackPos] = STACKCMD_ELSE_USED;

						// decrement nIfDefSkipLevel
						nIfDefSkipLevel--;
					}
					// otherwise we are in a used section
					else
					{
						// so switch to an unused section replace if with else in stack
						aryIfDefStack[nIfDefStackPos] = STACKCMD_ELSE_UNUSED;

						// increment nIfDefSkipLevel
						nIfDefSkipLevel++;
					}
				}
			}

			// is this an endif special command
			if (stricmp(sSpecialCommand,"endif") == 0)
			{
				// make sure the stack has something in it
				if (nIfDefStackPos >= 0)
				{
					// if we were in an unused section
					if (aryIfDefStack[nIfDefStackPos] > STACKCMD_UNUSED)
					{
						// decrement nIfDefSkipLevel
						nIfDefSkipLevel--;
					}

					// decrement stack pointer
					nIfDefStackPos--;
				}
				
			}

			// skip to end of line we don't care what comes after a # command is finished
			while (!FileEOF())
			{
				if (cVal == m_cEndOfLine) break;
				cVal = FileGetChar();
			}
			continue;

		}

		// check the ifdef skip level and see if we should go on or skip this line
		if (nIfDefSkipLevel > 0)
		{
			while (!FileEOF())
			{
				if (cVal == m_cEndOfLine) break;
				cVal = FileGetChar();
			}
			continue;
		}

		// check for a section header
		if (cVal == m_cSectionStart)
		{
			CLithSimpAryStat<char,CONTROLFILEMAXSTRINGSIZE> sSectionName;

			// read in name (note this can include spaces or anything up until the ] character)
			int nCount = 0;
			while (!FileEOF())
			{
				cVal = FileGetChar();
				if (cVal == m_cSectionEnd) break;
				if (cVal == m_cEndOfLine) break;
				if (FileEOF()) break;
				if (nCount < (CONTROLFILEMAXSTRINGSIZE-1))
				{
					sSectionName[nCount] = cVal;
					nCount++;
				}
			}
			sSectionName[nCount] = '\0';

			// find the section
			pCurSection = m_lstSections.Find(sSectionName);

			// if we did not find the section then make it
			if (pCurSection == NULL)
			{
				LT_MEM_TRACK_ALLOC(pCurSection = new CControlFileSection,LT_MEM_TYPE_MISC);
				pCurSection->m_sSectionName.Alloc(nCount+1);
				pCurSection->m_sSectionName.Copy(sSectionName,nCount+1);
				m_lstSections.InsertLast(pCurSection);
			}
			
			continue;
		}

		// this must be a key so read it in
		{
			CLithSimpAryStat<char,CONTROLFILEMAXSTRINGSIZE> sKeyName;

			// read in name until we see whitespace or a line or file end
			int nKeyNameCount = 0;
			while (!FileEOF())
			{
				if (nKeyNameCount < (CONTROLFILEMAXSTRINGSIZE-1))
				{
					sKeyName[nKeyNameCount] = cVal;
					nKeyNameCount++;
				}
				if (FileEOF()) break;
				cVal = FileGetChar();
				if (cVal == m_cEndOfLine) break;
				if (strchr(m_sWhiteSpace,cVal) != NULL) break;
			}
			sKeyName[nKeyNameCount] = '\0';

			// create the new key and place it in the list
			CControlFileKey* pKey;
			LT_MEM_TRACK_ALLOC(pKey = new CControlFileKey,LT_MEM_TYPE_MISC);
			LT_MEM_TRACK_ALLOC(pKey->m_sKeyName.Alloc(nKeyNameCount+1),LT_MEM_TYPE_MISC);
			pKey->m_sKeyName.Copy(sKeyName,nKeyNameCount+1);
			pCurSection->m_lstKeys.InsertLast(pKey);

			// read in all the values for this key
			while (!FileEOF())
			{
				// get char from file
				cVal = FileGetChar();	

				// check for end of file
				if (FileEOF()) break;

				// check if this char is whitespace (if so go to next char)
				if (strchr(m_sWhiteSpace,cVal) != NULL) continue;

				// chck if this is an end of line (if so go to next char)
				if (cVal == m_cEndOfLine) break;

				// check for a comment character
				if (cVal == m_cComment)
				{
					while (!FileEOF())
					{
						if (cVal == '\n') break;
						cVal = FileGetChar();
					}
					break;
				}

				CLithSimpAryStat<char,CONTROLFILEMAXSTRINGSIZE> sWord;

				// read in string until we see whitespace or a line or file end
				int nWordLenCount = 0;
				while (!FileEOF())
				{
					if (nWordLenCount < (CONTROLFILEMAXSTRINGSIZE-1))
					{
						sWord[nWordLenCount] = cVal;
						nWordLenCount++;
					}
					if (FileEOF()) break;
					cVal = FileGetChar();
					if (cVal == m_cEndOfLine) break;
					if (strchr(m_sWhiteSpace,cVal) != NULL) break;
				}
				sWord[nWordLenCount] = '\0';

				// create the new word value and place in list
				CControlFileWord* pWord;
				LT_MEM_TRACK_ALLOC(pWord = new CControlFileWord,LT_MEM_TYPE_MISC);
				LT_MEM_TRACK_ALLOC(pWord->m_sVal.Alloc(nWordLenCount+1),LT_MEM_TYPE_MISC);
				pWord->m_sVal.Copy(sWord,nWordLenCount+1);
				pKey->m_lstWords.InsertLast(pWord);
				pWord->m_nStrLen = nWordLenCount;

				// check if this is an end of line (if so we are done reading in words)
				if (cVal == m_cEndOfLine) break;
			}
		}
	}

	// close file
	FileClose();
	
	return TRUE;
}


// open a file for reading in read only text mode
BOOL CControlFileMgr::FileOpen(const char* sName)
{
	// open the file
	m_pFile = fopen(sName,"rt"); 

	// return true if it opened OK
	if (m_pFile != NULL) return TRUE;

	// otherwise return false
	else return FALSE;
}


// close file 
void CControlFileMgr::FileClose()
{
	// close the file
	fclose(m_pFile);
}

	
// get next character in file
int CControlFileMgr::FileGetChar()
{
	// get a character from the file
	return fgetc(m_pFile);
}
	

// return TRUE if we are at the end of the file
BOOL CControlFileMgr::FileEOF()
{
	// return the EOF status of the file
	return feof(m_pFile);
}

