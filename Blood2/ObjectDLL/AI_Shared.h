// ----------------------------------------------------------------------- //
//
// MODULE  : AI_Shared.h
//
// PURPOSE : Shared AI modules
//
// CREATED : 
//
// ----------------------------------------------------------------------- //

#ifndef __SHARED_AI_H__
#define __SHARED_AI_H__

class AI_Shared 
{
	public :

		AI_Shared();
		~AI_Shared() {}

// Need New Functions        
		HOBJECT GetNewTarget()		{	return DNULL;	}
		DBOOL PickBestWeapon()		{	return DTRUE;	}

        DVector GetEyeLevel(HOBJECT m_hObject);
        
        DBOOL   IsLedge(HOBJECT m_hObject, DFLOAT m_fDist);
        
        DFLOAT  DistToWorld(HOBJECT m_hObject, DVector vDir);
        DFLOAT  DistToWorldForward(HOBJECT m_hObject);
		DFLOAT  DistToWorldBackward(HOBJECT m_hObject);
        DFLOAT  DistToWorldRight(HOBJECT m_hObject);
        DFLOAT  DistToWorldLeft(HOBJECT m_hObject);
        
        DBOOL   IsObjectVisible(HOBJECT m_hObject, HOBJECT hTestObj);
        DBOOL   IsObjectVisibleToAI(HOBJECT m_hObject, HOBJECT hTestObj, DFLOAT m_fSeeingDist);
        
        HOBJECT SenseForEnemy(HOBJECT m_hObject, DFLOAT fRange);
        HOBJECT SmellForEnemy(HOBJECT m_hObject, DFLOAT fRange);
        HOBJECT LookForEnemy(HOBJECT m_hObject, DFLOAT fRange);
        
        HOBJECT FindObjectInRadius(HOBJECT m_hObject, HCLASS hObjectTest, DFLOAT fRange, DBOOL bCheckVisible, char* sObjName, DBOOL bNear);
        
        DBOOL   CheckForProjectile(HOBJECT m_hObject);
        
        void    FacePosition(HOBJECT m_hObject, DVector vTargetPos);
        DFLOAT  PitchToObject(HOBJECT m_hObject, HOBJECT hPTarget);
        
        void    Strafe(HOBJECT m_hObject, DFLOAT m_strafeSpeed);
        DBOOL   Jump(HOBJECT m_hObject, DFLOAT m_YjumpSpeed, DFLOAT m_ZjumpSpeed);
        int     WalkThisWay(HOBJECT m_hObject, DFLOAT m_fSpeed, int m_theAnima);

        int     MoveBackward(HOBJECT m_hObject, DFLOAT m_fSpeed, int m_theAnima);

        DBOOL   StuckOnSomething(HOBJECT m_hObject, DFLOAT m_fSpeed, DVector m_vLastPos);

		void	Roll(HOBJECT m_hObject, DVector vDir, DFLOAT fSpeed);
        void    TurnRorL(HOBJECT m_hObject, DFLOAT m_fRadius);
        void    TurnR(HOBJECT m_hObject, DFLOAT m_fRadius);
        void    TurnL(HOBJECT m_hObject, DFLOAT m_fRadius);
		DBOOL	TurnToClear(HOBJECT hObject);
        
        
        DBOOL   FaceTarget(HOBJECT m_hObject, HOBJECT m_hTarget);

		void	CreateGibs(HOBJECT hObject,DDWORD dwFlags, int nCorpse = 0, int nNode = 999);
		void	CreateLimb(HOBJECT hObject, int nNode, DVector vDir);
		DBOOL	HideLimb(HOBJECT hObject, int nNode, DBOOL bInverse = DFALSE);
		DBOOL	HideNodeAndSubNodes(HOBJECT hObject, int nNode, DBOOL bHide);
		void	ResetNodes(HOBJECT hObject);
        
        void    SetAnimation(HOBJECT m_hObject, int nAni);
        void    ForceAnimation(HOBJECT m_hObject, int nAni);

// 10/24/97
        DBOOL LoadBrains(char* pfilename);
        
        // AIState BrainState(char* sName);
        // AISubState BrainSubState(char* sName);
        // AIMetaCommand BrainMetaCommand(char* sName);


	protected : // Member Variables

    DBOOL m_bTurnR;
    DBOOL m_bTurnL;

    DVector m_vLastStrafePos;
    DBOOL m_bStrafing;
    
    DBOOL m_bJumping;

};

#endif // __SHARED_AI_H__