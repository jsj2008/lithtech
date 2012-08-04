// ----------------------------------------------------------------------- //
//
// MODULE  : Lightning.h
//
// PURPOSE : Lightning - Definition
//
// CREATED : 4/17/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTNING_H__
#define __LIGHTNING_H__

#include "ClientSFX.h"
#include "SFXMsgIds.h"

LINKTO_MODULE( Lightning );

class Lightning : public CClientSFX
{
	public :

		Lightning();
		~Lightning();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

        LTBOOL		m_bOn;

        LTVector	m_vEndPos;
        LTVector	m_vLightColor;
        LTFLOAT		m_fMinWidth;
        LTFLOAT		m_fMaxWidth;
        LTFLOAT		m_fLifeTime;
        LTFLOAT		m_fAlphaLifeTime;
        LTFLOAT		m_fMinDelayTime;
        LTFLOAT		m_fMaxDelayTime;
        LTFLOAT		m_fPerturb;
        LTFLOAT		m_fLightRadius;
        LTFLOAT		m_fSoundRadius;
        LTBOOL		m_bOneTimeOnly;
        LTBOOL		m_bDynamicLight;
        LTBOOL		m_bPlaySound;
        LTBOOL		m_bAdditive;
        LTBOOL		m_bMultiply;
        uint8		m_nWidthStyle;
        uint8		m_nNumSegments;

        LTVector	m_vInnerColorStart;
        LTVector	m_vInnerColorEnd;
        LTVector	m_vOuterColorStart;
        LTVector	m_vOuterColorEnd;
					
        LTFLOAT		m_fAlphaStart;
        LTFLOAT		m_fAlphaEnd;

		HSTRING		m_hstrTexture;

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

        LTBOOL ReadProp(ObjectCreateStruct *pStruct);
		void InitialUpdate(int nInfo);
};

#endif // __LIGHTNING_H__