
// The server-side loader thread.

#ifndef __SLOADERTHREAD_H__
#define __SLOADERTHREAD_H__

#ifndef __SYSLTHREAD_H__
#include "syslthread.h"
#endif

struct UsedFile;

// Thread input.
#define SLT_LOADFILE	0
	// dwData[0] = FT_ define
	// pData[1] = UsedFile*

// Thread output.
	#define SLT_LOADEDFILE	0
	#define SLT_LOADERROR	1
		// dwData[0] = FT_ define
		// pData[1] = UsedFile*
		// pData[2] = the data (like a Model*)




class SLoaderThread : public LThread
{
public:

					SLoaderThread();

	LTRESULT		WaitForFile(UsedFile *pFile, void **ppObj);

	// Returns LTTRUE if the file will be loaded or it has been loaded 
	// and is waiting for the main thread to pick up.
	bool			IsLoadingFile(UsedFile *pFile);


// In-thread.
protected:

	void			ProcessMessage(LThreadMessage &msg);
};



#endif




