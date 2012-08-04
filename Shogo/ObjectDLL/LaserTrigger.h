// ----------------------------------------------------------------------- //
//
// MODULE  : LaserTrigger.h
//
// PURPOSE : LaserTrigger - Definition
//
// CREATED : 4/30/98
//
// ----------------------------------------------------------------------- //

#ifndef __LASER_TRIGGER_H__
#define __LASER_TRIGGER_H__

#include "Trigger.h"

class LaserTrigger : public Trigger
{
	public :

		LaserTrigger();
		~LaserTrigger();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		virtual void  HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead);

	private :

		DBOOL ReadProp(ObjectCreateStruct *pData);
		void InitialUpdate(int nInfo);

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		DVector		m_vColor;			// Model color adjust
		DFLOAT		m_fAlpha;			// Model alpha adjust
		HOBJECT		m_hModel;			// Model created
};

#endif // __LASER_TRIGGER_H__