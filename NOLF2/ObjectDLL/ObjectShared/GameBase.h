// ----------------------------------------------------------------------- //
//
// MODULE  : GameBase.h
//
// PURPOSE : Game base object class definition
//
// CREATED : 10/8/99
//
// (c) 1999 - 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_BASE_H__
#define __GAME_BASE_H__

#include "ltengineobjects.h"
#include "iobjectplugin.h"
#include "LtObjRef.h"

class CTransitionAggregate;
class CParsedMsg;

LINKTO_MODULE( GameBase );

class GameBase : public BaseClass, public ILTObjRefReceiver
{
	public :

        GameBase(uint8 nType=OT_NORMAL);
		virtual ~GameBase();

		virtual void CreateBoundingBox();
		virtual void RemoveBoundingBox();
		virtual void UpdateBoundingBox();

        uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		LTBOOL	CanTransition() const { return LTBOOL(!!m_pTransAgg); }

		virtual void AddToObjectList( ObjectList *pObjList, eObjListControl eControl = eObjListNODuplicates );

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

        virtual LTVector GetBoundingBoxColor();

		LTObjRefNotifier	m_hDimsBox;
        uint32      m_dwOriginalFlags;

		enum eUpdateControl
		{
			eControlDeactivation=0,
			eControlUpdateOnly
		};

		virtual void SetNextUpdate(LTFLOAT fDelta, eUpdateControl eControl=eControlDeactivation);

		virtual void MakeTransitionable( );
		virtual void DestroyTransitionAggregate( );

		// Called for each trigger message
		// Returns true if the message was handled
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);
	private :

		bool ReadProp(ObjectCreateStruct *pStruct);

		void TriggerMsg(HOBJECT hSender, const char* pMsg);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		CTransitionAggregate	*m_pTransAgg;
};


#endif  // __GAME_BASE_H__
