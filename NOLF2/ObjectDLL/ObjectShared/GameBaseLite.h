// ----------------------------------------------------------------------- //
//
// MODULE  : GameBaseLite.h
//
// PURPOSE : "Lite" game base object class definition  (For game objects without an HOBJECT)
//
// CREATED : 7/12/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_BASE_LITE_H__
#define __GAME_BASE_LITE_H__

#include "ltengineobjects.h"
#include "iobjectplugin.h"
#include "LtObjRef.h"
#include "LiteObjectMgr.h" // This is included here so objects can save/load/etc. GameBaseLite objects

#include <string>

class CParsedMsg;

LINKTO_MODULE( GameBaseLite );

class GameBaseLite : public BaseClass, public ILTObjRefReceiver
{
public :
    GameBaseLite(bool bStartActive = true);
	virtual ~GameBaseLite();

    virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

	// Save/load support
	virtual void Save(ILTMessage_Write *pMsg);
	virtual void Load(ILTMessage_Read *pMsg);

	// Update support
	virtual void InitialUpdate(float fUpdateType);
	virtual void Update() {}

	virtual bool IsActive() const { return m_bActive; }
	virtual void Activate();
	virtual void Deactivate();

	// Class access
	virtual void SetClass(HCLASS hClass) { m_hClass = hClass; }
	virtual HCLASS GetClass() const { return m_hClass; }

	// Name access
	virtual void SetName(const char *pName);
	virtual const char *GetName() const { return m_sName.c_str(); }

	// Serialization ID access
	virtual void SetSerializeID(uint32 nID) { m_nSerializeID = nID; }
	virtual uint32 GetSerializeID() const { return m_nSerializeID; }

	// Implementing classes will have this function called
	// when HOBJECT ref points to gets deleted.
	virtual void OnLinkBroken(LTObjRefNotifier *pRef, HOBJECT hObj) {}

protected :

	// Read the properties from the OCS
	virtual bool ReadProp(ObjectCreateStruct *pOCS);

	// Called for each trigger message
	// Returns true if the message was handled
	virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

private :

	void TriggerMsg(HOBJECT hSender, const char* pMsg);

	// These functions are not supported on lite objects
	virtual uint32 OnObjectCreated(float) 
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnUpdate()
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnTouch(HOBJECT, float)
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnLinkBroken(HOBJECT)
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnCrush(HOBJECT)
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnLoad(ILTMessage_Read *, float)
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnSave(ILTMessage_Write *, float)
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnAffectPhysics(ContainerPhysics *)
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnParentAttachmentRemoved()
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnActivate()
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnDeactivate()
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }
	virtual uint32 OnAllObjectsCreated()
		{ ASSERT(!"Invalid function call on lite game object"); return LT_ERROR; }

    uint8 GetType() const
		{ ASSERT(!"Invalid function call on lite game object"); return OT_NORMAL; }
    void SetType(uint8 type)
		{ ASSERT(!"Invalid function call on lite game object"); }
    HOBJECT GetHOBJECT() const
		{ ASSERT(!"Invalid function call on lite game object"); return 0; }
    void SetHOBJECT(HOBJECT h)
		{ ASSERT(!"Invalid function call on lite game object"); }
    void AddAggregate(LPAGGREGATE pAggregate)
		{ ASSERT(!"Invalid function call on lite game object"); }
    bool RemoveAggregate(LPAGGREGATE pAggregate)
		{ ASSERT(!"Invalid function call on lite game object"); return false; }

	// These functions may only be called by the engine
	virtual uint32 OnPrecreate(ObjectCreateStruct* pOCS, float precreateType);
	virtual uint32 EngineMessageFn(uint32 messageID, void *pData, float lData) 
		{ return BaseClass::EngineMessageFn(messageID, pData, lData); }

	// These variables are invalid on these objects
	HOBJECT m_hObject;
    uint8 m_nType;

private:
	// Internal variables

	// Is this object active?
	bool m_bActive;

	// Is this object registered with the lite object mgr?
	bool m_bRegistered;

	// The class of the object
	HCLASS m_hClass;

	// Name of the object
	std::string m_sName;

	// Serialization ID of the object
	uint32 m_nSerializeID;
};


#endif  // __GAME_BASE_LITE_H__
