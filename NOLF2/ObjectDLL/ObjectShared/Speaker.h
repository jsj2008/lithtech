// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __SPEAKER_H__
#define __SPEAKER_H__

#include "Character.h"

LINKTO_MODULE( Speaker );

class Speaker : public CCharacter
{
	public :

		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	protected :

		void InitialUpdate();
		void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		LTBOOL ReadProp(ObjectCreateStruct *pInfo);

		void PreCreateSpecialFX(CHARCREATESTRUCT& cs);

        LTBOOL CanLipSync() { return LTFALSE; }
		LTBOOL DoDialogueSubtitles() { return LTTRUE; }

		virtual float	GetVerticalThreshold() const { return 0.f; }
		virtual float	GetInfoVerticalThreshold() const { return 0.f; }
};

#endif
