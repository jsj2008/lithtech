#ifndef __CLIENT_LOADERTHREAD_H__
#define __CLIENT_LOADERTHREAD_H__


//Module that allocates and gives access to the "client loader thread"
class CLoaderThread;


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

class IClientLoaderThread : public IBase {
public:
    interface_version(IClientLoaderThread, 0);

    //gets the client's loader thread object.
    virtual CLoaderThread *LoaderThread() = 0;
};


#endif