//------------------------------------------------------------------
//
//	FILE	  : CompressedIO.h
//
//	PURPOSE	  : Defines the CCompressedIO class, which allows you
//              to compress any IO stream.
//
//	CREATED	  : February 15 1997
//
//	COPYRIGHT : Microsoft 1997 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __COMPRESSEDIO_H__
	#define __COMPRESSEDIO_H__


	// Includes....
	#include "AbstractIO.h"
	#include "Dynarray.h"


	class CCompressedIO : public CAbstractIO
	{
		// Initialization.
		public:

								CCompressedIO();
								~CCompressedIO();

			// Compresses the input stream into the output stream.
			// Throws CLithMemException and any exception thrown by pInStream or pOutStream.
			BOOL				WriteStream( CAbstractIO *pInStream, CAbstractIO *pOutStream, DWORD chunkSize, DWORD sizeToWrite );
			
			// Read from a stream compressed with WriteStream.
			BOOL				InitRead( CAbstractIO *pStreamToUse );
			void				Term();

		
		// Normal IO routines.
		public:

			// CANNOT WRITE TO A COMPRESSEDIO.  USE WRITESTREAM.
			virtual BOOL			Write( void *pBlock, DWORD blockSize )	{ ASSERT(FALSE); return FALSE; }

			// Functions to read data
			virtual BOOL			Read( void *pBlock, DWORD blockSize );

			virtual DWORD			GetCurPos();
			virtual DWORD			GetLen();

			virtual BOOL			SeekTo( DWORD pos );


		// Internal stuff.
		protected:

			CAbstractIO&		Stream()	{ return *m_pReadStream; }

			DWORD				WriteChunk( CAbstractIO &inStream, CAbstractIO &outStream, CMoByteArray &compressed, CMoByteArray &uncompressed, DWORD size );

			void				SwitchToChunk( DWORD iChunk, DWORD offset );
			void				LoadCurChunk();


		protected:

			// Space for the compressed data while uncompressing into the uncompressed chunk.
			CMoByteArray		m_CompressedChunkSpace;

			// The current uncompressed chunk.
			CMoByteArray		m_UncompressedChunk;

			
			// Where we are in the file.
			DWORD				m_iCurChunk;
			DWORD				m_CurChunkPos;

			// Just stored data .. the current chunk's uncompressed size.
			DWORD				m_CurChunkSize;

			
			// Which chunk is loaded (-1 if none).  It won't actually load a chunk and set this
			// until you try to read from a chunk.
			DWORD				m_iCurLoadedChunk;

			

			// Number of chunks in this file.
			DWORD				m_nChunks;

			// (Uncompressed) chunk size for this file.
			DWORD				m_ChunkSize;			

			// (Uncompressed) file length.
			DWORD				m_FileLen;


			// Offsets of each chunk into the file (NOT offset by m_BaseFilePos).
			CMoDWordArray		m_ChunkOffsets;

			// Size of each (compressed) chunk in the file.
			CMoDWordArray		m_ChunkSizes;


			// Size of each uncompressed chunk in the file.
			CMoDWordArray		m_UncompressedChunkSizes;


			// Used when seeking .. this way you can start up a compressed stream in the
			// middle of a big file containing lots of little compressed streams.
			DWORD				m_BaseFilePos;


			// Stream used for reading.
			CAbstractIO			*m_pReadStream;

	};


#endif  // __COMPRESSEDIO_H__

