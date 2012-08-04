//------------------------------------------------------------------
//
//  FILE      : ShellNet.cpp
//
//  PURPOSE   : 
//
//  CREATED   : February 25 1997
//                           
//  COPYRIGHT : Microsoft 1997 All Rights Reserved
//
//------------------------------------------------------------------

#include "bdefs.h"

#include "clientshell.h"
#include "clientmgr.h"
#include "packetdefs.h"
#include "sysinput.h"
#include "predict.h"
#include "sysstreamsim.h"
#include "clientde_impl.h"
#include "animtracker.h"
#include "console.h"
#include "ltpvalue.h"
#include "setupobject.h"
#include "impl_common.h"
#include "sysdebugging.h"
#include "render.h"
#include "iltmodel.h"
#include "ltobjectcreate.h"

#include "iltmessage.h"
#include "ltmessage_client.h"

#include "misctools.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);


extern int32 g_bLocalDebug;
extern int32 g_CV_UpdateRate;
extern int32 g_bDebugPackets;
extern int32 g_CV_BandwidthTargetClient;

// The main list of packet handlers.
struct ShellPacketHandler 
{
    LTRESULT (*fn)(CClientShell *pShell, CPacket_Read &cPacket);
};
ShellPacketHandler g_ShellHandlers[256];


// Convenience class for wrapping sub-packets in messages
class CSubMsg_Client : public CLTMsgRef_Read
{
public:
	CSubMsg_Client(const CPacket_Read &cPacket) :
		CLTMsgRef_Read(CLTMessage_Read_Client::Allocate_Client(CPacket_Read(cPacket, cPacket.Tell())))
	{}
	CSubMsg_Client(const CPacket_Read &cPacket, uint32 nLength) :
		CLTMsgRef_Read(CLTMessage_Read_Client::Allocate_Client(CPacket_Read(cPacket, cPacket.Tell(), nLength)))
	{}
};


// ------------------------------------------------------------------------- //
// Static helpers.
// ------------------------------------------------------------------------- //

static LTRESULT SwitchModelAnim(CClientShell *pShell, 
								ModelInstance *pInstance, 
								LTAnimTracker *pTracker, 
								uint32 animIndex,
								bool bLooping,
								bool bPlaying,
								uint32 animTime,
								float animRate, 
								bool bAllowTransition, 
								bool bAllowReset)
{
	
    trk_Loop(pTracker, bLooping);
    trk_Play(pTracker, bPlaying);

	pTracker->m_RateModifier = animRate;

    // Don't reset if they don't want.
    if (bPlaying && !bAllowReset && animIndex == trk_GetCurAnimIndex(pTracker)) 
	{
        return LT_OK;
    }
    
    trk_SetCurAnim(pTracker, animIndex, bAllowTransition);
    g_pClientMgr->UpdateModelDims(pInstance);

	if (bPlaying) 
	{
        trk_SetCurTime(pTracker, animTime, bAllowTransition);
    }
    else 
	{
        trk_SetAtKeyFrame(pTracker, animTime);
    }

    return LT_OK;
}


static void PrintPacketDebugInfo(LTObject *pObject, uint32 flags) 
{
    if (g_bDebugPackets == 2) 
	{
        con_WhitePrintf("");
        con_WhitePrintf("ObjectID: %d, type %d, flags %d", 
            pObject->m_ObjectID, pObject->m_ObjectType, flags);

        if (flags & CF_NEWOBJECT)						con_WhitePrintf("CF_NEWOBJECT");
        if (flags & CF_POSITION)						con_WhitePrintf("CF_POSITION");
        if (flags & CF_ROTATION)						con_WhitePrintf("CF_ROTATION");
        if (flags & CF_FLAGS)							con_WhitePrintf("CF_FLAGS");
        if (flags & CF_SCALE)							con_WhitePrintf("CF_SCALE");
        if (flags & (CF_MODELINFO|CF_FORCEMODELINFO))   con_WhitePrintf("CF_MODELINFO");
        if (flags & CF_RENDERINFO)						con_WhitePrintf("CF_RENDERINFO");
        if (flags & CF_ATTACHMENTS)						con_WhitePrintf("CF_ATTACHMENTS");
        if (flags & CF_FILENAMES)						con_WhitePrintf("CF_FILENAMES");
        if (flags & CF_DIMS)							con_WhitePrintf("CF_DIMS");
    }
    else if (g_bDebugPackets == 1) 
	{
        if (flags & CF_NEWOBJECT) 
		{
            con_WhitePrintf("");
            con_WhitePrintf("ObjectID: %d, type: %d, flags %d", 
                pObject->m_ObjectID, pObject->m_ObjectType, flags);

            if (flags & CF_NEWOBJECT) 
			{
                con_WhitePrintf("CF_NEWOBJECT");
            }
        }
    }
}

//tags all the trackers on the given model instance as needing to be removed
static void TagTrackersForRemove(ModelInstance* pInst)
{
	//we first need to tag every tracker as one that needs to be removed
	for(LTAnimTracker* pTagAsRemove = pInst->m_AnimTrackers; pTagAsRemove; pTagAsRemove = pTagAsRemove->GetNext())
	{
		//make sure to never remove the main tracker though...
		if(pTagAsRemove->m_ID != MAIN_TRACKER)
			pTagAsRemove->m_Flags |= AT_REMOVE;
	}
}

//removes all trackers with the remove flag still set
static void RemoveTaggedTrackers(ModelInstance* pInst)
{
	for(LTAnimTracker* pRemove = pInst->m_AnimTrackers; pRemove; )
	{
		//make sure to cache where we will be moving next (this handles the case if the item
		//gets removed)
		LTAnimTracker* pNextRemove = pRemove->GetNext();

		if(pRemove->m_Flags & AT_REMOVE)
		{
			//this one needs to be removed
			ilt_client->GetModelLT()->RemoveTracker((HOBJECT)pInst, pRemove->m_ID);
		}

		//onto the next item...
		pRemove = pNextRemove;
	}
}

