// ------------------------------------------------------------------------
// game_serialize.cpp
// save/load lt objects
// lithtech (c) 2000
// ------------------------------------------------------------------------

#include "bdefs.h"

#include "servermgr.h"
#include "ltmessage_server.h"
#include "dhashtable.h"
#include "s_client.h"
#include "moveobject.h"
#include "classbind.h"
#include "s_object.h"
#include "animtracker.h"
#include "smoveabstract.h"
#include "interlink.h"
#include "systimer.h"
#include "ltobjectcreate.h"

#include <vector>

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);

//server console state
#include "server_consolestate.h"
#include "concommand.h"
static IServerConsoleState *console_state;
define_holder(IServerConsoleState, console_state);

//IWorldServerBSP holder
#include "world_server_bsp.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);

#include "iltmodel.h"
static ILTModel *model_interface ;
define_holder_to_instance(ILTModel, model_interface, Server);

#define GAMESERIALIZE_CRC   0xABCDEFAF


// [KLS] Comment out DO_CRC_CHECKS to speed up saving/loading games and to
// decrease save game file size...(i.e., comment this out before shipping
// your product)...

// #define DO_CRC_CHECKS

// Use these versions of GS_STREAM_READ and GS_STREAM_WRITE during development to
// catch load/save errors...

#ifdef DO_CRC_CHECKS

static uint32 __GS__dwCRC;  // Used in macros below...

#define GS_STREAM_READ(_x_) \
	pStream->Read(&(_x_), sizeof(_x_)); \
	pStream->Read(&__GS__dwCRC, sizeof(uint32)); \
	if (__GS__dwCRC != GAMESERIALIZE_CRC) { ASSERT(false); }

#define GS_STREAM_WRITE(_x_) \
	pStream->Write(&(_x_), sizeof(_x_)); \
	__GS__dwCRC = GAMESERIALIZE_CRC; \
	pStream->Write(&__GS__dwCRC, sizeof(uint32));

#else  // Use normal stream read/writes

#define GS_STREAM_READ STREAM_READ
#define GS_STREAM_WRITE STREAM_WRITE

#endif // DO_CRC_CHECKS

uint32 g_dwSaveFileVersion = 2002;

static uint32 s_dwCurRestoreObject = 0;
static uint32 s_dwNumRestoreObjects = 0;

// Specialized load/save messaging classes which read/write a token after each operation
class CLTMessage_Load : public CLTMessage_Read_Server
{
public:
	CLTMessage_Load(const CPacket_Read &cPacket) :
		CLTMessage_Read_Server(cPacket)
	{}

	//////////////////////////////////////////////////////////////////////////////
	// ILTRefCount implementation

	virtual void Free() { delete this; }

	//////////////////////////////////////////////////////////////////////////////
	// ILTMessage_Read implementation

	virtual ILTMessage_Read *Clone() const { return new CLTMessage_Load(GetPacket()); }

	// Number of bits this message contains
	virtual uint32 Size() const {
		//ASSERT(!"This result is inaccurate due to read/write checking during save/load");
		return CLTMessage_Read_Server::Size();
	}
	// Seek by a number of bits in the message relative to the current read position
	virtual void Seek(int32 nOffset) {
		//ASSERT(!"This position is inaccurate due to read/write checking during save/load");
		CLTMessage_Read_Server::Seek(nOffset);
	}
	// Seek to a position in the message
	virtual void SeekTo(uint32 nPos) {
		if (nPos != 0)
		{
			//ASSERT(!"This position is inaccurate due to read/write checking during save/load");
		}
		CLTMessage_Read_Server::SeekTo(nPos);
	}
	// Where is the read position in the message?
	virtual uint32 Tell() const {
		//ASSERT(!"This result is inaccurate due to read/write checking during save/load");
		return CLTMessage_Read_Server::Tell();
	}

	virtual uint32 ReadBits(uint32 nBits) {
		ASSERT(!EOM());
		uint32 nResult = CLTMessage_Read_Server::ReadBits(nBits);
		VerifyRead();
		return nResult;
	}
	virtual uint64 ReadBits64(uint32 nBits) {
		ASSERT(!EOM());
		uint64 nResult = CLTMessage_Read_Server::ReadBits64(nBits);
		VerifyRead();
		return nResult;
	}
	virtual void ReadData(void *pData, uint32 nBits) {
		ASSERT(!EOM());
		CLTMessage_Read_Server::ReadData(pData, nBits);
		VerifyRead();
	}

	virtual ILTMessage_Read *ReadMessage() {
		ASSERT(!EOM());
		ILTMessage_Read *pResult = CLTMessage_Read_Server::ReadMessage();
		VerifyRead();
		return pResult;
	}
	virtual uint32 ReadString(char *pDest, uint32 nMaxLen) {
		ASSERT(!EOM());
		uint32 nResult = CLTMessage_Read_Server::ReadString(pDest, nMaxLen);
		VerifyRead();
		return nResult;
	}
	virtual HSTRING ReadHString() {
		ASSERT(!EOM());
		HSTRING hResult = CLTMessage_Read_Server::ReadHString();
		VerifyRead();
		return hResult;
	}
	virtual LTVector ReadCompLTVector() {
		ASSERT(!EOM());
		LTVector vResult = CLTMessage_Read_Server::ReadCompLTVector();
		VerifyRead();
		return vResult;
	}
	virtual LTVector ReadCompPos() {
		ASSERT(!EOM());
		LTVector vResult = CLTMessage_Read_Server::ReadCompPos();
		VerifyRead();
		return vResult;
	}
	virtual LTRotation ReadCompLTRotation() {
		ASSERT(!EOM());
		LTRotation cResult = CLTMessage_Read_Server::ReadCompLTRotation();
		VerifyRead();
		return cResult;
	}
	virtual uint32 ReadHStringAsString(char *pDest, uint32 nMaxLen) {
		ASSERT(!EOM());
		uint32 nResult = CLTMessage_Read_Server::ReadHStringAsString(pDest, nMaxLen);
		VerifyRead();
		return nResult;
	}
    virtual HOBJECT ReadObject() {
		ASSERT(!EOM());
		uint32 nObjectType = CLTMessage_Read_Server::ReadBits(4);
		uint32 nID = CLTMessage_Read_Server::ReadBits(16);
		VerifyRead();

        if ((nID == INVALID_SERIALIZEID) || (nObjectType > NUM_OBJECTTYPES))
			return LTNULL;

        // Look up the object  (um..  EW!)
		HOBJECT hResult = LTNULL;
        LTLink *pListHead = &g_pServerMgr->m_ObjectMgr.m_ObjectLists[nObjectType].m_Head;
        for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
		{
			// Sanity check.
			if (NULL == pCur)
			{
				break;
			}

            LTObject *pObject = (LTObject*)pCur->m_pData;

            if (pObject->m_SerializeID == nID)
			{
                hResult = pObject;
                break;
            }
        }

		return hResult;
	}
	virtual LTRotation ReadYRotation() {
		ASSERT(!EOM());
		LTRotation cResult = CLTMessage_Read_Server::ReadYRotation();
		VerifyRead();
		return cResult;
	}

protected:
	void VerifyRead() {
#ifdef DO_CRC_CHECKS
		uint32 nToken = CLTMessage_Read_Server::ReadBits(32);
		ASSERT(nToken == GAMESERIALIZE_CRC);
#endif // DO_CRC_CHECKS
	}
};

