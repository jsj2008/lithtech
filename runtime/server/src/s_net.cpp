//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//  FILE      : S_Net.cpp
//
//  PURPOSE   : Implements net-related stuff in the ServerMgr.
//
//  CREATED   : January 12 1996
//
//
//------------------------------------------------------------------

#include "bdefs.h"

#include "servermgr.h"
#include "geomroutines.h"
#include "s_net.h"
#include "s_concommand.h"
#include "soundtrack.h"
#include "s_object.h"
#include "impl_common.h"
#include "sysdebugging.h"
#include "server_interface.h"
#include "animtracker.h"
#include "serverevent.h"
#include "s_client.h"
#include "ftserv.h"
#include "server_filemgr.h"
#include "ltobjectcreate.h"
#include "ltmessage_server.h"

#include "misctools.h"

#include "compress.h"
static ICompress* compress;
define_holder(ICompress, compress);


extern int32 g_bLocalDebug;
extern int32 g_CV_ModelOnlyUpdateDirtyTrackers;

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//server console state
#include "server_consolestate.h"
#include "concommand.h"
static IServerConsoleState *console_state;
define_holder(IServerConsoleState, console_state);

//IWorld holder
#include "world_server_bsp.h"
#include "de_mainworld.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);

//IServerShell game server shell object.
#include "iservershell.h"
static IServerShell *i_server_shell;
define_holder(IServerShell, i_server_shell);






ServerPacketHandler g_ServerHandlers[256];




// ----------------------------------------------------------------------- //
//   Packet handlers
// ----------------------------------------------------------------------- //

LTRESULT OnSoundUpdatePacket(CPacket_Read &cPacket, Client *pClient)
{
    if (!pClient)
        return LT_OK;

    // Client has sent the sounds it has finished playing...
    while (!cPacket.EOP())
    {
        // Pull the sound track info out of the message...
		uint16 objectID = cPacket.Readuint16();
		LTRecord *pRecord = sm_FindRecord(objectID);
        if (!pRecord || (pRecord->m_nRecordType != RECORDTYPE_SOUND))
			continue;

		CSoundTrack *pSoundTrack = (CSoundTrack *)pRecord->m_pRecordData;
		if (pSoundTrack == LTNULL)
			continue;

		// Skip it if the sound isn't done yet
        if ((pClient->m_ObjInfos[ objectID ].m_nSoundFlags & OBJINFOSOUNDF_CLIENTDONE) != 0)
			continue;

		pSoundTrack->Release(&pClient->m_ObjInfos[ objectID ].m_nSoundFlags);
    }

    return LT_OK;
}


LTRESULT OnClientUpdatePacket(CPacket_Read &cPacket, Client *pClient)
{
    if (!pClient)
        return LT_OK;

	// Read the new receive bandwidth
	uint32 nNewReceiveBandwidth = (uint32)cPacket.Readuint16() * 8000;

	// Decide if we're going to need to change the flow control
	bool bFlowControlChanged = (nNewReceiveBandwidth != pClient->m_nDesiredReceiveBandwidth);
	pClient->m_nDesiredReceiveBandwidth = nNewReceiveBandwidth;

	if (bFlowControlChanged)
		g_pServerMgr->ResetFlowControl();

    if (!cPacket.EOP())
    {
        return OnSoundUpdatePacket(cPacket, pClient);
    }
    else
    {
        return LT_OK;
    }
}


LTRESULT OnClientDisconnectPacket(CPacket_Read &cPacket, Client *pClient)
{
    if (pClient && pClient->m_ConnectionID)
    {
        // This will in turn call DisconnectNotify().
        g_pServerMgr->m_NetMgr.Disconnect(pClient->m_ConnectionID, DISCONNECTREASON_VOLUNTARY_SERVERSIDE);
    }

    return LT_OK;
}


LTRESULT OnCommandStringPacket(CPacket_Read &cPacket, Client *pClient)
{
    if ((pClient->m_ClientFlags & CFLAG_LOCAL) == 0)
		return LT_OK;

	uint32 nCommandSize = cPacket.PeekString(0,0) + 1;
	char *pCommand = (char *)alloca(nCommandSize);
	cPacket.ReadString(pCommand, nCommandSize);
    return sm_HandleCommand(console_state->State(), pCommand);
}


LTRESULT OnMessagePacket(CPacket_Read &cPacket, Client *pClient)
{
	if (!pClient)
		return LT_OK;

	if (!i_server_shell)
		return LT_OK;

	CPacket_Read cSubPacket(cPacket, cPacket.Tell());
	CLTMsgRef_Read cMsg(CLTMessage_Read_Server::Allocate_Server(cSubPacket));

    i_server_shell->OnMessage((HCLIENT)pClient, cMsg);
    
    return LT_OK;
}


LTRESULT OnConnectStagePacket(CPacket_Read &cPacket, Client *pClient)
{
    uint8 type = cPacket.Readuint8();

    if (type == 0)
    {
        pClient->m_PuttingIntoWorldStage = PUTTINGINTOWORLD_LOADEDWORLD;
    }
    else
    {
        pClient->m_PuttingIntoWorldStage = PUTTINGINTOWORLD_PRELOADED;
    }

    return LT_OK;
}


LTRESULT OnHelloPacket(CPacket_Read &cPacket, Client *pClient)
{
    pClient->m_ClientDataLen = cPacket.Readuint16();
    if (pClient->m_ClientDataLen)
    {
        LT_MEM_TRACK_ALLOC(pClient->m_pClientData = new char[pClient->m_ClientDataLen],LT_MEM_TYPE_NETWORKING);
		cPacket.ReadData(pClient->m_pClientData, pClient->m_ClientDataLen * 8);
    }

    pClient->m_ClientFlags |= CFLAG_GOT_HELLO;
    return LT_OK;
}

// ----------------------------------------------------------------------- //
// These 2 are notification messages from CNetMgr.
// ----------------------------------------------------------------------- //
bool CServerMgr::NewConnectionNotify(CBaseConn *id, bool bIsLocal)
{
    Client * pClient = sm_OnNewConnection(id, bIsLocal);
    return (pClient != 0);
}


