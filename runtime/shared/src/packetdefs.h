//------------------------------------------------------------------
//
//	FILE	  : PacketDefs.h
//
//	PURPOSE	  : Defines all the packets used by DirectEngine.
//
//	CREATED	  : November 10 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __PACKETDEFS_H__
#define __PACKETDEFS_H__


// Layout..


//////////////////////////////////////////////////////////////
// NOTE NOTE NOTE
// The high bit of the packet ID is used by the net manager, so NEVER
// make packet IDs > 127.
// Packet ID 1 is reserved for CNetMgr.


// The first byte of each packet is the packet ID (one of the
// SMSG_ or CMSG_ defines below).


// Each time the protocol is updated, this number should be incremented.
#define LT_NET_PROTOCOL_VERSION		7	// 7 == LithTech 3.0 (spring 2001)


#define DEFAULT_CLIENT_UPDATE_RATE	10



// Server->client preload types.
#define PRELOADTYPE_START			0	// Marks the start of the preload list.
#define PRELOADTYPE_END				1	// Marks the end of the preload list.
#define PRELOADTYPE_MODEL			2
#define PRELOADTYPE_TEXTURE			3
#define PRELOADTYPE_SPRITE			4
#define PRELOADTYPE_SOUND			5
#define PRELOADTYPE_MODEL_CACHED    6


#define UUF_YROTATION	0x8 // Tells if this is a Y rotation (1 byte).
#define UUF_POS			0x4 
#define UUF_ROT			0x2
#define UUF_ANIMINFO	0x1 // Includes animation info (1 byte).
#define UUF_FLAGCOUNT	4 // Number of UUF flags
// Maximum object ID
#define ID_MAX				0xFFFE
// ID indicating a timestamp
#define ID_TIMESTAMP		0xFFFF

// This uses an extra byte in the world position compression but gives it a 256k
// resolution instead of 65k.

#define NUM_EXTRA_BITS_X	3
#define NUM_EXTRA_BITS_Y	2
#define NUM_EXTRA_BITS_Z	3

#define NUM_POSITION_BITS_X	(16+NUM_EXTRA_BITS_X)
#define NUM_POSITION_BITS_Y	(16+NUM_EXTRA_BITS_Y)
#define NUM_POSITION_BITS_Z	(16+NUM_EXTRA_BITS_Z)




//the PacketID typedef is gone... this is a cool idea in principle, but
//practically, it is not sufficiently useful, and only leads to detrimental
//header file dependencies.  Just use uint8 where you used to use PacketID.


// Used for the object type part of the ACV PackHOBJECTCreateInfo and UnpackHOBJECTCreateInfo methods.
#define	LONGSFXMESSAGE_FLAG		0x80
#define	SFXMESSAGE_FLAG			0x40
#define OBJECTTYPE_MASK			~(SFXMESSAGE_FLAG | LONGSFXMESSAGE_FLAG)


//////// Server packets. /////////

// This packet ID must NOT change.
#define SMSG_NETPROTOCOLVERSION		4

#define PACKETID_SERVERBASE		5

// Unload the current world (if any).
#define SMSG_UNLOADWORLD		(PACKETID_SERVERBASE+0)

// Load a world (remote version)
// Game time: float
// World file ID: uint16
#define SMSG_LOADWORLD			(PACKETID_SERVERBASE+1)

// Sets the object ID for your client object.
// The object ID for your client object.
#define SMSG_CLIENTOBJECTID		(PACKETID_SERVERBASE+2)

// Server update .. sent from server to client.
#define SMSG_UPDATE				(PACKETID_SERVERBASE+3)

// Unguaranteed server update.  Contains positions and rotations.
#define SMSG_UNGUARANTEEDUPDATE		(PACKETID_SERVERBASE+5)

// The first packet sent by the server.
// Word: Your client ID.
// Byte: bLocal (selects local or remote protocol)
#define SMSG_YOURID				(PACKETID_SERVERBASE+7)

// Sends a message packet to a client.
// message data
#define SMSG_MESSAGE			(PACKETID_SERVERBASE+8)

// Message group.. a collection of unguaranteed packets.
// uint8: packet len
// packet data
// ... (while there's data left in the packet)
#define SMSG_PACKETGROUP		(PACKETID_SERVERBASE+9)

// Change an object's filenames.
// uint16: Object ID
//  -uint8:  File Type (object, texture, or render style) (0xFF indicates end of list)
//  -uint8:  Optional file index
//  -uint16: File ID
//#define SMSG_CHANGEOBJECTFILENAMES	(PACKETID_SERVERBASE+10)

// Only sent to local clients.
// STRING:  Variable Name
// STRING:  Variable Data
#define SMSG_CONSOLEVAR			(PACKETID_SERVERBASE+10)

// Sky definition.
// SkyDef
// uint16: Number of objects
// uint16 for each object ID (0xFFFF if none for that slot)..
#define SMSG_SKYDEF				(PACKETID_SERVERBASE+11)

// Instant special effect.
// message data
#define SMSG_INSTANTSPECIALEFFECT	(PACKETID_SERVERBASE+12)

// List of things to preload.
// uint8: type (one of the PRELOADTYPE defines).
// uint16: file ID
// ...	
#define SMSG_PRELOADLIST		(PACKETID_SERVERBASE+14)

// uint8: FT_ define
// uint16: file ID
#define SMSG_THREADLOAD			(PACKETID_SERVERBASE+15)
#define SMSG_UNLOAD				(PACKETID_SERVERBASE+16)


// Global light information
// uint16 StaticSunLight object ID
#define SMSG_GLOBALLIGHT	 	(PACKETID_SERVERBASE+17)