class CLTMessage_Save : public CLTMessage_Write_Server
{
public:
	CLTMessage_Save() :
		CLTMessage_Write_Server()
	{}

	//////////////////////////////////////////////////////////////////////////////
	// ILTRefCount implementation

	virtual void Free() { delete this; }

	//////////////////////////////////////////////////////////////////////////////
	// ILTMessage_Write implementation

	// Return an ILTMessage_Read initialized with this message's data
	// Note : This will reset the message
	virtual ILTMessage_Read *Read() { return new CLTMessage_Load(CPacket_Read(GetPacket())); }

	// Number of bits which have been written to this message
	virtual uint32 Size() const {
		//ASSERT(!"This number is inaccurate due to read/write checking during save/load");
		return CLTMessage_Write_Server::Size();
	}

	// Basic data writing functions
	// Note : nSize is in bits
	virtual void WriteBits(uint32 nValue, uint32 nSize) { CLTMessage_Write_Server::WriteBits(nValue, nSize); VerifyWrite(); }
	virtual void WriteBits64(uint64 nValue, uint32 nSize) { CLTMessage_Write_Server::WriteBits64(nValue, nSize); VerifyWrite(); }
	virtual void WriteData(const void *pData, uint32 nSize) { CLTMessage_Write_Server::WriteData(pData, nSize); VerifyWrite(); }

	// Complex data type writing functions
	virtual void WriteMessage(const ILTMessage_Read *pMsg) { CLTMessage_Write_Server::WriteMessage(pMsg); VerifyWrite(); }
	virtual void WriteString(const char *pString) { CLTMessage_Write_Server::WriteString(pString); VerifyWrite(); }
	virtual void WriteHString(HSTRING hString) { CLTMessage_Write_Server::WriteHString(hString); VerifyWrite(); }
	virtual void WriteCompLTVector(const LTVector &vVec) { CLTMessage_Write_Server::WriteCompLTVector(vVec); VerifyWrite(); }
	virtual void WriteCompPos(const LTVector &vPos) { CLTMessage_Write_Server::WriteCompPos(vPos); VerifyWrite(); }
	virtual void WriteCompLTRotation(const LTRotation &cRotation) { CLTMessage_Write_Server::WriteCompLTRotation(cRotation); VerifyWrite(); }
	virtual void WriteHStringArgList(int nStringCode, va_list *pList) { CLTMessage_Write_Server::WriteHStringArgList(nStringCode, pList); VerifyWrite(); }
	virtual void WriteStringAsHString(const char *pString) { CLTMessage_Write_Server::WriteStringAsHString(pString); VerifyWrite(); }
	virtual void WriteObject(HOBJECT hObj) {
		if (hObj && !( hObj->m_InternalFlags & IFLAG_OBJECTGOINGAWAY ))
		{
			CLTMessage_Write_Server::WriteBits(hObj->m_ObjectType, 4);
			CLTMessage_Write_Server::WriteBits(hObj->m_SerializeID, 16);
		}
		else
		{
			CLTMessage_Write_Server::WriteBits(NUM_OBJECTTYPES, 4);
			CLTMessage_Write_Server::WriteBits(INVALID_SERIALIZEID, 16);
		}
		VerifyWrite();
	}
	virtual void WriteYRotation(const LTRotation &cRotation) { CLTMessage_Write_Server::WriteYRotation(cRotation); VerifyWrite(); }

protected:
	void VerifyWrite() {
#ifdef DO_CRC_CHECKS
		CLTMessage_Write_Server::WriteBits(GAMESERIALIZE_CRC, 32);
#endif // DO_CRC_CHECKS
	}
};


namespace
{

// Objects larger than this size are considered "suspicious" and spit out a warning.
const uint32 	k_nSuspiciousObjectSize = 0x10000; // 64k

//////////////////////////////////////////////////////////////////////////////
// Internal serialization buffer
// This is a static buffer used by the serialization routines so they can
// avoid allocations from the stack without having to constantly hit the heap

uint8* 			s_pTempBuffer 		= NULL;
unsigned		s_nTempBufferSize 	= k_nSuspiciousObjectSize;

void DeleteTempBuffer()
{
	// Delete the old buffer.
	if (LTNULL != s_pTempBuffer)
	{
		delete [] s_pTempBuffer;
		s_pTempBuffer = LTNULL;
	}
}

void CreateTempBuffer(unsigned BufSize)
{
	DeleteTempBuffer();

	// Allocate the new buffer.
	LT_MEM_TRACK_ALLOC(s_pTempBuffer = new uint8[BufSize], LT_MEM_TYPE_MISC);
	ASSERT(LTNULL != s_pTempBuffer);

	// Reset the buffer size.
	s_nTempBufferSize = BufSize;
}

void AdjustTempBufferSize(unsigned NewBufSize)
{
	if (NewBufSize > s_nTempBufferSize)
	{
		DeleteTempBuffer();
		CreateTempBuffer(NewBufSize);
	}
}


} // unnamed namespace



