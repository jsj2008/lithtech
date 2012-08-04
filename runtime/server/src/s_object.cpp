//------------------------------------------------------------------
//
//  FILE      : s_object.cpp
//
//  PURPOSE   : Implements object-related stuff in the ServerMgr.
//
//  CREATED   : January 12 1996
//
//  COPYRIGHT : LithTech Inc., 1996-2000
//
//------------------------------------------------------------------

#include "bdefs.h"

#include "servermgr.h"
#include "geomroutines.h"
#include "moveobject.h"
#include "s_net.h"
#include "s_object.h"
#include "model.h"
#include "animtracker.h"
#include "model_ops.h"
#include "conparse.h"
#include "sysstreamsim.h"
#include "interlink.h"
#include "syscounter.h"
#include "sysdebugging.h"
#include "motion.h"
#include "smoveabstract.h"
#include "classbind.h"
#include "packetdefs.h"
#include "dhashtable.h"
#include "s_client.h"
#include "ltobjectcreate.h"



//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorld holder
#include "world_server_bsp.h"
#include "de_mainworld.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);


//---------------------------------------------------------------------------//
void GetPhysicsVector (LTObject *pObj, float dt, LTVector& dr)
{
    //if the object is a container, find what other objects are in contact
    //with it and affect their physics
    if (!(pObj->m_Flags & FLAG_CONTAINER)
        &&
        pObj->sd->m_Links.m_pNext != &pObj->sd->m_Links)
    {
        LTLink *pCur, *pListHead;
        InterLink *pLink;
        ContainerPhysics cPhysics;

        cPhysics.m_Acceleration = pObj->m_Acceleration;
        cPhysics.m_Velocity     = pObj->m_Velocity;
        cPhysics.m_Flags        = pObj->m_Flags;
        cPhysics.m_hObject      = ServerObjToHandle(pObj);

        // Let each container modify the physics.
        int32 nActualContainers = 0;
        pListHead = &pObj->sd->m_Links;

        for (pCur = pListHead->m_pNext ; pCur != pListHead ;)
        {
            pLink = (InterLink*)pCur->m_pData;
            pCur = pCur->m_pNext;

            if (pLink->m_Type == LINKTYPE_CONTAINER
                &&
                (pLink->m_pOwner->m_Flags & FLAG_CONTAINER))
            {
                LTObject* pContainer = pLink->m_pOwner;

//              pContainer->sd->m_pObject->EngineMessageFn(MID_AFFECTPHYSICS, &cPhysics, 0);
				pContainer->sd->m_pObject->OnAffectPhysics(&cPhysics);
                ++nActualContainers;
            }
        }

        CalcMotion(&g_pServerMgr->m_MotionInfo,
                    pObj,
                    dr,
                    cPhysics.m_Velocity,
                    cPhysics.m_Acceleration,
                    (cPhysics.m_Flags & FLAG_GRAVITY) != 0,
                    dt);

        // Don't let this flag clear when in a container.
        if (nActualContainers)
        {
            //enable physics
            pObj->m_InternalFlags |= IFLAG_APPLYPHYSICS;
        }
    }
    else
    {
        CalcMotion(&g_pServerMgr->m_MotionInfo,
                    pObj,
                    dr,
                    pObj->m_Velocity,
                    pObj->m_Acceleration,
                    (pObj->m_Flags & FLAG_GRAVITY) != 0,
                    dt);
    }
}


void PhysicsUpdateObject(LTObject *pObj)
{
    //if physics is disabled, drop out early
    if (!(pObj->m_InternalFlags & IFLAG_APPLYPHYSICS))
        return;

	//store the initial position since it can change below and we need to know the delta
    const LTVector P0 = pObj->GetPos();

    // Apply physics stuff.
	LTVector dr;
    GetPhysicsVector(pObj, g_pServerMgr->m_FrameTime, dr);

    // Reset the acceleration.
    pObj->m_Acceleration.Init();

    // Call MoveObject() for it automatically if tried to move at all.
    if (dr.MagSqr() > 0.0001f)
    {
        const LTVector P1 = pObj->GetPos() + dr;

        FullMoveObject(pObj, &P1, MO_DETACHSTANDING | MO_MOVESTANDINGONS);

        // Remove it if it's outside.
        if (((pObj->m_Flags & FLAG_REMOVEIFOUTSIDE) != 0) &&
            world_bsp_server->IsLoaded() &&
            world_bsp_server->IsOutsideWorld(pObj->GetPos()))
        {
            AddObjectToRemoveList(pObj);
            return;
        }

        // If it's still in the world, set its change flags..
        if (pObj->m_InternalFlags & IFLAG_INWORLD)
        {
            const LTVector v = pObj->GetPos() - P0;//net displacement
            const float d2 = v.Dot(v);//displacement squared

            if (d2 > 0.001f)
            {
                SetObjectChangeFlags(pObj, CF_POSITION);
            }
        }
    }
}