// Reads and applies animation info out of the packet.
static LTRESULT ReadAnimInfo(CClientShell *pShell, 
							 ModelInstance *pInst, 
							 CPacket_Read &cPacket,
							 bool bAllowTransition,
							 bool bAllowReset)
{
	//we first need to tag every tracker as one that needs to be removed
	if( pInst )
		TagTrackersForRemove(pInst);

	bool bFirstTracker = true;

	while (cPacket.Readbool())
	{
		//data that will be read in about the tracker
		bool	bLooping;
		bool	bPlaying;
		uint32	nWeightSet;
		uint16	nAnimIndex;
		uint32	nAnimTime;
		float	fAnimRate;
		uint8	nID;

		//handle loading in all of our data from the packet

		bool bDirty = cPacket.Readbool();

		if (cPacket.Readbool())
		{
			nAnimIndex = cPacket.ReadBits(MODELINFO_ANIMINDEX_LONG);
			bPlaying = cPacket.Readbool();
		}
		else
		{
			nAnimIndex = cPacket.ReadBits(MODELINFO_ANIMINDEX_SHORT);
			bPlaying = true;
		}
		bLooping = cPacket.Readbool();

		uint32 nAnimSizeType = cPacket.ReadBits(2);
		const int aAnimLengths[4] = {
			MODELINFO_ANIMTIME_SIZE0,
			MODELINFO_ANIMTIME_SIZE1,
			MODELINFO_ANIMTIME_SIZE2,
			MODELINFO_ANIMTIME_SIZE3
		};
		nAnimTime = cPacket.ReadBits(aAnimLengths[nAnimSizeType]) * MODELINFO_ANIMTIME_RES;

		if (bFirstTracker) 
		{
			bFirstTracker = false;
			nID = MAIN_TRACKER;
			nWeightSet = INVALID_MODEL_WEIGHTSET;
		}
		else 
		{
			nID = cPacket.Readuint8();
			nWeightSet = cPacket.ReadBits(cPacket.Readbool() ? MODELINFO_WEIGHTSET_LONG : MODELINFO_WEIGHTSET_SHORT);

			//this should never be the main tracker since that is always assumed to be sent
			//first
			assert(nID != MAIN_TRACKER);
		}

		if (cPacket.Readbool())
			fAnimRate = cPacket.Readfloat();
		else
			fAnimRate = MODELINFO_RATEMODIFIER_DEFAULT;

		//we've finished reading in all the data for the tracker, so now create it
		if( pInst )
		{
			//find the tracker 
			LTAnimTracker *pTracker = pInst->GetTracker(nID);

			//see if we were able to find it, if not, it didn't exist and needs to be created
			if (!pTracker) 
			{
				//the tracker doesn't exist, so create one..
				if(ilt_client->GetModelLT()->AddTracker((HOBJECT)pInst, nID) != LT_OK)
				{
					//failed on this tracker
					continue;
				}

				//ok, this tracker worked, try and get it
 				pTracker = pInst->GetTracker(nID);
				if(!pTracker)
				{
					//don't know what happened here
					assert(false);
					continue;
				}
			}

			LTRESULT dResult = LT_OK;

			if(bDirty)
			{
				//handle updating the information with the info sent from the server (includes animation, etc)
				LTRESULT dResult = SwitchModelAnim(	pShell, pInst, pTracker, nAnimIndex, bLooping, bPlaying, nAnimTime,  
													fAnimRate, bAllowTransition, bAllowReset);
			}

			if (dResult != LT_OK) 
			{
				assert(false);
				continue;
			}
   
			pTracker->m_TimeRef.m_iWeightSet = nWeightSet;

			//make sure that this tracker doesn't get removed
			pTracker->m_Flags &= ~AT_REMOVE;		
		}
    }

	//now we can go through and any anim tracker that still has a remove flag can be removed
	if( pInst )
		RemoveTaggedTrackers(pInst);

	return LT_OK;
}

static bool ReadModelFiles( CPacket_Read &cPacket, InternalObjectSetup *pStruct ) 
{
	uint32 i = 0;

	pStruct->m_Filename[0].m_FileType = FILE_SERVERFILE;
    pStruct->m_Filename[0].m_FileID = cPacket.Readuint16();

	if( pStruct->m_Filename[0].m_FileID == 0xFFFF )
	{
		ASSERT( !"ReadModelFiles:  Invalid fileid received." );
		return false;
	}
	
	uint32 nNumChildModels = cPacket.ReadBits(FNumBits<MAX_CHILD_MODELS>::k_nValue);
	for (i=1; i < nNumChildModels; i++)
	{
		pStruct->m_Filename[i].m_FileID = cPacket.Readuint16();
	}
	for (; i < MAX_CHILD_MODELS; i++)
	{
		pStruct->m_Filename[i].m_FileID = 0xFFFF;
	}

	uint32 nNumTextures = cPacket.ReadBits(FNumBits<MAX_MODEL_TEXTURES>::k_nValue);
    for (i=0; i < nNumTextures; i++) 
	{
        pStruct->m_SkinNames[i].m_FileType = FILE_SERVERFILE;
        pStruct->m_SkinNames[i].m_FileID = cPacket.Readuint16();
    }
	for (; i < MAX_MODEL_TEXTURES; i++)
	{
        pStruct->m_SkinNames[i].m_FileType = FILE_SERVERFILE;
        pStruct->m_SkinNames[i].m_FileID = 0xFFFF;
	}
	uint32 nNumRenderStyles = cPacket.ReadBits(FNumBits<MAX_MODEL_RENDERSTYLES>::k_nValue);
    for (i=0; i < nNumRenderStyles; i++) 
	{
        pStruct->m_RenderStyleNames[i].m_FileType = FILE_SERVERFILE;
        pStruct->m_RenderStyleNames[i].m_FileID = cPacket.Readuint16();
    }
    for (; i < MAX_MODEL_RENDERSTYLES; i++) 
	{
        pStruct->m_RenderStyleNames[i].m_FileType = FILE_SERVERFILE;
        pStruct->m_RenderStyleNames[i].m_FileID = 0xFFFF;
    }

	return true;
}


