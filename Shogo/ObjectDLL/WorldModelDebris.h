// ----------------------------------------------------------------------- //
//
// MODULE  : WorldModelDebris.h
//
// PURPOSE : WorldModelDebris class - implementation
//
// CREATED : 2/27/98
//
// ----------------------------------------------------------------------- //

#ifndef __WORLD_MODEL_DEBRIS_H__
#define __WORLD_MODEL_DEBRIS_H__

#include "DestructableDoor.h"
#include "cpp_engineobjects_de.h"

class WorldModelDebris : public DestructableDoor
{
	public :

		WorldModelDebris();

		void	Start(DVector *pvRotationPeriods, DVector* pvVel);

	protected :

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

		virtual void	UpdateRotation();

	private :

		void  Update();
		void  ReadProp(ObjectCreateStruct *pStruct);

		DBOOL	m_bRotate;
		DFLOAT	m_fXRotVel;
		DFLOAT	m_fYRotVel;
		DFLOAT	m_fZRotVel;
		DFLOAT	m_fPitch;
		DFLOAT	m_fYaw;
		DFLOAT	m_fRoll;
		DFLOAT	m_fLastTime;

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

#endif // __WORLD_MODEL_DEBRIS_H__
