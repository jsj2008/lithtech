// ----------------------------------------------------------------------- //
//
// MODULE  : CameraOffsetMgr.h
//
// PURPOSE : Camera offset mgr - Definition
//
// CREATED : 8/23/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERA_OFFSET_MGR__
#define __CAMERA_OFFSET_MGR__

#include "WaveFn.h"

#define	MAX_CAMERA_DELTAS			20
#define MAX_STATIC_CAMERA_DELTAS	10

// The CameraAdjustVar struct is used to maintain a single camera orientation
// variable (i.e., pitch, yaw, roll, pos.x, pos.y, or pos.z)...

struct CameraAdjustVar
{
	CameraAdjustVar()
	{
		Init();
	}

	void		UpdateVar(float fTimeDelta);
	float		GetValue()	const { return m_fValue; }

	float		fVar;		// Total value change
	float		fTime1;		// Time to apply change
	float		fTime2;		// Time to reverse change
	WaveType	eWave1;		// How var changes
	WaveType	eWave2;		// How reverse var changes

    bool       bIncrement; // Return value as an increment

  private :

	int			m_nDir;				// Forward (1) or reverse (-1)
	float		m_fValue;			// Current variable value
	float		m_fCurTime;			// Current time delta
	float		m_fRealValue;		// Real calculated value
	float		m_fLastRealValue;	// Last real calculated value

	inline void	Init()
	{
		m_nDir		= 1;
		m_fValue	= 0.0f;
		m_fCurTime	= 0.0f;

		m_fRealValue = 0.0f;
		m_fLastRealValue = 0.0f;

        bIncrement = false;

		fVar	= 0.0f;
		fTime1	= 0.0f;
		fTime2	= 0.0f;
		eWave1	= Wave_Linear;
		eWave2	= Wave_Linear;
	}
};


// CameraDelta holds all of the possible camera offset variables...

struct CameraDelta
{
	CameraDelta()
	{
        Pitch.bIncrement	= false;
        Roll.bIncrement		= false;
        Yaw.bIncrement		= false;
	}

	inline float GetTotalDelta() const
	{
		double fTotal = 0.0f;

		fTotal += fabs(Pitch.fVar);
		fTotal += fabs(Yaw.fVar);
		fTotal += fabs(Roll.fVar);
		fTotal += fabs(PosX.fVar);
		fTotal += fabs(PosY.fVar);
		fTotal += fabs(PosZ.fVar);

		return (float)fTotal;
	}

	CameraAdjustVar		Pitch;
	CameraAdjustVar		Yaw;
	CameraAdjustVar		Roll;
	CameraAdjustVar		PosX;
	CameraAdjustVar		PosY;
	CameraAdjustVar		PosZ;
};


class CCameraOffsetMgr
{
	public:

		CCameraOffsetMgr();

        bool   Init();
		void	Update();

		void	AddDelta(CameraDelta & delta);

		CameraDelta* GetStaticDelta(int nIndex);
		void	     SetStaticDelta(CameraDelta & delta, int nIndex);

        inline LTVector  GetPosDelta()           const { return m_vPosDelta; }
        inline LTVector  GetPitchYawRollDelta()  const { return m_vPitchYawRollDelta; }

	private :

		void			ValidateDeltas();

		CameraDelta		 m_CameraDeltas[MAX_CAMERA_DELTAS];
		CameraDelta		 m_StaticCameraDeltas[MAX_STATIC_CAMERA_DELTAS];
        LTVector         m_vPitchYawRollDelta;
        LTVector         m_vPosDelta;

		void			ProcessTestingVars();
};


#endif  // __CAMERA_OFFSET_MGR__

