// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerCamera.h
//
// PURPOSE : PlayerCamera - Implementation
//
// CREATED : 10/5/97
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYERCAMERA_H__
#define __PLAYERCAMERA_H__

#include "cpp_client_de.h"


class CPlayerCamera
{
	public:

//		enum CameraState	{ SOUTH, SOUTHEAST, EAST, NORTHEAST, NORTH, 
//							  NORTHWEST, WEST, SOUTHWEST, MLOOK, HOLD };

		enum CameraMode		{ FIRSTPERSON, CHASE, GOINGFIRSTPERSON, GOINGCHASE, 
							  CIRCLING, SCRIPT, DEATH };

		enum CameraPoint	{ AT_POSITION, IN_DIRECTION, AUTOMATIC };

		// Constructor
		CPlayerCamera();

		DBOOL Init(CClientDE* pClientDE)
		{
			if (!pClientDE) return DFALSE;
			m_pClientDE = pClientDE;
			return DTRUE;
		}

		// Update the position & orientation of the camera based on the target
		void CameraUpdate(DFLOAT deltaTime);

		// Set the state/orientation of the camera
//		void SetCameraState(CameraState eOrientation);
//		CameraState GetCameraState() const { return m_eOrientation; }

		void StartCircle();

		// Attach the camera to an object
		void AttachToObject(HLOCALOBJ hObj);


		// Get the object that the camera is attached to
		HLOCALOBJ GetAttachedObject()
		{
			return m_hTarget;
		}

		// Save the camera's current orientation
//		void SaveState()
//		{
//			m_eSaveOrientation = m_eOrientation;
//		}
		
		// Restore the camera's last saved orientation
//		void RestoreState()
//		{
//			SetCameraState(m_eSaveOrientation);
//		}

		void GoChaseMode()
		{
//			m_eCameraMode = CHASE;
			if((m_eCameraMode != GOINGCHASE) && (m_eCameraMode != CHASE))
			{
				m_fTransitionStart = m_pClientDE->GetTime();
				m_eCameraMode = GOINGCHASE;
			}
		}

		void GoFirstPerson()
		{
			if((m_eCameraMode != GOINGFIRSTPERSON) && (m_eCameraMode != FIRSTPERSON))
			{
				m_fTransitionStart = m_pClientDE->GetTime();
				m_eCameraMode = GOINGFIRSTPERSON;
			}
		}

		void SetFirstPerson()
		{
 			m_eCameraMode = FIRSTPERSON;
		}

		void GoDeathMode()
		{
			if (m_eCameraMode != DEATH)
			{
				StartCircle();
				SaveCameraMode();
//				StartCircle(0.0f, 75.0f, 0.0f, 3.0f);
				m_fTransitionStart = m_pClientDE->GetTime();
				m_eCameraMode = DEATH;
			}
		}

		void Reset()
		{
			RestoreCameraMode();
		}


		DBOOL IsChaseView()		const { return (m_eCameraMode == CHASE); }
		DBOOL IsFirstPerson()	const { return (m_eCameraMode == FIRSTPERSON); }
		DRotation GetRotation() const { return m_rRotation; }
		DVector	GetPos()		const { return m_vPos; }

//		void SetDistUp(DFLOAT f)			{ m_CameraDistUp = f; }
		void SetPointAtOffset(DVector v)	{ VEC_COPY(m_TargetPointAtOffset, v); }
		void SetChaseOffset(DVector v)		{ VEC_COPY(m_TargetChaseOffset, v); }
		void SetFirstPersonOffset(DVector v) { VEC_COPY(m_TargetFirstPersonOffset, v); }
		void SetDeathOffset(DVector v) { VEC_COPY(m_TargetDeathOffset, v); }

		void SetOptZ(DFLOAT f)	 { m_OptZ = f; }

	private : // Data members

		// Target object for the camera to follow
		HLOCALOBJ		m_hTarget;
		CClientDE*		m_pClientDE;

		DRotation		m_rRotation;		// Replaces m_Angles
		DVector			m_vPos;				// Replaces m_Pos
	
		DVector			m_vLastTargetPos;	// The last position of our target
		DVector			m_vLastOptPos;		// Last optimal camera pos
		DRotation		m_rLastTargetRot;

//		CameraState		m_eOrientation;
//		CameraState		m_eSaveOrientation;

		CameraMode		m_eCameraMode;
		CameraMode		m_eSaveCameraMode;

		// Current optimal camera positions and orientation variables
		DFLOAT			m_OptX, m_OptY, m_OptZ;
		DFLOAT			m_DeathOptX, m_DeathOptZ;

		// Optimal camera distance from the target
		DFLOAT			m_CameraDistBack, m_CameraDistUp, m_CameraDistDiag;

		// Does the camera slide or not
		DBOOL			m_bSlide;

		// Offset from the target's position.
		// If a wall is between the optimal camera position and the target, the camera
		// will move closer to the target's position added to this offset. (Chase view)
		DVector			m_TargetChaseOffset;

		// Offset from the target's position...
		// The camera will point at this potision (Chase view)
		DVector			m_TargetPointAtOffset;

		DVector			m_TargetFirstPersonOffset;

		DVector			m_TargetDeathOffset;

		// How high from the center of the target should the camera be during a circle
		DFLOAT			m_CircleHeightOffset;
		// What is the radius of the circle
		DFLOAT			m_CircleRadius;
		// How long in seconds should it take for a circle to complete
		DFLOAT			m_CircleTime;

		DFLOAT			m_fTransitionStart;		// start time for transition
		DFLOAT			m_fTransitionTime;		// transition time

		DBOOL			m_bStartCircle;
		DFLOAT			m_CircleStartTime;
		DFLOAT			m_SaveAnglesY;
		DBOOL			m_bRestoreFirstPerson;


	// Internal functions..
	private:

		// Matrix apply function.
		DVector Apply(DVector right, DVector up, DVector forward, DVector copy);

		// Point the camera at a position from a position
		void PointAtPosition(DVector pos, DRotation rot, DVector pointFrom);

		// Point the camera in a direction
		void PointInDirection(DRotation rRot)
		{
			ROT_COPY(m_rRotation, rRot);
		}

		// Dynamically point the camera based on the distance between the camera and the target
		void PointAutomatic(DVector pos, DVector angles, DVector pointFrom)
		{
		}

		// Rotate the camera clockwise or counterclockwise around the target
		void RotateCameraState(DBOOL bClockwise);

		// Move the camera to a position over a time period
		void MoveCameraToPosition(DVector pos, DVector vStartPos, DFLOAT deltaTime);

		// Find the first person camera position
		DVector	FindFirstPersonCameraPosition(DVector vPos, DVector vF);

		// Find the optimal camera position
		DVector	FindOptimalCameraPosition();

		void PrintVector(DVector v);

		void CircleAroundTarget();

		void SaveCameraMode()
		{
			m_eSaveCameraMode = m_eCameraMode;
		}

		void RestoreCameraMode()
		{
			m_eCameraMode = m_eSaveCameraMode;
//			if((m_eCameraMode == CIRCLING) && (m_eSaveCameraMode == FIRSTPERSON))
//				GoFirstPerson();
//			else
//				m_eCameraMode = m_eSaveCameraMode;
		}

		DBOOL VCompare(DVector a, DVector b);
};


#endif // __PLAYER_CAMERA_H__