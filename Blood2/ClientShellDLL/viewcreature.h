
#ifndef __VIEWCREATURE_H__
#define __VIEWCREATURE_H__

#include "cpp_clientshell_de.h"
#include "SharedDefs.h"

#define	HAND_OFFSET_U	-1.0f
#define	HAND_OFFSET_R	0.0f
#define	HAND_OFFSET_F	1.1f

#define	BONE_OFFSET_U	-2.5f
#define	BONE_OFFSET_R	0.0f
#define	BONE_OFFSET_F	0.6f

#define	THIEF_OFFSET_U	-0.4f
#define	THIEF_OFFSET_R	0.0f
#define	THIEF_OFFSET_F	2.0f

#define NUM_USE_HITS	5
#define MAXNUM_USE_HITS	25

class CViewCreature
{
	public:

		CViewCreature();
		~CViewCreature();

		DDWORD		GetType()			{return m_dwType;}
		DVector		GetLightScale()		{return m_vLightScale;}

		HLOCALOBJ	Create(CClientDE* pClientDE, DDWORD dwType, HLOCALOBJ hServer, HLOCALOBJ hEnemy);
		void		Term();
		void		Detach();
		void		UseKeyHit();
		void		Update(DFLOAT fPitch, DFLOAT fYaw, DVector *pos);

		HLOCALOBJ	m_hObject;			
		HLOCALOBJ	m_hServerObject;	// Local handle to Server-side object
		HLOCALOBJ	m_hEnemyObject;

	private:

		DDWORD		m_dwType;
		DFLOAT		m_fDmgTime;
		DFLOAT		m_fLastHitUse;
		int			m_nNumUseHits;
		int			m_nTotalNumUseHits;
		DVector		m_vLightScale;

		CClientDE*	m_pClientDE;
};

#endif __VIEWCREATURE_H__