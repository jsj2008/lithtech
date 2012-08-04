// ----------------------------------------------------------------------- //
//
// MODULE  : Prop.h
//
// PURPOSE : Model Prop - Definition
//
// CREATED : 10/9/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROP_H__
#define __PROP_H__

#include "GameBase.h"
#include "DestructibleModel.h"


class Prop : public GameBase
{
	public :

 		Prop();
		virtual ~Prop();

		CDestructible* GetDestructible() { return &m_damage; }

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		virtual void TriggerMsg(HOBJECT hSender, const char* szMsg);

		CDestructibleModel			m_damage;

        LTBOOL						m_bMoveToFloor;
        LTBOOL                      m_bFirstUpdate;
        LTVector                    m_vScale;
        LTVector                    m_vObjectColor;
        LTFLOAT                     m_fAlpha;
        uint32                      m_dwUsrFlgs;
        uint32                      m_dwFlags;
        uint32                      m_dwFlags2;
		HSTRING						m_hstrTouchSound;
        LTFLOAT                     m_fTouchSoundRadius;
        HLTSOUND                    m_hTouchSnd;

		char*						m_pDebrisOverride;

	private :

		void	ReadProp(ObjectCreateStruct *pStruct);
		void	PostPropRead(ObjectCreateStruct *pStruct);
		void	InitialUpdate();

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void	CacheFiles();

		void	HandleTouch(HOBJECT hObj);
};

class CPropPlugin : public IObjectPlugin
{
  public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

  protected :

	  CDestructibleModelPlugin m_DestructibleModelPlugin;
};

#endif // __PROP_H__