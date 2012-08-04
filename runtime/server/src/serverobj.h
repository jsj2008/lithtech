//------------------------------------------------------------------
//
//	FILE	  : ServerObj.h
//
//	PURPOSE	  : Defines the CServerObj -- the base class for all server objects.
//
//	CREATED	  : November 26 1996
//
//	COPYRIGHT : MONOLITH Inc 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __SERVEROBJ_H__
#define __SERVEROBJ_H__


#include "packet.h"

class HHashElement;
struct ClassDef;
struct Client;
struct UsedFile;

// This turns an LPBASECLASS pointer into a LTObject.
#define GetObjectServerObj(_pObject) ((LTObject*)((LPBASECLASS)(_pObject))->m_hObject)

// This turns a LTObject into an LPBASECLASS.
#define ServerObjToBaseClass(_pObject) ((LPBASECLASS)((_pObject)->sd->m_pObject))

// This turns an LPBASECLASS into a LTObject.
#define BaseClassToServerObj(_pBaseClass) HandleToServerObj((_pBaseClass)->m_hObject)

// This turns an LPBASECLASS into a handle.
#define BaseClassToHandle(_pBaseClass) ServerObjToHandle(GetObjectServerObj(_pBaseClass))

// This turns a HOBJECT into an LPBASECLASS.
#define HandleToBaseClass(_handle) ServerObjToBaseClass(HandleToServerObj(_handle))

// This turns an HOBJECT into a LTObject (an HOBJECT IS a LTObject so it just casts).
#define HandleToServerObj(_theHandle) ((LTObject*)(_theHandle))

// This turns a LTObject into an HOBJECT.
#define ServerObjToHandle(_theObj) ((HOBJECT)(_theObj))

class SObjData
{
public:

					SObjData() {
						uint32 i;
						for (i=0; i < MAX_MODEL_TEXTURES; i++)		m_pSkins[i] = NULL;
						for (i=0; i < MAX_MODEL_RENDERSTYLES; i++)	m_pRenderStyles[i] = NULL; }

	LTLink			m_Links;	// All the InterLinks this object is using.

	HHashElement    *m_hName;		// LTNULL if the name is "" or set if it has a valid name.

	float			m_NextUpdate;	// If this is <= 0, then it never updates the object.
										// If it's > 0 and goes to <= 0, then it updates the object.	
	
	Client			*m_pClient;		// If this is set, this object is a client's object..
	LPBASECLASS		m_pObject;		// Object type is m_ObjectType.
	ClassDef		*m_pClass;		// The class of this object.

	LTLink			*m_pIDLink;

	CPacket_Read	m_cSpecialEffectMsg;	// The special effect message, if any.

	UsedFile		*m_pFile;			// Skin name and model name.  Allocated or LTNULL.
	
	union 
	{
		UsedFile	*m_pSkin;
		UsedFile	*m_pSkins[MAX_MODEL_TEXTURES]; 
	};
	
	union 
	{
		UsedFile	*m_pRenderStyle;
		UsedFile	*m_pRenderStyles[MAX_MODEL_TEXTURES]; 
	};

	LTLink			m_ListNode;			// The node for the server mugger to use.

	LTLink			m_ChangedNode;	// Used in the linked list of changed objects..

	uint16			m_ChangeFlags;		// Stored during updates.
	uint16			m_NetFlags;			// Net flags (combination of NETFLAG_ defines).
};


#endif  // __SERVEROBJ_H__





