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

LINKTO_MODULE( KeyPad );

class CCharacter;

class KeyPad : public Prop
{
	public :

		KeyPad();
		~KeyPad();

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	protected :

        LTBOOL  ReadProp(ObjectCreateStruct *pData);
        LTBOOL  InitialUpdate();
		void	Update();

		void	SetupDisabledState();
		void	HandleGadgetMsg(const CParsedMsg &cMsg);

	protected :

		HSTRING		m_hstrDisabledCmd;
		HSTRING		m_hstrPickSound;
		LTObjRefNotifier	m_hDeciphererModel;
        HLTSOUND    m_hPickSound;
        LTFLOAT     m_fMinPickTime;
        LTFLOAT     m_fMaxPickTime;
        LTFLOAT     m_fPickSoundRadius;

		LTBOOL		m_bSpaceCodeBreaker;

		CTimer		m_DecipherTimer;

	private :

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		void	SpawnGadget();
};

#endif // __KEY_PAD_H__