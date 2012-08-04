//-------------------------------------------------------------------
// LTAWriter.h
//
// Provides definition for CLTAWriter, which is used to write LTA
// files. This automatically handles formatting and writing to
// the file.
//
// Created: 1/17/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTAWRITER_H__
#define __LTAWRITER_H__

#ifndef __LTAFILE_H__
#	include "ltafile.h"
#endif

class CLTAWriter
{
public:

	CLTAWriter();
	CLTAWriter(const char* pszFilename, bool bCompress, bool bAppend = false);

	~CLTAWriter();

	//determines if this object is valid for writing
	bool IsValid() const;

	//open up the specified file for writing
	bool Open(const char* pszFilename, bool bCompress, bool bAppend = false);

	//close the currently open file
	bool Close();

	//begin a new node
	bool BeginNode();

	//end a node
	bool EndNode();

	//write out strings. The second with the passed in length
	//is for optimizations so strlen need not be called if it
	//can already be determined ahead of time. 
	bool Write(const char* pszString, bool bAddQuotes=false);
	bool Write(const char* pszString, uint32 nLength, bool bAddQuotes=false);

	bool WriteF(bool bAddQuotes, const char* pszString, ...);

	//insert a carriage return and indent to the same level
	bool BreakLine();

private:

	//file for writing
	CLTAFile	m_File;

	//the current depth
	uint32		m_nNodeDepth;

	//---------------
	//formatting vars

	//flag indicating if the next value used needs a tab to be printed.
	//this will be reset after the value is written
	bool		m_bPrintTab;

	//determines if the next list printed needs to move down to another line
	bool		m_bPrintNewline;
};

#endif