static LTRESULT UnpackObjectChange( CClientShell *pShell, 
									uint16 changeFlags, 
									LTObject *pObject, 
								    CPacket_Read &cPacket)
{
 
    LTVector newPos, newScale;
	LTVector newVel;
    LTRotation newRot;
    LTVector offset;
    LTRotation rotationOffset;
    uint32 nodeIndex;
    LTRESULT dResult;
    ModelInstance *pInst;
    uint16 childID;



    // Do some debug output.
    PrintPacketDebugInfo(pObject, changeFlags);

    // Automatically teleport new objects.
    if (changeFlags & CF_NEWOBJECT) {
        changeFlags |= CF_TELEPORT;
    }

    if (changeFlags & CF_FILENAMES) 
	{
		ObjectCreateStruct createStruct;
		InternalObjectSetup objectSetup;
		objectSetup.m_pSetup = &createStruct;

        if (pObject->m_ObjectType == OT_MODEL) 
		{
			if( !ReadModelFiles( cPacket, &objectSetup ))
				return LT_ERROR;

		}
        else if (pObject->m_ObjectType == OT_SPRITE) 
		{
			objectSetup.m_Filename[0].m_FileID = cPacket.Readuint16();
			objectSetup.m_Filename[0].m_FileType = FILE_SERVERFILE;
        }

		// Reinitialize its 'extra data' stuff.
		LTRESULT dResult = so_ExtraInit(pObject, &objectSetup, false);
		if( LT_OK != dResult )
			return dResult;
    }

    if (changeFlags & (CF_MODELINFO|CF_FORCEMODELINFO)) 
	{
        if (pObject->m_ObjectType == OT_MODEL) 
		{
		
         
		   dResult = ReadAnimInfo(pShell, 
								   ToModel(pObject), 
								   cPacket, 
								   !(changeFlags & CF_NEWOBJECT), 
								   true);

            if (dResult != LT_OK) 
			{
                return dResult;
            }
        }
        else if (pObject->m_ObjectType == OT_SPRITE) 
		{
			cPacket.ReadType(&(((SpriteInstance*)pObject)->m_ClipperPoly));
        }
    }

    // Unpack the data.
    if (changeFlags & CF_FLAGS) 
	{
        // Read in the flags but keep anything that wasn't in the client flag mask.
        pObject->m_Flags = cPacket.Readuint16() | (pObject->m_Flags & ~CLIENT_FLAGMASK);
        pObject->m_Flags2 = cPacket.Readuint16();
        pObject->m_UserFlags = cPacket.Readuint32();
    }

    if (changeFlags & CF_RENDERINFO) 
	{
        pObject->m_ColorR		= cPacket.Readuint8();
        pObject->m_ColorG		= cPacket.Readuint8();
        pObject->m_ColorB		= cPacket.Readuint8();
        pObject->m_ColorA		= cPacket.Readuint8();
		pObject->m_nRenderGroup = cPacket.Readuint8();

        if (pObject->m_ObjectType == OT_LIGHT) 
		{
            ((DynamicLight*)pObject)->m_LightRadius = (float)cPacket.Readuint16();

            // Relocate the object in the BSP (if it isn't going to be relocated anyway).
            if (!(changeFlags & CF_SCALE)) 
			{
                g_pClientMgr->RelocateObject(pObject);
            }
        }
    }

    if (changeFlags & CF_SCALE) 
	{
        newScale.x = cPacket.Readfloat();
        newScale.y = cPacket.Readfloat();
        
        if (pObject->m_ObjectType == OT_SPRITE)
		{
            newScale.z = 1.0f;
        }
        else 
		{
            newScale.z = cPacket.Readfloat();
        }

 
        g_pClientMgr->ScaleObject(pObject, &newScale);
    }
    
    if (changeFlags & (CF_POSITION|CF_TELEPORT)) 
	{
        if (pObject->m_Flags & FLAG_FULLPOSITIONRES) 
		{
			newPos = cPacket.ReadLTVector();
			newVel = cPacket.ReadLTVector();
        }
        else 
		{
			newPos = CLTMessage_Read_Client::ReadCompPos(cPacket);
			newVel = CLTMessage_Read::ReadCompLTVector(cPacket);
        }
		pd_OnObjectMove(pShell, pObject, &newPos, &newVel, (changeFlags & CF_NEWOBJECT) != 0, (changeFlags & CF_TELEPORT) != 0);
    }


 
    if (changeFlags & (CF_ROTATION|CF_SNAPROTATION)) 
	{
        if (pObject->m_Flags & FLAG_FULLPOSITIONRES) 
		{
			cPacket.ReadType(&newRot);
        }
        else 
		{
			newRot = CLTMessage_Read::ReadCompLTRotation(cPacket);
        }

        pd_OnObjectRotate(pShell, pObject, &newRot, (changeFlags & CF_NEWOBJECT) != 0, (changeFlags & (CF_TELEPORT | CF_SNAPROTATION)) != 0);
    }


    if (changeFlags & CF_ATTACHMENTS) 
	{
        // Remove its attachments.
        om_RemoveAttachments(&g_pClientMgr->m_ObjectMgr, pObject);

        while ((childID = cPacket.Readuint16()) != INVALID_OBJECTID) 
		{
			nodeIndex = cPacket.Readuint32();

			offset = cPacket.ReadLTVector();

            rotationOffset = CLTMessage_Read::ReadCompLTRotation(cPacket);
            
            om_CreateAttachment(&g_pClientMgr->m_ObjectMgr, pObject, childID, 
								nodeIndex, &offset, &rotationOffset, LTNULL);
        }

        if (pObject->m_ObjectType == OT_MODEL) 
		{
            pInst = ToModel(pObject);

			for(uint32 nCurrPiece = 0; nCurrPiece < MAX_PIECES_PER_MODEL / 32; nCurrPiece++)
			{
				pInst->m_HiddenPieces[nCurrPiece] = cPacket.Readuint32();
			}
        }
    }

	if (changeFlags & CF_DIMS)
	{
		pObject->SetDims(cPacket.ReadLTVector());
	}
    
    return LT_OK;
}

static LTRESULT ReadNewObjectInfo(CPacket_Read &cPacket, InternalObjectSetup *pStruct, CPacket_Write &cSFXData) 
{
    bool bSFXMessage;
    uint8 objectType, longSFXMark;

    objectType = cPacket.Readuint8();

	// [KLS 3/12/02] - Added support for GlobalForceOverride
	pStruct->m_pSetup->m_GlobalForceOverride = cPacket.ReadLTVector();

    bSFXMessage = (objectType & SFXMESSAGE_FLAG) != 0;
    longSFXMark = objectType & LONGSFXMESSAGE_FLAG;
    objectType = objectType & OBJECTTYPE_MASK;

    // Read special effect information.
	uint32 nSFXLength = 0;
    if (bSFXMessage) 
	{
        if (longSFXMark) 
		{
			nSFXLength = cPacket.Readuint16();
        }
        else 
		{
			nSFXLength = cPacket.Readuint8();
        }
    }

    if (nSFXLength) 
	{
		cSFXData.WritePacket(CPacket_Read(cPacket, cPacket.Tell(), nSFXLength));
		cPacket.Seek(nSFXLength);
    }
    
    // Read filename and/or skin name.
    pStruct->m_Filename[0].m_FileType = FILE_SERVERFILE;
    if (objectType == OT_WORLDMODEL) 
	{
		cPacket.ReadString(pStruct->m_pSetup->m_Filename, MAX_CS_FILENAME_LEN);
    }
    else if (objectType == OT_CONTAINER) 
	{
        cPacket.ReadString(pStruct->m_pSetup->m_Filename, MAX_CS_FILENAME_LEN);
        pStruct->m_pSetup->m_ContainerCode = cPacket.Readuint16();
    }
    else if (objectType == OT_MODEL) 
	{
		if( !ReadModelFiles( cPacket, pStruct ))
			return LT_ERROR;
    }
    else if (objectType == OT_SPRITE) 
	{
        pStruct->m_Filename[0].m_FileID = cPacket.Readuint16();
        pStruct->m_Filename[0].m_FileType = FILE_SERVERFILE;
    }

    pStruct->m_pSetup->m_ObjectType = objectType;
    return LT_OK;
}

static LTRESULT OnYourIDPacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
    pShell->m_ClientID = cPacket.Readuint16();
    pShell->m_bLocal = cPacket.Readuint8() != 0;

    con_Printf(CONRGB(100,100,250), 1, "Got ID packet (%d)", pShell->m_ClientID);

    return LT_OK;
}


static LTRESULT OnLoadWorldPacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
	pShell->m_ClientObjectID = (uint16)-1;
	
    LTRESULT dResult = pShell->DoLoadWorld(cPacket, false);
	if (dResult != LT_OK)
		return dResult;

	// Send a response.
	CPacket_Write cResponse;
	cResponse.Writeuint8(CMSG_CONNECTSTAGE);
	cResponse.Writeuint8(0);
	g_pClientMgr->m_NetMgr.SendPacket(CPacket_Read(cResponse), pShell->m_HostID);
	
	return LT_OK;
}


static LTRESULT OnUnloadWorldPacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
    pShell->m_ClientObjectID = (uint16)-1;
    cs_UnloadWorld(pShell);
    return LT_OK;
}

