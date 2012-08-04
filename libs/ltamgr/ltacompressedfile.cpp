#include "ltacompressedfile.h"

//the current version of the file
#define CURR_FILE_VERSION		0

CLTACompressedFile::CLTACompressedFile() :
	m_nDecSpanLen(0),
	m_nDecWndPos(0)
{
}


CLTACompressedFile::~CLTACompressedFile()
{
	Close();
}

//opens up a compressed file to be read from
bool CLTACompressedFile::Open(const char* pszFilename, CLTAFileBuffer::EOpenMode eMode, 
							  uint32 nBufferSize, bool bAppend)
{
	if(m_BitFile.Open(pszFilename, eMode, nBufferSize, bAppend) == false)
	{
		return false;
	}

	//if we are writing, we need to init the window
	if(eMode == CLTAFileBuffer::OPEN_WRITE)
	{
		//write out the file version
		uint32 nSize = (sizeof(uint32) * 8);
		for(uint32 nCurrBit = 1; nCurrBit <= nSize; nCurrBit++)
		{
			m_BitFile.SetBit(((uint32)CURR_FILE_VERSION & (1 << (nSize - nCurrBit))) ? 1 : 0);
		}
		
		m_CompWindow.Init();
	}
	else
	{
		//need to read in the version
		uint32 nCurrVersion = 0;
		for(uint32 nCurrBit = 0; nCurrBit < (sizeof(uint32) * 8); nCurrBit++)
		{
			uint8 nVal;
			m_BitFile.GetBit(nVal);
			nCurrVersion <<= 1;
			if(nVal)
			{
				nCurrVersion |= (1 << nCurrBit);
			}
		}

		//make sure that the version is valid
		if(nCurrVersion != CURR_FILE_VERSION)
		{
			Close();
			return false;
		}

		m_nDecSpanLen = 0;
		m_nDecWndPos  = 1;
	}

	return true;
}

//closes the file. This will return true if no errors were encountered
//in the course of the file, or false if an error occurred
bool CLTACompressedFile::Close()
{
	//see if we need to flush out the look ahead
	if(GetMode() == CLTAFileBuffer::OPEN_WRITE)
	{
		m_CompWindow.FlushLookAhead(m_BitFile);
	}

	return m_BitFile.Close();
}

//determines if this file is open and valid
bool CLTACompressedFile::IsValid() const
{
	return m_BitFile.IsValid();
}

//determines the mode this file was opened in
CLTAFileBuffer::EOpenMode CLTACompressedFile::GetMode() const
{
	return m_BitFile.GetMode();
}

//writes out a block of the file
bool CLTACompressedFile::WriteBlock(const uint8* pBlock, uint32 nBlockSize)
{
	bool bRV = true;

	for(uint32 nCurrByte = 0; nCurrByte < nBlockSize; nCurrByte++)
	{
		bRV = WriteByte(pBlock[nCurrByte]) && bRV;
	}

	return bRV;
}

//reads in a byte
bool CLTACompressedFile::ReadByte(uint8& nByte)
{
	//sanity check
	ASSERT(GetMode() == CLTAFileBuffer::OPEN_READ);

	//see if we are at the end of a span
	if(m_nDecSpanLen == 0)
	{
		//we are, so we need to load in a new span
		
		//see if it is raw data or a span
		uint8 nType;
		
		if(m_BitFile.GetBit(nType) == false)
		{
			//end of input stream
			return false;
		}

		if(nType == 0)
		{
			//we have a span
			uint8 nBit;
			uint32 nCurrBit;

			//read in the offset
			m_nDecSpanPos = 0; 

			for(nCurrBit = 0; nCurrBit < NUM_OFFSET_BITS; nCurrBit++)
			{
				m_nDecSpanPos <<= 1;
				m_BitFile.GetBit(nBit);
				if(nBit)
				{
					m_nDecSpanPos |= 1;
				}
			}
			
			//see if this is the special end of stream token
			if(m_nDecSpanPos == 0)
			{
				return false;
			}

			//now read in the span length
			m_nDecSpanLen = 0;

			for(nCurrBit = 0; nCurrBit < NUM_LENGTH_BITS; nCurrBit++)
			{
				m_nDecSpanLen <<= 1;
				m_BitFile.GetBit(nBit);
				if(nBit)
				{
					m_nDecSpanLen |= 1;
				}
			}

			//adjust for the values that aren't possible
			m_nDecSpanLen += (BREAK_EVEN_POINT + 1);
		}
		else
		{
			//we have a single character, read in the value
			nByte			= 0;
			uint8 nBit		= 0;

			for(uint32 nCurrBit = 0; nCurrBit < 8; nCurrBit++)
			{
				nByte <<= 1;

				m_BitFile.GetBit(nBit);
				if(nBit)
				{
					nByte |= 1;
				}
			}

			//add this to the window
			m_DecWnd[m_nDecWndPos] = nByte;

			//adjust the position in the window
			m_nDecWndPos = WINDOW_POS(m_nDecWndPos + 1);

			//return the value
			return true;
		}
	}

	//so we should have a span now
	ASSERT(m_nDecSpanLen > 0);

	//so, lets just get that value, and adjust the window accordingly
	nByte = m_DecWnd[m_nDecWndPos] = m_DecWnd[m_nDecSpanPos];

	//adjust the window write position, and the span read position
	m_nDecSpanPos	= WINDOW_POS(m_nDecSpanPos + 1);
	m_nDecWndPos	= WINDOW_POS(m_nDecWndPos + 1);

	//decrement the length of the span since we just took a character from it
	m_nDecSpanLen--;

	return true;
}

//reads in a block of the file
bool CLTACompressedFile::ReadBlock(uint8* pBlock, uint32 nBlockSize)
{
	bool bRV = true;

	for(uint32 nCurrByte = 0; nCurrByte < nBlockSize; nCurrByte++)
	{
		bRV = ReadByte(pBlock[nCurrByte]) && bRV;
	}

	return bRV;
}