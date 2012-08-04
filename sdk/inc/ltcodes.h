/*!
This module defines all the DirectEngine return codes.
*/

#ifndef __LTCODES_H__
#define __LTCODES_H__

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif



/*!
LTRESULT is used for all return values.
High 8 bits: error flags.
Low 16 bits: error code.
*/
#define ERROR_FLAGS(err) ((err) & 0xFF000000)
#define ERROR_CODE(err) ((err) & 0xFFFF)


#define NUM_AXIS_OFFSETS 3




/*!
Definitions.
*/

typedef enum {
    ClientType=0,
    ServerType
} ClientServerType;

/*! 
Client-side object flags.
*/
enum
{
/*!
The engine calls ClientShell::OnObjectRemove() when the object goes away.
*/
    CF_NOTIFYREMOVE =     (1<<0),

//! Calls ClientShellDE::OnModelKey when it hits model keys.
    CF_NOTIFYMODELKEYS =  (1<<1),

/*! 
The engine automatically sets the object's dimensions to its 
animation dimensions.  This tells it not to.
*/
	CF_DONTSETDIMS =      (1<<2)  ,

//! Hint to the renderer that this canvas is solid so it will be drawn first.
    CF_SOLIDCANVAS =      (1<<4)  ,
};


/*!
File types.
*/
enum
{
    FT_MODEL =        0,
    FT_SPRITE =       1,
    FT_TEXTURE =      2,
    FT_SOUND =        3,
};




/*!
Types of global panning things.
*/
enum
{
    GLOBALPAN_SKYSHADOW =     0,
    GLOBALPAN_FOGLAYER =      1,
    NUM_GLOBALPAN_TYPES =     2,
};





/*!
NetService flags.
*/

enum
{

/*!
This is the engine's TCP/IP driver.  When you call
GetServiceList, you must pass in a list of IP addresses
to check on, formatted like <ip:port>, separated by
semicolons.  The port is optional.  Sample:
"joe.blow.com:1322 ; 204.342.1.2:123 ; 2.3.4.5"
*/
    NETSERVICE_TCPIP =    (1<<0)  ,
};


/*!
Engine event ID's
*/
enum
{
/*! 
Client disconnected from server.  \b dwParam will 
be a error flag found in de_codes.h.
*/
    LTEVENT_DISCONNECT =      1   ,

/*!
Engine shutting down.  \b dwParam will be an error 
flag found in de_codes.h.
*/
    LTEVENT_SHUTDOWN =        2   ,

/*!
The renderer was initialized.  This is called if 
you call SetRenderMode or if focus was lost and
regained.
*/
    LTEVENT_RENDERINIT =      3,

/*!
The renderer is being shut down.  This happens when
ShutdownRender() is called or if the application loses focus.
*/
    LTEVENT_RENDERTERM =      4,

/*!
Called when the renderer has switched into
the new mode but before it reloads all the textures
so you can display a loading screen.
*/
    LTEVENT_RENDERALMOSTINITTED = 5   ,

//! Called when \b NullRender is 1 and we lose focus.
    LTEVENT_LOSTFOCUS =           6   ,

//! Called when \b NullRender is 1 and we have regained focus.
    LTEVENT_GAINEDFOCUS =         7   ,
};


/*!
Flags.
*/

/*@{*/
/*!
\name ShutdownRender Flags.
Flags to ClientDE::ShutdownRender.
*/

enum
{
//! Minimize the app's window.
    RSHUTDOWN_MINIMIZEWINDOW =        1   ,

//! Hide the app's window.
    RSHUTDOWN_HIDEWINDOW =            2   ,
};
/*@}*/

/*!
Flags for sending messages.
*/

enum
{
/*!  
Send with guaranteed delivery.  Try to send as many network
messages as possible without guaranteed delivery.  Initialization
messages and occasional messages requiring ordered delivery may
need to be sent using guaranteed delivery.  But you should avoid
use guaranteed delivery wherever possible.

Keep in mind that messages sent without guaranteed delivery may arrive
out of order and are subject to being discarded at the discretion of 
the engine.  Rather than develop your own mechanisms to resend
unguaranteed messages to ensure that they arrive at their destination
in the proper order, you may want to strongly consider using 
guaranteed messages.
*/
    MESSAGE_GUARANTEED =  (1<<0)
};



/*!
Portal flags.
*/
enum
{
    PORTAL_OPEN =     1,
};


/*!
Video playback flags.
*/
enum
{
    PLAYBACK_FULLSCREEN = (1<<0), // currently does nothing //
    PLAYBACK_YINTERLACE = (1<<1),
    PLAYBACK_FROMHANDLE = (1<<2),
    PLAYBACK_FROMMEMORY = (1<<3),
    PLAYBACK_LASTFLAG =   (1<<3),
    PLAYBACK_FLAGMASK =   0x7
};



