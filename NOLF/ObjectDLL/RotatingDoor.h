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

		LTBOOL IsStuck() const { return m_bStuck; }

	protected:

        uint32  EngineMessageFn(uint32 messageID, void *pData, float fData);

        virtual void	SetOpen(LTBOOL bInitialize=LTFALSE);
        virtual void	SetClosed(LTBOOL bInitialize=LTFALSE);
		virtual void	SetOpening();
		virtual void	Opening();
		virtual void	Closing();

        virtual LTBOOL   CalcPosAndRot(LTVector & vPos, LTRotation & rRot,
            LTBOOL bTestCollisions=LTFALSE);

        virtual LTBOOL   CalcAngle(LTFLOAT & fAngle, LTFLOAT fInitial,
            LTFLOAT fTarget, LTFLOAT fDir, LTFLOAT fAmount);

        virtual LTBOOL   GetMoveTestPosRot(LTVector & vTestPos,
            LTRotation & rTestRot);

        virtual LTBOOL   TestObjectCollision(HOBJECT hTest, LTVector vTestPos,
            LTRotation rTestRot, HOBJECT* pCollisionObj=LTNULL);

		virtual void	SetLightAnimOpen();
		virtual void	SetLightAnimClosed();

        virtual LTBOOL   TreatLikeWorld() { return LTFALSE; }

		// Returns the position it should put the light animation at.
		virtual float	GetRotatingLightAnimPercent();

		// This is here so we can treat pitch, yaw, and roll like a vector..
        float           GetCurAnglesDim(uint32 iDim);

        LTVector	m_vRotationAngles;  // Direction to open
        LTVector	m_vRotPtOffset;     // Offset from door's postion to rotation point
        LTVector	m_vRotationPoint;   // Point to rotate around
        LTVector	m_vOpenAngles;      // Angles when object is open
        LTVector	m_vClosedAngles;    // Angles when object is closed
        LTVector	m_vOriginalPos;     // Door's original position
        LTVector	m_vOpenDir;         // Direction door opens
					
        LTFLOAT		m_fPitch;           // Pitch of door
        LTFLOAT		m_fYaw;             // Yaw of door
        LTFLOAT		m_fRoll;            // Roll of door

		LTBOOL		m_bStuck;			// Are we stuck?

	private :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
        void    Save(HMESSAGEWRITE hWrite, uint8 nType);
        void    Load(HMESSAGEREAD hRead, uint8 nType);
};


// Used by the preprocessor functions to determine where the door will be rotating.
void RotatingDoor_CalcPosAndRot(
    ILTMath *pMathLT,
    LTVector vOriginalPos,   // Original object position.
    LTVector vRotationPoint, // Point to rotate around.
    LTVector vAngles,        // Rotation angles.
    LTVector &vOutPos,
    LTRotation &rOutRot);

// Used by preprocessor lighting stuff.
void SetupTransform_RotatingDoor(ILTPreLight *pInterface,
	HPREOBJECT hObject,
	float fPercent,
    LTVector &vOutPos,
    LTRotation &rOutRotation);



#endif // __ROTATING_DOOR_H__