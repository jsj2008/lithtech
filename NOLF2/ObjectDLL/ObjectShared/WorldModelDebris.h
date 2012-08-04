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

#include "Door.h"
#include "ltengineobjects.h"

LINKTO_MODULE( WorldModelDebris );

class WorldModelDebris : public Door
{
	public :

		WorldModelDebris();

        void    Start(LTVector *pvRotationPeriods, LTVector* pvVel);

	protected :

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);

		virtual void	UpdateRotation();

	private :

		void  Update();
		void  ReadProp(ObjectCreateStruct *pStruct);

        LTBOOL   m_bRotate;
        LTFLOAT  m_fXRotVel;
        LTFLOAT  m_fYRotVel;
        LTFLOAT  m_fZRotVel;
        LTFLOAT  m_fPitch;
        LTFLOAT  m_fYaw;
        LTFLOAT  m_fRoll;
        LTFLOAT  m_fLastTime;

	private :

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
};

#endif // __WORLD_MODEL_DEBRIS_H__