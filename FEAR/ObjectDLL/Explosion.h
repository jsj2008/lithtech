// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.h
//
// PURPOSE : Explosion - Definition
//
// CREATED : 11/25/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __EXPLOSION_H__
#define __EXPLOSION_H__

#include "GameBase.h"
#include "DamageTypes.h"
#include "EngineTimer.h"
#include "EventCaster.h"

class GameClientData;

LINKTO_MODULE( Explosion );

class Explosion : public GameBase
{
	public :

 		Explosion();
		void	Setup( HOBJECT hFiredFrom, HAMMO hAmmo, bool bUseAIAmmo );

		HOBJECT GetFiredFrom() const { return m_hFiredFrom; }

	protected :

		uint32	EngineMessageFn(uint32 messageID, void *pData, float lData);

		virtual void Start(HOBJECT hFiredFrom=NULL);

		void			SetFiredFrom( HOBJECT hFiredFrom );

		// Declare our delegate to receive removeclient events.
		static void OnRemoveClient( Explosion* pExplosion, GameClientData* pGameClientData, EventCaster::NotifyParams& notifyParams );
		Delegate< Explosion, GameClientData, Explosion::OnRemoveClient > m_delegateRemoveClient;

		// Declare our delegate to receive playerswitched events.
		static void OnPlayerSwitched( Explosion* pExplosion, GameClientData* pGameClientData, EventCaster::NotifyParams& notifyParams );
		Delegate< Explosion, GameClientData, Explosion::OnPlayerSwitched > m_delegatePlayerSwitched;

		float			m_fDamageRadius;
		float			m_fDamageRadiusMin;
		float			m_fImpulse;

		float			m_fMaxDamage;
		float			m_fPenetration;
		DamageType		m_eDamageType;

		LTObjRef		m_hFiredFrom;

		LTVector		m_vPos;
		bool			m_bRemoveWhenDone;
		uint32			m_nImpactFXId;

		float			m_fProgDamage;
		float			m_fProgDamageLifetime;
		float			m_fProgDamageDuration;
		float			m_fProgDamageRadius;
		DamageType		m_eProgDamageType;

		StopWatchTimer	m_ProgDamageTimer;

		HAMMO			m_hAmmo;

	private :

		void	Update();
		void	ReadProp(const GenericPropList *pProps);
		void	AreaDamageObjectsInSphere();
		void	AreaDamageObject(HOBJECT hObj);
		void	ProgDamageObjectsInSphere();
		void	ProgDamageObject(HOBJECT hObj);
		bool	CharacterEarlyOut(HOBJECT hObj);

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);


		// Message Handlers...

		DECLARE_MSG_HANDLER( Explosion, HandleOnMsg );
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

};

#endif // __EXPLOSION_H__