// ----------------------------------------------------------------------------- //
// Internal helpers.
// ----------------------------------------------------------------------------- //

static void sm_SaveObjectData(LTObject *pObj, ILTStream *pStream, uint32 dwParam)
{
    // Make space for the 'next object' indicator.
    uint32 curPos = 0;

    uint32 nextPos = pStream->GetPos();
    GS_STREAM_WRITE(curPos);

    uint32 objDataPos = pStream->GetPos();
    GS_STREAM_WRITE(curPos);

    // Get the save message from the object
    CLTMessage_Save cSaveMessage;
	// Make sure nobody tries to free this message, since it's not allocated dynamically
	cSaveMessage.IncRef();
	pObj->sd->m_pObject->OnSave(&cSaveMessage, (float)dwParam);
	cSaveMessage.StaticDecRef();

	// Dump it into the serialization buffer
	uint32 nSpecialEffectLen = pObj->sd->m_cSpecialEffectMsg.Size();
	uint32 nObjectMsgLen = cSaveMessage.Size();

	if ((nSpecialEffectLen + nObjectMsgLen) > k_nSuspiciousObjectSize)
	{
		dsi_ConsolePrint("Warning!  Suspiciously large object save encountered!");
	}

    // Save the object state.
    pStream->WriteString(pObj->sd->m_pClass->m_ClassName);


    // Save stuff for the ObjectCreateStruct.
    GS_STREAM_WRITE(pObj->m_ObjectType);
    GS_STREAM_WRITE(pObj->m_Flags);
    GS_STREAM_WRITE(pObj->m_Flags2);
	GS_STREAM_WRITE(pObj->m_nRenderGroup);

    *pStream << pObj->GetPos();
    *pStream << pObj->GetGlobalForceOverride();
    GS_STREAM_WRITE(pObj->m_Scale);
    GS_STREAM_WRITE(pObj->m_Rotation);
    GS_STREAM_WRITE(pObj->m_UserFlags);

    char *pStr;
    if (pObj->sd->m_hName != NULL)
	{
        pStr = (char*)hs_GetElementKey(pObj->sd->m_hName, LTNULL);
    }
    else
	{
        pStr = "";
    }

    pStream->WriteString(pStr);

    if (pObj->m_ObjectType == OT_WORLDMODEL)
	{
        pStream->WriteString(ToWorldModel(pObj)->m_pOriginalBsp->m_WorldName);
    }
    else
	{
		// if it ain't a model, then fine.
		if( pObj->m_ObjectType != OT_MODEL )
		{
			pStr = server_filemgr->GetUsedFilename(pObj->sd->m_pFile);
			pStream->WriteString(pStr);
		}
    }

    if (pObj->m_ObjectType == OT_MODEL)
	{
		char filename[256];
		// we have to save which childmodels have been loaded here.
		// because they may not be in the original model.
		ModelInstance *pModelInstance =pObj->ToModel();
		Model *pModel = pModelInstance->GetModelDB();

		if( model_interface->GetModelDBFilename(pObj,filename, 256) == LT_OK )
		{
			pStream->WriteString(filename);
		}

		// the other dependent filenames. i.e childmodels.
		uint16 i;
		for (i=1; i < MAX_CHILD_MODELS ; i++ )
		{
			if( pModel )
			{
				if( i < pModel->NumChildModels() )
				{
					Model *pChildModel = pModel->GetChildModel(i)->m_pModel;

					pStream->WriteString(pChildModel->GetFilename());
					continue ;
				}
			}

			pStr = "";
			pStream->WriteString(pStr);
		}

		// textures.
        for (i=0; i < MAX_MODEL_TEXTURES; i++)
		{
            pStr = server_filemgr->GetUsedFilename(pObj->sd->m_pSkins[i]);
            pStream->WriteString(pStr);
		}
		// renderstyles.
        for (i=0; i < MAX_MODEL_RENDERSTYLES; i++)
		{
            pStr = server_filemgr->GetUsedFilename(pObj->sd->m_pRenderStyles[i]);
            pStream->WriteString(pStr);
		}
    }
    else
	{
        pStr = server_filemgr->GetUsedFilename(pObj->sd->m_pSkin);
        pStream->WriteString(pStr);
        pStr = server_filemgr->GetUsedFilename(pObj->sd->m_pRenderStyle);
        pStream->WriteString(pStr);
   }

    if (pObj->m_ObjectType == OT_LIGHT)
	{
        GS_STREAM_WRITE(((DynamicLight*)pObj)->m_LightRadius);
    }

    if (pObj->m_ObjectType == OT_CONTAINER)
	{
        GS_STREAM_WRITE(((ContainerInstance *)pObj)->m_ContainerCode);
    }

    GS_STREAM_WRITE(pObj->sd->m_NextUpdate);

    // Save other stuff.
    GS_STREAM_WRITE(pObj->m_BPriority);

    // Write the special effect message.
	uint8 bSpecialEffectMessage = (!pObj->sd->m_cSpecialEffectMsg.Empty());
    GS_STREAM_WRITE(bSpecialEffectMessage);
    if (bSpecialEffectMessage)
	{
		ASSERT(pObj->sd->m_cSpecialEffectMsg.Size() < 0x10000);
		uint16 nTemp = (uint16)nSpecialEffectLen;
        GS_STREAM_WRITE(nTemp);

		// Make sure our buffer is large enough.
		AdjustTempBufferSize(nSpecialEffectLen);

		pObj->sd->m_cSpecialEffectMsg.PeekData(s_pTempBuffer, nSpecialEffectLen);
        pStream->Write(s_pTempBuffer, (nSpecialEffectLen + 7) / 8);
    }

	//write out the animation trackers if applicable
	if(pObj->m_ObjectType == OT_MODEL)
	{
		ModelInstance * pInst = pObj->ToModel();

		//now save out the animation trackers on the model...
		uint8 nNumTrackers = pInst->GetNumAnimTrackers();
		GS_STREAM_WRITE(nNumTrackers);

		//ok, now save each and every tracker
		for(LTAnimTracker* pTracker = pInst->m_AnimTrackers; pTracker; pTracker = pTracker->GetNext())
		{
			//write out the ID first so that when loading we can use this to create the new tracker
			GS_STREAM_WRITE(pTracker->m_ID);

			//save out all the properties for it
			GS_STREAM_WRITE(pTracker->m_CurKey);
			GS_STREAM_WRITE(pTracker->m_dwLastHintTime);
			GS_STREAM_WRITE(pTracker->m_Flags);
			GS_STREAM_WRITE(pTracker->m_hHintNode);
			GS_STREAM_WRITE(pTracker->m_hLastHintAnim);
			GS_STREAM_WRITE(pTracker->m_InterpolationMS);
			GS_STREAM_WRITE(pTracker->m_RateModifier);
			GS_STREAM_WRITE(pTracker->m_tfLastHint);
			GS_STREAM_WRITE(pTracker->m_TimeRef.m_Percent);
			GS_STREAM_WRITE(pTracker->m_TimeRef.m_iWeightSet);
			GS_STREAM_WRITE(pTracker->m_TimeRef.m_Prev.m_iAnim);
			GS_STREAM_WRITE(pTracker->m_TimeRef.m_Prev.m_iFrame);
			GS_STREAM_WRITE(pTracker->m_TimeRef.m_Prev.m_Time);
			GS_STREAM_WRITE(pTracker->m_TimeRef.m_Cur.m_iAnim);
			GS_STREAM_WRITE(pTracker->m_TimeRef.m_Cur.m_iFrame);
			GS_STREAM_WRITE(pTracker->m_TimeRef.m_Cur.m_Time);
		}
	}

    GS_STREAM_WRITE(pObj->m_ColorR);
    GS_STREAM_WRITE(pObj->m_ColorG);
    GS_STREAM_WRITE(pObj->m_ColorB);
    GS_STREAM_WRITE(pObj->m_ColorA);
    *pStream << pObj->GetDims();
    GS_STREAM_WRITE(pObj->m_Mass);
    GS_STREAM_WRITE(pObj->m_Velocity);

    GS_STREAM_WRITE(pObj->m_FrictionCoefficient);
    GS_STREAM_WRITE(pObj->m_Acceleration);
    GS_STREAM_WRITE(pObj->m_ForceIgnoreLimitSqr);

    GS_STREAM_WRITE(pObj->m_InternalFlags);

    if (pObj->m_InternalFlags & IFLAG_INSKY)
	{
        uint16 skyIndex = 0;
        for (uint16 i=0; i < MAX_SKYOBJECTS; i++)
		{
            if (g_pServerMgr->m_SkyObjects[i] == pObj->m_ObjectID)
			{
                skyIndex = i;
            }
        }

        GS_STREAM_WRITE(skyIndex);
    }

    // Save client data.
    uint8 bHasClient = !!pObj->sd->m_pClient;
    GS_STREAM_WRITE(bHasClient);

    if (bHasClient)
	{
        GS_STREAM_WRITE(pObj->sd->m_pClient->m_ClientFlags);
        pStream->WriteString(pObj->sd->m_pClient->m_Name);
    }

    // Set the 'object data position' indicator.
    curPos = pStream->GetPos();
    pStream->SeekTo(objDataPos);
    GS_STREAM_WRITE(curPos);
    pStream->SeekTo(curPos);

	// Make sure our buffer is large enough.
	AdjustTempBufferSize(nObjectMsgLen);

	// Write out the object save packet
	CPacket_Read cSaveMessage_Read(cSaveMessage.GetPacket());
	cSaveMessage_Read.ReadData(s_pTempBuffer, nObjectMsgLen);
	GS_STREAM_WRITE(nObjectMsgLen);
	pStream->Write(s_pTempBuffer, (nObjectMsgLen + 7) / 8);

    pStream->WriteVal((uint32)GAMESERIALIZE_CRC);

    // Store the 'next object position' indicator.
    curPos = pStream->GetPos();
    pStream->SeekTo(nextPos);
    GS_STREAM_WRITE(curPos);
    pStream->SeekTo(curPos);
}

