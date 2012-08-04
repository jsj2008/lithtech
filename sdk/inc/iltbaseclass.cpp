/*!

 MODULE  : iltbaseclass.cpp

 PURPOSE : ILTBaseClass implementation for C++ LT game objects

 CREATED : 5/31/2001

*/

#include "iltbaseclass.h"
#include "iltcommon.h"
#include "iaggregate.h"
#include <stdio.h>

/*!
 Used by some tools that need the class defs but not the server stuff.

*/
#ifdef AUTO_DEFINE_CLASSES
	DEFINE_CLASSES()
#endif

/*!

 ILTBaseClass.

*/

ILTBaseClass::~ILTBaseClass()
{
}

void
ILTBaseClass::AddAggregate(
	LPAGGREGATE pAggregate
	)
{
	pAggregate->m_pNextAggregate = m_pFirstAggregate;
	m_pFirstAggregate = pAggregate;
} // ILTBaseClass::AddAggregate


bool
ILTBaseClass::RemoveAggregate(
	LPAGGREGATE pAggregate
	)
{
	if (!m_pFirstAggregate)
		return false;

/*!
	 See if the aggregate is the first thing on the list.

*/
	if(m_pFirstAggregate == pAggregate)
	{
		m_pFirstAggregate = m_pFirstAggregate->m_pNextAggregate;
		return true;
	}
	else
	{


/*!
		 Start on the second item on the list.

*/
		LPAGGREGATE pPrevAggregate = m_pFirstAggregate;
		LPAGGREGATE pCurAggregate = pPrevAggregate->m_pNextAggregate;;

		while (pCurAggregate)
		{
			if (pCurAggregate == pAggregate)
			{
				pPrevAggregate->m_pNextAggregate = pCurAggregate->m_pNextAggregate;
				return true;
			}

			pPrevAggregate = pCurAggregate;
			pCurAggregate = pCurAggregate->m_pNextAggregate;
		}
	}

	return false;
} // ILTBaseClass::RemoveAggregate


uint32
ILTBaseClass::EngineMessageFn(
	uint32 messageID, 
	void *pData, 
	float fData
	)
{
/*!
	 Handle ReadProp.

*/

/*!
	 Call the aggregates.

*/
	LPAGGREGATE pAggregate = m_pFirstAggregate;
	while(pAggregate)
	{
		pAggregate->EngineMessageFn(this, messageID, pData, fData);
		pAggregate = pAggregate->m_pNextAggregate;
	}

/*!
	 Default return is 1.

*/
	return 1;
} // ILTBaseClass::EngineMessageFn


uint32 ILTBaseClass::OnPrecreate( ObjectCreateStruct* pOCS, float precreateType )
{ return EngineMessageFn(MID_PRECREATE, pOCS, precreateType); }

uint32 ILTBaseClass::OnObjectCreated( float createType )
{ return EngineMessageFn(MID_OBJECTCREATED, NULL, createType); }

uint32 ILTBaseClass::OnUpdate()
{ return EngineMessageFn(MID_UPDATE, NULL, 0.0f); }

uint32 ILTBaseClass::OnTouch( HOBJECT obj, float force )				
{ return EngineMessageFn(MID_TOUCHNOTIFY, obj, force); }

uint32 ILTBaseClass::OnLinkBroken( HOBJECT linkObj )			
{ return EngineMessageFn(MID_LINKBROKEN, linkObj, 0.0f); }

uint32 ILTBaseClass::OnCrush( HOBJECT crusherObj )		
{ return EngineMessageFn(MID_CRUSH, crusherObj, 0.0f); }

uint32 ILTBaseClass::OnLoad( ILTMessage_Read *readMsg, float dwParam )			
{ return EngineMessageFn(MID_LOADOBJECT, readMsg, dwParam); }

uint32 ILTBaseClass::OnSave( ILTMessage_Write *writeMsg, float dwParam )			
{ return EngineMessageFn(MID_SAVEOBJECT, writeMsg, dwParam); }

uint32 ILTBaseClass::OnAffectPhysics( ContainerPhysics* pCP )	
{ return EngineMessageFn(MID_AFFECTPHYSICS, pCP, 0.0f); }

uint32 ILTBaseClass::OnParentAttachmentRemoved( )							
{ return EngineMessageFn(MID_PARENTATTACHMENTREMOVED, NULL, 0.0f); }

uint32 ILTBaseClass::OnActivate()							
{ return EngineMessageFn(MID_ACTIVATING, NULL, 0.0f); }

uint32 ILTBaseClass::OnDeactivate()							
{ return EngineMessageFn(MID_DEACTIVATING, NULL, 0.0f); }

uint32 ILTBaseClass::OnAllObjectsCreated()							
{ return EngineMessageFn(MID_ALLOBJECTSCREATED, NULL, 0.0f); }

uint32 ILTBaseClass::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pRead)
{
	LPAGGREGATE pAggregate;

/*!
	 Call the aggregates.

*/
	pAggregate = m_pFirstAggregate;
	while(pAggregate)
	{
		pAggregate->ObjectMessageFn(this, hSender, pRead);
		pAggregate = pAggregate->m_pNextAggregate;
	}

	return 1;
}