void CServerMgr::DisconnectNotify(CBaseConn *id, EDisconnectReason eDisconnectReason )
{
    sm_OnBrokenConnection(id);    
}


void CServerMgr::HandleUnknownPacket(const CPacket_Read &cPacket, uint8 senderAddr[4], uint16 senderPort)
{
    if (m_pServerAppHandler)
    {
		CLTMsgRef_Read cMsg(CLTMessage_Read_Server::Allocate_Server(cPacket));
        m_pServerAppHandler->ProcessPacket(*cMsg, senderAddr, senderPort);
    }

    // Tell the server shell about unknown packet.
    if (i_server_shell != NULL)
	{
		CLTMsgRef_Read cMsg(CLTMessage_Read_Server::Allocate_Server(cPacket));
        i_server_shell->ProcessPacket(cMsg, senderAddr, senderPort);
	}
}


bool sm_WriteModelFiles(LTObject *pObj, CPacket_Write &cPacket)
{
	// filename
	Model* pModel = pObj->ToModel()->GetModelDB();
	if( pModel )
	{
		cPacket.Writeuint16((uint16)pModel->m_FileID);
		ASSERT( pModel->m_FileID != 0xFFFF );
	}
	else
	{
		ASSERT( !"sm_WriteModelFiles:  Writing modelinstance without a model." );
		DEBUG_PRINT(1, ("sm_WriteModelFiles:  Writing modelinstance without a model." ));

		return false;
	}
    
    UsedFile* pUsedFile;

	if (pModel)
	{
		uint32 num_child = pModel->NumChildModels();

		cPacket.WriteBits(num_child, FNumBits<MAX_CHILD_MODELS>::k_nValue);
		
		for (uint32 i = 1; (i < num_child); i++)
		{	
			cPacket.Writeuint16((uint16)pModel->GetChildModel(i)->m_pModel->m_FileID);
		}	
	}
	else
	{
		cPacket.WriteBits(0, FNumBits<MAX_CHILD_MODELS>::k_nValue);
	}

	// textures & renderstyles
	uint32 nNumTextures = MAX_MODEL_TEXTURES;
	while (nNumTextures && (pObj->sd->m_pSkins[nNumTextures - 1] == 0))
		--nNumTextures;

	cPacket.WriteBits(nNumTextures, FNumBits<MAX_MODEL_TEXTURES>::k_nValue);
	for (uint32 nCurTexture = 0; nCurTexture < nNumTextures; nCurTexture++) 
	{
		pUsedFile = pObj->sd->m_pSkins[nCurTexture];
		cPacket.Writeuint16(pUsedFile ? (uint16)pUsedFile->m_FileID : (uint16)-1); 
	}

	uint32 nNumRenderStyles = MAX_MODEL_RENDERSTYLES;
	while (nNumRenderStyles && (pObj->sd->m_pRenderStyles[nNumRenderStyles - 1] == 0))
		--nNumRenderStyles;

	cPacket.WriteBits(nNumRenderStyles, FNumBits<MAX_MODEL_RENDERSTYLES>::k_nValue);
	for (uint32 nCurRS = 0; nCurRS < nNumRenderStyles; nCurRS++) 
	{
		pUsedFile = pObj->sd->m_pRenderStyles[nCurRS];
		cPacket.Writeuint16(pUsedFile ? (uint16)pUsedFile->m_FileID : (uint16)-1);
	}

	return true;
}

void sm_WriteChangedModelFiles(LTObject *pObj, CPacket_Write &cPacket, ObjectCreateStruct* pStruct)
{
	Model *pModel = pObj->ToModel()->GetModelDB();

	// Output the base filename if necessary
	if (pStruct->m_Filename[0] != '\0')
	{
		// File type and index
		cPacket.Writeuint8((uint8)eObjectResource_ObjectFile);
		cPacket.Writeuint8(0);

		if( pModel )
		{
			cPacket.Writeuint16((uint16)pModel->m_FileID);
			ASSERT( pModel->m_FileID != 0xFFFF );
		}
		else
		{
			ASSERT( !"sm_WriteChangedModelFiles:  Writing modelinstance without a model." );
			DEBUG_PRINT(1, ("sm_WriteChangedModelFiles:  Writing modelinstance without a model." ));

			// Note : I'd prefer to use a constant here instead of 0xFFFF, but I
			// can't find one.  Other code uses this magic number for invalid
			// file ID's.  And since all my friends are doing it, that makes it OK.
			cPacket.Writeuint16(0xFFFF);
		}
	}

	// Transmit child models

	if (pModel)
	{
		uint32 num_child = pModel->NumChildModels();
		for (uint32 nCurChild = 1 ; (nCurChild < num_child); nCurChild++)
		{
			cPacket.Writeuint8((uint8)eObjectResource_ChildModel);
			cPacket.Writeuint8((uint8)nCurChild);
		
			cPacket.Writeuint16((uint16)pModel->GetChildModel(nCurChild)->m_pModel->m_FileID);
		}	
	}

	for (uint32 nCurTexture = 0; nCurTexture < MAX_MODEL_TEXTURES; nCurTexture++) 
	{
		if (pStruct->m_SkinNames[nCurTexture][0] == '\0')
			continue;

		cPacket.Writeuint8((uint8)eObjectResource_Texture);
		cPacket.Writeuint8((uint8)nCurTexture);

		UsedFile* pUsedFile = pObj->sd->m_pSkins[nCurTexture];
		cPacket.Writeuint16(pUsedFile ? (uint16)pUsedFile->m_FileID : (uint16)-1); 
	}

	for (uint32 nCurRS = 0; nCurRS < MAX_MODEL_RENDERSTYLES; nCurRS++)
	{
		if (pStruct->m_RenderStyleNames[nCurRS][0] == '\0')
			continue;

		cPacket.Writeuint8((uint8)eObjectResource_RenderStyle);
		cPacket.Writeuint8((uint8)nCurRS);

		UsedFile* pUsedFile = pObj->sd->m_pRenderStyles[nCurRS];
		cPacket.Writeuint16(pUsedFile ? (uint16)pUsedFile->m_FileID : (uint16)-1);
	}

	// Output the 'no more' token
	cPacket.Writeuint8(0xFF);
}

