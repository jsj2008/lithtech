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

#include "DestructableBrush.h"
#include "cpp_engineobjects_de.h"
#include "Rotating.h"

class WorldModelDebris : public CDestructableBrush
{
	public :

		WorldModelDebris();
		virtual ~WorldModelDebris();

//		void	Start(DVector *pvRotationPeriods, DVector* pvVel);
		void	Start();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void	Update();
		void	ReadProp(ObjectCreateStruct *pStruct);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		Rotating m_Rotating;
		DBOOL	m_bOn;
		DVector	m_vInitialVelocity;
		DFLOAT	m_fGravityMultiplier;
};

#endif // __WORLD_MODEL_DEBRIS_H__
