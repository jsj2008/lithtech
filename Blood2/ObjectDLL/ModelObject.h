// ----------------------------------------------------------------------- //
//
// MODULE  : ModelObject.h
//
// PURPOSE : ModelObject class - implementation
//
// CREATED : 1/9/98
//
// ----------------------------------------------------------------------- //

#ifndef __MODELOBJECT_H__
#define __MODELOBJECT_H__

#include "cpp_engineobjects_de.h"

class CModelObject : public BaseClass
{
	public :

		CModelObject();
		virtual ~CModelObject();

		void				Setup( DFLOAT fLifeTime, DVector *pvRotationPeriods, DBOOL bStopRotateOnGround, DBOOL bRandomizeRotation);
		void				GetBaseDims(DVector *vDims) { VEC_COPY(*vDims, m_vBaseDims); }

	protected :

		virtual DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

		virtual void		FirstUpdate();
		virtual void		UpdateRotation();

	private :

		DVector				m_vBaseDims;

		DBOOL				InitialUpdate(DVector *pMovement);
		DBOOL				Update(DVector *pMovement);

		DFLOAT				m_fStartTime;
		DFLOAT				m_fLifeTime;
		DBOOL				m_bFirstUpdate;
		DBOOL				m_bRotate;
		DBOOL				m_bStopRotateOnGround;
		DFLOAT				m_fXRotVel;
		DFLOAT				m_fYRotVel;
		DFLOAT				m_fZRotVel;
		DFLOAT				m_fPitch;
		DFLOAT				m_fYaw;
		DFLOAT				m_fRoll;
		DFLOAT				m_fLastTime;
};

#endif // __MODELOBJECT_H__