// ----------------------------------------------------------------------- //
// Writes the model animation info into the packet.
// ----------------------------------------------------------------------- //
void WriteAnimInfo(ModelInstance *pInst, CPacket_Write &cPacket)
{
	for (LTAnimTracker *pTracker = pInst->m_AnimTrackers; pTracker; pTracker=pTracker->GetNext())
    {
		if (!pTracker->IsValid())
		{
			// Don't send anything if the main tracker isn't valid
			if (pTracker == pInst->m_AnimTrackers)
			{
				break;
			}
			continue;
		}

		// set this true for the first tracker
		cPacket.Writebool(true);

		if(g_CV_ModelOnlyUpdateDirtyTrackers)
		{
			//If this is the main tracker, then send the info
			if(pTracker == pInst->m_AnimTrackers)
			{
				cPacket.Writebool(true);
			}
			else // We're a user-made tracker
			{
				// Are we dirty?
				if(pTracker->m_bDirty)
				{
					cPacket.Writebool(true);
					pTracker->m_bDirty = false;
				}
				else
				{
					// We don't need to do a full update, we haven't changed
					cPacket.Writebool(false);
				}
			}
		}
		else
		{
			cPacket.Writebool(true);
		}
		
	    uint16 nAnimIndex = (uint16)trk_GetCurAnimIndex(pTracker);
		bool bLongModelInfo = (nAnimIndex >= (1 << MODELINFO_ANIMINDEX_SHORT)) || ((pTracker->m_Flags & AT_PLAYING) == 0);
		cPacket.Writebool(bLongModelInfo);
		if (bLongModelInfo)
		{
			cPacket.WriteBits(nAnimIndex, MODELINFO_ANIMINDEX_LONG);
			cPacket.Writebool((pTracker->m_Flags & AT_PLAYING) != 0);
		}
		else
		{
			cPacket.WriteBits(nAnimIndex, MODELINFO_ANIMINDEX_SHORT);
		}
		cPacket.Writebool((pTracker->m_Flags & AT_LOOPING) != 0);

		//by default it should fall into the first category
		uint32 animLength		= MODELINFO_ANIMTIME_SIZE0;
		uint32 animLengthType	= 0;

	    uint32 animTime = 0;

	    ModelAnim *pAnim = pTracker->GetCurAnim();
        if (pAnim)
        {
			uint32 nScaledAnimTime = (pAnim->GetAnimTime() + (MODELINFO_ANIMTIME_RES - 1)) / MODELINFO_ANIMTIME_RES;
            animTime = (pTracker->m_TimeRef.m_Cur.m_Time + (MODELINFO_ANIMTIME_RES - 1)) / MODELINFO_ANIMTIME_RES;


			
			//quick lookup table of the sizes of the boundary values of the size
			const uint32 aMaxAnimLengths[4] = { 
				MODELINFO_ANIMTIME_MIN / MODELINFO_ANIMTIME_RES,
				(1 << MODELINFO_ANIMTIME_SIZE1) - 1,
				(1 << MODELINFO_ANIMTIME_SIZE2) - 1,
				//(1 << MODELINFO_ANIMTIME_SIZE3) - 1
				0xFFFFFFFF					// [dlj] hardcoding because of warnings (1 << MODELINFO_ANIMTIME_SIZE3) - 1;
			};

			if (nScaledAnimTime > aMaxAnimLengths[2])
			{
				animLength = MODELINFO_ANIMTIME_SIZE3;
				animLengthType = 3;
			}
			else if (nScaledAnimTime > aMaxAnimLengths[1])
			{
				animLength = MODELINFO_ANIMTIME_SIZE2;
				animLengthType = 2;
			}
			else if (nScaledAnimTime > aMaxAnimLengths[0])
			{
				animLength = MODELINFO_ANIMTIME_SIZE1;
				animLengthType = 1;
			}
        }

		cPacket.WriteBits(animLengthType, 2);
		cPacket.WriteBits(animTime, animLength);
        
        if (pTracker != pInst->m_AnimTrackers)
        {
			cPacket.Writeuint8((uint8)pTracker->m_ID);

			bool bLargeWeightSet = (pTracker->m_TimeRef.m_iWeightSet >= (1 << MODELINFO_WEIGHTSET_SHORT));
			cPacket.Writebool(bLargeWeightSet);
			cPacket.WriteBits(pTracker->m_TimeRef.m_iWeightSet, bLargeWeightSet ? MODELINFO_WEIGHTSET_LONG : MODELINFO_WEIGHTSET_SHORT);
        }
		else
		{
			ASSERT(pTracker->m_ID == MAIN_TRACKER);
		}

		bool bWriteRateModifier = (pTracker->m_RateModifier != MODELINFO_RATEMODIFIER_DEFAULT);
		cPacket.Writebool(bWriteRateModifier);
		if (bWriteRateModifier)
			cPacket.Writefloat(pTracker->m_RateModifier);
    }

	cPacket.Writebool(false);
}