/*!
Setting this says it can draw the console if
the console is up.
*/
#define END3D_CANDRAWCONSOLE (1<<0)

/*!
Screen flip flags.
*/
enum
{
//! Do we copy?
//  #define FLIPSCREEN_COPY             (1<<1)      <- Use the device creation console variable in it's place...

//! This just does dirty rectangles
    FLIPSCREEN_DIRTY =           (1<<2),      //<-- Dirty rects support has been removed... (ok, not yet, but very soon)
};


/*!
Screen clear flags.
*/

enum
{
//! Clear the visual screen contents.
    CLEARSCREEN_SCREEN =          1   ,

//! You must clear with this bit any place you are about to render.
    CLEARSCREEN_RENDER =          2   ,
};


/*!
Intersect flags.
*/
enum
{
//! Try to hit objects.  There is a small speed hit by doing so.
    INTERSECT_OBJECTS =       1   ,

//! Ignore nonsolid objects and nonsolid surfaces. Just a helper filter.
    IGNORE_NONSOLID =         2   ,

//! Fill in the \b HPOLY for this intersection (this slows it down a little).
    INTERSECT_HPOLY =         4   ,

//! HACK FIX - Gee wouldn't it be nice to intersect objects if the from point
//  is inside the object's dims.  If so, you need to set this flag since a bunch
//  of game code counts on broken behavoir
	CHECK_FROM_POINT_INSIDE_OBJECTS =	8,

//! Try to hit model oriented bounding boxes if the model has them.
//  (There is a performance hit for doing so)
	INTERSECT_MODELOBBS =		16,
};


/*!
Object net flags.
*/

enum
{
//! Animation info sent unguaranteed. 
    NETFLAG_ANIMUNGUARANTEED =    (1<<0)  ,

/*!
Position and rotation updates for this object
are sent with unguaranteed delivery.  Good for
objects in netgames that move or rotate a lot.
Bad for objects that stay still.
*/
    NETFLAG_POSUNGUARANTEED =     (1<<1)  ,

    NETFLAG_ROTUNGUARANTEED =     (1<<2),
};

/*!
Video status.
*/
enum
{
    VIDEO_STARTED =       10,
    VIDEO_ERROR =         11,
    VIDEO_PLAYING =       12,
    VIDEO_NOTPLAYING =    13,
};


/*!
\b DE_ERROR flags.
*/

enum
{
/*!
\note On an allocation failure, the engine shuts down on the
spot so there is no \b DE_OUTOFMEMORY error.
*/
    
//! Disconnect from the server.
    ERROR_DISCONNECT =    (1<<25) ,

//! Shut everything down.
    ERROR_SHUTDOWN =      (1<<26) ,

/*!
No problem. 
*/
    LT_OK =       0   ,

/*! 
Problem.
*/
    LT_ERROR =    1   ,

/*! 
Done with operation. 
*/
    LT_FINISHED = 2 ,

//! Tried to remove a client's object.
    LT_TRIEDTOREMOVECLIENTOBJECT =    20  ,

//! Tried to perform an operation but a world wasn't running.
    LT_NOTINWORLD =                   21  ,

//! Missing the requested file.
    LT_MISSINGFILE =                  22  ,

//! Missing the requested WorldModel.
    LT_MISSINGWORLDMODEL =            23  ,

//! Invalid model file.
    LT_INVALIDMODELFILE =             24  ,

//! Tried to modify an object but it was removed from the world.
    LT_OBJECTNOTINWORLD =             25  ,

//! Can't remove a server object from the client.
    LT_CANTREMOVESERVEROBJECT =       26  ,

//! Was missing game rezfile or directory.  
    LT_CANTLOADGAMERESOURCES =        27  ,

//! Unable to initialize input.
    LT_CANTINITIALIZEINPUT =          28  ,

    LT_MISSINGSHELLDLL =              29,

    LT_INVALIDSHELLDLL =              30,

    LT_INVALIDSHELLDLLVERSION =       31,

    LT_CANTCREATECLIENTSHELL =        32,

    LT_UNABLETOINITSOUND =            35,

    LT_MISSINGMUSICDLL =              36,

    LT_INVALIDMUSICDLL =              37,

    LT_UNABLETOINITMUSICDLL =         38,

    LT_CANTINITDIRECTPLAY =           39,

//! User canceled connect dialog.
    LT_USERCANCELED =                 40  ,

//! Missing world file from server.
    LT_MISSINGWORLDFILE =             41  ,

//! Invalid world file.
    LT_INVALIDWORLDFILE =             42  ,

//! Error binding world to renderer.
    LT_ERRORBINDINGWORLD =            43  ,

//! Got a bad packet from the server.
    LT_INVALIDSERVERPACKET =          44  ,

    LT_MISSINGSPRITEFILE =            45,

    LT_INVALIDSPRITEFILE =            46,

