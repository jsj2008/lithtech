/****************************************************************************
;
;	MODULE:			SkeletonObject.cpp
;
;	PURPOSE:		SkeletonObject class implementation
;
;	HISTORY:		2/15/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/


#include "stdafx.h"
#include "SkeletonObject.h"

BEGIN_CLASS(SkeletonObject)
	// For this generic object, we'll have it be a model so we'll need
	// filename and skin props. If you don't want this object to be a model
	// you can remove these two props.
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_FILENAME | PF_LOCALDIMS | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	
	// We'll also add a prop for our DWORD variable
	ADD_LONGINTPROP(SomeNumber, 0)

	// TODO: Add additional props here

END_CLASS_DEFAULT(SkeletonObject, GameBase, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SkeletonObject::SkeletonObject()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
SkeletonObject::SkeletonObject() : GameBase()
{
	// Initialize the variables
	m_dwSomeVariable = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SkeletonObject::~SkeletonObject()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //
SkeletonObject::~SkeletonObject()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SkeletonObject::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //
uint32 SkeletonObject::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			// First we call down to the base class.
			// This isn't currently done in all objects, but 
			// it really is good practice to do it this way.
			LTRESULT ret = GameBase::EngineMessageFn(messageID, pData, fData);

			if((fData == PRECREATE_WORLDFILE) || (fData == PRECREATE_STRINGPROP))
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			// Passing this message to the base class first insures that all the props
			// will be read in when we get here
			PostPropRead((ObjectCreateStruct*)pData);
			
			return ret;
		}

		case MID_INITIALUPDATE:
		{
			// Initial update
			InitialUpdate();
			break;
		}

		case MID_UPDATE:
		{
			// Main update function
			Update();
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
			break;
		}

		default: break;
	}

	return(GameBase::EngineMessageFn(messageID, pData, fData));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SkeletonObject::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
LTBOOL SkeletonObject::ReadProp(ObjectCreateStruct *pStruct)
{
	// Sanity check
    if (!pStruct) return LTFALSE;

	// Read them in
	GenericProp genProp;
	if(g_pLTServer->GetPropGeneric("SomeNumber", &genProp) == LT_OK)
	{
	    m_dwSomeVariable = (uint32)genProp.m_Long;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SkeletonObject::PostPropRead
//
//	PURPOSE:	Handle post-property initialization
//
// ----------------------------------------------------------------------- //
LTBOOL SkeletonObject::PostPropRead(ObjectCreateStruct *pStruct)
{
	if(!pStruct) return LTFALSE;

	// TODO: Post property initialization

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SkeletonObject::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //
void SkeletonObject::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	// Save our important data members
	SAVE_DWORD(m_dwSomeVariable);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SkeletonObject::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //
void SkeletonObject::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	// Load our important data members
	LOAD_DWORD(m_dwSomeVariable);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SkeletonObject::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
uint32 SkeletonObject::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	// TODO: Handle object to object messages here
	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SkeletonObject::InitialUpdate
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //
LTBOOL SkeletonObject::InitialUpdate()
{
	// TODO: This is the object's first update
	// do any necessary stuff you want here such as setting
	// the next update time or the object flags.

	// For example... setting the object's flags such that the
	// object will be activateable when the user moves
	// the cursor over it. (The client can examine these flags)
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SkeletonObject::Update
//
//	PURPOSE:	Main update function
//
// ----------------------------------------------------------------------- //
LTBOOL SkeletonObject::Update()
{
	// TODO: If your object recieves updates,
	// do any necessary stuff here.
	// Be sure to call SetNextUpdate() if you want another update.

	return LTTRUE;
}