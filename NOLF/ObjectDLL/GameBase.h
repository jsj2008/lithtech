// ----------------------------------------------------------------------- //
//
// MODULE  : GameBase.h
//
// PURPOSE : Game base object class definition
//
// CREATED : 10/8/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_BASE_H__
#define __GAME_BASE_H__

#include "ltengineobjects.h"
#include "iobjectplugin.h"

class GameBase : public BaseClass
{
	public :

        GameBase(uint8 nType=OT_NORMAL);
		virtual ~GameBase();

		virtual void CreateBoundingBox();
		virtual void RemoveBoundingBox();
		virtual void UpdateBoundingBox();

        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		ObjectList* GetMarkList() const { return m_pMarkList; }
		void AddMark(HOBJECT hMark);

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

        virtual LTVector GetBoundingBoxColor();

		HOBJECT		m_hDimsBox;
		ObjectList*	m_pMarkList;
        uint32      m_dwOriginalFlags;

		uint32		m_nSaveVersion;
		HSTRING		m_hstrSave;

		enum eUpdateControl
		{
			eControlDeactivation=0,
			eControlUpdateOnly
		};

		virtual void SetNextUpdate(LTFLOAT fDelta, eUpdateControl eControl=eControlDeactivation);

	private :

		void TriggerMsg(HOBJECT hSender, const char* pMsg);
		void HandleLinkBroken(HOBJECT hObj);
		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);
};


#endif  // __GAME_BASE_H__
