// ----------------------------------------------------------------------- //
//
// MODULE  : Prop.h
//
// PURPOSE : Model Prop - Definition
//
// CREATED : 10/9/97
//
// ----------------------------------------------------------------------- //
//
// Object Use:
//
// The prop is used to place models to decorate a level.  The prop can be
// made to take damage and blow up.  The damage aggregate can send messages
// that it is being damaged or has been destroyed.  The prop can also spawn
// pickup items.
//
// Property					Type		Description
// ========					====		===========
// Name						String		To uniquely identify the object (optional)
// Pos						Vector		Position in the world
// Rotation					Rotation	Specifies direction object points
// DamageTriggerTarget		String		Specifies name of objects that get 
//										DamageTriggerMessage (optional)
// DamageTriggerMessage		String		Message to send DamageTriggerTarget when 
//										damage taken (optional)
// DeathTriggerTarget		String		Specifies name of objects that get 
//										DeathTriggerMessage (optional)
// DeathTriggerMessage		String		Message to send DeathTriggerTarget when 
//										destroyed (optional)
// Filename					String		Filename of model to use for prop
// Skin						String		Filename of the skin to place on model
// Masses					String		List of masses of prop (needed for destructable props)
// HitPoints				String		List of number of hit points to expire before death.  
//										If greater than zero, then prop is considered 
//										destructable (optional)
// ArmorPoints				String		List of number of armor points (optional)
// DestroyedModels			String		List of Filename of model to use for prop when all hit
//										points are gone (optional)
// DestroyedSkins			String		List of Filename of skin to put on DestroyedModel (optional)
// DestroyedSounds			String		List of Filename of sound to play when destroyed (optional)
// SoundRadius				Float		Max radius Sound can be heard (optional)
// NumDebris				String		List of number of debris props to create
// Hollow					Bool		Whether prop is a hollow form, which determines which
//										set of models are used for debris.
// Spawn					String		Class of item to spawn
//
// ----------------------------------------------------------------------- //


#ifndef __PROP_H__
#define __PROP_H__

#include "cpp_engineobjects_de.h"
#include "Destructable.h"
#include "Projectile.h"
#include "DebrisTypes.h"
#include "Activation.h"

class Prop : public BaseClass
{
	public :

 		Prop( );
		~Prop( );

		SurfaceType				GetSurfaceType( ) { return m_eSurfaceType; }
		DebrisType				GetDebrisType( )  { return m_eDebrisType; }

	protected :

		DDWORD					EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD					ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		virtual void			SubClassSetup() {}
		virtual void			Damage();
		virtual void			DoExplosion();

	private :

		void					ReadProp(ObjectCreateStruct *pStruct);
		void					PostPropRead(ObjectCreateStruct *pStruct);
		void					InitialUpdate(DVector *pMovement);
		DBOOL					Update( );
		void					CreateDebris(DVector *pvPos, DVector *pvDims);
		void					SpawnItem();
		void					InitProperties( );
		DBOOL					GetNextToken( HSTRING *hString, char *pszValue, int nLength );

		void					TriggerMsg(HOBJECT hSender, HSTRING hMsg);

		void					CacheFiles( );

	protected :

		CDestructable			m_Damage;
		CActivation				m_activation;				// Handle activation

		HSTRING					m_hstrDestroyedModels;
		HSTRING					m_hstrDestroyedSkins;
		DFLOAT					m_fMass;
		DFLOAT					m_fHitPoints;
		DFLOAT					m_fArmorPoints;
		HSTRING					m_hstrDestroyedSounds;
		DFLOAT					m_fSoundRadius;
		DBYTE					m_nMinNumDebris;
		DBYTE					m_nMaxNumDebris;
		HSTRING					m_hstrSpawn;
		DFLOAT					m_fLife;
		DBOOL					m_bFirstUpdate;
		DBOOL					m_bActive;
		DBOOL					m_bReadProp;
		DBOOL					m_bMoveToFloor;
		DBOOL					m_bDamageDone;
		DBOOL					m_bChrome;

		HSTRING					m_hstrMasses;
		HSTRING					m_hstrHitPoints;
		HSTRING					m_hstrArmorPoints;

		DVector					m_vScale;
		DVector					m_vModelAdd;
		SurfaceType				m_eSurfaceType;
		DebrisType				m_eDebrisType;

		DBOOL					m_bCreateExplosion;
		DBYTE					m_nExplosionWeaponId;
		ModelSize				m_eExplosionSize;
		DBOOL					m_bFireAlongForward;
		DFLOAT					m_fDamageFactor;

		DVector					m_vTintColor;
		DFLOAT					m_fTranslucency;

	private :

		void AdjustFlags(ObjectCreateStruct *pStruct);
			
		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

#endif // __PROP_H__
