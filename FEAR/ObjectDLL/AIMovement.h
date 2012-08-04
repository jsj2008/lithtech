// ----------------------------------------------------------------------- //
//
// MODULE  : AIMovement.h
//
// PURPOSE : 
//
// CREATED : 
//
// (c) 1997-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AI_MOVEMENT_H__
#define __AI_MOVEMENT_H__

class CAI;

enum ENUM_FaceType
{
	kFaceType_None,
	kFaceType_Object,
	kFaceType_Dir,
	kFaceType_Pos,
};

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIMovement
//
// PURPOSE : AI Movement manager
//
// ----------------------------------------------------------------------- //

class CAIMovement
{
	public : // Public methods

		// Ctors/Dtors/etc

		CAIMovement();
		~CAIMovement();

        bool InitAIMovement(CAI* pAI);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Methods

        void SetMovementDest(const LTVector& vDest);
		void ClearMovement();
		void SetParabola( float fHeight );
		void SetSpeed( float fSpeed ) { m_fSetSpeed = fSpeed; }
		void FaceDest( bool bFaceDest ) { m_bFaceDest = bFaceDest; }
		void AllowTargetPenetration( bool bPenetrate ) { m_bAllowTargetPenetration = bPenetrate; }

		bool	IsAtDest(const LTVector& vDest);

        void	Update();
		bool	UpdateConstantVelocity(EnumAnimDesc eMovementType, LTVector* pvNewPos);
		bool	UpdateMovementEncoding(EnumAnimDesc eMovementType, LTVector* pvNewPos);
		float	UpdateParabola();

		// Underwater?

        void SetUnderwater(bool bUnderwater) { m_bUnderwater = bUnderwater; }

		// Simple accessors

        bool IsUnset() const { return m_eState == eStateUnset; }
        bool IsSet() const { return m_eState == eStateSet; }
        bool IsDone() const { return m_eState == eStateDone; }
		const LTVector& GetDest() const { return m_vDest; }
		EnumAnimDesc GetCurrentMovementType() const { return m_eCurrentMovementType; }
		EnumAnimDesc GetLastMovementType() const { return m_eLastMovementType; }

		const LTVector&	GetRight() const {return m_vObjectRight; }
		const LTVector&	GetUp() const {return m_vObjectUp; }
		const LTVector&	GetForward() const {return m_vObjectForward; }
		const LTVector& GetTargetForward() { return m_vTargetForward; } 

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

private :

		void SetMovementDone();

		void	UpdateRotation();
		bool	UpdateMovement();
		float	GetSpeed();

		void	SetupMovementScaling();
		void	SetupHeightInterpolation();
		float	InterpolateMovementHeight();

		void	FacePath( EnumAnimDesc eMovementType, const LTVector& vDest, const LTVector& vDestDir );
		void	ApplyMovementEncodingToRot( LTRotation* pRot, const LTRigidTransform& tMovementEncoding );
		void	AvoidDynamicObstacles(LTVector* pvNewPos, State eState, EnumAnimDesc eMovementType);
		void	SlideAlongObstacles(LTVector* pvNewPos, State eStatePrev, EnumAnimDesc eMovementType);
		void	PreventMovementIntoObstacles( LTVector* pvNewPos, State eStatePrev, EnumAnimDesc eMovementType );

		void	FaceTargetRotImmediately();
		bool	FacePos(const LTVector& vTargetPos);
		bool	FaceDir(const LTVector& vDir);
		bool	FaceObject(HOBJECT hObj);

		void	LimitRotation( LTVector* pvNewPos, State eState );
		bool	PreventTargetPenetration( const LTVector& vNewPos );

private : // Private member variables

		CAI*				m_pAI;
		State				m_eState;
        LTVector			m_vDest;
		bool				m_bUnderwater;
        bool				m_bClimbing;
		bool				m_bFaceDest;
		float				m_fSetSpeed;
		EnumAnimDesc		m_eCurrentMovementType;
		EnumAnimDesc		m_eLastMovementType;
		double				m_fLastMovementUpdate;
		float				m_fAnimRate;
		bool				m_bNoDynamicPathfinding;
		bool				m_bAllowTargetPenetration;
		bool				m_bMoved;

		bool			m_bDoParabola;
		LTVector		m_vParabolaOrigin;
		float			m_fParabolaPeakDist;
		float			m_fParabolaPeakHeight;
		float			m_fParabola_a;
		bool			m_bParabolaPeaked;

		bool			m_bSetupScale;
		LTVector		m_vScale;

		bool			m_bSetupHeightInterpolation; //SAVE LOAD
		float			m_fHeightInterpolationTotalDist;
		float			m_fHeightInterpolationTotalHeight;
		float			m_fHeightInterpolationInitialHeight;

		float			m_fSpeed;

		// Rotation

		LTRotation			m_rTargetRot;               // Our target rotation
        LTRotation			m_rStartRot;                // Our starting rotation
        LTVector			m_vTargetForward;           // Object's target forward vector
        double				m_fRotationTime;            // Time for our rotation interpolation
        double				m_fRotationTimer;           // Timer for our rotation interpolation
		bool				m_bLimitRotation;

		LTVector			m_vObjectRight;
		LTVector			m_vObjectUp;
		LTVector			m_vObjectForward;

		double				m_flLastSlide;
};

#endif