// ----------------------------------------------------------------------- //
// Looks at the flags in pInfo and fills the packet with update data.
// ----------------------------------------------------------------------- //
bool FillPacketFromInfo(Client *pClient, LTObject *pObj, ObjInfo *pInfo, CPacket_Write &cPacket)
{
    // Does the client's object want its rotations sent?
    uint16 changeFlags = pInfo->m_ChangeFlags;
    if (!(pClient->m_ClientFlags & CFLAG_SENDCOBJROTATION) && pClient->m_pObject == pObj)
        changeFlags &= ~(CF_ROTATION|CF_SNAPROTATION);

    // Clear certain flags if we'll be sending that stuff unguaranteed.
    if (pObj->sd->m_NetFlags & NETFLAG_POSUNGUARANTEED)
    {
        changeFlags &= ~CF_POSITION;
    }

    if (pObj->sd->m_NetFlags & NETFLAG_ROTUNGUARANTEED)
    {
        changeFlags &= ~CF_ROTATION;
    }

    if (pObj->m_ObjectType == OT_MODEL && pObj->sd->m_NetFlags & NETFLAG_ANIMUNGUARANTEED)
    {
        changeFlags &= ~CF_MODELINFO;
    }

	// Turn on the model info flag if it's being forced
	if (changeFlags & CF_FORCEMODELINFO)
		changeFlags |= CF_MODELINFO;

    // If it's a new object the client will teleport it automatically.
    if (changeFlags & CF_NEWOBJECT)
    {
        changeFlags |= CF_POSITION;
        changeFlags &= ~CF_TELEPORT;
		changeFlags &= ~CF_FILENAMES;

		// If they got flags, we'll need to send those too.
		if (pObj->m_Flags & CLIENT_FLAGMASK || pObj->m_UserFlags != 0)
		{
			changeFlags |= CF_FLAGS;
	    }

    }

	// If it needs a position cap, set the local position change flag
	if (changeFlags & CF_POSITION_PREDICTIONCAP)
	{
		changeFlags |= CF_POSITION;
	}

    // Only send color info for objects that will use it.
    if (changeFlags & CF_RENDERINFO)
    {
        if (pObj->m_ObjectType == OT_NORMAL || pObj->m_ObjectType == OT_CONTAINER ||
            pObj->m_ObjectType == OT_CAMERA)
        {
            changeFlags &= ~CF_RENDERINFO;
        }
    }

	// Don't send the dims for objects that don't care about that.
	if ((changeFlags & CF_DIMS) && ((pObj->m_Flags2 & FLAG2_SERVERDIMS) == 0))
	{
		changeFlags &= ~CF_DIMS;
	}
	 
	if (!changeFlags)
        return false;

    // Write the header.
    if (changeFlags & CF_OTHERFLAGMASK)
    {
		cPacket.Writeuint8((uint8)(changeFlags | CF_OTHER));
		cPacket.Writeuint8((uint8)(changeFlags >> 8));
    }
    else
    {
		cPacket.Writeuint8((uint8)changeFlags);
    }

    // Write the object ID
	cPacket.Writeuint16(pObj->m_ObjectID);

    // Add some extra info if it's a new object.
    if (changeFlags & CF_NEWOBJECT)
    {
		uint8 nObjectType = pObj->m_ObjectType;

		const CPacket_Read &cSFXMsg = pObj->sd->m_cSpecialEffectMsg;
		if (!cSFXMsg.Empty())
		{
			nObjectType |= SFXMESSAGE_FLAG;
            if (cSFXMsg.Size() >= 256)
                nObjectType |= LONGSFXMESSAGE_FLAG;
        }

		cPacket.Writeuint8(nObjectType);

		// [KLS 3/12/02] - Added support for GlobalForceOverride
		cPacket.WriteLTVector(pObj->GetGlobalForceOverride());

        // Write its special effect info.
        if (!cSFXMsg.Empty())
		{
			cPacket.WriteBits(cSFXMsg.Size(), (cSFXMsg.Size() >= 256) ? 16 : 8);
            cPacket.WritePacket(cSFXMsg);
        }
    
        // Write the WorldModel name or the filename.
        if (pObj->m_ObjectType == OT_WORLDMODEL)
        {
			cPacket.WriteString(ToWorldModel(pObj)->m_pOriginalBsp->m_WorldName);
        }
        else if (pObj->m_ObjectType == OT_CONTAINER)
        {
			cPacket.WriteString(ToContainer(pObj)->m_pOriginalBsp->m_WorldName);
			cPacket.Writeuint16(ToContainer(pObj)->m_ContainerCode);
        }
        else if (pObj->m_ObjectType == OT_MODEL)
        {
            if( !sm_WriteModelFiles(pObj, cPacket))
				return false;
        }
        else if (pObj->m_ObjectType == OT_SPRITE)
        {
            ASSERT(pObj->sd->m_pFile);
			cPacket.Writeuint16((uint16)pObj->sd->m_pFile->m_FileID);
        }
    }

    // Write the filename changes?
    if( changeFlags & CF_FILENAMES )
    {
        if (pObj->m_ObjectType == OT_MODEL)
        {
            if( !sm_WriteModelFiles(pObj, cPacket))
				return false;
        }
        else if (pObj->m_ObjectType == OT_SPRITE)
        {
            ASSERT(pObj->sd->m_pFile);
			cPacket.Writeuint16((uint16)pObj->sd->m_pFile->m_FileID);
        }
    }

    // Write the model info?
    if (changeFlags & (CF_MODELINFO|CF_FORCEMODELINFO))
    {
        if (pObj->m_ObjectType == OT_MODEL)
        {
            WriteAnimInfo(ToModel(pObj), cPacket);
        }
        else if (pObj->m_ObjectType == OT_SPRITE)
        {
			cPacket.WriteType(ToSprite(pObj)->m_ClipperPoly);
        }
    }

    // Write which things have changed.
    if (changeFlags & CF_FLAGS)
    {
		cPacket.Writeuint16((uint16)(pObj->m_Flags & CLIENT_FLAGMASK));
		cPacket.Writeuint16((uint16)pObj->m_Flags2);
		cPacket.Writeuint32(pObj->m_UserFlags);
    }

    if (changeFlags & CF_RENDERINFO)
    {
		cPacket.Writeuint8(pObj->m_ColorR);
		cPacket.Writeuint8(pObj->m_ColorG);
		cPacket.Writeuint8(pObj->m_ColorB);
		cPacket.Writeuint8(pObj->m_ColorA);
		cPacket.Writeuint8(pObj->m_nRenderGroup);

        if (pObj->m_ObjectType == OT_LIGHT)
        {
			cPacket.Writeuint16((uint16)LTMAX(ToDynamicLight(pObj)->m_LightRadius, 0.0f));
        }
    }

    if (changeFlags & CF_SCALE)
    {
		cPacket.Writefloat(pObj->m_Scale.x);
		cPacket.Writefloat(pObj->m_Scale.y);
        
        if (pObj->m_ObjectType != OT_SPRITE)
        {
			cPacket.Writefloat(pObj->m_Scale.z);
        }
    }

 
    if (changeFlags & (CF_POSITION|CF_TELEPORT|CF_POSITION_PREDICTIONCAP))
    {
		LTVector vObjectVelocity = pObj->m_Velocity;

		// Turn off the prediction cap if we're sending the prediction cap position message
		if ((pInfo->m_ChangeFlags & CF_POSITION) == 0)
		{
			pInfo->m_ChangeFlags &= ~CF_POSITION_PREDICTIONCAP;
			pInfo->m_ChangeFlags |= CF_POSITION;
			vObjectVelocity.Init();
		}
		// Otherwise turn it on
		else
		{
			pInfo->m_ChangeFlags |= CF_POSITION_PREDICTIONCAP;
		}

        if (pObj->m_Flags & FLAG_FULLPOSITIONRES)
        {
			cPacket.WriteLTVector(pObj->GetPos());
			cPacket.WriteLTVector(vObjectVelocity);
        }
        else 
		{
			CLTMessage_Write_Server::WriteCompPos(cPacket, pObj->GetPos());
			CLTMessage_Write::WriteCompLTVector(cPacket, vObjectVelocity);
        }
    }


    if (changeFlags & (CF_ROTATION|CF_SNAPROTATION))
    {
		if (pObj->m_Flags & FLAG_FULLPOSITIONRES)
        {
			cPacket.WriteType(pObj->m_Rotation);
        }
        else
        {
			CLTMessage_Write::WriteCompLTRotation(cPacket, pObj->m_Rotation);
        }
    }

    if (changeFlags & CF_ATTACHMENTS)
    {
        // Write the attachments.
	    Attachment *pCur = pObj->m_Attachments;
        while (pCur)
        {
			cPacket.Writeuint16(pCur->m_nChildID);
			cPacket.Writeuint32(pCur->m_iSocket);

            // Not sending position compressed because it may be important that this is right on...
			cPacket.WriteLTVector(pCur->m_Offset.m_Pos);

			CLTMessage_Write::WriteCompLTRotation(cPacket, pCur->m_Offset.m_Rot);

            pCur = pCur->m_pNext;
        }

		cPacket.Writeuint16(INVALID_OBJECTID);

        // Write the hidden piecelist.
        if (pObj->m_ObjectType == OT_MODEL)
        {
			// THERE IS A LOT OF ROOM FOR IMPROVEMENT HERE!
			for (uint32 nCurrPiece = 0; nCurrPiece < MAX_PIECES_PER_MODEL / 32; nCurrPiece++)
			{
				cPacket.Writeuint32(ToModel(pObj)->m_HiddenPieces[nCurrPiece]);
			}
        }
    }

	if (changeFlags & CF_DIMS)
	{
		// Write the dims
		cPacket.WriteLTVector(pObj->GetDims());
	}

    return true;
}