    LT_MISSINGMODELFILE =             47,

//! Couldn't restore video mode.
    LT_UNABLETORESTOREVIDEO =         48  ,

//! Got an error from the server.
    LT_SERVERERROR =                  49  ,

//! Was unable to create a server.
    LT_CANTCREATESERVER =             50  ,

//! Error loading the render DLL.
    LT_ERRORLOADINGRENDERDLL =        51  ,

//! Missing a needed class from object.dll.
    LT_MISSINGCLASS =                 52  ,

//! Unable to create a server shell.
    LT_CANTCREATESERVERSHELL =        53  ,

//! Invalid (or missing) object DLL.
    LT_INVALIDOBJECTDLL =             54  ,

//! Invalid object DLL version.
    LT_INVALIDOBJECTDLLVERSION =      55  ,

//! Couldn't initialize net driver.
    LT_ERRORINITTINGNETDRIVER =       56  ,

//! No game resources specified.
    LT_NOGAMERESOURCES =              57  ,

//! Couldn't restore an object.
    LT_CANTRESTOREOBJECT =            58  ,

//! Couldn't find the specified model node.
    LT_NODENOTFOUND =                 59  ,

//! Invalid parameters passed to function (like \b NULL for an \b HOBJECT).
    LT_INVALIDPARAMS =                60  ,

//! Something was not found.
    LT_NOTFOUND =                     61  ,

//! Something already exists.
    LT_ALREADYEXISTS =                62  ,

//! Not currently on a server.  
    LT_NOTCONNECTED =                 63  ,

//! Inside. 
    LT_INSIDE =                       64  ,

//! Outside.
    LT_OUTSIDE =                      65  ,

//! Invalid data.
    LT_INVALIDDATA =                  66  ,

/*!
Couldn't get enough memory.  The engine
will always shut down before giving this
error, so it's only used in tools.
*/
    LT_OUTOFMEMORY =                  67  ,

//! Internal.
    LT_MISSINGPALETTE =               68  ,

//! Invalid version.
    LT_INVALIDVERSION =               69  ,

//! Nothing was changed.
    LT_NOCHANGE =                     70  ,

//! Input buffer overflowed.
    LT_INPUTBUFFEROVERFLOW =          71  ,

//! Overflow.
    LT_OVERFLOW =                     71  ,

//! Wasn't able to switch to new mode.      
    LT_KEPTSAMEMODE =                 72  ,

    LT_NOTINITIALIZED =               73,

//! Already between a Start3D/End3D block.
    LT_ALREADYIN3D =                  74  ,

//! Not between a Start3D/End3D block.
    LT_NOTIN3D =                      75  ,

    LT_ERRORCOPYINGFILE =             76,
    LT_INVALIDFILE =                  77,

/*! 
Tried to connect to a server with a different
network protocol version.
*/
    LT_INVALIDNETVERSION =            78  ,

//! Timed out.
    LT_TIMEOUT =                      79  ,

//! Couldn't bind to the requested port.    
    LT_CANTBINDTOPORT =               80  ,

//! Connection rejected.
    LT_REJECTED =                     81  ,


/*!
The host you tried to connect to was running a game
with a different application \b DGUID.
*/
    LT_NOTSAMEGUID =                  82  ,

    LT_UNSUPPORTED =                  83,
    LT_INPROGRESS =                   84,

//! Unable to initialize the 3D sound provider
    LT_NO3DSOUNDPROVIDER =            85  ,

    LT_YES =                          86,

    LT_NO =                           87,


//! The cursor you want isn't here.
    LT_MISSINGCURSORRESOURCE =        88  ,

//! The ESC key was pressed to abort an operation
    LT_ESCABORT =                     89  ,

//! Error creating or initializing the multi-protocol manager.
    LT_ERRORINITTINGMPM =             90,

//! The supplied buffer is too small
	LT_BUFFERTOOSMALL =				91,

//! Attempt to re-initialize an object
    LT_ALREADYINITIALIZED =			92,

    LT_RETRY =                        93
};

/*!
\b Disconnection reasons
*/

enum 
{
	//! Unknown reason for disconnecting
	LT_DISCON_UNKNOWN =			0,
	//! Missing a file
	LT_DISCON_MISSINGFILE =		1,
	//! Loss of connection (server/network down)
	LT_DISCON_CONNECTLOST =		2,
	//! Connection terminated from this side
	LT_DISCON_CONNECTTERM =		3,
	//! Server booted the client
	LT_DISCON_SERVERBOOT =		4,
	//! Network connection timed out
	LT_DISCON_TIMEOUT =			5,
	//! World CRC check failed
	LT_DISCON_WORLDCRC =		6,
	//! Misc resource CRC check failed
	LT_DISCON_MISCCRC =			7,
	//! Wrong password
	LT_DISCON_PASSWORD =		8,
	//! Start your game-side disconnection messages here
	LT_DISCON_USER =			0x10000
};

#endif  //! __LTCODES_H__




