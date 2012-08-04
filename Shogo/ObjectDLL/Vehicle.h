// ----------------------------------------------------------------------- //
//
// MODULE  : Vehicle.h
//
// PURPOSE : Vehicle base class - Definition
//
// CREATED : 5/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __VEHICLE_H__
#define __VEHICLE_H__

#include "cpp_engineobjects_de.h"
#include "BaseAI.h"

class Vehicle : public BaseAI
{
	public :

 		Vehicle();
 		~Vehicle();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

		virtual void HandleWeaponChange();
		virtual void TargetPos(DVector vTargetPos);
		virtual DVector	HandHeldWeaponFirePos();
		virtual DVector GetTargetDir(DVector & vFirePos, DVector & vTargetPos);

		virtual void SetWeaponDamageFactor(CWeapon* pWeapon) { }
		virtual char* GetTurretFireNodeName() { return DNULL; }

		virtual void HandleDead(DBOOL bRemoveObj);
		virtual void SetRemoveCmd();
	
		DBOOL		m_bHeadLights;					// Create/attach head lights
		DBOOL		m_bExhaust;						// Create/attach exhaust

		DRotation		m_rTurretRot;
		DRotation		m_rTurretOffset;

		HOBJECT			m_hExhaust;					// Exhaust object...
		HOBJECT			m_hLHeadLight;				// Left head light object...
		HOBJECT			m_hRHeadLight;				// Right head light object...

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		char*		m_pLLightName;					// Left light node name
		char*		m_pRLightName;					// Right light node name
		char*		m_pExhaustName;					// Exhaust node name

		char*		m_pIdleSound;
		char*		m_pRunSound;


	private :

		HSOUNDDE	m_hIdleSound;
		HSOUNDDE	m_hRunSound;

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();

		DBOOL   ReadProp(ObjectCreateStruct *pInfo);
		void	InitialUpdate();
		void	CreateExhaust();
		void	CreateHeadLights();
		void	RemoveAttachments();
};

#endif // __VEHICLE_H__
