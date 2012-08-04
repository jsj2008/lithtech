//-------------------------------------------------------------------
// LTACompressedFile.h
//
// Provides definition for CLTACompressedFile which is a class that
// allows the opening of compressed files that can be used like a
// standard stream. This file uses an adaptive huffman scheme, so it
// does not need the entire file at once
//
// Created: 1/12/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTACOMPRESSEDFILE_H__
#define __LTACOMPRESSEDFILE_H__

#ifndef __LTAFILEBUFFER_H__
#	include "ltabitfile.h"
#endif

#ifndef __LZSSWINDOW_H__
#	include "lzsswindow.h"
#endif

class CLTACompressedFile
{
public:

	CLTACompressedFile();
	~CLTACompressedFile();

	//opens up a compressed file to be read from
	bool Open(	const char* pszFilename, CLTAFileBuffer::EOpenMode eMode,
				uint32 nBufferSize = CLTAFileBuffer::DEFAULT_BUFFER_SIZE, 
				bool bAppend = false);

	//closes the file. This will return true if no errors were encountered
	//in the course of the file, or false if an error occurred
	bool Close();

	//determines if this file is open and valid
	bool IsValid() const;

	//determines the mode this file was opened in
	CLTAFileBuffer::EOpenMode	GetMode() const;

	//writes out a byte to the file
	inline bool WriteByte(uint8 nByte);

	//writes out a block of the file
	bool WriteBlock(const uint8* pBlock, uint32 nBlockSize);

	//reads in a byte
	bool ReadByte(uint8& nByte);

	//reads in a block of the file
	bool ReadBlock(uint8* pBlock, uint32 nBlockSize);

private:

	//don't allow copying of this object
	CLTACompressedFile(const CLTACompressedFile& rhs)	{}

	//the bit stream we will be using
	CLTABitFile			m_BitFile;

	//the window for LZSS compression
	CLZSSWindow			m_CompWindow;

	//the window that will be used for decompression
	uint8				m_DecWnd[WINDOW_SIZE];

	//the current pos in the decompression window for writing new bytes
	uint32				m_nDecWndPos;

	//the lenght of the span left before another needs to be loaded
	uint32				m_nDecSpanLen;
	
	//the base offset of the span
	uint32				m_nDecSpanPos;

};

//---------------------------------
// Inlines


//writes out a byte to the file
bool CLTACompressedFile::WriteByte(uint8 nByte)
{
	ASSERT(GetMode() == CLTAFileBuffer::OPEN_WRITE);

	return m_CompWindow.AddByte(nByte, m_BitFile);
}

#endif
