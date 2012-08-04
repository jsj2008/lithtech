// ----------------------------------------------------------------------- //
//
// MODULE  : ServerMeleeCollisionController.h
//
// PURPOSE : Server-side controller for managing rigidbody collisions
//
// CREATED : 01/20/05
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVERMELEECOLLISIONCONTROLLER_H__
#define __SERVERMELEECOLLISIONCONTROLLER_H__

// ----------------------------------------------------------------------- //

class CServerMeleeCollisionController : public IAggregate
{
	public:

		CServerMeleeCollisionController();
		virtual ~CServerMeleeCollisionController();

		void Init(HOBJECT hParent);

	protected :

		uint32	EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float lData);
		uint32	ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg);

		LTObjRef	m_hObject;			// The object I'm associated with.

	private :

		void Save(ILTMessage_Write *pMsg, uint8 nType);
		void Load(ILTMessage_Read *pMsg, uint8 nType);
};

// ----------------------------------------------------------------------- //

#endif//__SERVERMELEECOLLISIONCONTROLLER_H__
