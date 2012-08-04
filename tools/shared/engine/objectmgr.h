
// The object manager manages the lists of objects and creation of them.

#ifndef __OBJECTMGR_H__
#define __OBJECTMGR_H__


#ifndef __DE_OBJECTS_H__
#include "de_objects.h"
#endif

#ifndef __WORLDTREEHELPER_H__
#include "worldtreehelper.h"
#endif


#define OBJECT_PREALLOCATIONS	32
#define MODEL_PREALLOCATIONS	256
#define SPRITE_PREALLOCATIONS	128


// ---------------------------------------------------------------------- //
// Helpful conversion routines.
// None of these use the stupid 'instance' suffix.
// ---------------------------------------------------------------------- //

// b/w compatibility...
inline uint32 IsServerObject(LTObject *pObj)		{return pObj->GetCSType() == ServerType;}
inline LTBOOL HasWorldModel(LTObject *pObject)		{return pObject->HasWorldModel();}
inline ModelInstance* ToModel(LTObject *pObject)	{return pObject->ToModel();}
inline SpriteInstance *ToSprite(LTObject *pObject)	{return pObject->ToSprite();}
inline WorldModelInstance* ToWorldModel(LTObject *pObject)	{return pObject->ToWorldModel();}
inline DynamicLight* ToDynamicLight(LTObject *pObject){return pObject->ToDynamicLight();}
inline CameraInstance* ToCamera(LTObject *pObject)	{return pObject->ToCamera();}
inline LTParticleSystem* ToParticleSystem(LTObject *pObject)  {return pObject->ToParticleSystem();}
inline LTPolyGrid* ToPolyGrid(LTObject *pObject)      {return pObject->ToPolyGrid();}
inline LineSystem* ToLineSystem(LTObject *pObject)	{return pObject->ToLineSystem();}
inline ContainerInstance* ToContainer(LTObject *pObject)	{return pObject->ToContainer();}
inline Canvas* ToCanvas(LTObject *pObject)			{return pObject->ToCanvas();}


// ---------------------------------------------------------------------- //
// Structures.
// ---------------------------------------------------------------------- //

class ObjectMgr : public WorldTreeHelper
{
public:

					ObjectMgr();


// WorldTreeHelper overrides.
public:

	virtual uint32	IncFrameCode();
	virtual uint32	GetFrameCode();


public:

	uint32		m_CurFrameCode;

	LTLink		m_InternalLink; // Used by the ObjectMgr module.

	// Banks for objects.
	StructBank	m_ParticleBank;
	StructBank	m_LineBank;

	// Attachments.
	StructBank	m_AttachmentBank;

	ObjectBank<LTObject>				m_ObjectBankNormal;
	ObjectBank<ModelInstance>			m_ObjectBankModel;
	ObjectBank<WorldModelInstance>		m_ObjectBankWorldModel;
	ObjectBank<SpriteInstance>			m_ObjectBankSprite;
	ObjectBank<DynamicLight>			m_ObjectBankLight;
	ObjectBank<CameraInstance>			m_ObjectBankCamera;
    ObjectBank<LTParticleSystem>          m_ObjectBankParticleSystem;
    ObjectBank<LTPolyGrid>                m_ObjectBankPolyGrid;
	ObjectBank<LineSystem>				m_ObjectBankLineSystem;
	ObjectBank<ContainerInstance>		m_ObjectBankContainer;
	ObjectBank<Canvas>					m_ObjectBankCanvas;

	// Useful so it can tree the banks like an array.
	BaseObjectBank	*m_ObjectBankPointers[NUM_OBJECTTYPES];

	// List of each object type.
	LTList	m_ObjectLists[NUM_OBJECTTYPES];
};



// ---------------------------------------------------------------------- //
// Functions.
// ---------------------------------------------------------------------- //

LTRESULT om_Init(ObjectMgr *pMgr, LTBOOL bClient);
LTRESULT om_Term(ObjectMgr *pMgr);

LTRESULT om_CreateObject(ObjectMgr *pMgr, ObjectCreateStruct *pStruct, LTObject **ppObject);
LTRESULT om_DestroyObject(ObjectMgr *pMgr, LTObject *pObject);

// Create an attachment.
LTRESULT om_CreateAttachment(ObjectMgr *pMgr, LTObject *pParent, uint16 childID, int iSocket,
	LTVector *pOffset, LTRotation *pRotationOffset, Attachment **ppAttachment);

// Remove an attachment.  Returns DE_OK or DE_ERROR if it can't find the attachment.
LTRESULT om_RemoveAttachment(ObjectMgr *pMgr, LTObject *pParent, Attachment *pAttachment);

// Sets m_SerializeID to INVALID_SERIALIZEID for all objects.
void om_ClearSerializeIDs(ObjectMgr *pMgr);


// Remove all attachments from the object.
inline void om_RemoveAttachments(ObjectMgr *pMgr, LTObject *pObj)
{
	Attachment *pCur, *pNext;

	pCur = pObj->m_Attachments;
	while(pCur)
	{
		pNext = pCur->m_pNext;
		sb_Free(&pMgr->m_AttachmentBank, pCur);
		pCur = pNext;
	}
	pObj->m_Attachments = NULL;
}



#endif  // __OBJECTMGR_H__