// ------------------------------------------------------------------------
// CreateObject from a file.
// ------------------------------------------------------------------------
static LTRESULT sm_CreateNextObject(ILTStream *pStream,
									LTObject  **pOut,
									uint32 dwParam)
{
    LTRESULT dResult;

    // Next position indicators.
	uint32 nextObjectPos;
    GS_STREAM_READ(nextObjectPos);
    if (nextObjectPos == (uint32)-1)
	{
        return LT_FINISHED;
    }

	uint32 objDataPos;
    GS_STREAM_READ(objDataPos);


    // Setup an ObjectCreateStruct and create the object.
    // It reads each member into the LTObject first to make sure
    // it reads in the right data type.
    ObjectCreateStruct createStruct;
    createStruct.Clear();
    char className[256];
    pStream->ReadString(className, sizeof(className));

	CClassData *pClassData = g_pServerMgr->m_ClassMgr.FindClassData(className);
    ClassDef *pClass = pClassData->m_pClass;
    if (!pClass)
	{
        sm_SetupError(LT_CANTRESTOREOBJECT, className);
        RETURN_ERROR_PARAM(1, sm_CreateNextObject, LT_CANTRESTOREOBJECT, className);
    }

	ASSERT((pClass->m_ClassFlags & CF_CLASSONLY) == 0);

    LTObject tempObj;
    GS_STREAM_READ(tempObj.m_ObjectType);
    GS_STREAM_READ(tempObj.m_Flags);
    GS_STREAM_READ(tempObj.m_Flags2);
	GS_STREAM_READ(tempObj.m_nRenderGroup);

    LTVector tempVec;
	*pStream >> tempVec;
    tempObj.SetPos(tempVec);

	*pStream >> createStruct.m_GlobalForceOverride;

    GS_STREAM_READ(tempObj.m_Scale);
    GS_STREAM_READ(tempObj.m_Rotation);
    GS_STREAM_READ(tempObj.m_UserFlags);

    createStruct.m_ObjectType	= tempObj.m_ObjectType;
    createStruct.m_Flags		= tempObj.m_Flags;
    createStruct.m_Flags2		= tempObj.m_Flags2;
    createStruct.m_Pos			= tempObj.GetPos();
    createStruct.m_Scale		= tempObj.m_Scale;
    createStruct.m_Rotation		= tempObj.m_Rotation;
	createStruct.m_nRenderGroup = tempObj.m_nRenderGroup;

    pStream->ReadString(createStruct.m_Name, MAX_CS_FILENAME_LEN);
    pStream->ReadString(createStruct.m_Filename, MAX_CS_FILENAME_LEN);

    if (tempObj.m_ObjectType == OT_MODEL)
	{
	    uint16 i;
		for( i = 1 ; i < MAX_CHILD_MODELS ; i++ )
		{
			pStream->ReadString(createStruct.m_Filenames[i], MAX_CS_FILENAME_LEN);
		}
        for (i=0; i < MAX_MODEL_TEXTURES; i++)
		{
            pStream->ReadString(createStruct.m_SkinNames[i], MAX_CS_FILENAME_LEN);
		}
        for (i=0; i < MAX_MODEL_RENDERSTYLES; i++)
		{
            pStream->ReadString(createStruct.m_RenderStyleNames[i], MAX_CS_FILENAME_LEN);
		}

    }
    else
	{
        pStream->ReadString(createStruct.m_SkinName, MAX_CS_FILENAME_LEN);
        pStream->ReadString(createStruct.m_RenderStyleName, MAX_CS_FILENAME_LEN);
    }

    float tempRadius = 0.0f;
    if (tempObj.m_ObjectType == OT_LIGHT)
	{
        GS_STREAM_READ(tempRadius);
    }

    if (tempObj.m_ObjectType == OT_CONTAINER)
	{
        GS_STREAM_READ(createStruct.m_ContainerCode);
    }

    GS_STREAM_READ(createStruct.m_NextUpdate);

    // Create the object.
    LPBASECLASS pBaseClass = sm_AllocateObjectOfClass(pClass);

	createStruct.m_hClass = (HCLASS)pClassData;

	uint32 nPreCreateResult = pBaseClass->OnPrecreate(&createStruct, PRECREATE_SAVEGAME);

	// Skip this object if they didn't want it created
	if (!nPreCreateResult)
	{
		sm_FreeObjectOfClass(pClass, pBaseClass);
		pStream->SeekTo(nextObjectPos);
		return LT_OK;
	}

	LTObject *pObj;
	dResult = sm_AddObjectToWorld(pBaseClass, pClass, &createStruct,
		INVALID_OBJECTID, OBJECTCREATED_SAVEGAME, &pObj);
	if (dResult != LT_OK)
	{
		return dResult;
	}

    // Copy data over.
    pObj->m_UserFlags = tempObj.m_UserFlags;
    if (pObj->m_ObjectType == OT_LIGHT)
	{
        ((DynamicLight*)pObj)->m_LightRadius = tempRadius;
    }

    // Now read the rest of the data in.
    GS_STREAM_READ(pObj->m_BPriority);

    // Read the special effect message.
    uint8 bSpecialEffectMessage;
    GS_STREAM_READ(bSpecialEffectMessage);
    if (bSpecialEffectMessage)
	{
	    uint16 messageLen;
        GS_STREAM_READ(messageLen);

		// Make sure our buffer is large enough.
		AdjustTempBufferSize(messageLen);

		uint32 nMessageBytes = (messageLen + 7) / 8;
		pStream->Read(s_pTempBuffer, nMessageBytes);
		CPacket_Write cTempMsg;
		cTempMsg.WriteData(s_pTempBuffer, messageLen);
		pObj->sd->m_cSpecialEffectMsg = CPacket_Read(cTempMsg);
    }

	//read in the model animation trackers if applicable
	if(pObj->m_ObjectType == OT_MODEL)
	{
		ModelInstance * pInst = pObj->ToModel();

		//now save out the animation trackers on the model...
		uint8 nNumTrackers = 0;
		GS_STREAM_READ(nNumTrackers);

		//ok, now save each and every tracker
		for(uint32 nCurrTracker = 0; nCurrTracker < nNumTrackers; nCurrTracker++)
		{
			//write out the ID first so that when loading we can use this to create the new tracker
			ANIMTRACKERID NewID;
			GS_STREAM_READ(NewID);

			//now lets actually create our anim tracker (if it isn't the main one)
			if(NewID != MAIN_TRACKER)
				model_interface->AddTracker((HOBJECT)pObj, NewID);

			//now get the tracker
			LTAnimTracker* pTracker = pInst->GetTracker(NewID);
			assert(pTracker);

			//read in all the properties for it
			GS_STREAM_READ(pTracker->m_CurKey);
			GS_STREAM_READ(pTracker->m_dwLastHintTime);
			GS_STREAM_READ(pTracker->m_Flags);
			GS_STREAM_READ(pTracker->m_hHintNode);
			GS_STREAM_READ(pTracker->m_hLastHintAnim);
			GS_STREAM_READ(pTracker->m_InterpolationMS);
			GS_STREAM_READ(pTracker->m_RateModifier);
			GS_STREAM_READ(pTracker->m_tfLastHint);
			GS_STREAM_READ(pTracker->m_TimeRef.m_Percent);
			GS_STREAM_READ(pTracker->m_TimeRef.m_iWeightSet);
			GS_STREAM_READ(pTracker->m_TimeRef.m_Prev.m_iAnim);
			GS_STREAM_READ(pTracker->m_TimeRef.m_Prev.m_iFrame);
			GS_STREAM_READ(pTracker->m_TimeRef.m_Prev.m_Time);
			GS_STREAM_READ(pTracker->m_TimeRef.m_Cur.m_iAnim);
			GS_STREAM_READ(pTracker->m_TimeRef.m_Cur.m_iFrame);
			GS_STREAM_READ(pTracker->m_TimeRef.m_Cur.m_Time);
		}
	}

    GS_STREAM_READ(pObj->m_ColorR);
    GS_STREAM_READ(pObj->m_ColorG);
    GS_STREAM_READ(pObj->m_ColorB);
    GS_STREAM_READ(pObj->m_ColorA);
    *pStream >> tempVec;
    pObj->SetDims(tempVec);
    GS_STREAM_READ(pObj->m_Mass);
    GS_STREAM_READ(pObj->m_Velocity);

    GS_STREAM_READ(pObj->m_FrictionCoefficient);
    GS_STREAM_READ(pObj->m_Acceleration);
    GS_STREAM_READ(pObj->m_ForceIgnoreLimitSqr);

    uint32 tempInternalFlags;
    GS_STREAM_READ(tempInternalFlags);

    uint16 skyIndex = 0;
    if (tempInternalFlags & IFLAG_INSKY)
	{
        GS_STREAM_READ(skyIndex);
        if (skyIndex >= MAX_SKYOBJECTS)
		{
            sm_SetupError(LT_CANTRESTOREOBJECT, className);
            RETURN_ERROR_PARAM(1, sm_CreateNextObject, LT_CANTRESTOREOBJECT, className);
        }
    }

    MoveState moveState;
	moveState.Setup(world_bsp_server->ServerTree(), g_pServerMgr->m_MoveAbstract, pObj, pObj->m_BPriority);
	LTVector vNewDims = pObj->GetDims();
	ChangeObjectDimensions(&moveState, vNewDims, LTFALSE, LTFALSE);

	// This makes sure it gets in the correct active/inactive list.
	sm_SetObjectStateFlags(pObj, tempInternalFlags & IFLAG_INACTIVE_MASK);
	pObj->m_InternalFlags = tempInternalFlags;


	// AddObjectToWorld ignores m_Pos for world models so really move it.
	FullMoveObject(pObj, &createStruct.m_Pos, MO_SETCHANGEFLAG|MO_TELEPORT);

	// Put it back in the sky..
	if (tempInternalFlags & IFLAG_INSKY)
	{
		g_pServerMgr->m_SkyObjects[skyIndex] = pObj->m_ObjectID;
		sm_SetSendSkyDef();
	}


    // Read in client info.
	uint8 bHasClient;
    GS_STREAM_READ(bHasClient);
    if (bHasClient)
	{
	    uint32 clientFlags;
        GS_STREAM_READ(clientFlags);
	    char clientName[512];
        pStream->ReadString(clientName, sizeof(clientName));

	    ClientRef *pClientRef;
		LT_MEM_TRACK_ALLOC(pClientRef = (ClientRef*)dalloc(sizeof(ClientRef) + strlen(clientName)),LT_MEM_TYPE_MISC);
		pClientRef->m_ClientFlags = clientFlags;
		strcpy(pClientRef->m_ClientName, clientName);
		pClientRef->m_ObjectID = pObj->m_ObjectID;

		// Store this so we can lose the client ref when the object goes away.
		pObj->m_InternalFlags |= IFLAG_HASCLIENTREF;

		dl_AddHead(&g_pServerMgr->m_ClientReferences, &pClientRef->m_Link, pClientRef);
    }

    *pOut = pObj;

    // Seek to the next object position.
    pStream->SeekTo(nextObjectPos);
    return LT_OK;
}


