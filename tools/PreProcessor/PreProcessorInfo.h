//------------------------------------------------------------------
//
//	FILE	  : PreProcessorInfo.h
//
//	PURPOSE	  : The CPreProcessorInfo class just holds lots of
//              data to be sent in to the preprocessor thread.
//
//	CREATED	  : October 11 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __PREPROCESSORINFO_H__
	#define __PREPROCESSORINFO_H__


	#include "preworld.h"

	
	class CPreProcessorView;
	class CPreProcessorDoc;
	class CPreProcessorThread;

	
	class CPreProcessorInfo
	{
		public:
	
									CPreProcessorInfo()
									{
										m_nThreads = 0;
									}
			
			CPreWorldPtrArray		m_Worlds;
			CString					m_Filename;
			
			BOOL					m_bMakeBsp;
			BOOL					m_bMakeHulls;
			BOOL					m_bMakeVisList;
			DWORD					m_nThreads;

			// The thread sets this to NULL when it's done.
			CPreProcessorThread		**m_ppThread;
			
			// The window it puts text updates on.
			HWND					m_hWnd;
	
	};


#endif  // __PREPROCESSORINFO_H__