// ----------------------------------------------------------------------- //
// Updates any server structures for the object (like its LTAnimTracker).
// ----------------------------------------------------------------------- //

void ServerStringKeyCallback(LTAnimTracker *pTracker, AnimKeyFrame *pFrame)
{
    LTObject *pObj;
    ArgList argList;
    ConParse parse;

    // Make sure we have a server object for it.
    pObj = pTracker->GetModelInstance();
    if (pObj)
    {
        // Does this object care?
        if (pObj->m_Flags & FLAG_MODELKEYS)
        {
            parse.Init(pFrame->m_pString);
            argList.argv = parse.m_Args;

            while (parse.Parse())
            {
                if (parse.m_nArgs > 0)
                {
                    argList.argc = parse.m_nArgs;
                    pObj->sd->m_pObject->EngineMessageFn(MID_MODELSTRINGKEY, &argList, 0);
                }
            }
        }
    }
}



// ----------------------------------------------------------------------- //
// Sets up the server structures for the object.
// ----------------------------------------------------------------------- //

bool IsObjectInChangedList(LTObject *pObj)
{
	return !( pObj->sd->m_ChangedNode.IsTiedOff( ));
}


LTRESULT sm_UpdateInBspStatus(LTObject *pObject)
{
    if (!CanOptimizeObject(pObject))
    {
        if (!pObject->IsInWorldTree())
        {
            world_bsp_server->ServerTree()->InsertObject(pObject);
        }
    }
    else
    {
        pObject->RemoveFromWorldTree();
    }

    return LT_OK;
}


uint32 sm_GetNewObjectChangeFlags(LTObject *pObject)
{
    uint32 changeFlags;

    changeFlags = CF_NEWOBJECT | CF_TELEPORT;

    if (!pObject->m_Rotation.IsIdentity())
    {
        changeFlags |= CF_SNAPROTATION;
    }

//    if (pObject->m_Flags & CLIENT_FLAGMASK || pObject->m_UserFlags != 0)
//    {
//        changeFlags |= CF_FLAGS;
//    }

    if (pObject->m_Scale.x != 1.0f || pObject->m_Scale.y != 1.0f || pObject->m_Scale.z != 1.0f)
    {
        changeFlags |= CF_SCALE;
    }

    // Objects default to 255:255:255:255.
    if (pObject->m_ColorR != 255 || pObject->m_ColorG != 255 || pObject->m_ColorB != 255 || pObject->m_ColorA != 255)
    {
        changeFlags |= CF_RENDERINFO;
    }

	if(pObject->m_nRenderGroup != 0)
	{
		changeFlags |= CF_RENDERINFO;
	}

    if (pObject->m_ObjectType == OT_MODEL)
    {
        changeFlags |= CF_MODELINFO;
    }

    if (pObject->m_Attachments)
    {
        changeFlags |= CF_ATTACHMENTS;
    }
	else
	{
		//note that the else is because if we have already set the attachment flag, no
		//need to check for it below then

		//see if this object is a model that has any hidden pieces
		if(pObject->m_ObjectType == OT_MODEL)
		{
			for(uint32 nCurrPiece = 0; nCurrPiece < MAX_PIECES_PER_MODEL / 32; nCurrPiece++)
			{
				if(ToModel(pObject)->m_HiddenPieces != 0)
				{
					changeFlags |= CF_ATTACHMENTS;
					break;
				}
			}
		}
	}

    return changeFlags;
}


// ----------------------------------------------------------------------- //
// Fully updates the object (called once per frame).
// ----------------------------------------------------------------------- //