static LTRESULT ReadPlaySound(CClientShell *pShell, CPacket_Read &cPacket) 
{
    PlaySoundInfo playSoundInfo;
    FileRef playSoundFileRef;
    bool bLocalOverride;
    LTVector vUp, vRight;
    FileIDInfo *pFileIDInfo, fileIDInfoNew;

    PLAYSOUNDINFO_INIT(playSoundInfo);

    playSoundFileRef.m_FileType = FILE_SERVERFILE;
    playSoundFileRef.m_FileID = cPacket.Readuint16();

    // Get the saved fileid info structure...
    pFileIDInfo = pShell->GetClientFileIDInfo(playSoundFileRef.m_FileID);
    if (!pFileIDInfo) 
	{
        pFileIDInfo = &fileIDInfoNew;
    }

	pFileIDInfo->m_nChangeFlags = cPacket.Readuint8();

    if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_SOUNDPLAYSOUNDFLAGS)) 
	{
        // Only a word's worth of data is needed for flags right now...
        pFileIDInfo->m_wSoundPlaySoundFlags = cPacket.Readuint16();
    }
    playSoundInfo.m_dwFlags = pFileIDInfo->m_wSoundPlaySoundFlags;

    playSoundInfo.m_dwFlags |= PLAYSOUND_CLIENT;
    bLocalOverride = (playSoundInfo.m_dwFlags & PLAYSOUND_CLIENTLOCAL) ? true : false;
    
    if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_SOUNDPRIORITY)) 
	{
        pFileIDInfo->m_nSoundPriority = cPacket.Readuint8();
    }

    playSoundInfo.m_nPriority = pFileIDInfo->m_nSoundPriority;

    if (playSoundInfo.m_dwFlags & (PLAYSOUND_AMBIENT | PLAYSOUND_3D)) 
	{
        // If this is a remote positional sound, then get the radii info...
        if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_RADIUS)) 
		{
            pFileIDInfo->m_nSoundOuterRadius = cPacket.Readuint16();
            pFileIDInfo->m_nSoundInnerRadius = cPacket.Readuint8();
        }
        playSoundInfo.m_fOuterRadius = (float)pFileIDInfo->m_nSoundOuterRadius;
        playSoundInfo.m_fInnerRadius = (float)pFileIDInfo->m_nSoundInnerRadius * playSoundInfo.m_fOuterRadius / 255.0f;
    }
    if (playSoundInfo.m_dwFlags & PLAYSOUND_CTRL_VOL) 
	{
        playSoundInfo.m_nVolume = cPacket.Readuint8();
    }
    else 
	{
        playSoundInfo.m_nVolume = 100;
    }
    
    if (playSoundInfo.m_dwFlags & PLAYSOUND_CTRL_PITCH) 
	{
        playSoundInfo.m_fPitchShift = cPacket.Readfloat();
    }
    else {
        playSoundInfo.m_fPitchShift = 1.0f;
    }

    if (playSoundInfo.m_dwFlags & (PLAYSOUND_AMBIENT | PLAYSOUND_3D)) 
	{
        // If this is a remote positional sound, then get the position...
        if (!bLocalOverride) 
		{
			playSoundInfo.m_vPosition = CLTMessage_Read_Client::ReadCompPos(cPacket);
        }
        else 
		{
            if (pShell->m_pFrameClientObject) 
			{
                playSoundInfo.m_vPosition = pShell->m_pFrameClientObject->GetPos();
            }
            else 
			{
                VEC_INIT(playSoundInfo.m_vPosition);
            }
        }
    }

    playSoundInfo.m_UserData = cPacket.Readuint32();

    g_pClientMgr->PlaySound(&playSoundInfo, &playSoundFileRef, 0.0f);

    return LT_OK;
}


inline LTRESULT ReadObjectSubPacket(CClientShell *pShell, 
									CPacket_Read &cPacket, 
									uint16        objectID, 
									uint16        flags) 
{
    LTObject *pObject;
    LTRESULT dResult;
    LTRecord *pRecord;
    
    pObject = LTNULL;
    
	CPacket_Write cSFXData;

	ObjectCreateStruct createStruct;
	InternalObjectSetup objectSetup;
	objectSetup.m_pSetup = &createStruct;

    if (flags & CF_NEWOBJECT) 
	{
        // If it already exists (which it really shouldn't), then get rid of the old object.
        if (objectID == (uint16)-1) 
		{
            pObject = LTNULL;
        }
        else 
		{
            // Get rid of a previous object, if there is one.
            pRecord = g_pClientMgr->FindRecord(objectID);
            if (pRecord && pRecord->m_pRecordData) 
			{
                if (pRecord->m_nRecordType == RECORDTYPE_LTOBJECT) 
				{
                    g_pClientMgr->RemoveObjectFromClientWorld((LTObject *)pRecord->m_pRecordData);
                }
                else 
				{
                    GetClientILTSoundMgrImpl()->RemoveInstance(*(CSoundInstance *)pRecord->m_pRecordData);
                }
            }
        }

        dResult = ReadNewObjectInfo(cPacket, &objectSetup, cSFXData);
		if( dResult != LT_OK )
			return dResult;

        // Add the object.  If a new position is coming or a new rotation, then we'll take care of that later...
        dResult = g_pClientMgr->AddObjectToClientWorld(objectID, &objectSetup, &pObject, !(flags & CF_POSITION), !(flags & CF_ROTATION));
        if (dResult != LT_OK) 
		{
			LTVector vOffset;
			ilt_client->GetSourceWorldOffset(vOffset);
			LTVector vWorldPos = objectSetup.m_pSetup->m_Pos + vOffset;
			dsi_ConsolePrint("Error adding object #%d to world. (%0.2f, %0.2f, %0.2f)", objectID, VEC_EXPAND(vWorldPos));
            //return dResult;
        }
    }
    else 
	{
        pObject = g_pClientMgr->FindObject(objectID);
        if (!pObject) 
		{
            g_pClientMgr->SetupError(LT_INVALIDSERVERPACKET);
            RETURN_ERROR(1, ReadObjectSubPacket, LT_INVALIDSERVERPACKET);
		}
    }

        
    if (pObject) 
	{
        dResult = UnpackObjectChange(pShell, flags, pObject, cPacket);
        if (dResult != LT_OK) 
		{
            return dResult;
        }
    
        // If it was a new object and had special effect info, notify the client shell.
        if (flags & CF_NEWOBJECT) 
		{
            if (!cSFXData.Empty())
			{
                if (i_client_shell != NULL) 
				{
                    i_client_shell->SpecialEffectNotify(pObject, 
						CSubMsg_Client(CPacket_Read(cSFXData)));
                }
            }
        }
    }

    return LT_OK;
}

