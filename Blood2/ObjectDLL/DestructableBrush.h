#ifndef __DESTRUCTABLEBRUSH_H__
#define __DESTRUCTABLEBRUSH_H__


#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "Destructable.h"
#include "Debris.h"
#include "B2BaseClass.h"
#include "SharedDefs.h"


// CDestructableBrush class
class CDestructableBrush : public B2BaseClass
{
	public:

		CDestructableBrush();
		virtual		~CDestructableBrush();

		DBOOL		ReadProp(ObjectCreateStruct *pStruct);
		void		PostPropRead(ObjectCreateStruct *pStruct);
		SurfaceType	GetSurfaceType();
		DBOOL		IsMarkable() const { return m_bAllowMarks; }
		DBOOL		IsFireThrough() const { return m_bFireThrough; }
		DBOOL		IsDestructable() const { return m_bDestructable; }
	protected:

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, float lData);
		DDWORD		ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		void		CreateWorldModelDebris();

	private:

		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

	protected:

		CDestructable	m_damage;
		CDebris			m_Debris;
		DFLOAT			m_fInitHitPoints;
		DBOOL			m_bDestructable;
		DBOOL			m_bBoxPhysics;
		DBOOL			m_bPushable;
		DFLOAT			m_fMass;
		HSTRING			m_hstrDebrisSkin;
		SurfaceType		m_eSurfType;
		DBOOL			m_bAllowMarks;
		DBOOL			m_bFireThrough;
};


#endif // __DESTRUCTABLEBRUSH_H__