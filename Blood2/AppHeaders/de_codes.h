
// This module defines all the DirectEngine return codes.

#ifndef __DE_RETURNCODES_H__
#define __DE_RETURNCODES_H__

	
	// DRESULT is used for all return values.
	// High 8 bits: error flags
	// Low 16 bits: error code
	#define ERROR_FLAGS(err) ((err) & 0xFF000000)
	#define ERROR_CODE(err) ((err) & 0xFFFF)

	typedef unsigned long DRESULT;


	#define NUM_AXIS_OFFSETS 3


	// --------------------------------------------------------------- //
	// Defs.
	// --------------------------------------------------------------- //

	typedef enum
	{
		ClientType=0,
		ServerType
	} ClientServerType;

	// File types.
	#define FT_MODEL		0
	#define FT_SPRITE		1
	#define FT_TEXTURE		2
	#define FT_SOUND		3

	// Types of global panning things.
	#define GLOBALPAN_SKYSHADOW		0
	#define GLOBALPAN_FOGLAYER		1
	#define NUM_GLOBALPAN_TYPES		2


	// NetService flags.
	#define NETSERVICE_TCPIP	(1<<0)	// This is the engine's TCP/IP driver.  When you call
										// GetServiceList, you must pass in a list of IP addresses
										// to check on, formatted like <ip:port>, separated by
										// semicolons.  The port is optional.  Sample:
										// "joe.blow.com:1322 ; 204.342.1.2:123 ; 2.3.4.5"


	// Engine event ID's
	#define	LTEVENT_DISCONNECT		1	// Client disconnected from server.  dwParam will 
										// be a error flag found in de_codes.h.
	#define	LTEVENT_SHUTDOWN		2	// Engine shutting down.  dwParam will be a error 
										// flag found in de_codes.h.
	#define LTEVENT_RENDERINIT		3	// The renderer was initialized.  This is called if 
										// you call SetRenderMode or if focus was lost and
										// regained.
	#define LTEVENT_RENDERTERM		4	// The renderer is being shutdown.  This happens when
										// ShutdownRender is called or if the app loses focus.

	#define LTEVENT_RENDERALMOSTINITTED	5	// Called when the renderer has switched into
											// the new mode but before it reloads all the textures
											// so you can display a loading screen.

	// --------------------------------------------------------------- //
	// Flags.
	// --------------------------------------------------------------- //

	// Flags to ClientDE::ShutdownRender.
	#define RSHUTDOWN_MINIMIZEWINDOW		1	// Minimize the app's window.
	#define RSHUTDOWN_HIDEWINDOW			2	// Hide the app's window.

	// Flags to EndMessage2().
	#define MESSAGE_NAGGLE		(1<<0)	// Setting this flag allows the engine to group the 
										// message into a larger chunk, but it might not necessarily
										// get sent right away.  This helps a lot if you're sending
										// a bunch of small messages.
										// NOTE: ALL naggled messages get discarded if a new world is
										// loaded on the server to prevent old info from getting to 
										// the client, but those cases still can pop up if you send
										// unguaranteed messages.
	#define MESSAGE_NAGGLEFAST	(1<<1)	// Same as naggle but these ones are sent out 15 times per second.
	
	// DON'T change this value.
	#define MESSAGE_GUARANTEED	(1<<7)	// Send with guaranteed delivery.  Try to send as many
										// network messages as possible without guaranteed delivery.
										// Initialization messages and stuff like that should be
										// guaranteed but anything that can be discarded shouldn't
										// use guaranteed delivery.	 Keep in mind that if you send
										// without guaranteed delivery you can't do any kind of 
										// incremental updating, you'll have to send it periodically
										// if you want to make sure the other side is updated on it.

	#define MESSAGE_NOTHROTTLE	(1<<6)	// By default, the engine limits its outgoing bandwidth by
										// dropping packets if it's sending too fast (guaranteed messages
										// will still be guaranteed, they just take longer to get through).
										// This forces it to send a packet regardless of how much data is 
										// being sent.


	#define MESSAGE_NAGGLEMASK	(MESSAGE_NAGGLE|MESSAGE_NAGGLEFAST)

	
	// Portal flags.
	#define PORTAL_OPEN		1

	// Video playback flags.
	#define PLAYBACK_FULLSCREEN	1
	#define PLAYBACK_YINTERLACE	2

	// Screen flip flags.
	#define FLIPSCREEN_CANDRAWCONSOLE	1	// Setting this says it can draw the console if
											// the console is up.

	// Screen clear flags.
	#define CLEARSCREEN_SCREEN			1	// Clear the visual screen contents.
	#define CLEARSCREEN_RENDER			2	// You must clear with this bit any place you're about to render.

	// Intersect flags.
	#define INTERSECT_OBJECTS		1	// Try to hit objects.. small speed hit by doing so.
	#define IGNORE_NONSOLID			2	// Ignore nonsolid objects and nonsolid surfaces. Just a helper filter.
	#define INTERSECT_HPOLY			4	// Fill in the HPOLY for this intersection (this slows it down a little).


	// Object net flags.
	#define NETFLAG_ANIMUNGUARANTEED	(1<<0)	// Animation info sent unguaranteed. 

	#define NETFLAG_POSUNGUARANTEED		(1<<1)	// Position and rotation updates for this object
												// are sent with unguaranteed delivery.  Good for
												// objects in netgames that move or rotate a lot.
												// Bad for objects that stay still.
	#define NETFLAG_ROTUNGUARANTEED		(1<<2)


	// --------------------------------------------------------------- //
	// DE_ERROR flags.
	// Note: on an allocation failure, the engine shuts down on the
	//       spot so there is no DE_OUTOFMEMORY error.
	// --------------------------------------------------------------- //
	
	#define ERROR_DISCONNECT	(1<<25)	// Disconnect from the server.
	#define ERROR_SHUTDOWN		(1<<26)	// Shutdown everything.


	// --------------------------------------------------------------- //
	// Return codes.
	// --------------------------------------------------------------- //

	#define LT_OK		0	// No problem!
	#define DE_OK		0	
	
	#define LT_ERROR	1	// Uh oh..
	#define DE_ERROR	1	
	
	#define DE_FINISHED	2	// Done with operation.

	// Video status.
	#define VIDEO_STARTED		10
	#define VIDEO_ERROR			11
	#define VIDEO_PLAYING		12
	#define VIDEO_NOTPLAYING	13

	#define DE_TRIEDTOREMOVECLIENTOBJECT	20	// Tried to remove a client's object.
	#define DE_NOTINWORLD					21	// Tried to do an operation but a world wasn't running.
	#define DE_MISSINGFILE					22	// Missing the requested file.
	#define DE_MISSINGWORLDMODEL			23	// Missing the requested WorldModel.
	#define DE_INVALIDMODELFILE				24	// Invalid model file.
	#define DE_OBJECTNOTINWORLD				25	// Tried to modify an object but it was removed from the world.
	#define DE_CANTREMOVESERVEROBJECT		26	// Can't remove a server object from the client.
	
	#define DE_CANTLOADGAMERESOURCES		27	// Was missing game rezfile or directory.
	#define DE_CANTINITIALIZEINPUT			28	// Unable to initialize input.
	#define DE_MISSINGSHELLDLL				29
	#define DE_INVALIDSHELLDLL				30
	#define DE_INVALIDSHELLDLLVERSION		31
	#define DE_CANTCREATECLIENTSHELL		32
	#define DE_UNABLETOINITSOUND			35
	#define DE_MISSINGMUSICDLL				36
	#define DE_INVALIDMUSICDLL				37
	#define DE_UNABLETOINITMUSICDLL			38
	#define DE_CANTINITDIRECTPLAY			39
	#define DE_USERCANCELED					40	// User canceled connect dialog.
	#define DE_MISSINGWORLDFILE				41	// Missing world file from server.
	#define DE_INVALIDWORLDFILE				42	// Invalid world file.
	#define DE_ERRORBINDINGWORLD			43	// Error binding world to renderer.
	#define DE_INVALIDSERVERPACKET			44	// Got a bad packet from the server.
	#define DE_MISSINGSPRITEFILE			45
	#define DE_INVALIDSPRITEFILE			46
	#define DE_MISSINGMODELFILE				47
	#define DE_UNABLETORESTOREVIDEO			48	// Couldn't restore video mode.
	#define DE_SERVERERROR					49	// Got an error from the server.
	#define DE_CANTCREATESERVER				50	// Was unable to create a server.
	#define DE_ERRORLOADINGRENDERDLL		51	// Error loading the render DLL.

	#define DE_MISSINGCLASS					52	// Missing a needed class from object.dll.
	#define DE_CANTCREATESERVERSHELL		53	// Unable to create a server shell.
	#define DE_INVALIDOBJECTDLL				54	// Invalid (or missing) object DLL.
	#define DE_INVALIDOBJECTDLLVERSION		55	// Invalid object DLL version.
	#define DE_ERRORINITTINGNETDRIVER		56	// Couldn't initialize net driver.
	#define LT_NOGAMERESOURCES				57	// No game resources specified.
	#define DE_CANTRESTOREOBJECT			58	// Couldn't restore an object.

	#define DE_NODENOTFOUND					59	// Couldn't find the specified model node.
	#define DE_INVALIDPARAMS				60	// Invalid parameters passed to function (like NULL for an HOBJECT).

	#define DE_NOTFOUND						61	// Something was not found.
	#define DE_ALREADYEXISTS				62	// Something already exists.
	#define DE_NOTCONNECTED					63	// Not currently on a server.
	
	#define DE_INSIDE						64	// Inside.
	#define DE_OUTSIDE						65	// Outside.

	#define DE_INVALIDDATA					66	// Invalid data.
	#define DE_OUTOFMEMORY					67	// Couldn't get enough memory.  The engine
												// will always shutdown before giving this
												// error, so it's only used in tools.
	#define DE_MISSINGPALETTE				68	// Internal.
	#define DE_INVALIDVERSION				69	// Invalid version.

	#define LT_NOCHANGE						70	// Nothing was changed.

	#define LT_INPUTBUFFEROVERFLOW			71	// Input buffer overflowed.
	#define LT_OVERFLOW						71	// Overflow (no shit).
		
	#define LT_KEPTSAMEMODE					72	// Wasn't able to switch to new mode.

	#define LT_NOTINITIALIZED				73
	#define LT_ALREADYIN3D					74	// Already between a Start3D/End3D block.
	#define LT_NOTIN3D						75	// Not between a Start3D/End3D block.

	#define LT_ERRORCOPYINGFILE				76
	#define LT_INVALIDFILE					77

	#define LT_INVALIDNETVERSION			78	// Tried to connect to a server with a different
												// network protocol version.

	#define LT_TIMEOUT						79	// Timed out..
	
	#define LT_CANTBINDTOPORT				80	// Couldn't bind to the requested port.
	#define LT_REJECTED						81	// Connection rejected.
	#define LT_NOTSAMEGUID					82	// The host you tried to connect to was running a game
												// with a different app DGUID.

	#define LT_NO3DSOUNDPROVIDER			83	// Unable to initialize the 3d sound provider


#endif  // __DE_RETURNCODES_H__




