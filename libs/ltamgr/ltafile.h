//-------------------------------------------------------------------
// LTAFile.h
//
// Provides definition for CLTAFile class which provides an abstract
// means of manipulating files for the LTA library. It can handle
// either standard files or compressed.
//
// Created: 1/12/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTAFILE_H__
#define __LTAFILE_H__

//base types
#include "ltbasedefs.h"


#ifndef __LTACOMPRESSEDFILE_H__
#	include "ltacompressedfile.h"
#endif


class CLTAFile
{
public:

	//default constructor
	CLTAFile();

	//constructor that opens up a file
	CLTAFile(const char* pszFilename, bool bRead, bool bCompressed, bool bAppend = false);

	//destructor
	~CLTAFile();

	//open up a specific file, specifying the mode whether it be read (true)
	//or write (false), and whether or not the file is compressed
	bool Open(const char* pszFilename, bool bRead, bool bCompressed, bool bAppend = false);

	//closes the file. This will return true if no errors were encountered
	//in the course of the file, or false if an error occurred
	bool Close();

	//determine if the file is open and valid for use
	bool IsValid() const;

	//read in a block of data of the specified number of bytes
	inline bool Read(uint8* pBuffer, uint32 nBufferLen);

	//read in a single byte
	inline bool ReadByte(uint8& nByte);

	//write out a block of memory of the specified number of bytes
	inline bool Write(const uint8* pBuffer, uint32 nBufferLen);

	//write out a single byte
	inline bool WriteByte(uint8 nByte);

	//writes out a string to the file
	inline bool WriteStr(const char* pszString);

	//write out a string that can be formatted
	inline bool	WriteStrF(const char* pszString, ...);
	inline bool WriteStrFA(const char* pszString, va_list args);


private:

	//don't allow copying of this object
	CLTAFile(const CLTAFile&) {}

	//determine if this the file is compresed or not
	bool					m_bCompressed;

	//the different files
	CLTACompressedFile		m_CompressedFile;
	CLTAFileBuffer			m_FileBuffer;

};


//--------------------------------------
// Inlines
//--------------------------------------
#include <stdio.h>
#include <stdarg.h>

//read in a block of data of the specified number of bytes
bool CLTAFile::Read(uint8* pBuffer, uint32 nBufferLen)
{
	if(m_bCompressed)
	{
		return m_CompressedFile.ReadBlock(pBuffer, nBufferLen);
	}
	else
	{
		return m_FileBuffer.ReadBlock(pBuffer, nBufferLen);
	}
}


//read in a single byte
bool CLTAFile::ReadByte(uint8& nByte)
{
	if(m_bCompressed)
	{
		return m_CompressedFile.ReadByte(nByte);
	}
	else
	{
		return m_FileBuffer.ReadByte(nByte);
	}
}


//write out a block of memory of the specified number of bytes
bool CLTAFile::Write(const uint8* pBuffer, uint32 nBufferLen)
{
	if(m_bCompressed)
	{
		return m_CompressedFile.WriteBlock(pBuffer, nBufferLen);
	}
	else
	{
		return m_FileBuffer.WriteBlock(pBuffer, nBufferLen);
	}
}


//write out a single byte
bool CLTAFile::WriteByte(uint8 nByte)
{
	if(m_bCompressed)
	{
		return m_CompressedFile.WriteByte(nByte);
	}
	else
	{
		return m_FileBuffer.WriteByte(nByte);
	}
}

bool CLTAFile::WriteStr(const char* pszString)
{
	//now we write out the buffer
	return Write((uint8*)pszString, strlen(pszString));
}

bool CLTAFile::WriteStrF(const char* pszString, ...)
{
	//the parameter list
	va_list vlParams;
	va_start(vlParams, pszString);

	bool bRV = WriteStrFA(pszString, vlParams);

	va_end(vlParams);

	return bRV;
}

bool CLTAFile::WriteStrFA(const char* pszString, va_list args)
{
	//a buffer to hold the formatted data in
	static char pszBuffer[2048];

	//start the list
	uint32 nStrLen = vsprintf(pszBuffer, pszString, args);

	//now we write out the buffer
	return Write((uint8*)pszBuffer, nStrLen);
}

#endif