// uint16 object id
// uint16 add/remove flag
// uint16 file_id
#define SMSG_CHANGE_CHILDMODEL     (PACKETID_SERVERBASE+18)

// Server subpackets, contained in SMSG_UPDATE packets.
// Subpackets are only sent when flags==0 at the start of the object update data.

// Play a sound
#define UPDATESUB_PLAYSOUND		0
#define UPDATESUB_SOUNDTRACK	1
#define UPDATESUB_ENDUPDATE		2
#define UPDATESUB_OBJECTREMOVES	3 // Same as ENDUPDATE but not including timing info.

// Model animation info constants
#define MODELINFO_ANIMINDEX_LONG 14
#define MODELINFO_ANIMINDEX_SHORT 8
#define MODELINFO_WEIGHTSET_LONG 8
#define MODELINFO_WEIGHTSET_SHORT 3
#define MODELINFO_RATEMODIFIER_DEFAULT 1.0f
#define MODELINFO_ANIMTIME_RES 8 // Animation time resolution in milliseconds
#define MODELINFO_ANIMTIME_SIZE0 1 // Animation time sizes (Divided by _RES, expressed in bits)
#define MODELINFO_ANIMTIME_SIZE1 6 // Time <= 512
#define MODELINFO_ANIMTIME_SIZE2 10 // Time <= 8192
#define MODELINFO_ANIMTIME_SIZE3 32 // Time <= big
#define MODELINFO_ANIMTIME_MIN 64 // Any animations shorter than this will be considered "two-frame", and use the smallest time size

//////// Change flags .. tells what will be sent for the given object. /////////

// Change flags.
#define CF_NEWOBJECT	(1<<0)	// Client(s) need to be told about this one.
#define CF_POSITION		(1<<1)	// Position changed.
#define CF_ROTATION		(1<<2)	// Rotation changed.
#define CF_FLAGS		(1<<3)	// Flags (and user flags) changed.
#define CF_SCALE		(1<<4)	// Scale changed.
#define CF_MODELINFO	(1<<5)	// For sprites, the change flags are followed by a sprite info.
								// For models, the change flags are followed by model info.
#define CF_SOUNDINFO	(1<<5)	// For sounds, sound has had killsoundloop called on it.
#define CF_RENDERINFO	(1<<6)	// Render info changed (RGBA, render group, and light radius).
#define CF_OTHER		(1<<7)	// There's an extra change flag byte.

// Things included in the CF_OTHER byte.
#define CF_ATTACHMENTS	(1<<8)	// Attachment info...
#define CF_TELEPORT		(1<<9)	// The object was teleported.
#define CF_SNAPROTATION	(1<<10)	// The object was teleported.
#define CF_FILENAMES	(1<<11)	// The model or sprite had its filenames changed.

#define CF_SENTINFO		(1<<12)	// Only used in the server's ObjInfos for each client.. tracks whether
								// or not info about a particular object was sent to a client.

#define CF_FORCEMODELINFO (1<<13) // Used to force a model info change, even if NETFLAG_ANIMUNGUARANTEED is set.

#define CF_POSITION_PREDICTIONCAP (1<<14) // Indicates that an extra position message should be sent down to cap off the prediction

#define CF_DIMS			(1<<15)	// The object changed dims

// Used by the server to see if any of the CF_OTHER-relevant flags are set.
#define CF_OTHERFLAGMASK	(CF_ATTACHMENTS | CF_TELEPORT | CF_SNAPROTATION | CF_FILENAMES | CF_DIMS )

#define CF_CLEARMASK	(CF_POSITION_PREDICTIONCAP) // Use this mask to "clear" the client flags to avoid clearing frame-coherent flags

// Tells what file specific info is sent in a message.
struct FileIDInfo
{
	uint8	m_nChangeFlags;

	uint16	m_wSoundPlaySoundFlags;
	uint8	m_nSoundPriority;
	uint16	m_nSoundOuterRadius;
	uint8	m_nSoundInnerRadius;
};

#define	FILEIDINFOF_SOUNDPLAYSOUNDFLAGS		(1<<0)
#define	FILEIDINFOF_SOUNDPRIORITY			(1<<1)
#define	FILEIDINFOF_RADIUS					(1<<2)
#define	FILEIDINFOF_NEWMASK					( FILEIDINFOF_SOUNDPLAYSOUNDFLAGS | \
												FILEIDINFOF_SOUNDPRIORITY | FILEIDINFOF_RADIUS )

//////// Client packets. /////////

#define PACKETID_CLIENTBASE		5

// Client hello
// Parameter 1: Flag (local/remote)
#define CMSG_HELLO				(PACKETID_CLIENTBASE+0)

// Client goodbye
#define CMSG_GOODBYE			(PACKETID_CLIENTBASE+1)

// Client update .. sent from client to server.
#define CMSG_UPDATE				(PACKETID_CLIENTBASE+2)

// This is a subset of the CMSG_UPDATE packet.. sent separately if there isn't
// room to fit in a CMSG_UPDATE.
#define CMSG_SOUNDUPDATE		(PACKETID_CLIENTBASE+3)

// Response to a connect stage.
// uint16: Which stage (currently, only stage is 'loading world').
#define CMSG_CONNECTSTAGE		(PACKETID_CLIENTBASE+4)

// Sends a command string to the server
// Parameter 1: String with the command
#define CMSG_COMMANDSTRING		(PACKETID_CLIENTBASE+5)

// Sends a message packet to the server.
// <data defined and used by packetmessage.cpp>
#define CMSG_MESSAGE			(PACKETID_CLIENTBASE+6)

// Used for testing (when the client blasts the server).
#define CMSG_TEST				(PACKETID_CLIENTBASE+7)


#endif  // __PACKETDEFS_H__