static LTRESULT sm_RestoreNextObject(ILTStream *pStream, LTObject *pObj, uint32 dwParam)
{
    // Next position indicators.
	uint32 nextObjectPos;
    GS_STREAM_READ(nextObjectPos);
    if (nextObjectPos == (unsigned long)-1)
        return LT_FINISHED;

	uint32 objDataPos;
    GS_STREAM_READ(objDataPos);

    // Find the object.
    char className[256];
    pStream->ReadString(className, sizeof(className));


    // Read their serialize message.
    pStream->SeekTo(objDataPos);
	uint32 nObjectMsgLen;
	GS_STREAM_READ(nObjectMsgLen);

	// Make sure our buffer is large enough.
	AdjustTempBufferSize(nObjectMsgLen);

	uint32 nObjectMsgBytes = (nObjectMsgLen + 7) / 8;
	pStream->Read(s_pTempBuffer, nObjectMsgBytes);
	CPacket_Write cTempMsg;
	cTempMsg.WriteData(s_pTempBuffer, nObjectMsgLen);

    // Tell them to process their serialize message.
	// Make sure nobody tries to free this message, since it's not allocated dynamically
	CPacket_Read cConvertToRead(cTempMsg);
	CLTMessage_Load cLoadMessage(cConvertToRead);
	cLoadMessage.IncRef();
	pObj->sd->m_pObject->OnLoad(&cLoadMessage, (float)dwParam);
	cLoadMessage.StaticDecRef();

    uint32 dwCRC;
    *pStream >> dwCRC;
    if (dwCRC != GAMESERIALIZE_CRC)
	{
		uint32 curPos = pStream->GetPos();

		dsi_ConsolePrint("LOAD ERROR: Couldn't restore object (%d of %d) '%s'!!!",
			s_dwCurRestoreObject, s_dwNumRestoreObjects, className);
		dsi_ConsolePrint("  Object saved %d bytes", nextObjectPos - objDataPos);
		dsi_ConsolePrint("  Object tried to load %d bytes!", curPos - objDataPos);

		if (curPos < nextObjectPos)
		{
			dsi_ConsolePrint("  Restore UNDER wrote %d bytes!", nextObjectPos - curPos);
		}
		else
		{
			dsi_ConsolePrint("  Restore OVER wrote %d bytes!", curPos - nextObjectPos);
		}

        RETURN_ERROR(1, sm_RestoreNextObject, LT_INVALIDFILE);
    }

    // Seek to the next object position.
    pStream->SeekTo(nextObjectPos);
    return LT_OK;
}


