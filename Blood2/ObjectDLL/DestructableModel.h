#ifndef __DESTRUCTABLEMODEL_H__
#define __DESTRUCTABLEMODEL_H__


#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "Destructable.h"
#include "Debris.h"
#include "B2BaseClass.h"


// CDestructableModel class
class CDestructableModel : public B2BaseClass
{
	public:

		CDestructableModel();
		virtual ~CDestructableModel();

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		DDWORD	GetSurfaceType() { return m_dwSurfType; }
	
		DBOOL	IsDestructable() const { return m_bDestructable; }
	protected:

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private:

		void	TouchNotify( HOBJECT hObj, DFLOAT fForce );
		void	Update( );
		void	InitialUpdate( DDWORD nData );

		void	HandleTriggerMessage( char *pMsg );
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);
		void	HandleDamage();
		void	HandleDestruction();
		char *	GetSlidingSound( );
		void	RotateDims( DRotation *pRot, DVector *pvDims );

	protected:

		CDestructable	m_damage;
		CDebris			m_Debris;
		HSTRING			m_hstrDamagedFilename;
		HSTRING			m_hstrDamagedSkinName;
		HSTRING			m_hstrDestroyFilename;
		HSTRING			m_hstrDestroySkinName;

		DVector			m_InitDims;
		DVector			m_DamageDims;
		DVector			m_DestroyDims;
		DDWORD			m_dwDestroyFlags;
		DBOOL			m_bDeadState;
		DFLOAT			m_fInitHitPoints;
		DFLOAT			m_fDamagedHitPoints;
		DFLOAT			m_fMass;

		DFLOAT			m_fAlpha;
		DFLOAT			m_fTargetAlpha;
		DFLOAT			m_fAlphaFadeRate;
		DFLOAT			m_fLastTime;

		DBOOL			m_bDestroyVisible;
		DBOOL			m_bDestroySolid;
		DBOOL			m_bDestroyGravity;
		DBOOL			m_bPushable;
		DBOOL			m_bDestructable;

		DDWORD			m_dwSurfType;

		DFLOAT			m_fScale;
		DVector			m_vTintColor;
		DBOOL			m_bChrome;

		DBOOL			m_bSliding;
		HSOUNDDE		m_hSlidingSound;
		DVector			m_vLastPos;
		DBYTE			m_nSlidingFrameCounter;
		DBOOL			m_bStandingOn;
		DBYTE			m_nStandingOnFrameCounter;
		HSTRING			m_hstrSlidingSound;

		DFLOAT			m_fYaw;
};


#endif // __DESTRUCTABLEMODEL_H__