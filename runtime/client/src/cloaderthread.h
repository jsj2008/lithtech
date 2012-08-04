#ifndef __CLOADERTHREAD_H__
#define __CLOADERTHREAD_H__

#ifndef __SYSLTHREAD_H__
#include "syslthread.h"
#endif

struct FileIdentifier;

// Messages in the loader thread.
// m_Data[0].m_dwData is a FT_ define from de_codes.h
// m_Data[1].m_pData is the FileIdentifier*.
// m_Data[2].m_dwData is a TEXTURELOAD_ define.
#define CLT_LOADFILE	0

// Notification that a resource is done loading.
#define CLT_LOADEDFILE	0
#define CLT_LOADERROR	1


class CClientMgr;


class CLoaderThread : public LThread
{
// Out of thread.
public:

					CLoaderThread();

	// Are we loading this file or is it in our queue to load?
	bool			IsLoadingFile(FileIdentifier *pIdent);

	// Wait for the specified file to complete loading.
	LTRESULT		WaitForFile(FileIdentifier *pIdent, void **pObj);


// In-thread.
protected:

	virtual void	ProcessMessage(LThreadMessage &msg);

	// After a model is loaded, this does the work of integrating it into the client
	// (add to the models list, update placeholder models, etc).
	void			LoadModel(FileIdentifier *pIdent);


};


#endif