static void sm_SaveAttachments(LTObject *pObject, ILTStream *pStream)
{
    Attachment *pCur;
    uint16 count;


    // Get the number of (valid) attachments.
    count = 0;
    for (pCur=pObject->m_Attachments; pCur; pCur=pCur->m_pNext)
	{
		LTObject* pChild = sm_FindObject(pCur->m_nChildID);
        if (pChild)
		{
            if (pChild->m_SerializeID != INVALID_SERIALIZEID)
			{
                ++count;
            }
        }
    }

    // Save each one out.
    GS_STREAM_WRITE(count);
    for (pCur=pObject->m_Attachments; pCur; pCur=pCur->m_pNext)
	{
		LTObject* pChild = sm_FindObject(pCur->m_nChildID);
        if (pChild)
		{
            if (pChild->m_SerializeID != INVALID_SERIALIZEID)
			{
                GS_STREAM_WRITE(pCur->m_Offset.m_Pos);
                GS_STREAM_WRITE(pCur->m_Offset.m_Rot);
                GS_STREAM_WRITE(pChild->m_SerializeID);
                GS_STREAM_WRITE(pCur->m_iSocket);
            }
        }
    }
}


static void sm_SaveInterlinks(LTObject *pObject, ILTStream *pStream)
{
    uint16 count;
    LTLink *pCur;
    InterLink *pLink;
    LTObject *pOther;
    uint8 tempType;

    count=0;
    for (pCur=pObject->sd->m_Links.m_pNext; pCur != &pObject->sd->m_Links; pCur=pCur->m_pNext)
	{
        pLink = (InterLink*)pCur->m_pData;

        if (pLink->m_pOwner == pObject &&
            (pLink->m_Type == LINKTYPE_INTERLINK || pLink->m_Type == LINKTYPE_CONTAINER))
        {
            pOther = (LTObject*)pLink->m_pOther;
            if (pOther->m_SerializeID != INVALID_SERIALIZEID)
			{
                ++count;
            }
        }
    }

    GS_STREAM_WRITE(count);
    for (pCur=pObject->sd->m_Links.m_pNext; pCur != &pObject->sd->m_Links; pCur=pCur->m_pNext)
	{
        pLink = (InterLink*)pCur->m_pData;

        if (pLink->m_pOwner == pObject &&
            (pLink->m_Type == LINKTYPE_INTERLINK || pLink->m_Type == LINKTYPE_CONTAINER))
        {
            pOther = (LTObject*)pLink->m_pOther;
            if (pOther->m_SerializeID != INVALID_SERIALIZEID)
			{
                tempType = (uint8)pLink->m_Type;
                GS_STREAM_WRITE(tempType);
                GS_STREAM_WRITE(pOther->m_SerializeID);
            }
        }
    }
}


