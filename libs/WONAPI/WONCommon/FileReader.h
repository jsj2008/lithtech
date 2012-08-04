#ifndef __WON_FILEREADER_H__
#define __WON_FILEREADER_H__
#include "WONShared.h"

#include <stdio.h>
#include <exception>
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class FileReader
{
private:
	FILE *mFile;
	const char *mMemDataP;
	unsigned long mMemDataLen;
	unsigned long mMemDataPos;
	bool mOwnMemData;

	void Init();

public:
	FileReader();
	FileReader(const char *theFilePath);
	FileReader(const void *theMemDataP, unsigned long theMemDataLen);

	~FileReader();

	bool Open(const char *theFilePath);
	void Open(const void *theMemDataP, unsigned long theMemDataLen);
	void Close();

	int ReadMaxBytes(void *theBuf, int theMaxBytes);
	void ReadBytes(void *theBuf, unsigned long theNumBytes);
	unsigned char ReadByte();
	unsigned short ReadShort();
	unsigned long ReadLong();

	unsigned long pos();

	void SkipBytes(unsigned long theNumBytes);
	void SetPos(unsigned long thePos);
	void ReadIntoMemory();

	const char* GetMemData() { return mMemDataP; }
	unsigned long GetMemDataLen() { return mMemDataLen; }
	void SetOwnMemData(bool own) { mOwnMemData = own; }

	FILE* GetFileHandle() { return  mFile; }
	bool IsOpen() { return mFile!=NULL || mMemDataP!=NULL; }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class FileReaderException : public std::exception
{
public:
	std::string what;
	FileReaderException() : exception() { }
	FileReaderException(const std::string &theWhat) { what = theWhat; }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct AutoFile 
{
	FILE *mFile;
	AutoFile(FILE *theFile) : mFile(theFile) {}
	~AutoFile() { fclose(mFile); }
};

}; // namespace WONAPI

#endif