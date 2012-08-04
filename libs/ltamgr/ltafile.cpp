#include "ltafile.h"



//default constructor
CLTAFile::CLTAFile() :
	m_bCompressed(false)
{
}

//constructor that opens up a file
CLTAFile::CLTAFile(const char* pszFilename, bool bRead, bool bCompressed, bool bAppend) :
	m_bCompressed(false)
{
	//open the file
	Open(pszFilename, bRead, bCompressed, bAppend);
}


//destructor
CLTAFile::~CLTAFile()
{
	Close();
}


//open up a specific file, specifying the mode whether it be read (true)
//or write (false), and whether or not the file is compressed
bool CLTAFile::Open(const char* pszFilename, bool bRead, bool bCompressed, bool bAppend)
{
	//save the compressed scheme
	m_bCompressed = bCompressed;

	//get the open type
	CLTAFileBuffer::EOpenMode eMode = (bRead) ? CLTAFileBuffer::OPEN_READ : 
												CLTAFileBuffer::OPEN_WRITE;

	if(m_bCompressed)
	{
		//try and open it
		return m_CompressedFile.Open(pszFilename, eMode, CLTAFileBuffer::DEFAULT_BUFFER_SIZE, bAppend);
	}
	else
	{
		return m_FileBuffer.Open(pszFilename, eMode, CLTAFileBuffer::DEFAULT_BUFFER_SIZE, bAppend);
	}

	//what?
	return false;
}


//closes the file. This will return true if no errors were encountered
//in the course of the file, or false if an error occurred
bool CLTAFile::Close()
{
	if(m_bCompressed)
	{
		return m_CompressedFile.Close();
	}
	else
	{
		return m_FileBuffer.Close();
	}
}


//determine if the file is open and valid for use
bool CLTAFile::IsValid() const
{
	if(m_bCompressed)
	{
		return m_CompressedFile.IsValid();
	}
	else
	{
		return m_FileBuffer.IsValid();
	}
}



