//----------------------------------------------------------
//
// MODULE  : DestructableWorldModel.h
//
// PURPOSE : Destructable world model aggregate class
//
// CREATED : 4/23/98
//
//----------------------------------------------------------

#ifndef __DESTRUCTABLE_WORLD_MODEL_H__
#define __DESTRUCTABLE_WORLD_MODEL_H__

#include "Destructable.h"
#include "DebrisTypes.h"

#define INFINITE_MASS			100000.0f


// Use ADD_DESTRUCTABLE_WORLD_MODEL_AGGREGATE() in your class definition 
// to enable the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DESTRUCTABLE_WORLD_MODELAGGREGATE()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_DESTRUCTABLE_WORLD_MODEL_AGGREGATE() \
	ADD_DESTRUCTABLE_AGGREGATE() \
	ADD_REALPROP(Mass, INFINITE_MASS) \
	ADD_REALPROP(HitPoints, 100.0f) \
	ADD_REALPROP(Armor, 100.0f) \
	ADD_STRINGPROP(DestroySound, "") \
	ADD_REALPROP(SoundRadius, 1500.0f) \
	ADD_LONGINTPROP(DebrisType, DBT_METAL_BIG) \
	ADD_REALPROP(MinNumDebris, 10) \
	ADD_REALPROP(MaxNumDebris, 20) \
	ADD_STRINGPROP(Spawn, "") \
	PROP_DEFINEGROUP(ExplosionStuff, PF_GROUP1) \
		ADD_BOOLPROP_FLAG(CreateExplosion, 0, PF_GROUP1) \
		ADD_LONGINTPROP_FLAG(WeaponId, GUN_BULLGUT_ID, PF_GROUP1) \
		ADD_LONGINTPROP_FLAG(ExplosionSize, MS_NORMAL, PF_GROUP1) \
		ADD_BOOLPROP_FLAG(FireAlongForward, 0, PF_GROUP1) \
		ADD_REALPROP_FLAG(DamageFactor, 1.0f, PF_GROUP1)


class CDestructableWorldModel : public CDestructable
{
	public :

		CDestructableWorldModel();
		virtual ~CDestructableWorldModel();

	protected:

		DDWORD EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		DBOOL ReadProp(ObjectCreateStruct *);
		void  CreateWorldModelDebris();
		void  CreateDebris();
		void  DoExplosion();

		DFLOAT	m_fMass;
		DFLOAT	m_fHitPts;
		DFLOAT	m_fArmor;

		HSTRING	m_hstrDestroySound;
		DFLOAT	m_fSoundRadius;

		DBOOL		m_bCreatedDebris;
		DebrisType	m_eDebrisType;

		DBYTE		m_nMinNumDebris;
		DBYTE		m_nMaxNumDebris;

		DBOOL		m_bCreateExplosion;
		DBYTE		m_nExplosionWeaponId;
		ModelSize	m_eExplosionSize;
		DBOOL		m_bFireAlongForward;
		DFLOAT		m_fDamageFactor;

		HSTRING		m_hstrSpawn;

		SurfaceType	m_eSurfaceType;

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();
		void SpawnItem();
};

#endif // __DESTRUCTABLE_WORLD_MODEL_H__