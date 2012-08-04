// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __SPEAKER_H__
#define __SPEAKER_H__

#include "Character.h"

LINKTO_MODULE( Speaker );

class Speaker : public CCharacter
{
	public :

		uint32 EngineMessageFn(uint32 messageID, void *pData, float fData);
		virtual void	CreateAttachments() { }

	protected :

		void InitialUpdate();
		void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		bool ReadProp(const GenericPropList *pProps);

		void PreCreateSpecialFX(CHARCREATESTRUCT& cs);

        bool CanLipSync() { return false; }
		bool DoDialogueSubtitles() { return true; }

		virtual float	GetVerticalThreshold() const { return 0.f; }

		// No hit box needed.
		virtual void	CreateHitBox() { }

		// Get the User-Flag version of our surface type
		virtual uint32	GetUserFlagSurfaceType() { return 0; }
};

#endif
