//------------------------------------------------------------------
//
//	FILE	  : ArchiveIO.h
//
//	PURPOSE	  : Defines the CMoArchiveIO class -- based on MFC's CArchive.
//
//	CREATED	  : December 11, 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __ARCHIVEIO_H__
	#define __ARCHIVEIO_H__


	#include "stdafx.h"
	#include "AbstractIO.h"


	class CMoArchiveIO : public CAbstractIO
	{
		public:

			// Member functions

							CMoArchiveIO();
							CMoArchiveIO( CArchive *pArchive );

			void			SetArchive( CArchive *pArchive );

			// Functions to toWrite data
			BOOL			Write( void *pBlock, DWORD blockSize );
			BOOL			Read( void *pBlock, DWORD blockSize );

			DWORD			GetCurPos();
			DWORD			GetLen();

			BOOL			SeekTo( DWORD pos );

		
		public:

			CArchive		*m_pArchive;

	};


#endif