static LTRESULT ReadNewSoundInfo(CClientShell *pShell, CPacket_Read &cPacket, PlaySoundInfo *pPlaySoundInfo, FileRef *pFileRef, float *pfOffsetTime) 
{
    bool bLocalOverride;
    FileIDInfo *pFileIDInfo, fileIDInfoNew;
    uint32 dwOffsetTime;

    PLAYSOUNDINFO_INIT(*pPlaySoundInfo);

    pFileRef->m_FileType = FILE_SERVERFILE;
    pFileRef->m_FileID = cPacket.Readuint16();

	// Get the saved fileid info structure...
	pFileIDInfo = pShell->GetClientFileIDInfo( pFileRef->m_FileID );
	if( !pFileIDInfo )
		pFileIDInfo = &fileIDInfoNew;

    // The fileidinfo change flags indicate which pieces of information were sent.  Most of the time,
    // the info doesn't change for a particular file id.
    pFileIDInfo->m_nChangeFlags = cPacket.Readuint8();

    // Only a word's worth of data is needed for flags right now...
    if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_SOUNDPLAYSOUNDFLAGS)) 
	{
        pFileIDInfo->m_wSoundPlaySoundFlags = cPacket.Readuint16();
    }
    pPlaySoundInfo->m_dwFlags = pFileIDInfo->m_wSoundPlaySoundFlags;

    if (pPlaySoundInfo->m_dwFlags & PLAYSOUND_CLIENTLOCAL) 
	{
        bLocalOverride = true;
    }
    else 
	{
        bLocalOverride = false;
    }
    
    if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_SOUNDPRIORITY)) 
	{
        pFileIDInfo->m_nSoundPriority = cPacket.Readuint8();
    }
    pPlaySoundInfo->m_nPriority = pFileIDInfo->m_nSoundPriority;

    if (pPlaySoundInfo->m_dwFlags & (PLAYSOUND_AMBIENT | PLAYSOUND_3D)) 
	{
        // If this is a remote positional sound, then get the radii info...
        if (pFileIDInfo->m_nChangeFlags & (FILEIDINFOF_RADIUS)) 
		{
            pFileIDInfo->m_nSoundOuterRadius = cPacket.Readuint16();
            pFileIDInfo->m_nSoundInnerRadius = cPacket.Readuint8();
        }
        pPlaySoundInfo->m_fOuterRadius = (float)pFileIDInfo->m_nSoundOuterRadius;
        pPlaySoundInfo->m_fInnerRadius = (float)pFileIDInfo->m_nSoundInnerRadius * pPlaySoundInfo->m_fOuterRadius / 255.0f;
    }
    if (pPlaySoundInfo->m_dwFlags & PLAYSOUND_CTRL_VOL) 
	{
        pPlaySoundInfo->m_nVolume = cPacket.Readuint8();
    }
    else 
	{
        pPlaySoundInfo->m_nVolume = 100;
    }

    if (pPlaySoundInfo->m_dwFlags & PLAYSOUND_CTRL_PITCH) 
	{
        pPlaySoundInfo->m_fPitchShift = cPacket.Readfloat();
    }
    else 
	{
        pPlaySoundInfo->m_fPitchShift = 1.0f;
    }

    if (pPlaySoundInfo->m_dwFlags & PLAYSOUND_TIMESYNC) 
	{
        dwOffsetTime = cPacket.Readuint8();
        if (dwOffsetTime == 0xFF) 
		{
            dwOffsetTime = cPacket.Readuint32();
        }
        *pfOffsetTime = dwOffsetTime / 1000.0f;
    }
    else 
	{
        *pfOffsetTime = 0;
    }

    pPlaySoundInfo->m_UserData = cPacket.Readuint32();

    if (bLocalOverride) 
	{
        if (pShell->m_pFrameClientObject) 
		{
            pPlaySoundInfo->m_vPosition = pShell->m_pFrameClientObject->GetPos();
        }
        else 
		{
            pPlaySoundInfo->m_vPosition.Init();
        }
    }

    return LT_OK;
}


inline LTRESULT ReadSoundSubPacket(CClientShell *pShell, CPacket_Read &cPacket, uint16 objectID, uint16 flags) 
{
    CSoundInstance *pSoundInst;
    LTRecord *pRecord;
    FileRef fileRef;
    float fOffsetTime;
    PlaySoundInfo playSoundInfo;
    LTVector vPos;
    
    pSoundInst = LTNULL;
    
    if (flags & CF_NEWOBJECT) 
	{
        // If it already exists (which it really shouldn't), then get rid of the old object.
        if (objectID != (uint16)-1) 
		{
            // Get rid of a previous sound, if there is one.
            pRecord = g_pClientMgr->FindRecord(objectID);
            if (pRecord && pRecord->m_pRecordData) 
			{
                if (pRecord->m_nRecordType == RECORDTYPE_LTOBJECT) 
				{
                    g_pClientMgr->RemoveObjectFromClientWorld((LTObject *)pRecord->m_pRecordData);
                }
                else 
				{
                    GetClientILTSoundMgrImpl()->RemoveInstance(*(CSoundInstance *)pRecord->m_pRecordData);
                }
            }
        }

        ReadNewSoundInfo(pShell, cPacket, &playSoundInfo, &fileRef, &fOffsetTime);

        playSoundInfo.m_hSound = (HLTSOUND)objectID;
    }
    else 
	{
        // The playsound may have failed, which is not fatal...
        pRecord = g_pClientMgr->FindRecord(objectID);
        if (pRecord && pRecord->m_pRecordData && pRecord->m_nRecordType == RECORDTYPE_SOUND) 
		{
            pSoundInst = (CSoundInstance *)pRecord->m_pRecordData;
        }
    }

    // Object sound is attached to had a change of position.  This flag isn't set if the sound is 
    // attached to the client object...
    if (flags & CF_POSITION) 
	{
		playSoundInfo.m_vPosition = CLTMessage_Read_Client::ReadCompPos(cPacket);

        if (pSoundInst) 
		{
            pSoundInst->SetPosition(playSoundInfo.m_vPosition);
        }
    }

    // Server is killing the loop.
    if (flags & CF_SOUNDINFO) 
	{
        if (pSoundInst) 
		{
            pSoundInst->EndLoop();
        }
    }

    if (flags & CF_NEWOBJECT) 
	{
        g_pClientMgr->PlaySound(&playSoundInfo, &fileRef, fOffsetTime);
    }

    return LT_OK;
}

static LTRESULT ReadObjectRemoves(CClientShell *pShell, CPacket_Read &cPacket) 
{
    uint16 id;
    LTRecord *pRecord;
    CSoundInstance *pSoundInstance;

    // Remove all the objects listed here.
    while (!cPacket.EOP()) 
	{
        id = cPacket.Readuint16();

        if (g_bDebugPackets == 1) 
		{
            con_WhitePrintf("Remove id %d", id);
        }
        
        pRecord = g_pClientMgr->FindRecord(id);
        if (pRecord && pRecord->m_pRecordData) 
		{
            if (pRecord->m_nRecordType == RECORDTYPE_LTOBJECT) 
			{
                g_pClientMgr->RemoveObjectFromClientWorld((LTObject *)pRecord->m_pRecordData);
            }
            else 
			{
                pSoundInstance = (CSoundInstance *)pRecord->m_pRecordData;

                // If sound is ending a loop, then just flag it for removal.
                if (pSoundInstance->GetSoundInstanceFlags() & SOUNDINSTANCEFLAG_ENDLOOP) 
				{
                    pSoundInstance->DisconnectFromServer();
                }
                else 
				{
                    GetClientILTSoundMgrImpl()->RemoveInstance(*pSoundInstance);
                }
            }
        }
    }
    
    return LT_OK;
}

static LTRESULT ReadEndPacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
    float newTime;
    
    newTime = cPacket.Readfloat();
    if (newTime > pShell->m_GameTime) 
	{
        pShell->m_GameTime = newTime;
    }

    // Update our server period estimate.
    // USE THE CONSOLE VARIABLE.. FRAMETIME DELTAS ARE TOO JITTERY.
    pShell->m_ServerPeriod = 1.0f / g_CV_UpdateRate;

    return LT_OK;
}


static LTRESULT OnPacketGroupPacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
	while (!cPacket.EOP())
	{
		uint16 nLength = cPacket.Readuint8();
		if (nLength > cPacket.TellEnd())
		{
			g_pClientMgr->SetupError(LT_INVALIDSERVERPACKET);
			RETURN_ERROR_PARAM(1, OnMessageGroupPacket, LT_INVALIDSERVERPACKET, "invalid packet");
		}
		else if (nLength == 0)
		{
			// (This signals the end of the grouped packets).
			return LT_OK;
		}

		// Set up a sub-packet.
		CPacket_Read cSubPacket(cPacket, cPacket.Tell(), nLength);
		cPacket.Seek(nLength);

		uint8 nPacketID = cSubPacket.Readuint8();
		if (g_ShellHandlers[nPacketID].fn)
		{
			LTRESULT dResult = (g_ShellHandlers[nPacketID].fn)(pShell, cSubPacket);
			if (dResult != LT_OK)
				return dResult;
		}
	}

	return LT_OK;
}