// ----------------------------------------------------------------------- //
// Sends the given packet to everyone.
// ----------------------------------------------------------------------- //
void SendServerMessage(const CPacket_Read &cPacket, uint32 packetFlags)
{
    LTLink *pCur, *pListHead;
    Client *pClient;

    pListHead = &g_pServerMgr->m_Clients.m_Head;  
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
    {
        pClient = (Client*)pCur->m_pData;
        
        if (pClient->m_ConnectionID)
        {
            g_pServerMgr->m_NetMgr.SendPacket(cPacket, pClient->m_ConnectionID, packetFlags);
        }
    }
}


// ----------------------------------------------------------------------- //
// Sends the given packet to the given client.
// ----------------------------------------------------------------------- //
void SendToClient(Client *pClient, 
    const CPacket_Read &cPacket, bool bSendToAttachments, uint32 packetFlags)
{
    // Send to the client.
    if (pClient->m_ConnectionID)
    {
		if (!g_pServerMgr->m_NetMgr.SendPacket(cPacket, pClient->m_ConnectionID, packetFlags))
		{
			// Track the lost send packet count
			g_pServerMgr->m_nDroppedSendPackets++;
		}
		// Track the total packet count
		g_pServerMgr->m_nSendPackets++;
    }

    // Send to attachments.
    if (bSendToAttachments)
    {
	    LTLink *pCur;
        for (pCur = pClient->m_Attachments.m_pNext; pCur != &pClient->m_Attachments; pCur=pCur->m_pNext)
        {
		    Client *pAttachment = (Client*)pCur->m_pData;

            if (pAttachment->m_ConnectionID)
            {
	            g_pServerMgr->m_NetMgr.SendPacket(cPacket, pAttachment->m_ConnectionID, packetFlags);
            }
        }
    }
}


void SendToClientsInWorld(const CPacket_Read &cPacket)
{
    LTLink *pListHead = &g_pServerMgr->m_Clients.m_Head;  
    for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
    {
		Client *pClient = (Client*)pCur->m_pData;
        
        if (pClient->m_ConnectionID && pClient->m_State == CLIENT_INWORLD)
        {
            g_pServerMgr->m_NetMgr.SendPacket(cPacket, pClient->m_ConnectionID, MESSAGE_GUARANTEED);
        }
    }
}


// ----------------------------------------------------------------------- //
// Creates an event of the specified type and adds it to the client's structures.
// ----------------------------------------------------------------------- //

CServerEvent* CreateServerEvent(int type)
{
	// Don't do anything if there aren't any clients in the world
	if (!g_pServerMgr->m_Clients.m_nElements)
		return LTNULL;

    CServerEvent *pRet = (CServerEvent*)sb_Allocate(&g_pServerMgr->m_ServerEventBank);
    memset(pRet, 0, sizeof(*pRet));
    dl_InitList(&pRet->m_ClientStructNodeList);

    // Add the events to the client structs.
    uint16 curEvent = 0;
    
    LTLink *pListHead = &g_pServerMgr->m_Clients.m_Head;
    for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
    {
	    Client *pClient = (Client*)pCur->m_pData;
        if (pClient->m_State != CLIENT_INWORLD)
            continue;

	    ClientStructNode *pNode = (ClientStructNode *)sb_Allocate(&g_pServerMgr->m_ClientStructNodeBank);
        dl_AddHead(&pRet->m_ClientStructNodeList, &pNode->m_Link, pNode);
        dl_AddHead(&pClient->m_Events, &pNode->m_mllNode, pRet);
        pRet->m_RefCount++;
    }

    pRet->m_EventType = type;

    return pRet;
}

