//------------------------------------------------------------------
//
//	FILE	  : MemoryIO.h
//
//	PURPOSE	  : Defines the CMemoryIO class.
//
//	CREATED	  : July 25 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __MEMORYIO_H__
	#define __MEMORYIO_H__


	// Includes....
	#include "AbstractIO.h"
	#include "DynArray.h"


	class CMemoryIO : public CAbstractIO
	{
		public:

			// Constructor
								CMemoryIO();

			
			// Member functions

			BOOL				Open( const char *pFilename, const char *pAccess )	{ return TRUE; }
			void				Close() {}

			BOOL				Write( void *pBlock, DWORD blockSize );
			BOOL				Read( void *pBlock, DWORD blockSize );

			DWORD				GetCurPos();
			DWORD				GetLen();

			BOOL				SeekTo( DWORD pos );

			// New functions...
			void				SetCacheSize( WORD size )	{ m_Data.SetCacheSize(size); }
			
			BOOL				SetDataSize( DWORD size )	{ return m_Data.SetSize( size ); }
			void				*GetData()					{ return m_Data.GetArray(); }
			
			void				Clear();


		public:

			BOOL				m_bRanOutOfMemory;
	
		
		private:
			
			// Private member variables
			CMoByteArray		m_Data;
			DWORD				m_Pos;

	};


#endif


