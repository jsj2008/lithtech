//-------------------------------------------------------------------
// LTAFileBuffer.h
//
// Provides definition for CLTAFileBuffer which is a class that
// allows the opening of files for buffered input or output. Note
// that it can only be opened up in EITHER read or write mode, but
// not both.
//
// Created: 1/10/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTAFILEBUFFER_H__
#define __LTAFILEBUFFER_H__

//standard types
#include "ltbasedefs.h"
#include <stdio.h>

class CLTAFileBuffer
{
public:

	//default size for buffer
	enum	{ DEFAULT_BUFFER_SIZE = 1024};

	//open modes
	enum	EOpenMode{	OPEN_READ,
						OPEN_WRITE,
						OPEN_ERROR	};

	//default constructor
	CLTAFileBuffer();

	//default destructor
	~CLTAFileBuffer();

	//opens the specified file in the given mode, allocating size for a buffer of
	//the given size. Will return false if unable to open the file or allocate memory
	//for the cache
	bool Open(	const char* pszFilename, EOpenMode eMode, 
				uint32 nBufferSize = DEFAULT_BUFFER_SIZE, bool bAppend = false);

	//closes the file. This will return true if no errors were encountered
	//in the course of the file, or false if an error occurred
	bool Close();

	//determines the mode the file was opened in
	EOpenMode	GetMode() const;

	//determines if file is opened and valid to read or write to
	bool		IsValid() const;

	//reads a block of data in from the file. Will return true if the entire block could
	//be read.
	inline bool		ReadBlock(uint8* pBuffer, uint32 nBufferSize);

	//reads in a single byte of data
	inline bool		ReadByte(uint8& nData);

	//writes out a block of data
	inline bool		WriteBlock(const uint8* pBuffer, uint32 nBufferSize);

	//writes out a single byte of data
	inline bool		WriteByte(uint8 nData);


private:

	//diable accidental copying by making copy constructor private
	CLTAFileBuffer(const CLTAFileBuffer&) {}

	//allocate the cache
	bool	AllocateCache(uint32 nBufferSize);

	//clears the cache and the size variables associated with it
	void	FreeCache();

	//writes the cache out to disk in write mode
	bool	FlushCache();

	//reads the cache in in read mode
	bool	FillCache();

	//the cache
	uint8*			m_pCache;

	//the maximum size of the cache
	uint32			m_nMaxCacheSize;

	//the amount of data filled in the cache
	uint32			m_nCurrCacheSize;

	//the offset of the filepointer in the current cache
	uint32			m_nCachePos;

	//the mode the file was opened in
	EOpenMode		m_eMode;

	//the file pointer
	FILE*			m_pFile;
};

//---------------------------------------
// Inlines
//---------------------------------------
//reads a block of data in from the file. Will return true if the entire block could
//be read.
bool CLTAFileBuffer::ReadBlock(uint8* pBuffer, uint32 nBufferSize)
{
	//make sure this is okay to do
	ASSERT(m_eMode == OPEN_READ);

	//read in blocks until we can read no more
	uint32 nBytesRead = 0;

	while(nBytesRead < nBufferSize)
	{
		//figure out how much more to read
		uint32 nAmountToRead = m_nCurrCacheSize - m_nCachePos;

		//make sure we don't read too much though
		if(nAmountToRead > nBufferSize - nBytesRead)
		{
			nAmountToRead = nBufferSize - nBytesRead;
		}

		//read in that block
		memcpy(pBuffer + nBytesRead, m_pCache + m_nCachePos, nAmountToRead);

		//update the cursor
		m_nCachePos += nAmountToRead;

		//see if we need to update the buffer
		if(m_nCachePos >= m_nCurrCacheSize)
		{
			if(FillCache() == false)
			{
				//hit the end of file
				return false;
			}
		}

		//update the count read
		nBytesRead += nAmountToRead;
	}

	return true;
}

//reads in a single byte of data
bool CLTAFileBuffer::ReadByte(uint8& nData)
{
	//make sure this is okay to do
	ASSERT(m_eMode == OPEN_READ);

	//see if we need to cache the file
	if(m_nCachePos >= m_nCurrCacheSize)
	{
		//we need to fill up the cache again
		if(FillCache() == false)
		{
			//hit the end of the file
			return false;
		}
	}

	//read in the byte
	nData = m_pCache[m_nCachePos++];

	//success
	return true;
}

//writes out a block of data
bool CLTAFileBuffer::WriteBlock(const uint8* pBuffer, uint32 nBufferSize)
{
	//make sure this is okay to do
	ASSERT(m_eMode == OPEN_WRITE);

	//while there is stuff to write, write it
	uint32 nBytesWritten = 0;

	while(nBytesWritten < nBufferSize)
	{
		//find out how much to write
		uint32 nAmountToWrite = m_nMaxCacheSize - m_nCachePos;

		//make sure we don't write too much
		if(nAmountToWrite > (nBufferSize - nBytesWritten))
		{
			nAmountToWrite = nBufferSize - nBytesWritten;
		}

		//write that into the cache
		memcpy(&m_pCache[m_nCachePos], &pBuffer[nBytesWritten], nAmountToWrite);

		//update the write cursor
		m_nCachePos += nAmountToWrite;

		//see if we need to flush
		if(m_nCachePos >= m_nMaxCacheSize)
		{
			//flush
			if(FlushCache() == false)
			{
				//error, disk must be full
				return false;
			}
		}

		//update the amount written
		nBytesWritten += nAmountToWrite;
	}

	//success
	return true;
}

//writes out a single byte of data
bool CLTAFileBuffer::WriteByte(uint8 nData)
{
	//make sure this is okay to do
	ASSERT(m_eMode == OPEN_WRITE);

	//see if we need to flush the cache
	if(m_nCachePos >= m_nMaxCacheSize)
	{
		if(FlushCache() == false)
		{
			//disk is full
			return false;
		}
	}

	//write the data
	m_pCache[m_nCachePos++] = nData;

	return true;
}


#endif

