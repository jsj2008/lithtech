/****************************************************************************

	MODULE:		ControlFileMgr (.H)

	PURPOSE:	Class for reading and parsing control files (like ini's)

	HISTORY:	
				Nov-5-1999  : Added new GetVal and GetKeyVal functions for 
							  floats and doubles.Added new file access 
							  functions that user can override to do their
							  own type of file access.  This is necessary
							  to use a control file from a rez file. Added
							  even more comments to code. BLB

				Sep-22-1999 : Added special commands starting with # (ifdef, 
							  ifndef, else, endif) they work pretty much like
							  c source files.  Also added more comments to
							  header file. BLB

				04/19/98 [blb]
				

	NOTICE:		Copyright (c) 1998, 1999 Monolith Productions, Inc.

***************************************************************************/

#ifndef __CONTROLFILEMGR_H__
#define __CONTROLFILEMGR_H__

#ifndef __LITH_H__
#include "lith.h"
#endif

#ifndef __STDIO_H__
#include "stdio.h"
#endif


//**************************************************************************
class CControlFileWord : public CLithBaseListItem<CControlFileWord>
{
public:
	BOOL GetVal(char* sVal, int nMaxStrLen);
	int GetStrLen();
	BOOL GetVal(int& nVal);
	BOOL GetVal(float& nVal);
	BOOL GetVal(double& nVal);
	char* GetVal() { return m_sVal; };
private:
	friend class CControlFileMgr;
	CLithSimpAry<char> m_sVal;
	int m_nStrLen;
};


//**************************************************************************
class CControlFileWordList : public CLithBaseList<CControlFileWord> {};


//**************************************************************************
class CControlFileKey : public CLithBaseListItem<CControlFileKey>
{
public:
	CControlFileWord* GetFirstWord();
	CControlFileKey* NextWithSameName();
	char* GetKeyName() { return m_sKeyName; };
private:
	friend class CControlFileKeyList;
	friend class CControlFileMgr;
	CLithSimpAry<char> m_sKeyName;
	CControlFileWordList m_lstWords;
};


//**************************************************************************
class CControlFileKeyList : public CLithBaseList<CControlFileKey> 
{
public:
	CControlFileKey* Find(const char* sKeyName);
};


//**************************************************************************
class CControlFileSection : public CLithBaseListItem<CControlFileSection>
{
public:
	CControlFileKey* GetKey(const char* sKeyName = NULL); // NULL gets the first key in the list of keys
	char* GetSectionName() { return m_sSectionName; };
private:
	friend class CControlFileSectionList;
	friend class CControlFileMgr;
	CLithSimpAry<char> m_sSectionName;
	CControlFileKeyList m_lstKeys;
};


//**************************************************************************
class CControlFileSectionList : public CLithBaseList<CControlFileSection> 
{
public:
	CControlFileSection* Find(const char* sSectionName);
};


//**************************************************************************
class CControlFileDefine : public CLithBaseListItem<CControlFileDefine>
{
public:
	char* GetVal() { return m_sVal; };
private:
	friend class CControlFileMgr;
	CLithSimpAry<char> m_sVal;
};


//**************************************************************************
class CControlFileDefineList : public CLithBaseList<CControlFileDefine> {};


//**************************************************************************
class CControlFileMgr
{
public:

	// default constructor
	CControlFileMgr();

	// constructor that also calls Init
	CControlFileMgr(const char* sFileName, const char* sWhiteSpace = NULL);

	// destructor (calls Term if it has not already been called
	~CControlFileMgr();

	// Initialize the Mgr and read in the file
	BOOL Init(const char* sFileName, const char* sWhiteSpace = NULL);

	// Terminate the Mgr
	void Term();

	// gets a point to a section of the control file
	CControlFileSection* GetSection(const char* sSectionName = NULL); // NULL gets the first (empty name) section

	// finds the first occurance of the specified key in the specified section
	CControlFileKey* GetKey(const char* sSectionName, const char* sKeyName);

	// gets the integer value of the specified key in the specified section and places it in nVal (first word only)
	BOOL GetKeyVal(const char* sSectionName, const char* sKeyName, int& nVal);

	// gets the integer value of the specified key in the specified section and places it in nVal (first word only)
	BOOL GetKeyVal(const char* sSectionName, const char* sKeyName, float& nVal);

	// gets the integer value of the specified key in the specified section and places it in nVal (first word only)
	BOOL GetKeyVal(const char* sSectionName, const char* sKeyName, double& nVal);

	// gets the string value of the specified key in the specified section and places it in sVal (first word only)
	BOOL GetKeyVal(const char* sSectionName, const char* sKeyName, char* sVal, int nMaxStrLen);

	// find out the string length of the first word in the specified key in the specified section
	int GetKeyValStrLen(const char* sSectionName, const char* sKeyName);

	// add a define that can be checked with #ifdef or #ifndef special commands
	// this must be done before init is called!
	BOOL AddDefine(const char* sDefine);

	// clears all defines that have been made with AddDefine
	void ClearDefines();

	/////////////////////////////////////////////////////////////////////////////////////////////
	// File access functions that the user can override to implement their own file access.
	//
	// These only get used during a call to Init and everything takes place in that one
	// call so you can assume that the file will be opened read and closed when you make a call
	// to Init.
	/////////////////////////////////////////////////////////////////////////////////////////////

	// open a file for reading in read only text mode
	virtual BOOL FileOpen(const char* sName);

	// close file 
	virtual void FileClose();
	
	// get next character in file
	virtual int FileGetChar();
	
	// return TRUE if we are at the end of the file
	virtual BOOL FileEOF();

protected:
	// used by file access functions for the default file implementation
	FILE* m_pFile;

private:
	// returns true if the given string has been passed into AddDefine
	BOOL IsDefined(const char* sDefine);

	// reads in and processes the specified file
	BOOL ProcessFile(const char* sFileName);

	// name of the control file that we are using
	CLithSimpAry<char> m_sFileName;

	// contains all of the characters that are defined as whitespace
	CLithSimpAry<char> m_sWhiteSpace;

	// contains the various special characters
	char m_cEndOfLine;
	char m_cSectionStart;
	char m_cSectionEnd;
	char m_cComment;
	char m_cStringMarker;
	char m_cSpecialCommand;

	// list of all the sections in the control file data structure
	CControlFileSectionList m_lstSections;

	// contains a list of all of the defines that have been created with AddDefine
	CControlFileDefineList m_lstDefines;

	// if true the mgr has been initialized
	BOOL m_bInitialized;
};

#endif