// Processes an update packet.  Any errors generated in here will cause
// a disconnection from the server.
static LTRESULT OnUpdatePacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
    uint16 flags, objectID;
    uint8 subType;
    LTRESULT dResult;

    // Debug output..
    if (g_bDebugPackets > 3) 
	{
        con_WhitePrintf("Update packet with game time %f", pShell->m_GameTime);
        con_WhitePrintf("");
    }

    while (!cPacket.EOP()) 
	{
		uint32 nUpdateSize = cPacket.Readuint32();
		uint32 nUpdateStart = cPacket.Tell();

		flags = cPacket.Readuint8();
        
        // Get more change flags if there are any.
        if (flags & CF_OTHER) 
		{
            flags |= (uint16)cPacket.Readuint8() << 8;
        }
        
        // Object identification.
        if (flags) 
		{
			objectID = cPacket.Readuint16();
			dResult = ReadObjectSubPacket(pShell, cPacket, objectID, flags);
			if (dResult != LT_OK) 
			{
				return dResult;
			}
        }
        else 
		{
            subType = cPacket.Readuint8();
    
            if (subType == UPDATESUB_PLAYSOUND) 
			{
                dResult = ReadPlaySound(pShell, cPacket);
                if (dResult != LT_OK) 
				{
                    return dResult;
                }
            }
            else if (subType == UPDATESUB_SOUNDTRACK) 
			{
                flags = cPacket.Readuint8();
                
                // Object identification.
                objectID = cPacket.Readuint16();

                // Do some debug output.
                //PrintPacketDebugInfo(objectID, flags);

                dResult = ReadSoundSubPacket(pShell, cPacket, objectID, flags);
                if (dResult != LT_OK) 
				{
                    return dResult;
                }
            }
            else if (subType == UPDATESUB_OBJECTREMOVES) 
			{
				dResult = ReadObjectRemoves(pShell, cPacket);
                if (dResult != LT_OK) 
				{
                    return dResult;
                }
            }
            else 
			{
                g_pClientMgr->SetupError(LT_INVALIDSERVERPACKET);
                RETURN_ERROR(1, OnUpdatePacket, LT_INVALIDSERVERPACKET);
            }
        }
		if (cPacket.Tell() != nUpdateStart + nUpdateSize)
		{
			ASSERT(FALSE);
			return LT_INVALIDSERVERPACKET;
		}
    }

    return LT_OK;
}


 
static LTRESULT OnUnguaranteedUpdatePacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
    uint16 id;
	uint8 flags;
    LTVector newPos, newVel;
    LTRotation newRot;
    LTObject *pObject;
    LTRESULT dResult;

    while (!cPacket.EOP())
	{
		id = cPacket.Readuint16();
        flags = cPacket.ReadBits(UUF_FLAGCOUNT);

        if (id == ID_TIMESTAMP) 
		{
            // Read the rest of the packet.
            dResult = ReadEndPacket(pShell, cPacket);
            if (dResult != LT_OK) 
			{
                return dResult;
            }
        }
        else 
		{
            pObject = g_pClientMgr->FindObject(id);
           			
            if (flags & UUF_POS) 
			{
				newPos = CLTMessage_Read_Client::ReadCompPos(cPacket);
				if (cPacket.Readbool())
					newVel = CLTMessage_Read::ReadCompLTVector(cPacket);
				else
					newVel.Init();

                if (pObject) 
				{
					bool bSameAsLast = false;
					ClientData *pData;
					pData = &pObject->cd;
					if (pData)
					{
						if (newPos.NearlyEquals(pData->m_LastUpdatePosServer, 0.1f) && 
							(newVel.NearlyEquals(pObject->m_Velocity, 0.1f)))
							bSameAsLast = true;
						if (!pData->m_MovingLink.IsTiedOff())
							bSameAsLast = false;
					}
					// Skip duplicate updates
					if (!bSameAsLast)
					{
						CompWorldPos cCompPos;
						LTVector vCurCompPos;
						world_bsp_client->EncodeCompressWorldPosition(&cCompPos, &pObject->GetPos());
						world_bsp_client->DecodeCompressWorldPosition(&vCurCompPos, &cCompPos);
						if (!vCurCompPos.NearlyEquals(newPos, 0.001f))
						{
							pd_OnObjectMove(pShell, pObject, &newPos, &newVel, false, false);
						}
					}
                }
            }

            if (flags & UUF_YROTATION) 
			{
				newRot = CLTMessage_Read::ReadYRotation(cPacket);
			
                if (pObject) 
				{
                    pd_OnObjectRotate(pShell, pObject, &newRot, false, false);
                }
            }
            else if (flags & UUF_ROT) 
			{
				newRot = CLTMessage_Read::ReadCompLTRotation(cPacket);
			
                if (pObject) 
				{
                    pd_OnObjectRotate(pShell, pObject, &newRot, false, false);
                }
            }
        
            if (flags & UUF_ANIMINFO) 
			{
				if (pObject && (pObject->m_ObjectType != OT_MODEL))
				{
					// If we got an animinfo update on a non-model, this packet is probably dead...  Stop reading it.
					ASSERT(!"AnimInfo update on non-model object!");
					break;
				}
				ReadAnimInfo(pShell, pObject ? ToModel(pObject) : NULL, cPacket, true, false);
            }
        }
    }

    return LT_OK;
}


static LTRESULT OnMessagePacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
	// Send it to the shell
    i_client_shell->OnMessage(CSubMsg_Client(cPacket));

    return LT_OK;
}

/*
static LTRESULT OnChangeObjectFilenamesPacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
    uint16 objectID;
    LTObject *pObject;
    ObjectCreateStruct createStruct;
    InternalObjectSetup objectSetup;

    objectSetup.m_pSetup = &createStruct;

    objectID = cPacket.Readuint16();
    pObject = g_pClientMgr->FindObject(objectID);

	//now we read in the changed files one by one until we hit the end
	bool bDone = false;

	while (!bDone)
	{
		// read in the type of the file
		uint8 nFileType = cPacket.Readuint8();

		// read in the index of it
		uint8 nIndex = cPacket.Readuint8();


		switch(nFileType)
		{
		case eObjectResource_ObjectFile:
			//the main file
			objectSetup.m_Filename[0].m_FileID = cPacket.Readuint16();
			objectSetup.m_Filename[0].m_FileType = FILE_SERVERFILE;
			break;
		case eObjectResource_Texture:
			objectSetup.m_SkinNames[nIndex].m_FileID = cPacket.Readuint16();
			objectSetup.m_SkinNames[nIndex].m_FileType = FILE_SERVERFILE;
			break;
		case eObjectResource_RenderStyle:
			objectSetup.m_RenderStyleNames[nIndex].m_FileID = cPacket.Readuint16();
			objectSetup.m_RenderStyleNames[nIndex].m_FileType = FILE_SERVERFILE;
			break;
		case eObjectResource_ChildModel :
			objectSetup.m_Filename[nIndex].m_FileID = cPacket.Readuint16();
			objectSetup.m_Filename[nIndex].m_FileType = FILE_SERVERFILE;
		case 0xFF:
			//the finished flag
			bDone = true;
			break;
		default:
			//something has gotten out of sync in the networking stuff...
			assert(false);
			bDone = true;
			break;		
		}
	}

	if (!pObject)
		return LT_ERROR;

	// Reinitialize its 'extra data' stuff.
    return so_ExtraInit(pObject, &objectSetup, false);
}
*/

