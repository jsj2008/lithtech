#ifndef __ISERVERSHELL_H__
#define __ISERVERSHELL_H__

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __LTENGINEOBJECTS_H__
#include "ltengineobjects.h"
#endif

#ifndef __ILTSERVER_H__
#include "iltserver.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

class ILTServer;
extern ILTServer *g_pLTServer;

/*!  IServerShell interface.  You must implement this for a server.  */

class IServerShell : public IBase {
public:
    interface_version(IServerShell, 4);

    inline ILTServer *GetServerDE() const {return g_pLTServer;}
    inline ILTServer *GetLTServer() const {return g_pLTServer;}

/*!
\return \b LT_OK - Everything is fine.
\return \b LT_ERROR - Fatal error occurred.

This function is called after the IServerShell implementation class has
been allocated in game code, and before any other IServerShell function.

Things that used to done in the old CreateServerShell function can now be done
here.
*/
    virtual LTRESULT OnServerInitialized() = 0;

/*!
Called just before the engine unloads the servershell and shuts down.

Things that used to be done in the old DestroyServerShell function can
now be done here.

Used for: ServerShell Callbacks.
*/
	virtual void OnServerTerm() = 0;

/*!  
\param  pMsg    String message sent from the server app.
\param  nLen    Number of chars in string.

\return \b SendToServerShell() returns the callback return value.

Receive message from the server app. Typically this is a standalone server 
front-end.

Used for: ServerShell Callbacks.
*/
    virtual LTRESULT ServerAppMessageFn( ILTMessage_Read& msg ) = 0;

/*!
\param  hClient     Client handle.

Called when client first connects after client has been given an ID, but 
before any data is sent to it.

Used for: ServerShell Callbacks.
*/
    virtual void OnAddClient(HCLIENT hClient) = 0;

/*!
\param  hClient     Client handle.

\return The engine ignores the callback return value.

  Called almost immediately after OnClientExitWorld() (just after client has 
unloaded the world).

Used for: ServerShell Callbacks.
*/
    virtual void OnRemoveClient(HCLIENT hClient) = 0;

/*!
\param  bSwitchingWorlds    World switching indicator. True if currently 
                            running a world. False *only* if no world is 
                            currently running (e.g. first world, or coming 
                            from a pre-game menu).

Called just after SRand(), before any previously loaded level objects, etc, 
have been unloaded.

Used for: ServerShell Callbacks.
*/
    virtual void PreStartWorld(bool bSwitchingWorlds) = 0;

/*!
Called after CacheFiles(), and all unused models and sounds are freed.

Used for: ServerShell Callbacks.
*/
    virtual void PostStartWorld() = 0;

/*!
\param  hClient         Connecting client's handle.
\param  pClientData     Client's StartGameRequest.
\param  clientDataLen   Size (in bytes) of client data.

\return You MUST return the pointer to the new client instance (the C++ class, 
        not its m_hObject.

Called after the client has actually entered the world (ie. after 
OnAddClient()). You MUST create an object to represent the client here and 
return it (for PVS, etc).

Used for: ServerShell Callbacks.
*/
    virtual LPBASECLASS OnClientEnterWorld(HCLIENT hClient) = 0;

/*!
\param  hClient     Exiting client's handle.

Called when client leaves the world (just before the engine tells it to 
unload the world).

Used for: ServerShell Callbacks.
*/
    virtual void OnClientExitWorld(HCLIENT hClient) = 0;

/*!
\param  timeElapsed     Time since last call to Update().

Called every frame tick. Typically, this function is used for 
synchronization, if the game requires it.

Used for: ServerShell Callbacks.
*/
    virtual void Update(LTFLOAT timeElapsed) = 0;

/*!
\param  hSender     Handle to client that sent the message.
\param  pMessage    The message.

Called when the server receives a message from a client (hSender) via 
SendToServer().

\see ILTClient::SendToServer()

Used for: ServerShell Callbacks.
*/
    virtual void OnMessage(HCLIENT hSender, ILTMessage_Read *pMessage) = 0;

/*!
\param  pSender     Pointer to object that sent the message.
\param  pMessage    The message.

Called when the server receives a message from an object (pSender) via
SendToServer().

\see ILTServer::SendToServer()

Used for: ServerShell Callbacks.
*/
    virtual void OnObjectMessage(LPBASECLASS pSender, ILTMessage_Read *pMessage) = 0;

/*!
Called when a demo playback is done.

Used for: ServerShell Callbacks.
*/
    virtual void OnPlaybackFinish() = 0;

/*!
Called after world is loaded and it's objects, their skins, and all sprites 
have been cached. Call ILTServer::CacheFile() for each additional sprite, 
model, texture, and sound that you want to make sure are in memory for the 
level.

\see ILTServer::CacheFile()

Used for: ServerShell Callbacks.
*/
    virtual void CacheFiles() = 0;

/*!
\param  uiRand  The seed that the engine passes to you (usually the system 
                time).

First thing to be called when a world is requested, but not started loading yet.
uiRand defaults to current time.  A good use of this function would be to save 
the random seed for savegames, recordings, etc.

Used for: ServerShell Callbacks.
*/
    virtual void SRand(unsigned int uiRand) = 0;

/*!
\param  pFilename   Filename.
\param  status      \b LT_OK if file loaded ok. \b LT_ERROR otherwise.

\return The engine ignores the callback return value.

Called when a (model) file is done loading (from a call to ThreadLoadFile).
For example, when loading a model to represent your player, FileLoadNotify() 
is called afterwards.

Used for: ServerShell Callbacks.
*/
    virtual LTRESULT FileLoadNotify(const char *pFilename, LTRESULT status) = 0;

/*!
\param  pData       The packet data.
\param  dataLen     Length of the packet data.
\param  senderAddr  IP address of sender.
\param  senderPort  Port of sender.

\return The engine ignores the callback return value.

Called when the server gets a network packet that it cannot understand.
Typically, this is used in conjunction with apps like GameSpy, etc.

Used for: ServerShell Callbacks.
*/
    virtual LTRESULT ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort) = 0;
};


