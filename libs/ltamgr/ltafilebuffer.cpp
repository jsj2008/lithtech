#include "ltafilebuffer.h"
#include <stdio.h>

//default constructor
CLTAFileBuffer::CLTAFileBuffer() :
	m_pFile(NULL),
	m_nMaxCacheSize(0),
	m_pCache(NULL),
	m_nCurrCacheSize(0),
	m_nCachePos(0),
	m_eMode(OPEN_ERROR)
{
}

//default destructor
CLTAFileBuffer::~CLTAFileBuffer()
{
	Close();
}

//opens the specified file in the given mode, allocating size for a buffer of
//the given size. Will return false if unable to open the file or allocate memory
//for the cache
bool CLTAFileBuffer::Open(const char* pszFilename, EOpenMode eMode, uint32 nBufferSize, bool bAppend)
{
	//close out any existing file
	Close();

	//make sure that the open mode is valid
	if(eMode == OPEN_ERROR)
	{
		//assert, this is a code bug
		ASSERT(false);
		return false;
	}

	//make sure that they aren't opening it up for reading with append. That just
	//doesn't make sense
	if((eMode == OPEN_READ) && bAppend)
	{
		//assert, this is a code bug
		ASSERT(false);
		return false;
	}

	//attempt to open up their file
	m_pFile = fopen(pszFilename, (eMode == OPEN_READ) ? "rb" : (bAppend ? "ab+" : "wb"));

	//see if this file was able to open
	if(m_pFile == NULL)
	{
		return false;
	}

	//file opened okay, so lets try the cache
	if(AllocateCache(nBufferSize) == false)
	{
		//failed. Low memory conditions, or programmer error (bad number)
		ASSERT(false);

		//return to an invalid state
		fclose(m_pFile);
		m_pFile = NULL;

		return false;
	}

	//reset the cursor positions
	m_nCurrCacheSize	= 0;
	m_nCachePos			= 0;


	//save the open mode
	m_eMode = eMode;

	//need to init the cache if reading
	if(m_eMode == OPEN_READ)
	{
		FillCache();
	}

	//success
	return true;
}


//closes the file. This will return true if no errors were encountered
//in the course of the file, or false if an error occurred
bool CLTAFileBuffer::Close()
{
	//if this is a write file, we need to make sure that the whole output
	//buffer got written
	if(GetMode() == OPEN_WRITE)
	{
		FlushCache();
	}

	//clean up the cache
	FreeCache();

	//determine if an error occurred
	bool bRV = true; 

	//close the file
	if(m_pFile)
	{
		bRV = ferror(m_pFile) ? false : true;
		fclose(m_pFile);
		m_pFile = NULL;
	}

	//set the mode to invalid
	m_eMode = OPEN_ERROR;

	return bRV;
}

//determines the mode the file was opened in
CLTAFileBuffer::EOpenMode CLTAFileBuffer::GetMode() const
{
	return m_eMode;
}

//determines if file is opened and valid to read or write to
bool CLTAFileBuffer::IsValid() const
{
	return (m_eMode != OPEN_ERROR) ? true : false;
}



//allocate the cache
bool CLTAFileBuffer::AllocateCache(uint32 nBufferSize)
{
	//make sure that the old cache is free
	FreeCache();

	//allocate a new one
	LT_MEM_TRACK_ALLOC(m_pCache = new uint8[nBufferSize],LT_MEM_TYPE_MISC);

	//see if it worked
	if(m_pCache == NULL)
	{
		//low memory conditions or programmer error
		ASSERT(false);
		return false;
	}

	//setup the size information
	m_nMaxCacheSize		= nBufferSize;
	m_nCurrCacheSize	= 0;
	m_nCachePos			= 0;

	//success
	return true;
}

//clears the cache and the size variables associated with it
void CLTAFileBuffer::FreeCache()
{
	//clear the memory
	delete [] m_pCache;
	m_pCache = NULL;

	//now the sizes
	m_nMaxCacheSize		= 0;
	m_nCurrCacheSize	= 0;
	m_nCachePos			= 0;

}


//writes the cache out to disk in write mode
bool CLTAFileBuffer::FlushCache()
{
	//make sure that we are okay to do this
	ASSERT(m_eMode == OPEN_WRITE);

	//write out the cache to disk
	uint32 nWritten = fwrite(m_pCache, sizeof(uint8), m_nCachePos, m_pFile);

	//determine success
	bool bSuccess = (nWritten == m_nCachePos) ? true : false;

	//reset the cursor
	m_nCachePos = 0;

	//return success code
	return bSuccess;
}

//reads the cache in in read mode
bool CLTAFileBuffer::FillCache()
{
	//make sure that we are okay to do this
	ASSERT(m_eMode == OPEN_READ);

	//read in the new data, and keep track of the current cache size
	m_nCurrCacheSize = fread(m_pCache, sizeof(uint8), m_nMaxCacheSize, m_pFile);

	//reset the cursor
	m_nCachePos = 0;

	//if anything was read in, it was a success
	return (m_nCurrCacheSize > 0) ? true : false;
}