static LTRESULT OnConsoleVar(CClientShell *pShell, CPacket_Read &cPacket) 
{
	uint32 nVarLength = cPacket.PeekString(0, 0) + 1;
	char *pszVarName = (char*)alloca(nVarLength);
	cPacket.ReadString(pszVarName, nVarLength);

	uint32 nValueLength = cPacket.PeekString(0, 0) + 1;
	char *pszStringVal = (char*)alloca(nValueLength);
	cPacket.ReadString(pszStringVal, nValueLength);

    cc_SetConsoleVariable(&g_pClientMgr->m_ServerConsoleMirror, pszVarName, pszStringVal);

    return LT_OK;
}


static LTRESULT OnSkyDef(CClientShell *pShell, CPacket_Read &cPacket) 
{
    uint16 nObjects, i;

	cPacket.ReadType(&g_pClientMgr->m_SkyDef);

    nObjects = cPacket.Readuint16();
    if (nObjects > MAX_SKYOBJECTS) 
	{
        g_pClientMgr->SetupError(LT_INVALIDSERVERPACKET);
        RETURN_ERROR_PARAM(1, OnSkyDef, LT_ERROR, "invalid packet");
    }
        
    memset(g_pClientMgr->m_SkyObjects, 0xFF, sizeof(g_pClientMgr->m_SkyObjects));
    for (i=0; i < nObjects; i++) 
	{
        g_pClientMgr->m_SkyObjects[i] = cPacket.Readuint16();
    }   
    
    return LT_OK;
}

static LTRESULT OnGlobalLight(CClientShell *pShell, CPacket_Read &cPacket) 
{
    ilt_client->SetGlobalLightDir(cPacket.ReadLTVector());

	ilt_client->SetGlobalLightColor(cPacket.ReadLTVector());

	ilt_client->SetGlobalLightConvertToAmbient(cPacket.Readfloat());

    return LT_OK;
}


static LTRESULT OnInstantSpecialEffect(CClientShell *pShell, CPacket_Read &cPacket) 
{
    if (i_client_shell != NULL) 
	{
		// Pass it on to the shell
        i_client_shell->SpecialEffectNotify(LTNULL, CSubMsg_Client(cPacket));
    }

    return LT_OK;
}


static LTRESULT OnClientObjectID(CClientShell *pShell, CPacket_Read &cPacket) 
{
    pShell->m_ClientObjectID = cPacket.Readuint16();
    return LT_OK;
}

static LTRESULT OnPreloadListPacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
    uint8 type;
    FileRef ref;   
	FileIdentifier* pFileIdent ;
    Model *pModel;
    Sprite *pSprite;
    SharedTexture *pTexture;

    ref.m_FileType = FILE_SERVERFILE;

    type = cPacket.Readuint8();
    switch (type) 
	{
        case PRELOADTYPE_START:
        {
            // Untag all textures.
            g_pClientMgr->UntagAllTextures();

            // Untag all sounds.
            GetClientILTSoundMgrImpl()->UntagAllSoundBuffers();
        }
        break;

        case PRELOADTYPE_END:
        {
            // Get rid of sounds we don't need.
            GetClientILTSoundMgrImpl()->RemoveAllUntaggedSoundBuffers();

       		// Tell the server we're ready.
			CPacket_Write cResponse;
			cResponse.Writeuint8(CMSG_CONNECTSTAGE);
			cResponse.Writeuint8(1);
			g_pClientMgr->m_NetMgr.SendPacket(CPacket_Read(cResponse), pShell->m_HostID);
		}
		break;

        case PRELOADTYPE_MODEL:
        {
            while (!cPacket.EOP()) 
			{
				ref.m_FileID = cPacket.Readuint16();
				
				if (g_pClientMgr->LoadModel(ref, pModel) != LT_OK)
				{
					DEBUG_MODEL_REZ(("model-rez: client preload model file-id(%d) not found", ref.m_FileID ));
				}
				else 
				{
					DEBUG_MODEL_REZ(("model-rez: client preload model file-id(%d)", ref.m_FileID));
				}
            }
        }
        break;

		case PRELOADTYPE_MODEL_CACHED :
		{
			while (!cPacket.EOP())
			{
				ref.m_FileID = cPacket.Readuint16();
			
				if (g_pClientMgr->CacheModelFile( ref ) != LT_OK)
				{
					DEBUG_MODEL_REZ(("model-rez: client preload cached modelfile-id(%d) not found", ref.m_FileID ));
				}
				else
				{
					DEBUG_MODEL_REZ(("model-rez: client preload cached model file-id(%d)", ref.m_FileID));
				}
            }
		}
		break ;
        case PRELOADTYPE_TEXTURE:
        {
            while (!cPacket.EOP()) 
			{
                ref.m_FileID = cPacket.Readuint16();
                pTexture = g_pClientMgr->AddSharedTexture(&ref);
                if (pTexture) 
				{
                    pTexture->SetFlags(pTexture->GetFlags() | ST_TAGGED);
                }
            }
        }
        break;

        case PRELOADTYPE_SPRITE:
        {
            while (!cPacket.EOP())
			{
                ref.m_FileID = cPacket.Readuint16();
				LoadSprite(&ref, &pSprite);
            }
        }
        break;

 
        case PRELOADTYPE_SOUND:
        {
            while (!cPacket.EOP()) 
			{
                ref.m_FileID = cPacket.Readuint16();
                pFileIdent = client_file_mgr->GetFileIdentifier(&ref, TYPECODE_SOUND);
                if (pFileIdent) 
				{
                    GetClientILTSoundMgrImpl()->CreateBuffer(*pFileIdent);
                }
            }
        }
        break;
    }

    return LT_OK;
}


static LTRESULT OnNetProtocolVersionPacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
    uint32 version = cPacket.Readuint32();
    if (version != LT_NET_PROTOCOL_VERSION) 
    {
        g_pClientMgr->SetupError(LT_INVALIDNETVERSION, LT_NET_PROTOCOL_VERSION, version);
        RETURN_ERROR(1, OnNetProtocolVersionPacket, LT_INVALIDNETVERSION);
    }

	// Try not to overflow the server's bandwidth if they've got a smaller pipe than we do...
	uint32 nServerBandwidth = cPacket.Readuint32();
	if (nServerBandwidth < pShell->m_HostID->GetBandwidth())
		pShell->m_HostID->SetBandwidth(nServerBandwidth);

	return LT_OK;
}


static LTRESULT OnThreadLoadPacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
    FileRef ref;
    uint8 fileType;

    fileType = FT_TEXTURE; //cPacket.Readuint8();
    ref.m_FileType = FILE_SERVERFILE;
    ref.m_FileID = cPacket.Readuint16();
    
    if (fileType == FT_MODEL) 
	{
        RETURN_ERROR(1, OnThreadLoadPacket, LT_UNSUPPORTED);
    }
    else if (fileType == FT_TEXTURE) 
	{
        g_pClientMgr->AddSharedTexture(&ref);
        return LT_OK;
    }
    else 
	{
        RETURN_ERROR(1, OnThreadLoadPacket, LT_INVALIDSERVERPACKET);
    }
}                

// ------------------------------------------------------------------------
// OnUnloadPacket( client-shell, packet )
// note:
// for models, this is always an "uncache this file" msg. All other 
// unload requests are basically moot, since if the client has a file
// and the server wants it destroyed, the server should tell the client to 
// remove the ltobject associated with it, this will dec-ref the associated file.

