// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypes.h
//
// PURPOSE : Definition of damage types
//
// CREATED : 11/26/97
//
// ----------------------------------------------------------------------- //

#ifndef __DAMAGE_TYPES_H__
#define __DAMAGE_TYPES_H__

#include "engineobjects_de.h"
#include "generic_msg_de.h"
#include "cpp_server_de.h"
#include "cpp_engineobjects_de.h"

enum DamageType 
{ 
	DT_UNSPECIFIED=0,	// Unknown type (self-damage -instantaneous)
	DT_MELEE,			// (caused by melee weapon)
	DT_PUNCTURE,		// (bullets, instantaneous)
	DT_BURN,			// (fire, corrosives-continuous damage)
	DT_IMPACT,			// (falling, crushing, collision-instantaneous)
	DT_ELECTROCUTE,		// (electricity-continuous)
	DT_CHOKE,			// (water, hostile atmosphere-continuous)
	DT_ENERGY,			// (pulse rifle, grenade launcher)
	DT_EXPLODE,			// (explosions-instantaneous AND continuous: 
						//	i.e. initial blast plus progressive damage)
	DT_FREEZE,			// (liquid nitrogen, freezing air or fluid-continuous)
	DT_SQUEAKY,			// Squeaky toy damage
	DT_KATO,			// Red-riot (vaporize death)
	DT_BURST,			// Sniper Rifle (expand death)
	DT_ENDLESS_FALL		// Falling and can never get up
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RadiusDamageType()
//
//	PURPOSE:	Is this damage type a radius damage type
//
// ----------------------------------------------------------------------- //

inline DBOOL RadiusDamageType(DamageType eType)
{
	DBOOL bRet = DFALSE;

	switch (eType)
	{
		case DT_EXPLODE:
		case DT_ENERGY:
		case DT_KATO:
			bRet = DTRUE;
		break;

		default : 
		break;
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecificObjectFilterFn()
//
//	PURPOSE:	Filter a specific object out of CastRay and/or 
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

inline DBOOL SpecificObjectFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj) return DFALSE;

	return (hObj != (HOBJECT)pUserData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageObject()
//
//	PURPOSE:	Damage the specified object
//
//  PARAMETERS:
//
//		hResponsible -	Handle to the object responsible for the damage 
//						(may or may not be the same as pDamager->m_hObject)
//		pDamager	 -	Pointer to the object that is doing the damage.
//		hDamagee	 -	Handle to the object taking the damage
//		fDamage		 -	The amount of damage being inflicted
//		vDir		 -	The direction the damage is coming from
//		eType		 -	The type of damage being inflicted.
//
// ----------------------------------------------------------------------- //

inline void DamageObject(HOBJECT hResponsible, LPBASECLASS pDamager, 
						 HOBJECT hDamagee, DFLOAT fDamage, DVector vDir, 
						 DamageType eType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hResponsible || !pDamager || !hDamagee) return;

	// Damage object...

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pDamager, hDamagee, MID_DAMAGE);
	pServerDE->WriteToMessageVector(hMessage, &vDir);
	pServerDE->WriteToMessageFloat(hMessage, fDamage);
	pServerDE->WriteToMessageByte(hMessage, eType);
	pServerDE->WriteToMessageObject(hMessage, hResponsible);
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageObjectsInRadius()
//
//	PURPOSE:	Damage objects within a certain radius
//
//  PARAMETERS:
//
//		hResponsible -	Handle to the object responsible for the damage 
//						(may or may not be the same as pDamager->m_hObject)
//		pDamager	 -	Pointer to the object that is doing the damage.
//		vOrigin		 -	Center of the damage sphere
//		fRadius 	 -	The radius of the damage sphere
//		fDamage		 -	The amount of damage inflicted.
//		eType		 -	The type of damage being inflicted.
//
// ----------------------------------------------------------------------- //

inline void DamageObjectsInRadius(HOBJECT hResponsible, LPBASECLASS pDamager,
								  DVector vOrigin, DFLOAT fRadius,
								  DFLOAT fDamage, DamageType eType=DT_EXPLODE)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || fRadius <= 0.0f) return;

	ObjectList* pList = pServerDE->FindObjectsTouchingSphere(&vOrigin, fRadius);
	if (!pList) return;

	ObjectLink* pCurObjLink = pList->m_pFirstLink;

	// TESTING
	// pServerDE->BPrint("DOIR: Radius = %.2f, Damage = %.2f", fRadius, fDamage);
	// TESTING

	while (pCurObjLink) 
	{
		HOBJECT hObj = pCurObjLink->m_hObject;

		DVector vDir, vObjPos;
		pServerDE->GetObjectPos(hObj, &vObjPos);
		
		VEC_SUB(vDir, vObjPos, vOrigin);
		DFLOAT fDistance = (DFLOAT)fabs(VEC_MAG(vDir));

		if (fDistance < 1.0f) 
		{
			fDistance = 1.0f;
		}

		if (fDistance < fRadius) 
		{
			// Make sure the hObj is not blocked by another object...
					
			IntersectInfo iInfo;
			IntersectQuery qInfo;

			VEC_COPY(qInfo.m_From, vOrigin);
			VEC_COPY(qInfo.m_To, vObjPos);

			qInfo.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
			qInfo.m_FilterFn  = SpecificObjectFilterFn;
			qInfo.m_pUserData = hObj;

			if (!pServerDE->IntersectSegment(&qInfo, &iInfo))
			{
				DamageObject(hResponsible, pDamager, hObj, fDamage, vDir, eType);
			}
		}
	
		pCurObjLink = pCurObjLink->m_pNext;
	}

	pServerDE->RelinquishList(pList);
}

#endif // __DAMAGE_TYPES_H__