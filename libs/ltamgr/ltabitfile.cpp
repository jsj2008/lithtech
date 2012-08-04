#include "ltabitfile.h"

//default constructor
CLTABitFile::CLTABitFile() :
	m_nCurrData(0),
	m_nMask(0x01)
{
}

//default destructor
CLTABitFile::~CLTABitFile()
{
}

//opens up a file for bit input
bool CLTABitFile::Open(const char* pszFilename, CLTAFileBuffer::EOpenMode eMode, 
					   uint32 nBufferSize, bool bAppend)
{
	//init the data
	m_nCurrData = 0;
	m_nMask		= 0x00;

	//open the file
	if(m_File.Open(pszFilename, eMode, nBufferSize, bAppend) == false)
	{
		return false;
	}

	//set the mask accordingly to the mode
	if(eMode == CLTAFileBuffer::OPEN_READ)
	{
		//set it to 0, indicating that a new byte was hit
		m_nMask = 0x00;
	}
	else
	{
		//set it to the first bit for writing
		m_nMask = 0x01;
	}

	return true;
}

//determines if this bitfile is open and valid
bool CLTABitFile::IsValid() const
{
	return m_File.IsValid();
}

//gets the mode this file was opened in
CLTAFileBuffer::EOpenMode CLTABitFile::GetMode() const
{
	return m_File.GetMode();
}

//closes the currently open file
bool CLTABitFile::Close()
{
	return m_File.Close();
}