static LTRESULT sm_RestoreAttachments(LTObject *pObject, ILTStream *pStream,
    LTObject **pObjects, int objectCount)
{
    uint16 i, count, serializeID;
    uint32 nodeIndex;
    LTVector offset;
    LTRotation rotationOffset;
    LTObject *pChild;


    GS_STREAM_READ(count);
    for (i=0; i < count; i++)
	{
        GS_STREAM_READ(offset);
        GS_STREAM_READ(rotationOffset);
        GS_STREAM_READ(serializeID);
        GS_STREAM_READ(nodeIndex);

        if (serializeID >= objectCount)
		{
            RETURN_ERROR(1, RestoreAttachments, LT_ERROR);
        }

        pChild = pObjects[serializeID];
        om_CreateAttachment(&g_pServerMgr->m_ObjectMgr, pObject, pChild->m_ObjectID, nodeIndex,
            &offset, &rotationOffset, LTNULL);
    }

    return LT_OK;
}


static LTRESULT sm_RestoreInterlinks(LTObject *pObject, ILTStream *pStream,
    LTObject **pObjects, int objectCount)
{
    uint16 i, count;
    uint8 tempType;
    uint16 id;

    GS_STREAM_READ(count);
    for (i=0; i < count; i++)
	{
        GS_STREAM_READ(tempType);
        GS_STREAM_READ(id);

        if (id >= objectCount)
		{
            RETURN_ERROR(1, RestoreInterlinks, LT_ERROR);
        }

        CreateInterLink(pObject, pObjects[id], tempType);
    }

    return LT_OK;
}


// ----------------------------------------------------------------------------- //
// Main interface functions.
// ----------------------------------------------------------------------------- //


void sm_SaveObjects(ILTStream *pStream, ObjectList *pList, uint32 dwParam, uint32 flags)
{
    unsigned long terminator;
    LTObject *pObj;
    ObjectLink *pLink;
    HHashIterator *hIterator;
    HHashElement *hElement;
    uint8 bAnyMore;
    LTCommandVar *pCurVar;
    char strTemp[201];
    LTObject **pObjects;
    int nObjects;
	int i;

	// Create the temporary buffer.
	CreateTempBuffer(s_nTempBufferSize);

	// Make the actual list of ones we're going to save.
    LT_MEM_TRACK_ALLOC(pObjects = new LTObject*[pList->m_nInList],LT_MEM_TYPE_MISC);
    if (!pObjects)
	{
		// Free the temporary buffer.
		DeleteTempBuffer();

		return;
	}

    nObjects = 0;
    for (pLink=pList->m_pFirstLink; pLink; pLink=pLink->m_pNext)
	{
        pObj = HandleToServerObj(pLink->m_hObject);

		// Skip the always load objects.
        if( cb_IsClassFlagSet( &g_pServerMgr->m_ClassMgr.m_ClassModule, pObj->sd->m_pClass, CF_ALWAYSLOAD ))
			continue;

		// Skip objects that are flagged to go away.
		if( pObj->m_InternalFlags & IFLAG_OBJECTGOINGAWAY )
			continue;

		pObjects[nObjects++] = pObj;
    }


    // Setup the serialize IDs.
    om_ClearSerializeIDs(&g_pServerMgr->m_ObjectMgr);
    for (i=0; i < nObjects; i++)
	{
        pObjects[i]->m_SerializeID = (uint16)i;
    }


    // Save version. Version checks will be done by the game.
	g_dwSaveFileVersion = 2002;
    GS_STREAM_WRITE(g_dwSaveFileVersion);


    GS_STREAM_WRITE(nObjects);

    GS_STREAM_WRITE(g_pServerMgr->m_GameTime);
	GS_STREAM_WRITE(g_pServerMgr->m_nTrueLastTimeMS);
    GS_STREAM_WRITE(g_pServerMgr->m_nTimeOffsetMS);

    // Write out the game console state..
    if (flags & SAVEOBJECTS_SAVEGAMECONSOLE)
	{
        bAnyMore = 1;

        hIterator = hs_GetFirstElement(console_state->State()->m_VarHash);
        while (hIterator)
		{
            hElement = hs_GetNextElement(hIterator);
            pCurVar = (LTCommandVar *)hs_GetElementUserData(hElement);

            GS_STREAM_WRITE(bAnyMore);

            // It does the strncpy stuff here so the file doesn't become unreadable later
            // if the string was too long for the read buffer.
            LTStrCpy(strTemp, pCurVar->pVarName, sizeof(strTemp));
            pStream->WriteString(strTemp);

            LTStrCpy(strTemp, pCurVar->pStringVal, sizeof(strTemp));
            pStream->WriteString(strTemp);
        }
    }

    bAnyMore = 0;
    GS_STREAM_WRITE(bAnyMore);


    for (i=0; i < nObjects; i++)
	{
        sm_SaveObjectData(pObjects[i], pStream, dwParam);
    }

    // Write the terminator.
    terminator = (unsigned long)-1;
    GS_STREAM_WRITE(terminator);


    // Save attachments and interlinks.
    for (i=0; i < nObjects; i++)
	{
        sm_SaveAttachments(pObjects[i], pStream);
        sm_SaveInterlinks(pObjects[i], pStream);
    }

    bAnyMore = 0;
    GS_STREAM_WRITE(bAnyMore);
    delete [] pObjects;

	// Free the temporary buffer.
	DeleteTempBuffer();
}

