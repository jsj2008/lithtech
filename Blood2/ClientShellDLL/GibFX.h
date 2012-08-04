
 // ----------------------------------------------------------------------- //
//
// MODULE  : GibFX.h
//
// PURPOSE : Gib special fx class - Definition
//
// CREATED : 8/3/98
//
// ----------------------------------------------------------------------- //

#ifndef __GIBSFX_H__
#define __GIBSFX_H__

#include "SpecialFX.h"
#include "client_physics.h"
#include "SharedDefs.h"


#define TYPE_FLESH			0x00000001
#define TYPE_STONE			0x00000002
#define TYPE_WOOD			0x00000004
#define TYPE_METAL			0x00000008
#define TYPE_GLASS			0x00000010

#define TYPE_MASK			0x000000ff

#define SIZE_LARGE			0x00000100
#define SIZE_SMALL			0x00000200
#define SIZE_MASK			0x00000f00

#define TRAIL_BLOOD			0x00001000
#define TRAIL_SMOKE			0x00002000
#define TYPEFLAG_CUSTOM		0x00004000


struct GIBCREATESTRUCT : public SFXCREATESTRUCT
{
	GIBCREATESTRUCT::GIBCREATESTRUCT();

	DVector		m_Pos;
	DVector		m_Dir;
	DVector		m_Dims;
	HSTRING		m_hstrModel;
	HSTRING		m_hstrSkin;
	HSTRING		m_hstrSound;
	DDWORD		m_dwFlags;
	DFLOAT		m_fScale;
};

inline GIBCREATESTRUCT::GIBCREATESTRUCT()
{
	memset(this, 0, sizeof(GIBCREATESTRUCT));
}


class CGibFX : public CSpecialFX
{
	public :

		CGibFX() : CSpecialFX()
		{
			VEC_INIT( m_vPos );
			ROT_INIT( m_Rotation );
			VEC_INIT( m_vMinVel );
			VEC_INIT( m_vMaxVel );
			VEC_INIT( m_vDims );
			m_hstrModel = DNULL;
			m_hstrSkin = DNULL;
			m_hstrSound = DNULL;
			m_dwFlags = 0;

			m_fPitch			= 0.0f;
			m_fYaw				= 0.0f;
			m_fPitchVel			= 0.0f;
			m_fYawVel			= 0.0f;

			m_bFade = DFALSE;

			memset(&moveObj,0,sizeof(MovingObject));

			m_fScaleCount = 0.0f;

			m_bBlood = DFALSE;
			m_bSmoke = DFALSE;

			m_fStartTime	= -1.0f;
			m_fOffsetTime	= 0.0f;

			m_fScale = 1.0f;

			m_nType			= SURFTYPE_UNKNOWN;
		}

		~CGibFX()
		{
			if( m_hstrModel && m_pClientDE )
				m_pClientDE->FreeString( m_hstrModel );

			if( m_hstrSkin && m_pClientDE )
				m_pClientDE->FreeString( m_hstrSkin );

			if( m_hstrSound && m_pClientDE )
				m_pClientDE->FreeString( m_hstrSound );
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		DBOOL		  Term();
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();
		DBOOL		  UpdateMovement(MovingObject* pObject, ClientIntersectInfo* pInfo);

		HOBJECT	GetHandle()		{return m_hObject;}
		DBOOL	CreateBloodSpurt(DVector *pvDir);
		void	CreateBloodSplat(DVector *pvPos, ClientIntersectInfo *pInfo);

	private :

		DVector		m_vPos;
		DRotation	m_Rotation;
		DVector		m_vDir;
		DVector		m_vDims;
		DVector		m_vMinVel;
		DVector		m_vMaxVel;
		HSTRING		m_hstrModel;
		HSTRING		m_hstrSkin;
		HSTRING		m_hstrSound;
		DDWORD		m_dwFlags;

		DBOOL		m_bFade;
		int			m_nBounceCount;
		DFLOAT		m_fPitch;		
		DFLOAT		m_fYaw;
		DFLOAT		m_fPitchVel;
		DFLOAT		m_fYawVel;

		MovingObject	moveObj;
		DFLOAT		m_fScaleCount;

		DBOOL		m_bBlood;
		DBOOL		m_bSmoke;

		DFLOAT		m_fStartTime;
		DFLOAT		m_fOffsetTime;

		DFLOAT		m_fScale;

		int			m_nType;
};

#endif // __GIBSFX_H__
