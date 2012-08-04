// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrush.h
//
// PURPOSE : VolumeBrush definition
//
// CREATED : 1/29/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUME_BRUSH_H__
#define __VOLUME_BRUSH_H__

#include "GameBase.h"
#include "ContainerCodes.h"
#include "DamageTypes.h"

class VolumeBrush : public GameBase
{
	public :

		VolumeBrush();
		~VolumeBrush();

        LTFLOAT			GetGravity()    const { return m_fGravity; }
        LTFLOAT			GetFriction()   const { return m_fFriction; }
        LTFLOAT			GetViscosity()  const { return m_fViscosity; }
		ContainerCode	GetCode()		const { return m_eContainerCode; }
        LTVector		GetCurrent()    const { return m_vCurrent; }
        LTBOOL			GetHidden()     const { return m_bHidden; }

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		virtual void ReadProp(ObjectCreateStruct *pStruct);
		virtual void UpdatePhysics(ContainerPhysics* pCPStruct);
		virtual void UpdateLiquidPhysics(ContainerPhysics* pCPStruct);

		virtual void WriteSFXMsg(HMESSAGEWRITE hMessage);
		virtual void HandleTrigger(HOBJECT hSender, const char* szMsg);

		HSTRING			m_hstrSurfaceSprite;
		HOBJECT			m_hSurfaceObj;

        LTVector        m_vCurrent;
        LTVector        m_vSurfaceColor1;
        LTVector        m_vSurfaceColor2;
        LTVector        m_vLastPos;
        LTVector        m_vFogColor;
        LTVector        m_vTintColor;
        LTVector        m_vLightAdd;
        LTFLOAT         m_fViscosity;
        LTFLOAT         m_fFriction;
        LTFLOAT         m_fDamage;
        LTFLOAT         m_fSurfaceHeight;
        LTFLOAT         m_fGravity;
        LTFLOAT         m_fXScaleMin;
        LTFLOAT         m_fXScaleMax;
        LTFLOAT         m_fYScaleMin;
        LTFLOAT         m_fYScaleMax;
        LTFLOAT         m_fXScaleDuration;
        LTFLOAT         m_fYScaleDuration;
        LTFLOAT         m_fFogFarZ;
        LTFLOAT         m_fFogNearZ;
        LTFLOAT         m_fSurfAlpha;
        uint32			m_dwNumSurfPolies;
        uint32          m_dwFlags;
        uint32          m_dwSaveFlags;

		DamageType		m_eDamageType;
		ContainerCode	m_eContainerCode;
        LTBOOL          m_bShowSurface;
        LTBOOL          m_bFogEnable;
        LTBOOL          m_bHidden;
        LTBOOL          m_bAdditive;
        LTBOOL          m_bMultiply;
		LTBOOL			m_bCanPlayMoveSnds;

        uint8           m_nSfxMsgId;
		uint8			m_nSoundFilterId;

	private :

		void CreateSurface();
		void CreateSpecialFXMsg();

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void CacheFiles();

		void PostPropRead(ObjectCreateStruct *pStruct);
		void InitialUpdate();
		void Update();
};

class CVolumePlugin : public IObjectPlugin
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

	  CSoundFilterMgrPlugin m_SoundFilterMgrPlugin;
};

#endif // __VOLUME_BRUSH_H__