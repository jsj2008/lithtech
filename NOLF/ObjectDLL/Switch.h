// ----------------------------------------------------------------------- //
//
// MODULE  : Switch.h
//
// PURPOSE : A Switch object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#ifndef __SWITCH_H__
#define __SWITCH_H__

#include "Door.h"

class Switch : public Door
{
	public:

		Switch();
		virtual ~Switch();

	protected:

        uint32  EngineMessageFn(uint32 messageID, void *pData, float fData);

		virtual void SetOpening();
		virtual void SetClosing();

		HSTRING m_hstrOnTriggerTarget;
		HSTRING m_hstrOnTriggerMessage;
		HSTRING m_hstrOffTriggerTarget;
		HSTRING m_hstrOffTriggerMessage;

	private :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
        void    Save(HMESSAGEWRITE hWrite, uint8 nType);
        void    Load(HMESSAGEREAD hRead, uint8 nType);
};


#endif // __SWITCH_H__