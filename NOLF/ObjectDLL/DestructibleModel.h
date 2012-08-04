// ----------------------------------------------------------------------- //
//
// MODULE  : DestructibleModel.h
//
// PURPOSE : Destructible model aggregate class
//
// CREATED : 4/23/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DESTRUCTABLE_MODEL_H__
#define __DESTRUCTABLE_MODEL_H__

#include "Destructible.h"
#include "DebrisFuncs.h"
#include "SurfaceMgr.h"

#define DEFAULT_DWMA_MASS			2000.0f

// Use ADD_DESTRUCTIBLE_WORLD_MODEL_AGGREGATE() in your class definition
// to enable the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_DESTRUCTIBLE_WORLD_MODELAGGREGATE()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_DESTRUCTIBLE_MODEL_AGGREGATE(group, flags) \
	ADD_DESTRUCTIBLE_AGGREGATE((group), (flags)) \
	ADD_REALPROP_FLAG(Mass, DEFAULT_DWMA_MASS, (group) | (flags)) \
	PROP_DEFINEGROUP(ExplosionProperties, ((group)<<1) | (flags)) \
		ADD_BOOLPROP_FLAG(CreateExplosion, 0, ((group)<<1) | (flags)) \
		ADD_LONGINTPROP_FLAG(WeaponId, 0, ((group)<<1) | (flags)) \
		ADD_BOOLPROP_FLAG(FireAlongForward, 0, ((group)<<1) | (flags)) \
		ADD_REALPROP_FLAG(DamageFactor, 1.0f, ((group)<<1) | (flags)) \
		ADD_STRINGPROP_FLAG(Spawn, "", ((group)<<1) | (flags)) \
	ADD_DEBRISTYPE_PROPERTY((flags)) \
	ADD_STRINGPROP_FLAG(SurfaceOverride, "", (flags) | PF_STATICLIST)

class CDestructibleModelPlugin : public IObjectPlugin
{

  public:

    virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

  private:
	CDebrisPlugin		m_DebrisPlugin;
	CSurfaceMgrPlugin	m_SurfaceMgrPlugin;
};

class CDestructibleModel : public CDestructible
{
	public :

		CDestructibleModel();
		virtual ~CDestructibleModel();

        void        DoExplosion(char* pTargetName=LTNULL);

        uint8       m_nDebrisId;

        LTBOOL      m_bCreatedDebris;

        LTBOOL      m_bCreateExplosion;
        uint8       m_nExplosionWeaponId;
        LTBOOL      m_bFireAlongForward;
        LTFLOAT     m_fDamageFactor;

        LTBOOL      m_bRemoveOnDeath;

		HSTRING		m_hstrSpawn;
		HSTRING		m_hstrSurfaceOverride;

		uint32		m_dwOriginalFlags;
		LTBOOL		m_bSaveNeverDestroy;
		LTBOOL		m_bSaveCanDamage;

	protected:

        uint32  EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData);
        uint32  ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

        LTBOOL   ReadProp(ObjectCreateStruct *);
		void	CreateWorldModelDebris();
		void	CreateDebris();
		void	SetSurfaceType();

	private :

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void CacheFiles();
		void SpawnItem();
};

#endif // __DESTRUCTABLE_MODEL_H__