// ----------------------------------------------------------------------- //
// Creates an event of the specified type and adds it to a single client's structures.
// ----------------------------------------------------------------------- //

CServerEvent* CreateServerToClientEvent(int type, Client *pClient)
{
    CServerEvent *pRet=LTNULL;
    uint16 curEvent;
    ClientStructNode *pNode;
    
    if (pClient->m_State != CLIENT_INWORLD)
        return LTNULL;

    pRet = (CServerEvent*)sb_Allocate(&g_pServerMgr->m_ServerEventBank);
    memset(pRet, 0, sizeof(*pRet));

    dl_InitList(&pRet->m_ClientStructNodeList);

    // Add the events to the client structs.
    curEvent=0;
    
    pNode = (ClientStructNode *)sb_Allocate(&g_pServerMgr->m_ClientStructNodeBank);
    dl_AddHead(&pRet->m_ClientStructNodeList, &pNode->m_Link, pNode);
    dl_AddHead(&pClient->m_Events, &pNode->m_mllNode, pRet);

    pRet->m_EventType = type;
    pRet->m_RefCount = 1;

    return pRet;
}

// Set the change flags based on what's different...
void GetSoundFileIDInfoFlags(FileIDInfo *pFileIDInfo, FileIDInfo *pCurrent)
{
    if (pFileIDInfo->m_nChangeFlags & FILEIDINFOF_SOUNDPLAYSOUNDFLAGS || pFileIDInfo->m_wSoundPlaySoundFlags != pCurrent->m_wSoundPlaySoundFlags)
    {
        pFileIDInfo->m_nChangeFlags |= FILEIDINFOF_SOUNDPLAYSOUNDFLAGS;
        pFileIDInfo->m_wSoundPlaySoundFlags = pCurrent->m_wSoundPlaySoundFlags;
    }
    if (pFileIDInfo->m_nChangeFlags & FILEIDINFOF_SOUNDPRIORITY || pFileIDInfo->m_nSoundPriority != pCurrent->m_nSoundPriority)
    {
        pFileIDInfo->m_nChangeFlags |= FILEIDINFOF_SOUNDPRIORITY;
        pFileIDInfo->m_nSoundPriority = pCurrent->m_nSoundPriority;
    }
    if (pFileIDInfo->m_nChangeFlags & FILEIDINFOF_RADIUS || pFileIDInfo->m_nSoundOuterRadius != pCurrent->m_nSoundOuterRadius || pFileIDInfo->m_nSoundInnerRadius != pCurrent->m_nSoundInnerRadius)
    {
        pFileIDInfo->m_nChangeFlags |= FILEIDINFOF_RADIUS;
        pFileIDInfo->m_nSoundOuterRadius = pCurrent->m_nSoundOuterRadius;
        pFileIDInfo->m_nSoundInnerRadius = pCurrent->m_nSoundInnerRadius;
    }
}
            
void FillInPlaysoundMessage(CServerEvent *pEvent, Client *pClient, CPacket_Write &cPacket)
{
    ASSERT(pEvent);
    ASSERT(pEvent->m_pUsedFile);

    if (!pEvent || !pEvent->m_pUsedFile)
        return;

    PlaySoundInfo *pPlaySoundInfo = &pEvent->m_PlaySoundInfo;
    uint16 wFlags = (uint16)(0xFFFF & pPlaySoundInfo->m_dwFlags);

    // Check if the sound is supposed to be played locally to a client object...
    bool bLocalOverride = false;
    if (wFlags & PLAYSOUND_CLIENTLOCAL)
    {
        if (HandleToServerObj(pPlaySoundInfo->m_hObject) == pClient->m_pObject)
        {
            bLocalOverride = true;
        }
        else
        {
            wFlags &= ~PLAYSOUND_CLIENTLOCAL;
            // Play it as a 3D sound on the non-local clients
            wFlags |= PLAYSOUND_3D;
        }
    }

    // Check if sound is in range...
    if (wFlags & (PLAYSOUND_3D | PLAYSOUND_AMBIENT))
    {
		// [RP] 8/29/02 Check the distance from the clients view position rather than the clients object
		//      position since the view pos is a more accurate representation of where the client should
		//		hear the sound.

		const LTVector3f d = pPlaySoundInfo->m_vPosition - pClient->m_ViewPos;

        if (d.LengthSqr() > 4.0f * pPlaySoundInfo->m_fOuterRadius * pPlaySoundInfo->m_fOuterRadius)
        {
            return;
        }
    }

    // Signal the event subpacket.
	cPacket.Writeuint8(0);
    cPacket.Writeuint8(UPDATESUB_PLAYSOUND);
	cPacket.Writeuint16((uint16)pEvent->m_pUsedFile->m_FileID);
	
	uint16 nOuter = ( uint16 )LTMIN( pPlaySoundInfo->m_fOuterRadius, 65535.0f );
	uint16 nInner = LTMIN(( uint16 )( pPlaySoundInfo->m_fInnerRadius * 255.0f / nOuter ), 255 );

	// Some of the info for sounds using the same file don't change from instance to instance, so
	// the server remembers what it sent to the client for a file and only sends the changed info...
	// Get the info we sent to this client based on file id...
    FileIDInfo *pFileIDInfo, fileIDInfoCurrent;
	pFileIDInfo = sm_GetClientFileIDInfo( pClient, (uint16)pEvent->m_pUsedFile->m_FileID);
	if (!pFileIDInfo)
	{
		// Should never get here, but we gotta do our error detection now don't we...
		fileIDInfoCurrent.m_nChangeFlags = FILEIDINFOF_NEWMASK;
		pFileIDInfo = &fileIDInfoCurrent;
	}
	else
	{
		// Compare the current values with the value we sent last time and create change flags...
		fileIDInfoCurrent.m_wSoundPlaySoundFlags = wFlags;
		fileIDInfoCurrent.m_nSoundPriority = pPlaySoundInfo->m_nPriority;
		fileIDInfoCurrent.m_nSoundOuterRadius = (uint16)nOuter;
		fileIDInfoCurrent.m_nSoundInnerRadius = (uint8)nInner;
		GetSoundFileIDInfoFlags( pFileIDInfo, &fileIDInfoCurrent );
	}

    // Write the change flags for the file dependent info...
	cPacket.Writeuint8(pFileIDInfo->m_nChangeFlags);

    if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_SOUNDPLAYSOUNDFLAGS))
    {
        // Only write a word of the flags, since upper word isn't being used yet...
        pFileIDInfo->m_nChangeFlags &= ~FILEIDINFOF_SOUNDPLAYSOUNDFLAGS;
		cPacket.Writeuint16(wFlags);
    }
    
    if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_SOUNDPRIORITY))
    {
        pFileIDInfo->m_nChangeFlags &= ~FILEIDINFOF_SOUNDPRIORITY;
		cPacket.Writeuint8(pPlaySoundInfo->m_nPriority);
    }

    if (wFlags & (PLAYSOUND_AMBIENT | PLAYSOUND_3D))
    {
        if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_RADIUS))
        {
            pFileIDInfo->m_nChangeFlags &= ~FILEIDINFOF_RADIUS;
			cPacket.Writeuint16((uint16)nOuter);
			cPacket.Writeuint8((uint8)nInner);
        }
    }

    if (wFlags & PLAYSOUND_CTRL_VOL)
		cPacket.Writeuint8(pPlaySoundInfo->m_nVolume);
    
    if (wFlags & PLAYSOUND_CTRL_PITCH)
		cPacket.Writefloat(pPlaySoundInfo->m_fPitchShift);

    if (!bLocalOverride && (wFlags & (PLAYSOUND_AMBIENT | PLAYSOUND_3D)))
    {
		CLTMessage_Write_Server::WriteCompPos(cPacket, pPlaySoundInfo->m_vPosition);
    }

	cPacket.Writeuint32(pPlaySoundInfo->m_UserData);
}

