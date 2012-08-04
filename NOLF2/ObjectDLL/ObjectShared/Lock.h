// ----------------------------------------------------------------------- //
//
// MODULE  : Lock.cpp
//
// PURPOSE : Definition of the Lock object
//
// CREATED : 01/11/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LOCK_H__
#define __LOCK_H__

#include "Prop.h"

LINKTO_MODULE( Lock );

class Lock : public Prop
{
	public :

		Lock();
		~Lock();

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
        LTBOOL   InitialUpdate();

		void	SetupUnlockState();
		void	HandleGadgetMsg(HOBJECT hSender, const CParsedMsg &cMsg);

		HSTRING	m_hstrUnlockCmd;
		HSTRING m_hstrUnlockSnd;
		HSTRING m_hstrPickSnd;
        LTFLOAT  m_fSndRadius;
        LTFLOAT  m_fUnlockHitPts;
        LTFLOAT  m_fMinUnlockHitPts;
        LTFLOAT  m_fMaxUnlockHitPts;
        LTBOOL   m_bShootable;
        LTBOOL   m_bWeldable;
        LTBOOL   m_bLightable;
        LTBOOL   m_bRemoveWhenDone;
        LTBOOL   m_bUnlocked;

	private :

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);
};

#endif // __LOCK_H__