void FullObjectUpdate(LTObject *pObj)
{
    // Update its server object if its a model instance.
	if (pObj->m_ObjectType == OT_MODEL)
    {
		if(!pObj->IsPaused())
		{
			// Use the TrueFrameTime, otherwise the time will notbe synced with the client.
			// If time is out of sync between client/server, movement breaks.
			if (g_pServerMgr->m_nTrueFrameTimeMS > 0)
			{
				pObj->ToModel()->SetStringKeyCallback(ServerStringKeyCallback);
				pObj->ToModel()->ServerUpdate(g_pServerMgr->m_nTrueFrameTimeMS);
			}
		}
    }

    // Update the object (if the m_NextUpdate countdown has gone past zero).
    if (pObj->sd->m_NextUpdate > 0.0f)
    {
        pObj->sd->m_NextUpdate -= g_pServerMgr->m_FrameTime;

        if (pObj->sd->m_NextUpdate <= 0.0f)
        {
            // Call the update.
			pObj->sd->m_pObject->OnUpdate();

            // Don't do anything else if it was removed.
            if (!(pObj->m_InternalFlags & IFLAG_INWORLD))
                return;
        }
    }

    // Update the object's physics.
    PhysicsUpdateObject(pObj);
}



// ----------------------------------------------------------------------- //
// Loads and instantiates objects from the given world file.
// ----------------------------------------------------------------------- //
LTRESULT LoadObjects(ILTStream *pStream, const char *pWorldName, bool bAllObjects, uint32 nObjectDataOffset)
{
    uint32 i, k, nObjects, nProperties, objStartPos;
    uint16 propLen, objDataLen;
    char typeName[256], propName[256];

    uint32 dummyPropFlags;
    uint8 propCode;
    ClassDef *pClass;

    CClassMgr *pClassMgr;

    LPBASECLASS pObject;
    LTObject *pObj;
    ObjectCreateStruct createStruct;
    LTRESULT dResult;
    typedef std::vector<uint8> TPropData;
	TPropData aPropData;

	//make sure that our object create struct has enough room to hold a decent number of properties.
	//It will resize if it needs more, but we should try and avoid that
	createStruct.m_cProperties.ReserveProps(256, false);

	//preserve the old create struct and replace it with our own
	ObjectCreateStruct *pOldOCS = g_pServerMgr->m_pCurOCS;
	g_pServerMgr->m_pCurOCS = &createStruct;

    pClassMgr = &g_pServerMgr->m_ClassMgr;

    // Load the objects.
    pStream->SeekTo(nObjectDataOffset);

    // For each object....
    STREAM_READ(nObjects);
    for (i=0; i < nObjects; i++)
    {
        STREAM_READ(objDataLen);
        objStartPos = pStream->GetPos();

        pStream->ReadString(typeName, sizeof(typeName));

        if (pStream->ErrorStatus() != LT_OK)
        {
            sm_SetupError(LT_INVALIDWORLDFILE, pWorldName);
			g_pServerMgr->m_pCurOCS = pOldOCS;
            RETURN_ERROR(1, LoadObjects, LT_INVALIDWORLDFILE);
        }

        // Get the class.
		CClassData *pClassData = g_pServerMgr->m_ClassMgr.FindClassData(typeName);
		pClass = (pClassData) ? pClassData->m_pClass : LTNULL;

        // Set things up to succeed anyway if we don't have that class
        if (pClass)
        {
            // If it's not supposed to be created at runtime, ignore it.
            if (pClass->m_ClassFlags & CF_NORUNTIME)
                pClass = LTNULL;
            // If only loading LOADALWAYS objects, then skip the ones without the flag set...
            else if (!bAllObjects && !cb_IsClassFlagSet(&g_pServerMgr->m_ClassMgr.m_ClassModule, pClass, CF_ALWAYSLOAD))
                pClass = LTNULL;
        }
        else
        {
			// This can happen if a level used an object that did not exist in the class module.
			// If it does exist, it can happen if it exists in an obj of a static lib that
			// is not being referenced, which causes the linker to not use it when linking to
			// the dll.
			char szError[256];
			LTSNPrintF( szError, sizeof(szError), "LoadObjects - Server is missing class %s", typeName );
            dsi_ConsolePrint( szError );
			ASSERT( !"LoadObjects - Server is missing class" );
        }

        // Create and construct an instance of it.
        if (pClass)
        {
            pObject = sm_AllocateObjectOfClass(pClass);
        }
        else
            pObject = LTNULL;

        createStruct.Clear();
        createStruct.m_Flags = 0;
        createStruct.m_ObjectType = OT_NORMAL;
        createStruct.m_Filename[0] = 0;
        createStruct.m_SkinName[0] = 0;
        createStruct.m_Pos.Init();


        // Read in all the properties.
        STREAM_READ(nProperties);

		// Make sure that the object create struct has enough room for all the properties
		if(createStruct.m_cProperties.GetMaxProps() < nProperties)
		{
			createStruct.m_cProperties.ReserveProps(nProperties, false);
		}


        for (k=0; k < nProperties; k++)
        {
            // Name.
            pStream->ReadString(propName, sizeof(propName));

            // Property length.
            STREAM_READ(propCode);
            STREAM_READ(dummyPropFlags)
            STREAM_READ(propLen);

			LT_MEM_TRACK_ALLOC(aPropData.resize(propLen), LT_MEM_TYPE_PROPERTY);

            if (propCode == PT_STRING)
            {
                pStream->ReadString((char*)(&(*aPropData.begin())), propLen);
            }
            else
            {
                pStream->Read(static_cast<void*>(&(*aPropData.begin())), propLen);
            }

			// Add it as a property if we're going to tell them about it
			if (pClass && pObject)
			{
				switch (propCode)
				{
					case LT_PT_VECTOR :
					case LT_PT_COLOR :
					{
						ASSERT(propLen == sizeof(LTVector));
						createStruct.m_cProperties.AddProp(propName, GenericProp(*(LTVector*)(&(*aPropData.begin())), propCode));
						break;
					}
					case LT_PT_STRING :
					{
						createStruct.m_cProperties.AddProp(propName, GenericProp((const char *)(&(*aPropData.begin())), propCode));
						break;
					}
					case LT_PT_REAL :
					{
						ASSERT(propLen == sizeof(float));
						createStruct.m_cProperties.AddProp(propName, GenericProp(*(float*)(&(*aPropData.begin())), propCode));
						break;
					}
					case LT_PT_LONGINT :
					case LT_PT_FLAGS :
					{
						ASSERT(propLen == sizeof(float));
						// Note : LONGINT/FLAGS properties are stored as a float, cast to an int.
						// This is because from the tools perspective, there's no such thing
						// as an integer property.
						createStruct.m_cProperties.AddProp(propName, GenericProp((int32)(*(float*)(&(*aPropData.begin()))), propCode));
						break;
					}
					case LT_PT_BOOL :
					{
						ASSERT(propLen == sizeof(uint8));
						createStruct.m_cProperties.AddProp(propName, GenericProp(*aPropData.begin() != 0, propCode));
						break;
					}
					case LT_PT_ROTATION :
					{
						ASSERT(propLen == sizeof(LTRotation));
						// These need to be handled a bit differently due to being
						// stored as eulers embedded in an LTRotation.  (Hey, don't blame
						// me, I wasn't the one that started this mess...)
						GenericProp cRotationProp(*(LTVector*)(&(*aPropData.begin())), propCode);
						cRotationProp.m_Rotation = LTRotation(VEC_EXPAND(cRotationProp.m_Vec));
						createStruct.m_cProperties.AddProp(propName, cRotationProp);
						break;
					}
					default :
					{
						ASSERT(!"Unknown property type encountered on object load");
						break;
					}
				}
			}
        }

        // Send it the precreate message so it can read in its properties.
        if (pClass && pObject)
        {
			createStruct.m_hClass = (HCLASS)pClassData;
			uint32 nPreCreateResult = pObject->OnPrecreate(&createStruct, PRECREATE_WORLDFILE);
			if (nPreCreateResult && ((pClass->m_ClassFlags & CF_CLASSONLY) == 0))
			{
	            dResult = sm_AddObjectToWorld(pObject, pClass, &createStruct,
		            INVALID_OBJECTID, OBJECTCREATED_WORLDFILE, &pObj);
			}
			else
				dResult = LT_OK;
            if ((!nPreCreateResult) || (dResult != LT_OK))
            {
                sm_FreeObjectOfClass(pClass, pObject);
            }
        }
    }

	g_pServerMgr->m_pCurOCS = pOldOCS;

    if (pStream->ErrorStatus() != LT_OK)
    {
        sm_SetupError(LT_INVALIDWORLDFILE, pWorldName);
        RETURN_ERROR(1, LoadObjects, LT_INVALIDWORLDFILE);
    }

    return LT_OK;
}


