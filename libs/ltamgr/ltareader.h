//-------------------------------------------------------------------
// LTAReader.h
//
// Provides the definition for CLTAReader which opens up a file
// and allows the tokenization of that file.
//
// Created: 1/17/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTAREADER_H__
#define __LTAREADER_H__

#ifndef __LTAFILE_H__
#	include "ltafile.h"
#endif

class CLTAReader
{
public:

	//the different types of tokens that can be pulled out of the stream
	enum	ETokenType{	TK_ERROR,			//error getting next token
						TK_BEGINNODE,		//a (
						TK_ENDNODE,			//a )
						TK_VALUE,			//a value without quotes
						TK_STRING			//a value with quotes
					};

	CLTAReader();
	~CLTAReader();

	//open up the specified file for reading
	bool Open(const char* pszFilename, bool bCompressed);

	//closes the currently open file
	void Close();

	//determines if the file is valid for reading
	bool IsValid() const;

	//reads the next token from the file. Assumes that the buffer is not
	//NULL and nBufferLen > 0
	ETokenType	NextToken(char* pszValueBuffer, uint32 nBufferLen);


private:

	//used to carry over characters to prevent them from being lost
	//when tokens butt up against each other
	uint8		m_nPeekChar;

	CLTAFile	m_File;

};

#endif

