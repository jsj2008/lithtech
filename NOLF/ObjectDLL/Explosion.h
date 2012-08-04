// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.h
//
// PURPOSE : Explosion - Definition
//
// CREATED : 11/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __EXPLOSION_H__
#define __EXPLOSION_H__

#include "GameBase.h"
#include "Projectile.h"
#include "DamageTypes.h"
#include "FXButeMgr.h"
#include "Timer.h"

class Explosion : public GameBase
{
	public :

 		Explosion();
        void Setup(HOBJECT hFiredFrom, uint8 nAmmoType);

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        virtual LTVector GetBoundingBoxColor();

        virtual void Start(HOBJECT hFiredFrom=LTNULL);

        LTFLOAT			m_fDamageRadius;
        LTFLOAT         m_fMaxDamage;
		DamageType		m_eDamageType;
		HOBJECT			m_hFiredFrom;
        LTVector        m_vPos;
        LTBOOL          m_bRemoveWhenDone;
        uint8           m_nImpactFXId;

        LTFLOAT         m_fProgDamage;
        LTFLOAT         m_fProgDamageLifetime;
        LTFLOAT         m_fProgDamageDuration;
        LTFLOAT         m_fProgDamageRadius;
		DamageType		m_eProgDamageType;

		CTimer			m_ProgDamageTimer;

	private :

		void Update();
		void ReadProp();
		void AreaDamageObjectsInSphere();
		void AreaDamageObject(HOBJECT hObj);
		void ProgDamageObjectsInSphere();
		void ProgDamageObject(HOBJECT hObj);

		void CacheFiles();

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
};

class CExplosionPlugin : public IObjectPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
            uint32* pcStrings,
            const uint32 cMaxStrings,
            const uint32 cMaxStringLength);

	private:

		CFXButeMgrPlugin	m_FXButeMgrPlugin;
};

#endif // __EXPLOSION_H__