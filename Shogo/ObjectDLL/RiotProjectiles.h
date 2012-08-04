// ----------------------------------------------------------------------- //
//
// MODULE  : RiotProjectiles.h
//
// PURPOSE : RiotProjectiles class - definition
//
// CREATED : 10/3/97
//
// ----------------------------------------------------------------------- //

#ifndef __RIOT_PROJECTILES_H__
#define __RIOT_PROJECTILES_H__

#include "Projectile.h"
#include "Bouncer.h"

class CRedRiotProjectile : public CProjectile
{
	public :
		CRedRiotProjectile();
};

class CPulseRifleProjectile : public CProjectile
{
	public :
		CPulseRifleProjectile();
	protected :
		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
};

class CJuggernautProjectile : public CProjectile
{
	public :
		CJuggernautProjectile();
};

class CBullgutProjectile : public CProjectile
{
	public :
		CBullgutProjectile();
};

class CTOWProjectile : public CProjectile
{
	public :
		CTOWProjectile();
};

class CGrenadeProjectile : public CProjectile
{
	public :
		CGrenadeProjectile();
};

class CKatoGrenadeProjectile : public CGrenadeProjectile
{
	public :
		CKatoGrenadeProjectile();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		virtual void HandleImpact(HOBJECT hObj);

	private :

		void DoBounce();
};

class CStickyGrenadeProjectile : public CProjectile
{
	public :
		CStickyGrenadeProjectile();

	protected :
		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		virtual void HandleImpact(HOBJECT hObj);
		virtual void RemoveObject();

	private :

		void Update();
		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		char*		m_pThudSound;			// Thud sound filename

		DBOOL		m_bAttached;			// Are we attached to something?
		HOBJECT		m_hHostObj;				// Object we're attached to
		DVector		m_vPosOffset;			// Relative offset from host object

		DDWORD		m_dwFireAni;			// Fire animation
		DDWORD		m_dwFlightAni;			// Flight animation
		DDWORD		m_dwImpactAni;			// Impact animation
};


#endif //  __RIOT_PROJECTILES_H__