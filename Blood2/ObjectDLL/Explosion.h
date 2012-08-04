// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.h
//
// PURPOSE : Base explosion object definition
//
// CREATED : 2/20/97
//
// ----------------------------------------------------------------------- //
//
// Object Use:
//
// The Explosion object is used to place a Sprite explosion in a level.  Objects 
// that are within the damage radius are damaged based on their position relative 
// to the position of the Explosion, and the value of the max damage data member.
// (see DamageObjectsInRadius())
//
// Property					Type		Description
// ========					====		===========
//
// SpriteFile				String		Filename of sprite to use for the explosion
// Sound					String		Filename of the explosion sound (optional)
// SoundRadius				Float		Max radius Sound can be heard (optional)
// DamageRadius				Float		Max distance objects can be and still be damaged (optional)
// MaxDamage				Float		Max damage that can be inflicted on an object (at epicenter)
// Depth					Integer		Number of Sprites to create (thickness of explosion)
// MinScale					Float		Minimum Scale size of Sprites
// MaxScale					Float		Maximum Scale size of Sprites
// Duration					Float		Time Explosion lasts
// CreateSmoke				Bool		Signals whether smoke is created
// CreateShockwave			Bool		Signals whether a shockwave is created (optional)
// SpriteFile				String		Filename of sprite to use for the shockwave (optional)
// ShockwaveScaleMin		Vector		Minimum shockwave size (optional)
// ShockwaveScaleMax		Vector		Maximum shockwave size (optional)
// ShockwaveDuration		Float		Time shockwave lasts (optional)
//
// ----------------------------------------------------------------------- //


#ifndef __EXPLOSION_H__
#define __EXPLOSION_H__

#include "cpp_engineobjects_de.h"
#include "Projectile.h"
#include "B2BaseClass.h"

class CImpact;

class Explosion : public B2BaseClass
{
	public :

 		Explosion();
		~Explosion();

		void Setup(char* pSound, DFLOAT fSoundRadius, DFLOAT fDuration,
				   char* pSkinName, DFLOAT fDamageRadius, DFLOAT fMaxDamage, 
				   DFLOAT fMinScale, DFLOAT fMaxScale, DBOOL bCreateMark=DFALSE, 
				   DBOOL bAddSparks=DFALSE, DBOOL bCreateSmoke=DFALSE);

		void SetupShockwave(char *pSprite, DVector vScaleMin, DVector vScaleMax,
							DFLOAT fDuration);

		void SetupLight(DBOOL bCreateLight, DVector vLightColor, DFLOAT fMinRadius,
						DFLOAT fMaxRadius);

		void Explode(DFLOAT fDelay = 0.0f);

	protected :

		DDWORD			EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD			ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		void			HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);

	private :

		void			Update();
		void			ReadProp(ObjectCreateStruct *pStruct);
		void			CreateExplosion( DVector *pPos );
		void			CreateLight( DVector *pvPos );
		CImpact*		CreateImpact(ObjectCreateStruct & theStruct, HSTRING hstrFile);

		void			AddShockwave(DVector *pvPoint);
		void			CreateMark(DVector *pvPoint);
		void			CreateSmoke(DVector *pvPoint);
		void			CreateFX(DVector *pvPoint);

		HSTRING			m_hstrModelName;
		HSTRING			m_hstrSkinName;
		HSTRING			m_hstrSound;
		DFLOAT			m_fSoundRadius;
		DFLOAT			m_fDamageRadius;
		DFLOAT			m_fMaxDamage;
		DFLOAT			m_fMinScale;
		DFLOAT			m_fMaxScale;
		DFLOAT			m_fDuration;
		DBOOL			m_bCreateSmoke;
		DFLOAT			m_fLastDamageTime;
		DVector			m_vRotation;

		DBOOL			m_bCreateShockwave;
		DBOOL			m_bFirstUpdate;
		HSTRING			m_hstrShockwaveSprite;
		DVector			m_vShockwaveScaleMin;
		DVector			m_vShockwaveScaleMax;
		DFLOAT			m_fShockwaveDuration;

		DBOOL			m_bCreateMark;
		DBOOL			m_bAddSparks;

		DFLOAT			m_fStartTime;
		DFLOAT			m_fDelay;
		DBOOL			m_bCreateLight;
		DVector			m_vLightColor;
		DFLOAT			m_fMinLightRadius;
		DFLOAT			m_fMaxLightRadius;
		HOBJECT			m_hLight;
		HOBJECT			m_hModel;
		HOBJECT			m_hShockwave;
};

#endif // __EXPLOSION_H__
