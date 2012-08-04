// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingDoor.h
//
// PURPOSE : A RotatingDoor object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#ifndef __ROTATING_DOOR_H__
#define __ROTATING_DOOR_H__

#include "door.h"

class RotatingDoor : public Door
{
	public:

		RotatingDoor();

		DBOOL	CanPushPlayerBack()	const { return m_bPushPlayerBack; }

	protected:

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float fData);

		virtual void Opening();
		virtual void Closing();

	private:

		DBOOL	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		void	DoRotation();
		DBOOL	CalcAngle(DFLOAT & fAngle, DFLOAT fInitial, DFLOAT fTarget, int nDir, DFLOAT fAmount);

		DVector m_vRotationAngles;		// Direction to open
		DVector m_vRotationPoint;		// Point to rotate around
		DVector m_vOpenAngles;			// Angles when object is open
		DVector m_vClosedAngles;		// Angles when object is closed
		DVector	m_vOriginalPos;			// Door's original position

		short	m_nOpenDirX;			// Direction of rotation in X
		short	m_nOpenDirY;			// Direction of rotation in Y
		short	m_nOpenDirZ;			// Direction of rotation in Z

		DFLOAT	m_fPitch;				// Pitch of door	
		DFLOAT	m_fYaw;					// Yaw of door
		DFLOAT	m_fRoll;				// Roll of door

		DBOOL	m_bPushPlayerBack;		// Push the player backwards
};


#endif // __ROTATING_DOOR_H__