void AddObjectToRemoveList(LTObject *pObj)
{
    LTLink *pLink;

    if (~pObj->m_InternalFlags & IFLAG_OBJECTGOINGAWAY)
    {
        pLink = g_DLinkBank.Allocate();

        pLink->m_pData = pObj;
        dl_Insert(&g_pServerMgr->m_RemovedObjectHead, pLink);
        pObj->m_InternalFlags |= IFLAG_OBJECTGOINGAWAY;

        // Add this object to the list of objects to remove for each client.
        pObj->m_InternalFlags &= ~IFLAG_INWORLD;
    }
}


void sm_RemoveObjectsThatNeedToGetRemoved()
{
    LTLink *pLink, *pNext;
    LTObject *pObject;

    pLink = g_pServerMgr->m_RemovedObjectHead.m_pNext;
    while (pLink != &g_pServerMgr->m_RemovedObjectHead)
    {
        pObject = (LTObject*)pLink->m_pData;

        sm_RemoveObjectFromWorld(pObject->sd->m_pObject);

        pNext = pLink->m_pNext;

        // MUST use dl_Remove here so it doesn't screw up if they remove objects
        // in their destructors (ie: m_RemovedObjectHead gets stuff added to it).
        dl_Remove(pLink);
        g_DLinkBank.Free(pLink);

        pLink = pNext;
    }
}


