// ----------------------------------------------------------------------- //
//
// MODULE  : DamageFXMgr.h
//
// PURPOSE : Damage FX Manager class - Definition
//
// CREATED : 1/20/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DAMAGE_FX_MGR_H__
#define __DAMAGE_FX_MGR_H__

//
// Includes...
//

#include "GameDatabaseMgr.h"
#include "VarTrack.h"
#include "TemplateList.h"
#include "ClientFXMgr.h"

//
// Defins...
//

	#define DMGFXMGR_MAX_NAME_LEN	32
	#define DMGFXMGR_MAX_PATH		64


//
// Forwards...
//
	
	class CDamageFXMgr;
	typedef HRECORD HDAMAGEFX;

//
// Globals...
//

	extern CDamageFXMgr	*g_pDamageFXMgr;


struct DAMAGEFX
{
	DAMAGEFX();
	~DAMAGEFX();

	bool		Init( HDAMAGEFX hDX );
	void		Start( );
	void		Stop( bool bFade = true );
	void		StartSoundAndVisuals();
	void		StopSoundAndVisuals( bool bFade = true );
	void		Update( float fElapsedTime );

	inline bool IsActive( ) const { return m_bActive || m_bFade; }
	
	uint32		m_nID;
	DamageFlags	m_nDamageFlag;

	const char	*m_szName;
	const char	*m_szStartSound;
	const char	*m_szLoopSound;
	const char	*m_szFXName;
	const char	*m_szTakingHealthFXName;
	const char	*m_szTakingArmorFXName;
	const char	*m_sz3rdPersonFXName;
	const char	*m_sz1stPersonInstFXName;
	const char	*m_sz3rdPersonInstFXName;
	const char	*m_sz1stPersonDeathFXName;
	const char	*m_sz3rdPersonDeathFXName;
	const char	*m_szBodyFXName;
	char		m_szVarTrackName[64];

	float		m_fRotSpeed;		// Rate at which we roll around the cameras forward
	float		m_fRotMax;			// The maximum angle we can roll
	float		m_fRotDir;			// Which direction are we rolling
    float		m_fOffsetRot;		
    float		m_fMaxRot;
    float		m_fMinRot;
    float		m_fMoveMult;
	float		m_fMinFXPercent;

	bool		m_bActive;			// Are we active?
	bool		m_bFade;			// Are we fading?
	float		m_fFadeTm;			// How long does it take to fade this fx

	bool		m_bAllowMovement;
	bool		m_bAllowInput;
	bool		m_bAllowWeapons;

	bool		m_bJumpRequested;	// Did the player tap jump?
	int8		m_nNumJumps;		// How many times the player jumped so far
	int8		m_nJumpsToEscape;	// How many jumps it takes to free the player

	HLTSOUND	m_hLoopSound;		

	//the timer that is used to time transitions between states.
	float		m_fElapsedTime;		// the amount of time that has already elapsed
	float		m_fEndTime;			// The time that the current state ends at
	VarTrack	m_vtTestFX;			// Used for testing the specific DamageFX

	bool		m_bInstantEffect;	// Does this effect go away immediately instead of looping?

	CClientFXLink	m_DamageFXInstance;	// A link to an instance of a FXEdit created FX

	bool		m_bAttachCameraToAni;	// Should we attach the camera to the head of the animation
	bool		m_bShowClientModel;		// Should make the local client model visible

	bool		m_bAnimationControlsFX;		// Does the animation contain keys for turning certain fx on and off
	bool		m_bUpdateSoundAndVisuals;	// Should we update the sound and visual fx... can be controlled by anis

	bool		m_bRemoveOnNextInstantDamage;	// Does this effect get removed if new damage of the same type comes in while it's still playing?  This will allow the next DamageFX of the same damage type to get used instead - for progressively worse damage withing short periods of time.
	bool		m_bRenewOnNextInstantDamage;	// Does this effect get readded/reset if new damage of the same type comes in while it's still playing?
};

typedef CTList<DAMAGEFX*> DamageFxList;
typedef	CTList<DAMAGEFX*> ActiveDamageFxList;

class CDamageFXMgr : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CDamageFXMgr );

	public: // Methods...

		bool		Init( const char *szDatabaseFile = DB_Default_File );
		void		Term();

		DAMAGEFX	*GetDamageFX( uint32 nID );
		DAMAGEFX	*GetDamageFX( char *pName );
		
		int			GetNumDamageFX() const { return m_lstDamageFx.GetLength(); }

		DAMAGEFX	*GetFirstDamageFX() { return ( m_lstDamageFx.GetLength() ? *m_lstDamageFx.GetItem( TLIT_FIRST ) : NULL ); }
		DAMAGEFX	*GetNextDamageFX() { return ( m_lstDamageFx.GetLength() ? 
													(m_lstDamageFx.GetItem( TLIT_NEXT ) ? 
														*m_lstDamageFx.GetItem( TLIT_CURRENT ) : NULL) : NULL ); }

		DAMAGEFX	*GetFirstActiveFX() { return ( m_lstActiveDmgFx.GetLength() ? *m_lstActiveDmgFx.GetItem( TLIT_FIRST ) : NULL ); }
		DAMAGEFX	*GetNextActiveFX() { return ( m_lstActiveDmgFx.GetLength() ?
														(m_lstActiveDmgFx.GetItem( TLIT_NEXT ) ?
															*m_lstActiveDmgFx.GetItem( TLIT_CURRENT ) : NULL) : NULL); }

		bool		IsDamageActive( DamageFlags nDmgFlag );

		bool		AllowMovement() const { return m_bAllowMovement; }
		bool		AllowInput() const { return m_bAllowInput; }
		bool		AllowWeapons( ) const { return ( m_nDisableWeaponCounts == 0 ); }

		void		SetDamageFXMovementAndInput( bool bMove, bool bInput, bool bAlloWeapons );
		
		void		Update();
		void		Clear();

		bool		OnModelKey( HLOCALOBJ hObj, ArgList *pArgs );

		void		AddToActive(DAMAGEFX* pDFX) {m_lstActiveDmgFx.Add( pDFX );}
		void		RemoveFromActive(DAMAGEFX* pDFX) {m_lstActiveDmgFx.Remove( pDFX );}


	private :	// Members...

		DamageFxList		m_lstDamageFx;
		ActiveDamageFxList	m_lstActiveDmgFx;
		bool				m_bAllowMovement;
		bool				m_bAllowInput;
		uint32				m_nDisableWeaponCounts;

		HCATEGORY			m_hDamageFXCat;

};

#endif //__DAMAGE_FX_MGR_H__