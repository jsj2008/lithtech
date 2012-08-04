// ----------------------------------------------------------------------- //
//
// MODULE  : DefenseGlobeLegs.h
//
// PURPOSE : DefenseGlobeLegs header
//
// CREATED : 12/30/97
//
// ----------------------------------------------------------------------- //

#ifndef __DEFENSEGLOBE_LEGS_H__
#define __DEFENSEGLOBE_LEGS_H__

#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "B2BaseClass.h"

class DefenseGlobeLegs : public B2BaseClass
{
	public :

		DefenseGlobeLegs();
		~DefenseGlobeLegs();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DBOOL m_bActive;   

	private :

		void ObjectTouch(HOBJECT hObj);

		DBOOL InitialUpdate(DVector *pMovement);
		DBOOL Update(DVector *pMovement);
		DBOOL ReadProp(ObjectCreateStruct *pStruct);
		void  PostPropRead(ObjectCreateStruct *pStruct);

};

#endif // __DEFENSEGLOBE_LEGS_H__