void FillSoundTrackPacketFromInfo_Internal(CSoundTrack *pSoundTrack, ObjInfo *pInfo, Client *pClient, CPacket_Write &cPacket)
{
    uint16 wFlags = (uint16)(0xFFFF & pSoundTrack->m_dwFlags);

    // If the sound is attached to this client's object, then we can assume certain things about the position
    // and orientation and not have to send a message for position and orientation changes...
    bool bLocalOverride = false;
    if (wFlags & (PLAYSOUND_ATTACHED | PLAYSOUND_CLIENTLOCAL))
    {
        if (pSoundTrack->GetObject() == pClient->m_pObject)
        {
            wFlags |= PLAYSOUND_CLIENTLOCAL;
            pInfo->m_ChangeFlags &= ~(CF_POSITION);
            bLocalOverride = true;
        }
        else
        {
            if (wFlags & PLAYSOUND_CLIENTLOCAL)
            {
                // Make sure it sends the location of the client making the sound
                pSoundTrack->m_vPosition = pSoundTrack->GetObject()->GetPos();
                pInfo->m_ChangeFlags |= CF_POSITION;
            }
            // Not attached to the client object, so this client doesn't get the PLAYSOUND_CLIENTLOCAL flag...
            wFlags &= ~PLAYSOUND_CLIENTLOCAL;
            // But it does need to be 3D at that point..
            wFlags |= PLAYSOUND_3D;
        }
    }

	cPacket.Writeuint8(0);
	cPacket.Writeuint8(UPDATESUB_SOUNDTRACK);

    // Send the change flags...
	cPacket.Writeuint8((uint8)(pInfo->m_ChangeFlags));

    // Send the id for this sound...
    cPacket.Writeuint16((uint16)GetLinkID(pSoundTrack->m_pIDLink));

	// Handle the new sound info...
	if (pInfo->m_ChangeFlags & CF_NEWOBJECT)
	{
		pSoundTrack->AddRef();
		cPacket.Writeuint16((uint16)pSoundTrack->m_pFile->m_FileID);

	    uint16 nOuter = (uint16)LTMIN(pSoundTrack->m_fOuterRadius, 65535.0f);
	    uint16 nInner = LTMIN((uint8)(pSoundTrack->m_fInnerRadius * 255.0f / nOuter), 255);

        // Some of the info for sounds using the same file don't change from instance to instance, so
        // the server remembers what it sent to the client for a file and only sends the changed info...

		// Get the info we sent to this client based on file id...
	    FileIDInfo *pFileIDInfo, fileIDInfoCurrent;
		pFileIDInfo = sm_GetClientFileIDInfo( pClient, (uint16)pSoundTrack->m_pFile->m_FileID );
		if( !pFileIDInfo )
		{
			// Should never get here, but we gotta do our error detection now don't we...
			fileIDInfoCurrent.m_nChangeFlags = FILEIDINFOF_NEWMASK;
			pFileIDInfo = &fileIDInfoCurrent;
		}
		else
		{
			// Compare the current values with the value we sent last time and create change flags...
			fileIDInfoCurrent.m_wSoundPlaySoundFlags = wFlags;
			fileIDInfoCurrent.m_nSoundPriority = pSoundTrack->m_nPriority;
			fileIDInfoCurrent.m_nSoundOuterRadius = (uint16)nOuter;
			fileIDInfoCurrent.m_nSoundInnerRadius = (uint8)nInner;
			GetSoundFileIDInfoFlags( pFileIDInfo, &fileIDInfoCurrent );
		}

        // Write the change flags for the file dependent info...
        cPacket.Writeuint8(pFileIDInfo->m_nChangeFlags);

        if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_SOUNDPLAYSOUNDFLAGS))
        {
            // Only write a word of the flags, since upper word isn't being used yet...
            pFileIDInfo->m_nChangeFlags &= ~FILEIDINFOF_SOUNDPLAYSOUNDFLAGS;
			cPacket.Writeuint16(wFlags);
        }
        
        if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_SOUNDPRIORITY))
        {
            pFileIDInfo->m_nChangeFlags &= ~FILEIDINFOF_SOUNDPRIORITY;
            cPacket.Writeuint8(pSoundTrack->m_nPriority);
        }

        // Send the radii info.  The client object will hear the sound with infinite radii...
        if (wFlags & (PLAYSOUND_AMBIENT | PLAYSOUND_3D))
        {
            if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_RADIUS))
            {
                pFileIDInfo->m_nChangeFlags &= ~FILEIDINFOF_RADIUS;
				cPacket.Writeuint16((uint16)nOuter);
				cPacket.Writeuint8((uint8)nInner);
            }
        }

        if (wFlags & PLAYSOUND_CTRL_VOL)
        {
			cPacket.Writeuint8(pSoundTrack->m_nVolume);
        }
        
        if (wFlags & PLAYSOUND_CTRL_PITCH)
        {
			cPacket.Writefloat(pSoundTrack->m_fPitchShift);
        }

        if (wFlags & PLAYSOUND_TIMESYNC)
        {
		    uint16 dwOffsetTime = (uint16)((1000.0 * (g_pServerMgr->m_GameTime - pSoundTrack->GetStartTime())) + 0.5);
            if (dwOffsetTime < 255)
            {
				cPacket.Writeuint8((uint8)dwOffsetTime);
            }
            else
            {
                cPacket.Writeuint8((uint8)0xFF);
                cPacket.Writeuint32(dwOffsetTime);
            }
        }

        // Send the user data
		cPacket.Writeuint32(pSoundTrack->m_UserData);
    }

    // Position info
    if (pInfo->m_ChangeFlags & CF_POSITION) 
	{
		CLTMessage_Write_Server::WriteCompPos(cPacket, pSoundTrack->m_vPosition);
    }
}