// ------------------------------------------------------------------------
static LTRESULT OnUnloadPacket(CClientShell *pShell, CPacket_Read &cPacket) 
{
    FileRef file_ref;
    FileIdentifier *pIdent;
    SharedTexture *pSharedTexture;
    uint8 fileType;

    fileType = cPacket.Readuint8();
    file_ref.m_FileType = FILE_SERVERFILE;
    file_ref.m_FileID = cPacket.Readuint16();
    
    if (fileType == FT_MODEL) 
	{
		Model *pModel = g_ModelMgr.Find(file_ref.m_FileID);
		if (pModel)
		{
			DEBUG_MODEL_REZ(("model-rez: client server->unloadcachedmodel %s", pModel->GetFilename()) );
			g_pClientMgr->UncacheModelFile(pModel);
		}

        return LT_OK;
    }
    else if (fileType == FT_TEXTURE) 
	{
        pIdent = client_file_mgr->GetFileIdentifier(&file_ref, TYPECODE_MODEL);
        if (pIdent && pIdent->m_pData) 
		{
            pSharedTexture = (SharedTexture*)pIdent->m_pData;
            r_UnbindTexture(pSharedTexture,true);
        }

        return LT_OK;
    }
    else 
	{
        RETURN_ERROR(1, OnThreadLoadPacket, LT_INVALIDSERVERPACKET);
    }
}   

// ------------------------------------------------------------------------
// OnAddChildModel()
// uint16 add flag
// uint16 parent-file-id
// uint16 child-file_id
// ------------------------------------------------------------------------
static LTRESULT OnChangeChildModel(CClientShell *pShell, CPacket_Read &cPacket)
{
	uint16 parent_file_id = cPacket.Readuint16();
	uint16 child_file_id  = cPacket.Readuint16();

	// get the target model.
	Model *pParentModel = g_ModelMgr.Find(parent_file_id);
    
    if (!pParentModel) 
	{
        GENERATE_ERROR(1, OnChangeChildModel, LT_INVALIDSERVERPACKET, "Missing object" );
		return LT_OK;
    }

	// lets find the model based on the file_id 
	Model* pChildModel = g_ModelMgr.Find(child_file_id);
	// no model? bail
	if (!pChildModel)
	{
        GENERATE_ERROR(1, OnChangeChildModel, LT_INVALIDSERVERPACKET, "Missing model" );
		return LT_OK;
	}

	if (pParentModel)
	{
		char err_str[256];
		const char *par_filename = pParentModel->GetFilename();
		const char *filename = pChildModel->GetFilename();
	
		DEBUG_MODEL_REZ(("model-rez: client-change-child-model(add) %s -> %s ", filename, par_filename ));
		pParentModel->AddChildModel(pChildModel, 
									const_cast<char*>(filename),
									err_str, sizeof(err_str));
			
		return LT_OK;
	}

	return LT_OK ;
}


// ------------------------------------------------------------------------- //
// Main routines.
// ------------------------------------------------------------------------- //

bool CClientShell::NewConnectionNotify(CBaseConn *id, bool bIsLocal) 
{
    if (m_HostID) 
	{
        return false;
    }
    else 
	{
        m_HostID = id;
        m_bOnServer = true;
		id->SetBandwidth(g_CV_BandwidthTargetClient);

        return true;
    }
}


void CClientShell::DisconnectNotify(CBaseConn *id, EDisconnectReason eDisconnectReason ) 
{
    con_Printf(CONRGB(100,100,250), 1, "Disconnected from server");

    m_HostID = INVALID_CONNID;

    if (i_client_shell != NULL) 
	{
		uint32 wParam = 0;
		if( eDisconnectReason == DISCONNECTREASON_KICKED )
			wParam = LT_REJECTED;
        i_client_shell->OnEvent(LTEVENT_DISCONNECT, wParam);
    }
}


void CClientShell::HandleUnknownPacket(const CPacket_Read &cPacket, uint8 senderAddr[4], uint16 senderPort) 
{
    // Tell the clien tshell about unknown packet.
    if (i_client_shell != NULL)
        i_client_shell->ProcessPacket(CSubMsg_Client(cPacket), senderAddr, senderPort);
}


void CClientShell::InitHandlers() 
{
    memset(g_ShellHandlers, 0, sizeof(g_ShellHandlers));

    g_ShellHandlers[SMSG_YOURID].fn = &OnYourIDPacket;
    g_ShellHandlers[SMSG_LOADWORLD].fn = &OnLoadWorldPacket;
    g_ShellHandlers[SMSG_UNLOADWORLD].fn = &OnUnloadWorldPacket;

    g_ShellHandlers[SMSG_UPDATE].fn = &OnUpdatePacket;
    g_ShellHandlers[SMSG_UNGUARANTEEDUPDATE].fn = &OnUnguaranteedUpdatePacket;

    g_ShellHandlers[SMSG_MESSAGE].fn = &OnMessagePacket;
    g_ShellHandlers[SMSG_PACKETGROUP].fn = &OnPacketGroupPacket;
    g_ShellHandlers[SMSG_CONSOLEVAR].fn = &OnConsoleVar;
    g_ShellHandlers[SMSG_SKYDEF].fn = &OnSkyDef;
    g_ShellHandlers[SMSG_GLOBALLIGHT].fn = &OnGlobalLight;
    g_ShellHandlers[SMSG_INSTANTSPECIALEFFECT].fn = &OnInstantSpecialEffect;
    g_ShellHandlers[SMSG_CLIENTOBJECTID].fn = &OnClientObjectID;
    g_ShellHandlers[SMSG_PRELOADLIST].fn = &OnPreloadListPacket;
    g_ShellHandlers[SMSG_NETPROTOCOLVERSION].fn = &OnNetProtocolVersionPacket;
    
    g_ShellHandlers[SMSG_THREADLOAD].fn = &OnThreadLoadPacket;
    g_ShellHandlers[SMSG_UNLOAD].fn = &OnUnloadPacket;

	g_ShellHandlers[SMSG_CHANGE_CHILDMODEL].fn = &OnChangeChildModel ;
}

LTRESULT CClientShell::ProcessPacket(const CPacket_Read &cPacket) 
{
	CPacket_Read cReadPacket(cPacket);

	uint8 packetID = cReadPacket.Readuint8();

    // Call the appropriate packet handler.
    if (g_ShellHandlers[packetID].fn) 
	{
        LTRESULT dResult = (g_ShellHandlers[packetID].fn)(this, cReadPacket);
        if (dResult != LT_OK) 
		{
            return dResult;
        }
    }
    else 
	{
        client_file_mgr->ProcessPacket(cReadPacket);
    }
    return LT_OK;
} 

LTRESULT CClientShell::ProcessPackets(void) 
{
    LTRESULT dResult = LT_OK;

    // Process all the packets.
    g_pClientMgr->m_NetMgr.StartGettingPackets();

	CPacket_Read cPacket;
	CBaseConn *pSender;
    while (g_pClientMgr->m_NetMgr.GetPacket(NETMGR_TRAVELDIR_SERVER2CLIENT, &cPacket, &pSender)) 
	{
        dResult = ProcessPacket(cPacket);
        if (dResult != LT_OK) 
		{
            break;
        }
    }

    g_pClientMgr->m_NetMgr.EndGettingPackets();
    return dResult;
}

void CClientShell::SendGoodbye(void) 
{
    if (m_HostID == INVALID_CONNID) 
		return;

	CPacket_Write cGoodbye;
	cGoodbye.Writeuint8(CMSG_GOODBYE);
	g_pClientMgr->m_NetMgr.SendPacket(CPacket_Read(cGoodbye), m_HostID);
}






