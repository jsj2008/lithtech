// ----------------------------------------------------------------------- //
//
// MODULE  : Debris.h
//
// PURPOSE : Debris definition
//
// CREATED : 5/12/98
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRIS_H__
#define __DEBRIS_H__

#include "cpp_aggregate_de.h"
#include "cpp_engineobjects_de.h"
#include "SharedDefs.h"

#define ADD_DEBRIS_AGGREGATE() \
	PROP_DEFINEGROUP(DebrisStuff, PF_GROUP2) \
		ADD_LONGINTPROP_FLAG(Amount, 10, PF_GROUP2) \
		ADD_REALPROP_FLAG(Scale, 1.0f, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Stone, DFALSE, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Metal, DFALSE, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Wood, DFALSE, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Energy, DFALSE, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Glass, DFALSE, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Terrain, DFALSE, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Plastic, DFALSE, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Flesh, DFALSE, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Liquid, DFALSE, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Exploding, DFALSE, PF_GROUP2) \
		ADD_BOOLPROP_FLAG(Custom, DFALSE, PF_GROUP2) \
		ADD_STRINGPROP_FLAG(CustomTexture, "", PF_GROUP2) \
		ADD_STRINGPROP_FLAG(CustomSound, "", PF_GROUP2) \
		ADD_STRINGPROP_FLAG(CustomModel, "", PF_GROUP2) \
		ADD_REALPROP_FLAG(ExplodeDamage, 100, PF_GROUP2) \
		ADD_REALPROP_FLAG(DamageRadius, 200, PF_GROUP2) \

class CDebris : public Aggregate
{
	public :

		CDebris();
		virtual ~CDebris();
		DBOOL	Init(HOBJECT hObject);
		void	Create(DVector vDir, DFLOAT fDamage);

		SurfaceType	GetType() const	{ return m_eType; }
		DBOOL	IsExploding() const { return m_bExploding; }

	protected :

		DDWORD EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData);
	
		HOBJECT	m_hObject; 

		HSTRING m_hstrSound;
		HSTRING	m_hstrTexture1;
		HSTRING	m_hstrTexture2;
		HSTRING	m_hstrModel1;
		HSTRING	m_hstrModel2;

		DBYTE	m_nAmount;
		DFLOAT	m_fScale;
		DBOOL	m_bStone;
		DBOOL	m_bMetal;
		DBOOL	m_bWood;
		DBOOL	m_bEnergy;
		DBOOL	m_bGlass;
		DBOOL	m_bTerrain;
		DBOOL	m_bPlastic;
		DBOOL	m_bFlesh;
		DBOOL	m_bLiquid;
		DBOOL	m_bExploding;
		DBOOL	m_bCustom;
		SurfaceType	m_eType;

		DFLOAT	m_fDamageRadius;
		DFLOAT	m_fExplodeDamage;

	private :

		void	InitialUpdate();
		DBOOL	ReadProp(ObjectCreateStruct *pInfo);
		void	AddExplosion(DVector &vPos);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);
};

#endif // __DEBRIS_H__