/*!
IServerShellStub class. This class gives empty bodies to all IServerShell
functions.  Derive your IServerShell class from IServerShellStub and
you only need to declare functions that have non-trivial bodies.
*/

class IServerShellStub : public IServerShell {
    virtual LTRESULT OnServerInitialized() {return LT_OK;}
    virtual void OnServerTerm() {};
    virtual LTRESULT ServerAppMessageFn( ILTMessage_Read& msg ) {return LT_OK;}
    virtual void OnAddClient(HCLIENT hClient) { }
    virtual void OnRemoveClient(HCLIENT hClient) {}
    virtual LPBASECLASS OnClientEnterWorld(HCLIENT hClient)=0;
    virtual void OnClientExitWorld(HCLIENT hClient) {}
    virtual void PreStartWorld(bool bSwitchingWorlds) {}
    virtual void PostStartWorld() {}     
    virtual void OnMessage(HCLIENT hSender, ILTMessage_Read *pMessage) {}
    virtual void OnObjectMessage(LPBASECLASS pSender, ILTMessage_Read *pMessage) {}     
    virtual void Update(LTFLOAT timeElapsed) {}
    virtual void OnPlaybackFinish() {}
    virtual void CacheFiles() {}
    virtual void SRand(unsigned int uiRand) {srand(uiRand);}
    virtual LTRESULT FileLoadNotify(const char *pFilename, LTRESULT status) {return LT_OK;}
    virtual LTRESULT ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort) { return LT_OK; }
};


/*!
Interface class that manages the server shell DLL's windows instance handle.
*/
class IInstanceHandleServer : public IBase {
public:
    interface_version(IInstanceHandleServer, 1);

/*!
Called by the engine if the interface is defined.  If the server shell
is in a windows DLL, the instance handle is passed in.  On other platforms,
or if the server shell is not in a DLL, NULL is passed in.
*/
    virtual void SetInstanceHandle(void *handle) = 0;

/*!
Call this function to retrieve the instance handle that was given by the engine.
*/
    virtual void *GetInstanceHandle() = 0;
};



/*!
Macro that defines the global g_pLTServer holder
*/

#define SETUP_GPLTSERVER()                  \
    ILTServer *g_pLTServer;                 \
    define_holder(ILTServer, g_pLTServer);  \


/*!
Macro that defines and instantiates an implementation of the IInstanceHandleServer
interface.
*/
#define SETUP_DLLINSTANCESERVER()                                           \
    class CInstanceHandleServer : public IInstanceHandleServer {            \
      public:                                                               \
        declare_interface(CInstanceHandleServer);                                                \
        CInstanceHandleServer() {hLTDLLInstance = NULL;}                    \
        void SetInstanceHandle(void *handle) {hLTDLLInstance = handle;}     \
        void *GetInstanceHandle() {return hLTDLLInstance;}                  \
        void *hLTDLLInstance;                                               \
    };                                                                      \
    define_interface(CInstanceHandleServer, IInstanceHandleServer);         \


/*!
Use this macro one time somewhere in your game server code.
*/
#define SETUP_SERVERSHELL()         \
    SETUP_GPLTSERVER();             \
    SETUP_DLLINSTANCESERVER();      \



#endif  //! __ISERVERSHELL_H__