LTObject* sm_FindObject(uint16 objectID)
{
    if (objectID < g_pServerMgr->m_ObjectMap.GetSize())
    {
        if (g_pServerMgr->m_ObjectMap[objectID].m_nRecordType == RECORDTYPE_LTOBJECT)
            return (LTObject *)g_pServerMgr->m_ObjectMap[objectID].m_pRecordData;
    }

    return LTNULL;
}


LTRecord* sm_FindRecord(uint16 objectID)
{
    if (objectID < g_pServerMgr->m_ObjectMap.GetSize())
    {
        return &g_pServerMgr->m_ObjectMap[objectID];
    }

    return LTNULL;
}


LTLink* sm_FindInFreeList(uint16 objectID)
{
    LTLink *pListHead, *pCur;

    // Look for one with this ID in the free list.
    pListHead = &g_pServerMgr->m_FreeIDs;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        if ((GetLinkID(pCur) & ~IDFLAG_MASK) == objectID)
        {
            return pCur;
        }
    }

    return LTNULL;
}


void sm_SetObjectSpecialEffectMessage(LTObject *pObj, const CPacket_Read &cPacket)
{
    // Point at the packet
	pObj->sd->m_cSpecialEffectMsg = cPacket;

    sm_UpdateInBspStatus(pObj);
}


void sm_SetObjectStateFlags(LTObject *pObj, uint32 flags)
{
    uint32 oldFlags;

    ASSERT((flags & ~IFLAG_INACTIVE_MASK) == 0);

    oldFlags = pObj->m_InternalFlags & IFLAG_INACTIVE_MASK;
    if (flags != oldFlags)
    {
        // Set the flags.
        pObj->m_InternalFlags = (pObj->m_InternalFlags & ~IFLAG_INACTIVE_MASK) | flags;

        // Replace it in the list.
        dl_RemoveAt(&g_pServerMgr->m_Objects, &pObj->sd->m_ListNode);
        if (flags)
        {
            dl_AddTail(&g_pServerMgr->m_Objects, &pObj->sd->m_ListNode, pObj);
			pObj->sd->m_pObject->OnDeactivate();
        }
        else
        {
            dl_AddHead(&g_pServerMgr->m_Objects, &pObj->sd->m_ListNode, pObj);
			pObj->sd->m_pObject->OnActivate();
        }
    }
}


void sm_ClearClientReferenceList()
{
    LTLink *pCur, *pNext, *pListHead;

    pListHead = &g_pServerMgr->m_ClientReferences.m_Head;
    pCur = pListHead->m_pNext;
    while (pCur != pListHead)
    {
        pNext = pCur->m_pNext;
        dfree(pCur->m_pData);
        pCur = pNext;
    }

    dl_InitList(&g_pServerMgr->m_ClientReferences);
}


