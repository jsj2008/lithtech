//-------------------------------------------------------------------
// LTABitFile.h
//
// Provides definition for CLTABitFile which is a class that
// allows the opening of files for manipulation on the bit level. 
// this is constructed on top of CLTAFileBuffer, so for more 
// information, see that class. 
//
// Created: 1/10/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTABITFILE_H__
#define __LTABITFILE_H__


//include the basis of this class
#ifndef __LTAFILEBUFFER_H__
#	include "ltafilebuffer.h"
#endif

class CLTABitFile
{
public:

	//default constructor
	CLTABitFile();

	//default destructor
	~CLTABitFile();

	//opens up a file for bit input
	bool Open(	const char* pszFilename, CLTAFileBuffer::EOpenMode eMode,
				uint32 nBufferSize = CLTAFileBuffer::DEFAULT_BUFFER_SIZE, bool bAppend = false);

	//determines if this bitfile is open and valid
	bool IsValid() const;

	//gets the mode this file was opened in
	CLTAFileBuffer::EOpenMode	GetMode() const;

	//closes the currently open file
	bool Close();

	//extracts a bit. Note: The value should only be treated as a boolean, not
	//as a literal. There is only the gurantee if the next bit was set, that the
	//nVal will not be 0. If it returns false, the value for nVal is undefined
	inline bool GetBit(uint8& nVal);

	//sets a bit
	inline bool SetBit(uint8 nVal);

private:

	//don't allow copying of these objects
	CLTABitFile(const CLTABitFile&) {}

	//internal file
	CLTAFileBuffer		m_File;

	//the current byte of data
	uint8				m_nCurrData;

	//the current mask
	uint8				m_nMask;
};


//----------------------------------------
// Inlines
//----------------------------------------

//extracts a bit. Note: The value should only be treated as a boolean, not
//as a literal. There is only the gurantee if the next bit was set, that the
//nVal will not be 0. If it returns false, the value for nVal is undefined
bool CLTABitFile::GetBit(uint8& nVal)
{
	//make sure we are in the right mode
	ASSERT(GetMode() == CLTAFileBuffer::OPEN_READ);

	//see if we need to get new data
	if(m_nMask == 0x00)
	{
		if(m_File.ReadByte(m_nCurrData) == false)
		{
			return false;
		}
		m_nMask = 0x01;
	}

	//set the value
	nVal = m_nCurrData & m_nMask;

	//update the mask
	m_nMask <<= 1;	

	return true;
}

//sets a bit
bool CLTABitFile::SetBit(uint8 nVal)
{
	//make sure this is safe
	ASSERT(GetMode() == CLTAFileBuffer::OPEN_WRITE);

	//modify the data
	if(nVal)
	{
		m_nCurrData |= m_nMask;
	}
	
	//see if we went over
	if(m_nMask == 0x80)
	{
		//write out the data
		if(m_File.WriteByte(m_nCurrData) == false)
		{
			return false;
		}

		//reset the data and mask
		m_nCurrData = 0;
		m_nMask		= 0x01;
	}
	else
	{
		//adjust the mask
		m_nMask <<= 1;
	}


	return true;	
}


#endif