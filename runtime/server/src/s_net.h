//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#ifndef __S_NET_H__
#define __S_NET_H__

class CServerMgr;
class CServerEvent;
struct Client;
class ModelInstance;
struct ObjInfo;
class CSoundTrack;

extern int32 g_bDebugPackets;

// It makes sure it has this much room in a packet before writing events into it.
#define MAX_EVENTPACKET_LEN         35
#define MAX_SOUNDTRACKPACKET_LEN    45

#define FU_FORCEFLUSH   0xFFFFFFFF



// Here's the packet handler array.
struct ServerPacketHandler
{
    LTRESULT (*m_Fn)(CPacket_Read &cPacket, Client *pClient);
};
extern ServerPacketHandler g_ServerHandlers[256];


// Write the file and skin info to the packet.
bool sm_WriteModelFiles(LTObject *pObj, CPacket_Write &cPacket);

//will write out the model files into the packet if they are non-null in the ocs, this allows
//for sending down of only changed files
void sm_WriteChangedModelFiles(LTObject *pObj, CPacket_Write &cPacket, ObjectCreateStruct* pStruct);

// Writes the model animation info into the packet.
void WriteAnimInfo(ModelInstance *pInst, CPacket_Write &cPacket);

// Looks at the flags in pInfo and fills the packet with update data.
// Returns FALSE if no info needed to be sent.
bool FillPacketFromInfo(Client *pClient, LTObject *pObj, ObjInfo *pInfo, CPacket_Write &cPacket);

// Sends the given packet to everyone.
void SendServerMessage(const CPacket_Read &cPacket, 
    uint32 packetFlags = MESSAGE_GUARANTEED);

// Sends the given packet to the given client (and its attachments if you want).
void    SendToClient(Client *pClient, 
    const CPacket_Read &cPacket, bool bSendToAttachments, 
    uint32 packetFlags = MESSAGE_GUARANTEED);

// Send to clients that are in the world.
void    SendToClientsInWorld(const CPacket_Read &cPacket);


// Creates an event of the specified type and adds it to the client's structures.
CServerEvent* CreateServerEvent(int type);

// Creates an event of the specified type and adds it to a single client's structures.
CServerEvent* CreateServerToClientEvent(int type, Client *pClient);

// Writes the event info into the packet.
void WriteEventToPacket(CServerEvent *pEvent, Client *pClient, CPacket_Write &cPacket);

// Process an incoming packet
LTRESULT ProcessIncomingPacket(Client *pFromClient, const CPacket_Read &cPacket);

// Reads in all packets from the net.
LTRESULT ProcessIncomingPackets();

// Just sets up some pointers to functions for packet receivers.
void InitServerNetHandlers();

// Send sound message based on change info
void FillSoundTrackPacketFromInfo(CSoundTrack *pSoundTrack, ObjInfo *pObjInfo, Client *pClient, CPacket_Write &cPacket);

#endif  // __S_NET_H__