LTRESULT sm_RestoreConsoleVars(ILTStream *pStream)
{
    uint8 bMore;
    char tempStr[256], tempStr2[256], cmd[512];

    // Read in the console state variables.
    for (;;)
	{
        GS_STREAM_READ(bMore);
        if (!bMore) break;

        pStream->ReadString(tempStr, sizeof(tempStr));
        pStream->ReadString(tempStr2, sizeof(tempStr2));
        if (pStream->ErrorStatus() != LT_OK)
		{
            RETURN_ERROR(1, RestoreObjects, LT_INVALIDFILE);
        }

        LTSNPrintF(cmd, sizeof(cmd), "%s %s", tempStr, tempStr2);
        cc_HandleCommand2(console_state->State(), cmd, CC_NOCOMMANDS);
    }

	return LT_OK;
}

LTRESULT sm_RestoreObjects(ILTStream *pStream, uint32 dwParam, uint32 flags)
{
    unsigned long startPos;
    LTRESULT dResult;
    LTObject **pObjects;
    int i, objectCount;

	// Create the temporary buffer.
	CreateTempBuffer(s_nTempBufferSize);

	// Load version
    GS_STREAM_READ(g_dwSaveFileVersion);
    GS_STREAM_READ(objectCount);

	// Used for debugging...
	s_dwNumRestoreObjects = objectCount;

    // Restore timers.
    if (flags & RESTOREOBJECTS_RESTORETIME)
	{
        GS_STREAM_READ(g_pServerMgr->m_GameTime);
		GS_STREAM_READ(g_pServerMgr->m_nTrueLastTimeMS);
        GS_STREAM_READ(g_pServerMgr->m_nTimeOffsetMS);
    }
    else
	{
		float dummy_GameTime;
		uint32 dummy_TrueLastTimeMS;
		uint32 dummy_TimeOffsetMS;
        GS_STREAM_READ(dummy_GameTime);
 		GS_STREAM_READ(dummy_TrueLastTimeMS);
        GS_STREAM_READ(dummy_TimeOffsetMS);
    }

    g_pServerMgr->m_nTimeOffsetMS = (int32)(g_pServerMgr->m_nTrueLastTimeMS) - (int32)(time_GetMSTime() + g_pServerMgr->m_nTimeOffsetMS);

	// Only allow these 2 versions.  The game will handle versioning of save files from
	// here on.
    if( g_dwSaveFileVersion != 2001 && g_dwSaveFileVersion != 2002 )
	{
		// Delete our temporary buffer.
		DeleteTempBuffer();

		RETURN_ERROR(1, sm_RestoreObjects, LT_INVALIDVERSION);
    }

	if (sm_RestoreConsoleVars(pStream) != LT_OK)
	{
		// Delete our temporary buffer.
		DeleteTempBuffer();

		RETURN_ERROR(1, sm_RestoreObjects, LT_INVALIDFILE);
	}

    startPos = pStream->GetPos();

    LT_MEM_TRACK_ALLOC(pObjects = (LTObject**)dalloc_z(sizeof(LTObject*) * objectCount),LT_MEM_TYPE_MISC);

    // Create the objects and restore their engine variable state.
    pStream->SeekTo(startPos);
    i = 0;
    while ((dResult = sm_CreateNextObject(pStream, &pObjects[i++], dwParam)) == LT_OK);

    if (dResult == LT_FINISHED)
	{ // Was there an error?

        // Set their serialize IDs.
        om_ClearSerializeIDs(&g_pServerMgr->m_ObjectMgr);
        for (i=0; i < objectCount; i++)
		{
            pObjects[i]->m_SerializeID = (uint16)i;
        }

        // Then tell each object to restore itself.
        pStream->SeekTo(startPos);
		s_dwCurRestoreObject = 0;
		dResult = LT_OK;
        while( dResult == LT_OK && ( int )s_dwCurRestoreObject < objectCount )
		{
			LTObject* pObj = pObjects[s_dwCurRestoreObject++];
			dResult = sm_RestoreNextObject(pStream, pObj, dwParam);
		}

		if(( int )s_dwCurRestoreObject == objectCount )
		{
			dResult = LT_FINISHED;
		    unsigned long terminator;
			GS_STREAM_READ(terminator);
		}

        // If all was well, restore attachments.
        if (dResult == LT_FINISHED)
		{
            for (i=0; i < objectCount; i++)
			{
                dResult = sm_RestoreAttachments(pObjects[i], pStream, pObjects, objectCount);
                if (dResult != LT_OK) break;

                dResult = sm_RestoreInterlinks(pObjects[i], pStream, pObjects, objectCount);
                if (dResult != LT_OK) break;
            }
        }
    }

    dfree(pObjects);

	// Delete our temporary buffer.
	DeleteTempBuffer();

    if (dResult != LT_OK && dResult != LT_FINISHED)
	{// Was there an error?
        return dResult;
    }

    return LT_OK;
}


