// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIVOLUME_H__
#define __AIVOLUME_H__

#include "Character.h"

class Speaker : public CCharacter
{
	public :

		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	protected :

		void InitialUpdate();
		void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		LTBOOL ReadProp(ObjectCreateStruct *pInfo);

		void PreCreateSpecialFX(CHARCREATESTRUCT& cs)
		{
			CCharacter::PreCreateSpecialFX(cs);

			cs.nTrackers = 0;
			cs.nDimsTracker = 0;
		}

        LTBOOL CanLipSync() { return LTFALSE; }
		LTBOOL DoDialogueSubtitles() { return LTTRUE; }
};

#endif
