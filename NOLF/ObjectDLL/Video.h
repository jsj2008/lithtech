// (c) 1999 Monolith Productions, Inc.  All Rights Reserved

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "ClientSFX.h"
#include "SFXMsgIds.h"

class Video : public CClientSFX
{
	public :

		Video();
		~Video();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        void	Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void	Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

        LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	InitialUpdate(int nInfo);

		void	HandleMsg(HOBJECT hSender, const char* szMsg);

	private :

        LTBOOL		m_bOn;
		HSTRING		m_hstrVideo;
};

#endif // __Video_H__