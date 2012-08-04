// ----------------------------------------------------------------------- //
//
// MODULE  : KeyPad.h
//
// PURPOSE : Definition of the Key pad Model
//
// CREATED : 4/30/99
//
// ----------------------------------------------------------------------- //

#ifndef __KEY_PAD_H__
#define __KEY_PAD_H__

#include "Prop.h"
#include "Timer.h"

class CCharacter;

class KeyPad : public Prop
{
	public :

		KeyPad();
		~KeyPad();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected :

        LTBOOL  ReadProp(ObjectCreateStruct *pData);
        LTBOOL  InitialUpdate();
		void	Update();

		void	SetupDisabledState();
		void	HandleGadgetMsg(ConParse & parse);

	protected :

		HSTRING		m_hstrDisabledCmd;
		HSTRING		m_hstrPickSound;
		HOBJECT		m_hDeciphererModel;
        HLTSOUND    m_hPickSound;
        LTFLOAT     m_fMinPickTime;
        LTFLOAT     m_fMaxPickTime;
        LTFLOAT     m_fPickSoundRadius;

		LTBOOL		m_bSpaceCodeBreaker;

		CTimer		m_DecipherTimer;

	private :

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

		void	SpawnGadget();
};

#endif // __KEY_PAD_H__