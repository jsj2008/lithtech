// Gib.h: interface for the CGib class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GIB_H__4A6310F2_95F2_11D1_A430_006097098780__INCLUDED_)
#define AFX_GIB_H__4A6310F2_95F2_11D1_A430_006097098780__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "basedefs_de.h"

#include "destructablemodel.h"
#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "B2BaseClass.h"

#define GIB_HEAD		0x00000001
#define GIB_ARM			0x00000002
#define GIB_LEG			0x00000004
#define GIB_CORPSE		0x00000008

#define GIB_HUMAN		0x00000010
#define GIB_MONSTER		0x00000020

#define GIB_METAL		0x00000100
#define GIB_STONE		0x00000200
#define GIB_WOOD		0x00000400
#define GIB_GLASS		0x00000800
#define GIB_FLESH		0x00001000

#define GIB_LARGE		0x00010000
#define GIB_SMALL		0x00020000

// Special effects
#define GIB_SMOKETRAIL	0x00100000
#define GIB_BLOODTRAIL	0x00200000


#define NUM_FLESH_GIBS	3
#define NUM_METAL_GIBS	2
#define NUM_STONE_GIBS	2
#define NUM_WOOD_GIBS	3
#define NUM_GLASS_GIBS	2


class CGib : public B2BaseClass  
{
public:
		CGib();
		virtual ~CGib();
			
		// Corpse class used for re-animating these guys
		void	SetCorpseClass(HCLASS hClass) { m_hCorpseClass = hClass; }
		HCLASS	GetCorpseClass() { return m_hCorpseClass; }
		void	SetSurfaceType(DDWORD dwSurfType) { m_dwSurfType = dwSurfType; }
		void	SetKickable(DBOOL bKickable) { m_bKickable = bKickable; }
		DDWORD	GetSurfaceType() { return m_dwSurfType; }

		static void	CreateGibs(HOBJECT hObject, DDWORD dwGibType, DBYTE dmgType, DVector vDmgDir, int nNumDebris = 10);

	protected:

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	PostPropRead(ObjectCreateStruct *pStruct);
		void	InitialUpdate(DVector *pMovement);
		DBOOL	Update();

		void		HandleTouch(HOBJECT hObj, DFLOAT fData);
		void		PlayBounceSound();
		void		Damage(DVector vDir);
		void		InitProperties();

		char			m_szInitAnim[32];
		CDestructable	m_damage;
		HCLASS			m_hCorpseClass;
		DFLOAT			m_fMass;
		DFLOAT			m_fHitPoints;
		DDWORD			m_dwGibType;
		DBOOL			m_bGibbed;
		DFLOAT			m_fPitchVel;
		DFLOAT			m_fYawVel;
		DFLOAT			m_fRollVel;
		DFLOAT			m_fPitch;
		DFLOAT			m_fYaw;
		DFLOAT			m_fRoll;
		DFLOAT			m_fLastTime;
		DBOOL			m_bAddSmokeTrail;
		DBOOL			m_bAddBloodTrail;
		HOBJECT			m_hSmokeTrail;	// For a hunka hunka burnin' flesh
		HOBJECT			m_hBloodTrail;
		DBOOL			m_bPlaySplat;
		DVector			m_vLastPos;

		DBOOL			m_bKickable;	

		DBOOL			m_bFirstUpdate;

		DDWORD			m_dwSurfType;	// Surface type
		int				m_nBounceCount;

		int				m_nAxisAlign;

		DFLOAT			m_fRemoveTime;

		DFLOAT			m_fRotateTime;

		DBOOL			m_bAddBloodSpurt;
};

#endif // !defined(AFX_GIB_H__4A6310F2_95F2_11D1_A430_006097098780__INCLUDED_)