ClientRef* sm_FindClientRefFromObject(LTObject *pObj)
{
    LTLink *pCur, *pListHead;
    ClientRef *pClientRef;

    pListHead = &g_pServerMgr->m_ClientReferences.m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pClientRef = (ClientRef*)pCur->m_pData;

        if (pClientRef->m_ObjectID == pObj->m_ObjectID)
            return pClientRef;
    }

    return LTNULL;
}


void sm_RemoveOldClientRefObjects()
{
    LTLink *pListHead, *pCur, *pNext;
    LTObject *pObj;

    pListHead = &g_pServerMgr->m_Objects.m_Head;

    pCur = pListHead->m_pNext;
    while (pCur != pListHead)
    {
        pNext = pCur->m_pNext;

        pObj = (LTObject*)pCur->m_pData;
        if (pObj->m_InternalFlags & IFLAG_HASCLIENTREF)
        {
            sm_RemoveObjectFromWorld(pObj->sd->m_pObject);
        }

        pCur = pNext;
    }
}

bool CanOptimizeObject(LTObject *pObj)
{

    return (pObj->m_Flags & FLAG_FORCEOPTIMIZEOBJECT) || 
		( pObj->sd->m_cSpecialEffectMsg.Empty() && !( pObj->m_Flags & FLAG_OPTIMIZEMASK ));

}

uint16 sm_GetNewSoundTrackChangeFlags(CSoundTrack *pSoundTrack)
{
    uint16 wChangeFlags;

    // Set the initial change flags...
    wChangeFlags = CF_NEWOBJECT | CF_POSITION;
    return wChangeFlags;
}

void AddObjectToChangeList(LTObject *pObj)
{
    if (!g_pServerMgr->m_bWorldLoaded)
    {
        return;
    }

	if( pObj->m_InternalFlags & IFLAG_OBJECTGOINGAWAY )
		return;
 
    // This object should be in the world..
    ASSERT(pObj->m_InternalFlags & IFLAG_INWORLD);
    
    // Better not be re-adding it!
    ASSERT(!IsObjectInChangedList(pObj));

	dl_AddHead( &g_pServerMgr->m_ChangedObjectHead, &pObj->sd->m_ChangedNode, pObj );
}

LTRESULT SetObjectChangeFlags(LTObject *pObj, uint32 flags)
{
 
    // This object should be in the world..
    if (!(pObj->m_InternalFlags & IFLAG_INWORLD))
    {
        RETURN_ERROR_PARAM(1, SetObjectChangeFlags, LT_OBJECTNOTINWORLD, 
            pObj->sd->m_pClass->m_ClassName);
    }


 
    if (g_pServerMgr->m_ObjectMap[pObj->m_ObjectID].m_nRecordType != RECORDTYPE_LTOBJECT || !g_pServerMgr->m_ObjectMap[pObj->m_ObjectID].m_pRecordData)
    {
        RETURN_ERROR_PARAM(1, SetObjectChangeFlags, LT_ERROR, 
            pObj->sd->m_pClass->m_ClassName);
    }


    // Make sure not to re-add it and screw it up.
    if (pObj->sd->m_ChangeFlags == 0)
    {
        AddObjectToChangeList(pObj);
    }

    pObj->sd->m_ChangeFlags |= flags;
    ASSERT(pObj->sd->m_ChangeFlags);

    return LT_OK;
}

LPBASECLASS sm_AllocateObjectOfClass(ClassDef *pClass)
{
    LPBASECLASS pObject;
    CClassData *pClassData;

    pClassData = (CClassData*)pClass->m_pInternal[g_pServerMgr->m_ClassMgr.m_ClassIndex];

	LT_MEM_TRACK_ALLOC(pObject = (LPBASECLASS)sb_Allocate(&pClassData->m_ObjectBank), LT_MEM_TYPE_MISC);
    pObject->m_hObject = 0;
    pObject->m_pFirstAggregate = LTNULL;
    pClass->m_ConstructFn(pObject);

    return pObject;
}

void sm_FreeObjectOfClass(ClassDef *pClass, LPBASECLASS pObject)
{
    CClassData *pClassData;

    pClassData = (CClassData*)pClass->m_pInternal[g_pServerMgr->m_ClassMgr.m_ClassIndex];

    pClass->m_DestructFn(pObject);
    sb_Free(&pClassData->m_ObjectBank, pObject);
}

