// ----------------------------------------------------------------------- //
//
// MODULE  : HeadBobMgr.h
//
// PURPOSE : Head Bob Mgr - Definition
//
// CREATED : 01/09/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HEAD_BOB_MGR__
#define __HEAD_BOB_MGR__

class CHeadBobMgr;
extern CHeadBobMgr* g_pHeadBobMgr;

class CHeadBobMgr
{
	public:

		CHeadBobMgr();

        LTBOOL   Init();
		void	Update();

		void	OnEnterWorld();

        void AdjustCameraPos(LTVector &vPos);

	private :

        LTFLOAT  m_fBobHeight;
        LTFLOAT  m_fBobAmp;
        LTFLOAT  m_fBobPhase;
        LTFLOAT  m_fSwayPhase;

		void	UpdateHeadBob();
		void	UpdateHeadCant();

		enum CantType { eCantNone=0, eCantLeft, eCantRight };
		void	AdjustCant(CantType eType, LTFLOAT fDelta, LTFLOAT fMaxCant);

};

inline void CHeadBobMgr::OnEnterWorld()
{
}

#endif  // __HEAD_BOB_MGR__