void FillSoundTrackPacketFromInfo(CSoundTrack *pSoundTrack, ObjInfo *pInfo, Client *pClient, CPacket_Write &cPacket)
{
	CPacket_Write cSubPacket;
	FillSoundTrackPacketFromInfo_Internal(pSoundTrack, pInfo, pClient, cSubPacket);
	cPacket.Writeuint32(cSubPacket.Size());
	cPacket.WritePacket(CPacket_Read(cSubPacket));
}

// ----------------------------------------------------------------------- //
// Adds subpackets for each event for the client.
// ----------------------------------------------------------------------- //

void WriteEventToPacket(CServerEvent *pEvent, Client *pClient, CPacket_Write &cPacket)
{
	CPacket_Write cSubPacket;

    switch(pEvent->m_EventType)
    {
        case EVENT_PLAYSOUND:
        {
            FillInPlaysoundMessage(pEvent, pClient, cSubPacket);
            break;
        }
        default:
        {
            ASSERT(false);
            break;
        }
    }

	if (!cSubPacket.Empty())
	{
		cPacket.Writeuint32(cSubPacket.Size());
		cPacket.WritePacket(CPacket_Read(cSubPacket));
	}
}


// ----------------------------------------------------------------------- //
// Processes an incoming packet from the network.
// ----------------------------------------------------------------------- //

LTRESULT ProcessIncomingPacket(Client * pFromClient, const CPacket_Read &cPacket)
{
	// Fish out the packet ID
	CPacket_Read cSubPacket(cPacket);
    uint8 packetID = cSubPacket.Readuint8();
	cSubPacket = CPacket_Read(cSubPacket, cSubPacket.Tell());

    LTRESULT dResult = LT_OK;
    ServerPacketHandler * pHandler = &g_ServerHandlers[packetID];
    if (pHandler->m_Fn)
	{
        dResult = (pHandler->m_Fn)(cSubPacket, pFromClient);
	}
    else
    {   
        // let the file transfer manager have the packet (including the ID...)
        if (pFromClient)
            fts_ProcessPacket(pFromClient->m_hFTServ, cPacket);
    }
    return dResult;
}


// ----------------------------------------------------------------------- //
// Reads in all packets from the net.
// ----------------------------------------------------------------------- //

LTRESULT ProcessIncomingPackets() {
    LTRESULT dResult = LT_OK;
    CPacket_Read cPacket;
	CBaseConn *pSender;
    
    g_pServerMgr->m_NetMgr.StartGettingPackets();
        
    while (g_pServerMgr->m_NetMgr.GetPacket(NETMGR_TRAVELDIR_CLIENT2SERVER, &cPacket, &pSender))
    {
        Client * pClient = (pSender ? sm_FindClient(pSender) : LTNULL);

        dResult = ProcessIncomingPacket(pClient, cPacket);
        
        if (dResult != LT_OK)
            break;             
    }

    g_pServerMgr->m_NetMgr.EndGettingPackets();
    return dResult;
}


// ----------------------------------------------------------------------- //
// Just sets up some pointers to functions for packet receivers.
// ----------------------------------------------------------------------- //

void InitServerNetHandlers()
{
    memset(g_ServerHandlers, 0, sizeof(g_ServerHandlers));

    g_ServerHandlers[CMSG_GOODBYE].m_Fn = &OnClientDisconnectPacket;
    g_ServerHandlers[CMSG_UPDATE].m_Fn = &OnClientUpdatePacket;
    g_ServerHandlers[CMSG_SOUNDUPDATE].m_Fn = &OnSoundUpdatePacket;
    g_ServerHandlers[CMSG_COMMANDSTRING].m_Fn = &OnCommandStringPacket;
    g_ServerHandlers[CMSG_MESSAGE].m_Fn = &OnMessagePacket;
    g_ServerHandlers[CMSG_CONNECTSTAGE].m_Fn = &OnConnectStagePacket;
    g_ServerHandlers[CMSG_HELLO].m_Fn = &OnHelloPacket;
}


