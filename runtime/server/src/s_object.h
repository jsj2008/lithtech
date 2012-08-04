#ifndef __S_OBJECT_H__
#define __S_OBJECT_H__

class CServerMgr;
class CSoundTrack;
struct ClassDef;
class Model;
struct LTRecord;
struct ClientRef;
class CPacket_Read;


// Searches the changed list for the object.. should only be used for debugging.
bool IsObjectInChangedList(LTObject *pObj);

bool CanOptimizeObject(LTObject *pObj);

// Either adds or removes the object from the BSP depending on its flags and stuff.
LTRESULT sm_UpdateInBspStatus(LTObject *pObject);

// Figures out what change flags need to be set for this object to 
// tell clients about it.
uint32 sm_GetNewObjectChangeFlags(LTObject *pObject);

// Figures out what change flags need to be set for this soundtrack to 
// tell clients about it.
uint16 sm_GetNewSoundTrackChangeFlags(CSoundTrack *pSoundTrack);

// Adds the object to the list of changed objects.
void AddObjectToChangeList(LTObject *pObj);

// ORs the object's flags with the flags you specify and adds
// the object to the 'changed object' list.
LTRESULT SetObjectChangeFlags(LTObject *pObj, uint32 flags);

// Registers a script callback function at a certain position for a model.
bool RegisterModelCallback(LTObject *pObj, uint32 iAnim, uint32 msTime, char *pFunctionName);

// Allocate/construct and deallocate/destruct objects of a class.
LPBASECLASS sm_AllocateObjectOfClass(ClassDef *pClass);

void sm_FreeObjectOfClass(ClassDef *pClass, LPBASECLASS pObject);

// Loads the model.
Model* LoadModel(char *pName);

// Gets a pointer to the given model, or loads it if it's not loaded yet.
Model* LoadOrGetModel(char *pName);

// Deletes all the models that the server has.
void DeleteModels();



// Fully updates the object (called once per frame).
void FullObjectUpdate(LTObject *pObj);

// Loads and instantiates objects from the given world file.
LTRESULT LoadObjects(ILTStream *pStream, const char *pWorldName, bool bAllObjects, uint32 nObjectDataOffset );

// Add this object to the 'remove list'..
void AddObjectToRemoveList(LTObject *pObj);

// Remove the objects in the remove list.
void sm_RemoveObjectsThatNeedToGetRemoved();


// Gets the object based on its ID.  Returns LTNULL if the ID is invalid.
LTObject* sm_FindObject(uint16 objectID);

// Gets the record from the objectmap based on its ID.  Returns LTNULL if the ID is invalid.
LTRecord* sm_FindRecord(uint16 objectID);

// Find the ID link in the free ID list.
LTLink* sm_FindInFreeList(uint16 objectID);


// Set an object's special effect message.
void sm_SetObjectSpecialEffectMessage(LTObject *pObj, const CPacket_Read &cPacket);

// Change an object's state flags (its active/inactive IFLAGs).
void sm_SetObjectStateFlags(LTObject *pObj, uint32 flags);

// Clear the client reference list.
void sm_ClearClientReferenceList();

// Find a client reference.
ClientRef* sm_FindClientRefFromObject(LTObject *pObj);

// Destroy the keepalive object list.
void sm_DestroyKeepaliveObjectList();

// Gets rid of any objects with client references.
void sm_RemoveOldClientRefObjects();


#endif


