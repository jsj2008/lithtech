#ifndef __SERVER_LOADERTHREAD_H__
#define __SERVER_LOADERTHREAD_H__

//Module that allocates and gives access to the "server loader thread"


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

class SLoaderThread;


class IServerLoaderThread : public IBase {
public:
    interface_version(IServerLoaderThread, 0);

    //gets the server's loader thread object.
    virtual SLoaderThread *LoaderThread() = 0;
};


#endif

