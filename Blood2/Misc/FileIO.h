//------------------------------------------------------------------
//
//	FILE	  : FileIO.h
//
//	PURPOSE	  : Defines the CMoFileIO class.
//
//	CREATED	  : 1st May 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __FILEIO_H__
	#define __FILEIO_H__


	// Includes....
	#include "AbstractIO.h"


	class CMoFileIO : public CAbstractIO
	{
		public:

			// Constructor
								CMoFileIO();
								~CMoFileIO();

			
			// Member functions

			BOOL				Open( const char *pFilename, const char *pAccess );
			void				Close();

			BOOL				IsOpen();
			
			void				SetBoundaries( DWORD min, DWORD max );

			BOOL				Write( void *pBlock, DWORD blockSize );
			BOOL				Read( void *pBlock, DWORD blockSize );

			DWORD				GetCurPos();
			DWORD				GetLen();

			BOOL				SeekTo( DWORD pos );
	
		
		public:

			// Private member functions


		public:
			
			// Private member variables
			FILE				*m_pFile;
			
			// File boundaries...
			DWORD				m_FileMin, m_FileMax;
			
			DWORD				m_FileLen;

	};


#endif


